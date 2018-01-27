#pragma once

#include <d3d12.h>
#include <vector>

namespace rlr {

struct Texture;

struct TextureArray {
	std::vector<Texture*> textures;
	ID3D12DescriptorHeap* texture_heap;
};

} /* rlr */
