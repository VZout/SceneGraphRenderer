#include "vk_viewport.h"

#include "../interface.h"

namespace rlr {

void Create(Viewport& viewport, int width, int height) {
	viewport.m_viewport.x = 0.0f;
	viewport.m_viewport.y = 0 + height;
	viewport.m_viewport.width = (float)width;
	viewport.m_viewport.height = -height;
	viewport.m_viewport.minDepth = 0.0f;
	viewport.m_viewport.maxDepth = 1.0f;

	viewport.m_scissor.offset = vk::Offset2D{ 0, 0 };
	viewport.m_scissor.extent = vk::Extent2D(width, height);
}

} /* rlr */
