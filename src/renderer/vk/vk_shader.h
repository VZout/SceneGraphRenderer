#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_defines.h"

namespace rlr {

FWRD_DEVICE_PTR

struct Shader {
	INTERNAL_DEVICE_PTR

	vk::ShaderModule module;
};

} /* rlr */
