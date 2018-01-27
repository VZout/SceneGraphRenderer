#pragma once

#include <d3d12.h>
#include "../interface.h"

namespace rlr {

struct RootSignature {
	RootSignatureCreateInfo create_info;
	ID3D12RootSignature* native;
};

} /* rlr */
