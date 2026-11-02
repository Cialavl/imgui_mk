#pragma once
// gallery 主窗口 = 框架主窗口（Mk::MainWindow）：构造即建窗，并一行挂载组合界面。
//   - 组合界面 GalleryLayout 直接挂在本窗口下（父节点 = 主窗口）；
//   - 子界面各自的初始化（setup/ready）由框架沿树驱动，应用只写自己的一行。
#include "tools/inspector_window.h"
#include "status_bar.h"
#include "menu_bar.h"
#include "content_area/content_area.h"
#include <Mainwindow.h>
#include <imgui_mk/ui_context.h>

#include <cstdlib>

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名

// ============ 组合界面：主体三行 + 检查器窗口节点 ============
struct GalleryLayout {
	static constexpr auto id = ui::Id("galleryLayout");

	using ContentRow = ui::HBox<ui::Id("contentRow"), ContentArea>;                   // 中间内容区
	using MainColumn = ui::VBox<ui::Id("mainCol"), MenuBar, ContentRow>;             // 菜单 + 内容（页脚在内容列内）
	using Root = ui::Overlay<ui::Id("galleryLayout"), MainColumn, InspectorWindow>;  // 树根（窗口节点浮在上面）

	Root root;

	GalleryLayout() {
		// 布局参数：整块贴边、内容行吃剩余高度、内容区吃剩余宽度
		root.marginX = 0.0f;
		root.marginY = 0.0f;

		auto& mainCol = root.at<ui::Id("mainCol")>();
		mainCol.marginX = 0.0f;
		mainCol.marginY = 0.0f;
		mainCol.spacing = 0.0f;   // 行间不留缝：菜单栏→内容由页面留白控制（8px）

		auto& row = root.at<ui::Id("mainCol"), ui::Id("contentRow")>();
		row.marginX = 0.0f;
		row.marginY = 0.0f;
		ui::setStretch(row, 1.0f);
		ui::setStretch(contentArea(), 1.0f);
	}

	// —— 结构访问器：从树上按路径取 ——
	MenuBar& menuBar() { return root.at<ui::Id("mainCol"), ui::Id("menuBar")>(); }

	StatusBar& statusBar() { return contentArea().statusBar(); }
	ContentArea& contentArea() {
		return root.at<ui::Id("mainCol"), ui::Id("contentRow"), ui::Id("content")>();
	}
	InspectorWindow& inspector() { return root.at<ui::Id("inspector")>(); }
};

// ============ gallery 主窗口：把组合界面挂进来 ============
struct GalleryWindow : Mk::MainWindow<Mk::window> {
	GalleryLayout layout;   // 直接挂在本窗口的控件

	GalleryWindow(const char* title = "mkui Widget Gallery")
		: Mk::MainWindow<Mk::window>(title) {
		setContent(layout);   // ← 一行挂载：父节点 / 树初始化 / 中央视图都由框架完成
	}
};

} // namespace gallery
