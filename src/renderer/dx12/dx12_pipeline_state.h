#pragma once

#include <d3d12.h>
#include <memory>
#include <vector>

namespace rlr {

struct RootSignature;
struct Shader;

struct PipelineState {
	ID3D12PipelineState* native;
	RootSignature* root_signature;
	Shader* vertex_shader;
	Shader* pixel_shader;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type;
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;
	bool temp = false;
};

} /* rlr */
