#pragma once
// gallery 环境助手：等宽代码字体句柄 + 引擎级字号查询/重烘焙。
// 页面只通过这里访问引擎，不直接碰窗口对象。
#include <imgui.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>

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

} // namespace gallery
