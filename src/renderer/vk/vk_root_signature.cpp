#include "vk_root_signature.h"

#include "../interface.h"
#include "vk_device.h"

namespace rlr {

	void Create(RootSignature& root_signature, RootSignatureCreateInfo create_info) {
		root_signature.m_num_constant_buffers = create_info.num_const_buffers;
	}

	void Destroy(RootSignature& root_signature, Device& device) {
		vk::Device n_device = device.m_native;

		n_device.destroyDescriptorSetLayout(root_signature.m_desc_set_layout_img);
		n_device.destroyDescriptorSetLayout(root_signature.m_desc_set_layout);
		n_device.destroyDescriptorPool(root_signature.m_desc_pool);
	}

	void Finalize(RootSignature& root_signature, Device& device) {
		vk::Device n_device = device.m_native;

		// Create the layout
		{
			vk::DescriptorSetLayoutBinding ubo_layout_binding = {};
			ubo_layout_binding.binding = 0;
			ubo_layout_binding.descriptorCount = root_signature.m_num_constant_buffers;
			ubo_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			ubo_layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;
			ubo_layout_binding.pImmutableSamplers = nullptr; // Used for sampling.

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { ubo_layout_binding };

			vk::DescriptorSetLayoutCreateInfo desc_set_create_info = {};
			desc_set_create_info.bindingCount = bindings.size();
			desc_set_create_info.pBindings = bindings.data();

			vk::Result r = n_device.createDescriptorSetLayout(&desc_set_create_info, nullptr, &root_signature.m_desc_set_layout);
			if (r != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create descriptor set layout.");
			}
		}

		// Second layout
		{
			vk::DescriptorSetLayoutBinding sampler_layout_binding = {};
			sampler_layout_binding.binding = 0;
			sampler_layout_binding.descriptorCount = 3;
			sampler_layout_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			sampler_layout_binding.pImmutableSamplers = nullptr;
			sampler_layout_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

			std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { sampler_layout_binding };

			vk::DescriptorSetLayoutCreateInfo desc_set_create_info = {};
			desc_set_create_info.bindingCount = bindings.size();
			desc_set_create_info.pBindings = bindings.data();

			vk::Result r = n_device.createDescriptorSetLayout(&desc_set_create_info, nullptr, &root_signature.m_desc_set_layout_img);
			if (r != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create descriptor set layout.");
			}
		}

		// Create the pool
		std::vector<vk::DescriptorPoolSize> pool_sizes = {
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
		};

		vk::DescriptorPoolCreateInfo desc_pool_create_info = {};
		desc_pool_create_info.poolSizeCount = pool_sizes.size();
		desc_pool_create_info.pPoolSizes = pool_sizes.data();
		desc_pool_create_info.maxSets = 99;
		desc_pool_create_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		vk::Result r = n_device.createDescriptorPool(&desc_pool_create_info, nullptr, &root_signature.m_desc_pool);
		if (r != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create descriptor pool.");
		}
	}

} /* rlr */
