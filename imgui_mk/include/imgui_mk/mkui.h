#pragma once
#include <memory>
#include <vector>
#include <imgui_mk/windowRendereren.h>

namespace Mk {

	// 全局引擎模板：按窗口类型实例化，Mk::mkui<WindowT>::RegistrationWindow() 返回引擎单例，
	// 创建 WindowT 窗口时自动注册进该引擎，Mk::mkui<WindowT>::run() 运行引擎直到所有窗口关闭。
	template <typename WindowT>
	struct mkui {
		using ImplT = typename WindowT::Impl;

		static mkui& RegistrationWindow() {
			// 引擎单例（每种窗口类型一个，Meyers singleton）
			static mkui engine;
			return engine;
		}
		static void run() {
			auto& engine = RegistrationWindow();
			while (!engine.m_Windows.empty()) {
				glfwPollEvents();
				for (auto it = engine.m_Windows.begin(); it != engine.m_Windows.end(); ) {
					if ((*it)->shouldClose()) {
						// 真正关闭：销毁 GLFW 窗口与 ImGui 上下文（调用方持有的句柄同步失效）
						(*it)->close();
						it = engine.m_Windows.erase(it);
					}
					else {
						(*it)->render();
						++it;
					}
				}
			}
		}
		// 注册窗口：引擎与调用方共享窗口状态（shared_ptr），调用方无需 release，
		// 且初始化之后源对象仍可继续使用
		void Registration(std::shared_ptr<ImplT> hand) {
			m_Windows.push_back(std::move(hand));
		}
	private:
		std::vector<std::shared_ptr<ImplT>> m_Windows;
	};

}