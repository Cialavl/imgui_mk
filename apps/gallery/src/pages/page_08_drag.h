#pragma once
// p8 拖拽滑块：DragF/DragI/SliderF/SliderInt/SliderAngle
#include "../gallery_chrome.h"

namespace gallery {

struct PageDrag {
	DragF dragF;
	DragI dragI;
	SliderF slideF;
	SliderInt slideI;
	SliderAngle slideA;

	Page page;
	PageDrag(State& s) {
		page.setup("拖拽与滑块",
			"DragF/DragI（speed/mn/mx）与 SliderF/SliderInt/SliderAngle（角度转弧度）。值直接写回外部变量。",
			snips::kDrag);
		dragF.self.name = "拖拽f";  dragF.self.value = &s.drag;
		dragF.self.speed = 0.01f;
		dragI.self.name = "拖拽i";  dragI.self.value = &s.dragInt;
		slideF.self.name = "滑块f"; slideF.self.value = &s.volume;
		slideI.self.name = "滑块i"; slideI.self.value = &s.slideI;
		slideA.self.name = "角度";  slideA.self.valueRad = &s.angle;
		page.addWidget(dragF);
		page.addWidget(dragI);
		page.addWidget(slideF);
		page.addWidget(slideI);
		page.addWidget(slideA);
	}
};

} // namespace gallery
