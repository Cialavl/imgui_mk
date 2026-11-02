#pragma once
// 顶部菜单栏：File / Theme / Window / Help
//   setup(ctx)：菜单从控件树上下文拿事件/窗口，自己完成装配；
//   布局内容 = root，浮层 = about（Help → About 弹窗）；两者都由框架驱动：
//   容器经 hintOf/policyOf 取尺寸，drawWidget 画 root 后提交 overlays()。
#include <imgui_mk/ui_context.h>
#include <imgui_mk/ui/wg2/wg2.h>

#include <tuple>

namespace gallery {

// 菜单发出的事件（发布方声明，消费方 include 本头文件即可）
struct ToggleInspector {};   // 显示/隐藏检查器
struct OpenFile {};          // 请求打开文件（文件页处理）

namespace ui = Mk::wg2;

struct MenuBar {
	static constexpr auto id = ui::Id("menuBar");

	using Root = ui::MenuBar<ui::Id("menuBarInner"),
		ui::Menu<ui::Id("menuFile"),   ui::MenuItem<ui::Id("miOpen")>, ui::MenuItem<ui::Id("miExit")>>,
		ui::Menu<ui::Id("menuTheme"),  ui::MenuItem<ui::Id("miDark")>, ui::MenuItem<ui::Id("miLight")>>,
		ui::Menu<ui::Id("menuWindow"), ui::MenuItem<ui::Id("miInspector")>>,
		ui::Menu<ui::Id("menuHelp"),   ui::MenuItem<ui::Id("miAbout")>>>;
	Root root;

	// 菜单触发的弹窗：开合状态与界面都归菜单自己
	using AboutPopup = ui::PopupModal<ui::Id("aboutPopup"),
		ui::Label<ui::Id("aboutText")>,
		ui::Button<ui::Id("aboutOk")>>;
	AboutPopup about;

	// 浮层声明：框架只提交、不参与布局
	auto overlays() { return std::tie(about); }

	bool darkActive = true;   // 供主题菜单勾选态

	// 控件树初始化（框架沿树自动调用）：菜单只依赖上下文（事件通道 + 主窗口）
	void setup(Mk::UiContext& ctx) {
		Mk::EventBus& bus = *ctx.bus;

		auto& file = root.at<ui::Id("menuFile")>();
		file.name = "File";
		auto& open = root.at<ui::Id("menuFile"), ui::Id("miOpen")>();
		open.name = "Open...";
		open.shortcut = "Ctrl+O";
		open.onClick = [&bus] { bus.emit(OpenFile{}); };
		auto& exit_ = root.at<ui::Id("menuFile"), ui::Id("miExit")>();
		exit_.name = "Exit";
		exit_.onClick = [window = ctx.window] { if (window) window->requestClose(); };

		auto& theme = root.at<ui::Id("menuTheme")>();
		theme.name = "Theme";
		auto& dark = root.at<ui::Id("menuTheme"), ui::Id("miDark")>();
		dark.name = "Dark";
		dark.onClick = [&bus] { bus.emit(Mk::ThemeChanged{Mk::wg2::Theme::dark()}); };   // 换肤走框架广播
		dark.selectedFn = [this] { return darkActive; };
		auto& light = root.at<ui::Id("menuTheme"), ui::Id("miLight")>();
		light.name = "Light";
		light.onClick = [&bus] { bus.emit(Mk::ThemeChanged{Mk::wg2::Theme::light()}); };
		light.selectedFn = [this] { return !darkActive; };

		auto& window = root.at<ui::Id("menuWindow")>();
		window.name = "Window";
		auto& insp = root.at<ui::Id("menuWindow"), ui::Id("miInspector")>();
		insp.name = "Show/Hide Inspector";
		insp.onClick = [&bus] { bus.emit(ToggleInspector{}); };

		auto& help = root.at<ui::Id("menuHelp")>();
		help.name = "Help";
		auto& aboutItem = root.at<ui::Id("menuHelp"), ui::Id("miAbout")>();
		aboutItem.name = "About Gallery";
		aboutItem.onClick = [this] { about.open = true; };   // 菜单只投递打开请求

		// 主题变化时更新勾选态（菜单自己跟踪，不依赖应用壳）
		bus.on<Mk::ThemeChanged>([this](const Mk::ThemeChanged& e) {
			darkActive = e.theme.windowBg.x < 0.5f;
		});

		// 菜单自己持有的 About 弹窗
		about.idname = "about";
		about.at<ui::Id("aboutText")>().text =
			"mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg2 template struct widget library";
		auto& ok = about.at<ui::Id("aboutOk")>();
		ok.label = "OK";
		ok.onClick = [] { ImGui::CloseCurrentPopup(); };
	}
};

} // namespace gallery
