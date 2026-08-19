#pragma once
// p15 弹窗：Popup/PopupModal（open 标志驱动）
#include "../gallery_chrome.h"

namespace gallery {

struct PagePopup {
	Button popBtn, modalBtn;
	Label popNote;

	Page page;
	PagePopup(State& s) {
		page.setup("弹窗",
			"弹窗形式：open 标志位 + 帧内 OpenPopup 自动配对。普通弹窗是常驻点；"
			"Popup/PopupModal 由本页按钮只改标志位，ID 用 node.idname。",
			snips::kPopup);
		popBtn.self.setName("打开普通弹窗");
		popBtn.self.onClick = [&] { s.popupOpen = true; };
		modalBtn.self.setName("打开模态弹窗");
		modalBtn.self.onClick = [&] { s.modalOpen = true; };
		popNote.self.setName("模态弹窗阻挡主窗口交互，普通弹窗可随处关闭。");
		page.addWidget(popBtn);
		page.addWidget(modalBtn);
		page.addWidget(popNote);
	}
};

} // namespace gallery
