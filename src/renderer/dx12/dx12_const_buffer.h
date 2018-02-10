#pragma once

#include <d3d12.h>
#include <array>

namespace rlr {

struct ConstantBuffer {
	unsigned int num_buffers;
	std::array<ID3D12Resource*, 3> buffer; // FIXME: Don't hardcode the array size.
	std::array<UINT8*, 3> adress;

	unsigned int size;
	bool first_update = true;
};

} /* rlr */
