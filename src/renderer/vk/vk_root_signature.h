#pragma once

#include <vulkan/vulkan.hpp>

namespace rlr {

struct RootSignature {
	unsigned int m_num_constant_buffers;
	vk::DescriptorSetLayout m_desc_set_layout_img;
	vk::DescriptorSetLayout m_desc_set_layout;
	vk::DescriptorPool m_desc_pool;
};

} /* rlr */