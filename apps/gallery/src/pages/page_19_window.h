#pragma once
// p19 多窗口：双顶层窗口共享状态实时链接（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageWindow {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::LabelText<Mk::wg2::Id("gal")>,
Mk::wg2::LabelText<Mk::wg2::Id("ins")>,
Mk::wg2::Button<Mk::wg2::Id("toggle")>,
Mk::wg2::TextWrapped<Mk::wg2::Id("note")>
>;

Page page;
Root root;

PageWindow(std::function<void()> toggleInspector, Mk::window* win, Mk::window* inspectorWin) {
page.setup("多窗口链接",
"两个顶层窗口各自一个 ImGui 上下文与帧循环；绑定 wg2 控件到同一批外部变量 = 实时链接。",
snips::kMultiWindow);

root.at<Mk::wg2::Id("gal")>().name = "Gallery 帧";
root.at<Mk::wg2::Id("gal")>().valueFn = [&] { return std::to_string(win->frameIndex()); };
root.at<Mk::wg2::Id("ins")>().name = "Inspector 帧";
root.at<Mk::wg2::Id("ins")>().valueFn = [&] { return inspectorWin ? std::to_string(inspectorWin->frameIndex()) : std::string("-"); };
root.at<Mk::wg2::Id("toggle")>().label = "显示/隐藏 Inspector";
root.at<Mk::wg2::Id("toggle")>().onClick = [&] { toggleInspector(); };
root.at<Mk::wg2::Id("note")>().text = "拖动一个窗口的滑块，另一个同帧刷新（绑同一变量）。"
" Inspector 只隐藏；Gallery 关闭后跟随退出。";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
