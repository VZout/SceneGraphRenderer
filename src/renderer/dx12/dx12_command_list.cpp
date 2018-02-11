#include "dx12_command_list.h"

#include "../interface.h"
#include "d3dx12.h"

#include <vector>
#include <iostream>
#include <d3d12.h>
#include <locale>
#include <codecvt>
#include <string>

namespace rlr {

void Allocate(CommandList** cmd_list, Device* device, unsigned int num) {
	auto* new_cmd_list = new CommandList();

	new_cmd_list->num_allocators = num;
	new_cmd_list->allocators = new ID3D12CommandAllocator*[num];

	// Create the allocators
	for (int i = 0; i < num; i++) {
		HRESULT hr = device->native->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&new_cmd_list->allocators[i]));
		if (FAILED(hr)) {
			throw "Failed to create command allocator";
		}

		new_cmd_list->allocators[i]->SetName(L"CommandList allocator.");
	}

	// Create the command lists
	HRESULT hr = device->native->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		new_cmd_list->allocators[0],
		NULL,
		IID_PPV_ARGS(&new_cmd_list->native)
	);
	if (FAILED(hr)) {
		throw "Failed to create command list";
	}

	new_cmd_list->native->SetName(L"Native Commandlist");
	new_cmd_list->native->Close(); // TODO: Can be optimized away.

	(*cmd_list) = new_cmd_list;
}

void SetName(CommandList* cmd_list, std::string const& name)
{
#ifdef _DEBUG
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wname = converter.from_bytes(name);

	cmd_list->native->SetName(wname.c_str());
#endif
}

void SetName(CommandList* cmd_list, std::wstring const& name)
{
#ifdef _DEBUG
	cmd_list->native->SetName(name.c_str());
#endif
}

void Begin(CommandList& cmd_list, unsigned int frame_idx) {
	// TODO: move resetting to when the command list is executed. This is how vulkan does it.
	HRESULT hr = cmd_list.allocators[frame_idx]->Reset();
	if (FAILED(hr)) {
		throw "Failed to reset cmd allocators";
	}

	// Only reset with pipeline state if using bundles since only then this will impact fps.
	// Otherwise its just easier to pass NULL and suffer the insignificant performance loss.
	hr = cmd_list.native->Reset(cmd_list.allocators[frame_idx], NULL);

	if (FAILED(hr)) {
		throw "Failed to reset command list";
	}
}

void End(CommandList& cmd_list) {
	cmd_list.native->Close();
}

void Bind(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_idx, bool clear, bool deferred, bool temp_compo) {
	if (temp_compo) {
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle0(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 0, render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle1(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 1, render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle;
		if (render_target.create_info.create_dsv_buffer) dsv_handle = render_target.depth_stencil_resource_heap->GetCPUDescriptorHandleForHeapStart();

		CD3DX12_CPU_DESCRIPTOR_HANDLE handles[]{ rtv_handle0, rtv_handle1 };
		cmd_list.native->OMSetRenderTargets(2, handles, false, render_target.create_info.create_dsv_buffer ? &dsv_handle : nullptr);
		if (clear) {
			cmd_list.native->ClearRenderTargetView(rtv_handle0, render_target.clear_color.data(), 0, nullptr);
			cmd_list.native->ClearRenderTargetView(rtv_handle1, render_target.clear_color.data(), 0, nullptr);
			if (render_target.create_info.create_dsv_buffer)  cmd_list.native->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}
	else if (deferred) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle0(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 0, render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle1(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 1, render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle2(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 2, render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle;
		if (render_target.create_info.create_dsv_buffer) dsv_handle = render_target.depth_stencil_resource_heap->GetCPUDescriptorHandleForHeapStart();

		CD3DX12_CPU_DESCRIPTOR_HANDLE handles[]{ rtv_handle0, rtv_handle1, rtv_handle2 };
		cmd_list.native->OMSetRenderTargets(3, handles, false, render_target.create_info.create_dsv_buffer ? &dsv_handle : nullptr);
		if (clear) {
			cmd_list.native->ClearRenderTargetView(rtv_handle0, render_target.clear_color.data(), 0, nullptr);
			cmd_list.native->ClearRenderTargetView(rtv_handle1, render_target.clear_color.data(), 0, nullptr);
			cmd_list.native->ClearRenderTargetView(rtv_handle2, render_target.clear_color.data(), 0, nullptr);
			if (render_target.create_info.create_dsv_buffer) cmd_list.native->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}
	else { // normal
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_idx % render_target.render_targets.size(), render_target.rtv_descriptor_increment_size);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle;
		if (render_target.create_info.create_dsv_buffer) dsv_handle = render_target.depth_stencil_resource_heap->GetCPUDescriptorHandleForHeapStart();

		cmd_list.native->OMSetRenderTargets(1, &rtv_handle, false, render_target.create_info.create_dsv_buffer ? &dsv_handle : nullptr);
		if (clear) {
			cmd_list.native->ClearRenderTargetView(rtv_handle, render_target.clear_color.data(), 0, nullptr);
			if (render_target.create_info.create_dsv_buffer) cmd_list.native->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
	}
}


void SBind(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_idx) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle;
	if (render_target.create_info.create_dsv_buffer) dsv_handle = render_target.depth_stencil_resource_heap->GetCPUDescriptorHandleForHeapStart();

	cmd_list.native->OMSetRenderTargets(0, nullptr, false, render_target.create_info.create_dsv_buffer ? &dsv_handle : nullptr);
	cmd_list.native->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Bind(CommandList& cmd_list, Viewport& viewport) {
	cmd_list.native->RSSetViewports(1, &viewport.viewport); // set the viewports
	cmd_list.native->RSSetScissorRects(1, &viewport.scissor_rect); // set the scissor rects
}

void Bind(CommandList& cmd_list, std::vector<DescriptorHeap*> heaps) {
	std::array<ID3D12DescriptorHeap*, 4> native_heaps;
	for (auto i = 0; i < heaps.size(); i++) {
		native_heaps[i] = heaps[i]->native;
	}

	cmd_list.native->SetDescriptorHeaps(heaps.size(), native_heaps.data());
}

void Bind(CommandList& cmd_list, PipelineState* pipeline_state) {
	cmd_list.native->SetPipelineState(pipeline_state->native);
	cmd_list.native->SetGraphicsRootSignature(pipeline_state->root_signature->native);
}

void SetPrimitiveTopology(CommandList& cmd_list, D3D12_PRIMITIVE_TOPOLOGY topology) {
	cmd_list.native->IASetPrimitiveTopology(topology);
}
	
void Bind(CommandList* cmd_list, ConstantBuffer* buffer, unsigned int root_parameter_idx, unsigned int frame_idx) {
	cmd_list->native->SetGraphicsRootConstantBufferView(root_parameter_idx, buffer->buffer[frame_idx]->GetGPUVirtualAddress());
}

void Bind(CommandList& cmd_list, TextureArray& ta, unsigned int root_param_index) {
	cmd_list.native->SetGraphicsRootDescriptorTable(root_param_index, ta.texture_heap->GetGPUDescriptorHandleForHeapStart());
}

void Bind(CommandList& cmd_list, DescHeapGPUHandle& handle, unsigned int root_param_index) {
	cmd_list.native->SetGraphicsRootDescriptorTable(root_param_index, handle.native);
}

void BindVertexBuffer(CommandList* cmd_list, StagingBuffer* buffer) {
	D3D12_VERTEX_BUFFER_VIEW view;
	view.BufferLocation = buffer->buffer->GetGPUVirtualAddress();
	view.StrideInBytes = buffer->stride_in_bytes;
	view.SizeInBytes = buffer->size;

	cmd_list->native->IASetVertexBuffers(0, 1, &view);
}

void BindIndexBuffer(CommandList* cmd_list, StagingBuffer* buffer, unsigned int offset) {
	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = buffer->buffer->GetGPUVirtualAddress();
	view.Format = DXGI_FORMAT_R32_UINT;
	view.SizeInBytes = buffer->size;

	cmd_list->native->IASetIndexBuffer(&view);
}

void Draw(CommandList* cmd_list, unsigned int vertex_count, unsigned int inst_count) {
	cmd_list->native->DrawInstanced(vertex_count, inst_count, 0, 0);
}

void DrawIndexed(CommandList* cmd_list, unsigned int idx_count, unsigned int inst_count) {
	cmd_list->native->DrawIndexedInstanced(idx_count, inst_count, 0, 0, 0);
}

void Transition(CommandList& cmd_list, RenderTarget& render_target, unsigned int frame_index, ResourceState from, ResourceState to) {
	CD3DX12_RESOURCE_BARRIER end_transition = CD3DX12_RESOURCE_BARRIER::Transition(
		render_target.render_targets[frame_index % render_target.render_targets.size()], // TODO: Mod here should be unnessessary and should throw instead because performance.
		(D3D12_RESOURCE_STATES)from,
		(D3D12_RESOURCE_STATES)to
	);

	cmd_list.native->ResourceBarrier(1, &end_transition);
}

void Transition(CommandList& cmd_list, RenderTarget& render_target, ResourceState from, ResourceState to) {
	std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
	barriers.resize(render_target.num_back_buffers);
	for (auto i = 0; i < render_target.num_back_buffers; i++) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			render_target.render_targets[i],
			(D3D12_RESOURCE_STATES)from,
			(D3D12_RESOURCE_STATES)to
		);

		barriers[i] = barrier;
	}
	cmd_list.native->ResourceBarrier(barriers.size(), barriers.data());
}

void Destroy(CommandList& cmd_list) {
	cmd_list.native->Release();
	for (auto i = 0; i < cmd_list.num_allocators; i++) {
		cmd_list.allocators[i]->Release();
	}
}

} /* rlr */
