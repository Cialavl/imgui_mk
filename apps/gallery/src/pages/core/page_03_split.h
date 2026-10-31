#pragma once
// p3 分格模式：SplitH/SplitV/Wrap/Grid（wg2 编译期布局）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageSplit {
using Root = VBox<
Id("root"),
Label<Id("intro")>,
SplitH<Id("splitH"),
Label<Id("sfA")>,
Label<Id("sfB")>,
Label<Id("sfC")>
>,
SplitV<Id("splitV"),
Label<Id("svA")>,
Label<Id("svB")>
>,
Wrap<Id("wrap"),
Label<Id("wA")>,
Label<Id("wB")>,
Label<Id("wC")>,
Label<Id("wD")>
>,
Grid<Id("grid"),
Label<Id("g00")>,
Label<Id("g01")>,
Label<Id("g02")>,
Label<Id("g10")>,
Label<Id("g11")>,
Label<Id("g12")>
>
>;

Page page;
Root root;

PageSplit() {
page.setup("分格模式",
"wg2 SplitH/SplitV 是 HBox/VBox 别名；stretch 与 fixed size 都存在 WidgetState。"
"Wrap 换行、Grid 固定列网格。",
snips::kSplit);

root.at<Id("intro")>().text = "分格演示：SplitH 权重分割 / SplitV 垂直分割 / Wrap 换行 / Grid 网格";
auto& _w1 = root.at<Id("splitH"), Id("sfA")>();
_w1.text = "fixed 160";
_w1.state.size.fixedW = 160.0f;
auto& _w2 = root.at<Id("splitH"), Id("sfB")>();
_w2.text = "stretch 1";
_w2.state.stretch = 1.0f;
auto& _w3 = root.at<Id("splitH"), Id("sfC")>();
_w3.text = "stretch 2";
_w3.state.stretch = 2.0f;

auto& _w4 = root.at<Id("splitV"), Id("svA")>();
_w4.text = "fixed 44";
_w4.state.size.fixedH = 44.0f;
auto& _w5 = root.at<Id("splitV"), Id("svB")>();
_w5.text = "VSplit stretch";
_w5.state.stretch = 1.0f;

root.at<Id("wrap"), Id("wA")>().text = "换行一";
root.at<Id("wrap"), Id("wB")>().text = "换行二（较长项）";
root.at<Id("wrap"), Id("wC")>().text = "换行三";
root.at<Id("wrap"), Id("wD")>().text = "换行四";

root.at<Id("grid")>().cols = 3;
root.at<Id("grid"), Id("g00")>().text = "格 0-0";
root.at<Id("grid"), Id("g01")>().text = "格 0-1";
root.at<Id("grid"), Id("g02")>().text = "格 0-2";
root.at<Id("grid"), Id("g10")>().text = "格 1-0";
root.at<Id("grid"), Id("g11")>().text = "格 1-1";
root.at<Id("grid"), Id("g12")>().text = "格 1-2";

bindPage(page, root);
}

};

} // namespace gallery
