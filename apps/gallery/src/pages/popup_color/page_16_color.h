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

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};

PageColor() {
page.setup("颜色编辑",
"wg2 ColorEdit3/4 与 ColorPicker3/4：std::array<float,3|4>* 绑定；拾取器可包进折叠容器。",
snips::kColor);

auto& _w1 = root.at<Id("rgb")>();
_w1.name = "RGB";
_w1.color = &col3;
auto& _w2 = root.at<Id("rgba")>();
_w2.name = "RGBA";
_w2.color = &col4;
auto& _w3 = root.at<Id("collapsing")>();
_w3.title = "拾取器（默认展开）";
_w3.defaultOpen = true;
auto& _w4 = root.at<Id("collapsing"), Id("pickRow"), Id("pick3")>();
_w4.name = "Pick RGB";
_w4.color = &pick3;
auto& _w5 = root.at<Id("collapsing"), Id("pickRow"), Id("pick4")>();
_w5.name = "Pick RGBA";
_w5.color = &pick4;

bindPage(page, root);
}

};

} // namespace gallery
