#include "dx12_fence.h"

#include "../interface.h"

namespace rlr {

void Create(Fence** fence, Device* device) {
	HRESULT hr;
	Fence* new_fence = new Fence();

	// create the fences
	hr = device->native->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&new_fence->native));
	if (FAILED(hr)) {
		throw "Failed to create fence.";
	} 
	new_fence->native->SetName(L"SimpleFence");
	
	// create a handle to a fence event
	new_fence->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (new_fence->fence_event == nullptr) {
		throw "Failed to create fence event.";
	}

	(*fence) = new_fence;
}

void Signal(Fence* fence, CommandQueue* cmd_queue) {
	HRESULT hr = cmd_queue->native->Signal(fence->native, fence->fence_value);
	if (FAILED(hr)) {
		throw "Failed to set fence signal.";
	}
}

void WaitFor(Fence* fence) {
	if (fence->native->GetCompletedValue() < fence->fence_value) {
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		HRESULT hr = fence->native->SetEventOnCompletion(fence->fence_value, fence->fence_event);
		if (FAILED(hr)) {
			throw "Failed to set fence event.";
		}

		WaitForSingleObject(fence->fence_event, INFINITE);
	}

	// increment fenceValue for next frame
	fence->fence_value++;
}

void Destroy(Fence* fence) {
	fence->native->Release();
	fence->native = 0;
	CloseHandle(fence->fence_event);
	fence->fence_value = 0;
}

} /* rlr */
