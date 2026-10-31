#pragma once
// p8 拖拽滑块：DragF/DragI/SliderF/SliderInt/SliderAngle（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageDrag {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::DragF<Mk::wg2::Id("dragF")>,
Mk::wg2::DragI<Mk::wg2::Id("dragI")>,
Mk::wg2::SliderF<Mk::wg2::Id("slideF")>,
Mk::wg2::SliderInt<Mk::wg2::Id("slideI")>,
Mk::wg2::SliderAngle<Mk::wg2::Id("slideA")>
>;

Page page;
Root root;

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

root.at<Mk::wg2::Id("dragF")>().name = "拖拽f";  root.at<Mk::wg2::Id("dragF")>().value = &drag;
root.at<Mk::wg2::Id("dragF")>().speed = 0.01f;
root.at<Mk::wg2::Id("dragI")>().name = "拖拽i";  root.at<Mk::wg2::Id("dragI")>().value = &dragInt;
root.at<Mk::wg2::Id("slideF")>().name = "滑块f"; root.at<Mk::wg2::Id("slideF")>().value = &volume;
root.at<Mk::wg2::Id("slideI")>().name = "滑块i"; root.at<Mk::wg2::Id("slideI")>().value = &slideI;
root.at<Mk::wg2::Id("slideA")>().name = "角度";  root.at<Mk::wg2::Id("slideA")>().valueRad = &angle;

bindPage(page, root);
}

};

} // namespace gallery
