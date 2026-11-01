#pragma once
// p28 布局·复杂动态：RuntimeGrid + RuntimeTabs 组合，随窗口伸缩
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutDynamic {
	using Root = VBox<
		Id("root"),
		Label<Id("intro")>,
		HBox<Id("bar"),
			Button<Id("addTile")>,
			Button<Id("addTab")>,
			Button<Id("clearTiles")>>,
		HBox<Id("body"),
			SubWindow<Id("gridZone"), RuntimeGrid<Id("grid")>>,
			SubWindow<Id("tabZone"), RuntimeTabs<Id("tabs")>>>,
		Label<Id("note")>>;

	Page page;
	Root root;
	int tiles = 0;
	int tabs = 0;

	PageLayoutDynamic() {
		page.setup("布局·复杂动态：RuntimeGrid + RuntimeTabs",
			"两个动态容器并排：左边网格随窗口宽度自动排布列，右边页签运行时追加；"
			"整体由 HBox 分配，SubWindow 给各自滚动。",
			snips::kLayoutDynamic);

		root.at<Id("intro")>().text = "左边网格加卡片、右边页签加页；窗口变窄时网格自动压列宽";
		auto& w_addTile = root.at<Id("bar"), Id("addTile")>();
		w_addTile.label = "网格 +1";
		w_addTile.onClick = [this] {
			auto tile = std::make_unique<Label<Id("tile")>>(
				"卡片 " + std::to_string(++tiles));
			AnyWidget any = AnyWidget::own(std::move(tile));
			any.idname = "tile" + std::to_string(tiles);
			root.at<Id("body"), Id("gridZone")>().child<0>().add(std::move(any));
		};
		auto& w_addTab = root.at<Id("bar"), Id("addTab")>();
		w_addTab.label = "页签 +1";
		w_addTab.onClick = [this] {
			auto body = std::make_unique<Label<Id("tabBody")>>(
				"第 " + std::to_string(++tabs) + " 页的内容");
			AnyWidget any = AnyWidget::own(std::move(body));
			any.idname = "tab" + std::to_string(tabs);
			root.at<Id("body"), Id("tabZone")>().child<0>().add("页 " + std::to_string(tabs), std::move(any));
		};
		auto& w_clearTiles = root.at<Id("bar"), Id("clearTiles")>();
		w_clearTiles.label = "清空网格";
		w_clearTiles.onClick = [this] {
			root.at<Id("body"), Id("gridZone")>().child<0>().clear();
		};

		auto& w_gridZone = root.at<Id("body"), Id("gridZone")>();
		w_gridZone.idname = "layoutDynGrid";
		w_gridZone.height = 220.0f;
		w_gridZone.border = true;
		w_gridZone.child<0>().cols = 2;
		w_gridZone.child<0>().spacing = 8.0f;

		auto& w_tabZone = root.at<Id("body"), Id("tabZone")>();
		w_tabZone.idname = "layoutDynTabs";
		w_tabZone.height = 220.0f;
		w_tabZone.border = true;
		root.at<Id("note")>().text = "两个容器都是 wg2 的动态层（AnyWidget 类型擦除），静态树其余部分完全不受影响。";

		bindPage(page, root);
	}
};

} // namespace gallery
