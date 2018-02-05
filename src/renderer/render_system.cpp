#include "render_system.h"

#include <iostream>
#include <vector>
#include <d3d12.h>
#ifdef _DEBUG
#define INITGUID
#include <DXGIDebug.h>
#include <DirectXMath.h>
#endif

#include "dx12\imgui_impl_dx12.h"
#include "dx12\d3dx12.h"
#include "vertex.h"
#include "model.h"
#include "drawable.h"

#include "math\vec.hpp"

#include "util\thread_pool.h"

namespace rlr {

	RenderSystem::RenderSystem(Window& window) : window(window) {
		CreateLight({ 0, 0, -3 }, { 1, 1, 1 });
		CreateLight({ -2, 0, 0 }, { 1, 0, 0 });
		CreateLight({ 2, 0, 0 }, { 0, 1, 0 });

		thread_pool = new rlu::ThreadPool(2);
	}

	RenderSystem::~RenderSystem() {
		for (auto i = 0; i < 3; i++) {
			WaitFor(fences[i]);
			Destroy(fences[i]);
		}

		ImGui_ImplDX12_Shutdown();
		ImGui::ShutdownDock();

		ClearAllPipelines();
		Destroy(*imgui_cmd_list);
		Destroy(*main_cmd_list);
		Destroy(render_window);
		Destroy(game_render_target);
		Destroy(main_cmd_queue);
		
		delete device;
		delete main_cmd_queue;

//#ifdef _DEBUG
		Microsoft::WRL::ComPtr<IDXGIDebug> dxgiControler;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiControler)))) {
			dxgiControler->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_SUMMARY));
		}
//#endif	
	}

	void RenderSystem::SetupSwapchain(int width, int height) {
		Create(render_window, *device, window, *main_cmd_queue);

		RenderTargetCreateInfo game_rt_create_info;
		game_rt_create_info.num_rtv_formats = 3;
		game_rt_create_info.dsv_format = Format::D32_FLOAT;
		game_rt_create_info.rtv_formats[0] = Format::R8G8B8A8_UNORM; // albade
		game_rt_create_info.rtv_formats[1] = Format::R16G16B16A16_FLOAT; // normal
		game_rt_create_info.rtv_formats[2] = Format::R16G16B16A16_FLOAT; // position

		Create(game_render_target, *device, *main_cmd_queue, width, height, game_rt_create_info);

		RenderTargetCreateInfo shadow_rt_create_info;
		shadow_rt_create_info.num_rtv_formats = 1;
		shadow_rt_create_info.dsv_format = Format::D32_FLOAT;
		shadow_rt_create_info.rtv_formats[0] = Format::R8G8B8A8_UNORM;

		Create(shadow_render_target, *device, *main_cmd_queue, SHADOW_SIZE, SHADOW_SIZE, shadow_rt_create_info);

		RenderTargetCreateInfo deferred_rt_create_info;
		deferred_rt_create_info.num_rtv_formats = 2;
		deferred_rt_create_info.create_dsv_buffer = false;
		deferred_rt_create_info.dsv_format = Format::D32_FLOAT;
		deferred_rt_create_info.rtv_formats[0] = Format::R16G16B16A16_UNORM;
		deferred_rt_create_info.rtv_formats[1] = Format::R16G16B16A16_UNORM;

		Create(deferred_render_target, *device, *main_cmd_queue, width, height, deferred_rt_create_info);

		RenderTargetCreateInfo imgui_rt_create_info;
		imgui_rt_create_info.num_rtv_formats = 1;
		imgui_rt_create_info.create_dsv_buffer = false;
		imgui_rt_create_info.dsv_format = Format::D32_FLOAT;
		imgui_rt_create_info.rtv_formats[0] = Format::R8G8B8A8_UNORM;

		Create(imgui_game_render_target, *device, *main_cmd_queue, width, height, imgui_rt_create_info);

		// blur
		RenderTargetCreateInfo blur_rt_create_info;
		blur_rt_create_info.num_rtv_formats = 1;
		blur_rt_create_info.create_dsv_buffer = false;
		blur_rt_create_info.dsv_format = Format::D32_FLOAT;
		blur_rt_create_info.rtv_formats[0] = Format::R16G16B16A16_UNORM;

		Create(blur_render_target, *device, *main_cmd_queue, width/2, height/2, blur_rt_create_info);

		// ssao
		RenderTargetCreateInfo ssao_rt_create_info;
		ssao_rt_create_info.num_rtv_formats = 1;
		ssao_rt_create_info.create_dsv_buffer = false;
		ssao_rt_create_info.dsv_format = Format::D32_FLOAT;
		ssao_rt_create_info.rtv_formats[0] = Format::R8_UNORM; // ssao

		Create(ssao_render_target, *device, *main_cmd_queue, width, height, ssao_rt_create_info);

		// ssao blur
		Create(blur_ssao_render_target, *device, *main_cmd_queue, width, height, ssao_rt_create_info);

		if (render_engine) {
			composition_render_target = imgui_game_render_target;
		}
		else {
			composition_render_target = render_window;
		}
	}


	void RenderSystem::SetupDescriptorHeaps() {

		// ### Descriptor Heap 0 ###
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC noise_texture_view_desc = {};
			noise_texture_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			noise_texture_view_desc.Format = ssao_texture.desc.Format;
			noise_texture_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			noise_texture_view_desc.Texture2D.MipLevels = 1;

			D3D12_SHADER_RESOURCE_VIEW_DESC sky_texture_view_desc = {};
			sky_texture_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			sky_texture_view_desc.Format = sky_texture.desc.Format;
			sky_texture_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			sky_texture_view_desc.TextureCube.MipLevels = sky_texture.desc.MipLevels;
			sky_texture_view_desc.TextureCube.ResourceMinLODClamp = 0;

			DescHeapCPUHandle handle = GetCPUHandle(srv_descriptor_heap_0);
			/* 0-2 */CreateSRVFromRTV(game_render_target, *device, handle, game_render_target.create_info.num_rtv_formats, game_render_target.create_info.rtv_formats);
			/* 3   */CreateSRVFromDSV(shadow_render_target, *device, handle);
			/* 4   */device->native->CreateShaderResourceView(ssao_texture.resource, &noise_texture_view_desc, handle.native);
			Offset(handle, 1, srv_descriptor_heap_0->increment_size);
			/* 5   */CreateSRVFromRTV(blur_ssao_render_target, *device, handle, blur_ssao_render_target.create_info.num_rtv_formats, blur_ssao_render_target.create_info.rtv_formats);
			/* 6   */device->native->CreateShaderResourceView(sky_texture.resource, &sky_texture_view_desc, handle.native);
			Offset(handle, 1, srv_descriptor_heap_0->increment_size);
		}
		// ### Descriptor Heap 1 ###
		{
			DescHeapCPUHandle handle = GetCPUHandle(srv_descriptor_heap_1);
			/* 0   */CreateSRVFromRTV(ssao_render_target, *device, handle, ssao_render_target.create_info.num_rtv_formats, ssao_render_target.create_info.rtv_formats);
		}
		// ### Descriptor Heap 2 ###
		{
			DescHeapCPUHandle handle = GetCPUHandle(srv_descriptor_heap_2);
			CreateSRVFromRTV(deferred_render_target, *device, handle, deferred_render_target.create_info.num_rtv_formats, deferred_render_target.create_info.rtv_formats);
		}
	}

	void RenderSystem::SetupRootSignatures() {
		/* ROOT SIGNATURE 0*/
		{ 
			CD3DX12_DESCRIPTOR_RANGE  desc_table_ranges;
			desc_table_ranges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0);

			rlr::RootSignatureCreateInfo rs_info;
			rs_info.samplers.push_back({ TextureFilter::FILTER_POINT, TextureAddressMode::TAM_CLAMP });
			rs_info.parameters.resize(3);
			rs_info.parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
			rs_info.parameters[1].InitAsDescriptorTable(1, &desc_table_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
			rs_info.parameters[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			Create(root_signature_0, rs_info);
			Finalize(root_signature_0, *device);
		}

		/* ROOT SIGNATURE 1*/
		{
			CD3DX12_DESCRIPTOR_RANGE  desc_table_ranges;
			desc_table_ranges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			rlr::RootSignatureCreateInfo rs_info;
			rs_info.samplers.push_back({ TextureFilter::FILTER_POINT, TextureAddressMode::TAM_MIRROR });
			rs_info.parameters.resize(3);
			rs_info.parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rs_info.parameters[1].InitAsDescriptorTable(1, &desc_table_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
			rs_info.parameters[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			Create(root_signature_1, rs_info);
			Finalize(root_signature_1, *device);
		}

		/* ROOT SIGNATURE 2*/
		{
			CD3DX12_DESCRIPTOR_RANGE  desc_table_ranges;
			desc_table_ranges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

			rlr::RootSignatureCreateInfo rs_info;
			rs_info.samplers.push_back({ TextureFilter::FILTER_LINEAR, TextureAddressMode::TAM_MIRROR });
			rs_info.parameters.resize(3);
			rs_info.parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rs_info.parameters[1].InitAsDescriptorTable(1, &desc_table_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
			rs_info.parameters[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			Create(root_signature_2, rs_info);
			Finalize(root_signature_2, *device);
		}
	}

	void RenderSystem::Setup() {
		window.BindOnResize([&](int width, int height) {
			// Make sure all command queues are finished.
			for (auto i = 0; i < 3; i++) {
				WaitFor(fences[i]);
				Signal(fences[i], main_cmd_queue); // If I don't signal them they will be stuck when calling waitfor at the beginning of the frame.
			}

			cam->SetAspectRatio((float)width / (float)height);

			Destroy(srv_descriptor_heap_0);
			Destroy(srv_descriptor_heap_1);
			Destroy(srv_descriptor_heap_2);

			Resize(render_window, *device, *main_cmd_queue, width, height);
			Resize(game_render_target, *device, *main_cmd_queue, width, height);
			Resize(deferred_render_target, *device, *main_cmd_queue, width, height);
			Resize(ssao_render_target, *device, *main_cmd_queue, width, height);
			Resize(blur_ssao_render_target, *device, *main_cmd_queue, width, height);
			Resize(blur_render_target, *device, *main_cmd_queue, width/2, height/2);

			if (!render_engine) {
				composition_render_target = render_window; // Update pointer.
			}
			else {
				Resize(imgui_game_render_target, *device, *main_cmd_queue, width, height);
				composition_render_target = imgui_game_render_target;
			}

			CreateMainDescriptorHeap();

			SetupDescriptorHeaps();

			CreateSRVFromSwapchain();

			Begin(*imgui_cmd_list, render_window.frame_idx);

			// transitions
			Transition(*imgui_cmd_list, game_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Transition(*imgui_cmd_list, ssao_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Transition(*imgui_cmd_list, blur_ssao_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Transition(*imgui_cmd_list, deferred_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Transition(*imgui_cmd_list, blur_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			if (render_engine) Transition(*imgui_cmd_list, composition_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Transition(*imgui_cmd_list, imgui_game_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);

			End(*imgui_cmd_list);
			std::vector<CommandList> cmd_lists = { *imgui_cmd_list };
			Execute(main_cmd_queue, cmd_lists, fences[render_window.frame_idx]);
			Signal(fences[render_window.frame_idx], main_cmd_queue);
		});

		int width, height;
		window.GetSize(&width, &height);

		Create(&device);

		main_cmd_queue = new CommandQueue();
		Create(main_cmd_queue, device, CmdQueueType::CMD_QUEUE_DIRECT);

		CreateMainDescriptorHeap();

		SetupSwapchain(width, height);

		Allocate(&main_cmd_list, *device, 3);
		main_cmd_list->native->SetName(L"Maincmd");

		Allocate(&imgui_cmd_list, *device, 3);
		imgui_cmd_list->native->SetName(L"imgui");

		Allocate(&shadow_cmd_list, *device, 3);
		shadow_cmd_list->native->SetName(L"shadow cmd");

		Allocate(&deferred_cmd_list, *device, 3);
		deferred_cmd_list->native->SetName(L"deferred cmd");

		Allocate(&ssao_cmd_list, *device, 3);
		ssao_cmd_list->native->SetName(L"ssao cmd");

		Create(shadow_viewport, SHADOW_SIZE, SHADOW_SIZE);

		for (auto i = 0; i < fences.size(); i++) {
			Create(&fences[i], device);
		}

		Create(deferred_const_buffer, *device, sizeof(CBSceneStruct));
		Create(ssao_const_buffer, *device, sizeof(CBSSAOStruct));
		Create(projection_view_const_buffer, *device, sizeof(PVCBStruct));
		Create(shadow_projection_view_const_buffer, *device, sizeof(PVCBStruct));

		DescriptorHeapCreateInfo imgui_heap_create_info;
		imgui_heap_create_info.num_descriptors = 4;
		imgui_heap_create_info.type = DescriptorHeapType::DESC_HEAP_TYPE_CBV_SRV_UAV;
		Create(&imgui_descriptor_heap, device, imgui_heap_create_info);

		Create(compo_const_buffer, *device, sizeof(CBCompo));

		//if (render_engine) {
		ImGui::ApplyCustomStyle();
		ImGui_ImplDX12_Init(window.native, render_window.num_back_buffers, device->native, imgui_cmd_list->native,
			GetCPUHandle(imgui_descriptor_heap),
			GetGPUHandle(imgui_descriptor_heap));
		ImGui_ImplDX12_CreateDeviceObjects();
		//}
	}

	void RenderSystem::CreateSRVFromSwapchain() {
		int offset_size = device->native->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DescHeapCPUHandle rtv_handle(GetCPUHandle(imgui_descriptor_heap));
		Offset(rtv_handle, 1, imgui_descriptor_heap->increment_size);

		std::array<Format, 1> formats = { Format::R8G8B8A8_UNORM };
		CreateSRVFromRTV(imgui_game_render_target, *device, rtv_handle, formats.size(), formats.data());
	}

	void RenderSystem::Init() {
		cam = new Camera(1280.0f / 720.0f);
		cam->SetPos(fm::vec(0, 0, -3));
		cam->Update();

		shadow_cam = new Camera(1.f, true);
		shadow_cam->SetPos(fm::vec(0, 2, -3));
		shadow_cam->SetEuler(fm::vec(-40, 0, 0));
		shadow_cam->SetOrthographic();
		shadow_cam->Update();

		int width, height;
		window.GetSize(&width, &height);

		prev = std::chrono::high_resolution_clock::now();

		/*# DEFERRED #*/
		SetupRootSignatures();

		Load(deferred_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/engine/shaders/def_vertex.hlsl");
		Load(deferred_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/engine/shaders/def_pixel.hlsl");

		rlr::PipelineStateCreateInfo deferred_pso_info;
		deferred_pso_info.dsv_format = Format::UNKNOWN;
		deferred_pso_info.num_rtv_formats = 2;
		deferred_pso_info.rtv_formats[0] = Format::R16G16B16A16_UNORM;
		deferred_pso_info.rtv_formats[1] = Format::R16G16B16A16_UNORM;

		rlr::SetVertexShader(deferred_ps, &deferred_vertex_shader);
		rlr::SetFragmentShader(deferred_ps, &deferred_pixel_shader);
		rlr::SetRootSignature(deferred_ps, &root_signature_0);
		rlr::Finalize(deferred_ps, *device, render_window, deferred_pso_info);

		/*# BLURRY BLURRY #*/
		Load(blur_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/engine/shaders/blur_vertex.hlsl");
		Load(blur_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/engine/shaders/blur_pixel.hlsl");

		rlr::PipelineStateCreateInfo blur_pso_info;
		blur_pso_info.dsv_format = Format::UNKNOWN;
		blur_pso_info.num_rtv_formats = 1;
		blur_pso_info.rtv_formats[0] = Format::R16G16B16A16_UNORM;

		rlr::SetVertexShader(blur_ps, &blur_vertex_shader);
		rlr::SetFragmentShader(blur_ps, &blur_pixel_shader);
		rlr::SetRootSignature(blur_ps, &root_signature_2);
		rlr::Finalize(blur_ps, *device, render_window, blur_pso_info);

		/*# FINAL COMPOSITION #*/
		Load(composition_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/engine/shaders/compo_vertex.hlsl");
		Load(composition_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/engine/shaders/compo_pixel.hlsl");

		rlr::PipelineStateCreateInfo composition_pso_info;
		composition_pso_info.dsv_format = Format::UNKNOWN;
		composition_pso_info.num_rtv_formats = 1;
		composition_pso_info.rtv_formats[0] = Format::R8G8B8A8_UNORM;

		rlr::SetVertexShader(final_composition_ps, &composition_vertex_shader);
		rlr::SetFragmentShader(final_composition_ps, &composition_pixel_shader);
		rlr::SetRootSignature(final_composition_ps, &root_signature_2);
		rlr::Finalize(final_composition_ps, *device, render_window, composition_pso_info);

		/*# SCREEN SPACE AMBIENT OCCLUSION #*/

		Load(ssao_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/engine/shaders/ssao_vertex.hlsl");
		Load(ssao_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/engine/shaders/ssao_pixel.hlsl");

		rlr::PipelineStateCreateInfo ssao_pso_info;
		ssao_pso_info.dsv_format = Format::UNKNOWN;
		ssao_pso_info.num_rtv_formats = 1;
		ssao_pso_info.rtv_formats[0] = Format::R8_UNORM;

		rlr::SetVertexShader(ssao_ps, &ssao_vertex_shader);
		rlr::SetFragmentShader(ssao_ps, &ssao_pixel_shader);
		rlr::SetRootSignature(ssao_ps, &root_signature_0);
		rlr::Finalize(ssao_ps, *device, render_window, ssao_pso_info);

		Load(blur_ssao_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/engine/shaders/ssao_vertex.hlsl");
		Load(blur_ssao_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/engine/shaders/ssao_blur_pixel.hlsl");

		rlr::SetVertexShader(blur_ssao_ps, &blur_ssao_vertex_shader);
		rlr::SetFragmentShader(blur_ssao_ps, &blur_ssao_pixel_shader);
		rlr::SetRootSignature(blur_ssao_ps, &root_signature_1);
		rlr::Finalize(blur_ssao_ps, *device, render_window, ssao_pso_info);

		Begin(*main_cmd_list, render_window.frame_idx);

		Vertex vertices[] = {
			{ { -1.f, -1.f, 0.f },{ 0, 0, 0 },{ 0, 1 } },
			{ { 1.f, -1.f, 0.f },{ 0, 0, 0 },{ 1, 1 } },
			{ { 1.f, 1.f, 0.f },{ 0, 0, 0 },{ 1, 0 } },

			{ { 1.f, 1.f, 0.f },{ 0, 0, 0 },{ 1, 0 } },
			{ { -1.f, 1.f, 0.f },{ 0, 0, 0 },{ 0, 0 } },
			{ { -1.f, -1.f, 0.f },{ 0, 0, 0 },{ 0, 1 } },
		};

		Create(quad_vb, *device, vertices, 6 * sizeof(Vertex), sizeof(Vertex), ResourceState::VERTEX_AND_CONSTANT_BUFFER);
		StageBuffer(quad_vb, *main_cmd_list);

		rlr::Load(ssao_texture, "resources/engine/textures/ssao.png");
		rlr::Load(sky_texture, "resources/tests/sky.dds");

		StageTexture(ssao_texture, *device, *main_cmd_list); 
		StageTexture(sky_texture, *device, *main_cmd_list, true);

		staging_func(device, main_cmd_list);

		// transitions
		Transition(*main_cmd_list, game_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, ssao_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, blur_ssao_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, deferred_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, blur_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, shadow_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		if (render_engine) Transition(*main_cmd_list, composition_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Transition(*main_cmd_list, imgui_game_render_target, ResourceState::PRESENT, ResourceState::RENDER_TARGET);

		End(*main_cmd_list);

		//if (render_engine) {
		CreateSRVFromSwapchain();
		//}

		std::vector<CommandList> cmd_lists = { *main_cmd_list, *imgui_cmd_list };
		Execute(main_cmd_queue, cmd_lists, fences[render_window.frame_idx]);
		Signal(fences[render_window.frame_idx], main_cmd_queue);

		SetupDescriptorHeaps();

#ifdef ENABLE_PROFILER
		profiler::InitProfiler(device.native, main_cmd_queue->native);
#endif
	}

	void RenderSystem::CreateMainDescriptorHeap() {
		DescriptorHeapCreateInfo heap_0_create_info;
		heap_0_create_info.num_descriptors = 7;
		heap_0_create_info.type = DescriptorHeapType::DESC_HEAP_TYPE_CBV_SRV_UAV;
		Create(&srv_descriptor_heap_0, device, heap_0_create_info);

		DescriptorHeapCreateInfo heap_1_create_info;
		heap_1_create_info.num_descriptors = 1;
		heap_1_create_info.type = DescriptorHeapType::DESC_HEAP_TYPE_CBV_SRV_UAV;
		Create(&srv_descriptor_heap_1, device, heap_1_create_info);

		DescriptorHeapCreateInfo heap_2_create_info;
		heap_2_create_info.num_descriptors = 3;
		heap_2_create_info.type = DescriptorHeapType::DESC_HEAP_TYPE_CBV_SRV_UAV;
		Create(&srv_descriptor_heap_2, device, heap_2_create_info);
	}

	void RenderSystem::RenderImGui() {
		Begin(*imgui_cmd_list, render_window.frame_idx);
		PROFILER_BEGIN_ROOT_GPU("(GPU) Render", imgui_cmd_list->native)
		PROFILER_BEGIN_GPU("ImGui", imgui_cmd_list->native)
		Transition(*imgui_cmd_list, render_window, render_window.frame_idx, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
		Bind(*imgui_cmd_list, render_window, render_window.frame_idx);

		std::vector<DescriptorHeap*> heaps = { imgui_descriptor_heap };
		Bind(*imgui_cmd_list, heaps);

		ImGui_ImplDX12_NewFrame();

		if (imgui_render_func && render_engine) {
			imgui_render_func();
		}
		else {
			ImGui_RenderPerformanceOverlay();
		}

		ImGui::Render();

		PROFILER_END_GPU(imgui_cmd_list->native)
		Transition(*imgui_cmd_list, render_window, render_window.frame_idx, ResourceState::RENDER_TARGET, ResourceState::PRESENT);
		End(*imgui_cmd_list);
	}

	void RenderSystem::Render(SceneGraph* graph) {
		WaitFor(fences[render_window.frame_idx]);
		//WaitForSingleObjectEx(swapchainEvent, INFINITE, FALSE);

		shadow_cam->SetPos(cam->GetPos() + shadow_pos_offset);
		shadow_cam->SetEuler(shadow_rot);
		shadow_cam->Update();

		UpdatePVCB();
		UpdateSceneCB(); // TODO: Move this

		profiler::NewFrame();

		PROFILER_BEGIN_ROOT("(CPU) Render");
		if (render_engine) {
			PROFILER_BEGIN("ImGui");

			RenderImGui();

			PROFILER_END()
		}
		PROFILER_BEGIN("RLRenderer");

		frames++;
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = now - prev;

		if (diff.count() >= 1) {
			prev = std::chrono::high_resolution_clock::now();
			fps = frames;
			frames = 0;
		}

#ifdef REDUCE_PIPELINE_STATE_CHANGES
		last_pipeline_state = nullptr;
#endif

		/* # NORMAL RENDERING # */
		//auto f2 = thread_pool->enqueue([&]() {
			Begin(*main_cmd_list, render_window.frame_idx);
			PROFILER_BEGIN_GPU("Custom", main_cmd_list->native);
			PROFILER_BEGIN_GPU("Game rendering", main_cmd_list->native);
			Bind(*main_cmd_list, game_render_target, render_window.frame_idx, true, true);

			// Stage the static intanced rendering buffer.
			if (static_inst_needs_staging) {
				for (std::map<int, Batch*>::iterator it = static_instanced_batches.begin(); it != static_instanced_batches.end(); it++) {
					StageBuffer(it->second->instanced_staging_buffer, *main_cmd_list);
				}
				static_inst_needs_staging = false;
			}

			using recursive_func_t = std::function<void(std::shared_ptr<Node>)>;

			recursive_func_t recursive_draw = [&](std::shared_ptr<Node> node) {
				for (auto child : node->GetChildren()) {
					child->Render(*main_cmd_list, *cam, false);
					recursive_draw(child);
				}
			};

			recursive_draw(graph->root);

			Populate_InstancedDrawables(*main_cmd_list, *cam);

			PROFILER_END_GPU(main_cmd_list->native)
			End(*main_cmd_list);
		//});
		/* # NORMAL RENDERING # */

		/* # SHADOW DEPTH RENDERING # */
		///auto f1 = thread_pool->enqueue([&]() {
			Begin(*shadow_cmd_list, render_window.frame_idx);
			SBind(*shadow_cmd_list, shadow_render_target, render_window.frame_idx);
			PROFILER_BEGIN_GPU("Shadow rendering", shadow_cmd_list->native);

			recursive_func_t recursive_shadow_draw = [&](std::shared_ptr<Node> node) {
				for (auto child : node->GetChildren()) {
					child->Render(*shadow_cmd_list, *cam, true);
					recursive_shadow_draw(child);
				}
			};

			recursive_shadow_draw(graph->root);
			//Populate_Drawables(*shadow_cmd_list, drawables, *shadow_cam, 0, drawables.size(), true);

			PROFILER_END_GPU(shadow_cmd_list->native)
			End(*shadow_cmd_list);
		//});
		/* # SHADOW DEPTH RENDERING # */

		/* # SCREEN SPACE AMBIENT OCCLUSION # */
		Begin(*ssao_cmd_list, render_window.frame_idx);
		PROFILER_BEGIN_GPU("Screen Space Ambient Occlusion", ssao_cmd_list->native)

		// Standard SSAO
		PROFILER_BEGIN_GPU("SSAO - Standard", ssao_cmd_list->native)
		Bind(*ssao_cmd_list, ssao_render_target, render_window.frame_idx, false);
		Populate_FullscreenQuad(*ssao_cmd_list, ssao_ps, ssao_const_buffer, srv_descriptor_heap_0, graph->GetViewport());
		PROFILER_END_GPU(ssao_cmd_list->native);

		// Blur
		PROFILER_BEGIN_GPU("SSAO - Blurring", ssao_cmd_list->native)
		Bind(*ssao_cmd_list, blur_ssao_render_target, render_window.frame_idx, false);
		Populate_FullscreenQuad(*ssao_cmd_list, blur_ssao_ps, ssao_const_buffer, srv_descriptor_heap_1, graph->GetViewport());
		PROFILER_END_GPU(ssao_cmd_list->native);

		PROFILER_END_GPU(ssao_cmd_list->native);
		End(*ssao_cmd_list);
		/* # SCREEN SPACE AMBIENT OCCLUSION # */

		/* # DEFERRED # */
		Begin(*deferred_cmd_list, render_window.frame_idx);
		PROFILER_BEGIN_GPU("Deferred rendering", deferred_cmd_list->native)
		Bind(*deferred_cmd_list, deferred_render_target, render_window.frame_idx, false, false, true);

		Populate_FullscreenQuad(*deferred_cmd_list, deferred_ps, deferred_const_buffer, srv_descriptor_heap_0, graph->GetViewport());

		PROFILER_END_GPU(deferred_cmd_list->native);
		/* # DEFERRED # */

		/* # BLUR # */
		if (imgui_bloom){
			int amount = 10;

			Bind(*deferred_cmd_list, blur_render_target, render_window.frame_idx, true);
			PROFILER_BEGIN_GPU("Blurring", deferred_cmd_list->native)

			Bind(*deferred_cmd_list, blur_ps);
			Bind(*deferred_cmd_list, graph->GetViewport());

			std::vector<DescriptorHeap*> heaps = { srv_descriptor_heap_2 };
			Bind(*deferred_cmd_list, heaps);

			DescHeapGPUHandle handle = GetGPUHandle(srv_descriptor_heap_2);
			Bind(*deferred_cmd_list, handle, 1);

			BindVertexBuffer(*deferred_cmd_list, quad_vb);
			Bind(*deferred_cmd_list, deferred_const_buffer, 0, render_window.frame_idx);

			for (auto i = 0; i < amount; i++) {
				Draw(*deferred_cmd_list, 6, 1);

				if (i == 0) {
					DescHeapCPUHandle at_main_srv_handle = GetCPUHandle(srv_descriptor_heap_2);
					Offset(at_main_srv_handle, 2, srv_descriptor_heap_2->increment_size);
					CreateSRVFromRTV(blur_render_target, *device, at_main_srv_handle, blur_render_target.create_info.num_rtv_formats, blur_render_target.create_info.rtv_formats);
				}
			}

			PROFILER_END_GPU(deferred_cmd_list->native)
		}
		/* # BLUR # */

		/* # FINAL COMPOSITION # */
		{
			if (!render_engine) Transition(*deferred_cmd_list, composition_render_target, render_window.frame_idx, ResourceState::PRESENT, ResourceState::RENDER_TARGET);
			Bind(*deferred_cmd_list, composition_render_target, render_window.frame_idx, false);
			PROFILER_BEGIN_GPU("Final Composition", deferred_cmd_list->native)

			Populate_FullscreenQuad(*deferred_cmd_list, final_composition_ps, compo_const_buffer, srv_descriptor_heap_2, graph->GetViewport());

			if (!render_engine) Transition(*deferred_cmd_list, composition_render_target, render_window.frame_idx, ResourceState::RENDER_TARGET, ResourceState::PRESENT);
			PROFILER_END_GPU(deferred_cmd_list->native);
			PROFILER_END_GPU(deferred_cmd_list->native);
			PROFILER_END_ROOT_GPU(deferred_cmd_list->native)
			End(*deferred_cmd_list);
		}
		/* # FINAL COMPOSITION # */

		std::vector<CommandList> cmd_lists = { *shadow_cmd_list, *main_cmd_list, *ssao_cmd_list, *deferred_cmd_list };
		if (render_engine) {
			cmd_lists = { *imgui_cmd_list, *ssao_cmd_list, *shadow_cmd_list, *deferred_cmd_list,  *main_cmd_list };
		}

		//f2.get();
		//f1.get(); // shadow

		Execute(main_cmd_queue, cmd_lists, fences[render_window.frame_idx]);
		Signal(fences[render_window.frame_idx], main_cmd_queue);

		Present(render_window, *main_cmd_queue, *device);

		PROFILER_END()
		PROFILER_END_ROOT()

		num_pipeline_changes = 0;
	}

	std::string readablevec(fm::vec3 v) {
		return std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z);
	}

	/*! \brief Populates a command list with the actuall drawables being rendererd.
	*   This data can be used for either the gbuffers and/or shadow mapping.
	*   This function renders INSTANCED drawables to the a render target.
	*	This function requires a render target to be bound BEFORE being called.
	*   The render target is being cleared.
	*/
	void RenderSystem::Populate_InstancedDrawables(CommandList& cmd_list, Camera const& camera) {
		for (std::map<int, Batch*>::iterator it = static_instanced_batches.begin(); it != static_instanced_batches.end(); it++) {
			DrawableNode* drawable = it->second->inst_drawable;

			BIND_PIPELINE(cmd_list, drawable->pipeline_id)
			std::vector<ID3D12DescriptorHeap*> heaps = { drawable->ta->texture_heap };
			cmd_list.native->SetDescriptorHeaps(heaps.size(), heaps.data());

			for (auto i = 0; i < drawable->model->meshes.size(); i++) {
				D3D12_VERTEX_BUFFER_VIEW views[2];
				views[0].BufferLocation = drawable->model->meshes[i].vb.buffer->GetGPUVirtualAddress();
				views[0].StrideInBytes = drawable->model->meshes[i].vb.stride_in_bytes;
				views[0].SizeInBytes = drawable->model->meshes[i].vb.size;

				views[1].BufferLocation = it->second->instanced_staging_buffer.buffer->GetGPUVirtualAddress();
				views[1].StrideInBytes = it->second->instanced_staging_buffer.stride_in_bytes;
				views[1].SizeInBytes = it->second->instanced_staging_buffer.size;

				cmd_list.native->IASetVertexBuffers(0, 2, views);

				BindIndexBuffer(cmd_list, drawable->model->meshes[i].ib);
				Bind(cmd_list, *drawable->ta, 1);
				Bind(cmd_list, *drawable->const_buffer, 0, render_window.frame_idx);

				Bind(cmd_list, projection_view_const_buffer, 2, render_window.frame_idx);

				DrawIndexed(cmd_list, drawable->model->meshes[i].indices.size(), it->second->num_inst_model);
			}
		}
	}

	/*! \brief Populates a command list with deferred (target) commands.
	*	This function renders a quad to a render target and applies shading to it.
	*   Only a quad is being rendererd.
	*	This function requires a render target to be bound BEFORE being called.
	*	The render target is NOT being cleared.
	*/
	void RenderSystem::Populate_FullscreenQuad(CommandList& cmd_list, PipelineState& pipeline, ConstantBuffer& cb, DescriptorHeap* srv_heap, Viewport viewport) {
		Bind(cmd_list, pipeline);
		Bind(cmd_list, viewport);

		std::vector<DescriptorHeap*> heaps = { srv_heap };
		Bind(cmd_list, heaps);

		DescHeapGPUHandle handle = GetGPUHandle(srv_heap);
		Bind(cmd_list, handle, 1);

		BindVertexBuffer(cmd_list, quad_vb);
		Bind(cmd_list, cb, 0, render_window.frame_idx);
		Draw(cmd_list, 6, 1);
	}

	void RenderSystem::RegisterImGuiRenderFunc(std::function<void()> func) {
		imgui_render_func = func;
	}

	void RenderSystem::RegisterStagingFunc(std::function<void(Device*, CommandList*)> func) {
		staging_func = func;
	}

	unsigned int RenderSystem::GetFramerate() {
		return fps;
	}

	void RenderSystem::ShowEngine() {
		render_engine = true;
		composition_render_target = imgui_game_render_target;
	}

	void RenderSystem::HideEngine() {
		render_engine = false;
		composition_render_target = render_window;
	}

	ImTextureID RenderSystem::GetGameViewRenderTarget() {
		// Get thingy to render. using previous frame
		int to_render = (render_window.frame_idx);
		if (to_render == 0) to_render = 3;
		to_render = 1;
		DescHeapGPUHandle rtv_handle = GetGPUHandle(imgui_descriptor_heap);
		Offset(rtv_handle, to_render, imgui_descriptor_heap->increment_size);

		return (ImTextureID)rtv_handle.native.ptr;
	}

	void RenderSystem::RegisterPipeline(std::string id, PipelineState* ps) {
		registerd_pipelines.insert(std::pair<std::string, PipelineState*>(id, ps));

		rlr::PipelineStateCreateInfo create_info;
		create_info.dsv_format = Format::D32_FLOAT;
		create_info.num_rtv_formats = 3;
		create_info.rtv_formats[0] = Format::R8G8B8A8_UNORM; // albade
		create_info.rtv_formats[1] = Format::R16G16B16A16_FLOAT; // normal
		create_info.rtv_formats[2] = Format::R16G16B16A16_FLOAT; // position

		Finalize(*ps->root_signature, *device);
		Finalize(*ps, *device, render_window, create_info);
	}

	void RenderSystem::UnregisterPipeline(std::string id) {
		delete registerd_pipelines[id];

		registerd_pipelines.erase(id);
	}

	PipelineState* RenderSystem::GetPipeline(std::string id) {
		return registerd_pipelines[id];
	}

	void RenderSystem::ClearAllPipelines() {
		std::map<std::string, PipelineState*>::iterator it;
		for (it = registerd_pipelines.begin(); it != registerd_pipelines.end(); it++)
			Destroy(*it->second);

		registerd_pipelines.clear();
	}

	void RenderSystem::UpdateGenericCB(Drawable& drawable, fm::vec3 position, fm::vec3 rotation, fm::vec3 scale, bool all) {
		{
			CBStruct cb_data;

			DirectX::XMMATRIX tr = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
			DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(rotation.x);
			DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(rotation.y);
			DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(rotation.z);
			DirectX::XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
			DirectX::XMMATRIX sc = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

			DirectX::XMMATRIX model = sc * rotMat * tr;
			DirectX::XMStoreFloat4x4(&cb_data.model, model);

			cb_data.instanced = drawable.instanced;

			for (size_t i = 0; i < drawable.model.meshes[0].skeleton.bone_mats.size(); i++) {
				cb_data.weightmatrices[i] = drawable.model.meshes[0].skeleton.bone_mats[i];
			}

			if (all) {
				for (auto i = 0; i < 3; i++) {
					Update(drawable.const_buffer, i, sizeof(cb_data), &cb_data);
					Update(drawable.shadow_const_buffer, i, sizeof(cb_data), &cb_data);
				}
			}
			else {
				Update(drawable.const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
				Update(drawable.shadow_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
			}
		}
	}

	void RenderSystem::UpdateGenericCB(std::shared_ptr<DrawableNode> drawable, fm::vec3 position, fm::vec3 rotation, fm::vec3 scale, bool all) {
		{
			CBStruct cb_data;

			DirectX::XMMATRIX tr = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
			DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(rotation.x);
			DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(rotation.y);
			DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(rotation.z);
			DirectX::XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
			DirectX::XMMATRIX sc = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

			DirectX::XMMATRIX model = sc * rotMat * tr;
			DirectX::XMStoreFloat4x4(&cb_data.model, model);

			cb_data.instanced = drawable->m_instanced;

			for (size_t i = 0; i < drawable->model->meshes[0].skeleton.bone_mats.size(); i++) {
				cb_data.weightmatrices[i] = aiMatrix4x4(drawable->model->meshes[0].skeleton.bone_mats[i]);
			}

			if (all) {
				for (auto i = 0; i < 3; i++) {
					Update(*drawable->const_buffer, i, sizeof(cb_data), &cb_data);
					Update(*drawable->shadow_const_buffer, i, sizeof(cb_data), &cb_data);
				}
			}
			else {
				Update(*drawable->const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
				Update(*drawable->shadow_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
			}
		}
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void RenderSystem::UpdateSceneCB() {
		{ // compo
			CBCompo cb_data;

			cb_data.tonemapping = imgui_tonemapping_id;
			cb_data.gamma = imgui_gamma;
			cb_data.exposure = imgui_exposure;
			cb_data.contrast = imgui_contrast;
			cb_data.bloom = imgui_bloom;

			Update(compo_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
		}

		{ // deferred
			CBSceneStruct cb_data;

			DirectX::XMFLOAT4X4 view = cam->GetViewMat();
			DirectX::XMFLOAT4X4 proj = cam->GetProjMat();
			DirectX::XMFLOAT4X4 sview = shadow_cam->GetViewMat();
			DirectX::XMFLOAT4X4 sproj = shadow_cam->GetProjMat();

			cb_data.lights = lights;
			cb_data.num_lights = lights.size();
			cb_data.view = view;
			cb_data.proj = proj;
			cb_data.shadow_view = sview;
			cb_data.shadow_proj = sproj;

			Update(deferred_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
		}

		{ // ssao
			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
			std::default_random_engine generator;
			std::array<fm::vec4, KERNEL_SIZE> ssaoKernel;
			for (unsigned int i = 0; i < KERNEL_SIZE; ++i)
			{
				fm::vec4 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator), 1);
				sample = sample.Normalized();
				sample = sample * randomFloats(generator);
				float scale = float(i) / KERNEL_SIZE;

				// scale samples s.t. they're more aligned to center of kernel
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample = sample * scale;
				ssaoKernel[i] = sample;
			}

			CBSSAOStruct cb_data;

			cb_data.samples = ssaoKernel;

			DirectX::XMFLOAT4X4 view = cam->GetViewMat();
			DirectX::XMFLOAT4X4 proj = cam->GetProjMat();

			cb_data.view = view;
			cb_data.proj = proj;

			Update(ssao_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
		}
	}

	void RenderSystem::UpdatePVCB() {
		{ // Normal
			PVCBStruct cb_data;

			DirectX::XMFLOAT4X4 view = cam->GetViewMat();
			DirectX::XMFLOAT4X4 proj = cam->GetProjMat();

			cb_data.view = view;
			cb_data.proj = proj;

			Update(projection_view_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
		}

		{ // Shadow
			PVCBStruct cb_data;

			DirectX::XMFLOAT4X4 view = shadow_cam->GetViewMat();
			DirectX::XMFLOAT4X4 proj = shadow_cam->GetProjMat();

			cb_data.view = view;
			cb_data.proj = proj;

			Update(shadow_projection_view_const_buffer, render_window.frame_idx, sizeof(cb_data), &cb_data);
		}
	}

	int RenderSystem::CreateLight(fm::vec3 pos, fm::vec3 color) {
		for (auto i = 0; i < lights.size(); i++) {
			if (lights[i].radius == 0) {
				lights[i].pos = fm::vec4(pos.x, pos.y, pos.z, 1);
				lights[i].color = fm::vec4(color.x, color.y, color.z, 1);;

				float light_max = std::fmaxf(std::fmaxf(lights[i].color.x, lights[i].color.y), lights[i].color.z);
				lights[i].radius = (-lights[i].lin + std::sqrtf(lights[i].lin * lights[i].lin - 4 * lights[i].quadratic * (lights[i].constant - (256.0 / 5.0) * light_max))) / (2 * lights[i].quadratic);

				return i;
			}
		}

		return -1;
	}

	Light& RenderSystem::GetLight(int id) {
		return lights[id];
	}

} /* rlr */