#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/core.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <type_traits>
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

			// 中央内容挂载（Qt setCentralWidget 语义）：窗口每帧自动对其做根布局
						std::function<void(const wg2::Rect&)> m_CentralFn;   // wg2 回调挂载
		void* m_CentralRoot = nullptr;                   // 直挂控件标记（幂等判重：帧内重复挂载安全）
			float m_CentralPadX = 0.0f, m_CentralPadY = 0.0f;
			// 注册式每帧机制（引擎在 user UI 之前统一分发）
			struct Shortcut {
				ImGuiKey key;
				bool ctrl = false, shift = false, alt = false;
				std::function<void()> fn;
			};
			std::vector<Shortcut> m_Shortcuts;               // 快捷键轮询（addShortcut 注册）
			std::vector<std::function<void()>> m_FileWatchers;   // 文件对话框托管（watchFileDialog 注册，每帧 poll）
			struct AuxFont { float scale; ImFont** out; ImFont* baked = nullptr; };   // 追加字体（重烘焙后写回 baked 与 out）
			std::vector<AuxFont> m_AuxFonts;
			std::vector<std::shared_ptr<Impl>> m_Children;  // 挂载的子窗口（mountChild；父关随关，生命周期随父）

			// —— 运行时字号自适应状态（见 rescale/processAdaptiveScaling）——
			wg2::Theme m_Theme;                       // 当前已应用主题（字号保持实际烘焙值）
			float m_BaseFontSize = 0.0f;              // 设计字号（构造字号 / 构造时窗口 contentScale）
			float m_ContentScale = 1.0f;              // 当前窗口所在显示器缩放（跨屏时由回调更新）
			float m_RefW = 0.0f, m_RefH = 0.0f;       // 构造时窗口尺寸：尺寸比例基准（该尺寸下比例 = 1.0）
			float m_LastBakedSize = 0.0f;             // 实际烘焙字号（重烘焙比对基准）
			float m_PendingScale = 0.0f;              // >0：contentScale 回调置位，帧内安全处理
			float m_PendingRescale = -1.0f;             // >0：请求重烘焙（帧内安全时机执行）
			int m_LastW = 0, m_LastH = 0;             // 上一帧窗口尺寸（判断是否仍在拖动）
			double m_LastSizeChangeT = 0.0;           // 最近一次尺寸变化时刻（glfwGetTime 秒）
			bool m_ForceRescale = false;              // 跨屏 DPI 变化：跳过稳定等待立即重烘焙
			bool m_AutoScaleFont = false;             // 自适应总开关（跨屏 DPI + 尺寸比例）
			wg2::Theme m_PendingTheme;                  // 待应用主题（引擎广播 requestTheme 置位）
			bool m_ThemePending = false;
			unsigned long long m_FrameIndex = 0;   // 已渲染帧数（1 起）
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

			// 追加字体（等宽编程字体）：优先系统常见编程字体，找不到则退回内置点阵字体。
			// 再合并「常用汉字」子集——Consolas 之类没有 CJK，代码里写中文注释时不会出现 '?'。
			static ImFont* loadMonoFont(float px) {
				ImGuiIO& io = ImGui::GetIO();
				static const char* kMonoFonts[] = {
					"C:\\Windows\\Fonts\\consola.ttf",   // Consolas（VS / Windows 终端默认）
					"C:\\Windows\\Fonts\\CascadiaMono.ttf",
					"C:\\Windows\\Fonts\\CascadiaCode.ttf",
					"C:\\Windows\\Fonts\\lucon.ttf",     // Lucida Console
					"C:\\Windows\\Fonts\\cour.ttf",      // Courier New
				};
				auto exists = [](const char* path) {
					std::FILE* f = std::fopen(path, "rb");
					if(!f) return false;
					std::fclose(f);
					return true;
				};
				ImFont* mono = nullptr;
				for(const char* path : kMonoFonts) {
					if(!exists(path)) continue;
					mono = io.Fonts->AddFontFromFileTTF(path, px);
					if(mono) break;
				}
				if(!mono) {   // 兜底：内置点阵字体
					ImFontConfig cfg;
					cfg.SizePixels = px;
					return io.Fonts->AddFontDefault(&cfg);
				}
				static const char* kCjkFonts[] = {
					"C:\\Windows\\Fonts\\msyh.ttc",
					"C:\\Windows\\Fonts\\simhei.ttf",
				};
				for(const char* path : kCjkFonts) {
					if(!exists(path)) continue;
					ImFontConfig merge;
					merge.MergeMode = true;
					if(io.Fonts->AddFontFromFileTTF(path, px, &merge,
						   io.Fonts->GetGlyphRangesChineseSimplifiedCommon())) break;
				}
				return mono;
			}

			// 请求重烘焙：任意时机可调（含 UI 回调内），实际执行延迟到下一帧
			// render 顶部（NewFrame 之前）——回调内立即销毁字体纹理会让本帧
			// RenderDrawData 引用已删除的 GL 纹理
			void requestRescale(float fontSize) {
				if(fontSize <= 0.0f) return;
				m_PendingRescale = fontSize;
			}

			// 请求换肤（引擎 applyThemeAll 广播用）：延迟到本窗口自己帧内应用，
			// 避免在别的窗口帧内切换本窗口上下文（跨窗口安全）
			void requestTheme(const wg2::Theme& t) {
				m_PendingTheme = t;
				m_ThemePending = true;
			}

			// 重烘焙字体图集并按新字号重应用主题（几何随字号自动等比）。
			// 须在 GL 上下文 + ImGui 上下文均为本窗口时调用（render 顶部满足）。
			// 注意：全量 CJK 图集重烘焙有约 0.3~0.5s 一次性卡顿（防抖 + 5% 滞回已把频率降到最低）
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
				for(auto& aux : m_AuxFonts) {   // 追加字体：等宽编程字体（含常用汉字合并），句柄写回
					if(ImFont* f = loadMonoFont(fontSize * aux.scale)) {
						aux.baked = f;
						if(aux.out) *aux.out = f;
					}
				}
				// 销毁旧字体纹理：GL 后端在下一次 NewFrame 检测到空纹理会自动重建
				ImGui_ImplOpenGL3_DestroyFontsTexture();
				m_Theme.fontSize = fontSize;
				m_LastBakedSize = fontSize;
				m_PendingRescale = -1.0f;
				wg2::applyTheme(m_ImGuiContext, m_Theme);
			}

			// 帧内自适应处理（render 顶部，上下文已安全切到本窗口）：
			//   B 跨屏/系统 DPI：回调置 m_PendingScale → 窗口尺寸按比例同步 + 立即重烘焙（不等稳定）
			//   C 尺寸比例：比例 = min(宽/构造宽, 高/构造高)——取小者，宽而矮的窗口不会把字号放大到溢出；
			//      目标字号 = 设计字号 × 比例 × contentScale。
			//      重烘焙代价高（全量 CJK 图集 0.3~0.5s），因此：拖动中不重烘焙（尺寸稳定 200ms 才允许），
			//      且变化 < 5% 不重烘焙（滞回），避免边界抖动导致反复重建图集。
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
					m_ForceRescale = true;
				}
				if(!m_AutoScaleFont) return;

				int w = 0, h = 0;
				glfwGetWindowSize(m_Handle, &w, &h);
				if(w != m_LastW || h != m_LastH) {
					m_LastW = w;
					m_LastH = h;
					m_LastSizeChangeT = glfwGetTime();
				}

				const float ratioW = m_RefW > 0.0f ? static_cast<float>(w) / m_RefW : 1.0f;
				const float ratioH = m_RefH > 0.0f ? static_cast<float>(h) / m_RefH : 1.0f;
				const float ratio = std::clamp(std::min(ratioW, ratioH), 0.70f, 1.60f);
				const float target = m_BaseFontSize * ratio * m_ContentScale;

				const bool settled = (glfwGetTime() - m_LastSizeChangeT) > 0.20;
				const bool changed = m_LastBakedSize <= 0.0f
					|| std::fabs(target - m_LastBakedSize) > m_LastBakedSize * 0.05f;
				if(settled || m_ForceRescale) {
					if(changed) doRescale(target);
					m_ForceRescale = false;
				}
			}

			// 真正关闭窗口：销毁 GLFW 窗口并清理 ImGui 后端/上下文。
			// 与 shared_ptr 引用计数无关——引擎发现窗口关闭、或调用方主动 close()，
			// 都会立即销毁窗口状态，所有共享句柄同步失效（getHandle() 返回 nullptr）。
			void close() {
				// 子窗口跟随关闭：置 shouldClose 后由引擎循环在各自迭代执行有序清理
				//（不在本窗口 close 内嵌套销毁，保持各自上下文独立的关闭顺序）
				for(auto& c : m_Children) {
					if(c && c->m_Handle) glfwSetWindowShouldClose(c->m_Handle, true);
				}
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
				if(m_ThemePending) {   // 引擎广播的延迟换肤：本帧上下文即本窗口，安全应用
					m_Theme = m_PendingTheme;
					m_Theme.fontSize = m_LastBakedSize;
					wg2::applyTheme(m_ImGuiContext, m_Theme);
					m_ThemePending = false;
				}

				int display_w = 0, display_h = 0;
				glfwGetFramebufferSize(m_Handle, &display_w, &display_h);
				glViewport(0, 0, display_w > 0 ? display_w : 1, display_h > 0 ? display_h : 1);

				glClearColor(0.11f, 0.11f, 0.11f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				++m_FrameIndex;
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

				// 注册式机制分发：快捷键轮询 + 文件对话框 poll（user UI 之前，状态变更当帧可见）
				{
					ImGuiIO& io = ImGui::GetIO();
					for(const auto& sc : m_Shortcuts) {
						if(sc.ctrl == io.KeyCtrl && sc.shift == io.KeyShift && sc.alt == io.KeyAlt
						   && ImGui::IsKeyPressed(sc.key, false)) sc.fn();
					}
				}
				for(const auto& watch : m_FileWatchers) watch();



				// 布局由窗口自动驱动（控件树与窗口交互；应用侧零布局调用）
				// 外边距语义：默认 = 窗口样式 WindowPadding（主题/字号联动）；padX/padY 为增量（Qt setCentralWidget）
				if(m_CentralFn) {
const ImVec2 avail = ImGui::GetContentRegionAvail();
const ImVec2 wp = ImGui::GetStyle().WindowPadding;
const float x = wp.x + m_CentralPadX;
const float y = wp.y + m_CentralPadY;
const float w = std::max(0.0f, avail.x - m_CentralPadX * 2.0f);
const float h = std::max(0.0f, avail.y - m_CentralPadY * 2.0f);
m_CentralFn(wg2::Rect{x, y, w, h});
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
			impl.m_Theme = wg2::defaultTheme();
			const wg2::Theme& theme = impl.m_Theme;

			// 按主题字号加载默认字体链（中文全量字形，见 defaultFontLoader）
			Impl::defaultFontLoader(theme.fontSize);

			// 自适应状态初始化：
			//   设计字号 = 构造字号 / 本窗口所在显示器的 contentScale（剥离 scaledForMonitor 预乘）
			//   比例基准 = 构造时的窗口尺寸（此后按尺寸比例无级缩放，构造尺寸下比例恒为 1.0）
			{
				int w0 = 0, h0 = 0;
				glfwGetWindowSize(impl.m_Handle, &w0, &h0);
				impl.m_RefW = w0 > 0 ? static_cast<float>(w0) : 1.0f;
				impl.m_RefH = h0 > 0 ? static_cast<float>(h0) : 1.0f;
				float sx = 0.0f, sy = 0.0f;
				glfwGetWindowContentScale(impl.m_Handle, &sx, &sy);
				impl.m_ContentScale = (sx > 0.0f && sy > 0.0f) ? (sx + sy) * 0.5f : wg2::dpiScale();
			}
			impl.m_BaseFontSize = theme.fontSize / impl.m_ContentScale;
			impl.m_LastBakedSize = theme.fontSize;
			impl.m_LastW = static_cast<int>(impl.m_RefW);
			impl.m_LastH = static_cast<int>(impl.m_RefH);

			// 应用默认主题（几何随 fontSize 等比），可用 setTheme 换肤
			wg2::applyTheme(impl.m_ImGuiContext, theme);

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


	// 视图挂载（Qt setCentralWidget 语义）：窗口每帧自动 wg2::layout(root, 内容区)，应用零回调。
	// 每帧逻辑不暴露为窗口回调：用 wg2::Update<Id> 控件放进树里（不占空间，draw 时执行）。
	// 同 root 重复挂载幂等（帧内回调中调用安全）；root 须与 run() 同寿（对象地址 = 稳定 ImGui ID）。
	template<class W>
	requires wg2::StaticWidget<std::remove_cvref_t<W>>
	void setCentralWidget(W& root, float padX = 0.0f, float padY = 0.0f) {
		if(m_Impl) {
			void* p = static_cast<void*>(&root);
			if(m_Impl->m_CentralRoot == p) return;   // 幂等：同 root 重复挂载直接跳过
			m_Impl->m_CentralRoot = p;
			m_Impl->m_CentralPadX = padX;
			m_Impl->m_CentralPadY = padY;
			m_Impl->m_CentralFn = [&root](const wg2::Rect& r) { wg2::layout(root, r); };
		}
	}

	// 当前帧中央布局矩形（内容区 − pad 增量；须在帧内调用，手动布局应用按需取）
	wg2::Rect contentRect() const {
		if(!m_Impl) return {};
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const ImVec2 wp = ImGui::GetStyle().WindowPadding;
		return wg2::Rect{
			wp.x + m_Impl->m_CentralPadX, wp.y + m_Impl->m_CentralPadY,
			std::max(0.0f, avail.x - m_Impl->m_CentralPadX * 2.0f),
			std::max(0.0f, avail.y - m_Impl->m_CentralPadY * 2.0f)};
	}


// 切换本窗口主题（每窗口独立 ImGui 上下文，可各持一套皮肤）。
		// 换肤不换字号：字体位图不重建，几何按当前烘焙字号推导
		void setTheme(const wg2::Theme& theme) {
			if(m_Impl && m_Impl->m_Handle) {
				m_Impl->m_Theme = theme;
				m_Impl->m_Theme.fontSize = m_Impl->m_LastBakedSize;
				wg2::applyTheme(m_Impl->m_ImGuiContext, m_Impl->m_Theme);
			}
		}

		// 当前烘焙字号（自适应比例/重缩放后的实际值）；除以设计字号即当前 UI 缩放
		float fontSize() const { return m_Impl && m_Impl->m_Handle ? m_Impl->m_LastBakedSize : 0.0f; }

		// 已渲染帧数（1 起）：动画/计数类显示直接拉取，无需外部计数器
		unsigned long long frameIndex() const { return m_Impl ? m_Impl->m_FrameIndex : 0; }

		// —— 运行时字号自适应 API ——

		// 手动重烘焙：按新字号重建字体图集并重应用主题（几何等比联动）。
		// 任意时机可调（含 UI 回调内），下一帧生效
		void rescale(float fontSize) {
			if(m_Impl) {
				m_Impl->requestRescale(fontSize);
			}
		}

		// 自适应总开关（默认关，测试保持确定性）：开启后——
		//   跨屏拖动/系统缩放变化时字号与窗口尺寸同步重标定（立即重烘焙）；
		//   窗口尺寸变化时按「比例 = min(宽/构造宽, 高/构造高)」无级缩放字号（0.70~1.60），
		//   拖动中不重烘焙、尺寸稳定 200ms 且变化 >5% 才重建图集（见 processAdaptiveScaling）
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

		// 追加辅助字体（声明式传参）：默认中文链（或自定义装载器）之后按 scale×基准字号装载，
		// 每次重烘焙后把句柄写回 out（可为空）；注册即请求重烘焙（同 setFontLoader 语义）
		// 查询已烘焙的辅助字体（index 按 addAuxFont 注册顺序；未烘焙返回 nullptr）
		ImFont* auxFont(std::size_t index = 0) const {
			if(!m_Impl || index >= m_Impl->m_AuxFonts.size()) return nullptr;
			return m_Impl->m_AuxFonts[index].baked;
		}

		void addAuxFont(float scale, ImFont** out = nullptr) {
			if(m_Impl) {
				m_Impl->m_AuxFonts.push_back({scale, out});
				m_Impl->requestRescale(m_Impl->m_LastBakedSize);
			}
		}

		// 注册式快捷键：引擎每帧轮询（修饰键精确匹配，触发即回调；须在 run() 前注册）
		void addShortcut(ImGuiKey key, std::function<void()> fn,
			bool ctrl = false, bool shift = false, bool alt = false) {
			if(m_Impl) m_Impl->m_Shortcuts.push_back({key, ctrl, shift, alt, std::move(fn)});
		}

		// 文件对话框托管：引擎每帧 poll（dlg 须与 run() 同寿），完成即回调结果路径
		template<typename Dlg>
		void watchFileDialog(Dlg& dlg, std::function<void(std::string)> onDone) {
			if(m_Impl) m_Impl->m_FileWatchers.push_back([&dlg, fn = std::move(onDone)]() mutable {
				std::string path;
				if(dlg.poll(path)) fn(std::move(path));
			});
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

		// 窗口当前是否可见（隐藏后 show() 可恢复）
		bool visible() const {
			if(auto* h = getHandle()) return glfwGetWindowAttrib(h, GLFW_VISIBLE) != 0;
			return false;
		}

		// 请求关闭（不立即销毁）：下一轮引擎循环才真正关闭并清理，帧内调用安全
		void requestClose() {
			if(auto* h = getHandle()) {
				glfwSetWindowShouldClose(h, true);
			}
		}

		// X 按钮语义：true = 只隐藏（检查器/否决模式），false = 默认请求关闭
		void setCloseHides(bool on) {
			if(auto* h = getHandle()) {
				if(on) {
					glfwSetWindowCloseCallback(h, [](GLFWwindow* w) {
						glfwSetWindowShouldClose(w, GLFW_FALSE);   // 否决：不进引擎销毁流程
						glfwHideWindow(w);                          // 只是隐藏
					});
				} else {
					glfwSetWindowCloseCallback(h, nullptr);        // 恢复默认：X = 请求关闭
				}
			}
		}

		// 子窗口挂载到父窗口：父窗口关闭 → 子窗口跟随关闭；子窗口点 X = 隐藏（生命周期随父）。
		// 父窗口持有子窗口的共享引用——应用侧的窗口对象可以是局部变量
		void mountChild(window& child) {
			if(m_Impl && child.m_Impl && m_Impl != child.m_Impl) {
				m_Impl->m_Children.push_back(child.m_Impl);
				if(GLFWwindow* ch = child.getHandle()) {
					glfwSetWindowCloseCallback(ch, [](GLFWwindow* w) {
						glfwSetWindowShouldClose(w, GLFW_FALSE);
						glfwHideWindow(w);
					});
				}
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
