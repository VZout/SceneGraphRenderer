#pragma once

#ifdef USE_D3D12
#include <d3d12.h>
#elif USE_VULKAN
#include <vulkan\vulkan.hpp>
#endif

namespace rlr {

enum CmdQueueType {
	CMD_QUEUE_DIRECT,
	CMD_QUEUE_COMPUTE,
	CMD_QUEUE_COPY,
};

enum class ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	DOMAIN_SHADER,
	GEOMETRY_SHADER,
	HULL_SHADER,
	DIRECT_COMPUTE_SHADER,
};

#ifdef USE_D3D12
enum class TextureFilter {
	FILTER_LINEAR = (int)D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	FILTER_POINT = (int)D3D12_FILTER_MIN_MAG_MIP_POINT,
};

enum class TextureAddressMode {
	TAM_MIRROR_ONCE = (int)D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE,
	TAM_MIRROR = (int)D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
	TAM_CLAMP = (int)D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	TAM_BORDER = (int)D3D12_TEXTURE_ADDRESS_MODE_BORDER,
	TAM_WRAP = (int)D3D12_TEXTURE_ADDRESS_MODE_WRAP,
};

enum class DescriptorHeapType {
	DESC_HEAP_TYPE_CBV_SRV_UAV = (int)D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	DESC_HEAP_TYPE_SAMPLER = (int)D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	DESC_HEAP_TYPE_RTV = (int)D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	DESC_HEAP_TYPE_DSV = (int)D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
	DESC_HEAP_TYPE_NUM_TYPES = (int)D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES,
};

enum class ResourceState {
	VERTEX_AND_CONSTANT_BUFFER = (int)D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	INDEX_BUFFER = (int)D3D12_RESOURCE_STATE_INDEX_BUFFER,
	PRESENT = (int)D3D12_RESOURCE_STATE_PRESENT,
	RENDER_TARGET = (int)D3D12_RESOURCE_STATE_RENDER_TARGET,
};

enum BufferUsageFlag {
	INDEX_BUFFER = (int)D3D12_RESOURCE_STATE_INDEX_BUFFER,
	VERTEX_BUFFER = (int)D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
};

enum Format {
	UNKNOWN = (int)DXGI_FORMAT_UNKNOWN,
	R32G32B32A32_FLOAT = (int)DXGI_FORMAT_R32G32B32A32_FLOAT,
	R32G32B32A32_UINT = (int)DXGI_FORMAT_R32G32B32A32_UINT,
	R32G32B32A32_SINT = (int)DXGI_FORMAT_R32G32B32A32_SINT,
	R32G32B32_FLOAT = (int)DXGI_FORMAT_R32G32B32_FLOAT,
	R32G32B32_UINT = (int)DXGI_FORMAT_R32G32B32_UINT,
	R32G32B32_SINT = (int)DXGI_FORMAT_R32G32B32_SINT,
	R16G16B16A16_FLOAT = (int)DXGI_FORMAT_R16G16B16A16_FLOAT,
	R16G16B16A16_UINT = (int)DXGI_FORMAT_R16G16B16A16_UINT,
	R16G16B16A16_SINT = (int)DXGI_FORMAT_R16G16B16A16_SINT,
	R16G16B16A16_UNORM = (int)DXGI_FORMAT_R16G16B16A16_UNORM,
	R16G16B16A16_SNORM = (int)DXGI_FORMAT_R16G16B16A16_SNORM,
	R32G32_FLOAT = (int)DXGI_FORMAT_R32G32_FLOAT,
	R32G32_UINT = (int)DXGI_FORMAT_R32G32_UINT,
	R32G32_SINT = (int)DXGI_FORMAT_R32G32_SINT,
	//R10G10B10_UNORM = (int)DXGI_FORMAT_R10G10B10_UNORM,
	//R10G10B10_UINT = (int)vk::Format::eA2R10G10B10UintPack32, //FIXME: Their are more vulcan variants?
	R8G8B8A8_UNORM = (int)DXGI_FORMAT_R8G8B8A8_UNORM,
	R8G8B8A8_UNORM_SRGB = (int)DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	R8G8B8A8_SNORM = (int)DXGI_FORMAT_R8G8B8A8_SNORM,
	R8G8B8A8_UINT = (int)DXGI_FORMAT_R8G8B8A8_UINT,
	R8G8B8A8_SINT = (int)DXGI_FORMAT_R8G8B8A8_SINT,
	//R16G16_FLOAT = (int)vk::Format::eR16G16Sfloat,
	//R16G16_UNORM = (int)vk::Format::eR16G16Unorm,
	//R16G16_UINT = (int)vk::Format::eR16G16Uint,
	//R16G16_SNORM = (int)vk::Format::eR16G16Snorm,
	//R16G16_SINT = (int)vk::Format::eR16G16Sint,
	D32_FLOAT = (int)DXGI_FORMAT_D32_FLOAT,
	R32_UINT = (int)DXGI_FORMAT_R32_UINT,
	R32_SINT = (int)DXGI_FORMAT_R32_SINT,
	R32_FLOAT = (int)DXGI_FORMAT_R32_FLOAT,
	D24_UNFORM_S8_UINT = (int)DXGI_FORMAT_D24_UNORM_S8_UINT,
	//R8G8_UNORM = (int)vk::Format::eR8G8Unorm,
	//R8G8_UINT = (int)vk::Format::eR8G8Uint,
	//R8G8_SNORM = (int)vk::Format::eR8G8Snorm,
	//R8G8_SINT = (int)vk::Format::eR8G8Sint,
	//R16_FLOAT = (int)vk::Format::eR16Sfloat,
	//D16_UNORM = (int)vk::Format::eD16Unorm,
	//R16_UNORM = (int)vk::Format::eR16Unorm,
	//R16_UINT = (int)vk::Format::eR16Uint,
	//R16_SNORM = (int)vk::Format::eR16Snorm,
	//R16_SINT = (int)vk::Format::eR16Sint,
	R8_UNORM = (int)DXGI_FORMAT_R8_UNORM,
	/*R8_UINT = (int)vk::Format::eR8Uint,
	R8_SNORM = (int)vk::Format::eR8Snorm,
	R8_SINT = (int)vk::Format::eR8Sint,
	BC1_UNORM = (int)vk::Format::eBc1RgbUnormBlock, //FIXME: is this correct?
	BC1_UNORM_SRGB = (int)vk::Format::eBc1RgbSrgbBlock, //FIXME: is this correct?
	BC2_UNORM = (int)vk::Format::eBc2UnormBlock,
	BC2_UNORM_SRGB = (int)vk::Format::eBc2SrgbBlock,
	BC3_UNORM = (int)vk::Format::eBc3UnormBlock,
	BC3_UNORM_SRGB = (int)vk::Format::eBc3SrgbBlock,
	BC4_UNORM = (int)vk::Format::eBc4UnormBlock,
	BC4_SNORM = (int)vk::Format::eBc4SnormBlock,
	BC5_UNORM = (int)vk::Format::eBc5UnormBlock,
	BC5_SNORM = (int)vk::Format::eBc5SnormBlock,
	B5G6R5_UNORM = (int)vk::Format::eB5G6R5UnormPack16,
	B5G5R5A1_UNORM = (int)vk::Format::eB5G5R5A1UnormPack16,
	B8G8R8A8_UNORM = (int)vk::Format::eB8G8R8A8Unorm,
	B8G8R8A8_UNORM_SRGB = (int)vk::Format::eB8G8R8A8Srgb,
	B8G8R8A8_SNORM = (int)vk::Format::eB8G8R8A8Snorm,
	B8G8R8A8_UINT = (int)vk::Format::eB8G8R8A8Uint,
	B8G8R8A8_SINT = (int)vk::Format::eB8G8R8A8Sint,
	BC6H_UF16 = (int)vk::Format::eBc6HUfloatBlock,
	BC6H_SF16 = (int)vk::Format::eBc6HSfloatBlock,
	BC7_UNORM = (int)vk::Format::eBc7UnormBlock,
	BC7_UNORM_SRGB = (int)vk::Format::eBc7SrgbBlock,
	B4G4R4A4_UNORM = (int)vk::Format::eB4G4R4A4UnormPack16,*/
	D32_FLOAT_S8X24_UINT = (int)DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
};
#endif
#ifdef USE_VULKAN
enum class DescriptorHeapType {
	// TODO
};

enum class ImageTiling {
	LINEAR = (int)vk::ImageTiling::eLinear,
	OPTIMAL = (int)vk::ImageTiling::eOptimal,
};

enum class MemoryProperties {
	DEVICE_LOCAL = (int)vk::MemoryPropertyFlagBits::eDeviceLocal,
	HOST_CACHED = (int)vk::MemoryPropertyFlagBits::eHostCached,
	HOST_COHERENT = (int)vk::MemoryPropertyFlagBits::eHostCoherent,
	HOST_VISIBILE = (int)vk::MemoryPropertyFlagBits::eHostVisible,
	LAZILY_ALLOCATED = (int)vk::MemoryPropertyFlagBits::eLazilyAllocated,
};

enum class ImageAspect {
	COLOR = (int)vk::ImageAspectFlagBits::eColor,
	DEPTH = (int)vk::ImageAspectFlagBits::eDepth,
	METADATA = (int)vk::ImageAspectFlagBits::eMetadata,
	STENCIL = (int)vk::ImageAspectFlagBits::eStencil,
};

//TODO: This needs some checking. I've probably made some mistakes
enum class ResourceState {
	COMMON = (int)vk::ImageLayout::eGeneral,
	VERTEX_AND_CONSTANT_BUFFER = (int)vk::ImageLayout::eShaderReadOnlyOptimal,
	INDEX_BUFFER = (int)vk::ImageLayout::eShaderReadOnlyOptimal,
	RENDER_TARGET = (int)vk::ImageLayout::eGeneral,
	UNORDERED_ACCESS = (int)vk::ImageLayout::eShaderReadOnlyOptimal,
	DEPTH_WRITE = (int)vk::ImageLayout::eDepthStencilAttachmentOptimal,
	DEPTH_READ = (int)vk::ImageLayout::eDepthStencilReadOnlyOptimal,
	NON_PIXEL_SHADER_RESOURCE = (int)vk::ImageLayout::eGeneral,
	PIXEL_SHADER_RESOURCE = (int)vk::ImageLayout::eShaderReadOnlyOptimal,
	STREAM_OUT = (int)vk::ImageLayout::eGeneral,
	INDIRECT_ARGUMENT = (int)vk::ImageLayout::eGeneral,
	COPY_DEST = (int)vk::ImageLayout::eTransferDstOptimal,
	COPY_SOURCE = (int)vk::ImageLayout::eTransferSrcOptimal,
	RESOLVE_DEST = (int)vk::ImageLayout::eTransferDstOptimal,
	RESOLVE_SOURCE = (int)vk::ImageLayout::eTransferSrcOptimal,
	GENERAL_READ = (int)vk::ImageLayout::ePreinitialized, // GENERI_READ is a macro :(
	PRESENT = (int)vk::ImageLayout::ePresentSrcKHR,
	PREDICATION = (int)vk::ImageLayout::eUndefined,
	UNKNOWN = (int)vk::ImageLayout::eUndefined,
};

enum BufferUsageFlag {
	INDEX_BUFFER = (int)vk::BufferUsageFlagBits::eIndexBuffer,
	INDIRECT_BUFFER = (int)vk::BufferUsageFlagBits::eIndirectBuffer,
	STORAGE_BUFFER = (int)vk::BufferUsageFlagBits::eStorageBuffer,
	STORAGE_TEXEL_BUFFER = (int)vk::BufferUsageFlagBits::eStorageTexelBuffer,
	COPY_DEST = (int)vk::BufferUsageFlagBits::eTransferDst,
	COPY_SOURCE = (int)vk::BufferUsageFlagBits::eTransferSrc,
	UNIFORM_BUFFER = (int)vk::BufferUsageFlagBits::eUniformBuffer,
	UNIFORM_TEXEL_BUFFER = (int)vk::BufferUsageFlagBits::eUniformTexelBuffer,
	VERTEX_BUFFER = (int)vk::BufferUsageFlagBits::eVertexBuffer,
};

enum Format {
	UNKNOWN = (int)vk::Format::eUndefined,
	R32G32B32A32_FLOAT = (int)vk::Format::eR32G32B32A32Sfloat,
	R32G32B32A32_UINT = (int)vk::Format::eR32G32B32A32Uint,
	R32G32B32A32_SINT = (int)vk::Format::eR32G32B32A32Sint,
	R32G32B32_FLOAT = (int)vk::Format::eR32G32B32A32Sfloat,
	R32G32B32_UINT = (int)vk::Format::eR32G32B32A32Uint,
	R32G32B32_SINT = (int)vk::Format::eR32G32B32A32Sint,
	R16G16B16A16_FLOAT = (int)vk::Format::eR16G16B16A16Sfloat,
	R16G16B16A16_UINT = (int)vk::Format::eR16G16B16A16Uint,
	R16G16B16A16_SINT = (int)vk::Format::eR16G16B16A16Sint,
	R16G16B16A16_UNORM = (int)vk::Format::eR16G16B16A16Unorm,
	R16G16B16A16_SNORM = (int)vk::Format::eR16G16B16A16Snorm,
	R32G32_FLOAT = (int)vk::Format::eR32G32Sfloat,
	R32G32_UINT = (int)vk::Format::eR32G32Uint,
	R32G32_SINT = (int)vk::Format::eR32G32Sint,
	R10G10B10_UNORM = (int)vk::Format::eA2R10G10B10UnormPack32,
	R10G10B10_UINT = (int)vk::Format::eA2R10G10B10UintPack32, //FIXME: Their are more vulcan variants?
	R8G8B8A8_UNORM = (int)vk::Format::eR8G8B8A8Unorm,
	R8G8B8A8_UNORM_SRGB = (int)vk::Format::eR8G8B8A8Srgb,
	R8G8B8A8_SNORM = (int)vk::Format::eR8G8B8A8Snorm,
	R8G8B8A8_UINT = (int)vk::Format::eR8G8B8A8Uint,
	R8G8B8A8_SINT = (int)vk::Format::eR8G8B8A8Sint,
	R16G16_FLOAT = (int)vk::Format::eR16G16Sfloat,
	R16G16_UNORM = (int)vk::Format::eR16G16Unorm,
	R16G16_UINT = (int)vk::Format::eR16G16Uint,
	R16G16_SNORM = (int)vk::Format::eR16G16Snorm,
	R16G16_SINT = (int)vk::Format::eR16G16Sint,
	D32_FLOAT = (int)vk::Format::eD32Sfloat,
	R32_UINT = (int)vk::Format::eR32Uint,
	R32_SINT = (int)vk::Format::eR32Sint,
	R32_FLOAT = (int)vk::Format::eR32Sfloat,
	D24_UNFORM_S8_UINT = (int)vk::Format::eD24UnormS8Uint,
	R8G8_UNORM = (int)vk::Format::eR8G8Unorm,
	R8G8_UINT = (int)vk::Format::eR8G8Uint,
	R8G8_SNORM = (int)vk::Format::eR8G8Snorm,
	R8G8_SINT = (int)vk::Format::eR8G8Sint,
	R16_FLOAT = (int)vk::Format::eR16Sfloat,
	D16_UNORM = (int)vk::Format::eD16Unorm,
	R16_UNORM = (int)vk::Format::eR16Unorm,
	R16_UINT = (int)vk::Format::eR16Uint,
	R16_SNORM = (int)vk::Format::eR16Snorm,
	R16_SINT = (int)vk::Format::eR16Sint,
	R8_UNORM = (int)vk::Format::eR8Unorm,
	R8_UINT = (int)vk::Format::eR8Uint,
	R8_SNORM = (int)vk::Format::eR8Snorm,
	R8_SINT = (int)vk::Format::eR8Sint,
	BC1_UNORM = (int)vk::Format::eBc1RgbUnormBlock, //FIXME: is this correct?
	BC1_UNORM_SRGB = (int)vk::Format::eBc1RgbSrgbBlock, //FIXME: is this correct?
	BC2_UNORM = (int)vk::Format::eBc2UnormBlock,
	BC2_UNORM_SRGB = (int)vk::Format::eBc2SrgbBlock,
	BC3_UNORM = (int)vk::Format::eBc3UnormBlock,
	BC3_UNORM_SRGB = (int)vk::Format::eBc3SrgbBlock,
	BC4_UNORM = (int)vk::Format::eBc4UnormBlock,
	BC4_SNORM = (int)vk::Format::eBc4SnormBlock,
	BC5_UNORM = (int)vk::Format::eBc5UnormBlock,
	BC5_SNORM = (int)vk::Format::eBc5SnormBlock,
	B5G6R5_UNORM = (int)vk::Format::eB5G6R5UnormPack16,
	B5G5R5A1_UNORM = (int)vk::Format::eB5G5R5A1UnormPack16,
	B8G8R8A8_UNORM = (int)vk::Format::eB8G8R8A8Unorm,
	B8G8R8A8_UNORM_SRGB = (int)vk::Format::eB8G8R8A8Srgb,
	B8G8R8A8_SNORM = (int)vk::Format::eB8G8R8A8Snorm,
	B8G8R8A8_UINT = (int)vk::Format::eB8G8R8A8Uint,
	B8G8R8A8_SINT = (int)vk::Format::eB8G8R8A8Sint,
	BC6H_UF16 = (int)vk::Format::eBc6HUfloatBlock,
	BC6H_SF16 = (int)vk::Format::eBc6HSfloatBlock,
	BC7_UNORM = (int)vk::Format::eBc7UnormBlock,
	BC7_UNORM_SRGB = (int)vk::Format::eBc7SrgbBlock,
	B4G4R4A4_UNORM = (int)vk::Format::eB4G4R4A4UnormPack16,
	D32_FLOAT_S8X24_UINT = (int)vk::Format::eD32SfloatS8Uint,
};

enum ImageUsage {
	COLOR = (int)vk::ImageUsageFlagBits::eColorAttachment,
	DEPTH = (int)vk::ImageUsageFlagBits::eDepthStencilAttachment,
	INPUT = (int)vk::ImageUsageFlagBits::eInputAttachment,
	SAMPLED = (int)vk::ImageUsageFlagBits::eSampled,
	STORAGE = (int)vk::ImageUsageFlagBits::eStorage,
	TRANSFER_DST = (int)vk::ImageUsageFlagBits::eTransferDst,
	TRANSFER_SRC = (int)vk::ImageUsageFlagBits::eTransferSrc,
	TRANSIENT = (int)vk::ImageUsageFlagBits::eTransientAttachment,
};
#endif

} /* rlr */
