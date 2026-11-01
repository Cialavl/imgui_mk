#pragma once
// p26 布局·复杂静态：Grid 网格 / Wrap 换行 / Overlay 叠放（嵌套组合）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutGrid {
	using Root = VBox<
		Id("root"),
		Label<Id("intro")>,
		Grid<Id("grid"),
			Button<Id("g0")>,
			Button<Id("g1")>,
			Button<Id("g2")>,
			Button<Id("g3")>,
			Button<Id("g4")>,
			Button<Id("g5")>>,
		Label<Id("wrapTitle")>,
		Wrap<Id("wrap"),
			Button<Id("w0")>,
			Button<Id("w1")>,
			Button<Id("w2")>,
			Button<Id("w3")>,
			Button<Id("w4")>,
			Button<Id("w5")>>,
		Label<Id("ovTitle")>,
		Overlay<Id("ov"),
			Label<Id("ovBase")>,
			Offset<Id("ovOff"), Button<Id("ovBtn")>>>,
		Label<Id("note")>>;

	Page page;
	Root root;

	PageLayoutGrid() {
		page.setup("布局·复杂静态：Grid / Wrap / Overlay",
			"Grid 按列数铺网格、Wrap 放不下自动换行、Overlay 同区域叠放（配合 Offset 错位）。"
			"三者都可以再套 VBox/HBox 组成任意复杂结构。",
			snips::kLayoutGrid);

		root.at<Id("intro")>().text = "Grid：3 列 × 2 行，列宽均分";
		auto& w_grid = root.at<Id("grid")>();
		w_grid.cols = 3;
		w_grid.spacing = 8.0f;
		for (int i = 0; i < 6; ++i) {
			auto label = "格子 " + std::to_string(i);
			switch (i) {
			case 0: root.at<Id("grid"), Id("g0")>().label = label; break;
			case 1: root.at<Id("grid"), Id("g1")>().label = label; break;
			case 2: root.at<Id("grid"), Id("g2")>().label = label; break;
			case 3: root.at<Id("grid"), Id("g3")>().label = label; break;
			case 4: root.at<Id("grid"), Id("g4")>().label = label; break;
			case 5: root.at<Id("grid"), Id("g5")>().label = label; break;
			}
		}

		root.at<Id("wrapTitle")>().text = "Wrap：宽度不够自动换行（拖窄窗口看效果）";
		root.at<Id("wrap")>().spacing = 8.0f;
		root.at<Id("wrap"), Id("w0")>().label = "标签 A";
		root.at<Id("wrap"), Id("w1")>().label = "标签 B";
		root.at<Id("wrap"), Id("w2")>().label = "标签 C";
		root.at<Id("wrap"), Id("w3")>().label = "标签 D";
		root.at<Id("wrap"), Id("w4")>().label = "标签 E";
		root.at<Id("wrap"), Id("w5")>().label = "标签 F";

		root.at<Id("ovTitle")>().text = "Overlay：文字在底层，按钮用 Offset 浮在上面";
		root.at<Id("ov"), Id("ovBase")>().text = "底层文字（会被上面的按钮压住一部分）";
		auto& w_ovOff = root.at<Id("ov"), Id("ovOff")>();
		w_ovOff.dxEm = 9.0f;
		w_ovOff.dyEm = 0.2f;
		w_ovOff.child.label = "浮层按钮";
		root.at<Id("note")>().text = "复杂布局 = 这些容器随意嵌套，路径 at<Id, Id, ...>() 依旧可直达。";

		bindPage(page, root);
	}
};

} // namespace gallery
