#include "../interface.h"

#include "dx12_descriptor_heap.h"

namespace rlr {

void Create(DescriptorHeap& heap, Device& device, DescriptorHeapCreateInfo& create_info) {
	heap.create_info = create_info;
	heap.increment_size = device.native->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(create_info.type));

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
	heap_desc.NumDescriptors = create_info.num_descriptors;
	heap_desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)create_info.type;
	heap_desc.Flags = create_info.shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heap_desc.NodeMask = 0;
	HRESULT hr = device.native->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap.native));
}

DescHeapGPUHandle GetGPUHandle(DescriptorHeap& heap, int index) {
	DescHeapGPUHandle retval;;
	
	retval.native = heap.native->GetGPUDescriptorHandleForHeapStart();

	if (index > 0) {
		Offset(retval, index, heap.increment_size);
	}

	return retval;
}

DescHeapCPUHandle GetCPUHandle(DescriptorHeap& heap, int index) {
	DescHeapCPUHandle retval;;

	retval.native = heap.native->GetCPUDescriptorHandleForHeapStart();

	if (index > 0) {
		Offset(retval, index, heap.increment_size);
	}

	return retval;
}

void Offset(DescHeapGPUHandle& handle, int index, int increment_size ) {
	handle.native.ptr += index * increment_size;
}

void Offset(DescHeapCPUHandle& handle, int index, int increment_size) {
	handle.native.ptr += index * increment_size;
}

void Destroy(DescriptorHeap& heap) {
	heap.native->Release();
}

} /* rlr */
