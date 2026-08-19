#pragma once
// p13 菜单面板：Panel（浮动窗口）/ FrameLabel
#include "../gallery_chrome.h"

namespace gallery {

struct PageMenu {
	SliderF panelSlider;
	FrameLabel panelFrame;
	Panel<> panel;
	Label panelNote;

	Page page;
	PageMenu(State& s) {
		page.setup("菜单与面板",
			"Panel：浮动小窗口（标题 = node.idname，可拖动）；MenuBar/Menu/MenuItem 在主窗口顶部。",
			snips::kPanel);
		panelSlider.self.name = "面板内滑块";
		panelSlider.self.value = &s.volume;
		panelFrame.self.frames = &s.frames;
		panel.node.idname = "面板（可拖动）";
		panel.node.add(panelSlider.node);
		panel.node.add(panelFrame.node);
		panelNote.self.setName("Panel 是浮动窗口；菜单栏是常驻点（主窗口顶部）。");
		page.addWidget(panel);
		page.addWidget(panelNote);
	}
};

} // namespace gallery
