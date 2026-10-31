#pragma once
// gallery 公共设施：组件间事件、演示常量、FnDrawerImpl（FnRender 页用）+ gMono（等宽字体）。
// 数据不在这里聚合：各页面 / 各窗口组件自己持有。
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>




#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/texture.h>
#include <Mainwindow.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace gallery {

using namespace Mk::wg2;

// 等宽代码字体（main 在创建窗口后、首帧前 AddFontDefault 装载填充）
inline ImFont* gMono = nullptr;

// ============ 演示常量（命名空间级，页面各自取用）============
inline constexpr const char* kItems[] = {"Option A", "Option B", "Option C"};
inline constexpr const char* kHeaders[] = {"Name", "Value", "Note"};

// ============ 组件间事件（引擎通道，类型化）============
struct ThemeChanged {
	Theme theme;
};
struct ToggleInspector {};
struct OpenFile {};

// ============ 共享状态（归 GalleryWindow 组件所有，经引用传给页面）============

	// （原 State 聚合已移除：数据归各页面 / 各窗口组件所有）

// ============ 引擎访问助手（页面零窗口依赖）============

// 首个存活窗口的实际烘焙字号（无窗口返回 0）
inline float engineFontSize() {
	return Mk::mkui<Mk::window>::RegistrationWindow().fontSize();
}

// 引擎级字号重烘焙广播（各窗口帧内应用）
inline void engineRescale(float fontSize) {
	Mk::mkui<Mk::window>::RegistrationWindow().rescaleAll(fontSize);
}

	// FnRender 页演示用的裸绘制桥：自带演示值，不依赖应用状态
	struct FnDrawerImpl {
		void operator()() const {
			ImGui::ProgressBar(0.42f, ImVec2(-FLT_MIN, 0), "FnRender 演示条");
		}
	};

} // namespace gallery
