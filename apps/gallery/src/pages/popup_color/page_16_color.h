#pragma once
// p16 颜色编辑：ColorEdit3/4 + ColorPicker3/4（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageColor {
using Root = VBox<
Id("root"),
ColorEdit3<Id("rgb")>,
ColorEdit4<Id("rgba")>,
Collapsing<Id("collapsing"),
HBox<Id("pickRow"),
ColorPicker3<Id("pick3")>,
ColorPicker4<Id("pick4")>
>
>
>;

	Page<Root> page;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};

PageColor() {
page.setup("颜色编辑",
"wg2 ColorEdit3/4 与 ColorPicker3/4：std::array<float,3|4>* 绑定；拾取器可包进折叠容器。",
snips::kColor);

auto& w_rgb = page.root.at<Id("rgb")>();
w_rgb.name = "RGB";
w_rgb.color = &col3;
auto& w_rgba = page.root.at<Id("rgba")>();
w_rgba.name = "RGBA";
w_rgba.color = &col4;
auto& w_collapsing = page.root.at<Id("collapsing")>();
w_collapsing.title = "拾取器（默认展开）";
w_collapsing.defaultOpen = true;
auto& w_pick3 = page.root.at<Id("collapsing"), Id("pickRow"), Id("pick3")>();
w_pick3.name = "Pick RGB";
w_pick3.color = &pick3;
auto& w_pick4 = page.root.at<Id("collapsing"), Id("pickRow"), Id("pick4")>();
w_pick4.name = "Pick RGBA";
w_pick4.color = &pick4;

}

};

} // namespace gallery
