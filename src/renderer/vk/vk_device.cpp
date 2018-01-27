#include "vk_device.h"

#include "../interface.h"
#include "vk_internal.h"

#include <set>
#include <iostream>

namespace rlr {

bool HasValidationLayerSupport(Device& device) {
	uint32_t num_layers;
	vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

	std::vector<VkLayerProperties> layers(num_layers);
	vkEnumerateInstanceLayerProperties(&num_layers, layers.data());

	for (const char* layer_name : device.validation_layers) {
		bool found_layer = false;

		for (const auto& layerProperties : layers) {
			if (strcmp(layer_name, layerProperties.layerName) == 0) {
				found_layer = true;
				break;
			}
		}

		if (!found_layer) {
			return false;
		}
	}

	return true;
}

#ifdef ENABLE_VK_VALIDATION_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "validation layer: " << msg << "\n\n";

	return VK_FALSE;
};

void InitValidationLayers(Device& device) {
	VkDebugReportCallbackCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	create_info.pfnCallback = DebugCallback;

	// Dynamically load debugging function.
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr((VkInstance)device.instance, "vkCreateDebugReportCallbackEXT"); // FIXME: instance CAST

	VkResult r = CreateDebugReportCallback((VkInstance)device.instance, &create_info, nullptr, &device.debug_callback); // FIXME: instance CAST
	if (r != VK_SUCCESS) {
		throw std::runtime_error("Failed to create debug report callback");
	}
}
#endif

void Query(Device& device, RenderWindow& render_window) {
	device.swap_chain_support_details = internal_vk::QuerySwapChainSupport(device.physical_device, render_window);

	device.queue_family_indices = internal_vk::FindQueueFamilies(device.physical_device, render_window);
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	std::set<int> unique_queue_families = { device.queue_family_indices.graphics_family, device.queue_family_indices.present_family };
	float queue_priority = 1.0f;

	for (int queue_family : unique_queue_families) {
		vk::DeviceQueueCreateInfo info = {};
		info.queueFamilyIndex = queue_family;
		info.queueCount = 1;
		info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(info);
	}
}


void FindPhysicalDevice(Device& device, RenderWindow& render_window) {
	vk::PhysicalDevice physical_device;
	uint32_t num_devices = 0;

	// only to get the amount of devices.
	device.instance.enumeratePhysicalDevices(&num_devices, nullptr);

	if (num_devices == 0) {
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	std::vector<vk::PhysicalDevice> devices(num_devices);
	static_cast<vk::Instance>(device.instance).enumeratePhysicalDevices(&num_devices, devices.data());

	for (const auto& d : devices) {
		if (internal_vk::HasDeviceExtensionSupport(d, device.device_extensions) && internal_vk::IsSuitablePhysicalDevice(d, render_window)) {
			physical_device = d;
			break;
		}
	}

	if (!physical_device) {
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	device.physical_device = physical_device;
}

void Create(Device& device, RenderWindow& render_window) {
	// Vulkan Instance
	{
		vk::ApplicationInfo app_info = {};
		app_info.pApplicationName = "Vulkan application";
		app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		app_info.pEngineName = "Falcon";
		app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		app_info.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> extensions = internal_vk::GetRequiredExtensions();

		vk::InstanceCreateInfo create_info = {};
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = extensions.size();
		create_info.ppEnabledExtensionNames = extensions.data();

#ifdef ENABLE_VK_VALIDATION_LAYERS
		create_info.enabledLayerCount = device.validation_layers.size();
		create_info.ppEnabledLayerNames = device.validation_layers.data();
#else
		create_info.enabledLayerCount = 0;
#endif

		vk::Result r = vk::createInstance(&create_info, nullptr, &device.instance);
		if (r != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create vulkan instance");
		}
	}

	// Vulkan Device Related Stuff
#ifdef ENABLE_VK_VALIDATION_LAYERS
	if (!HasValidationLayerSupport(device))
		throw std::runtime_error("Validation layers requested, but not available.");

	InitValidationLayers(device);
#endif

	FindPhysicalDevice(device, render_window);
	device.swap_chain_support_details = internal_vk::QuerySwapChainSupport(device.physical_device, render_window);

	device.queue_family_indices = internal_vk::FindQueueFamilies(device.physical_device, render_window);
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
	std::set<int> unique_queue_families = { device.queue_family_indices.graphics_family, device.queue_family_indices.present_family };
	float queue_priority = 1.0f;

	for (int queue_family : unique_queue_families) {
		vk::DeviceQueueCreateInfo info = {};
		info.queueFamilyIndex = queue_family;
		info.queueCount = 1;
		info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(info);
	}

	vk::PhysicalDeviceFeatures device_features = {};

	vk::DeviceCreateInfo create_info = {};
	device_features.wideLines = true; //TODO: This can cause problems on some hardware (mobile?) or impact performance.
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.queueCreateInfoCount = queue_create_infos.size();
	create_info.pEnabledFeatures = &device_features;
	create_info.enabledExtensionCount = device.device_extensions.size();
	create_info.ppEnabledExtensionNames = device.device_extensions.data();
#ifdef ENABLE_VK_VALIDATION_LAYERS
	create_info.enabledLayerCount = device.validation_layers.size();
	create_info.ppEnabledLayerNames = device.validation_layers.data();
#else
	create_info.enabledLayerCount = 0;
#endif

	vk::Result r = device.physical_device.createDevice(&create_info, nullptr, &device.device);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create logical device");
	}

	/* Create The Command Pool */
	vk::Device n_device = device.device;

	vk::CommandPoolCreateInfo pool_info = {};
	pool_info.queueFamilyIndex = device.queue_family_indices.graphics_family;
	pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // Optional

	r = n_device.createCommandPool(&pool_info, nullptr, &device.cmd_pool);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create command pool.");
	}
}

#ifdef ENABLE_VK_VALIDATION_LAYERS
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}
#endif

// TODO: Maybe move debug report to instance?
void Destroy(Device& device) {
	vk::Device n_device = device.device;
	vk::Instance n_inst = device.instance;

#ifdef ENABLE_VK_VALIDATION_LAYERS
	DestroyDebugReportCallbackEXT((VkInstance)n_inst, device.debug_callback, nullptr);
#endif

	n_device.destroyCommandPool(device.cmd_pool);
	n_device.destroy();
	n_inst.destroy();
}

} /* rlr */
