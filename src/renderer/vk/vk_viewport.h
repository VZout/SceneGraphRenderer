#pragma once

#include <vulkan/vulkan.hpp>

namespace rlr {

struct Viewport {
	vk::Viewport m_viewport;
	vk::Rect2D m_scissor;
};

} /* rlr */
