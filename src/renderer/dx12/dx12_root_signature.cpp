#include "dx12_root_signature.h"

#include "d3dx12.h"

namespace rlr {

void Create(RootSignature& root_signature, RootSignatureCreateInfo create_info) {
	root_signature.create_info = create_info;
}

void Destroy(RootSignature& root_signature, Device& device) {

}

void Finalize(RootSignature& root_signature, Device& device) {
	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(root_signature.create_info.samplers.size());
	for (auto i = 0; i < root_signature.create_info.samplers.size(); i++) {
		auto sampler_info = root_signature.create_info.samplers[i];

		samplers[0].Filter = (D3D12_FILTER)sampler_info.filter;
		samplers[0].AddressU = (D3D12_TEXTURE_ADDRESS_MODE)sampler_info.address_mode;
		samplers[0].AddressV = (D3D12_TEXTURE_ADDRESS_MODE)sampler_info.address_mode;
		samplers[0].AddressW = (D3D12_TEXTURE_ADDRESS_MODE)sampler_info.address_mode;
		samplers[0].MipLODBias = 0;
		samplers[0].MaxAnisotropy = 0;
		samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplers[0].MinLOD = 0.0f;
		samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[0].ShaderRegister = i;
		samplers[0].RegisterSpace = 0;
		samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
	root_signature_desc.Init(root_signature.create_info.parameters.size(),
		root_signature.create_info.parameters.data(),
		root_signature.create_info.samplers.size(),
		samplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	ID3DBlob* error = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error); //TODO: FIX error parameter
	if (FAILED(hr)) {
		throw "Failed to create a serialized root signature";
	}

	hr = device.native->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature.native));
	if (FAILED(hr)) {
		throw "Failed to create root signature";
	}
	root_signature.native->SetName(L"Native D3D12RootSignature");
}

void Destroy(RootSignature& root_signature) {
	root_signature.native->Release();
}

} /* rlr */
