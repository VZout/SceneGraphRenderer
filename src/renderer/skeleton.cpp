#include "skeleton.h"

#include "model.h"
#include "animation_manager.h"

namespace rlr {

Skeleton::Skeleton() {
}

Skeleton::Skeleton(std::vector<Bone> bones, aiMatrix4x4 global_invere_transform) {
	Init(bones, global_invere_transform);
}

void Skeleton::Init(std::vector<Bone> bones, aiMatrix4x4 global_invere_transform) {
	this->bones = bones;
	this->global_invere_transform = global_invere_transform;

	for (int i = 0; i < this->bones.size(); i++) {
		this->bones.at(i).parent_skeleton = this;
	}
}

void Skeleton::UpdateBoneMatrices() {
	bone_mats.clear();

	for (int i = 0; i < 100; i++) {
		if (i > bones.size() - 1) { // if we are out of bones pass identity matrices
			bone_mats.push_back(aiMatrix4x4()); // Identity Matrix
		}
		else {
			aiMatrix4x4 concatenated_transformation = bones.at(i).node->mTransformation * bones.at(i).GetParentTransforms();
			bone_mats.push_back(bones.at(i).offset_matrix * concatenated_transformation * global_invere_transform);
		}
	}
}

void Skeleton::PlayAnimation(Animation* anim, bool loop, bool reset) {
	current_anim = anim;
	anim_loop = loop;

	if (!current_anim || reset) {
		anim_time = current_anim->start;
	}
}

void Skeleton::StopAnimation() {
	anim_time = current_anim->end;
	current_anim = nullptr;
}

void Skeleton::Update(float delta) {
	if (current_anim) {
		for (int i = 0; i < bones.size(); i++) {
			bones.at(i).UpdateKeyframeTransform(anim_time);
		}

		anim_time += (delta * 1000.0f) * (1.f / current_anim->frame_rate);
		if (anim_time > current_anim->end) {
			if (anim_loop) {
				anim_time = current_anim->start;
			}
			else {
				StopAnimation();
			}
		}
	}

	UpdateBoneMatrices();
}

} /* rlr */