#pragma once
// p15 弹窗：Popup/PopupModal（控件自持 open 请求位）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名

// —— 按钮行 / 普通弹窗 / 模态弹窗 ——
using ActionRow = ui::HBox<ui::Id("buttons"),
	ui::Button<ui::Id("popBtn")>,
	ui::Button<ui::Id("modalBtn")>>;

using NormalPopup = ui::Popup<ui::Id("pop"),
	ui::Label<ui::Id("popText")>>;

using ModalPopup = ui::PopupModal<ui::Id("modal"),
	ui::Label<ui::Id("modalText")>,
	ui::Button<ui::Id("modalClose")>>;

struct PagePopup {
	static constexpr const char* title = "弹窗";
	static constexpr const char* desc  = "wg2 Popup/PopupModal：控件自持 open 请求位，帧内 OpenPopup 自动配对。";
	static constexpr std::string_view code = snips::kPopup;

	// 根只列结构：按钮行 / 注解 / 两种弹窗
	using Root = ui::VBox<ui::Id("root"),
		ActionRow,
		ui::Label<ui::Id("note")>,
		NormalPopup,
		ModalPopup>;

	Root root;

	PagePopup() {
		root.at<ui::Id("note")>().text = "模态弹窗阻挡主窗口交互，普通弹窗可随处关闭。";

		// 开关归弹窗控件自己：按钮只投递“请求打开”
		auto& w_pop = root.at<ui::Id("pop")>();
		w_pop.idname = "wg2pop";
		root.at<ui::Id("pop"), ui::Id("popText")>().text = "wg2 普通弹窗";

		auto& w_popBtn = root.at<ui::Id("buttons"), ui::Id("popBtn")>();
		w_popBtn.label = "打开普通弹窗";
		w_popBtn.onClick = [&w_pop] { w_pop.open = true; };

		auto& w_modal = root.at<ui::Id("modal")>();
		w_modal.idname = "wg2modal";
		root.at<ui::Id("modal"), ui::Id("modalText")>().text = "wg2 模态弹窗";

		auto& w_modalBtn = root.at<ui::Id("buttons"), ui::Id("modalBtn")>();
		w_modalBtn.label = "打开模态弹窗";
		w_modalBtn.onClick = [&w_modal] { w_modal.open = true; };

		auto& w_close = root.at<ui::Id("modal"), ui::Id("modalClose")>();
		w_close.label = "关闭";
		w_close.onClick = [] { ImGui::CloseCurrentPopup(); };
	}
};

} // namespace gallery
