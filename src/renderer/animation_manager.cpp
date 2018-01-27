#include "animation_manager.h"

namespace rlr {

Animation::Animation() {

}

Animation::Animation(std::string name, float start, float end, int priority, float frame_rate) : name(name), start(start), end(end), priority(priority), frame_rate(frame_rate) {

}

Animation::~Animation() {

}

AnimationManager::AnimationManager() {

}

AnimationManager::~AnimationManager() {

}

} /* rlr */
