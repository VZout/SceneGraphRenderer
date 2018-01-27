#pragma once

#include <d3d12.h>

namespace rlr {

struct DescriptorHeap {
	DescriptorHeapCreateInfo create_info;
	ID3D12DescriptorHeap* native;
	unsigned int increment_size;
};

struct DescHeapGPUHandle {
	D3D12_GPU_DESCRIPTOR_HANDLE native;
};

struct DescHeapCPUHandle {
	D3D12_CPU_DESCRIPTOR_HANDLE native;
};

} /* rlr */
