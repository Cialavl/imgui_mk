#pragma once
// p16 颜色编辑：ColorEdit3/4 + ColorPicker3/4（wg2）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名

// —— 拾取器区：两个拾取器并排，收在折叠容器里 ——
using PickerRow = ui::HBox<ui::Id("pickRow"),
	ui::ColorPicker3<ui::Id("pick3")>,
	ui::ColorPicker4<ui::Id("pick4")>>;

using PickerBox = ui::Collapsing<ui::Id("collapsing"), PickerRow>;

struct PageColor {
	static constexpr const char* title = "颜色编辑";
	static constexpr const char* desc  = "wg2 ColorEdit3/4 与 ColorPicker3/4：std::array<float,3|4>* 绑定；拾取器可包进折叠容器。";
	static constexpr std::string_view code = snips::kColor;

	// 根只列结构：两个编辑框 + 拾取器区
	using Root = ui::VBox<ui::Id("root"),
		ui::ColorEdit3<ui::Id("rgb")>,
		ui::ColorEdit4<ui::Id("rgba")>,
		PickerBox>;

	Root root;

	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};

	PageColor() {
		auto& w_rgb = root.at<ui::Id("rgb")>();
		w_rgb.name = "RGB";
		w_rgb.color = &col3;

		auto& w_rgba = root.at<ui::Id("rgba")>();
		w_rgba.name = "RGBA";
		w_rgba.color = &col4;

		auto& w_box = root.at<ui::Id("collapsing")>();
		w_box.title = "拾取器（默认展开）";
		w_box.defaultOpen = true;

		auto& w_pick3 = root.at<ui::Id("collapsing"), ui::Id("pickRow"), ui::Id("pick3")>();
		w_pick3.name = "Pick RGB";
		w_pick3.color = &pick3;

		auto& w_pick4 = root.at<ui::Id("collapsing"), ui::Id("pickRow"), ui::Id("pick4")>();
		w_pick4.name = "Pick RGBA";
		w_pick4.color = &pick4;
	}
};

} // namespace gallery
