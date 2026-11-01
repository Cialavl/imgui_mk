#pragma once
// p15 弹窗：Popup/PopupModal（wg2 open 标志驱动）
#include "../../gallery_chrome.h"

namespace gallery {

struct PagePopup {
using Root = VBox<
Id("root"),
HBox<Id("buttons"),
Button<Id("popBtn")>,
Button<Id("modalBtn")>
>,
Label<Id("note")>,
Popup<Id("pop"),
Label<Id("popText")>
>,
PopupModal<Id("modal"),
Label<Id("modalText")>,
Button<Id("modalClose")>
>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	bool popupOpen = false;
	bool modalOpen = false;

PagePopup() {
page.setup("弹窗",
"wg2 Popup/PopupModal：open 标志位 + 帧内 OpenPopup 自动配对。",
snips::kPopup);

auto& w_popBtn = root.at<Id("buttons"), Id("popBtn")>();
w_popBtn.label = "打开普通弹窗";
w_popBtn.onClick = [&] { popupOpen = true; };
auto& w_modalBtn = root.at<Id("buttons"), Id("modalBtn")>();
w_modalBtn.label = "打开模态弹窗";
w_modalBtn.onClick = [&] { modalOpen = true; };
root.at<Id("note")>().text = "模态弹窗阻挡主窗口交互，普通弹窗可随处关闭。";
auto& w_pop = root.at<Id("pop")>();
w_pop.open = &popupOpen;
w_pop.idname = "wg2pop";
root.at<Id("pop"), Id("popText")>().text = "wg2 普通弹窗";
auto& w_modal = root.at<Id("modal")>();
w_modal.open = &modalOpen;
w_modal.idname = "wg2modal";
root.at<Id("modal"), Id("modalText")>().text = "wg2 模态弹窗";
	auto& w_modalClose = root.at<Id("modal"), Id("modalClose")>();
	w_modalClose.label = "关闭";
	w_modalClose.onClick = [this] {
		modalOpen = false;
		ImGui::CloseCurrentPopup();
	};

bindPage(page, root);
}

};

} // namespace gallery
