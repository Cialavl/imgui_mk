#pragma once
// p19 多窗口：双顶层窗口各自的帧计数 + 显隐切换（事件）
#include "../../demo_page.h"
#include "../../../menu_bar.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageWindow {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "多窗口链接";
	static constexpr const char* desc  = "两个顶层窗口各自一个 ImGui 上下文与帧循环；绑定 wg2 控件到同一批外部变量 = 实时链接。";
	static constexpr std::string_view code = snips::kMultiWindow;

	using Root = ui::VBox<
		ui::Id("root"),
		ui::LabelText<ui::Id("gal")>,
		ui::LabelText<ui::Id("ins")>,
		ui::Button<ui::Id("toggle")>,
		ui::TextWrapped<ui::Id("note")>
	>;

	Root root;

	// 构造参数存成成员：lambda 若用 [&] 捕获参数会悬垂（参数出构造即亡）
	Mk::window* win = nullptr;
	Mk::window* inspectorWin = nullptr;

	PageWindow(Mk::window* w = nullptr, Mk::window* ins = nullptr) : win(w), inspectorWin(ins) {
		

		auto& w_gal = root.at<ui::Id("gal")>();
		w_gal.name = "Gallery 帧";
		w_gal.valueFn = [this] { return std::to_string((win ? win->frameIndex() : 0ull)); };
		auto& w_ins = root.at<ui::Id("ins")>();
		w_ins.name = "Inspector 帧";
		w_ins.valueFn = [this] { return inspectorWin ? std::to_string(inspectorWin->frameIndex()) : std::string("-"); };
		auto& w_toggle = root.at<ui::Id("toggle")>();
		w_toggle.label = "显示/隐藏 Inspector";
		w_toggle.onClick = [] {
			Mk::mkui<Mk::window>::Events().emit(ToggleInspector{});
		};
		root.at<ui::Id("note")>().text = "Inspector 只隐藏；Gallery 关闭后跟随退出。";

	}
};

} // namespace gallery
