#pragma once
#define USE_RENDERER_OPENGL
// #define USE_RENDERER_DX
// #define USE_RENDERER_VULKAN

#ifdef USE_RENDERER_OPENGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#ifdef USE_RENDERER_VULKAN
#endif

#ifdef USE_RENDERER_DX
#endif

#include <concepts>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <iostream>

namespace Mk {

	
	struct opengl {};
	struct dx {};
	struct vulkan {};
	template <typename Renderer, typename... AppArgs> struct Rendererengine {};

	template <typename... N>
	struct Rendererengine<opengl, N...> {
		static GLFWwindow* init(const char* title, int width, int height) {
			if(!glfwInit()) {
				std::cerr << "Failed to initialize GLFW" << std::endl;
				return nullptr;
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
			if(!handle) {
				std::cerr << "Failed to create GLFW window" << std::endl;
				glfwTerminate();
				return nullptr;
			}

			glfwMakeContextCurrent(handle);
			if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
				std::cerr << "Failed to initialize GLAD" << std::endl;
				glfwDestroyWindow(handle);
				glfwTerminate();
				return nullptr;
			}
			return handle;
		}
	};

} 
