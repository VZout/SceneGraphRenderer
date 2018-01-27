#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>
#include <array>

namespace rlr {

struct RenderTarget {
	RenderTargetCreateInfo create_info;
	unsigned int num_back_buffers = 3;
	unsigned int frame_idx = 0;

	std::vector<ID3D12Resource*> render_targets;
	ID3D12DescriptorHeap* rtv_descriptor_heap;
	unsigned int rtv_descriptor_increment_size;

	ID3D12Resource* depth_stencil_buffer;
	ID3D12DescriptorHeap* depth_stencil_resource_heap;

	std::array<float, 4> clear_color = { 0, 0, 0, 0 };
};

struct RenderWindow : public RenderTarget {
	//const unsigned int m_num_back_buffers = 3;
	//unsigned int m_frame_idx = 0;

	HWND surface;
	IDXGISwapChain4* swap_chain;
	//std::vector<ID3D12Resource*> render_targets;
	//ID3D12DescriptorHeap* rtv_descriptor_heap;
	//unsigned int rtv_descriptor_increment_size;

	//ID3D12Resource* depthStencilBuffer;
	//ID3D12DescriptorHeap* dsDescriptorHeap;

	//std::array<float, 4> m_clear_color = { 1, 0, 0, 1 };
};

} /* rlr */
