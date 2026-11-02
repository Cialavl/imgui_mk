#pragma once
#include "events.h"
#include <imgui_mk/windowRendereren.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

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

		// ===== 共享状态注册表（应用默认注册一次，全部窗口/页面经引擎取用 = 数据联动）=====
		// 类型键多槽：不同状态类型互不干扰；未注册即取用抛异常（fail-fast）
		template<typename S>
		void registerState(std::shared_ptr<S> s) {
			m_States[typeid(S)] = std::move(s);
		}
		template<typename S>
		S& state() {
			auto it = m_States.find(typeid(S));
			if(it == m_States.end()) throw std::runtime_error("mkui: state 未注册");
			return *static_cast<S*>(it->second.get());
		}

		// ===== 类型化事件通道：组件经引擎收发事件（同步派发，帧内调用安全）=====
		// 组件之间不互相持有引用：发送方 emit，接收方 on，各自管自己的数据。
		// 事件通道：组件通过 bus() 收发（框架层统一实现）
		Mk::EventBus& bus() { return m_Bus; }
		static Mk::EventBus& Events() { return RegistrationWindow().bus(); }   // 组件侧便捷入口

		template<class E>
		void on(std::function<void(const E&)> fn) { m_Bus.on<E>(std::move(fn)); }
		template<class E>
		void emit(const E& e) { m_Bus.emit<E>(e); }

		// ===== 跨窗口联动广播（各窗口在自己帧内应用，避免跨窗口上下文切换）=====
		// requires 守卫：最小契约 Impl（render/shouldClose/close）不实例化这些路径
		template<typename T>
		void applyThemeAll(T&& t) {
			if constexpr (requires(ImplT& i, T&& v) { i.requestTheme(v); }) {
				for(auto& w : m_Windows) w->requestTheme(t);
			}
		}
		template<typename... A>
		void rescaleAll(A&&... a) {
			if constexpr (requires(ImplT& i, A&&... v) { i.requestRescale(v...); }) {
				for(auto& w : m_Windows) w->requestRescale(std::forward<A>(a)...);
			}
		}
		// 首个存活窗口的实际烘焙字号（无窗口或最小契约 Impl 返回 0）
		float fontSize() const {
			if constexpr (requires(ImplT& i) { i.m_Handle; i.m_LastBakedSize; }) {
				for(const auto& w : m_Windows) {
					if(w->m_Handle) return w->m_LastBakedSize;
				}
			}
			return 0.0f;
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
		std::unordered_map<std::type_index, std::shared_ptr<void>> m_States;
		Mk::EventBus m_Bus;
	};

}