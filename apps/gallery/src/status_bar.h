#pragma once
// 底部状态栏：FPS / 帧号 / 当前页标题
#include "demo_page.h"

#include <functional>

namespace gallery {

namespace ui = Mk::wg2;

struct StatusBar {
	static constexpr auto id = Mk::wg2::Id("statusBar");

	Mk::wg2::WidgetState<> state;

	using Root = ui::HBox<ui::Id("statusRow"),
		ui::LabelText<ui::Id("stFps")>,
		ui::LabelText<ui::Id("stFrame")>,
		ui::LabelText<ui::Id("stPage")>>;
	Root root;

	// win 提供帧号；title 由内容区提供（状态栏不碰页簿）
	void init(Mk::window* win, std::function<const char*()> title) {
		root.marginX = 0.0f;
		root.marginY = 0.0f;
		root.spacing = 12.0f;

		auto& fps = root.at<ui::Id("stFps")>();
		fps.name = "FPS";
		fps.valueFn = [] { return std::to_string(static_cast<int>(ImGui::GetIO().Framerate)); };

		auto& frame = root.at<ui::Id("stFrame")>();
		frame.name = "帧";
		frame.valueFn = [win] { return std::to_string(win ? win->frameIndex() : 0ull); };

		auto& page = root.at<ui::Id("stPage")>();
		page.name = "页";
		page.valueFn = [title = std::move(title)] { return std::string(title ? title() : ""); };
	}

	Mk::wg2::Size hint(const Mk::wg2::Size& a) const { return root.hint(a); }
	Mk::wg2::Policy policy() const { return root.policy(); }
	void draw(const Mk::wg2::Rect& r) { root.draw(r); }
};

} // namespace gallery
