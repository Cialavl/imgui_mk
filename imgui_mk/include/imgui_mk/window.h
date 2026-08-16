#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/mkui.h>
#include <functional>
#include <memory>

namespace Mk {

	class window {
	public:
		// 窗口实际状态（GLFW 句柄 + ImGui 上下文）。window 是共享句柄：
		// 引擎与调用方共同持有 Impl，任一先释放状态仍存活，全部释放后才销毁。
		struct Impl {
			GLFWwindow* m_Handle = nullptr;
			ImGuiContext* m_ImGuiContext = nullptr;
			// 每帧 UI 回调（组件注入窗口内容，如主窗口组件）
			std::function<void()> m_UserUI;

			~Impl() {
				if(m_Handle) {
					glfwDestroyWindow(m_Handle);
					m_Handle = nullptr;
				}
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

				if(m_UserUI) {
					m_UserUI();
				}

				ImGui::End();

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
				glfwSwapBuffers(m_Handle);
			}

			bool shouldClose() const {
				return glfwWindowShouldClose(m_Handle);
			}

			static void MouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_ImGuiContext) return;
				ImGui::SetCurrentContext(impl->m_ImGuiContext);
				ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
			}
			static void ScrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_ImGuiContext) return;
				ImGui::SetCurrentContext(impl->m_ImGuiContext);
				ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
			}
			static void KeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_ImGuiContext) return;
				ImGui::SetCurrentContext(impl->m_ImGuiContext);
				ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
			}
			static void CharCallback(GLFWwindow* win, unsigned int c) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_ImGuiContext) return;
				ImGui::SetCurrentContext(impl->m_ImGuiContext);
				ImGui_ImplGlfw_CharCallback(win, c);
			}
			static void CursorPosCallback(GLFWwindow* win, double x, double y) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_ImGuiContext) return;
				ImGui::SetCurrentContext(impl->m_ImGuiContext);
				ImGui_ImplGlfw_CursorPosCallback(win, x, y);
			}
		};

		window(const char* title, int width, int height) : m_Impl(std::make_shared<Impl>()) {
			Impl& impl = *m_Impl;

			impl.m_Handle = Rendererengine<opengl, int>::init(title, width, height);
			glfwSetWindowUserPointer(impl.m_Handle, &impl);
			glfwMakeContextCurrent(impl.m_Handle);

			IMGUI_CHECKVERSION();
			impl.m_ImGuiContext = ImGui::CreateContext();
			ImGui::SetCurrentContext(impl.m_ImGuiContext);

			// 修改全局样式，去掉内边距和边框
			ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
			ImGui::GetStyle().WindowBorderSize = 0.0f;
			ImGui::GetStyle().WindowRounding = 0.0f;

			ImGui::StyleColorsDark();
			ImGuiIO& io = ImGui::GetIO();

			// 初始化ImGui的OpenGL3后端
			ImGui_ImplGlfw_InitForOpenGL(impl.m_Handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");

			glfwSetMouseButtonCallback(impl.m_Handle, Impl::MouseButtonCallback);
			glfwSetScrollCallback(impl.m_Handle, Impl::ScrollCallback);
			glfwSetKeyCallback(impl.m_Handle, Impl::KeyCallback);
			glfwSetCharCallback(impl.m_Handle, Impl::CharCallback);
			glfwSetCursorPosCallback(impl.m_Handle, Impl::CursorPosCallback);

			// 创建窗口后自动注册到全局引擎（引擎与调用方共享窗口状态）
			Mk::mkui<window>::RegistrationWindow().Registration(m_Impl);
		}

		// 源对象仍然可用：所有操作委托给共享状态

		// 注入每帧 UI 回调（回调由调用方保证在窗口存活期间有效）
		void setUserUI(std::function<void()> ui) {
			if(m_Impl) {
				m_Impl->m_UserUI = std::move(ui);
			}
		}

		GLFWwindow* getHandle() const {
			return m_Impl ? m_Impl->m_Handle : nullptr;
		}

		void render() {
			if(m_Impl) {
				m_Impl->render();
			}
		}

		bool shouldClose() const {
			return m_Impl && m_Impl->shouldClose();
		}

	private:
		std::shared_ptr<Impl> m_Impl;
	};
} // namespace Mk