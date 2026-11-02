#pragma once
// p0 基础：wg2 直接 struct 控件树
#include "../../page_frame.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageBase {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "基础：wg2 直接 struct 控件";
	static constexpr const char* desc  = "wg2 不再使用 Control<Self> 外壳和 Node 成员；控件就是 template struct，"
"静态树由 VBox<Id(\"...\"), ...> 编译期组合，字符串路径访问。";
	static constexpr std::string_view code = snips::kBase;

using Root = ui::VBox<
ui::Id("root"),
ui::Label<ui::Id("l1")>,
ui::Label<ui::Id("l2")>,
ui::Label<ui::Id("l3")>,
ui::Label<ui::Id("l4")>,
ui::Button<ui::Id("btn")>,
ui::LabelText<ui::Id("clicks")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

PageBase() {


root.at<ui::Id("l1")>().text = "wg2：普通控件是直接模板 struct";
root.at<ui::Id("l2")>().text = "VBox<Id(\"root\"), A, B, C> 编译期组合整棵树";
root.at<ui::Id("l3")>().text = "at<Id(\"a\"), Id(\"b\")>() 字符串路径访问";
root.at<ui::Id("l4")>().text = "动态能力仅在 RuntimeList/RuntimeTabs/RuntimeGrid 中 opt-in";
auto& w_btn = root.at<ui::Id("btn")>();
w_btn.label = "点我看看行为";
w_btn.onClick = [&] { ++clicks; };
auto& w_clicks = root.at<ui::Id("clicks")>();
w_clicks.name = "点击次数";
w_clicks.valueFn = [&] { return std::to_string(clicks); };

}

};

} // namespace gallery
