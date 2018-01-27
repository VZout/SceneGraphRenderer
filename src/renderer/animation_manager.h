#pragma once

#include <string>

namespace rlr {

struct Animation {
	std::string name;
	float start = 0;
	float end = 0;
	unsigned int priority = 0;
	float frame_rate = 30;

	Animation();
	Animation(std::string name, float start, float end, int priority, float frame_rate = 30.0f);
	~Animation();

};

class AnimationManager {
public:
	AnimationManager();
	~AnimationManager();
};

} /* rlr */
