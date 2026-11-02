#pragma once
// 应用事件契约（叶子头：只定义类型，不依赖任何组件）
#include <imgui_mk/ui/wg2/style.h>

namespace gallery {

// 等宽代码字体（main 创建窗口后、首帧前装载并回填）
inline ImFont* gMono = nullptr;

// 首个存活窗口的实际烘焙字号（无窗口返回 0）
inline float engineFontSize() {
	return Mk::mkui<Mk::window>::RegistrationWindow().fontSize();
}

// 引擎级字号重烘焙广播（各窗口帧内应用）
inline void engineRescale(float fontSize) {
	Mk::mkui<Mk::window>::RegistrationWindow().rescaleAll(fontSize);
}

struct ThemeChanged {          // 换肤：各窗口自己 setTheme
	Mk::wg2::Theme theme;
};

struct ToggleInspector {};     // 显示/隐藏检查器：检查器窗口自己处理

struct OpenFile {};            // 打开文件：文件页自持对话框并处理


using namespace Mk::wg2;   // 页面代码沿用 wg2 名字


// ============ 引擎访问助手（页面零窗口依赖）============

} // namespace gallery
