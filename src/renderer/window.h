#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace rlr {

class  Window {
public:
	Window();
	~Window();

	void Create(std::string title, int width, int height, bool fullscreen);
	void GetSize(int* width, int* height);
	void PollEvents();
	bool ShouldClose();
	void Close();
	void BindOnResize(std::function<void(int, int)> func);

	GLFWwindow* native;
};

} /* rlr */
