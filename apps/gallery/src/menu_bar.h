#pragma once
// 顶部菜单栏：File / Theme / Window / Help
//   回调只做两件事：发事件、请求关窗——不依赖应用壳的数据
#include "demo_page.h"
#include <Mainwindow.h>
#include <functional>

namespace gallery {

namespace ui = Mk::wg2;

struct MenuBar {
	static constexpr auto id = Mk::wg2::Id("menuBar");

	Mk::wg2::WidgetState<> state;

	using FileMenu   = ui::Menu<ui::Id("menuFile"),   ui::MenuItem<ui::Id("miOpen")>, ui::MenuItem<ui::Id("miExit")>>;
	using ThemeMenu  = ui::Menu<ui::Id("menuTheme"),  ui::MenuItem<ui::Id("miDark")>, ui::MenuItem<ui::Id("miLight")>>;
	using WindowMenu = ui::Menu<ui::Id("menuWindow"), ui::MenuItem<ui::Id("miInspector")>>;
	using HelpMenu   = ui::Menu<ui::Id("menuHelp"),   ui::MenuItem<ui::Id("miAbout")>>;

	using Root = ui::MenuBar<ui::Id("menuBarInner"), FileMenu, ThemeMenu, WindowMenu, HelpMenu>;
	Root root;

	bool darkActive = true;   // 供主题菜单勾选态

	void init(Mk::EventBus& bus, Mk::MainWindow<Mk::window>* winRef, std::function<void()> onAbout) {
		auto& file = root.at<ui::Id("menuFile")>();
		file.name = "File";
		auto& open = root.at<ui::Id("menuFile"), ui::Id("miOpen")>();
		open.name = "Open...";
		open.shortcut = "Ctrl+O";
		open.onClick = [&bus] { bus.emit(OpenFile{}); };
		auto& exit_ = root.at<ui::Id("menuFile"), ui::Id("miExit")>();
		exit_.name = "Exit";
		exit_.onClick = [&bus, winRef] { if (winRef) winRef->requestClose(); };

		auto& theme = root.at<ui::Id("menuTheme")>();
		theme.name = "Theme";
		auto& dark = root.at<ui::Id("menuTheme"), ui::Id("miDark")>();
		dark.name = "Dark";
		dark.onClick = [winRef] { winRef->setTheme(Theme::dark()); };
		dark.selectedFn = [this] { return darkActive; };
		auto& light = root.at<ui::Id("menuTheme"), ui::Id("miLight")>();
		light.name = "Light";
		light.onClick = [winRef] { winRef->setTheme(Theme::light()); };
		light.selectedFn = [this] { return !darkActive; };

		auto& window = root.at<ui::Id("menuWindow")>();
		window.name = "Window";
		auto& insp = root.at<ui::Id("menuWindow"), ui::Id("miInspector")>();
		insp.name = "Show/Hide Inspector";
		insp.onClick = [&bus] { bus.emit(ToggleInspector{}); };

		auto& help = root.at<ui::Id("menuHelp")>();
		help.name = "Help";
		auto& about = root.at<ui::Id("menuHelp"), ui::Id("miAbout")>();
		about.name = "About Gallery";
		about.onClick = [onAbout = std::move(onAbout)] { if (onAbout) onAbout(); };

		// 主题变化时更新勾选态（菜单自己跟踪，不依赖壳）
		bus.on<Mk::ThemeChanged>([this](const Mk::ThemeChanged& e) {
			darkActive = e.theme.windowBg.x < 0.5f;
		});
	}

	Mk::wg2::Size hint(const Mk::wg2::Size& a) const { return root.hint(a); }
	Mk::wg2::Policy policy() const { return root.policy(); }
	void draw(const Mk::wg2::Rect& r) { root.draw(r); }
};

} // namespace gallery
