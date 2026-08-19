#pragma once
// p7 数值输入：InputInt/Float/Double + vec2
#include "../gallery_chrome.h"

namespace gallery {

struct PageNumber {
	InputInt numI;
	InputFloat numF;
	InputDouble numD;
	InputInt2 numIv2;
	InputFloat2 numFv2;

	Page page;
	PageNumber(State& s) {
		page.setup("数值输入",
			"InputInt/Float/Double 与 vec2 变体（std::array<T,2>* 绑定）。数值直接写回外部变量。",
			snips::kInputNum);
		numI.self.name = "整数";    numI.self.value = &s.iVal;
		numF.self.name = "浮点";    numF.self.value = &s.fVal;
		numD.self.name = "双精度";  numD.self.value = &s.dVal;
		numIv2.self.name = "ivec2"; numIv2.self.value = &s.iv;
		numFv2.self.name = "vec2";  numFv2.self.value = &s.fv;
		page.addWidget(numI);
		page.addWidget(numF);
		page.addWidget(numD);
		page.addWidget(numIv2);
		page.addWidget(numFv2);
	}
};

} // namespace gallery
