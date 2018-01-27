#pragma once

#include <d3d12.h>

namespace rlr {

struct Fence {
	ID3D12Fence* native;
	HANDLE fence_event;
	UINT64 fence_value = 0;
};

} /* rlr */
