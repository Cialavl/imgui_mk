#pragma once
// p0 基础：wg2 直接 struct 控件树
#include "../../gallery_chrome.h"

namespace gallery {

struct PageBase {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "基础：wg2 直接 struct 控件";
	static constexpr const char* desc  = "wg2 不再使用 Control<Self> 外壳和 Node 成员；控件就是 template struct，"
"静态树由 VBox<Id(\"...\"), ...> 编译期组合，字符串路径访问。";
	static constexpr std::string_view code = snips::kBase;

using Root = VBox<
Id("root"),
Label<Id("l1")>,
Label<Id("l2")>,
Label<Id("l3")>,
Label<Id("l4")>,
Button<Id("btn")>,
LabelText<Id("clicks")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

PageBase() {


root.at<Id("l1")>().text = "wg2：普通控件是直接模板 struct";
root.at<Id("l2")>().text = "VBox<Id(\"root\"), A, B, C> 编译期组合整棵树";
root.at<Id("l3")>().text = "at<Id(\"a\"), Id(\"b\")>() 字符串路径访问";
root.at<Id("l4")>().text = "动态能力仅在 RuntimeList/RuntimeTabs/RuntimeGrid 中 opt-in";
auto& w_btn = root.at<Id("btn")>();
w_btn.label = "点我看看行为";
w_btn.onClick = [&] { ++clicks; };
auto& w_clicks = root.at<Id("clicks")>();
w_clicks.name = "点击次数";
w_clicks.valueFn = [&] { return std::to_string(clicks); };

}

};

} // namespace gallery
