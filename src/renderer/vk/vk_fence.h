#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_defines.h"

namespace rlr {

FWRD_DEVICE_PTR

struct Fence {
	INTERNAL_DEVICE_PTR

	vk::Fence native;
};

} /* rlr */
