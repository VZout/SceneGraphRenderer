#include "render_system.h"

#include <locale>
#include <codecvt>

namespace rlr {

float profiler_multiply_result = 1;

void RenderSystem::ImGui_RenderMainMenuBarMenu() {
	if (ImGui::BeginMenu("Graphics")) {
		ImGui::MenuItem("Game View", NULL, &imgui_show_game_view);
		ImGui::MenuItem("Performance Overlay", NULL, &imgui_show_perf_overlay);
		ImGui::MenuItem("CPU/GPU Profiler", NULL, &imgui_show_profiler);
		ImGui::MenuItem("Light Properties", NULL, &imgui_show_light_properties);
		ImGui::MenuItem("Camera(s) Properties", NULL, &imgui_show_cam_properties);
		ImGui::EndMenu();
	}
}

void RenderSystem::ImGui_DrawRect(ImDrawList* dl, profiler::Precision time, profiler::Precision frame, bool align, bool solid_color, profiler::Precision* time_offset) {
	ImVec2 wpos = ImGui::GetCursorScreenPos();

	if (frame == 0)
		return;

	float width = 0;
	float height = 16;

	width = (ImGui::GetContentRegionAvailWidth()) * (time / frame);

	float offset = align && time_offset ? *time_offset : 0;
	if (time_offset && align) {
		*time_offset += width;
	}

	ImVec2 left = ImVec2(wpos.x + offset, wpos.y);
	ImVec2 right = ImVec2(wpos.x + offset + width, left.y + height);

	ImVec2 bg_left = ImVec2(wpos.x, wpos.y);
	ImVec2 bg_right = ImVec2(wpos.x + ImGui::GetContentRegionAvailWidth(), bg_left.y + height);

	ImColor color(ImVec4(1.0f, 0.5f, 0, 1.0f));
	ImColor bg_color(ImVec4(1, 1, 1, 0.2f));
	if (!solid_color) {
		float blend_val = time / frame;
		float r = blend_val;
		float g = (blend_val - 1) * -1;
		float b = (blend_val - 1) * -1;
		color = ImVec4(r, g, b, 1.0f);
	}

	dl->AddRectFilled(bg_left, bg_right, bg_color);
	dl->AddRectFilled(left, right, color);
}

void RenderSystem::ImGui_DrawBar(std::shared_ptr<profiler::Node> node, profiler::Precision entire_frame, bool align, bool solid_color, int idx, profiler::Precision* time_offset) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	profiler::Precision diff = (node->end - node->start) * profiler_multiply_result;

	bool node_open = false;
	if (!node->childs.empty()) {
		node_open = ImGui::TreeNode(node->name.c_str());
	}
	else {
		ImGui::Text(node->name.c_str());
	}

	ImGui::NextColumn();

	std::string diff_str = std::to_string(diff);
	ImGui::Text(diff_str.c_str());

	ImGui::NextColumn();

	ImGui_DrawRect(draw_list, diff, entire_frame, align, solid_color, time_offset);

	ImGui::NextColumn();

	if (node_open) {
		profiler::Precision offset = 0;

		for (auto i = 0; i < node->childs.size(); i++) {
			auto child = node->childs[i];
			ImGui_DrawBar(child, diff, align, solid_color, i, &offset);
			offset += diff;
		}

		ImGui::TreePop();
	}
}

void RenderSystem::ImGui_RenderGameView(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("Game View", &imgui_show_game_view)) {
		ImGui::SetWindowFontScale(1);
		ImGui::Checkbox("Constrain Aspect Ratio", &imgui_game_view_constrain_aspect_ratio);
		ImGui::SameLine();
		ImGui::DragFloat("Aspect Ratio", &imgui_game_view_aspect_ratio);
		ImVec2 size = ImGui::GetContentRegionAvail();

		ImVec2 cursor_pos = ImGui::GetCursorPos();
		cursor_pos.x += 5;
		cursor_pos.y += 5;

		ImGui::Image(GetGameViewRenderTarget(), size);
		
		if (!imgui_game_view_constrain_aspect_ratio) {
			cam->SetAspectRatio(ImGui::GetItemRectSize().x / ImGui::GetItemRectSize().y);
		}
		else {
			cam->SetAspectRatio(imgui_game_view_aspect_ratio);
		}

		if (imgui_show_perf_overlay) {
			ImGui::SetCursorPos(cursor_pos);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(0, 0, 0, 140));
			ImGui::BeginChildFrame(6969, ImVec2(80, 20));
			ImGui::Text("FPS: %d", fps);
			ImGui::EndChildFrame();
			ImGui::PopStyleColor();
		}
	}
	if (dock && imgui_show_light_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderProfiler(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("CPU/GPU Profiler", &imgui_show_profiler)) {
		ImGui::Checkbox("Align bars", &imgui_profiler_align);
		ImGui::SameLine();
		ImGui::Checkbox("Solid Colors", &imgui_profiler_solid_colors);
		ImGui::SameLine();
		ImGui::Checkbox("Use target fps", &imgui_profiler_use_target_fps);
		ImGui::SameLine();
		ImGui::DragFloat("Target fps", &imgui_profiler_target_fps);

		ImGui::Text("Pipeline State Changes: %d", num_pipeline_changes);

		ImGui::Separator();

		ImGui::Columns(3, NULL, true);
		ImGui::SetColumnWidth(1, 70);

		for (auto root_node : profiler::prev_root_nodes) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			profiler::Precision diff = (root_node->end - root_node->start) * profiler_multiply_result;
			ImGui_DrawBar(root_node, imgui_profiler_use_target_fps ? (1 / imgui_profiler_target_fps) : diff, imgui_profiler_align, imgui_profiler_solid_colors);
		}

		ImGui::Separator();

		for (auto root_node : profiler::prev_root_gpu_nodes) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			profiler::Precision diff = (root_node->end - root_node->start) * profiler_multiply_result;
			ImGui_DrawBar(root_node, imgui_profiler_use_target_fps ? (1 / imgui_profiler_target_fps) : diff, imgui_profiler_align, imgui_profiler_solid_colors);

		}
		ImGui::Columns(1);
	}
	if (dock && imgui_show_profiler) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderLightProperties(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("Light Properties", &imgui_show_light_properties)) {
		for (auto i = 0; i < lights.size(); i++) {
			std::string name = std::to_string(i);
			std::string posn = name + " pos";
			std::string colorn = name + " color";
			std::string constn = name + " constant";
			std::string linn = name + " linear";
			std::string quadn = name + " quadratic";
			std::string radn = name + " radius";
			ImGui::DragFloat3(posn.c_str(), lights[i].pos.data, 0.1); 
			ImGui::ColorEdit3(colorn.c_str(), lights[i].color.data, 0.1);
			ImGui::DragFloat(constn.c_str(), &lights[i].constant, 0.01);
			ImGui::DragFloat(linn.c_str(), &lights[i].lin, 0.01);
			ImGui::DragFloat(quadn.c_str(), &lights[i].quadratic);
			ImGui::DragFloat(radn.c_str(), &lights[i].radius);
			ImGui::Separator();

			float lightMax = std::fmaxf(std::fmaxf(lights[i].color.x, lights[i].color.y), lights[i].color.z);
			//lights[i].radius = (-lights[i].lin + std::sqrtf(lights[i].lin * lights[i].lin - 4 * lights[i].quadratic * (lights[i].constant - (256.0 / 5.0) * lightMax))) / (2 * lights[i].quadratic);
		}
	}
	if (dock && imgui_show_light_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderCameraProperties(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("Camera(s) Properties", &imgui_show_cam_properties)) {
		fm::vec3 cpos = cam->GetPos();
		fm::vec3 ceuler = cam->GetEuler();
		ImGui::DragFloat3("cam pos", cpos.data, 0.1);
		ImGui::DragFloat3("cam euler", ceuler.data, 0.1);
		cam->SetPos(cpos);
		cam->SetEuler(ceuler);
		cam->Update();

		ImGui::Separator();

		ImGui::DragFloat3("shadow cam offset pos", shadow_pos_offset.data, 0.1);
		ImGui::DragFloat3("shadow cam rotation", shadow_rot.data, 0.1);
	}
	if (dock && imgui_show_cam_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderPerformanceOverlay() {
	if (imgui_show_perf_overlay) { // FIXME: Why do I need to do this? Bug?
		if (ImGui::Begin("Performance Overlay", &imgui_show_perf_overlay, ImVec2(80, 20), 0.5, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::Text("FPS: %d", fps);
		}
		ImGui::End();
	}
}

void RenderSystem::ImGui_RenderHardwareDetails(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("Hardware Details", &imgui_show_cam_properties)) {
		if (ImGui::CollapsingHeader("System Information", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Page Size: %i", device.sys_info.dwPageSize);
			ImGui::Text("Application Address Range: %i - %i", device.sys_info.lpMinimumApplicationAddress, device.sys_info.lpMaximumApplicationAddress);
			ImGui::Text("Active Processor Mask: %i", device.sys_info.dwActiveProcessorMask);
			ImGui::Text("Processor Count: %i", device.sys_info.dwNumberOfProcessors);

			switch (device.sys_info.wProcessorArchitecture) {
			case 9: ImGui::Text("Processor Architecture: %s", "PROCESSOR_ARCHITECTURE_AMD64"); break;
			case 5: ImGui::Text("Processor Architecture: %s", "PROCESSOR_ARCHITECTURE_ARM"); break;
			case 6: ImGui::Text("Processor Architecture: %s", "PROCESSOR_ARCHITECTURE_IA64"); break;
			case 0: ImGui::Text("Processor Architecture: %s", "PROCESSOR_ARCHITECTURE_INTEL"); break;
			default: ImGui::Text("Processor Architecture: %s", "Unknown"); break;
			}
		}

		if (ImGui::CollapsingHeader("Graphics Adapter Information", ImGuiTreeNodeFlags_DefaultOpen)) {
			std::wstring wdesc(device.adapter_info.Description);
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;
			std::string desc = converter.to_bytes(wdesc);

			ImGui::Text("Description: %s", desc.c_str());
			ImGui::Text("Vendor ID: %i", device.adapter_info.VendorId);
			ImGui::Text("Device ID: %i", device.adapter_info.DeviceId);
			ImGui::Text("Subsystem ID: %i", device.adapter_info.SubSysId);
			ImGui::Text("Dedicated Video Memory: %i", device.adapter_info.DedicatedVideoMemory);
			ImGui::Text("Dedicated System Memory: %i", device.adapter_info.DedicatedSystemMemory);
			ImGui::Text("Shared System Memory: %i", device.adapter_info.SharedSystemMemory);
		}
	}
	if (dock && imgui_show_cam_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderPostProcessingSettings(IMGUI_RENDER_FUNC_PARAMS) {
	if (ImGui::BeginDock("Postprocessing Settings", &imgui_show_postprocessing_settings)) {
		const char* tonemapping_options[9] = {
			"Linear",
			"Simple Reinhard",
			"Luma Based Reinhard",
			"White Preserving Luma Based Reinhard",
			"RomBinDaHouse",
			"Filmic",
			"Uncharted 2",
			"Grayscale",
			"All Tonampping Algorithms",
		};
		ImGui::Combo("Tonemapping", &imgui_tonemapping_id, tonemapping_options, 9);

		ImGui::DragFloat("Gamma", &imgui_gamma, 0.05);
		ImGui::DragFloat("Exposure", &imgui_exposure, 0.05);
		ImGui::DragFloat("Contrast", &imgui_contrast, 0.05);
		ImGui::Checkbox("Bloom", &imgui_bloom);
	}
	if (dock && imgui_show_cam_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

void RenderSystem::ImGui_RenderRenderGraph(IMGUI_RENDER_FUNC_PARAMS, SceneGraph* graph) {
	if (ImGui::BeginDock("Render Graph", &imgui_show_render_graph)) {
		using recursive_func_t = std::function<void(std::shared_ptr<Node>)>;

		ImGui::Text("Node count: %f", 66.f);

		recursive_func_t recursive_graph = [&](std::shared_ptr<Node> node) {
			bool node_open = false;
			if (!node->GetChildren().empty()) {
				node_open = ImGui::TreeNode(node->GetName().c_str());
			}
			else {
				ImGui::Text(node->GetName().c_str());
			}

			for (auto child : node->GetChildren()) {
				if (node_open)  {
					recursive_graph(child);
				}
			}

			if (node_open) {
				ImGui::TreePop();
			}
		};

		recursive_graph(graph->root);

	}
	if (dock && imgui_show_cam_properties) ImGui::LetsDock(use_last_as_dest, style);
	ImGui::EndDock();
}

} /* rlr */
