#pragma once
// gallery 组件间事件：经引擎事件通道收发（on<E> / emit<E>）。
#include <imgui_mk/ui/wg2/style.h>

namespace gallery {

struct ThemeChanged {          // 换肤：各窗口自己 setTheme
	Mk::wg2::Theme theme;
};

struct ToggleInspector {};     // 显示/隐藏检查器：检查器窗口自己处理

struct OpenFile {};            // 打开文件：文件页自持对话框并处理

} // namespace gallery
