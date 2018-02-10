#include "bone.h"

#include "model.h"


namespace rlr {

Bone::Bone() : name ("UNKNOWN BONE"), id(-2) {
}

Bone::Bone(Mesh* mesh, unsigned int id, std::string name, aiMatrix4x4 offset_mat) : mesh(mesh), id(id), name(name), offset_matrix(offset_mat) {
}

aiMatrix4x4 Bone::GetParentTransforms() {
	Bone* b = parent_bone;

	// Gather Transforms
	std::vector<aiMatrix4x4> mats;
	while (b != nullptr) {
		aiMatrix4x4 tmp_mat = b->node->mTransformation;
		mats.push_back(tmp_mat);

		b = b->parent_bone; 
	}

	// Combine Transforms
	aiMatrix4x4 concatenated_transforms = aiMatrix4x4();
	for (int i = 0; i < mats.size(); i++) {
		concatenated_transforms *= mats.at(i);
	}

	return concatenated_transforms;
}

unsigned int Bone::FindPosition(float time) {
	for (unsigned int i = 0; i < anim_node->mNumPositionKeys - 1; i++) {
		if (time < (float)anim_node->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0; 
}

unsigned int Bone::FindRotation(float time) {
	for (unsigned int i = 0; i < anim_node->mNumRotationKeys - 1; i++) {
		if (time < (float)anim_node->mRotationKeys[i + 1].mTime)
			return i;
	}

	return 0;
}

aiVector3D Bone::CalcInterpolatedPosition(float time) {
	if (anim_node->mNumPositionKeys == 1) {
		return  anim_node->mPositionKeys[0].mValue;
	}

	unsigned int PositionIndex = FindPosition(time);
	unsigned int NextPositionIndex = (PositionIndex + 1);

	float delta = anim_node->mPositionKeys[NextPositionIndex].mTime - anim_node->mPositionKeys[PositionIndex].mTime;
	float factor = (time - (float)anim_node->mPositionKeys[PositionIndex].mTime) / delta;

	const aiVector3D start = anim_node->mPositionKeys[PositionIndex].mValue;
	const aiVector3D end = anim_node->mPositionKeys[NextPositionIndex].mValue;

	aiVector3D retval;
	Assimp::Interpolator<aiVector3D> inter;
	inter(retval, start, end, factor);

	return retval;
}

aiQuaternion Bone::CalcInterpolatedRotation(float time) {
	if (anim_node->mNumRotationKeys == 1) {
		return anim_node->mRotationKeys[0].mValue;
	}

	unsigned int RotationIndex = FindRotation(time);
	unsigned int NextRotationIndex = (RotationIndex + 1);

	float delta = anim_node->mRotationKeys[NextRotationIndex].mTime - anim_node->mRotationKeys[RotationIndex].mTime;
	float factor = (time - (float)anim_node->mRotationKeys[RotationIndex].mTime) / delta;

	const aiQuaternion& start = anim_node->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& end = anim_node->mRotationKeys[NextRotationIndex].mValue;

	aiQuaternion retval;
	Assimp::Interpolator<aiQuaternion> inter;
	inter(retval, start, end, factor);

	return retval;
}

void Bone::UpdateKeyframeTransform(float time) {
	if (anim_node == nullptr) {
		node->mTransformation = aiMatrix4x4(); // No animation? Use a identity matrix.
		return;
	}

	pos = CalcInterpolatedPosition(time);
	rot = CalcInterpolatedRotation(time);
	scale = aiVector3D(1.0, 1.0, 1.0);

	aiMatrix4x4 pos_mat = aiMatrix4x4::Translation(pos, pos_mat);
	aiMatrix4x4 rot_mat = aiMatrix4x4(rot.GetMatrix());
	aiMatrix4x4 scale_mat = aiMatrix4x4::Scaling(scale, scale_mat);

	aiMatrix4x4 mat = aiMatrix4x4();
	mat *= pos_mat;
	mat *= rot_mat;
	mat *= scale_mat;
	mat = mat.Transpose();

	node->mTransformation = mat;
}

} /* rlr */