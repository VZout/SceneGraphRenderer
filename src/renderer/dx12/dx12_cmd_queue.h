#pragma once

#include <d3d12.h>

namespace rlr {

struct CommandQueue {
	ID3D12CommandQueue* native;
};

} /* rlr */
