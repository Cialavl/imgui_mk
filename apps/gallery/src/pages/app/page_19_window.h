#pragma once
// p19 多窗口：双顶层窗口各自的帧计数 + 显隐切换（事件）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageWindow {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "多窗口链接";
	static constexpr const char* desc  = "两个顶层窗口各自一个 ImGui 上下文与帧循环；绑定 wg2 控件到同一批外部变量 = 实时链接。";
	static constexpr std::string_view code = snips::kMultiWindow;

	using Root = VBox<
		Id("root"),
		LabelText<Id("gal")>,
		LabelText<Id("ins")>,
		Button<Id("toggle")>,
		TextWrapped<Id("note")>
	>;

	Root root;

	// 构造参数存成成员：lambda 若用 [&] 捕获参数会悬垂（参数出构造即亡）
	Mk::window* win = nullptr;
	Mk::window* inspectorWin = nullptr;

	PageWindow(Mk::window* w, Mk::window* ins) : win(w), inspectorWin(ins) {
		

		auto& w_gal = root.at<Id("gal")>();
		w_gal.name = "Gallery 帧";
		w_gal.valueFn = [this] { return std::to_string(win->frameIndex()); };
		auto& w_ins = root.at<Id("ins")>();
		w_ins.name = "Inspector 帧";
		w_ins.valueFn = [this] { return inspectorWin ? std::to_string(inspectorWin->frameIndex()) : std::string("-"); };
		auto& w_toggle = root.at<Id("toggle")>();
		w_toggle.label = "显示/隐藏 Inspector";
		w_toggle.onClick = [] {
			Mk::mkui<Mk::window>::RegistrationWindow().emit(ToggleInspector{});
		};
		root.at<Id("note")>().text = "Inspector 只隐藏；Gallery 关闭后跟随退出。";

	}
};

} // namespace gallery
