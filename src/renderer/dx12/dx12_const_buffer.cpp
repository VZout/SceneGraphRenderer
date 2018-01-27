#include "dx12_const_buffer.h"

#include "../interface.h"
#include <iostream>
#include "d3dx12.h"

namespace rlr {
	//TODO: Variable amount of buffers
void Create(ConstantBuffer& buffer, Device& device, unsigned int size) {
	buffer.num_buffers = 3; //TODO: Don't hardcode this

	HRESULT hr;
	buffer.size = size;
	unsigned int mul_size = (size + 255) & ~255;

	for (unsigned int i = 0; i < 3; ++i) {
		hr = device.native->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(mul_size), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr,
			IID_PPV_ARGS(&buffer.buffer[i]));
		buffer.buffer[i]->SetName(L"Constant Buffer Upload Resource Heap");
		if (FAILED(hr)) {
			throw "Failed to create constant buffer resource";
		}

		CD3DX12_RANGE readRange(0, 0);
		hr = buffer.buffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&buffer.adress[i]));
		if (FAILED(hr)) {
			throw "Failed to map constant buffer resource";
		}
	}
}
void Update(ConstantBuffer& buffer, unsigned int frame_index, unsigned int size, void* data) {
	unsigned int mul_size = (size + 255) & ~255;

	memcpy(buffer.adress[frame_index], data, mul_size);
}

void Destroy(ConstantBuffer& buffer) {
	for (auto i = 0; i < buffer.num_buffers; i++) {
		CD3DX12_RANGE readRange(0, 0);

		buffer.buffer[i]->Unmap(0, &readRange);
		buffer.buffer[i]->Release();
	}
}

} /* rlr */
