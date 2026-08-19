#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/ui/wg_style.h>
#include <imgui_mk/ui/wg.h>   // Node（setCentralWidget 挂载）
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

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
			// 中央内容挂载（Qt setCentralWidget 语义）：窗口每帧自动对其做根布局
			wg::Node* m_Central = nullptr;
			float m_CentralPadX = 0.0f, m_CentralPadY = 0.0f;

			// —— 运行时字号自适应状态（见 rescale/processAdaptiveScaling）——
			wg::Theme m_Theme;                       // 当前已应用主题（字号保持实际烘焙值）
			float m_BaseFontSize = 0.0f;              // 逻辑基准字号（构造字号 / 构造时 contentScale）
			float m_ContentScale = 1.0f;              // 当前显示器缩放（跨屏时由回调更新）
			float m_LastBakedSize = 0.0f;             // 实际烘焙字号（档位比对基准）
			float m_PendingScale = 0.0f;              // >0：contentScale 回调置位，帧内安全处理
			float m_PendingRescale = -1.0f;             // >0：请求重烘焙（帧内安全时机执行）
			bool m_AutoScaleFont = false;             // 自适应总开关（B 跨屏 DPI + C 宽度档位）
			std::function<bool(float)> m_FontLoader;  // 字号 → 装入字体图集；空 = 默认 CJK 链

			~Impl() {
				close();
			}

			// 默认字体链：按字号加载中文（雅黑→黑体→宋体），全量 CJK 字形。
			// ImGui 默认字体仅含 ASCII，中文会渲染成“?”；全部缺失时退回默认字体
			static bool defaultFontLoader(float fontSize) {
				ImGuiIO& io = ImGui::GetIO();
				static const char* cjkFonts[] = {
					"C:\\Windows\\Fonts\\msyh.ttc",   // 微软雅黑
					"C:\\Windows\\Fonts\\simhei.ttf", // 黑体
					"C:\\Windows\\Fonts\\simsun.ttc", // 宋体
				};
				for(const char* path : cjkFonts) {
					if(io.Fonts->AddFontFromFileTTF(path, fontSize, nullptr,
						   io.Fonts->GetGlyphRangesChineseFull())) {
						return true;
					}
				}
				return false;
			}

			// 请求重烘焙：任意时机可调（含 UI 回调内），实际执行延迟到下一帧
			// render 顶部（NewFrame 之前）——回调内立即销毁字体纹理会让本帧
			// RenderDrawData 引用已删除的 GL 纹理
			void requestRescale(float fontSize) {
				if(fontSize <= 0.0f) return;
				m_PendingRescale = fontSize;
			}

			// 重烘焙字体图集并按新字号重应用主题（几何随字号自动等比）。
			// 须在 GL 上下文 + ImGui 上下文均为本窗口时调用（render 顶部满足）。
			// 注意：全量 CJK 图集重烘焙有约 0.3~0.5s 一次性卡顿（量化档位已把频率降到最低）
			void doRescale(float fontSize) {
				if(!m_Handle || !m_ImGuiContext) return;
				glfwMakeContextCurrent(m_Handle);
				ImGui::SetCurrentContext(m_ImGuiContext);
				// 最小化守卫：帧缓冲 0 尺寸时跳过整帧（0 尺寸布局会破坏 ImGui 窗口/ID 栈）
				int fbw = 0, fbh = 0;
				glfwGetFramebufferSize(m_Handle, &fbw, &fbh);
				if(fbw <= 0 || fbh <= 0) return;
				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->Clear();   // 全量重置（含 ConfigData）：ClearFonts 不清配置，重复 Add 会触发 ImGui 断言
				if(m_FontLoader) m_FontLoader(fontSize);
				else defaultFontLoader(fontSize);
				// 销毁旧字体纹理：GL 后端在下一次 NewFrame 检测到空纹理会自动重建
				ImGui_ImplOpenGL3_DestroyFontsTexture();
				m_Theme.fontSize = fontSize;
				m_LastBakedSize = fontSize;
				m_PendingRescale = -1.0f;
				wg::applyTheme(m_ImGuiContext, m_Theme);
			}

			// 帧内自适应处理（render 顶部，上下文已安全切到本窗口）：
			//   B 跨屏/系统 DPI：回调置 m_PendingScale → 窗口尺寸同步缩放 + 触发重烘焙
			//   C 宽度档位：按窗口宽量化（<900:0.75 <1400:1.0 <2000:1.25 其余:1.5），
			//      目标字号 = 基准 × 档位 × contentScale，与烘焙值不符才重烘焙（防抖动）
			void processAdaptiveScaling() {
				if(m_PendingRescale > 0.0f) {
					doRescale(m_PendingRescale);
				}
				if(m_PendingScale > 0.0f) {
					const float k = m_PendingScale / m_ContentScale;
					m_ContentScale = m_PendingScale;
					m_PendingScale = 0.0f;
					int w = 0, h = 0;
					glfwGetWindowSize(m_Handle, &w, &h);
					glfwSetWindowSize(m_Handle, (int)(w * k), (int)(h * k));
				}
				if(!m_AutoScaleFont) return;
				int w = 0, h = 0;
				glfwGetWindowSize(m_Handle, &w, &h);
				const float bucket =
					w < 900 ? 0.75f : w < 1400 ? 1.0f : w < 2000 ? 1.25f : 1.5f;
				const float target = m_BaseFontSize * bucket * m_ContentScale;
				if(target != m_LastBakedSize) {
					doRescale(target);
				}
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

				// 最小化守卫：帧缓冲 0 尺寸时跳过整帧（0 尺寸布局会破坏 ImGui 窗口/ID 栈）
				int fbw0 = 0, fbh0 = 0;
				glfwGetFramebufferSize(m_Handle, &fbw0, &fbh0);
				if(fbw0 <= 0 || fbh0 <= 0) return;
				processAdaptiveScaling();   // B/C 字号自适应（重烘焙须在 NewFrame 前）

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
					ImGuiWindowFlags_NoScrollbar |         // 全屏画布：自适应布局恒等于窗口尺寸，禁滚动
					ImGuiWindowFlags_NoScrollWithMouse |
					ImGuiWindowFlags_NoNavFocus;

				ImGui::Begin("MainFullWindow", nullptr, window_flags);

				if(m_UserUI) {
					m_UserUI();
				}


				// 布局由窗口自动驱动（控件树与窗口交互；应用侧零布局调用）
				// 外边距语义：默认 = 窗口样式 WindowPadding（主题/字号联动）；padX/padY 为增量（Qt setCentralWidget）
				if(m_Central) {
					const ImVec2 avail = ImGui::GetContentRegionAvail();
					const ImVec2 wp = ImGui::GetStyle().WindowPadding;
					m_Central->layout(wg::Rect{
						wp.x + m_CentralPadX, wp.y + m_CentralPadY,
						std::max(0.0f, avail.x - m_CentralPadX * 2.0f),
						std::max(0.0f, avail.y - m_CentralPadY * 2.0f)});
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
			// 跨屏拖动/系统缩放变化：只记录新比例（回调线程/时机不安全，
			// 不碰 GL 与 ImGui），下一帧 render 顶部统一处理
			static void ContentScaleCallback(GLFWwindow* win, float xscale, float) {
				auto impl = static_cast<Impl*>(glfwGetWindowUserPointer(win));
				if(!impl || !impl->m_AutoScaleFont || xscale <= 0.0f) return;
				impl->m_PendingScale = xscale;
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

			// 全局默认主题（setDefaultTheme 须在首个窗口创建前调用；此处读取烘焙）
			impl.m_Theme = wg::defaultTheme();
			const wg::Theme& theme = impl.m_Theme;

			// 按主题字号加载默认字体链（中文全量字形，见 defaultFontLoader）
			Impl::defaultFontLoader(theme.fontSize);

			// 自适应状态初始化：逻辑基准 = 构造字号 / 构造时显示器缩放
			//（scaledForMonitor 预乘过的字号在此被剥离，档位计算不会双重缩放）
			impl.m_ContentScale = wg::dpiScale();
			impl.m_BaseFontSize = theme.fontSize / impl.m_ContentScale;
			impl.m_LastBakedSize = theme.fontSize;

			// 应用默认主题（几何随 fontSize 等比），可用 setTheme 换肤
			wg::applyTheme(impl.m_ImGuiContext, theme);

			// 初始化ImGui的OpenGL3后端
			ImGui_ImplGlfw_InitForOpenGL(impl.m_Handle, true);
			ImGui_ImplOpenGL3_Init("#version 130");

			glfwSetMouseButtonCallback(impl.m_Handle, Impl::MouseButtonCallback);
			glfwSetScrollCallback(impl.m_Handle, Impl::ScrollCallback);
			glfwSetKeyCallback(impl.m_Handle, Impl::KeyCallback);
			glfwSetCharCallback(impl.m_Handle, Impl::CharCallback);
			glfwSetCursorPosCallback(impl.m_Handle, Impl::CursorPosCallback);
		glfwSetWindowContentScaleCallback(impl.m_Handle, Impl::ContentScaleCallback);

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

		// 挂载中央内容（Qt setCentralWidget 语义）：窗口每帧自动对其根布局（setUserUI 回调之后执行），
		// 布局由控件树与窗口交互自动完成；padX/padY 为外边距增量（默认 = 样式 WindowPadding）。
		void setCentralWidget(wg::Node& root, float padX = 0.0f, float padY = 0.0f) {
			if(m_Impl) {
				m_Impl->m_Central = &root;
				m_Impl->m_CentralPadX = padX;
				m_Impl->m_CentralPadY = padY;
			}
		}

		// 切换本窗口主题（每窗口独立 ImGui 上下文，可各持一套皮肤）。
		// 换肤不换字号：字体位图不重建，几何按当前烘焙字号推导
		void setTheme(const wg::Theme& theme) {
			if(m_Impl && m_Impl->m_Handle) {
				m_Impl->m_Theme = theme;
				m_Impl->m_Theme.fontSize = m_Impl->m_LastBakedSize;
				wg::applyTheme(m_Impl->m_ImGuiContext, m_Impl->m_Theme);
			}
		}

		// 当前烘焙字号（自适应档位/重缩放后的实际值）
		float fontSize() const { return m_Impl && m_Impl->m_Handle ? m_Impl->m_LastBakedSize : 0.0f; }

		// —— 运行时字号自适应 API ——

		// 手动重烘焙：按新字号重建字体图集并重应用主题（几何等比联动）。
		// 任意时机可调（含 UI 回调内），下一帧生效
		void rescale(float fontSize) {
			if(m_Impl) {
				m_Impl->requestRescale(fontSize);
			}
		}

		// 自适应总开关（默认关，测试保持确定性）：开启后——
		//   跨屏拖动/系统缩放变化时字号与窗口尺寸同步重标定；
		//   窗口宽度跨过档位阈值（900/1400/2000）时字号量化升降
		void setAutoScaleFont(bool on) {
			if(m_Impl) {
				m_Impl->m_AutoScaleFont = on;
			}
		}

		// 自定义字体装入链（字号 → 装入图集，返回是否成功）。
		// 设置后立即按当前字号重烘焙一次（替换初始默认链）。
		// loader 须在调用方保持有效；后续所有 rescale 都走它
		void setFontLoader(std::function<bool(float)> loader) {
			if(m_Impl && loader) {
				m_Impl->m_FontLoader = std::move(loader);
				m_Impl->requestRescale(m_Impl->m_LastBakedSize);
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

	// 窗口默认尺寸 = 主显示器工作区百分比（上限 maxW×maxH）：
	// 小屏不超出工作区、大屏饱满；取不到显示器信息时退回上限值
	inline std::pair<int, int> workAreaSize(float fracW, float fracH, int maxW, int maxH) {
		GLFWmonitor* m = glfwGetPrimaryMonitor();
		if(!m) return {maxW, maxH};
		int wx = 0, wy = 0, ww = 0, wh = 0;
		glfwGetMonitorWorkarea(m, &wx, &wy, &ww, &wh);
		if(ww <= 0 || wh <= 0) return {maxW, maxH};
		return {std::min(maxW, (int)(ww * fracW)), std::min(maxH, (int)(wh * fracH))};
	}
} // namespace Mk
