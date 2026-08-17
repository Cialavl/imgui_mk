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
				// 轮询前兜底：若当前 ImGui 上下文为空（如调用方在循环外手动 close 过窗口），
				// 切到任一存活窗口，避免下面 glfwPollEvents 派发消息时
				// 存活窗口的 WndProc 钩子拿到空上下文（后端无空指针保护）
				if (ImGui::GetCurrentContext() == nullptr) {
					switchToLiveContext(engine);
				}
				glfwPollEvents();
				for (auto it = engine.m_Windows.begin(); it != engine.m_Windows.end(); ) {
					if ((*it)->shouldClose()) {
						// 真正关闭：销毁 GLFW 窗口与 ImGui 上下文（调用方持有的句柄同步失效）
						(*it)->close();
						it = engine.m_Windows.erase(it);
						// 多窗口关键保护：close() 尾部 DestroyContext 会把当前上下文清空，
						// 而其它存活窗口的 WndProc 钩子仍装在各自 HWND 上、随消息触发；
						// 必须立刻切到存活窗口的有效上下文，不留空窗期
						if (ImGui::GetCurrentContext() == nullptr) {
							switchToLiveContext(engine);
						}
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
		// 把 ImGui 当前上下文切到任一仍存活的窗口（无存活窗口则不动）。
		// 仅当 Impl 暴露 m_Handle / m_ImGuiContext（真实窗口）时生效；
		// 最小契约 Impl（render/shouldClose/close）无 ImGui 状态，编译期跳过
		static void switchToLiveContext(mkui& engine) {
			if constexpr (requires(ImplT& impl) { impl.m_Handle; impl.m_ImGuiContext; }) {
				for (auto& w : engine.m_Windows) {
					if (w->m_Handle) {
						ImGui::SetCurrentContext(w->m_ImGuiContext);
						return;
					}
				}
			}
		}

		std::vector<std::shared_ptr<ImplT>> m_Windows;
	};

}