#include "vk_internal.h"

#include <vulkan\vulkan.hpp>
#include <set>

namespace rlr {

namespace internal_vk {

void CreateBuffer(vk::Buffer& buffer, vk::DeviceMemory memory, Device& device, uint64_t size, unsigned char usage_flags, int properties) {
	vk::Device n_device = device.device;

	vk::BufferCreateInfo buffer_info = {};
	buffer_info.size = size;
	buffer_info.usage = vk::BufferUsageFlagBits(usage_flags);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	if (n_device.createBuffer(&buffer_info, nullptr, &buffer) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements mem_requirements;
	n_device.getBufferMemoryRequirements(buffer, &mem_requirements);

	vk::MemoryAllocateInfo alloc_info = {};
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = internal_vk::FindMemoryType(device, mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits(properties));

	if (n_device.allocateMemory(&alloc_info, nullptr, &memory) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	n_device.bindBufferMemory(buffer, memory, 0);
}

bool HasDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& extensions) {
	uint32_t extensionCount;
	device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
	device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> required_extensions(extensions.begin(), extensions.end());

	for (const auto& extension : availableExtensions) {
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

bool IsSuitablePhysicalDevice(const vk::PhysicalDevice& device, RenderWindow& render_window) {
	vk::PhysicalDeviceProperties device_properties;
	vk::PhysicalDeviceFeatures device_features;
	QueueFamilyIndices queue_family_indices;


	device.getProperties(&device_properties);
	device.getFeatures(&device_features);
	queue_family_indices = internal_vk::FindQueueFamilies(device, render_window);

	bool swap_chain_is_suitable = false;
	SwapChainSupportDetails swapChainSupport = internal_vk::QuerySwapChainSupport(device, render_window);
	swap_chain_is_suitable = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();

	return queue_family_indices.IsComplete() && swap_chain_is_suitable;
}

SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device, RenderWindow& render_window) {
	vk::SurfaceKHR surface = render_window.m_surface;
	SwapChainSupportDetails details;

	device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

	uint32_t num_formats;
	device.getSurfaceFormatsKHR(surface, &num_formats, nullptr);

	if (num_formats != 0) {
		details.formats.resize(num_formats);
		device.getSurfaceFormatsKHR(surface, &num_formats, details.formats.data());
	}

	uint32_t num_present_modes;
	device.getSurfacePresentModesKHR(surface, &num_present_modes, nullptr);

	if (num_present_modes != 0) {
		details.present_modes.resize(num_present_modes);
		device.getSurfacePresentModesKHR(surface, &num_present_modes, details.present_modes.data());
	}

	return details;
}

QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device, RenderWindow& render_window) {
	QueueFamilyIndices indices;
	VkBool32 present_support = false;

	uint32_t queueFamilyCount = 0;
	device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

	std::vector<vk::QueueFamilyProperties> queue_families(queueFamilyCount);
	device.getQueueFamilyProperties(&queueFamilyCount, queue_families.data());

	for (size_t i = 0; i < queue_families.size(); i++) {
		device.getSurfaceSupportKHR(i, render_window.m_surface, &present_support);

		if (queue_families[i].queueCount > 0 && present_support) {
			indices.present_family = i;
		}

		if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphics_family = i;

		if (indices.IsComplete())
			break;
	}

	return indices;
}

void TransitionImageLayout(CommandList& cmd_list, vk::Image& image, Format format, ResourceState old_state, ResourceState new_state) {
	vk::ImageMemoryBarrier barrier = {};
	barrier.oldLayout = (vk::ImageLayout)old_state;
	barrier.newLayout = (vk::ImageLayout)new_state;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

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

vk::Format FindSupportedFormat(Device& device, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
	vk::PhysicalDevice n_phys_device = device.physical_device;

	for (vk::Format format : candidates) {
		vk::FormatProperties props = n_phys_device.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format.");
}

bool HasStencilComponent(vk::Format format) {
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

uint32_t FindMemoryType(Device& device, uint32_t type_filter, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDevice n_phys_device = device.physical_device;

	vk::PhysicalDeviceMemoryProperties mem_properties;
	n_phys_device.getMemoryProperties(&mem_properties);

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void CreateImage(Device& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& img_mem) {
	vk::Device n_device = device.device;
	
	vk::ImageCreateInfo image_info = {};
	image_info.imageType = vk::ImageType::e2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = vk::ImageLayout::ePreinitialized;
	image_info.usage = usage;
	image_info.samples = vk::SampleCountFlagBits::e1;
	image_info.sharingMode = vk::SharingMode::eExclusive;

	vk::Result r = n_device.createImage(&image_info, nullptr, &image);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create image.");
	}

	vk::MemoryRequirements mem_requirements;
	n_device.getImageMemoryRequirements(image, &mem_requirements);

	vk::MemoryAllocateInfo alloc_info = {};
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = FindMemoryType(device, mem_requirements.memoryTypeBits, properties);

	r = n_device.allocateMemory(&alloc_info, nullptr, &img_mem);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to allocate image memory.");
	}

	n_device.bindImageMemory(image, img_mem, 0);
		
}

void CreateImageView(Device& device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, vk::ImageView& image_view) {
	vk::Device n_device = device.device;

	vk::ImageViewCreateInfo view_info = {};
	view_info.image = image;
	view_info.viewType = vk::ImageViewType::e2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	vk::Result r = n_device.createImageView(&view_info, nullptr, &image_view);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create texture image view.");
	}
}

void CopyBuffer(vk::CommandBuffer cmd_buffer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
	vk::BufferCopy copyRegion = {};
	copyRegion.size = size;
	cmd_buffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
}

void CopyImage(vk::CommandBuffer cmd_buffer, vk::Image srcImage, vk::Image dstImage, uint32_t width, uint32_t height) {
	vk::ImageSubresourceLayers subResource = {};
	subResource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	vk::ImageCopy region = {};
	region.srcSubresource = subResource;
	region.dstSubresource = subResource;
	region.srcOffset = vk::Offset3D{ 0, 0, 0 };
	region.dstOffset = vk::Offset3D{ 0, 0, 0 };
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	cmd_buffer.copyImage(srcImage, vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);
}

vk::SurfaceFormatKHR PickSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
		return{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	for (const auto& format : formats) {
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}

	return formats[0];
}

vk::PresentModeKHR PickSwapPresentMode(const std::vector<vk::PresentModeKHR> present_modes) {
	for (const auto& present_mode : present_modes) {
		if (present_mode == vk::PresentModeKHR::eMailbox) {
			return present_mode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D PickSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities, uint32_t width, uint32_t height) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

// TODO: Move to window class.
std::vector<const char*> GetRequiredExtensions() {
	std::vector<const char*> extensions;

	unsigned int num_glfw_extensions = 0;
	const char** glfw_extensions;
	glfw_extensions = GetRequiredInstanceExtensions(&num_glfw_extensions);

	for (unsigned int i = 0; i < num_glfw_extensions; i++) {
		extensions.push_back(glfw_extensions[i]);
	}

#ifdef ENABLE_VK_VALIDATION_LAYERS
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	return extensions;
}

} /* internal_vk */

} /* rlr */
