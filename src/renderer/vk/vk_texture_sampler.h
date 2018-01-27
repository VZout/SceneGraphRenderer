#pragma once

#include <vulkan/vulkan.hpp>

namespace rlr {

struct TextureSampler {
	vk::Sampler m_native;
};

} /* rlr */
