#include "vk_shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "../interface.h"

namespace rlr {

void Load(Shader& shader, Device& device, ShaderType type, std::string const& path) {
	SET_INTERNAL_DEVICE_PTR(shader, device);

	vk::Device n_device = device.device;

	// load
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	// compile
	vk::ShaderModuleCreateInfo create_info = {};
	create_info.codeSize = fileSize;
	create_info.pCode = (uint32_t*)buffer.data();

	vk::Result r = n_device.createShaderModule(&create_info, nullptr, &shader.module);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create shader module");
	}
}

void Destroy(Shader& shader) {
	GET_NATIVE_INTERNAL_DEVICE(shader).destroyShaderModule(shader.module);
}

} /* rlr */
