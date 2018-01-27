#include "vk_texture_array.h"

#include "../interface.h"
#include "vk_internal.h"

namespace rlr {

void Create(TextureArray& ta, Device& device, std::vector<Texture*> textures) {
	SET_INTERNAL_DEVICE_PTR(ta, device)

	vk::Device n_device = device.device;

	// Create Descriptor Heap
	vk::DescriptorSetLayout layouts[] = { root_signature.m_desc_set_layout_img };
	vk::DescriptorSetAllocateInfo alloc_info = {};
	alloc_info.descriptorPool = root_signature.m_desc_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = layouts;

	vk::Result r = n_device.allocateDescriptorSets(&alloc_info, &ta.m_desc_set);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create descriptor set.");
	}

	// TODO: This is definatly not the place. this is root signature stuff.
	std::array<vk::DescriptorImageInfo, 3> image_info = {};
	std::array<vk::WriteDescriptorSet, 3> descriptorWrites = {};
	for (auto i = 0; i < descriptorWrites.size(); i++) {

		image_info[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		image_info[i].imageView = ta.m_resources[i].view;
		image_info[i].sampler = sampler.m_native;

		descriptorWrites[i].dstSet = ta.m_desc_set;
		descriptorWrites[i].dstBinding = 0;
		descriptorWrites[i].dstArrayElement = i;
		descriptorWrites[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pImageInfo = &image_info[i];
	}

	n_device.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

	// TODO: 
	// Create Image views here instead of in vk_texture.h/cpp.
	// We create a image view's only here for dx12. So should vulkan.
}

void Destroy(TextureArray& texture_array) {
	/*
	for (auto i = 0; i < texture_array.m_resources.size(); i++) {
		Destroy(texture_array.m_resources[i], device);
		if (texture_array.staging_img[i]) n_device.destroyImage(texture_array.staging_img[i]);
		if (texture_array.staging_img_mem[i]) n_device.freeMemory(texture_array.staging_img_mem[i]);
	}

	vk::DescriptorSet sets[1] = { texture_array.m_desc_set };
	n_device.freeDescriptorSets(root_signature.m_desc_pool, 1, sets);*/
}

void Transition(CommandList& cmd_list, TextureArray& texture_array, ResourceState old_state, ResourceState new_state) {
	for (unsigned int i = 0; i < texture_array.m_resources.size(); i++) {
		TransitionImageLayout(cmd_list, texture_array.m_resources[i], Format::B8G8R8A8_UNORM, old_state, new_state);																																												 // Not setting it to read only optimal seems to impact performance massivly though... FIXME!!!!!!!
	}
}

} /* rlr */
