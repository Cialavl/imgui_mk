#pragma once
// gallery 应用壳（wg2 迁移版）：
//   - 菜单、状态栏、侧面板、弹窗全部使用 Mk::wg2 静态树；
//   - 根节点通过 wg2 layout 回调 挂到旧引擎中央挂载点。
#include "gallery_inspector.h"
#include "gallery_pages.h"
#include "gallery_wg2_bridge.h"
#include <Mainwindow.h>
#include <functional>
#include <memory>
#include <vector>

namespace gallery {

struct GalleryWindow {

// wg2 应用壳
using ShellRoot = Mk::wg2::Overlay<
Mk::wg2::Id("root"),
Mk::wg2::VBox<Mk::wg2::Id("mainCol"),
Mk::wg2::MenuBar<Mk::wg2::Id("menuBar"),
Mk::wg2::Menu<Mk::wg2::Id("menuFile"),
Mk::wg2::MenuItem<Mk::wg2::Id("miOpen")>,
Mk::wg2::MenuItem<Mk::wg2::Id("miExit")>
>,
Mk::wg2::Menu<Mk::wg2::Id("menuTheme"),
Mk::wg2::MenuItem<Mk::wg2::Id("miDark")>,
Mk::wg2::MenuItem<Mk::wg2::Id("miLight")>
>,
Mk::wg2::Menu<Mk::wg2::Id("menuWindow"),
Mk::wg2::MenuItem<Mk::wg2::Id("miInspector")>
>,
Mk::wg2::Menu<Mk::wg2::Id("menuHelp"),
Mk::wg2::MenuItem<Mk::wg2::Id("miAbout")>
>
>,
Mk::wg2::HBox<Mk::wg2::Id("contentRow"),
PageNavWg2<Mk::wg2::Id("nav")>,
PageHostWg2<Mk::wg2::Id("host")>,
Mk::wg2::SubWindow<Mk::wg2::Id("sidePanel"),
Mk::wg2::SliderF<Mk::wg2::Id("panelSlider")>,
Mk::wg2::LabelText<Mk::wg2::Id("panelFrame")>
>
>,
Mk::wg2::HBox<Mk::wg2::Id("statusRow"),
Mk::wg2::LabelText<Mk::wg2::Id("stFps")>,
Mk::wg2::LabelText<Mk::wg2::Id("stFrame")>,
Mk::wg2::LabelText<Mk::wg2::Id("stPage")>
>
>,
Mk::wg2::Popup<Mk::wg2::Id("popupTree"),
Mk::wg2::Label<Mk::wg2::Id("popupText")>,
Mk::wg2::Button<Mk::wg2::Id("popupClose")>
>,
Mk::wg2::PopupModal<Mk::wg2::Id("modalTree"),
Mk::wg2::Label<Mk::wg2::Id("modalText")>,
Mk::wg2::Button<Mk::wg2::Id("modalClose")>
>,
Mk::wg2::PopupModal<Mk::wg2::Id("aboutTree"),
Mk::wg2::Label<Mk::wg2::Id("aboutText")>,
Mk::wg2::Button<Mk::wg2::Id("aboutOk")>
>
>;

ShellRoot shell;

Mk::MainWindow<Mk::window>* winRef = nullptr;
	Mk::window* win = nullptr;
InspectorWindow inspector;

	GalleryWindow(Mk::MainWindow<Mk::window>& w)
		: winRef(&w), inspector(w.getWindow()) {
		win = &w.getWindow();
		// 主题事件：本窗口组件自己应用
		Mk::mkui<Mk::window>::RegistrationWindow().on<ThemeChanged>([this](const ThemeChanged& e) {
			winRef->setTheme(e.theme);
		});
		initMenus();
		initPanels();
		initPopups();
		initStatus();
		initShell();
	}

	// —— 应用壳自己的数据（选中页 / 主题 / 侧栏演示值 / 弹窗开关 / 纹理句柄）——
	int sel = 0;
	float panelVolume = 0.6f;
	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	Mk::wg2::Theme theme = Mk::wg2::Theme::dark();
	bool darkActive = true;
	ImTextureID tex{};

	Mk::window* inspectorWin() { return inspector.win.get(); }

	// 换肤：自己的状态 + 通知各窗口组件
	void applyTheme(const Mk::wg2::Theme& t) {
		theme = t;
		darkActive = t.windowBg.x < 0.5f;
		Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{t});
	}

template<typename P>
void addPage(P& page, const char* group = nullptr) {
if (group && *group) shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().addEntry(group, true);
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().addEntry(page.page.title, false);
 auto render = [&page](const Mk::wg2::Rect& r) {
 page.page.renderTo(r);
 };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("host")>().addPage(render);
}

// 页面总数（导航条目中非组标题的数量）：供键盘翻页等使用
int pageCount() {
int n = 0;
for (const auto& e : shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().entries) {
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
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().current = &sel;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("host")>().current = &sel;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.prefW = 250.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.minW = 150.0f;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("host")>().state.stretch = 1.0f;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().idname = "sidepanel";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().border = true;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.prefW = 260.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.minW = 180.0f;
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
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupClose")>().onClick = [] { ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("modalTree")>().idname = "gmodal";
shell.at<Mk::wg2::Id("modalTree")>().open = &modalOpen;
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalText")>().text = "A modal popup blocks interaction with the host.";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().label = "关闭";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().onClick = [] { ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("aboutTree")>().idname = "about";
shell.at<Mk::wg2::Id("aboutTree")>().open = &aboutOpen;
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutText")>().text =
"mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg2 template struct widget library";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().label = "OK";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().onClick = [] { ImGui::CloseCurrentPopup(); };
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
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.prefWEm = 9.3f;
		shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.minWEm = 5.6f;
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
const int target = shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().current ? *shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().current : 0;
int i = 0;
 for (const auto& e : shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().entries) {
if (!e.header) {
if (i++ == target) return e.label;
}
}
return "(none)";
}
};

} // namespace gallery
