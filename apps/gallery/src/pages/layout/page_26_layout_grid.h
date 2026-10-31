#pragma once
// p26 布局·复杂静态：Grid 网格 / Wrap 换行 / Overlay 叠放（嵌套组合）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutGrid {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("intro")>,
		Mk::wg2::Grid<Mk::wg2::Id("grid"),
			Mk::wg2::Button<Mk::wg2::Id("g0")>,
			Mk::wg2::Button<Mk::wg2::Id("g1")>,
			Mk::wg2::Button<Mk::wg2::Id("g2")>,
			Mk::wg2::Button<Mk::wg2::Id("g3")>,
			Mk::wg2::Button<Mk::wg2::Id("g4")>,
			Mk::wg2::Button<Mk::wg2::Id("g5")>>,
		Mk::wg2::Label<Mk::wg2::Id("wrapTitle")>,
		Mk::wg2::Wrap<Mk::wg2::Id("wrap"),
			Mk::wg2::Button<Mk::wg2::Id("w0")>,
			Mk::wg2::Button<Mk::wg2::Id("w1")>,
			Mk::wg2::Button<Mk::wg2::Id("w2")>,
			Mk::wg2::Button<Mk::wg2::Id("w3")>,
			Mk::wg2::Button<Mk::wg2::Id("w4")>,
			Mk::wg2::Button<Mk::wg2::Id("w5")>>,
		Mk::wg2::Label<Mk::wg2::Id("ovTitle")>,
		Mk::wg2::Overlay<Mk::wg2::Id("ov"),
			Mk::wg2::Label<Mk::wg2::Id("ovBase")>,
			Mk::wg2::Offset<Mk::wg2::Id("ovOff"), Mk::wg2::Button<Mk::wg2::Id("ovBtn")>>>,
		Mk::wg2::Label<Mk::wg2::Id("note")>>;

	Page page;
	Root root;

	PageLayoutGrid() {
		page.setup("布局·复杂静态：Grid / Wrap / Overlay",
			"Grid 按列数铺网格、Wrap 放不下自动换行、Overlay 同区域叠放（配合 Offset 错位）。"
			"三者都可以再套 VBox/HBox 组成任意复杂结构。",
			snips::kLayoutGrid);

		root.at<Mk::wg2::Id("intro")>().text = "Grid：3 列 × 2 行，列宽均分";
		root.at<Mk::wg2::Id("grid")>().cols = 3;
		root.at<Mk::wg2::Id("grid")>().spacing = 8.0f;
		for (int i = 0; i < 6; ++i) {
			auto label = "格子 " + std::to_string(i);
			switch (i) {
			case 0: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g0")>().label = label; break;
			case 1: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g1")>().label = label; break;
			case 2: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g2")>().label = label; break;
			case 3: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g3")>().label = label; break;
			case 4: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g4")>().label = label; break;
			case 5: root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g5")>().label = label; break;
			}
		}

		root.at<Mk::wg2::Id("wrapTitle")>().text = "Wrap：宽度不够自动换行（拖窄窗口看效果）";
		root.at<Mk::wg2::Id("wrap")>().spacing = 8.0f;
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w0")>().label = "标签 A";
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w1")>().label = "标签 B";
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w2")>().label = "标签 C";
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w3")>().label = "标签 D";
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w4")>().label = "标签 E";
		root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("w5")>().label = "标签 F";

		root.at<Mk::wg2::Id("ovTitle")>().text = "Overlay：文字在底层，按钮用 Offset 浮在上面";
		root.at<Mk::wg2::Id("ov"), Mk::wg2::Id("ovBase")>().text = "底层文字（会被上面的按钮压住一部分）";
		root.at<Mk::wg2::Id("ov"), Mk::wg2::Id("ovOff")>().dxEm = 9.0f;
		root.at<Mk::wg2::Id("ov"), Mk::wg2::Id("ovOff")>().dyEm = 0.2f;
		root.at<Mk::wg2::Id("ov"), Mk::wg2::Id("ovOff")>().child.label = "浮层按钮";
		root.at<Mk::wg2::Id("note")>().text = "复杂布局 = 这些容器随意嵌套，路径 at<Id, Id, ...>() 依旧可直达。";

		bindPage(page, root);
	}
};

} // namespace gallery
