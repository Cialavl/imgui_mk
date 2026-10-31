#pragma once
// p15 弹窗：Popup/PopupModal（wg2 open 标志驱动）
#include "../gallery_chrome.h"

namespace gallery {

struct PagePopup {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::HBox<Mk::wg2::Id("buttons"),
Mk::wg2::Button<Mk::wg2::Id("popBtn")>,
Mk::wg2::Button<Mk::wg2::Id("modalBtn")>
>,
Mk::wg2::Label<Mk::wg2::Id("note")>,
Mk::wg2::Popup<Mk::wg2::Id("pop"),
Mk::wg2::Label<Mk::wg2::Id("popText")>
>,
Mk::wg2::PopupModal<Mk::wg2::Id("modal"),
Mk::wg2::Label<Mk::wg2::Id("modalText")>
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

root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("popBtn")>().label = "打开普通弹窗";
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("popBtn")>().onClick = [&] { popupOpen = true; };
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("modalBtn")>().label = "打开模态弹窗";
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("modalBtn")>().onClick = [&] { modalOpen = true; };
root.at<Mk::wg2::Id("note")>().text = "模态弹窗阻挡主窗口交互，普通弹窗可随处关闭。";
root.at<Mk::wg2::Id("pop")>().open = &popupOpen;
root.at<Mk::wg2::Id("pop")>().idname = "wg2pop";
root.at<Mk::wg2::Id("pop"), Mk::wg2::Id("popText")>().text = "wg2 普通弹窗";
root.at<Mk::wg2::Id("modal")>().open = &modalOpen;
root.at<Mk::wg2::Id("modal")>().idname = "wg2modal";
root.at<Mk::wg2::Id("modal"), Mk::wg2::Id("modalText")>().text = "wg2 模态弹窗";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
