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

	Page<Root> page;

PageSplit() {
page.setup("分格模式",
"wg2 SplitH/SplitV 是 HBox/VBox 别名；stretch 与 fixed size 都存在 WidgetState。"
"Wrap 换行、Grid 固定列网格。",
snips::kSplit);

page.root.at<Id("intro")>().text = "分格演示：SplitH 权重分割 / SplitV 垂直分割 / Wrap 换行 / Grid 网格";
auto& w_sfA = page.root.at<Id("splitH"), Id("sfA")>();
w_sfA.text = "fixed 160";
w_sfA.state.size.fixedW = 160.0f;
auto& w_sfB = page.root.at<Id("splitH"), Id("sfB")>();
w_sfB.text = "stretch 1";
w_sfB.state.stretch = 1.0f;
auto& w_sfC = page.root.at<Id("splitH"), Id("sfC")>();
w_sfC.text = "stretch 2";
w_sfC.state.stretch = 2.0f;

auto& w_svA = page.root.at<Id("splitV"), Id("svA")>();
w_svA.text = "fixed 44";
w_svA.state.size.fixedH = 44.0f;
auto& w_svB = page.root.at<Id("splitV"), Id("svB")>();
w_svB.text = "VSplit stretch";
w_svB.state.stretch = 1.0f;

page.root.at<Id("wrap"), Id("wA")>().text = "换行一";
page.root.at<Id("wrap"), Id("wB")>().text = "换行二（较长项）";
page.root.at<Id("wrap"), Id("wC")>().text = "换行三";
page.root.at<Id("wrap"), Id("wD")>().text = "换行四";

page.root.at<Id("grid")>().cols = 3;
page.root.at<Id("grid"), Id("g00")>().text = "格 0-0";
page.root.at<Id("grid"), Id("g01")>().text = "格 0-1";
page.root.at<Id("grid"), Id("g02")>().text = "格 0-2";
page.root.at<Id("grid"), Id("g10")>().text = "格 1-0";
page.root.at<Id("grid"), Id("g11")>().text = "格 1-1";
page.root.at<Id("grid"), Id("g12")>().text = "格 1-2";

}

};

} // namespace gallery
