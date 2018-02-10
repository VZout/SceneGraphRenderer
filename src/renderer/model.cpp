#include "model.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <algorithm>

#include "vertex.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "animation_manager.h"
#include "bone.h"

namespace rlr {

aiNodeAnim* FindAiNodeAnim(Model& model, std::string const& name) {
	for (int i = 0; i < model.ai_nodes_anim.size(); i++) {
		if (model.ai_nodes_anim.at(i)->mNodeName.data == name) {
			return model.ai_nodes_anim.at(i);
		}
	}

	return nullptr;
}

aiNode* FindAiNode(Model& model, std::string const& name) {
	for (int i = 0; i < model.ai_nodes.size(); i++) {
		if (model.ai_nodes.at(i)->mName.data == name)
			return model.ai_nodes.at(i);
	}

	return nullptr;
}

Bone* FindBone(Model& model, std::string const& name) {
	for (int i = 0; i < model.bones.size(); i++)
	{
		if (model.bones.at(i).name == name)
			return &model.bones.at(i);
	}

	return nullptr;
}

// Gather animation nodes and store them in a model
void GatherAnimationNodes(Model& model, const aiScene *scene) {
	if (scene->mNumAnimations == 0)
		return;

	for (auto i = 0; i < scene->mNumAnimations; i++) {
		std::string name = scene->mAnimations[i]->mName.C_Str();
		double duration = scene->mAnimations[i]->mDuration - 1;
		double frame_rate = scene->mAnimations[i]->mTicksPerSecond;
		model.animations.push_back(new Animation(name, 0, duration, 0, frame_rate));
	}

	for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++)
		model.ai_nodes_anim.push_back(scene->mAnimations[0]->mChannels[i]);

}

// Gather all bones and convert them to something usable.
void GatherBonesAsCustom(Model& model, const aiScene *scene) {
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		for (unsigned int j = 0; j < scene->mMeshes[i]->mNumBones; j++) {
			std::string b_name = scene->mMeshes[i]->mBones[j]->mName.data;
			aiMatrix4x4 b_mat = scene->mMeshes[i]->mBones[j]->mOffsetMatrix.Transpose();

			Bone bone(&model.meshes.at(i), i, b_name, b_mat);
			bone.node = FindAiNode(model, b_name);
			bone.anim_node = FindAiNodeAnim(model, b_name);

			model.bones.push_back(bone);

			if (bone.anim_node == nullptr) {
				// std::cout << "No Animations were found for " + b_name << std::endl;
			}
		}
	}

	// Now we have all the bones and their nodes we can set the parent.
	for (int i = 0; i < model.bones.size(); i++) {
		std::string b_name = model.bones.at(i).name;
		aiNode* b_node = FindAiNode(model, b_name);
		std::string parent_name = b_node->mParent->mName.data;

		Bone* p_bone = FindBone(model, parent_name);

		model.bones.at(i).parent_bone = p_bone;

		if (p_bone == nullptr) {
			// std::cout << "Parent Bone for " << b_name << " does not exist (is nullptr)" << std::endl;
		}
	}
}

Mesh ProcessMesh(Model& model, aiMesh *mesh, const aiScene *scene) {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	// Walk through each of the mesh's vertices and set the position, normal, texcorods and etc.
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		fm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.m_pos = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.m_normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) {
			fm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.m_texCoord = vec;
		}
		else {
			vertex.m_texCoord = fm::vec2(0.0f, 0.0f);
		}

		// tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		//vertex.m_tangent = vector;
		// bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		//vertex.m_bitangent = vector;
		vertices.push_back(vertex);
	}

	// Find and add all indicies to the mesh's indices vector.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	std::vector<float> bone_weights;
	bone_weights.resize(mesh->mNumVertices * WEIGHTS_PER_VERTEX);

	for (auto i = 0; i < bone_weights.size(); i++)
		bone_weights[i] = 0;

	// Go through all the bones and set the correct weight and id.
	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* ai_bone = mesh->mBones[i];
		for (int j = 0; j < ai_bone->mNumWeights; j++) {
			aiVertexWeight weight = ai_bone->mWeights[j];
			unsigned int vertexStart = weight.mVertexId * WEIGHTS_PER_VERTEX;
			for (int k = 0; k < WEIGHTS_PER_VERTEX; k++) {
				if (bone_weights.at(vertexStart + k) == 0) {
					bone_weights.at(vertexStart + k) = weight.mWeight;
					vertices.at(weight.mVertexId).id[k] = i + model.id_offset;
					vertices.at(weight.mVertexId).weight[k] = weight.mWeight;
					break;
				}
			}
		}
	}

	model.id_offset = mesh->mNumBones;

	// return a mesh object created from the extracted mesh data
	return Mesh{ vertices, indices /*, textures*/ };
}

void ProcessNode(Model& model, aiNode* node, const aiScene* scene) {
	model.ai_nodes.push_back(node);

	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model.meshes.push_back(ProcessMesh(model, mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessNode(model, node->mChildren[i], scene);
	}
}

void Load(Model& model, std::string const& path) {
	const aiScene *scene = model.importer.ReadFile(path, aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << model.importer.GetErrorString() << std::endl;
		return;
	}
	model.directory = path.substr(0, path.find_last_of('/'));

	GatherAnimationNodes(model, scene);
	model.global_invere_transform = scene->mRootNode->mTransformation.Inverse();

	ProcessNode(model, scene->mRootNode, scene);

	GatherBonesAsCustom(model, scene);

	if (model.meshes.size() > 0)
		model.meshes[0].skeleton.Init(model.bones, model.global_invere_transform);
}

void Stage(Model& model, Device* device, CommandList* cmd_list) {
	for (auto i = 0; i < model.meshes.size(); i++) {
		Create(&model.meshes[i].vb, device, model.meshes[i].vertices.data(), model.meshes[i].vertices.size() * sizeof(Vertex), sizeof(Vertex), ResourceState::VERTEX_AND_CONSTANT_BUFFER);
		Create(&model.meshes[i].ib, device, model.meshes[i].indices.data(), sizeof(DWORD) * model.meshes[i].indices.size(), sizeof(Vertex), ResourceState::VERTEX_AND_CONSTANT_BUFFER);

		StageBuffer(model.meshes[i].vb, cmd_list);
		StageBuffer(model.meshes[i].ib, cmd_list);
	}
}

void Destroy(Model& model) {
	for (auto i = 0; i < model.meshes.size(); i++) {
		FreeStagingBuffer(model.meshes[i].ib);
		FreeStagingBuffer(model.meshes[i].vb);
		Destroy(model.meshes[i].ib);
		Destroy(model.meshes[i].vb);
	}
}

} /* rlr */
