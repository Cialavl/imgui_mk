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
		auto& _w1 = root.at<Id("bar"), Id("addTile")>();
		_w1.label = "网格 +1";
		_w1.onClick = [this] {
			auto tile = std::make_unique<Label<Id("tile")>>(
				"卡片 " + std::to_string(++tiles));
			AnyWidget any = AnyWidget::own(std::move(tile));
			any.idname = "tile" + std::to_string(tiles);
			root.at<Id("body"), Id("gridZone")>().child<0>().add(std::move(any));
		};
		auto& _w2 = root.at<Id("bar"), Id("addTab")>();
		_w2.label = "页签 +1";
		_w2.onClick = [this] {
			auto body = std::make_unique<Label<Id("tabBody")>>(
				"第 " + std::to_string(++tabs) + " 页的内容");
			AnyWidget any = AnyWidget::own(std::move(body));
			any.idname = "tab" + std::to_string(tabs);
			root.at<Id("body"), Id("tabZone")>().child<0>().add("页 " + std::to_string(tabs), std::move(any));
		};
		auto& _w3 = root.at<Id("bar"), Id("clearTiles")>();
		_w3.label = "清空网格";
		_w3.onClick = [this] {
			root.at<Id("body"), Id("gridZone")>().child<0>().clear();
		};

		auto& _w4 = root.at<Id("body"), Id("gridZone")>();
		_w4.idname = "layoutDynGrid";
		_w4.height = 220.0f;
		_w4.border = true;
		_w4.child<0>().cols = 2;
		_w4.child<0>().spacing = 8.0f;

		auto& _w5 = root.at<Id("body"), Id("tabZone")>();
		_w5.idname = "layoutDynTabs";
		_w5.height = 220.0f;
		_w5.border = true;
		root.at<Id("note")>().text = "两个容器都是 wg2 的动态层（AnyWidget 类型擦除），静态树其余部分完全不受影响。";

		bindPage(page, root);
	}
};

} // namespace gallery
