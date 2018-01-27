#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_defines.h"

namespace rlr {

FWRD_DEVICE_PTR

struct CommandList {
	INTERNAL_DEVICE_PTR

	vk::CommandBuffer* m_native;
	unsigned int m_num;
	unsigned int m_current_frame_idx;
};

} /* rlr */
