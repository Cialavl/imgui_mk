#pragma once
// gallery 应用壳（wg2 迁移版）：
//   - 菜单、状态栏、侧面板、弹窗全部使用 Mk::wg2 静态树；
//   - 根节点通过 wg2 layout 回调 挂到旧引擎中央挂载点。
#include "gallery_inspector.h"
#include "gallery_pages.h"
#include "gallery_wg2_bridge.h"
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

State* sRef = nullptr;
Mk::MainWindow<Mk::window, State>* winRef = nullptr;
InspectorWindow inspector;

GalleryWindow(Mk::MainWindow<Mk::window, State>& win, State& s)
: sRef(&s), winRef(&win), inspector(win.getWindow(), s) {
initMenus(s);
initPanels(s);
initPopups(s);
initStatus(s);
initShell();
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
void initMenus(State& s) {
auto& f = shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile")>();
f.name = "File";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().name = "Open...";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().shortcut = "Ctrl+O";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miOpen")>().onClick = [&] { s.openFileDlg(); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miExit")>().name = "Exit";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuFile"), Mk::wg2::Id("miExit")>().onClick = [&] {
glfwSetWindowShouldClose(winRef->getHandle(), true);
};

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme")>().name = "Theme";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().name = "Dark";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().onClick = [&] { s.applyTheme(Theme::dark()); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miDark")>().selectedFn = [&s] { return s.darkActive; };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().name = "Light";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().onClick = [&] { s.applyTheme(Theme::light()); };
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuTheme"), Mk::wg2::Id("miLight")>().selectedFn = [&s] { return !s.darkActive; };

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow")>().name = "Window";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow"), Mk::wg2::Id("miInspector")>().name = "Show/Hide Inspector";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuWindow"), Mk::wg2::Id("miInspector")>().onClick = [&] { s.toggleInspector(); };

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp")>().name = "Help";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp"), Mk::wg2::Id("miAbout")>().name = "About Gallery";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("menuBar"), Mk::wg2::Id("menuHelp"), Mk::wg2::Id("miAbout")>().onClick = [&] { s.aboutOpen = true; };
}

void initPanels(State& s) {
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().current = &s.sel;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("host")>().current = &s.sel;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.prefW = 250.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("nav")>().state.size.minW = 150.0f;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("host")>().state.stretch = 1.0f;

shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().idname = "sidepanel";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().border = true;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.prefW = 260.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel")>().state.size.minW = 180.0f;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelSlider")>().name = "Panel slider";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelSlider")>().value = &s.volume;
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelFrame")>().name = "Frame";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("contentRow"), Mk::wg2::Id("sidePanel"), Mk::wg2::Id("panelFrame")>().valueFn = [&s] {
return std::to_string(s.win->frameIndex());
};
}

void initPopups(State& s) {
shell.at<Mk::wg2::Id("popupTree")>().idname = "gpop";
shell.at<Mk::wg2::Id("popupTree")>().open = &s.popupOpen;
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupText")>().text = "A regular popup.";
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupClose")>().label = "关闭";
shell.at<Mk::wg2::Id("popupTree"), Mk::wg2::Id("popupClose")>().onClick = [] { ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("modalTree")>().idname = "gmodal";
shell.at<Mk::wg2::Id("modalTree")>().open = &s.modalOpen;
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalText")>().text = "A modal popup blocks interaction with the host.";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().label = "关闭";
shell.at<Mk::wg2::Id("modalTree"), Mk::wg2::Id("modalClose")>().onClick = [] { ImGui::CloseCurrentPopup(); };

shell.at<Mk::wg2::Id("aboutTree")>().idname = "about";
shell.at<Mk::wg2::Id("aboutTree")>().open = &s.aboutOpen;
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutText")>().text =
"mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg2 template struct widget library";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().label = "OK";
shell.at<Mk::wg2::Id("aboutTree"), Mk::wg2::Id("aboutOk")>().onClick = [] { ImGui::CloseCurrentPopup(); };
}

void initStatus(State& s) {
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFps")>().name = "FPS";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFps")>().valueFn = [] {
return std::to_string(static_cast<int>(ImGui::GetIO().Framerate));   // 单位由 name="FPS" 显示，避免重复
};
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFrame")>().name = "帧";
shell.at<Mk::wg2::Id("mainCol"), Mk::wg2::Id("statusRow"), Mk::wg2::Id("stFrame")>().valueFn = [&s] { return std::to_string(s.win->frameIndex()); };
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
