#pragma once
// p13 菜单面板：SubWindow 面板（wg2 编译期子节点）
#include "../gallery_chrome.h"

namespace gallery {

struct PageMenu {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::SubWindow<Mk::wg2::Id("panel"),
Mk::wg2::SliderF<Mk::wg2::Id("slider")>,
Mk::wg2::LabelText<Mk::wg2::Id("frames")>
>,
Mk::wg2::Label<Mk::wg2::Id("note")>
>;

Page page;
Root root;

PageMenu(float& volume, Mk::window* win) {
page.setup("菜单与面板",
"wg2 Panel = SubWindow 别名；面板内容经模板参数编译期传入。",
snips::kPanel);

root.at<Mk::wg2::Id("panel")>().idname = "面板";
root.at<Mk::wg2::Id("panel")>().height = 110.0f;
root.at<Mk::wg2::Id("panel"), Mk::wg2::Id("slider")>().name = "面板内滑块";
root.at<Mk::wg2::Id("panel"), Mk::wg2::Id("slider")>().value = &volume;
root.at<Mk::wg2::Id("panel"), Mk::wg2::Id("frames")>().name = "帧";
root.at<Mk::wg2::Id("panel"), Mk::wg2::Id("frames")>().valueFn = [&] { return std::to_string(win->frameIndex()); };
root.at<Mk::wg2::Id("note")>().text = "Panel 是浮动窗口；菜单栏是常驻点（主窗口顶部）。";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
