#include <renderer/render_system.h>
#include <renderer/window.h>

#include <renderer\imgui\imgui.h>
#include <renderer/enums.h>
#include <d3d12.h>
#include <fstream>

rlr::Window* window = nullptr;
rlr::SceneGraph* graph = nullptr;
rlr::RenderSystem* render_system = nullptr;
std::shared_ptr<rlr::DrawableNode> dr0;
std::shared_ptr<rlr::DrawableNode> dr1;
std::shared_ptr<rlr::DrawableNode> back_wall;
std::shared_ptr<rlr::DrawableNode> left_wall;
std::shared_ptr<rlr::DrawableNode> right_wall;
rlr::Texture* spot_albedo = nullptr;
rlr::Texture* spot_spec = nullptr;
rlr::Texture* spot_metal = nullptr;
rlr::Texture* wall_texture = nullptr;
rlr::Texture* wall_texture_specular = nullptr;
rlr::Texture* wall_texture_normal = nullptr;
rlr::Model* floor_model = nullptr;

static int num_floors = 1;

bool first = true;
bool show_engine = true;

std::chrono::time_point<std::chrono::high_resolution_clock> last_frame;

void ImGui_Render()
{
	int width = ImGui::GetIO().DisplaySize.x, height = ImGui::GetIO().DisplaySize.y;
	float offset = 18;
	ImGui::RootDock(ImVec2(0, 18), ImVec2(width, height - 18));

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Toggle Engine", NULL, &show_engine);
			if (!show_engine)
			{
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
	render_system->ImGui_RenderProfiler(first);
	render_system->ImGui_RenderCameraProperties(first);

	first = false;
}

void RegisterPipelines()
{
	CD3DX12_DESCRIPTOR_RANGE  desc_table_ranges;
	desc_table_ranges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

	rlr::RootSignature* rs;
	rlr::RootSignatureCreateInfo rs_info;
	rs_info.samplers.push_back({ rlr::TextureFilter::FILTER_POINT, rlr::TextureAddressMode::TAM_MIRROR });
	rs_info.parameters.resize(3);
	rs_info.parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rs_info.parameters[1].InitAsDescriptorTable(1, &desc_table_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
	rs_info.parameters[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	Create(&rs, rs_info);

	rlr::Shader* pixel_shader; // TODO: These shaders are not being released
	rlr::Shader* vertex_shader;
	Load(&vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/tests/vertex.hlsl");
	Load(&pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/tests/pixel.hlsl");

	rlr::Shader* anim_pixel_shader;
	rlr::Shader* anim_vertex_shader;
	Load(&anim_vertex_shader, rlr::ShaderType::VERTEX_SHADER, "resources/tests/anim_vertex.hlsl");
	Load(&anim_pixel_shader, rlr::ShaderType::PIXEL_SHADER, "resources/tests/anim_pixel.hlsl");

	rlr::PipelineState* ps;
	rlr::Create(&ps);
	rlr::SetVertexShader(ps, vertex_shader);
	rlr::SetFragmentShader(ps, pixel_shader);
	rlr::SetRootSignature(ps, rs);

	rlr::PipelineState* sps;
	rlr::Create(&sps);
	rlr::SetVertexShader(sps, vertex_shader);
	rlr::SetFragmentShader(sps, pixel_shader);
	rlr::SetRootSignature(sps, rs);
	sps->temp = true;

	rlr::PipelineState* anim;
	rlr::Create(&anim);
	rlr::SetVertexShader(anim, anim_vertex_shader);
	rlr::SetFragmentShader(anim, anim_pixel_shader);
	rlr::SetRootSignature(anim, rs);

	rlr::PipelineState* sanim;
	rlr::Create(&sanim);
	rlr::SetVertexShader(sanim, anim_vertex_shader);
	rlr::SetFragmentShader(sanim, anim_pixel_shader);
	rlr::SetRootSignature(sanim, rs);
	sanim->temp = true;

	render_system->RegisterPipeline("basic", ps);
	render_system->RegisterPipeline("basic_shadow", sps);
	render_system->RegisterPipeline("anim", anim);
	render_system->RegisterPipeline("anim_shadow", sanim);
}

void Staging(rlr::Device* device, rlr::CommandList* cmd_list)
{
	Stage(spot_albedo, device, cmd_list);
	Stage(spot_spec, device, cmd_list);
	Stage(spot_metal, device, cmd_list);
	Stage(wall_texture, device, cmd_list);
	Stage(wall_texture_specular, device, cmd_list);
	Stage(wall_texture_normal, device, cmd_list);
	Stage(*dr0->GetModel(), device, cmd_list);
	Stage(*floor_model, device, cmd_list); 
	Stage(*dr1->GetModel(), device, cmd_list);
}

#ifdef _temp
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
#else
int main()
#endif
{
	window = new rlr::Window();
	window->Create("DX12 Renderer test", 1280, 720, false);

	render_system = new rlr::RenderSystem(*window);
	render_system->Setup();

	graph = new rlr::SceneGraph(*render_system, 1280, 720);

	dr0 = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Cow", "basic");
	dr1 = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Dancing Human", "anim", true);
	back_wall = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Back Wall", "basic");
	left_wall = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Left Wall", "basic");
	right_wall = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Right Wall", "basic");

	rlr::Load(&spot_albedo, "resources/tests/spot.png");
	rlr::Load(&spot_spec, "resources/tests/spot_specular.png");
	rlr::Load(&spot_metal, "resources/tests/spot_metal.png");
	rlr::Load(&wall_texture, "resources/tests/agedplanks1-albedo.png");
	rlr::Load(&wall_texture_specular, "resources/tests/agedplanks1-roughness.png");
	rlr::Load(&wall_texture_normal, "resources/tests/agedplanks1-normal4-ue.png");

	floor_model = new rlr::Model();
	rlr::Load(*floor_model, "resources/tests/floor.fbx");

	dr0->SetModel(new rlr::Model());
	dr1->SetModel(new rlr::Model());
	rlr::Load(*dr0->GetModel(), "resources/tests/spot.obj");
	rlr::Load(*dr1->GetModel(), "resources/tests/dance.fbx");

	render_system->RegisterImGuiRenderFunc(ImGui_Render);
	render_system->RegisterStagingFunc(Staging);
	render_system->Init();

	RegisterPipelines();

	window->BindOnResize([](int width, int height)
	{
		graph->ResizeViewport(width, height);
	});

	std::vector<rlr::Texture*> spot_textures = { spot_albedo, spot_spec, spot_albedo, spot_metal};
	std::vector<rlr::Texture*> metal_textures = { wall_texture, wall_texture_specular, wall_texture_normal };
	std::vector<rlr::Texture*> floor_textures = { wall_texture, wall_texture_specular, wall_texture_normal };
	std::vector<rlr::Texture*> wall_textures = { wall_texture, wall_texture_specular };
	dr0->SetTextures(spot_textures);
	back_wall->SetTextures(wall_textures);
	right_wall->SetTextures(metal_textures);
	left_wall->SetTextures(metal_textures);
	dr1->SetTextures(wall_textures);

	// Set the transforms of the enviroment.
	back_wall->GetTransform()->Set(fm::vec3(0, 0, 3), fm::vec3(3.14159265, 0, 0), fm::vec3(0.25, 0.25, 0.25));
	left_wall->GetTransform()->Set(fm::vec3(3, 0, 0), fm::vec3(0, -1.57079633, 0), fm::vec3(0.25, 0.25, 0.25));
	right_wall->GetTransform()->Set(fm::vec3(-3, 0, 0), fm::vec3(0, 1.57079633, 0), fm::vec3(0.25, 0.25, 0.25));

	// Set transform of dancing woman.
	dr1->GetTransform()->SetPosition(fm::vec3(1, -1, 0));
	dr1->GetTransform()->SetRotation(fm::vec3(0, 180, 0));
	dr1->GetTransform()->SetScale(fm::vec3(0.015, 0.015, 0.015));

	// Set transform of cow
	dr0->GetTransform()->SetPosition(fm::vec3(-1, -0.3, 0));
	dr0->GetTransform()->SetRotation(fm::vec3(0, -45, 0));
	dr0->GetTransform()->SetScale(fm::vec3(1, 1, 1));

	for (auto i = 0; i < num_floors; i++)
	{
		auto floor = graph->CreateChildNode<rlr::DrawableNode>(graph->root, "Instanced floor piece", "basic", false, false, true, 0);
		floor->SetInstancedPos(fm::vec3(i*24, 0, 0));
		floor->SetTextures(floor_textures);
		floor->SetModel(floor_model);

		floor->Init();

		floor->GetTransform()->Set(fm::vec3(0, -1, 0), fm::vec3(-1.57079633, 0, 0), fm::vec3(0.25, 0.25, 0.25));
	}

	back_wall->SetModel(floor_model);
	left_wall->SetModel(floor_model);
	right_wall->SetModel(floor_model);

	for (std::map<int, rlr::Batch*>::iterator it = render_system->static_instanced_batches.begin(); it != render_system->static_instanced_batches.end(); it++)
	{
		Create(&it->second->instanced_staging_buffer, render_system->device, it->second->inst_positions.data(), it->second->inst_positions.size() * sizeof(fm::vec3), sizeof(fm::vec3), rlr::ResourceState::VERTEX_AND_CONSTANT_BUFFER);
	}

	dr1->GetModel()->meshes[0].skeleton.PlayAnimation(dr1->GetModel()->animations[0]);

	last_frame = std::chrono::high_resolution_clock::now();

	graph->InitAll();

	while (!window->ShouldClose())
	{
		window->PollEvents();

		// Calculate Delta time
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = now - last_frame;
		last_frame = now;
		float delta = diff.count();

		dr1->GetModel()->meshes[0].skeleton.Update(delta);

		graph->Update();

		render_system->Render(graph);
	}

	delete render_system;
	delete window;

	return 0;
}