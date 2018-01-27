#pragma once

#include <d3d12.h>

namespace rlr {

struct CommandList {
	unsigned int num_allocators = 3;
	ID3D12CommandAllocator** allocators;
	ID3D12GraphicsCommandList* native;
};

} /* rlr */
