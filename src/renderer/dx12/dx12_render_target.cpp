#include "../interface.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "dx12_render_target.h"
#include "d3dx12.h"
#include "../window.h"

namespace rlr {

DXGI_FORMAT depth_format = DXGI_FORMAT_R32_TYPELESS;

void CreateSRVFromDSV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle) {
	unsigned int increment_size = device.native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;

	device.native->CreateShaderResourceView(render_target.depth_stencil_buffer, &srv_desc, handle.native);
	Offset(handle, 1, increment_size);
}

void CreateSRVFromRTV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle, int num, Format formats[8]) {
	unsigned int increment_size = device.native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (unsigned int i = 0; i < render_target.num_back_buffers; i++) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = (DXGI_FORMAT)formats[i];
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;

		device.native->CreateShaderResourceView(render_target.render_targets[i], &srv_desc, handle.native);
		Offset(handle, 1, increment_size);
	}
}

void CreateSRVFromSpecificRTV(RenderTarget& render_target, Device& device, DescHeapCPUHandle& handle, int id, Format format) {
	unsigned int increment_size = device.native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = (DXGI_FORMAT)format;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;

	device.native->CreateShaderResourceView(render_target.render_targets[id], &srv_desc, handle.native);
	Offset(handle, 1, increment_size);
}

void Create(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height, RenderTargetCreateInfo create_info) {
	render_target.render_targets.resize(create_info.num_rtv_formats);
	render_target.create_info = create_info;
	render_target.num_back_buffers = create_info.num_rtv_formats;

	for (auto i = 0; i < create_info.num_rtv_formats; i++) {
		CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D((DXGI_FORMAT)create_info.rtv_formats[i], width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		D3D12_CLEAR_VALUE optimized_clear_value = {
			(DXGI_FORMAT)create_info.rtv_formats[i],
			render_target.clear_color[0],
			render_target.clear_color[1],
			render_target.clear_color[2],
			render_target.clear_color[3]
		};

		// Create default heap
		HRESULT hr = device.native->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_PRESENT,
			&optimized_clear_value, // optimizes draw call
			IID_PPV_ARGS(&render_target.render_targets[i]));
		if (FAILED(hr)) {
			throw "Failed to create render target.";
		}

		render_target.render_targets[i]->SetName(L"Render Target view Resource Heap");

		UINT64 textureUploadBufferSize;
		device.native->GetCopyableFootprints(&resource_desc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);
	}

	CreateRenderTargetViews(render_target, device, cmd_queue, width, height);

	if (create_info.create_dsv_buffer) {
		CreateDepthStencilBuffer(render_target, device, cmd_queue, width, height);
	}
}

void IncrementFrameIdx(RenderTarget& render_target) {
	render_target.frame_idx++;
	render_target.frame_idx = render_target.frame_idx % render_target.num_back_buffers;
}

void Create(RenderWindow& render_window, Device& device, Window& window, CommandQueue& cmd_queue) {
	// Get surface
	render_window.surface = glfwGetWin32Window(window.native);

	// Create swapchain
	DXGI_SAMPLE_DESC sample_desc = {};
	sample_desc.Count = 1;
	sample_desc.Quality = 0;

	int width, height;
	window.GetSize(&width, &height);

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = width;
	swap_chain_desc.Height = height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.SampleDesc = sample_desc;
	swap_chain_desc.BufferCount = render_window.num_back_buffers; // FIXME: hardcoded
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain1* temp_swap_chain;
	HRESULT hr = device.dxgi_factory->CreateSwapChainForHwnd(
		cmd_queue.native,
		render_window.surface,
		&swap_chain_desc,
		NULL,
		NULL,
		&temp_swap_chain
	);
	if (FAILED(hr)) {
		throw "Failed to create swap chain.";
	}

	hr = device.dxgi_factory->MakeWindowAssociation(render_window.surface, 0);
	if (FAILED(hr)) {
		throw "Failed to make window association";
	}

	render_window.swap_chain = static_cast<IDXGISwapChain4*>(temp_swap_chain);
	render_window.frame_idx = (render_window.swap_chain)->GetCurrentBackBufferIndex();

	render_window.render_targets.resize(render_window.num_back_buffers);
	for (auto i = 0; i < render_window.num_back_buffers; i++) {
		HRESULT hr = render_window.swap_chain->GetBuffer(i, IID_PPV_ARGS(&render_window.render_targets[i]));
		if (FAILED(hr)) {
			throw "Failed to get swap chain buffer.";
		}
	}

	CreateRenderTargetViews(render_window, device, cmd_queue, width, height);
	CreateDepthStencilBuffer(render_window, device, cmd_queue, width, height);
}

void CreateDepthStencilBuffer(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height) {
	// TODO: Seperate the descriptor heap because that one might not need to be recreated when resizing.
	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device.native->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&render_target.depth_stencil_resource_heap));
	if (FAILED(hr)) {
		throw "Failed to create descriptor heap for depth buffer";
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	hr = device.native->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(depth_format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&render_target.depth_stencil_buffer)
	);
	render_target.depth_stencil_resource_heap->SetName(L"Depth/Stencil Resource Heap");
	if (FAILED(hr)) {
		throw "Failed to create commited resource.";
	}

	device.native->CreateDepthStencilView(render_target.depth_stencil_buffer, &depthStencilDesc, render_target.depth_stencil_resource_heap->GetCPUDescriptorHandleForHeapStart());
}

void CreateRenderTargetViews(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height) {
	// Create views
	D3D12_DESCRIPTOR_HEAP_DESC back_buffer_heap_desc = {};
	back_buffer_heap_desc.NumDescriptors = render_target.num_back_buffers;
	back_buffer_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	back_buffer_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device.native->CreateDescriptorHeap(&back_buffer_heap_desc, IID_PPV_ARGS(&render_target.rtv_descriptor_heap));
	if (FAILED(hr)) {
		throw "Failed to create descriptor heap.";
	}

	render_target.rtv_descriptor_increment_size = device.native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create render target view with the handle to the heap descriptor.
	render_target.render_targets.resize(render_target.num_back_buffers);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(render_target.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
	for (unsigned int i = 0; i < render_target.num_back_buffers; i++) {
		device.native->CreateRenderTargetView(render_target.render_targets[i], nullptr, rtv_handle);

		rtv_handle.Offset(1, render_target.rtv_descriptor_increment_size);
	}
}

void Resize(RenderTarget& render_target, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height) {
	if (render_target.create_info.dsv_format == Format::UNKNOWN && render_target.create_info.create_dsv_buffer) DestroyDepthStencilBuffer(render_target);
	DestroyRenderTargetViews(render_target);

	Create(render_target, device, cmd_queue, width, height, render_target.create_info);
}

void DestroyDepthStencilBuffer(RenderTarget& render_target) {
	render_target.depth_stencil_buffer->Release();
	render_target.depth_stencil_resource_heap->Release();
}

void DestroyRenderTargetViews(RenderTarget& render_target) {
	render_target.rtv_descriptor_heap->Release();
	render_target.frame_idx = 0;
	for (auto i = 0; i < render_target.render_targets.size(); i++) {
		render_target.render_targets[i]->Release();
	}
}

void Resize(RenderWindow& render_window, Device& device, CommandQueue& cmd_queue, unsigned int width, unsigned int height) {
	DestroyDepthStencilBuffer(render_window);
	DestroyRenderTargetViews(render_window);

	render_window.swap_chain->ResizeBuffers(render_window.num_back_buffers, width, height, DXGI_FORMAT_UNKNOWN, 0);

	render_window.render_targets.resize(render_window.num_back_buffers);
	for (auto i = 0; i < render_window.num_back_buffers; i++) {
		HRESULT hr = render_window.swap_chain->GetBuffer(i, IID_PPV_ARGS(&render_window.render_targets[i]));
		if (FAILED(hr)) {
			throw "Failed to get swap chain buffer.";
		}
	}

	CreateRenderTargetViews(render_window, device, cmd_queue, width, height);
	CreateDepthStencilBuffer(render_window, device, cmd_queue, width, height);
}

void Present(RenderWindow& render_window, CommandQueue& cmd_queue, Device& device) {
	render_window.swap_chain->Present(0, 0);
	render_window.frame_idx = render_window.swap_chain->GetCurrentBackBufferIndex();
}

void Destroy(RenderTarget& render_target) {
	if (render_target.create_info.create_dsv_buffer) {
		DestroyDepthStencilBuffer(render_target);
	}
	DestroyRenderTargetViews(render_target);
}

void Destroy(RenderWindow& render_window) {
	DestroyDepthStencilBuffer(render_window);
	render_window.rtv_descriptor_heap->Release();
	render_window.frame_idx = 0;

	render_window.swap_chain->Release();
}

} /* rlr */
