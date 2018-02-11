#pragma once

static int num_pipeline_changes = 0;

#define NUM_LIGHTS 10 // todo: REMOVE THIS
#define KERNEL_SIZE 32

#define SHADOW_SIZE 2048 // todo: remove this

#define IMGUI_RENDER_FUNC_PARAMS bool dock, bool use_last_as_dest, ImGui::DockStyle style
#define IMGUI_RENDER_FUNC_DEFAULT_PARAMS bool dock = false, bool use_last_as_dest = true, ImGui::DockStyle style = ImGui::DockStyle::CENTER

// Buffer structs
namespace rlr
{

struct PVCBStruct
	{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
};

struct CBStruct
{
	std::array<aiMatrix4x4, 100> weightmatrices;
	DirectX::XMFLOAT4X4 model;
	float instanced = 0;
	float shita = 0;
	float shitb = 1;
	float shitc = 2;
};

struct CBCompo
{
	float exposure;
	float gamma;
	float contrast;
	float chroma;
	int tonemapping = 0;
	int bloom;
};

struct Light
{
	fm::vec4 pos = fm::vec4(0, 0, 0);
	fm::vec4 color = fm::vec4(0, 0, 0);
	float constant = 1.f;
	float lin = 0.09f;
	float quadratic = 0.2f;
	float radius = 0;
};

struct CBSceneStruct
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
	DirectX::XMFLOAT4X4 shadow_view;
	DirectX::XMFLOAT4X4 shadow_proj;
	std::array<Light, NUM_LIGHTS> lights;
	unsigned int num_lights;
};

struct CBSSAOStruct
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
	std::array<fm::vec4, KERNEL_SIZE> samples;
};

}
