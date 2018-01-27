#pragma once

#include <vulkan/vulkan.hpp>

namespace rlr {

// TODO: Move these structs to the vk_internal header. They are utility.
struct QueueFamilyIndices {
	int graphics_family = -1;
	int present_family = -1;

	bool IsComplete() {
		return graphics_family >= 0
			&& present_family >= 0;
	}
};

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> present_modes;
};

struct Device {
	// Instance
	vk::Instance instance;
	const std::vector<const char*> validation_layers = { "VK_LAYER_LUNARG_standard_validation" };

	// Devices
	vk::Device device;
	vk::PhysicalDevice physical_device;
	vk::CommandPool cmd_pool;

	QueueFamilyIndices queue_family_indices;
	SwapChainSupportDetails swap_chain_support_details;
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME}; // VK_KHR_MAINTENANCE1_EXTENSION_NAME Allows for negative viewport height.
	VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;
};

} /* fr */