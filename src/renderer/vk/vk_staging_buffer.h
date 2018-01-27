#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_defines.h"

namespace rlr {

FWRD_DEVICE_PTR

struct StagingBuffer {
	INTERNAL_DEVICE_PTR

	vk::Buffer buffer;
	vk::DeviceMemory buffer_memory;

	vk::Buffer staging_buffer;
	vk::DeviceMemory staging_buffer_memory;

	uint64_t m_size;
};

} /* rlr */
