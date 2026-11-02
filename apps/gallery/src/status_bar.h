#pragma once
// 底部状态栏：页面 / FPS / 帧号
//   setup(ctx)：帧号取上下文主窗口，页标题取内容区在 ready 阶段登记的服务。
//   样式：框架 Panel（卡片：底色 + 圆角 + 描边 + 内边距），应用层不直接碰 ImGui 绘制。
#include "content_area/demo_page.h"
#include <imgui_mk/ui_context.h>

#include <string>

namespace gallery {

namespace ui = Mk::wg2;

struct StatusBar {
	static constexpr auto id = Mk::wg2::Id("statusBar");

	// 左：页面标题；右：FPS / 帧号
	using Row = ui::HBox<ui::Id("statusRow"),
		ui::Label<ui::Id("stPageLabel")>,
		ui::Label<ui::Id("stPageTitle")>,
		ui::Spacer<ui::Id("stGrow")>,
		ui::LabelText<ui::Id("stFps")>,
		ui::LabelText<ui::Id("stFrame")>>;
	using Root = ui::Card<ui::Id("statusCard"), Row>;
	Root root;

	Mk::UiContext* ctx_ = nullptr;

	void setup(Mk::UiContext& ctx) {
		ctx_ = &ctx;

		// 卡片：左右贴页面列，上下各 4px 外边距；文字内缩与页面标题同一条线
		root.marginX = 0.0f;
		root.marginY = 4.0f;
		root.paddingX = ImGui::GetStyle().WindowPadding.x + 12.0f;
		root.paddingY = 4.0f;
		root.border = true;
		root.background = ImGuiCol_MenuBarBg;

		auto& row = root.child;
		row.marginX = 0.0f;
		row.marginY = 0.0f;
		row.spacing = 8.0f;

		auto& page = row.at<ui::Id("stPageLabel")>();
		page.text = "页面";
		page.textColor = ImGuiCol_TextDisabled;

		row.at<ui::Id("stPageTitle")>().textFn = [this] {
			const char* title = (ctx_ && ctx_->pageTitle) ? ctx_->pageTitle() : "";
			return std::string(title ? title : "");
		};

		auto& fps = row.at<ui::Id("stFps")>();
		fps.name = "FPS";
		fps.valueFn = [] { return std::to_string(static_cast<int>(ImGui::GetIO().Framerate)); };

		auto& frame = row.at<ui::Id("stFrame")>();
		frame.name = "帧";
		frame.valueFn = [window = ctx.window] { return std::to_string(window ? window->frameIndex() : 0ull); };
	}
};

} // namespace gallery
