#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui_style.h>
#include <functional>
#include <memory>

namespace Mk {

	class window {
	public:
		// 窗口实际状态（GLFW 句柄 + ImGui 上下文）。window 是共享句柄：
		// 引擎与调用方共同持有 Impl；close() 会立即销毁 GLFW 窗口与 ImGui 上下文，
		// 但 Impl 对象本身存活到最后一个 shared_ptr 释放（释放时再次 close() 为空操作）。
		struct Impl {
			GLFWwindow* m_Handle = nullptr;
			ImGuiContext* m_ImGuiContext = nullptr;
			// 每帧 UI 回调（组件注入窗口内容，如主窗口组件）
			std::function<void()> m_UserUI;

			~Impl() {
				close();
			}

			// 真正关闭窗口：销毁 GLFW 窗口并清理 ImGui 后端/上下文。
			// 与 shared_ptr 引用计数无关——引擎发现窗口关闭、或调用方主动 close()，
			// 都会立即销毁窗口状态，所有共享句柄同步失效（getHandle() 返回 nullptr）。
			void close() {
				if(!m_Handle) return;

				glfwMakeContextCurrent(m_Handle);
				if(m_ImGuiContext) {
					ImGui::SetCurrentContext(m_ImGuiContext);

					// 清理顺序很关键：ImGui 的 GLFW 后端在 Windows 上把 WndProc 钩子装到每个窗口上，
					// 钩子会读取“当前上下文”的后端数据（GetBackendData），且对空指针无保护。
					// glfwDestroyWindow 期间系统会向仍存活的窗口同步发送激活等消息，
					// 因此 GLFW 后端必须存活到 DestroyWindow 之后，否则钩子拿到已释放的 bd → 空指针崩溃。
					ImGui_ImplOpenGL3_Shutdown();              // 1. 渲染后端（需要当前 GL 上下文）
					ImGui_ImplGlfw_RestoreCallbacks(m_Handle); // 2. 窗口尚存活时恢复 GLFW 回调，避免 Shutdown 访问已销毁窗口
					glfwMakeContextCurrent(nullptr);           // 3. 解除当前 GL 上下文，再销毁窗口
					glfwDestroyWindow(m_Handle);               // 4. 销毁窗口（此期间后端数据仍有效，钩子安全）
					m_Handle = nullptr;
					ImGui_ImplGlfw_Shutdown();                 // 5. 平台后端收尾（窗口已销毁，钩子清理失败无害）
					ImGui::DestroyContext(m_ImGuiContext);
					m_ImGuiContext = nullptr;
				}
				else {
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

			// 无句柄（已关闭）或收到关闭请求都视为应移出引擎
			bool shouldClose() const {
				return !m_Handle || glfwWindowShouldClose(m_Handle);
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

			// 加载中文字体：ImGui 默认字体仅含 ASCII，中文会渲染成“?”。
			// 依次尝试常见 Windows 中文字体（微软雅黑/黑体/宋体），成功一个即止；
			// 全部缺失时退回默认字体（英文不受影响）
			{
				ImGuiIO& io = ImGui::GetIO();
				static const char* cjkFonts[] = {
					"C:\\Windows\\Fonts\\msyh.ttc",   // 微软雅黑
					"C:\\Windows\\Fonts\\simhei.ttf", // 黑体
					"C:\\Windows\\Fonts\\simsun.ttc", // 宋体
				};
				for(const char* path : cjkFonts) {
					if(io.Fonts->AddFontFromFileTTF(
						   path, 18.0f, nullptr,
						   io.Fonts->GetGlyphRangesChineseFull())) {
						break;
					}
				}
			}

			// 应用默认主题（深色：内边距/圆角/控件间距/配色），可用 setTheme 换肤
			fui::applyTheme(impl.m_ImGuiContext, fui::Theme::dark());
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

		// 挂载控件树：每帧渲染 root 及其整棵子树（setUserUI 的控件树便捷形式）
		void setWidget(ui::Widget& root) {
			if(m_Impl) {
				m_Impl->m_UserUI = [&root]() { root.draw(); };
			}
		}

		// 切换本窗口主题（每窗口独立 ImGui 上下文，可各持一套皮肤）
		void setTheme(const fui::Theme& theme) {
			if(m_Impl && m_Impl->m_Handle) {
				fui::applyTheme(m_Impl->m_ImGuiContext, theme);
			}
		}

		// 显示窗口（GLFW 创建即可见；窗口被隐藏后可重新显示）
		void show() {
			if(auto* h = getHandle()) {
				glfwShowWindow(h);
			}
		}

		// 隐藏窗口（show() 可重新显示）
		void hide() {
			if(auto* h = getHandle()) {
				glfwHideWindow(h);
			}
		}

		// 关闭窗口：真正销毁 GLFW 窗口与 ImGui 上下文，所有共享句柄同步失效
		void close() {
			if(m_Impl) {
				m_Impl->close();
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