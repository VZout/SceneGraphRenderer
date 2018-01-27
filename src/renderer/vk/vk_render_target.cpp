#include "vk_render_target.h"

#include <GLFW\glfw3.h>

#include "vk_internal.h"
#include "../interface.h"

namespace rlr {

/* === RENDER TARGET == */

void Create(RenderTarget& render_target, Device& device, std::vector<RenderTargetAttachement> attachements, std::vector<vk::Image> swap_chain_images, Format format) {
	vk::Device vkn_device = device.device;

	/* Render Pass */
	// TODO: Unhardcode this and make attachements optional. Not sure yet how when keeping dx12 in mind.
	vk::AttachmentDescription color_attachment = {};
	color_attachment.format = (vk::Format)format;
	color_attachment.samples = vk::SampleCountFlagBits::e1;
	color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	color_attachment.initialLayout = vk::ImageLayout::eUndefined;
	color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	std::vector<vk::AttachmentDescription> attachments = { color_attachment };
	Resource* depth_resource = nullptr;

	for (auto a : attachements) {
		switch (a.type) {
		case RenderTargetAttachementType::DEPTH_ATTACHMENT: {
			depth_resource = a.resource;
			render_target.m_has_depth_attachment = true;

			vk::AttachmentDescription depth_attachment = {};
			depth_attachment.format = (vk::Format)(a.format);
			depth_attachment.samples = vk::SampleCountFlagBits::e1;
			depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
			depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
			depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			vk::AttachmentReference depth_attachment_ref = {};
			depth_attachment_ref.attachment = 1;
			depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			subpass.pDepthStencilAttachment = &depth_attachment_ref;
			attachments.push_back(depth_attachment);
			break;
		}
		default: throw "Unknown render target attachement.";
			break;
		}
	}

	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.srcAccessMask = vk::AccessFlagBits(0);
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo render_pass_info = {};
	render_pass_info.attachmentCount = attachments.size();
	render_pass_info.pAttachments = attachments.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	vk::Result r = vkn_device.createRenderPass(&render_pass_info, nullptr, &render_target.m_render_pass);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create render pass.");
	}

	/* Frame Buffers */
	render_target.m_framebuffers.resize(render_target.m_image_views.size()); //FIXME: Won't work when not using a render window.

	for (size_t i = 0; i < render_target.m_image_views.size(); i++) {
		std::vector<vk::ImageView> attachments = {
			render_target.m_image_views[i]
		};

		if (depth_resource != nullptr) {
			attachments.push_back(depth_resource->view);
		}

		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.renderPass = render_target.m_render_pass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = render_target.m_width; // FIXME: Hardcoded garbage
		framebufferInfo.height = render_target.m_height; // FIXME: Hardcoded garbage
		framebufferInfo.layers = 1;

		vk::Result r = vkn_device.createFramebuffer(&framebufferInfo, nullptr, &render_target.m_framebuffers[i]);
		if (r != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}
}

void Destroy(RenderTarget& render_target, Device& device) {
	vk::Device n_device = device.device;

	n_device.destroyRenderPass(render_target.m_render_pass);
	for (auto& frame_buffer : render_target.m_framebuffers) {
		n_device.destroyFramebuffer(frame_buffer);
	}
	for (auto& img_view : render_target.m_image_views) {
		n_device.destroyImageView(img_view);
	}

	render_target.m_frame_idx = 0;
}

/* === RENDER WINDOW == */

void PreInitialize(RenderWindow& render_window, Instance& inst, Window& window) {
	VkResult r = glfwCreateWindowSurface(static_cast<VkInstance>(inst.m_native), window.m_native, nullptr, (VkSurfaceKHR*)&render_window.m_surface);
	if (r != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface");
	}
}

void Create(RenderWindow& render_window, Device& device, CommandQueue& queue, int width, int height, std::vector<RenderTargetAttachement> attachements) {
	render_window.m_width = width;
	render_window.m_height = height;

	/* Create Swapchain*/
	vk::Device n_device = device.device;
	SwapChainSupportDetails swap_chain_support = device.swap_chain_support_details;

	QueueFamilyIndices indices = device.queue_family_indices;
	uint32_t queue_family_indices[] = { (uint32_t)indices.graphics_family, (uint32_t)indices.present_family };

	vk::SurfaceFormatKHR surface_format = internal_vk::PickSwapSurfaceFormat(swap_chain_support.formats);
	vk::PresentModeKHR present_mode = internal_vk::PickSwapPresentMode(swap_chain_support.present_modes);
	vk::Extent2D extent(width, height); // FIXME: Hardcoded

	uint32_t num_images = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && num_images > swap_chain_support.capabilities.maxImageCount) {
		num_images = swap_chain_support.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR create_info = {};
	create_info.surface = render_window.m_surface;
	create_info.minImageCount = num_images;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = vk::SharingMode::eConcurrent;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else {
		create_info.imageSharingMode = vk::SharingMode::eExclusive;
		create_info.queueFamilyIndexCount = 0; // Optional
		create_info.pQueueFamilyIndices = nullptr; // Optional
	}
	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = nullptr;

	vk::Result r = n_device.createSwapchainKHR(&create_info, nullptr, &render_window.m_swap_chain);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create swap chain");
	}

	n_device.getSwapchainImagesKHR(render_window.m_swap_chain, &num_images, nullptr);
	render_window.m_swap_chain_images.resize(num_images);
	n_device.getSwapchainImagesKHR(render_window.m_swap_chain, &num_images, render_window.m_swap_chain_images.data());

	render_window.m_swap_chain_image_format = surface_format.format;
	// _swap_chain_extent = extent; // MOVED to pipeline state. Also extend picking seems stupid because we know the size of the back buffers.

	/* Create Swapchain Image Views*/
	render_window.m_image_views.resize(render_window.m_swap_chain_images.size());

	for (uint32_t i = 0; i < render_window.m_swap_chain_images.size(); i++) {
		internal_vk::CreateImageView(device, render_window.m_swap_chain_images[i], render_window.m_swap_chain_image_format, vk::ImageAspectFlagBits::eColor, render_window.m_image_views[i]);
	}

	Create(render_window, device, attachements, render_window.m_swap_chain_images, Format::B8G8R8A8_UNORM);
}

/**
* Presents the current image.
* First aquires the next image and than presents it.
* @param cmd_queue The command queue to present.
* @param render_system The render system.
* @param signal_sema The present semaphore
* @param swap_chain The swapchain to present
* @param frm_idx The current swapchain image index.
*/
void Present(RenderWindow& render_window, CommandQueue& cmd_queue, Device& device, Fence& fence) {
	// Used for signaling the image available semaphore... Kinda shit? Can I drop semaphores entirely and only use fences and events?
	unsigned int image_idx;
	vk::Device n_device = device.device;
	vk::Result r = n_device.acquireNextImageKHR(render_window.m_swap_chain, std::numeric_limits<uint64_t>::max(), nullptr, fence.native, &image_idx);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to acquire next image.");
	}

	if (render_window.m_frame_idx != image_idx) { // FIXME: This shouldn't be possible AT ALL.
		throw "render window's frame index and the acquired image are not equal. This can cause unexpected behaviour.";
	}

	vk::PresentInfoKHR present_info = {};
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores = nullptr;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &render_window.m_swap_chain;
	present_info.pImageIndices = &image_idx;
	present_info.pResults = nullptr; // Optional

	cmd_queue.native.presentKHR(&present_info);

	render_window.m_frame_idx = (render_window.m_frame_idx + 1) % 3; // Increment frame index
}

void SetClearColor(RenderWindow& render_window, std::array<float, 4> clear_color) {
	render_window.m_clear_color = clear_color;
}

void Destroy(RenderWindow& render_window) {
	vk::Device n_device = device.native;
	vk::Instance n_inst = inst.m_native;

	for (auto& img_view : render_window.m_image_views) {
		n_device.destroyImageView(img_view);
	}

	n_device.destroySwapchainKHR(render_window.m_swap_chain);
	n_inst.destroySurfaceKHR(render_window.m_surface);
}

} /* rlr */

