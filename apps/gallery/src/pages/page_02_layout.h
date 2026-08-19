#pragma once
// p2 布局：HLayout 嵌 VLayout（独立布局类型组合演示）
#include "../gallery_chrome.h"

namespace gallery {

struct PageLayout {
	Label layIntro, layRight, layNote;
	SliderF laySlider;
	CheckBox layCheck;
	VLayout layCol;      // 内部 V
	HLayout layRow;      // 外部 H（嵌 col + 右标签）

	Page page;
	PageLayout(State& s) {
		page.setup("布局：Layout 嵌套",
			"独立布局类型 VLayout/HLayout/Wrap/Grid/SplitH/SplitV/Overlay（共享盒式求解器）。"
			"本页 = HLayout 嵌 VLayout，addWidget 装配；布局嵌布局自然递归。",
			snips::kLayout);
		layIntro.self.setName("布局步骤：measure（计算尺寸）→ arrange（定 bounds）→ draw");
		layRight.self.setName("H 行右格");
		laySlider.self.name = "音量";
		laySlider.self.value = &s.volume;
		layCheck.self.name = "静音";
		layCheck.self.checked = &s.mute;
		layNote.self.setName("布局数据约束：子控件在父 Layout 内，坐标传播全程一致。");

		layCol.addWidget(laySlider.node);
		layCol.addWidget(layCheck.node);

		layRow.addWidget(layCol.node);
		layRow.addWidget(layRight.node);
		page.addWidget(layIntro);
		page.addWidget(layRow);
		page.addWidget(layNote);
	}
};

} // namespace gallery
