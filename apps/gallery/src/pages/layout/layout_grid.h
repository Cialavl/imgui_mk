#pragma once
// p26 布局·复杂静态：Grid 网格 / Wrap 换行 / Overlay 叠放（嵌套组合）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名（与 gallery_app.h 的壳定义同风格）

// —— 三个演示区块各自命名 ——
using GridDemo = ui::Grid<ui::Id("grid"),
	ui::Button<ui::Id("g0")>, ui::Button<ui::Id("g1")>, ui::Button<ui::Id("g2")>,
	ui::Button<ui::Id("g3")>, ui::Button<ui::Id("g4")>, ui::Button<ui::Id("g5")>>;

using WrapDemo = ui::Wrap<ui::Id("wrap"),
	ui::Button<ui::Id("w0")>, ui::Button<ui::Id("w1")>, ui::Button<ui::Id("w2")>,
	ui::Button<ui::Id("w3")>, ui::Button<ui::Id("w4")>, ui::Button<ui::Id("w5")>>;

using OverlayDemo = ui::Overlay<ui::Id("ov"),
	ui::Label<ui::Id("ovBase")>,
	ui::Offset<ui::Id("ovOff"), ui::Button<ui::Id("ovBtn")>>>;

struct PageLayoutGrid {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "布局·复杂静态：Grid / Wrap / Overlay";
	static constexpr const char* desc  = "Grid 按列数铺网格、Wrap 放不下自动换行、Overlay 同区域叠放（配合 Offset 错位）。"
			"三者都可以再套 VBox/HBox 组成任意复杂结构。";
	static constexpr std::string_view code = snips::kLayoutGrid;

	// 根只列结构：标题 / 网格 / 换行 / 叠放 / 说明
	using Root = ui::VBox<ui::Id("root"),
		ui::Label<ui::Id("intro")>,
		GridDemo,
		ui::Label<ui::Id("wrapTitle")>,
		WrapDemo,
		ui::Label<ui::Id("ovTitle")>,
		OverlayDemo,
		ui::Label<ui::Id("note")>>;

	Root root;

	PageLayoutGrid() {
		

		root.at<ui::Id("intro")>().text = "Grid：3 列 × 2 行，列宽均分";

		auto& w_grid = root.at<ui::Id("grid")>();
		w_grid.cols = 3;
		w_grid.spacing = 8.0f;
		for (int i = 0; i < 6; ++i) {
			const auto label = "格子 " + std::to_string(i);
			switch (i) {
			case 0: w_grid.at<ui::Id("g0")>().label = label; break;
			case 1: w_grid.at<ui::Id("g1")>().label = label; break;
			case 2: w_grid.at<ui::Id("g2")>().label = label; break;
			case 3: w_grid.at<ui::Id("g3")>().label = label; break;
			case 4: w_grid.at<ui::Id("g4")>().label = label; break;
			case 5: w_grid.at<ui::Id("g5")>().label = label; break;
			}
		}

		root.at<ui::Id("wrapTitle")>().text = "Wrap：宽度不够自动换行（拖窄窗口看效果）";
		auto& w_wrap = root.at<ui::Id("wrap")>();
		w_wrap.spacing = 8.0f;
		w_wrap.at<ui::Id("w0")>().label = "标签 A";
		w_wrap.at<ui::Id("w1")>().label = "标签 B";
		w_wrap.at<ui::Id("w2")>().label = "标签 C";
		w_wrap.at<ui::Id("w3")>().label = "标签 D";
		w_wrap.at<ui::Id("w4")>().label = "标签 E";
		w_wrap.at<ui::Id("w5")>().label = "标签 F";

		root.at<ui::Id("ovTitle")>().text = "Overlay：文字在底层，按钮用 Offset 浮在上面";
		root.at<ui::Id("ov"), ui::Id("ovBase")>().text = "底层文字（会被上面的按钮压住一部分）";
		auto& w_ovOff = root.at<ui::Id("ov"), ui::Id("ovOff")>();
		w_ovOff.dxEm = 9.0f;
		w_ovOff.dyEm = 0.2f;
		w_ovOff.child.label = "浮层按钮";

		root.at<ui::Id("note")>().text = "复杂布局 = 这些容器随意嵌套，路径 at<Id, Id, ...>() 依旧可直达。";

	}
};

} // namespace gallery
