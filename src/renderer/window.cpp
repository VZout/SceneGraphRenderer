#include "window.h"

#include <glfw/glfw3.h>
#include <unordered_map>

namespace rlr {

Window::Window() {

}

Window::~Window() {
	glfwDestroyWindow(native);
	glfwTerminate();
}

void Window::Create(std::string title, int width, int height, bool fullscreen) {
	if (!glfwInit()) {
		throw std::runtime_error("GLFW initialization failed");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!fullscreen) {
		native = glfwCreateWindow(width,
			height,
			title.c_str(),
			NULL,
			NULL);
	}
	else {
		native = glfwCreateWindow(width,
			height,
			title.c_str(),
			glfwGetPrimaryMonitor(),
			NULL);
	}

	if (!native) {
		glfwTerminate();
		throw std::runtime_error("GLFW window creation failed");
	}
}

void Window::GetSize(int* width, int* height) {
	glfwGetWindowSize(native, width, height);
}

void Window::PollEvents() {
	glfwPollEvents();
}

bool Window::ShouldClose() {
	return glfwWindowShouldClose(native);
}

void Window::Close() {
	glfwSetWindowShouldClose(native, GLFW_TRUE);
}

std::unordered_map<GLFWwindow*, std::function<void(int, int)>> callbacks;
void BindOnResize(Window& window, std::function<void(int width, int height)> callback) {
	callbacks[window.native] = std::move(callback);

	glfwSetWindowSizeCallback(window.native, [](GLFWwindow* window, int width, int height) {
		callbacks[window](width, height);
	});
}

void Window::BindOnResize(std::function<void(int, int)> callback) {
	callbacks[native] = std::move(callback);

	glfwSetWindowSizeCallback(native, [](GLFWwindow* window, int width, int height) {
		callbacks[window](width, height);
	});
}


} /* rlr */
