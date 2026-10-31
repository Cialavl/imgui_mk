#pragma once
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>
#include <imgui_mk/ui/wg2/style.h>
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
		MainWindow(const char* title, int width = 1600, int height = 900)
			: MainWindow(title, width, height, nullptr) {
		}

		// 创建窗口并注入每帧 UI 回调
		MainWindow(const char* title, int width, int height, std::function<void()> ui)
			: m_Window(title, width, height) {
			if(ui) {
				m_Window.setUserUI(std::move(ui));
			}
			registerStateIfAny();   // S 指定时：创建及注册一步完成
		}

		// 共享状态取用（转发引擎注册表；类型键多槽，未注册抛异常）
		template<typename T>
		T& state() { return Mk::mkui<WindowT>::RegistrationWindow().template state<T>(); }
		template<typename T>
		const T& state() const { return Mk::mkui<WindowT>::RegistrationWindow().template state<T>(); }		// wg2 中央内容挂载：每帧把内容区 bounds 回调给应用层执行 layout()。
		template<class F>
void setCentralWidget(F&& fn,
				float padX = 0.0f, float padY = 0.0f) {
				m_Window.setCentralWidget(std::forward<F>(fn), padX, padY);
	}
		// —— 主窗口特定功能 ——

		// 显示窗口（转发到内部窗口的 show）
		void show() {
			m_Window.show();
		}

		// 隐藏窗口（转发到内部窗口的 hide）
		void hide() {
			m_Window.hide();
		}

		// 当前帧中央布局矩形（内容区 − pad 增量；须在帧内调用）
		Mk::wg2::Rect contentRect() const { return m_Window.contentRect(); }

		// 窗口是否可见 / 请求关闭 / X 按钮只隐藏（检查器模式）
		bool visible() const { return m_Window.visible(); }
		void requestClose() { m_Window.requestClose(); }
		void setCloseHides(bool on) { m_Window.setCloseHides(on); }

		// —— window.h API 的模板组合转发 ——

		// 注入每帧 UI 回调（在 run() 之前调用；转发到内部窗口的 setUserUI）
		// 统一 UI 入口：无参回调 = 普通每帧 UI；接收 Rect 的回调 = 中央布局 UI；
		// 接收 ImGuiIO& 的回调 = 每帧 UI（框架注入 io，省去手写 ImGui::GetIO()）。
template<class F>
void setUI(F&& ui) { m_Window.setUI(std::forward<F>(ui)); }
template<auto UI>
void setUI() { m_Window.setUI<UI>(); }

// window.h 同名别名
		template<class F>
void setUserUI(F&& ui) { m_Window.setUserUI(std::forward<F>(ui)); }

		// 子窗口挂载到本窗口：父关随关、子 X 隐藏（子窗口生命周期随父）
		void mountChild(Mk::window& child) { m_Window.mountChild(child); }

		// 切换窗口主题（转发到内部窗口的 setTheme）
		void setTheme(const Mk::wg2::Theme& theme) {
			m_Window.setTheme(theme);
		}

		// —— 字号自适应（全量转发 window.h API：应用层无需 getWindow() 绕行）——
		void setFontLoader(std::function<bool(float)> loader) { m_Window.setFontLoader(std::move(loader)); }
		void addAuxFont(float scale, ImFont** out = nullptr) { m_Window.addAuxFont(scale, out); }
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
		void registerStateIfAny() {
			if constexpr (!std::is_void_v<S>) {
				m_State = std::make_shared<S>();
				Mk::mkui<WindowT>::RegistrationWindow().registerState(m_State);
			}
		}

		WindowT m_Window;
		std::shared_ptr<S> m_State;   // S=void 时为空，不参与状态注册
	};

}

