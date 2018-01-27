#pragma once

#include <stdint.h>
#include "../interface.h"

namespace rlr {

namespace internal_vk {

void CreateBuffer(vk::Buffer& buffer, vk::DeviceMemory memory, Device& device, uint64_t size, unsigned char usage_flags, int properties);

bool HasDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& extensions);
bool IsSuitablePhysicalDevice(const vk::PhysicalDevice& device, RenderWindow& render_window);
SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device, RenderWindow& render_window);
QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device, RenderWindow& render_window);

void TransitionImageLayout(CommandList& cmd_list, vk::Image& image, Format format, ResourceState old_state, ResourceState new_state);

vk::Format FindSupportedFormat(Device& device, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
bool HasStencilComponent(vk::Format format);
	
uint32_t FindMemoryType(Device& device, uint32_t type_filter, vk::MemoryPropertyFlags properties);

void CreateImage(Device& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& img_mem);
void CreateImageView(Device& device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, vk::ImageView& image_view);
void CopyBuffer(vk::CommandBuffer cmd_buffer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
void CopyImage(vk::CommandBuffer cmd_buffer, vk::Image srcImage, vk::Image dstImage, uint32_t width, uint32_t height);

vk::SurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
vk::PresentModeKHR PickSwapPresentMode(const std::vector<vk::PresentModeKHR> present_modes);
vk::Extent2D PickSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);
std::vector<const char*> GetRequiredExtensions();

} /* internal_vk*/

} /* rlr */
