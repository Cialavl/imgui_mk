#pragma once
// p8 拖拽滑块：DragF/DragI/SliderF/SliderInt/SliderAngle（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageDrag {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "拖拽与滑块";
	static constexpr const char* desc  = "wg2 DragF/DragI（speed/mn/mx）与 SliderF/SliderInt/SliderAngle（角度转弧度）。值直接写回外部变量。";
	static constexpr std::string_view code = snips::kDrag;

using Root = VBox<
Id("root"),
DragF<Id("dragF")>,
DragI<Id("dragI")>,
SliderF<Id("slideF")>,
SliderInt<Id("slideI")>,
SliderAngle<Id("slideA")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	int dragInt = 100;
	int slideI = 50;
	float angle = 0.785f;
	float volume = 0.6f;

PageDrag() {


auto& w_dragF = root.at<Id("dragF")>();
w_dragF.name = "拖拽f";  root.at<Id("dragF")>().value = &drag;
w_dragF.speed = 0.01f;
root.at<Id("dragI")>().name = "拖拽i";  root.at<Id("dragI")>().value = &dragInt;
root.at<Id("slideF")>().name = "滑块f"; root.at<Id("slideF")>().value = &volume;
root.at<Id("slideI")>().name = "滑块i"; root.at<Id("slideI")>().value = &slideI;
root.at<Id("slideA")>().name = "角度";  root.at<Id("slideA")>().valueRad = &angle;

}

};

} // namespace gallery
