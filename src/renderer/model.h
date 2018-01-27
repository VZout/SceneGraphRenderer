#pragma once

#include <string>
#include <vector>
#include <map>
#include <assimp/Importer.hpp>

#include "math/vec.hpp"
#include "interface.h"
#include "bone.h"
#include "skeleton.h"
#include "vertex.h"

namespace rlr {

struct Animation;

static int WEIGHTS_PER_VERTEX = 4;

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	StagingBuffer vb;
	StagingBuffer ib;

	Skeleton skeleton;
};

struct Model {
	int id_offset = 0;

	std::vector<aiBone*> boned;

	std::vector<Mesh> meshes;
	std::vector<Bone> bones;
	std::vector<Animation*> animations;
	aiMatrix4x4 global_invere_transform;
	std::vector<aiNode*> ai_nodes;
	std::vector<aiNodeAnim*> ai_nodes_anim;
	std::string directory;
	Assimp::Importer importer;
};

void Load(Model& model, std::string const & path);
void Stage(Model& model, Device& device, CommandList& cmd_list);
void Destroy(Model& model);

} /* rlr */
