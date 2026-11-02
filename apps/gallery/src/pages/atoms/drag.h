#pragma once
// p8 拖拽滑块：ui::DragF/ui::DragI/ui::SliderF/ui::SliderInt/ui::SliderAngle（wg2）
#include "../../page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageDrag {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "拖拽与滑块";
	static constexpr const char* desc  = "wg2 DragF/DragI（speed/mn/mx）与 SliderF/SliderInt/SliderAngle（角度转弧度）。值直接写回外部变量。";
	static constexpr std::string_view code = snips::kDrag;

using Root = ui::VBox<
ui::Id("root"),
ui::DragF<ui::Id("dragF")>,
ui::DragI<ui::Id("dragI")>,
ui::SliderF<ui::Id("slideF")>,
ui::SliderInt<ui::Id("slideI")>,
ui::SliderAngle<ui::Id("slideA")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	int dragInt = 100;
	int slideI = 50;
	float angle = 0.785f;
	float volume = 0.6f;

PageDrag() {


auto& w_dragF = root.at<ui::Id("dragF")>();
w_dragF.name = "拖拽f";  root.at<ui::Id("dragF")>().value = &drag;
w_dragF.speed = 0.01f;
root.at<ui::Id("dragI")>().name = "拖拽i";  root.at<ui::Id("dragI")>().value = &dragInt;
root.at<ui::Id("slideF")>().name = "滑块f"; root.at<ui::Id("slideF")>().value = &volume;
root.at<ui::Id("slideI")>().name = "滑块i"; root.at<ui::Id("slideI")>().value = &slideI;
root.at<ui::Id("slideA")>().name = "角度";  root.at<ui::Id("slideA")>().valueRad = &angle;

}

};

} // namespace gallery
