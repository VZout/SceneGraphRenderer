#pragma once

#include <vulkan\vulkan.hpp>
#include <array>

#include "vk_defines.h"

namespace rlr {

FWRD_DEVICE_PTR

struct TextureArray {
	INTERNAL_DEVICE_PTR

	std::vector<vk::Image> staging_img;
	std::vector<vk::DeviceMemory> staging_img_mem;

	//std::vector<ImgData> m_img_data;

	std::vector<Resource> m_resources;
	vk::DescriptorSet m_desc_set;
};

} /* rlr */
