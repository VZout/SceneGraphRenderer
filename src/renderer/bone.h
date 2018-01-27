#pragma once

#include <string>
#include <vector>

#include <assimp/scene.h>
#include <iostream>

#include "math\mat.hpp"
#include <GLFW/glfw3.h>

namespace rlr {

struct Mesh;
struct Animation;

class GameObject;
class Skeleton;

class Bone {
public:
	Bone();
	Bone(Mesh* mesh, unsigned int id, std::string name, aiMatrix4x4 offset_mat);

	int id;
	std::string name;
	Mesh* mesh = nullptr;
	aiNode* node = nullptr;

	aiNodeAnim* anim_node = nullptr;
	Bone* parent_bone = nullptr;
	aiMatrix4x4 parent_transforms = aiMatrix4x4();
	aiMatrix4x4 offset_matrix = aiMatrix4x4();
	Skeleton* parent_skeleton = nullptr;
	
	aiVector3D pos;
	aiQuaternion rot;
	aiVector3D scale;

	aiMatrix4x4 GetParentTransforms();
	unsigned int FindPosition(float time);
	aiVector3D CalcInterpolatedPosition(float time);
	unsigned int FindRotation(float time);
	aiQuaternion CalcInterpolatedRotation(float time);

	void UpdateKeyframeTransform(float time);

};

} /* rlr */