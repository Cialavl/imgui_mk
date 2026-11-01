#pragma once
// p12 标签页：TabBar/Tab（wg2 编译期页签）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageTab {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "标签页";
	static constexpr const char* desc  = "wg2 TabBar/Tab 是编译期模板；页签标题为 Tab::title。";
	static constexpr std::string_view code = snips::kTabBar;

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

	Root root;

PageTab() {


root.at<Id("tabs")>().idname = "galtabs";
root.at<Id("tabs"), Id("tab1")>().title = "页一";
root.at<Id("tabs"), Id("tab1"), Id("text1")>().text = "TabBar 容器：页签渲染";
root.at<Id("tabs"), Id("tab2")>().title = "页二";
root.at<Id("tabs"), Id("tab2"), Id("text2")>().text = "第二页内容";

}

};

} // namespace gallery
