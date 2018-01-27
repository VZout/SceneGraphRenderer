#pragma once

#include <d3d12.h>
#include <dxgi1_5.h>

//#ifdef _DEBUG
#include <dxgidebug.h>
#include <wrl.h>
//#endif

namespace rlr {

struct Device {
	IDXGIAdapter1* adapter;
	ID3D12Device* native;
	IDXGIFactory5* dxgi_factory;
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

//#ifdef _DEBUG
	SYSTEM_INFO sys_info;
	DXGI_ADAPTER_DESC1 adapter_info;
	Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
//#endif
};

} /* rlr */
