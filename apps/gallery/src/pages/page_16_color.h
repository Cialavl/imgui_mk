#pragma once
// p16 颜色编辑：ColorEdit3/4 + ColorPicker3/4（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageColor {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::ColorEdit3<Mk::wg2::Id("rgb")>,
Mk::wg2::ColorEdit4<Mk::wg2::Id("rgba")>,
Mk::wg2::Collapsing<Mk::wg2::Id("collapsing"),
Mk::wg2::HBox<Mk::wg2::Id("pickRow"),
Mk::wg2::ColorPicker3<Mk::wg2::Id("pick3")>,
Mk::wg2::ColorPicker4<Mk::wg2::Id("pick4")>
>
>
>;

Page page;
Root root;

PageColor(std::array<float, 3>& col3, std::array<float, 4>& col4, std::array<float, 3>& pick3, std::array<float, 4>& pick4) {
page.setup("颜色编辑",
"wg2 ColorEdit3/4 与 ColorPicker3/4：std::array<float,3|4>* 绑定；拾取器可包进折叠容器。",
snips::kColor);

root.at<Mk::wg2::Id("rgb")>().name = "RGB";
root.at<Mk::wg2::Id("rgb")>().color = &col3;
root.at<Mk::wg2::Id("rgba")>().name = "RGBA";
root.at<Mk::wg2::Id("rgba")>().color = &col4;
root.at<Mk::wg2::Id("collapsing")>().title = "拾取器（默认展开）";
root.at<Mk::wg2::Id("collapsing")>().defaultOpen = true;
root.at<Mk::wg2::Id("collapsing"), Mk::wg2::Id("pickRow"), Mk::wg2::Id("pick3")>().name = "Pick RGB";
root.at<Mk::wg2::Id("collapsing"), Mk::wg2::Id("pickRow"), Mk::wg2::Id("pick3")>().color = &pick3;
root.at<Mk::wg2::Id("collapsing"), Mk::wg2::Id("pickRow"), Mk::wg2::Id("pick4")>().name = "Pick RGBA";
root.at<Mk::wg2::Id("collapsing"), Mk::wg2::Id("pickRow"), Mk::wg2::Id("pick4")>().color = &pick4;

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
