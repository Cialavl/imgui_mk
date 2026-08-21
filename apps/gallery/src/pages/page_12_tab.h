#pragma once
// p12 标签页：TabBar/Tab（wg2 编译期页签）
#include "../gallery_chrome.h"

namespace gallery {

struct PageTab {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::TabBar<Mk::wg2::Id("tabs"),
Mk::wg2::Tab<Mk::wg2::Id("tab1"),
Mk::wg2::Label<Mk::wg2::Id("text1")>
>,
Mk::wg2::Tab<Mk::wg2::Id("tab2"),
Mk::wg2::Label<Mk::wg2::Id("text2")>
>
>
>;

Page page;
Root root;

PageTab(State& s) {
(void)s;
page.setup("标签页",
"wg2 TabBar/Tab 是编译期模板；页签标题为 Tab::title。",
snips::kTabBar);

root.at<Mk::wg2::Id("tabs")>().idname = "galtabs";
root.at<Mk::wg2::Id("tabs"), Mk::wg2::Id("tab1")>().title = "页一";
root.at<Mk::wg2::Id("tabs"), Mk::wg2::Id("tab1"), Mk::wg2::Id("text1")>().text = "TabBar 容器：页签渲染";
root.at<Mk::wg2::Id("tabs"), Mk::wg2::Id("tab2")>().title = "页二";
root.at<Mk::wg2::Id("tabs"), Mk::wg2::Id("tab2"), Mk::wg2::Id("text2")>().text = "第二页内容";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
