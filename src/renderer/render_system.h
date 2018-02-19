#pragma once

#define USE_D3D12
#define REDUCE_PIPELINE_STATE_CHANGES

#include "window.h"
#include "interface.h"
#include "profiler.h"
#include "imgui\imgui.h"
#include "camera.h"

#include <memory>
#include <d3d12.h>
#include <chrono>
#include <functional>

#include "model.h"

#include "scene_graph\scene_graph.h"
#include "scene_graph\drawable_node.h"

#include "render_system_defines.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace rlu {
	class ThreadPool;
}

class DrawableNode;

namespace rlr {

struct Model;
struct Material;

struct Batch {
	DrawableNode* inst_drawable;
	int num_inst_model = 0;
	std::vector<fm::vec3> inst_positions;
	StagingBuffer* instanced_staging_buffer;
};


class RenderSystem {
private:
	std::map<std::string, PipelineState*> registerd_pipelines;
	std::map<std::string, Material*> registerd_materials;

	void RenderImGui();

#ifdef REDUCE_PIPELINE_STATE_CHANGES
	PipelineState* last_pipeline_state = nullptr;
#endif

	Window& window;
public:
	Device* device;

	CommandQueue* main_cmd_queue;
	CommandList* main_cmd_list;
	CommandList* imgui_cmd_list;
	CommandList* shadow_cmd_list;
	CommandList* ssao_cmd_list;

	rlu::ThreadPool* thread_pool;

	Texture* sky_texture;

	/*
	GBuffer 0
	GBuffer 1
	GBuffer 2
	Shadow Depth
	ssao noise
	ssao blur
	Sky texture
	*/
	DescriptorHeap* srv_descriptor_heap_0;
	RootSignature* root_signature_0;
	/*
	ssao
	*/
	DescriptorHeap* srv_descriptor_heap_1;
	RootSignature* root_signature_1;
	/*
	deferred hdr
	deferred overdose
	overdosed blur (deferred overdose after being blurred)
	*/
	DescriptorHeap* srv_descriptor_heap_2;
	RootSignature* root_signature_2;

	DescriptorHeap* imgui_descriptor_heap;

	// Instanced
	std::map<int, Batch*> static_instanced_batches;
	bool static_inst_needs_staging = false;
	ConstantBuffer* projection_view_const_buffer;

	ConstantBuffer* shadow_projection_view_const_buffer;

	// SSAO Rendering
	PipelineState* ssao_ps;
	rlr::Shader* ssao_pixel_shader;
	rlr::Shader* ssao_vertex_shader;
	ConstantBuffer* ssao_const_buffer;
	rlr::Texture* ssao_texture;

	// SSAO BLUR Rendering
	PipelineState* blur_ssao_ps;
	rlr::Shader* blur_ssao_pixel_shader;
	rlr::Shader* blur_ssao_vertex_shader;
	ConstantBuffer* blur_ssao_const_buffer;

	// Deferred Rendering
	CommandList* deferred_cmd_list;
	StagingBuffer* quad_vb;
	PipelineState* deferred_ps;
	PipelineState* blur_ps;
	PipelineState* final_composition_ps;
	rlr::Shader* deferred_pixel_shader;
	rlr::Shader* deferred_vertex_shader;
	rlr::Shader* composition_pixel_shader;
	rlr::Shader* composition_vertex_shader;
	rlr::Shader* blur_pixel_shader;
	rlr::Shader* blur_vertex_shader;

	ConstantBuffer* deferred_const_buffer;
	ConstantBuffer* compo_const_buffer;

	RenderTarget game_render_target;
	RenderTarget imgui_game_render_target;
	RenderTarget ssao_render_target;
	RenderTarget blur_ssao_render_target;
	RenderWindow render_window;
	RenderTarget deferred_render_target;
	RenderTarget blur_render_target;

	RenderTarget composition_render_target;

	std::array<Fence*, 3> fences;
	Viewport shadow_viewport;
	Camera* cam;

	// Shadow mapping
	fm::vec shadow_pos_offset = { 0, 3, -3};
	fm::vec shadow_rot = { -40, 0, 0};
	Camera* shadow_cam;
	RenderTarget shadow_render_target;

	std::function<void(Device*, CommandList*)> staging_func;
	std::function<void()> imgui_render_func;

	bool render_engine = true;
	unsigned int fps = 0;
	unsigned int frames = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> prev;

	void Populate_InstancedDrawables(CommandList* cmd_list, Camera const& camera);
	void Populate_FullscreenQuad(CommandList& cmd_list, PipelineState* pipeline, ConstantBuffer* cb, DescriptorHeap* srv_heap, Viewport viewport);

	void BindPipelineOptimized(CommandList* cmd_list, std::string const& id);
	void SetupSwapchain(int width, int height);
	void SetupDescriptorHeaps();
	void SetupRootSignatures();
	void CreateSRVFromSwapchain();

public:
	RenderSystem(Window& window);
	~RenderSystem();

	std::array<Light, NUM_LIGHTS> lights;

	void Setup();
	void Init();
	void CreateMainDescriptorHeap();

	void Render(SceneGraph* graph);
	void RegisterImGuiRenderFunc(std::function<void()> func);
	void RegisterStagingFunc(std::function<void(Device*, CommandList*)> func);
	unsigned int GetFramerate();
	void ShowEngine();
	void HideEngine();
	ImTextureID GetGameViewRenderTarget();

	void RegisterPipeline(std::string id, PipelineState* ps);
	void UnregisterPipeline(std::string id);
	void ClearAllPipelines();
	PipelineState* GetPipeline(std::string id);

	void UpdateGenericCB(std::shared_ptr<DrawableNode> drawable, fm::vec3 position, fm::vec3 rotation, fm::vec3 scale, bool all);
	void UpdateSceneCB();
	void UpdatePVCB();

	int CreateLight(fm::vec3 pos, fm::vec3 color);
	Light& GetLight(int id);

	// ImGui Stuff
private:
	int imgui_tonemapping_id = 0;
	float imgui_exposure = 1;
	float imgui_gamma = 1;
	float imgui_contrast = 1;
	float imgui_chroma = 0;
	bool imgui_bloom = true;

	bool imgui_profiler_align = true;
	bool imgui_profiler_solid_colors = false;
	bool imgui_profiler_use_target_fps = false;
	float imgui_profiler_target_fps = 60;

	bool imgui_game_view_constrain_aspect_ratio = false;
	float imgui_game_view_aspect_ratio = 1.7777778f;

	bool imgui_show_perf_overlay = true;
	bool imgui_show_profiler = true;
	bool imgui_show_render_graph = true;
	bool imgui_show_light_properties = true;
	bool imgui_show_cam_properties = true;
	bool imgui_show_hardware_details = true;
	bool imgui_show_game_view = true;
	bool imgui_show_postprocessing_settings = true;

	void ImGui_DrawRect(ImDrawList* dl, profiler::Precision time, profiler::Precision frame, bool align, bool solid_color, profiler::Precision* time_offset);
	void ImGui_DrawBar(std::shared_ptr<profiler::Node> node, profiler::Precision entire_frame, bool align = true, bool solid_color = false, int idx = 0, profiler::Precision* time_offset = 0);

public:
	void ImGui_RenderMainMenuBarMenu();
	void ImGui_RenderGameView(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderProfiler(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderLightProperties(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderCameraProperties(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderHardwareDetails(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderPostProcessingSettings(IMGUI_RENDER_FUNC_DEFAULT_PARAMS);
	void ImGui_RenderRenderGraph(IMGUI_RENDER_FUNC_DEFAULT_PARAMS, SceneGraph* graph = nullptr);
	void ImGui_RenderPerformanceOverlay();
};

}  /* rlr */