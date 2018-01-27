#include "dx12_staging_buffer.h"

#include "../interface.h"

#include "d3dx12.h"
#include "../vertex.h"

namespace rlr {

void Create(StagingBuffer& buffer, Device& device, void* data, uint64_t size, uint64_t stride, ResourceState resource_state) {
	buffer.target_resource_state = resource_state;
	buffer.size = size;

	buffer.stride_in_bytes = stride;

	device.native->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&buffer.buffer));
	buffer.buffer->SetName(L"Buffer Resource Heap");

	device.native->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer.staging));
	buffer.staging->SetName(L"Buffer Upload Resource Heap");
	
	buffer.data = data;
}

void Update(StagingBuffer& buffer, Device& device, unsigned int size, void* data) {

}

void StageBuffer(StagingBuffer& buffer, CommandList& cmd_list) {
	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertex_data = {};
	vertex_data.pData = reinterpret_cast<BYTE*>(buffer.data);
	vertex_data.RowPitch = buffer.size;
	vertex_data.SlicePitch = buffer.size;

	UpdateSubresources(cmd_list.native, buffer.buffer, buffer.staging, 0, 0, 1, &vertex_data);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	cmd_list.native->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.buffer, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)buffer.target_resource_state));
}

void FreeStagingBuffer(StagingBuffer& buffer) {
	buffer.staging->Release();
}

void Destroy(StagingBuffer& buffer) {
	buffer.buffer->Release();
}

} /* rlr */
