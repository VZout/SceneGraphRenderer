#pragma once

#include "enums.h"

#include <vector>
#include <any>

#include "dx12\d3dx12.h"

namespace rlr {

class Window;

#define RLR_HANDLE struct

RLR_HANDLE Device;
RLR_HANDLE CommandQueue;
RLR_HANDLE RenderTarget;
RLR_HANDLE RenderWindow;
RLR_HANDLE CommandList;
RLR_HANDLE Fence;
RLR_HANDLE PipelineState;
RLR_HANDLE Shader;
RLR_HANDLE RootSignature;
RLR_HANDLE Viewport;
RLR_HANDLE StagingBuffer;
RLR_HANDLE ConstantBuffer;
RLR_HANDLE Texture;
RLR_HANDLE DescriptorHeap;
RLR_HANDLE DescHeapCPUHandle;
RLR_HANDLE DescHeapGPUHandle;
RLR_HANDLE TextureArray;
RLR_HANDLE Resource;

// Device
void Create(Device** device);
void Destroy(Device* device);

// CommandQueue
void Create(CommandQueue** cmd_queue, Device* device, CmdQueueType type);
void Execute(CommandQueue* cmd_queue, std::vector<CommandList> cmd_lists, Fence* fence);
void Destroy(CommandQueue* cmd_queue);

// Descriptor Heap
struct DescriptorHeapCreateInfo {
	unsigned int num_descriptors;
	DescriptorHeapType type;
	bool shader_visible = true;
};

void Create(DescriptorHeap** hep, Device* device, DescriptorHeapCreateInfo const & create_info);
[[nodiscard]] DescHeapGPUHandle GetGPUHandle(DescriptorHeap* heap, int index = 0);
[[nodiscard]] DescHeapCPUHandle GetCPUHandle(DescriptorHeap* heap, int index = 0);
void Offset(DescHeapGPUHandle& handle, int index, int increment_size);
void Offset(DescHeapCPUHandle& handle, int index, int increment_size);
void Destroy(DescriptorHeap* cmd_queue);

// RenderTarget
struct RenderTargetCreateInfo {
	bool create_dsv_buffer = true;
	Format dsv_format;
	Format rtv_formats[8];
	unsigned int num_rtv_formats;
};

void Create(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height, RenderTargetCreateInfo create_info);
void CreateRenderTargetViews(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height);
void CreateDepthStencilBuffer(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height);
void CreateSRVFromDSV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle);
void CreateSRVFromRTV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle, int num, Format formats[8]);
void CreateSRVFromSpecificRTV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle, int id, Format format);
void Resize(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height);
void IncrementFrameIdx(RenderTarget& render_target);
void DestroyDepthStencilBuffer(RenderTarget& render_target);
void DestroyRenderTargetViews(RenderTarget& render_target);
void Destroy(RenderTarget& render_target);

// RenderWindow
void Create(RenderWindow& render_window, Device& device, Window& window, CommandQueue& cmd_queue);
void Resize(RenderWindow& render_window, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height);
void Present(RenderWindow& render_window, CommandQueue& cmd_queue, Device& device);
void Destroy(RenderWindow& render_window);

// CommandList
void Allocate(CommandList** cmd_list, Device* device, unsigned int num = 1);
void SetName(CommandList* cmd_list, std::string const& name);
void SetName(CommandList* cmd_list, std::wstring const& name);
void Begin(CommandList& cmd_list, unsigned int frame_idx);
void End(CommandList& cmd_list);
void Bind(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_idx, bool clear = true, bool deferred = false, bool temp_compo = false);
void SBind(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_idx);
void Bind(CommandList& cmd_list, Viewport& viewport);
void Bind(CommandList& cmd_list, PipelineState* pipeline_state);
void SetPrimitiveTopology(CommandList& cmd_list, D3D12_PRIMITIVE_TOPOLOGY topology);
void Bind(CommandList* cmd_list, ConstantBuffer* buffer, unsigned int root_parameter_idx, unsigned int frame_idx);
void Bind(CommandList& cmd_list, TextureArray& ta, unsigned int root_param_index);
void Bind(CommandList& cmd_list, DescHeapGPUHandle& handle, unsigned int root_param_index);
void Bind(CommandList& cmd_list, std::vector<DescriptorHeap*> heaps);
void BindVertexBuffer(CommandList* cmd_list, StagingBuffer* buffer);
void BindIndexBuffer(CommandList* cmd_list, StagingBuffer* buffer, unsigned int offset = 0);
void Draw(CommandList* cmd_list, unsigned int vertex_count, unsigned int inst_count);
void DrawIndexed(CommandList* cmd_list, unsigned int idx_count, unsigned int inst_count);
void Transition(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_index, ResourceState from, ResourceState to);
void Transition(CommandList& cmd_list, RenderTarget& render_target, ResourceState from, ResourceState to);
void Destroy(CommandList& cmd_list);

// Fence
void Create(Fence** fence, Device* device);
void Signal(Fence* fence, CommandQueue* cmd_queue);
void WaitFor(Fence* fence);
void Destroy(Fence* fence);

// Pipeline
struct PipelineStateCreateInfo {
	Format dsv_format;
	Format rtv_formats[8];
	unsigned int num_rtv_formats;
};

void Create(PipelineState** pipeline_state);
void SetVertexShader(PipelineState* pipeline_state, Shader* shader);
void SetFragmentShader(PipelineState* pipeline_state, Shader* shader);
void SetRootSignature(PipelineState* pipeline_state, RootSignature* root_signature);
void Finalize(PipelineState* pipeline_state, Device* device, RenderWindow* render_window, PipelineStateCreateInfo create_info); //FIXME: RENDER WINDOW IS NOT NEEDED
void Destroy(PipelineState* pipeline_state);

// RootSignature
struct SamplerInfo {
	TextureFilter filter;
	TextureAddressMode address_mode;
};

struct RootSignatureCreateInfo {
	std::vector<CD3DX12_ROOT_PARAMETER> parameters;
	std::vector<SamplerInfo> samplers;
};

void Create(RootSignature** root_signature, RootSignatureCreateInfo create_info);
void Destroy(RootSignature* root_signature);
void Finalize(RootSignature* root_signature, Device* device);
void Destroy(RootSignature* root_signature);

// Shader
void Load(Shader** shader, ShaderType type, std::string const & path);
void Destroy(Shader* shader);

// Texture
void Load(Texture** texture, std::string const & path);
void Stage(Texture* texture, Device* device, CommandList* cmd_list, bool dds = false);
void Destroy(Texture* texture);

// TextureArray
void Create(TextureArray** ta, Device* device, std::vector<Texture*> textures);
void Destroy(TextureArray* ta);

// Viewport
void Create(Viewport& viewport, int width, int height);
void Destroy(Viewport& viewport);

// StagingBuffer
void Create(StagingBuffer** buffer, Device* device, void* data, uint64_t size, uint64_t stride, ResourceState resource_state);
void Update(StagingBuffer* buffer, Device* device, unsigned int size, void* data);
void StageBuffer(StagingBuffer* buffer, CommandList* cmd_list);
void FreeStagingBuffer(StagingBuffer* buffer);
void Destroy(StagingBuffer* buffer);

// ConstantBuffer
void Create(ConstantBuffer** buffer, Device* device, unsigned int size);
void Update(ConstantBuffer* buffer, unsigned int frame_index, unsigned int size, void* data);
void Destroy(ConstantBuffer* buffer);

} /* rlr */

#ifdef USE_D3D12
#include "dx12\dx12_device.h"
#include "dx12\dx12_cmd_queue.h"
#include "dx12\dx12_render_target.h"
#include "dx12\dx12_command_list.h"
#include "dx12\dx12_fence.h"
#include "dx12\dx12_viewport.h"
#include "dx12\dx12_shader.h"
#include "dx12\dx12_texture.h"
#include "dx12\dx12_pipeline_state.h"
#include "dx12\dx12_root_signature.h"
#include "dx12\dx12_staging_buffer.h"
#include "dx12\dx12_const_buffer.h"
#include "dx12\dx12_texture_array.h"
#include "dx12\dx12_descriptor_heap.h"
#endif