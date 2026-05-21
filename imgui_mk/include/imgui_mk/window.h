#pragma once
#include <imgui_mk/windowRendereren.h>
#include <memory>

namespace Mk {

	class window {
	public:
		 window(const char* title, int width, int height) : m_Handle(nullptr),m_ImGuiContext(nullptr) {
		
			m_Handle = Rendererengine<opengl,int>::init(title, width, height);
			glfwSetWindowUserPointer(m_Handle, this);
			glfwMakeContextCurrent(m_Handle);
			IMGUI_CHECKVERSION();
			m_ImGuiContext = ImGui::CreateContext();
			ImGui::SetCurrentContext(m_ImGuiContext);
			ImGui::StyleColorsDark();
			ImGuiIO& io = ImGui::GetIO();
		
			// 初始化ImGui的OpenGL3后端
			ImGui_ImplGlfw_InitForOpenGL(m_Handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");

			glfwSetMouseButtonCallback(m_Handle, MouseButtonCallback);
			glfwSetScrollCallback(m_Handle, ScrollCallback);
			glfwSetKeyCallback(m_Handle, KeyCallback);
			glfwSetCharCallback(m_Handle, CharCallback);
			glfwSetCursorPosCallback(m_Handle, CursorPosCallback);

			// 设置ImGui样式(可选)
			ImGui::StyleColorsDark();
		}

		GLFWwindow* getHandle() const {
			return m_Handle;
		}
		void render() {
			if (!m_Handle) return;

			// 确保当前上下文与 ImGui 上下文
			glfwMakeContextCurrent(m_Handle);
			ImGui::SetCurrentContext(m_ImGuiContext);

			// 更新视口到当前帧缓冲大小（必要时窗口被缩放）
			int display_w = 0, display_h = 0;
			glfwGetFramebufferSize(m_Handle, &display_w, &display_h);
			glViewport(0, 0, display_w > 0 ? display_w : 1, display_h > 0 ? display_h : 1);

			// 每帧清除颜色缓冲，避免上一帧残影（重影）
			glClearColor(0.11f, 0.11f, 0.11f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			// 推荐的新帧调用顺序
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();
			ImGui::Text("Hello, world %d", 123);
			ImGui::Text("Hello, world %d", 123);
			ImGui::Text("Hello, world %d", 123);
			ImGui::Text("Hello, world %d", 123);
			ImGui::Text("Hello, world %d", 123);
			ImGui::Text("Hello, world %d", 123);
			ImGui::Button("encode");
			// 结束ImGui帧并渲染
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(m_Handle);
		}
		void show() {
		
			
		}

		bool shouldClose() const {
		
			return glfwWindowShouldClose(m_Handle);
		}

		~window() {
			if(m_Handle) {
				glfwDestroyWindow(m_Handle);
				m_Handle = nullptr;
			}
		}

	private:
		GLFWwindow* m_Handle;
		bool isshow=false;
		ImGuiContext* m_ImGuiContext;

	
		static void MouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if (!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
		}
		static void ScrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if (!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
		}
		static void KeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if (!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
		}
		static void CharCallback(GLFWwindow* win, unsigned int c) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if (!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_CharCallback(win, c);
		}
		static void CursorPosCallback(GLFWwindow* win, double x, double y) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if (!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_CursorPosCallback(win, x, y);
		}
	};
} // namespace Mk