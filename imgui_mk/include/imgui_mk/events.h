#pragma once
// 框架层事件通道：类型化、同步派发、组件之间不互相持有引用
//
//   bus.on<Ev>([this](const Ev& e) { ... });   // 订阅
//   bus.emit(Ev{...});                          // 发布
//
// 组件通过引用接收 EventBus（见各组件 init 的签名），不直接碰引擎单例。
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Mk {

class EventBus {
public:
	template<class E>
	void on(std::function<void(const E&)> fn) {
		m_Handlers[typeid(E)].push_back(
			[fn = std::move(fn)](const void* p) { fn(*static_cast<const E*>(p)); });
	}

	template<class E>
	void emit(const E& e) {
		auto it = m_Handlers.find(typeid(E));
		if (it == m_Handlers.end()) return;
		auto handlers = it->second;   // 拷贝：回调里允许继续 on/emit
		for (auto& h : handlers) h(&e);
	}

	void clear() { m_Handlers.clear(); }

private:
	std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> m_Handlers;
};

} // namespace Mk
