#pragma once
// p12 标签页：ui::TabBar/ui::Tab（wg2 编译期页签）
#include "../../page_frame.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageTab {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "标签页";
	static constexpr const char* desc  = "wg2 TabBar/Tab 是编译期模板；页签标题为 Tab::title。";
	static constexpr std::string_view code = snips::kTabBar;

using Root = ui::VBox<
ui::Id("root"),
ui::TabBar<ui::Id("tabs"),
ui::Tab<ui::Id("tab1"),
ui::Label<ui::Id("text1")>
>,
ui::Tab<ui::Id("tab2"),
ui::Label<ui::Id("text2")>
>
>
>;

	Root root;

PageTab() {


root.at<ui::Id("tabs")>().idname = "galtabs";
root.at<ui::Id("tabs"), ui::Id("tab1")>().title = "页一";
root.at<ui::Id("tabs"), ui::Id("tab1"), ui::Id("text1")>().text = "TabBar 容器：页签渲染";
root.at<ui::Id("tabs"), ui::Id("tab2")>().title = "页二";
root.at<ui::Id("tabs"), ui::Id("tab2"), ui::Id("text2")>().text = "第二页内容";

}

};

} // namespace gallery
