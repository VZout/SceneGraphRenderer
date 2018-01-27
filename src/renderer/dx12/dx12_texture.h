#pragma once

#include <wrl.h>
#include <wincodec.h>
#include <d3d12.h>
#include <cstdint>

namespace rlr {

struct Texture {
	ID3D12Resource* resource;
	ID3D12Resource* staging_resource;
	int bytes_per_row;
	char* buffer;
	BYTE* data;
	int size;
	D3D12_RESOURCE_DESC desc;
};

} /* rlr */
