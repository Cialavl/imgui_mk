#pragma once
// p28 布局·复杂动态：RuntimeGrid + RuntimeTabs 组合，随窗口伸缩
#include "../gallery_chrome.h"

namespace gallery {

struct PageLayoutDynamic {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("intro")>,
		Mk::wg2::HBox<Mk::wg2::Id("bar"),
			Mk::wg2::Button<Mk::wg2::Id("addTile")>,
			Mk::wg2::Button<Mk::wg2::Id("addTab")>,
			Mk::wg2::Button<Mk::wg2::Id("clearTiles")>>,
		Mk::wg2::HBox<Mk::wg2::Id("body"),
			Mk::wg2::SubWindow<Mk::wg2::Id("gridZone"), Mk::wg2::RuntimeGrid<Mk::wg2::Id("grid")>>,
			Mk::wg2::SubWindow<Mk::wg2::Id("tabZone"), Mk::wg2::RuntimeTabs<Mk::wg2::Id("tabs")>>>,
		Mk::wg2::Label<Mk::wg2::Id("note")>>;

	Page page;
	Root root;
	int tiles = 0;
	int tabs = 0;

	PageLayoutDynamic(State&) {
		page.setup("布局·复杂动态：RuntimeGrid + RuntimeTabs",
			"两个动态容器并排：左边网格随窗口宽度自动排布列，右边页签运行时追加；"
			"整体由 HBox 分配，SubWindow 给各自滚动。",
			snips::kLayoutDynamic);

		root.at<Mk::wg2::Id("intro")>().text = "左边网格加卡片、右边页签加页；窗口变窄时网格自动压列宽";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("addTile")>().label = "网格 +1";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("addTile")>().onClick = [this] {
			auto tile = std::make_unique<Mk::wg2::Label<Mk::wg2::Id("tile")>>(
				"卡片 " + std::to_string(++tiles));
			Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(tile));
			any.idname = "tile" + std::to_string(tiles);
			root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().child<0>().add(std::move(any));
		};
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("addTab")>().label = "页签 +1";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("addTab")>().onClick = [this] {
			auto body = std::make_unique<Mk::wg2::Label<Mk::wg2::Id("tabBody")>>(
				"第 " + std::to_string(++tabs) + " 页的内容");
			Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(body));
			any.idname = "tab" + std::to_string(tabs);
			root.at<Mk::wg2::Id("body"), Mk::wg2::Id("tabZone")>().child<0>().add("页 " + std::to_string(tabs), std::move(any));
		};
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("clearTiles")>().label = "清空网格";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("clearTiles")>().onClick = [this] {
			root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().child<0>().clear();
		};

		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().idname = "layoutDynGrid";
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().height = 220.0f;
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().border = true;
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().child<0>().cols = 2;
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("gridZone")>().child<0>().spacing = 8.0f;

		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("tabZone")>().idname = "layoutDynTabs";
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("tabZone")>().height = 220.0f;
		root.at<Mk::wg2::Id("body"), Mk::wg2::Id("tabZone")>().border = true;
		root.at<Mk::wg2::Id("note")>().text = "两个容器都是 wg2 的动态层（AnyWidget 类型擦除），静态树其余部分完全不受影响。";

		page.addWidget([&](const Mk::wg2::Rect& r) { Mk::wg2::layout(root, r); },
			[&](const Mk::wg2::Size& avail) { return root.hint(avail); });
	}
};

} // namespace gallery
