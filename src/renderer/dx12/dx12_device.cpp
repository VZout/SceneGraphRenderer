#include "../interface.h"

#include "dx12_device.h"

namespace rlr {

namespace internal {
#ifdef _DEBUG
	void InitDebugLayer(Device* device) {
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&device->debug_controller)))) {
			device->debug_controller->EnableDebugLayer();
		}
	}

	void GetSysInfo(Device* device) {
		GetNativeSystemInfo(&device->sys_info);
		device->adapter->GetDesc1(&device->adapter_info);
	}
#endif

	void FindPhysicalDevice(Device* device) {
		IDXGIAdapter1* adapter = nullptr;
		int adapterIndex = 0;

		// Find a compatible adapter.
		while ((device->dxgi_factory)->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			// Skip software adapters.
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				adapterIndex++;
				continue;
			}

			// Create a device to test if the adapter supports the specified feature level.
			HRESULT hr = D3D12CreateDevice(adapter, device->feature_level, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr)) {
				break;
			}

			adapterIndex++;
		}

		if (adapter == nullptr) {
			throw "No comaptible adapter found.";
		}
		else {
			device->adapter = adapter;
		}
	}
}

void Create(Device** device) {
	Device* new_device = new Device();

#ifdef _DEBUG
	internal::InitDebugLayer(new_device);
#endif

	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&new_device->dxgi_factory));
	if (FAILED(hr)) {
		throw "Failed to create DXGIFactory.";
	}

	internal::FindPhysicalDevice(new_device);

	hr = D3D12CreateDevice(new_device->adapter, new_device->feature_level, IID_PPV_ARGS(&new_device->native));
	if (FAILED(hr)) {
		throw "Failed to create DX12 device.";
	}

#ifdef _DEBUG
	internal::GetSysInfo(new_device);
#endif

	(*device) = new_device;
}

void Destroy(Device* device) {
	device->adapter->Release();
	device->native->Release();
	device->dxgi_factory->Release();
}

} /* rlr */
