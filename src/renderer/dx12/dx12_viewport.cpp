#include "dx12_viewport.h"

#include "../interface.h"

namespace rlr {

void Create(Viewport& viewport, int width, int height) {
	// Define viewport.
	viewport.viewport.TopLeftX = 0;
	viewport.viewport.TopLeftY = 0;
	viewport.viewport.Width = width;
	viewport.viewport.Height = height;
	viewport.viewport.MinDepth = 0.0f;
	viewport.viewport.MaxDepth = 1.0f;

	// Define scissor rect
	viewport.scissor_rect.left = 0;
	viewport.scissor_rect.top = 0;
	viewport.scissor_rect.right = width;
	viewport.scissor_rect.bottom = height;
}

void Destroy(Viewport& viewport) {
	// ...
}

} /* rlr */
