#pragma once
// 控件树上下文（Qt 式）：框架启动时沿控件树向下传，父节点只给上下文，子节点自己初始化。
#include <functional>

#include <imgui_mk/events.h>
#include <imgui_mk/window.h>

namespace Mk {

struct UiContext {
	Mk::window* window = nullptr;             // 主窗口
	Mk::window* inspectorWindow = nullptr;    // 附属窗口（由对应组件在 setup 中登记）
	Mk::EventBus* bus = nullptr;              // 事件通道
	std::function<const char*()> pageTitle;   // 当前页标题（内容区在 ready 中登记）
};

} // namespace Mk
