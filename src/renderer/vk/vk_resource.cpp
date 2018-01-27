#include "vk_resource.h"

#include "vk_internal.h"

namespace rlr {

void Create(Resource& resource, Device& device, ResourceCreateInfo const& create_info) {
	SET_INTERNAL_DEVICE_PTR(resource, device);

	vk::Format format = vk::Format(create_info.format);

	internal_vk::CreateImage(device, create_info.width, create_info.height,
		format,
		(vk::ImageTiling)create_info.tiling,
		(vk::ImageUsageFlagBits)create_info.usage,
		(vk::MemoryPropertyFlagBits)create_info.mem_properties,
		resource.image,
		resource.memory);

	internal_vk::CreateImageView(device,
		resource.image,
		format,
		(vk::ImageAspectFlagBits)create_info.aspect,
		resource.view);
}

void Destroy(Resource& resource) {
	vk::Device n_device = GET_NATIVE_INTERNAL_DEVICE(resource);

	n_device.destroyImage(resource.image);
	n_device.destroyImageView(resource.view);
	n_device.freeMemory(resource.memory);
}

void TransitionImageLayout(CommandList& cmd_list, Resource& resource, Format format, ResourceState old_state, ResourceState new_state) {
	vk::ImageMemoryBarrier barrier = {};
	barrier.oldLayout = (vk::ImageLayout)old_state;
	barrier.newLayout = (vk::ImageLayout)new_state;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = resource.image;

	//TODO: This probably isn't nessessary. DX12 allows you to set accessmasks as well. So lets hand that to the user.
	vk::ImageLayout n_old_layout = (vk::ImageLayout)old_state;
	vk::ImageLayout n_new_layout = (vk::ImageLayout)new_state;

	if (n_new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

		if (internal_vk::HasStencilComponent((vk::Format)format)) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (n_old_layout == vk::ImageLayout::ePreinitialized && n_new_layout == vk::ImageLayout::eTransferSrcOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	}
	else if (n_old_layout == vk::ImageLayout::ePreinitialized && n_new_layout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	}
	else if (n_old_layout == vk::ImageLayout::eTransferDstOptimal && n_new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	}
	else if (n_old_layout == vk::ImageLayout::eUndefined && n_new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits(0);
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	n_cmd_list.pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits(0),
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

} /* rlr */