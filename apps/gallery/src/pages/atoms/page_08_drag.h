#pragma once
// p8 拖拽滑块：DragF/DragI/SliderF/SliderInt/SliderAngle（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageDrag {
using Root = VBox<
Id("root"),
DragF<Id("dragF")>,
DragI<Id("dragI")>,
SliderF<Id("slideF")>,
SliderInt<Id("slideI")>,
SliderAngle<Id("slideA")>
>;

Page<Root> page;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	int dragInt = 100;
	int slideI = 50;
	float angle = 0.785f;
	float volume = 0.6f;

PageDrag() {
page.setup("拖拽与滑块",
"wg2 DragF/DragI（speed/mn/mx）与 SliderF/SliderInt/SliderAngle（角度转弧度）。值直接写回外部变量。",
snips::kDrag);

auto& w_dragF = page.root.at<Id("dragF")>();
w_dragF.name = "拖拽f";  page.root.at<Id("dragF")>().value = &drag;
w_dragF.speed = 0.01f;
page.root.at<Id("dragI")>().name = "拖拽i";  page.root.at<Id("dragI")>().value = &dragInt;
page.root.at<Id("slideF")>().name = "滑块f"; page.root.at<Id("slideF")>().value = &volume;
page.root.at<Id("slideI")>().name = "滑块i"; page.root.at<Id("slideI")>().value = &slideI;
page.root.at<Id("slideA")>().name = "角度";  page.root.at<Id("slideA")>().valueRad = &angle;

}

};

} // namespace gallery
