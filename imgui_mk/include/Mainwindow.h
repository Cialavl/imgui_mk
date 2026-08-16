#pragma once
#include <imgui_mk/window.h>
#include <memory>

namespace Mk {
	// 主窗口组件：主窗口是特化的窗口，只负责创建窗口（自动注册到全局引擎）并持有源对象，
	// 不含 UI（需要 UI 时用 Mk::window::setUserUI() 注入）。
	// 组件生命周期必须覆盖 Mk::mkui<WindowT>::run()。
	template <typename WindowT>
	class MainWindow {
	public:
		MainWindow(const char* title, int width = 1600, int height = 900)
			: m_Window(std::make_unique<WindowT>(title, width, height)) {
		}

		// 窗口源对象：run() 之后仍可使用
		WindowT& getWindow() const {
			return *m_Window;
		}

	private:
		std::unique_ptr<WindowT> m_Window;
	};

}