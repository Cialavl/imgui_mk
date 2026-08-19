#pragma once
// p16 颜色编辑：ColorEdit3/4 + ColorPicker3/4
#include "../gallery_chrome.h"

namespace gallery {

struct PageColor {
	ColorEdit3 colRgb;
	ColorEdit4 colRgba;
	ColorPicker3 colPick3;
	ColorPicker4 colPick4;
	HLayout colRow;
	Collapsing colColl;

	Page page;
	PageColor(State& s) {
		page.setup("颜色编辑",
			"ColorEdit3/4 与 ColorPicker3/4：std::array<float,3|4>* 绑定；拾取器可包进折叠容器。",
			snips::kColor);
		colRgb.self.name = "RGB";   colRgb.self.color = &s.col3;
		colRgba.self.name = "RGBA"; colRgba.self.color = &s.col4;
		colPick3.self.name = "Pick RGB";  colPick3.self.color = &s.pick3;
		colPick4.self.name = "Pick RGBA"; colPick4.self.color = &s.pick4;

		colRow.addWidget(colPick3.node);
		colRow.addWidget(colPick4.node);
		colColl.self.name = "拾取器（默认展开）";
		colColl.node.add(colRow.node);
		page.addWidget(colRgb);
		page.addWidget(colRgba);
		page.addWidget(colColl);
	}
};

} // namespace gallery
