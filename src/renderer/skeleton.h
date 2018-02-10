#pragma once

#include <string>
#include <vector>

#include <assimp/scene.h>
#include <iostream>

#include "bone.h"
#include <GLFW/glfw3.h>

namespace rlr {

struct Animation;
struct Mesh;

class Skeleton {
private:
	void UpdateBoneMatrices();

public:
	std::vector<Bone> bones;
	aiMatrix4x4 global_invere_transform;
	std::vector<aiMatrix4x4> bone_mats;
	
	float anim_time = 0;
	bool anim_loop;
	Animation* current_anim = nullptr;


	Skeleton();
	Skeleton(std::vector<Bone> bones, aiMatrix4x4 global_inverese_transform);

	void Init(std::vector<Bone> bones, aiMatrix4x4 global_inverese_transform);
	void PlayAnimation(Animation* anim, bool loop = true, bool reset = true);
	void StopAnimation();

	void Update(float delta);
};

} /* rlr */