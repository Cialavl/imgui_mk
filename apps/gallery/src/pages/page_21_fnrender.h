#pragma once
// p21 FnRender：裸 ImGui 绘制函数包装为 wg 节点
#include "../gallery_chrome.h"

namespace gallery {

struct PageFnRender {
	FnRender<FnDrawerImpl> fnView;
	Label fnNote;

	Page page;
	PageFnRender(State& s) {
		page.setup("FnRender 裸绘制",
			"裸 ImGui 绘制函数包装为 wg 节点（F 类型默认构造，状态用指针捕获）。",
			snips::kFnRender);
		fnView.self.fn = FnDrawerImpl{&s};
		fnNote.self.setName("上方进度条用裸 ImGui 函数绘制，由 FnRender 节点挂入 wg 布局。");
		page.addWidget(fnView.node);
		page.addWidget(fnNote.node);
	}
};

} // namespace gallery
