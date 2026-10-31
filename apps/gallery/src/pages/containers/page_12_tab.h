#pragma once
// p12 标签页：TabBar/Tab（wg2 编译期页签）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageTab {
using Root = VBox<
Id("root"),
TabBar<Id("tabs"),
Tab<Id("tab1"),
Label<Id("text1")>
>,
Tab<Id("tab2"),
Label<Id("text2")>
>
>
>;

Page page;
Root root;

PageTab() {
page.setup("标签页",
"wg2 TabBar/Tab 是编译期模板；页签标题为 Tab::title。",
snips::kTabBar);

root.at<Id("tabs")>().idname = "galtabs";
root.at<Id("tabs"), Id("tab1")>().title = "页一";
root.at<Id("tabs"), Id("tab1"), Id("text1")>().text = "TabBar 容器：页签渲染";
root.at<Id("tabs"), Id("tab2")>().title = "页二";
root.at<Id("tabs"), Id("tab2"), Id("text2")>().text = "第二页内容";

bindPage(page, root);
}

};

} // namespace gallery
