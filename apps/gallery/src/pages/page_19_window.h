#pragma once
// p19 多窗口：双顶层窗口共享状态实时链接
#include "../gallery_chrome.h"

namespace gallery {

struct PageWindow {
	LabelText winGal, winIns;
	Button winToggle;
	TextWrapped winNote;

	Page page;
	PageWindow(State& s) {
		page.setup("多窗口链接",
			"两个顶层窗口各自一个 ImGui 上下文与帧循环；绑定 wg 控件到同一批外部变量 = 实时链接。",
			snips::kMultiWindow);
		winGal.self.name = "Gallery 帧";
		winGal.self.valueFn = [&s] { return std::to_string(s.frames); };
		winIns.self.name = "Inspector 帧";
		winIns.self.valueFn = [&s] { return std::to_string(s.insFrames); };
		winToggle.self.setName("显示/隐藏 Inspector");
		winToggle.self.onClick = [&] { s.toggleInspector(); };
		winNote.self.name = "拖动一个窗口的滑块，另一个同帧刷新（绑同一变量）。"
			" Inspector 只隐藏；Gallery 关闭后跟随退出。";
		page.addWidget(winGal);
		page.addWidget(winIns);
		page.addWidget(winToggle);
		page.addWidget(winNote);
	}
};

} // namespace gallery
