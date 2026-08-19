#pragma once
// p10 反馈指示：ProgressBar/PlotLines/Separator/SeparatorText
#include "../gallery_chrome.h"

namespace gallery {

struct PageFeedback {
	ProgressBar fbBar1, fbBar2;
	PlotLines fbPlot;
	Separator fbSep;
	SeparatorText fbSection;

	Page page;
	PageFeedback(State& s) {
		page.setup("反馈与指示",
			"ProgressBar（fraction 绑定）/ PlotLines（外部 vector 只读绑定）/ Separator(Text)。",
			snips::kFeedback);
		fbBar1.self.fraction = &s.volume;
		fbBar1.self.overlay = "Loading";
		fbBar2.self.fraction = &s.volume;
		fbPlot.self.name = "波形历史";
		fbPlot.self.values = &s.history;
		fbSection.self.name = "分隔标题";
		page.addWidget(fbBar1);
		page.addWidget(fbBar2);
		page.addWidget(fbPlot);
		page.addWidget(fbSep);
		page.addWidget(fbSection);
	}
};

} // namespace gallery
