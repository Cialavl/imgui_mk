#pragma once
// p3 分格模式：SplitH/SplitV/Wrap/Grid（wg2 编译期布局）
#include "../gallery_chrome.h"

namespace gallery {

struct PageSplit {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Label<Mk::wg2::Id("intro")>,
Mk::wg2::SplitH<Mk::wg2::Id("splitH"),
Mk::wg2::Label<Mk::wg2::Id("sfA")>,
Mk::wg2::Label<Mk::wg2::Id("sfB")>,
Mk::wg2::Label<Mk::wg2::Id("sfC")>
>,
Mk::wg2::SplitV<Mk::wg2::Id("splitV"),
Mk::wg2::Label<Mk::wg2::Id("svA")>,
Mk::wg2::Label<Mk::wg2::Id("svB")>
>,
Mk::wg2::Wrap<Mk::wg2::Id("wrap"),
Mk::wg2::Label<Mk::wg2::Id("wA")>,
Mk::wg2::Label<Mk::wg2::Id("wB")>,
Mk::wg2::Label<Mk::wg2::Id("wC")>,
Mk::wg2::Label<Mk::wg2::Id("wD")>
>,
Mk::wg2::Grid<Mk::wg2::Id("grid"),
Mk::wg2::Label<Mk::wg2::Id("g00")>,
Mk::wg2::Label<Mk::wg2::Id("g01")>,
Mk::wg2::Label<Mk::wg2::Id("g02")>,
Mk::wg2::Label<Mk::wg2::Id("g10")>,
Mk::wg2::Label<Mk::wg2::Id("g11")>,
Mk::wg2::Label<Mk::wg2::Id("g12")>
>
>;

Page page;
Root root;

PageSplit() {
page.setup("分格模式",
"wg2 SplitH/SplitV 是 HBox/VBox 别名；stretch 与 fixed size 都存在 WidgetState。"
"Wrap 换行、Grid 固定列网格。",
snips::kSplit);

root.at<Mk::wg2::Id("intro")>().text = "分格演示：SplitH 权重分割 / SplitV 垂直分割 / Wrap 换行 / Grid 网格";
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfA")>().text = "fixed 160";
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfA")>().state.size.fixedW = 160.0f;
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfB")>().text = "stretch 1";
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfB")>().state.stretch = 1.0f;
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfC")>().text = "stretch 2";
root.at<Mk::wg2::Id("splitH"), Mk::wg2::Id("sfC")>().state.stretch = 2.0f;

root.at<Mk::wg2::Id("splitV"), Mk::wg2::Id("svA")>().text = "fixed 44";
root.at<Mk::wg2::Id("splitV"), Mk::wg2::Id("svA")>().state.size.fixedH = 44.0f;
root.at<Mk::wg2::Id("splitV"), Mk::wg2::Id("svB")>().text = "VSplit stretch";
root.at<Mk::wg2::Id("splitV"), Mk::wg2::Id("svB")>().state.stretch = 1.0f;

root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("wA")>().text = "换行一";
root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("wB")>().text = "换行二（较长项）";
root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("wC")>().text = "换行三";
root.at<Mk::wg2::Id("wrap"), Mk::wg2::Id("wD")>().text = "换行四";

root.at<Mk::wg2::Id("grid")>().cols = 3;
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g00")>().text = "格 0-0";
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g01")>().text = "格 0-1";
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g02")>().text = "格 0-2";
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g10")>().text = "格 1-0";
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g11")>().text = "格 1-1";
root.at<Mk::wg2::Id("grid"), Mk::wg2::Id("g12")>().text = "格 1-2";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
