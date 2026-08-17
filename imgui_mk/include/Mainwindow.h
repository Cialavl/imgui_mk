#pragma once
#include <imgui_mk/window.h>
#include <functional>
#include <utility>

namespace Mk {
	// 主窗口组件：以模板组合（成员对象 has-a）方式包装 WindowT（默认 Mk::window），
	// 创建窗口（自动注册到全局引擎）并持有源对象，window.h 的窗口 API 全部转发，
	// 因此 MainWindow 本身即可当作窗口使用。
	// 组件生命周期必须覆盖 Mk::mkui<WindowT>::run()。
	template <typename WindowT = Mk::window>
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

		// —— window.h API 的模板组合转发 ——

		// 注入每帧 UI 回调（在 run() 之前调用；转发到内部窗口的 setUserUI）
		void setUI(std::function<void()> ui) {
			m_Window.setUserUI(std::move(ui));
		}

		// 挂载控件树（转发到内部窗口的 setWidget）
		void setWidget(ui::Widget& root) {
			m_Window.setWidget(root);
		}

		// 切换窗口主题（转发到内部窗口的 setTheme）
		void setTheme(const fui::Theme& theme) {
			m_Window.setTheme(theme);
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
		WindowT m_Window;
	};

}
