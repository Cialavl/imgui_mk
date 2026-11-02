#pragma once
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>
#include <imgui_mk/ui_context.h>
#include <imgui_mk/ui/wg2/font.h>
#include <imgui_mk/ui/wg2/mount.h>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/tree.h>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace Mk {
	// 主窗口组件：以模板组合（成员对象 has-a）方式包装 WindowT（默认 Mk::window），
	// 创建窗口（自动注册到全局引擎）并持有源对象，window.h 的窗口 API 全部转发，
	// 因此 MainWindow 本身即可当作窗口使用。
	// S = 应用共享状态类型（默认 void = 不注册）：构造时创建并注册进引擎，
	// 全部窗口/页面经 state<T>() 取用同一实例（引擎层数据联动，应用不互指窗口）。
	// 组件生命周期必须覆盖 Mk::mkui<WindowT>::run()。
	template <typename WindowT = Mk::window, typename S = void>
	class MainWindow {
	public:
		// 默认尺寸：设计尺寸 × 显示器缩放，再限制到工作区比例（传 0 用默认值）
		MainWindow(const char* title = "mkui", int width = 0, int height = 0)
			: m_Window(title, width > 0 ? width : defaultWidth(), height > 0 ? height : defaultHeight()) {
			applyDefaults();        // 框架默认窗口能力：字号自适应 / 等宽字体 / 主题跟随
			registerStateIfAny();   // S 指定时：创建及注册一步完成
		}
		// 共享状态取用（转发引擎注册表；类型键多槽，未注册抛异常）
		template<typename T>
		T& state() { return Mk::mkui<WindowT>::RegistrationWindow().template state<T>(); }
		template<typename T>
		const T& state() const { return Mk::mkui<WindowT>::RegistrationWindow().template state<T>(); }		// wg2 中央内容挂载：每帧把内容区 bounds 回调给应用层执行 layout()。
		// 视图挂载（Qt setCentralWidget 语义）：窗口每帧自动布局；每帧逻辑用 wg2::Update 控件承载。
		void setCentralWidget(Mk::wg2::StaticWidget auto& root, float padX = 0.0f, float padY = 0.0f) {
			m_Window.setCentralWidget(root, padX, padY);
		}

		// —— 一行挂载复合界面（Qt setCentralWidget 语义）——
		// composite 约定：有 root（控件树根）；可选 setup(ctx) / ready(ctx)。
		// 框架负责：root 的父节点 = 本窗口 → 树的 setup/ready → 中央视图挂载。
		template <class Composite>
		void setContent(Composite& ui) {
			Mk::UiContext& ctx = m_UiContext;
			ctx.window = &m_Window;
			ctx.bus = &Mk::mkui<WindowT>::Events();
			Mk::wg2::attach(ui.root, m_Window);   // 复合界面的父节点 = 本窗口
			if constexpr (requires { ui.setup(ctx); }) ui.setup(ctx);   // 复合界面自己的初始化
			Mk::wg2::setupTree(ui.root, ctx);
			Mk::wg2::readyTree(ui.root, ctx);
			if constexpr (requires { ui.ready(ctx); }) ui.ready(ctx);   // 子树就绪后的处理
			setCentralWidget(ui.root);
		}

		// 跑引擎主循环（窗口已创建、内容已挂载）
		void run() { Mk::mkui<WindowT>::run(); }

		// 显示窗口（转发到内部窗口的 show）
		void show() {
			m_Window.show();
		}

		// 隐藏窗口（转发到内部窗口的 hide）
		void hide() {
			m_Window.hide();
		}

		// 已渲染帧数（1 起）：动画/计数显示直接拉取
		unsigned long long frameIndex() const { return m_Window.frameIndex(); }

		// 窗口状态持久化（转发）
		bool saveLayout(const std::string& path) const { return m_Window.saveLayout(path); }
		bool loadLayout(const std::string& path) { return m_Window.loadLayout(path); }

		// 当前帧中央布局矩形（内容区 − pad 增量；须在帧内调用）
		Mk::wg2::Rect contentRect() const { return m_Window.contentRect(); }

		// 窗口是否可见 / 请求关闭 / X 按钮只隐藏（检查器模式）
		bool visible() const { return m_Window.visible(); }
		void requestClose() { m_Window.requestClose(); }
		void setCloseHides(bool on) { m_Window.setCloseHides(on); }

		// —— window.h API 的模板组合转发 ——
		// 视图挂载与每帧回调统一走 setCentralWidget：
		//   setCentralWidget(root)                   静态树直挂（窗口每帧自动布局）
		//   setCentralWidget([](const Rect& r){...}) 回调式（回调里先跑逻辑再 layout）


		// 子窗口挂载到本窗口：父关随关、子 X 隐藏（子窗口生命周期随父）
		void mountChild(Mk::window& child) { m_Window.mountChild(child); }

		// 切换窗口主题（转发到内部窗口的 setTheme）
		void setTheme(const Mk::wg2::Theme& theme) {
			Mk::mkui<Mk::window>::Events().emit(Mk::ThemeChanged{theme});   // 框架统一广播
			m_Window.setTheme(theme);
		}

		// —— 字号自适应（全量转发 window.h API：应用层无需 getWindow() 绕行）——
		void setFontLoader(std::function<bool(float)> loader) { m_Window.setFontLoader(std::move(loader)); }
		void addAuxFont(float scale, ImFont** out = nullptr) { m_Window.addAuxFont(scale, out); }
		ImFont* auxFont(std::size_t index = 0) const { return m_Window.auxFont(index); }
		void setAutoScaleFont(bool on) { m_Window.setAutoScaleFont(on); }
		void rescale(float fontSize) { m_Window.rescale(fontSize); }
		float fontSize() const { return m_Window.fontSize(); }

		// —— 注册式每帧机制（转发）——
		void addShortcut(ImGuiKey key, std::function<void()> fn,
			bool ctrl = false, bool shift = false, bool alt = false) {
			m_Window.addShortcut(key, std::move(fn), ctrl, shift, alt);
		}
		template<typename Dlg>
		void watchFileDialog(Dlg& dlg, std::function<void(std::string)> onDone) {
			m_Window.watchFileDialog(dlg, std::move(onDone));
		}

		// GLFW 窗口句柄（关闭后为 nullptr）
		GLFWwindow* getHandle() const {
			return m_Window.getHandle();
		}

		// 手动渲染一帧（引擎 run() 会逐帧调用，一般无需手动）
		void render() {
			m_Window.render();
		}

		// 是否已请求关闭 / 已关闭
		bool shouldClose() const {
			return m_Window.shouldClose();
		}

		// 关闭窗口：真正销毁 GLFW 窗口与 ImGui 上下文
		void close() {
			m_Window.close();
		}

		// 底层窗口对象：需要直接访问 WindowT 时使用（run() 之后仍可使用）
		WindowT& getWindow() {
			return m_Window;
		}
		const WindowT& getWindow() const {
			return m_Window;
		}

	private:
		// 默认窗口尺寸：设计尺寸 × 显示器缩放，再限制到工作区比例
		static int defaultWidth() {
			const float dpi = Mk::wg2::dpiScale();
			const auto [w, h] = Mk::workAreaSize(0.75f, 0.8f,
				static_cast<int>(1440.0f * dpi), static_cast<int>(880.0f * dpi));
			(void)h;
			return w;
		}
		static int defaultHeight() {
			const float dpi = Mk::wg2::dpiScale();
			const auto [w, h] = Mk::workAreaSize(0.75f, 0.8f,
				static_cast<int>(1440.0f * dpi), static_cast<int>(880.0f * dpi));
			(void)w;
			return h;
		}

		// 框架默认窗口能力（应用可随后覆盖）
		void applyDefaults() {
			m_Window.setAutoScaleFont(true);
			m_Window.addAuxFont(0.85f);   // 等宽代码字体（×0.85 随字号联动）
			Mk::wg2::setMonoFontProvider([this] { return m_Window.auxFont(); });
			Mk::mkui<WindowT>::Events().on<Mk::ThemeChanged>([this](const Mk::ThemeChanged& e) {
				m_Window.setTheme(e.theme);
			});
		}

		void registerStateIfAny() {
			if constexpr (!std::is_void_v<S>) {
				m_State = std::make_shared<S>();
				Mk::mkui<WindowT>::RegistrationWindow().registerState(m_State);
			}
		}

		WindowT m_Window;
		std::shared_ptr<S> m_State;   // S=void 时为空，不参与状态注册
		Mk::UiContext m_UiContext;    // 控件树上下文（setContent 时填充，随窗口存活）
	};

	// ============ 框架主窗口（进程内一份，启动即由框架创建并配置）============
	// 首次调用创建：标题/尺寸缺省时给默认值（设计尺寸 × 显示器缩放，限制到工作区比例），
	// 并应用框架默认窗口能力：字号自适应、等宽字体（引擎 monoFont 的 provider）、主题跟随。
	// 之后任何组件都从同一个入口取，不需要各自持有主窗口。
	template <typename WindowT = Mk::window>
	MainWindow<WindowT>& mainWindow(const char* title = nullptr, int width = 0, int height = 0) {
		static std::unique_ptr<MainWindow<WindowT>> window;
		if (!window) {
			window = std::make_unique<MainWindow<WindowT>>(title ? title : "mkui", width, height);
		}
		return *window;
	}

	// 启动：框架主窗口一行挂载复合界面 + 跑引擎主循环
	template <class Composite, typename WindowT = Mk::window>
	void runUi(Composite& ui) {
		auto& main = mainWindow<WindowT>();
		main.setContent(ui);
		Mk::mkui<WindowT>::run();
	}

}

