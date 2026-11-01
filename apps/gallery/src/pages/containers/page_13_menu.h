#pragma once
// p13 菜单面板：SubWindow 面板（wg2 编译期子节点）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名（与 gallery_app.h 的壳定义同风格）

// —— 面板：滑块 + 帧计数 ——
using PanelDemo = ui::SubWindow<ui::Id("panel"),
	ui::SliderF<ui::Id("slider")>,
	ui::LabelText<ui::Id("frames")>>;

struct PageMenu {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "菜单与面板";
	static constexpr const char* desc  = "wg2 Panel = SubWindow 别名；面板内容经模板参数编译期传入。";
	static constexpr std::string_view code = snips::kPanel;

	// 根只列结构：面板 / 注解
	using Root = ui::VBox<ui::Id("root"), PanelDemo, ui::Label<ui::Id("note")>>;

	Root root;

	Mk::window* win = nullptr;   // 形参存成员，避免 [&] 捕获形参后悬垂
	float volume = 0.6f;         // 本页自己的数据

	PageMenu(Mk::window* w) : win(w) {
		

		auto& w_panel = root.at<ui::Id("panel")>();
		w_panel.idname = "面板";
		w_panel.height = 110.0f;

		auto& w_slider = w_panel.at<ui::Id("slider")>();
		w_slider.name = "面板内滑块";
		w_slider.value = &volume;

		auto& w_frames = w_panel.at<ui::Id("frames")>();
		w_frames.name = "帧";
		w_frames.valueFn = [this] { return std::to_string(win->frameIndex()); };

		root.at<ui::Id("note")>().text = "Panel 是浮动窗口；菜单栏是常驻点（主窗口顶部）。";

	}
};

} // namespace gallery
