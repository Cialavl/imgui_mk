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

auto& _w1 = root.at<Id("buttons"), Id("popBtn")>();
_w1.label = "打开普通弹窗";
_w1.onClick = [&] { popupOpen = true; };
auto& _w2 = root.at<Id("buttons"), Id("modalBtn")>();
_w2.label = "打开模态弹窗";
_w2.onClick = [&] { modalOpen = true; };
root.at<Id("note")>().text = "模态弹窗阻挡主窗口交互，普通弹窗可随处关闭。";
auto& _w3 = root.at<Id("pop")>();
_w3.open = &popupOpen;
_w3.idname = "wg2pop";
root.at<Id("pop"), Id("popText")>().text = "wg2 普通弹窗";
auto& _w4 = root.at<Id("modal")>();
_w4.open = &modalOpen;
_w4.idname = "wg2modal";
root.at<Id("modal"), Id("modalText")>().text = "wg2 模态弹窗";
	auto& _w5 = root.at<Id("modal"), Id("modalClose")>();
	_w5.label = "关闭";
	_w5.onClick = [this] {
		modalOpen = false;
		ImGui::CloseCurrentPopup();
	};

bindPage(page, root);
}

};

} // namespace gallery
