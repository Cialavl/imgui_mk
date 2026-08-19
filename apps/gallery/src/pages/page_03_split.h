#pragma once
// p3 分格模式：SplitH/SplitV/Wrap/Grid
#include "../gallery_chrome.h"

namespace gallery {

struct PageSplit {
	Label splitIntro, sfA, sfB, sfC, svA, svB;
	Label wA, wB, wC, wD;
	Label g00, g01, g02, g10, g11, g12;
	SplitHLayout splitH;  // SplitH：固定 + 1:2 权重
	SplitVLayout splitV;  // SplitV：固定 + stretch
	WrapLayout wrapRow;   // Wrap 模式
	GridLayout grid;      // Grid 网格

	Page page;
	PageSplit(State& s) {
		(void)s;
		page.setup("分格模式",
			"SplitH/SplitV：setFixedWidth 固定 + addWidget(w,stretch) 权重；Wrap 换行；"
			"Grid 固定列网格。分格需要明确尺寸（layout 传 host 尺寸）。",
			snips::kSplit);
		splitIntro.self.setName("分格演示：SplitH 权重分割 / SplitV 垂直分割 / Wrap 换行 / Grid 网格");
		sfA.self.setName("fixed 160");
		sfB.self.setName("stretch 1");
		sfC.self.setName("stretch 2");
		sfA.setFixedWidth(160.0f);



		splitH.addWidget(sfA.node);
		splitH.addWidget(sfB.node, 1.0f);
		splitH.addWidget(sfC.node, 2.0f);
		svA.self.setName("fixed 44");
		svB.self.setName("VSplit stretch");
		svA.setFixedHeight(44.0f);


		splitV.addWidget(svA.node);
		splitV.addWidget(svB.node, 1.0f);
		wA.self.setName("换行一");
		wB.self.setName("换行二（较长项）");
		wC.self.setName("换行三");
		wD.self.setName("换行四");

		wrapRow.addWidget(wA);
		wrapRow.addWidget(wB);
		wrapRow.addWidget(wC);
		wrapRow.addWidget(wD);

		g00.self.setName("格 0-0"); g01.self.setName("格 0-1"); g02.self.setName("格 0-2");
		g10.self.setName("格 1-0"); g11.self.setName("格 1-1"); g12.self.setName("格 1-2");

		grid.setCols(3);
		grid.addWidget(g00);
		grid.addWidget(g01);
		grid.addWidget(g02);
		grid.addWidget(g10);
		grid.addWidget(g11);
		grid.addWidget(g12);

		page.addWidget(splitIntro);
		page.addWidget(splitH);
		page.addWidget(splitV);
		page.addWidget(wrapRow);
		page.addWidget(grid);
	}
};

} // namespace gallery
