#pragma once
// p28 布局·复杂动态：RuntimeGrid + RuntimeTabs 组合，随窗口伸缩
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名（与 gallery_app.h 的壳定义同风格）

// —— 工具条 + 左右两块动态区 ——
using ToolBar = ui::HBox<ui::Id("bar"),
	ui::Button<ui::Id("addTile")>, ui::Button<ui::Id("addTab")>, ui::Button<ui::Id("clearTiles")>>;

using TileGrid = ui::SubWindow<ui::Id("gridZone"), ui::RuntimeGrid<ui::Id("grid")>>;
using TabPane  = ui::SubWindow<ui::Id("tabZone"),  ui::RuntimeTabs<ui::Id("tabs")>>;
using BodyRow  = ui::HBox<ui::Id("body"), TileGrid, TabPane>;

struct PageLayoutDynamic {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "布局·复杂动态：RuntimeGrid + RuntimeTabs";
	static constexpr const char* desc  = "两个动态容器并排：左边网格随窗口宽度自动排布列，右边页签运行时追加；"
			"整体由 HBox 分配，SubWindow 给各自滚动。";
	static constexpr std::string_view code = snips::kLayoutDynamic;

	// 根只列结构：说明 / 工具条 / 两块动态区 / 注解
	using Root = ui::VBox<ui::Id("root"),
		ui::Label<ui::Id("intro")>,
		ToolBar,
		BodyRow,
		ui::Label<ui::Id("note")>>;

	Root root;
	int tiles = 0;
	int tabs = 0;

	PageLayoutDynamic() {
		

		root.at<ui::Id("intro")>().text = "左边网格加卡片、右边页签加页；窗口变窄时网格自动压列宽";

		auto& w_body = root.at<ui::Id("body")>();
		auto& w_grid = w_body.at<ui::Id("gridZone")>();
		auto& w_tabs = w_body.at<ui::Id("tabZone")>();

		auto& w_addTile = root.at<ui::Id("bar"), ui::Id("addTile")>();
		w_addTile.label = "网格 +1";
		w_addTile.onClick = [this, &w_grid] {
			auto tile = std::make_unique<ui::Label<ui::Id("tile")>>(
				"卡片 " + std::to_string(++tiles));
			ui::AnyWidget any = ui::AnyWidget::own(std::move(tile));
			any.idname = "tile" + std::to_string(tiles);
			w_grid.child<0>().add(std::move(any));
		};

		auto& w_addTab = root.at<ui::Id("bar"), ui::Id("addTab")>();
		w_addTab.label = "页签 +1";
		w_addTab.onClick = [this, &w_tabs] {
			auto body = std::make_unique<ui::Label<ui::Id("tabBody")>>(
				"第 " + std::to_string(++tabs) + " 页的内容");
			ui::AnyWidget any = ui::AnyWidget::own(std::move(body));
			any.idname = "tab" + std::to_string(tabs);
			w_tabs.child<0>().add("页 " + std::to_string(tabs), std::move(any));
		};

		auto& w_clearTiles = root.at<ui::Id("bar"), ui::Id("clearTiles")>();
		w_clearTiles.label = "清空网格";
		w_clearTiles.onClick = [&w_grid] { w_grid.child<0>().clear(); };

		w_grid.idname = "layoutDynGrid";
		w_grid.height = 220.0f;
		w_grid.border = true;
		w_grid.child<0>().cols = 2;
		w_grid.child<0>().spacing = 8.0f;

		w_tabs.idname = "layoutDynTabs";
		w_tabs.height = 220.0f;
		w_tabs.border = true;

		root.at<ui::Id("note")>().text = "两个容器都是 wg2 的动态层（AnyWidget 类型擦除），静态树其余部分完全不受影响。";

	}
};

} // namespace gallery
