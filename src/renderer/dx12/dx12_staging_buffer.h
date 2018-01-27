#pragma once

#include <d3d12.h>

#include "../enums.h"

namespace rlr {

struct StagingBuffer {
	ID3D12Resource* buffer;
	ID3D12Resource* staging;
	unsigned int size;
	unsigned int stride_in_bytes;
	void* data;
	ResourceState target_resource_state;
};

} /* rlr */
