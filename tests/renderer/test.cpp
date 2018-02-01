#include <renderer/render_system.h>
#include <renderer/window.h>

#include <renderer\imgui\imgui.h>
#include <renderer/animation_manager.h>
#include <renderer/scene_graph/viewport_node.h>
#include <renderer/enums.h>
#include <d3d12.h>

rlr::Window* window = nullptr;
SceneGraph* graph = nullptr;
rlr::RenderSystem* render_system = nullptr;
std::shared_ptr<rlr::ViewportNode> viewport_node;
std::shared_ptr<rlr::DrawableNode> dr0;
std::shared_ptr<rlr::DrawableNode> dr1;
std::shared_ptr<rlr::DrawableNode> back_wall;
std::shared_ptr<rlr::DrawableNode> left_wall;
std::shared_ptr<rlr::DrawableNode> right_wall;
rlr::Texture* spot_albedo = nullptr;
rlr::Texture* spot_spec = nullptr;
rlr::Texture* spot_metal = nullptr;
rlr::Texture* floor_texture = nullptr;
rlr::Texture* floor_texture_specular = nullptr;
rlr::Texture* floor_texture_normal = nullptr;
rlr::Texture* metal_texture = nullptr;
rlr::Texture* metal_texture_specular = nullptr;
rlr::Texture* metal_texture_normal = nullptr;
rlr::Texture* wall_texture = nullptr;
rlr::Texture* wall_texture_specular = nullptr;
rlr::Texture* wall_texture_normal = nullptr;
rlr::Model* floor_model = nullptr;

static int num_floors = 1;

bool first = true;
bool opened = true;
float v = 1;
bool show_engine = true;

std::chrono::time_point<std::chrono::high_resolution_clock> last_frame;

void ImGui_Render() {
	int width = ImGui::GetIO().DisplaySize.x, height = ImGui::GetIO().DisplaySize.y;
	ImGui::RootDock(ImVec2(0, 18), ImVec2(width, height - 18));

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Toggle Engine", NULL, &show_engine);
			if (!show_engine) {
				render_system->HideEngine();
			}
			ImGui::EndMenu();
		}
		render_system->ImGui_RenderMainMenuBarMenu();
		ImGui::EndMainMenuBar();
	}

	render_system->ImGui_RenderHardwareDetails(first, false, ImGui::DockStyle::TOP);
	render_system->ImGui_RenderPostProcessingSettings(first, true, ImGui::DockStyle::CENTER);
	render_system->ImGui_RenderRenderGraph(first, true, ImGui::DockStyle::CENTER, graph);
	render_system->ImGui_RenderGameView(first, true, ImGui::DockStyle::RIGHT);
	render_system->ImGui_RenderLightProperties(first, false, ImGui::DockStyle::BOTTOM);
	render_system->ImGui_RenderCameraProperties(first);
	render_system->ImGui_RenderProfiler(first);

	first = false;
}

void RegisterPipelines() {
	CD3DX12_DESCRIPTOR_RANGE  desc_table_ranges;
	desc_table_ranges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

	rlr::RootSignature* rs = new rlr::RootSignature();
	rlr::RootSignatureCreateInfo rs_info;
	rs_info.samplers.push_back({ rlr::TextureFilter::FILTER_POINT, rlr::TextureAddressMode::TAM_MIRROR });
	rs_info.parameters.resize(3);
	rs_info.parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rs_info.parameters[1].InitAsDescriptorTable(1, &desc_table_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
	rs_info.parameters[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	Create(*rs, rs_info);

	rlr::Shader pixel_shader;
	rlr::Shader vertex_shader;
	Load(vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/tests/vertex.hlsl");
	Load(pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/tests/pixel.hlsl");

	rlr::Shader anim_pixel_shader;
	rlr::Shader anim_vertex_shader;
	Load(anim_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/tests/anim_vertex.hlsl");
	Load(anim_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/tests/anim_pixel.hlsl");

	rlr::PipelineState* ps = new rlr::PipelineState();
	rlr::SetVertexShader(*ps, &vertex_shader);
	rlr::SetFragmentShader(*ps, &pixel_shader);
	rlr::SetRootSignature(*ps, rs);

	rlr::PipelineState* sps = new rlr::PipelineState();
	rlr::SetVertexShader(*sps, &vertex_shader);
	rlr::SetFragmentShader(*sps, &pixel_shader);
	rlr::SetRootSignature(*sps, rs);
	sps->temp = true;

	rlr::PipelineState* anim = new rlr::PipelineState();
	rlr::SetVertexShader(*anim, &anim_vertex_shader);
	rlr::SetFragmentShader(*anim, &anim_pixel_shader);
	rlr::SetRootSignature(*anim, rs);

	rlr::PipelineState* sanim = new rlr::PipelineState();
	rlr::SetVertexShader(*sanim, &anim_vertex_shader);
	rlr::SetFragmentShader(*sanim, &anim_pixel_shader);
	rlr::SetRootSignature(*sanim, rs);
	sanim->temp = true;

	render_system->RegisterPipeline("basic", ps);
	render_system->RegisterPipeline("basic_shadow", sps);
	render_system->RegisterPipeline("anim", anim);
	render_system->RegisterPipeline("anim_shadow", sanim);
}

void Staging(rlr::Device* device, rlr::CommandList* cmd_list) {
	StageTexture(*spot_albedo, *device, *cmd_list);
	StageTexture(*spot_spec, *device, *cmd_list);
	StageTexture(*spot_metal, *device, *cmd_list);
	StageTexture(*floor_texture, *device, *cmd_list);
	StageTexture(*floor_texture_specular, *device, *cmd_list);
	StageTexture(*floor_texture_normal, *device, *cmd_list);
	StageTexture(*metal_texture, *device, *cmd_list);
	StageTexture(*metal_texture_specular, *device, *cmd_list);
	StageTexture(*metal_texture_normal, *device, *cmd_list);
	StageTexture(*wall_texture, *device, *cmd_list);
	StageTexture(*wall_texture_specular, *device, *cmd_list);
	StageTexture(*wall_texture_normal, *device, *cmd_list);
	Stage(*dr0->model, *device, *cmd_list);
	Stage(*floor_model, *device, *cmd_list);
	Stage(*dr1->model, *device, *cmd_list);
}

#include <stdlib.h>

int main() {

	window = new rlr::Window();
	window->Create("Rougelike Engine", 1280, 720, false);

	render_system = new rlr::RenderSystem(*window);
	render_system->Setup();

	graph = new SceneGraph(*render_system);

	viewport_node = graph->CreateChildNode<rlr::ViewportNode>(graph->root, "Default Viewport", 1280, 720);
	dr0 = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Cow", "basic");
	dr1 = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Dancing Human", "anim");
	back_wall = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Back Wall", "basic");
	left_wall = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Left Wall", "basic");
	right_wall = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Right Wall", "basic");

	spot_albedo = new rlr::Texture();
	spot_spec = new rlr::Texture();
	spot_metal = new rlr::Texture();
	floor_texture = new rlr::Texture();
	floor_texture_specular = new rlr::Texture();
	floor_texture_normal = new rlr::Texture();
	metal_texture = new rlr::Texture();
	metal_texture_specular = new rlr::Texture();
	metal_texture_normal = new rlr::Texture();
	wall_texture = new rlr::Texture();
	wall_texture_specular = new rlr::Texture();
	wall_texture_normal = new rlr::Texture();
	rlr::Load(*spot_albedo, "resources/tests/spot.png");
	rlr::Load(*spot_spec, "resources/tests/spot_specular.png");
	rlr::Load(*spot_metal, "resources/tests/spot_metal.png");
	rlr::Load(*floor_texture, "resources/tests/rustediron2_basecolor.png");
	rlr::Load(*floor_texture_specular, "resources/tests/rustediron2_metallic.png");
	rlr::Load(*floor_texture_normal, "resources/tests/rustediron2_normal.png");
	rlr::Load(*metal_texture, "resources/tests/greasy-metal-pan1-albedo.png");
	rlr::Load(*metal_texture_specular, "resources/tests/greasy-metal-pan1-roughness.png");
	rlr::Load(*metal_texture_normal, "resources/tests/greasy-metal-pan1-normal.png");
	rlr::Load(*wall_texture, "resources/tests/agedplanks1-albedo.png");
	rlr::Load(*wall_texture_specular, "resources/tests/agedplanks1-roughness.png");
	rlr::Load(*wall_texture_normal, "resources/tests/agedplanks1-normal4-ue.png");

	floor_model = new rlr::Model();
	rlr::Load(*floor_model, "resources/tests/floor.fbx");

	dr0->model = new rlr::Model();
	dr1->model = new rlr::Model();
	rlr::Load(*dr0->model, "resources/tests/spot.obj");
	rlr::Load(*dr1->model, "resources/tests/dance.fbx");

	render_system->RegisterImGuiRenderFunc(ImGui_Render);
	render_system->RegisterStagingFunc(Staging);
	render_system->Init();

	RegisterPipelines();

	graph->InitAll();

	std::vector<rlr::Texture*> spot_textures = { spot_albedo, spot_spec, spot_albedo, spot_metal};
	std::vector<rlr::Texture*> metal_textures = { metal_texture, metal_texture_specular, metal_texture_normal };
	std::vector<rlr::Texture*> floor_textures = { floor_texture, floor_texture_specular, floor_texture_normal };
	std::vector<rlr::Texture*> wall_textures = { wall_texture, wall_texture_specular };
	dr0->SetTextures(spot_textures);
	back_wall->SetTextures(wall_textures);
	right_wall->SetTextures(metal_textures);
	left_wall->SetTextures(metal_textures);
	dr1->SetTextures(wall_textures);

	for (auto i = 0; i < num_floors; i++) {
		auto floor = graph->CreateChildNode<rlr::DrawableNode>(viewport_node, "Instanced floor piece", "basic", false, true, 0);
		floor->SetInstancedPos(fm::vec3(i*24, 0, 0));
		floor->SetTextures(floor_textures);
		floor->model = floor_model;

		floor->Init();

		render_system->UpdateGenericCB(floor, fm::vec3(0, -1, 0), fm::vec3(-1.57079633, 0, 0), fm::vec3(0.25, 0.25, 0.25), true);
	}

	for (std::map<int, rlr::Batch*>::iterator it = render_system->static_instanced_batches.begin(); it != render_system->static_instanced_batches.end(); it++) {
		Create(it->second->instanced_staging_buffer, render_system->device, it->second->inst_positions.data(), it->second->inst_positions.size() * sizeof(fm::vec3), sizeof(fm::vec3), rlr::ResourceState::VERTEX_AND_CONSTANT_BUFFER);
	}

	dr1->model->meshes[0].skeleton.PlayAnimation(dr1->model->animations[0]);

	last_frame = std::chrono::high_resolution_clock::now();

	bool first = true;

	float w = 0.f;
	while (!window->ShouldClose()) {
		w += 0.1;
		window->PollEvents();

		if (first) {
			back_wall->model = floor_model;
			left_wall->model = floor_model;
			right_wall->model = floor_model;

			first = false;

			render_system->UpdateGenericCB(back_wall, fm::vec3(0, 0, 3), fm::vec3(3.14159265, 0, 0), fm::vec3(0.25, 0.25, 0.25), true);
			render_system->UpdateGenericCB(left_wall, fm::vec3(3, 0, 0), fm::vec3(0, -1.57079633, 0), fm::vec3(0.25, 0.25, 0.25), true);
			render_system->UpdateGenericCB(right_wall, fm::vec3(-3, 0, 0), fm::vec3(0, 1.57079633, 0), fm::vec3(0.25, 0.25, 0.25), true);
			render_system->UpdateGenericCB(dr0, fm::vec3(-1, -0.3, 0), fm::vec3(0, -45, 0), fm::vec3(1, 1, 1), true);
		}

		// Calculate Delta time
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = now - last_frame;
		last_frame = now;
		float delta = diff.count();

		dr1->model->meshes[0].skeleton.Update(delta);

		render_system->UpdateGenericCB(dr1, fm::vec3(1, -1, 0), fm::vec3(0, 180, 0), fm::vec3(0.015, 0.015, 0.015), false);

		render_system->Render(graph);
	}

	delete render_system;
	delete window;

	return 0;
}