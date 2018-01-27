#pragma once

#include <d3d12.h>

namespace rlr {

struct ConstantBuffer {
	unsigned int num_buffers;
	ID3D12Resource* buffer[3]; // FIXME: Don't hardcode the array size.
	UINT8* adress[3];

	unsigned int size;
	bool first_update = true;
};

} /* rlr */
