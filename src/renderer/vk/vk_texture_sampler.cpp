#include "vk_texture_sampler.h"

#include "../interface.h"

namespace rlr {

void Create(TextureSampler& sampler, Device& device, TextureSamplerCreateInfo& info) {
	vk::Device n_device = device.device;

	vk::SamplerCreateInfo sampler_info = {};
	sampler_info.magFilter = vk::Filter::eLinear;
	sampler_info.minFilter = vk::Filter::eLinear;
	sampler_info.addressModeU = info.mode;
	sampler_info.addressModeV = info.mode;
	sampler_info.addressModeW = info.mode;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = 16;
	sampler_info.borderColor = vk::BorderColor::eIntOpaqueBlack;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = vk::CompareOp::eAlways;
	sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	vk::Result r = n_device.createSampler(&sampler_info, nullptr, &sampler.m_native);
	if (r != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create texture sampler.");
	}
}

void Destroy(TextureSampler& sampler, Device& device) {
	vk::Device n_device = device.device;
	n_device.destroySampler(sampler.m_native);
}

} /* rlr */
