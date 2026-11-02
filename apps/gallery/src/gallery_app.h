#pragma once
// gallery 应用壳（wg2 迁移版）：
//   - 菜单、状态栏、侧面板、弹窗全部使用 Mk::wg2 静态树；
//   - 根节点通过 wg2 layout 回调 挂到旧引擎中央挂载点。
#include "inspector_window.h"
#include "popups.h"
#include "status_bar.h"
#include "menu_bar.h"
#include "content_area.h"
#include <imgui_mk/ui/wg2/texture.h>
#include <Mainwindow.h>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

namespace gallery {

// 程序生成 64x64 棋盘格纹理（无外部文件依赖）
inline ImTextureID makeCheckerTexture() {
	static unsigned char px[64 * 64 * 4];
	for(int y = 0; y < 64; ++y)
		for(int x = 0; x < 64; ++x) {
			const bool odd = ((x / 8) + (y / 8)) % 2 == 1;
			unsigned char* p = &px[(y * 64 + x) * 4];
			p[0] = odd ? 220 : 60; p[1] = odd ? 160 : 120;
			p[2] = odd ? 60 : 220; p[3] = 255;
		}
	return Mk::wg2::loadTextureRGBA(px, 64, 64);
}

namespace ui = Mk::wg2;   // 本文件用短名


// —— 主体三行 ——
using ContentRow = ui::HBox<ui::Id("contentRow"), ContentArea>;   // 中间内容区（页集合自成组件）
using AppBody    = ui::VBox<ui::Id("mainCol"), MenuBar, ContentRow, StatusBar>;

// —— 浮层 ——

// 应用壳的静态结构：主体 + 三个浮层
using ShellRoot = ui::Overlay<ui::Id("root"), AppBody, Popups>;   // 主体 + 浮层组件


struct GalleryWindow {

// wg2 应用壳

ShellRoot shell;

Mk::MainWindow<Mk::window>* winRef = nullptr;
	Mk::window* win = nullptr;
InspectorWindow inspector;

	GalleryWindow(Mk::MainWindow<Mk::window>& w)
		: winRef(&w), win(&w.getWindow()), inspector(w.getWindow()) {
		// 主题事件：本窗口组件自己应用
		Mk::mkui<Mk::window>::Events().on<ThemeChanged>([this](const ThemeChanged& e) {
			winRef->setTheme(e.theme);
		});
		menuBar().init(Mk::mkui<Mk::window>::Events(), winRef, [this] { popups().showAbout(); });
		initPanels();
		popups().init();
		statusBar().init(win, [this] { return currentPageTitle(); });
		initShell();

		contentArea().init(tex, win, inspector.win.get());   // 内容区注册 29 页并注入依赖
	}

	// —— 应用壳自己的数据（选中页 / 主题 / 侧栏演示值 / 弹窗开关 / 纹理句柄）——
	int sel = 0;	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	Mk::wg2::Theme theme = Mk::wg2::Theme::dark();
	bool darkActive = true;
	ImTextureID tex = makeCheckerTexture();   // 声明即创建



	// 树里的浮层组件实例
	MenuBar& menuBar() { return shell.at<ui::Id("mainCol"), ui::Id("menuBar")>(); }
	StatusBar& statusBar() { return shell.at<ui::Id("mainCol"), ui::Id("statusBar")>(); }
	Popups& popups() { return shell.at<ui::Id("popups")>(); }

	Mk::window* inspectorWin() { return inspector.win.get(); }

	// 换肤：自己的状态 + 通知各窗口组件
	void applyTheme(const Mk::wg2::Theme& t) {
		theme = t;
		darkActive = t.windowBg.x < 0.5f;
		Mk::mkui<Mk::window>::Events().emit(ThemeChanged{t});
	}

	template<class Content>
	void addPage(PageFrame<Content>& page, const char* group = nullptr) {
		page.setup();   // 装配外壳：读 Content::title / desc / code，并把演示槽挂到 content.root
		auto& book = contentArea().book;
		if (group && *group) book.addGroup(group);
		book.addPage(page.title, [&page](const Mk::wg2::Rect& r) { page.renderTo(r); });
	}

// 页面总数（导航条目中非组标题的数量）：供键盘翻页等使用
int pageCount() {
int n = 0;
for (const auto& e : contentArea().book.nav.entries) {
if (!e.header) ++n;
}
return n;
}



private:

	// 树里的内容区实例（页簿与页面都在这棵树上）
	ContentArea& contentArea() {
		return shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("content")>();
	}

	void initPanels() {
		contentArea().book.nav.current = &sel;      // 导航与宿主共用一个选中下标
		contentArea().book.host.current = &sel;
		contentArea().book.host.state.stretch = 1.0f;
	}



	void initShell() {
		// 侧栏宽度用 em：字号变化时布局自动跟随
		contentArea().state.stretch = 1.0f;                 // 内容区吃掉整行剩余宽度
		contentArea().book.navWidthEm = 9.3f;
		contentArea().book.nav.state.size.prefWEm = 9.3f;
		contentArea().book.nav.state.size.minWEm = 5.6f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().marginX = 0.0f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().marginY = 0.0f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().state.stretch = 1.0f;
		shell.at<Mk::wg2::Id("mainCol")>().marginX = 0.0f;
		shell.at<Mk::wg2::Id("mainCol")>().marginY = 0.0f;
		shell.marginX = 0.0f;
		shell.marginY = 0.0f;
	}


const char* currentPageTitle() {
const int target = contentArea().book.nav.current ? *contentArea().book.nav.current : 0;
int i = 0;
 for (const auto& e : contentArea().book.nav.entries) {
if (!e.header) {
if (i++ == target) return e.label.c_str();
}
}
return "(none)";
}
};

} // namespace gallery
