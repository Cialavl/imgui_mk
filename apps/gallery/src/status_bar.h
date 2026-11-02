#pragma once
// 底部状态栏：页面 / FPS / 帧号
//   setup(ctx)：帧号取上下文主窗口，页标题取内容区在 ready 阶段登记的服务。
//   样式：整条底栏铺底色 + 顶部细分隔线，和侧栏/页面形成一致的层级。
#include "content_area/demo_page.h"
#include <imgui_mk/ui_context.h>

#include <algorithm>
#include <functional>
#include <string>

namespace gallery {

namespace ui = Mk::wg2;

struct StatusBar {
	static constexpr auto id = Mk::wg2::Id("statusBar");

	// 左：页面标题；右：FPS / 帧号
	using Root = ui::HBox<ui::Id("statusRow"),
		ui::Label<ui::Id("stPageLabel")>,
		ui::Label<ui::Id("stPageTitle")>,
		ui::Spacer<ui::Id("stGrow")>,
		ui::LabelText<ui::Id("stFps")>,
		ui::LabelText<ui::Id("stFrame")>>;
	Root root;
	Mk::UiContext* ctx_ = nullptr;

	// 控件树初始化：状态栏只依赖上下文，不直接碰其它组件
	void setup(Mk::UiContext& ctx) {
		ctx_ = &ctx;
		root.marginX = 0.0f;
		root.marginY = 0.0f;
		root.spacing = 8.0f;   // 与全局 4/8/16 刻度一致

		root.at<ui::Id("stPageLabel")>().text = "页面";


		auto& fps = root.at<ui::Id("stFps")>();
		fps.name = "FPS";
		fps.valueFn = [] { return std::to_string(static_cast<int>(ImGui::GetIO().Framerate)); };

		auto& frame = root.at<ui::Id("stFrame")>();
		frame.name = "帧";
		frame.valueFn = [window = ctx.window] { return std::to_string(window ? window->frameIndex() : 0ull); };
	}

	// 浮起的小卡片：外边距 8、圆角随主题，内部左右留 12
	Size hint(const Size& avail) const {
		const Size h = root.hint(Size{std::max(0.0f, avail.w - 24.0f), 0.0f});
		return Size{avail.w, h.h + 16.0f};   // 上下各 8px 外边距
	}

	void draw(const Mk::wg2::Rect& r) {
		const ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		// 卡片左右边界与页面内容列一致，文字再内缩 12
		const Mk::wg2::Rect box{r.x, r.y + 4.0f, r.w, std::max(0.0f, r.h - 12.0f)};
		const ImVec2 p0(p.x + (box.x - r.x), p.y + (box.y - r.y));
		const ImVec2 p1(p0.x + box.w, p0.y + box.h);
		const float rad = ImGui::GetStyle().WindowRounding;
		dl->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_MenuBarBg), rad);
		dl->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_Border), rad, 0, 1.0f);

		// 次要文字统一走主题的 disabled 色
		// 页标题每帧拉取（内容区在 ready 阶段登记的服务）
		const char* title = (ctx_ && ctx_->pageTitle) ? ctx_->pageTitle() : "";
		root.at<ui::Id("stPageTitle")>().text = title ? title : "";

		const ImVec4 muted = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		Mk::wg2::styleOf(root.at<ui::Id("stPageLabel")>()).text = muted;

		const Mk::wg2::Rect row{box.x + 12.0f, box.y + 4.0f,
			std::max(0.0f, box.w - 24.0f), std::max(0.0f, box.h - 8.0f)};
		Mk::wg2::layout(root, row);
	}
};

} // namespace gallery
