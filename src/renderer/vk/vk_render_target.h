#pragma once

#include <vulkan/vulkan.hpp>

namespace rlr {

struct RenderTarget {
	vk::RenderPass m_render_pass;
	std::vector<vk::Framebuffer> m_framebuffers;
	std::vector<vk::ImageView> m_image_views; // TODO: move to wherever swapchain is gonna get created.

	int m_width = 1280;
	int m_height = 720;
	bool m_has_depth_attachment = false;
	std::array<float, 4> m_clear_color = { 0, 0, 0, 1 };

	unsigned int m_frame_idx = 0;
};

struct RenderWindow : public RenderTarget {
	vk::SurfaceKHR m_surface;
	vk::SwapchainKHR m_swap_chain;
	std::vector<vk::Image> m_swap_chain_images; // no need to destroy this. is done by the deleteswapchain
	vk::Format m_swap_chain_image_format;

	const unsigned int m_num_back_buffers = 3;
};

} /* rlr */

