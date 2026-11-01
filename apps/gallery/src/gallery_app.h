#pragma once
// gallery 应用壳（wg2 迁移版）：
//   - 菜单、状态栏、侧面板、弹窗全部使用 Mk::wg2 静态树；
//   - 根节点通过 wg2 layout 回调 挂到旧引擎中央挂载点。
#include "gallery_inspector.h"
#include "gallery_pages.h"
#include "gallery_nav.h"
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

// —— 菜单 ——
using FileMenu   = ui::Menu<ui::Id("menuFile"),   ui::MenuItem<ui::Id("miOpen")>, ui::MenuItem<ui::Id("miExit")>>;
using ThemeMenu  = ui::Menu<ui::Id("menuTheme"),  ui::MenuItem<ui::Id("miDark")>, ui::MenuItem<ui::Id("miLight")>>;
using WindowMenu = ui::Menu<ui::Id("menuWindow"), ui::MenuItem<ui::Id("miInspector")>>;
using HelpMenu   = ui::Menu<ui::Id("menuHelp"),   ui::MenuItem<ui::Id("miAbout")>>;
using AppMenuBar = ui::MenuBar<ui::Id("menuBar"), FileMenu, ThemeMenu, WindowMenu, HelpMenu>;

// —— 主体三行 ——
using SidePanel  = ui::SubWindow<ui::Id("sidePanel"), ui::SliderF<ui::Id("panelSlider")>, ui::LabelText<ui::Id("panelFrame")>>;
using ContentRow = ui::HBox<ui::Id("contentRow"), ui::PageBook<ui::Id("book")>, SidePanel>;
using StatusRow  = ui::HBox<ui::Id("statusRow"), ui::LabelText<ui::Id("stFps")>, ui::LabelText<ui::Id("stFrame")>, ui::LabelText<ui::Id("stPage")>>;
using AppBody    = ui::VBox<ui::Id("mainCol"), AppMenuBar, ContentRow, StatusRow>;

// —— 浮层 ——
using PopupTree = ui::Popup<ui::Id("popupTree"), ui::Label<ui::Id("popupText")>, ui::Button<ui::Id("popupClose")>>;
using ModalTree = ui::PopupModal<ui::Id("modalTree"), ui::Label<ui::Id("modalText")>, ui::Button<ui::Id("modalClose")>>;
using AboutTree = ui::PopupModal<ui::Id("aboutTree"), ui::Label<ui::Id("aboutText")>, ui::Button<ui::Id("aboutOk")>>;

// 应用壳的静态结构：主体 + 三个浮层
using ShellRoot = ui::Overlay<ui::Id("root"), AppBody, PopupTree, ModalTree, AboutTree>;


struct GalleryWindow {

// wg2 应用壳

ShellRoot shell;

Mk::MainWindow<Mk::window>* winRef = nullptr;
	Mk::window* win = nullptr;
InspectorWindow inspector;

	GalleryWindow(Mk::MainWindow<Mk::window>& w)
		: winRef(&w), win(&w.getWindow()), inspector(w.getWindow()) {
		// 主题事件：本窗口组件自己应用
		Mk::mkui<Mk::window>::RegistrationWindow().on<ThemeChanged>([this](const ThemeChanged& e) {
			winRef->setTheme(e.theme);
		});
		initMenus();
		initPanels();
		initPopups();
		initStatus();
		initShell();

		// 注册表：{页, 分组}；分组为空 = 沿用上一组
		const std::pair<Page*, const char*> kRegistry[] = {
			{&pageLayoutBasic.page, kGroupLayout}, {&pageLayoutSize.page, nullptr},
			{&pageLayoutGrid.page, nullptr}, {&pageLayoutRuntime.page, nullptr},
			{&pageLayoutDynamic.page, nullptr}, {&pageBase.page, kGroupWg},
			{&pageLifecycle.page, nullptr}, {&pageWg2.page, nullptr},
			{&pageWg2Catalog.page, nullptr}, {&pageLayout.page, nullptr},
			{&pageSplit.page, nullptr}, {&pageText.page, kGroupAtoms},
			{&pageButton.page, nullptr}, {&pageInput.page, nullptr},
			{&pageNumber.page, nullptr}, {&pageDrag.page, nullptr},
			{&pageSelect.page, nullptr}, {&pageFeedback.page, nullptr},
			{&pageContainer.page, kGroupContainers}, {&pageTab.page, nullptr},
			{&pageMenu.page, nullptr}, {&pageTable.page, nullptr},
			{&pagePopup.page, kGroupPopColor}, {&pageColor.page, nullptr},
			{&pageDeco.page, kGroupDeco}, {&pageTheme.page, kGroupApp},
			{&pageWindow.page, nullptr}, {&pageFile.page, nullptr},
			{&pageFnRender.page, nullptr},
		};
		for (const auto& [page, group] : kRegistry) addPage(*page, group);
	}

	// —— 应用壳自己的数据（选中页 / 主题 / 侧栏演示值 / 弹窗开关 / 纹理句柄）——
	int sel = 0;
	float panelVolume = 0.6f;
	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	Mk::wg2::Theme theme = Mk::wg2::Theme::dark();
	bool darkActive = true;
	ImTextureID tex = makeCheckerTexture();   // 声明即创建

	// —— 29 个演示页：由应用壳持有并注册（main 不再关心）——
	gallery::PageLayoutBasic   pageLayoutBasic;
	gallery::PageLayoutSize    pageLayoutSize;
	gallery::PageLayoutGrid    pageLayoutGrid;
	gallery::PageLayoutRuntime pageLayoutRuntime;
	gallery::PageLayoutDynamic pageLayoutDynamic;
	gallery::PageBase          pageBase;
	gallery::PageLifecycle     pageLifecycle;
	gallery::PageWg2           pageWg2;
	gallery::PageWg2Catalog    pageWg2Catalog;
	gallery::PageLayout        pageLayout;
	gallery::PageSplit         pageSplit;
	gallery::PageText          pageText;
	gallery::PageInput         pageInput;
	gallery::PageNumber        pageNumber;
	gallery::PageDrag          pageDrag;
	gallery::PageSelect        pageSelect;
	gallery::PageContainer     pageContainer;
	gallery::PageTab           pageTab;
	gallery::PageTable         pageTable;
	gallery::PagePopup         pagePopup;
	gallery::PageColor         pageColor;
	gallery::PageDeco          pageDeco;
	gallery::PageTheme         pageTheme;
	gallery::PageFnRender      pageFnRender;
	gallery::PageButton        pageButton{tex};
	gallery::PageFeedback      pageFeedback{win};
	gallery::PageMenu          pageMenu{win};
	gallery::PageWindow        pageWindow{win, inspector.win.get()};
	gallery::PageFile          pageFile{win};

	Mk::window* inspectorWin() { return inspector.win.get(); }

	// 换肤：自己的状态 + 通知各窗口组件
	void applyTheme(const Mk::wg2::Theme& t) {
		theme = t;
		darkActive = t.windowBg.x < 0.5f;
		Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{t});
	}

void addPage(Page& page, const char* group = nullptr) {
if (group && *group) shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.addEntry(group, true);
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.addEntry(page.title, false);
 auto render = [&page](const Mk::wg2::Rect& r) { page.renderTo(r); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().host.addPage(render);
}

// 页面总数（导航条目中非组标题的数量）：供键盘翻页等使用
int pageCount() {
int n = 0;
for (const auto& e : shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.entries) {
if (!e.header) ++n;
}
return n;
}



private:
void initMenus() {
auto& f = shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile")>();
f.name = "File";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().name = "Open...";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().shortcut = "Ctrl+O";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().onClick = [] {
		Mk::mkui<Mk::window>::RegistrationWindow().emit(OpenFile{});
	};
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miExit")>().name = "Exit";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miExit")>().onClick = [&] {
winRef->requestClose();
};

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme")>().name = "Theme";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().name = "Dark";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().onClick = [this] { applyTheme(Theme::dark()); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().selectedFn = [this] { return darkActive; };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().name = "Light";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().onClick = [this] { applyTheme(Theme::light()); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().selectedFn = [this] { return !darkActive; };

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow")>().name = "Window";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow"), Mk::wg2::Id("miInspector")>().name = "Show/Hide Inspector";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow"), Mk::wg2::Id("miInspector")>().onClick = [] {
		Mk::mkui<Mk::window>::RegistrationWindow().emit(ToggleInspector{});
	};

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp")>().name = "Help";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp"), Mk::wg2::Id("miAbout")>().name = "About Gallery";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp"), Mk::wg2::Id("miAbout")>().onClick = [this] { aboutOpen = true; };
}

void initPanels() {
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.current = &sel;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().host.current = &sel;


shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().host.state.stretch = 1.0f;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().idname = "sidepanel";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().border = true;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelSlider")>().name = "Panel slider";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelSlider")>().value = &panelVolume;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelFrame")>().name = "Frame";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelFrame")>().valueFn = [this] {
return std::to_string(win->frameIndex());
};
}

void initPopups() {
shell.at<Mk::wg2::Id("popupTree")>().idname = "gpop";
shell.at<Mk::wg2::Id("popupTree")>().open = &popupOpen;
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupText")>().text = "A regular popup.";
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupClose")>().label = "关闭";
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupClose")>().onClick = [this] { popupOpen = false; ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("modalTree")>().idname = "gmodal";
shell.at<Mk::wg2::Id("modalTree")>().open = &modalOpen;
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalText")>().text = "A modal popup blocks interaction with the host.";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().label = "关闭";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().onClick = [this] { modalOpen = false; ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("aboutTree")>().idname = "about";
shell.at<Mk::wg2::Id("aboutTree")>().open = &aboutOpen;
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutText")>().text =
"mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg2 template struct widget library";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().label = "OK";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().onClick = [this] { aboutOpen = false; ImGui::CloseCurrentPopup(); };
}

void initStatus() {
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFps")>().name = "FPS";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFps")>().valueFn = [] {
return std::to_string(static_cast<int>(ImGui::GetIO().Framerate));   // 单位由 name="FPS" 显示，避免重复
};
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFrame")>().name = "帧";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFrame")>().valueFn = [this] { return std::to_string(win->frameIndex()); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stPage")>().name = "页";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stPage")>().valueFn = [this] { return std::string(currentPageTitle()); };
}

void initShell() {
		// 侧栏/侧面板宽度用 em 表达：字号变化时布局自动跟随，无需每帧同步
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.state.size.prefWEm = 9.3f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.state.size.minWEm = 5.6f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.prefWEm = 9.6f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.minWEm = 6.7f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().marginX = 0.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().marginY = 0.0f;
 shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow")>().state.stretch = 1.0f;
 shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow")>().marginX = 0.0f;
 shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow")>().marginY = 0.0f;
 shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow")>().spacing = 12.0f;
shell.at<Mk::wg2::Id("mainCol")>().marginX = 0.0f;
shell.at<Mk::wg2::Id("mainCol")>().marginY = 0.0f;
shell.marginX = 0.0f;
shell.marginY = 0.0f;
}


const char* currentPageTitle() {
const int target = shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.current ? *shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.current : 0;
int i = 0;
 for (const auto& e : shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("book")>().nav.entries) {
if (!e.header) {
if (i++ == target) return e.label.c_str();
}
}
return "(none)";
}
};

} // namespace gallery
