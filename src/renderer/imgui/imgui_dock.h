#pragma once


namespace Lumix { namespace FS { class OsFile; } }


struct lua_State;


namespace ImGui
{

	enum class DockStyle {
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		CENTER,
	};

IMGUI_API void ShutdownDock();
IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);
IMGUI_API bool BeginDock(const char* label, bool* opened = nullptr, ImGuiWindowFlags extra_flags = 0, const ImVec2& default_size = ImVec2(600, 400));
IMGUI_API void EndDock();
IMGUI_API void SetDockActive();
IMGUI_API void LetsDock(bool use_last_as_dest, DockStyle style);
//IMGUI_API void SaveDock(Lumix::FS::OsFile& file);
//IMGUI_API void LoadDock(lua_State* L);


} // namespace ImGui
