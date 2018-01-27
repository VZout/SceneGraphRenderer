#include "dx12_texture_array.h"

#include "../interface.h"
#include "d3dx12.h"

namespace rlr {

void Create(TextureArray& ta, Device& device, std::vector<Texture*> textures) {
	ta.textures = textures;

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
	heap_desc.NumDescriptors = ta.textures.size();
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heap_desc.NodeMask = 0;
	HRESULT hr = device.native->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&ta.texture_heap));
	if (FAILED(hr)) {
		throw "Failed to create texture heap";
	}

	int offset_size = device.native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(ta.texture_heap->GetCPUDescriptorHandleForHeapStart());
	for (unsigned int i = 0; i < ta.textures.size(); i++) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = ta.textures[i]->desc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device.native->CreateShaderResourceView(ta.textures[i]->resource, &srvDesc, rtv_handle);

		rtv_handle.Offset(1, offset_size);
	}
}

void Destroy(TextureArray& ta) {
	for (auto i = 0; i < ta.textures.size(); i++) {
		Destroy(*ta.textures[i]);
	}
	ta.texture_heap->Release();
}

} /* rlr */
