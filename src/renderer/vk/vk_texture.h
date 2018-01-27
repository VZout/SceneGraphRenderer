#pragma once

#include <vulkan/vulkan.hpp>

#include "vk_defines.h"

namespace rlr {

struct TextureDescription {
	int width;
	int height;
	Format format;
};

FWRD_DEVICE_PTR

struct Texture {
	INTERNAL_DEVICE_PTR

	vk::Image staging_img;
	vk::DeviceMemory staging_img_mem;
	Resource resource;

	TextureDescription desc;

	int bytes_per_row;
	char* buffer;
	unsigned char* data;
	int size;
};

} /* rlr */
