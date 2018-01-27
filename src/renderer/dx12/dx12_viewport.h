#pragma once

#include <d3d12.h>

namespace rlr {

struct Viewport {
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor_rect;
};

} /* rlr */
