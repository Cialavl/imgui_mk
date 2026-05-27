#pragma once
#include <imgui_mk/windowRendereren.h>
#include <memory>

namespace Mk {

	class window {
	public:
		window(const char* title, int width, int height) : m_Handle(nullptr), m_ImGuiContext(nullptr) {

		

			m_Handle = Rendererengine<opengl, int>::init(title, width, height);
			glfwSetWindowUserPointer(m_Handle, this);
			glfwMakeContextCurrent(m_Handle);

			IMGUI_CHECKVERSION();
			m_ImGuiContext = ImGui::CreateContext();
			ImGui::SetCurrentContext(m_ImGuiContext);

			// 修改全局样式，去掉内边距和边框
			ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
			ImGui::GetStyle().WindowBorderSize = 0.0f;
			ImGui::GetStyle().WindowRounding = 0.0f;

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
		}

		GLFWwindow* getHandle() const {
			return m_Handle;
		}

		void render() {
			if(!m_Handle) return;

			glfwMakeContextCurrent(m_Handle);
			ImGui::SetCurrentContext(m_ImGuiContext);

			int display_w = 0, display_h = 0;
			glfwGetFramebufferSize(m_Handle, &display_w, &display_h);
			glViewport(0, 0, display_w > 0 ? display_w : 1, display_h > 0 ? display_h : 1);

			glClearColor(0.11f, 0.11f, 0.11f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGuiViewport* viewport = ImGui::GetMainViewport();

			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);

			ImGuiWindowFlags window_flags =
				ImGuiWindowFlags_NoTitleBar |        // 无标题栏
				ImGuiWindowFlags_NoResize |          // 不可改变大小
				ImGuiWindowFlags_NoMove |            // 不可移动
				ImGuiWindowFlags_NoCollapse |        // 不可折叠
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoNavFocus;

			ImGui::Begin("MainFullWindow", nullptr, window_flags);



			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(m_Handle);
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
		ImGuiContext* m_ImGuiContext;

		static void MouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if(!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
		}
		static void ScrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if(!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
		}
		static void KeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if(!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
		}
		static void CharCallback(GLFWwindow* win, unsigned int c) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if(!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_CharCallback(win, c);
		}
		static void CursorPosCallback(GLFWwindow* win, double x, double y) {
			auto w = static_cast<Mk::window*>(glfwGetWindowUserPointer(win));
			if(!w || !w->m_ImGuiContext) return;
			ImGui::SetCurrentContext(w->m_ImGuiContext);
			ImGui_ImplGlfw_CursorPosCallback(win, x, y);
		}
	};
} // namespace Mk