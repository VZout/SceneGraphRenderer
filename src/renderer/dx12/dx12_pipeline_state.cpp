#include "dx12_pipeline_state.h"

#include "../interface.h"

#include <stdexcept>
#include <iostream>

#include "../vertex.h" // FIXME: Should'nt be nessessary.
#include "d3dx12.h"

namespace rlr {

void SetVertexShader(PipelineState& pipeline_state, Shader* shader) {
	pipeline_state.vertex_shader = shader;
}

void SetFragmentShader(PipelineState& pipeline_state, Shader* shader) {
	pipeline_state.pixel_shader = shader;
}

void SetRootSignature(PipelineState& pipeline_state, RootSignature* root_signature) {
	pipeline_state.root_signature = root_signature;
}

void Finalize(PipelineState& pipeline_state, Device& device, RenderWindow& render_window, PipelineStateCreateInfo create_info) {
	pipeline_state.topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	D3D12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_DEPTH_STENCIL_DESC depth_stencil_state = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	if (create_info.dsv_format == Format::UNKNOWN) {
		depth_stencil_state.DepthEnable = false;
		depth_stencil_state.StencilEnable = false;
	}

	D3D12_RASTERIZER_DESC rasterize_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterize_desc.FrontCounterClockwise = true;
	depth_stencil_state.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	if (pipeline_state.temp && create_info.dsv_format != Format::UNKNOWN) {
		rasterize_desc.DepthBias = 1.25;
		rasterize_desc.SlopeScaledDepthBias = 1.75f;
		rasterize_desc.CullMode = D3D12_CULL_MODE_FRONT;
		depth_stencil_state.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}
	else {
		rasterize_desc.CullMode = D3D12_CULL_MODE_BACK;
	}
	DXGI_SAMPLE_DESC sample_desc = { 1, 0 };

	pipeline_state.input_layout = Vertex::GetInputLayout();

	D3D12_INPUT_LAYOUT_DESC input_layout_desc = {};
	input_layout_desc.NumElements = pipeline_state.input_layout.size();
	input_layout_desc.pInputElementDescs = pipeline_state.input_layout.data();

	D3D12_SHADER_BYTECODE vs_bytecode = {};
	vs_bytecode.BytecodeLength = pipeline_state.vertex_shader->native->GetBufferSize();
	vs_bytecode.pShaderBytecode = pipeline_state.vertex_shader->native->GetBufferPointer();

	D3D12_SHADER_BYTECODE ps_bytecode = {};
	ps_bytecode.BytecodeLength = pipeline_state.pixel_shader->native->GetBufferSize();
	ps_bytecode.pShaderBytecode = pipeline_state.pixel_shader->native->GetBufferPointer();

	// TODO: num render targets and formats should not be hardcoded ofcourse....
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	for (auto i = 0; i < create_info.num_rtv_formats; i++) { // FIXME: memcpy
		pso_desc.RTVFormats[i] = (DXGI_FORMAT)create_info.rtv_formats[i];
	}
	pso_desc.DSVFormat = (DXGI_FORMAT)create_info.dsv_format;
	pso_desc.SampleDesc = sample_desc;
	pso_desc.SampleMask = 0xffffffff;
	pso_desc.RasterizerState = rasterize_desc;
	pso_desc.BlendState = blend_desc;
	pso_desc.DepthStencilState = depth_stencil_state;
	pso_desc.NumRenderTargets = create_info.num_rtv_formats;
	pso_desc.pRootSignature = pipeline_state.root_signature->native;
	pso_desc.VS = vs_bytecode;
	pso_desc.PS = ps_bytecode;
	pso_desc.InputLayout = input_layout_desc;

	HRESULT hr = device.native->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state.native));
	if (FAILED(hr)) {
		throw "Failed to create graphics pipeline";
	}
	pipeline_state.native->SetName(L"My sick pipeline object");
}

void Destroy(PipelineState& pipeline_state) {
	pipeline_state.native->Release();
}

} /* rlr */
