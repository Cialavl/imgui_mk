#pragma once
// p0 基础：wg2 直接 struct 控件树
#include "../../gallery_chrome.h"

namespace gallery {

struct PageBase {
using Root = VBox<
Id("root"),
Label<Id("l1")>,
Label<Id("l2")>,
Label<Id("l3")>,
Label<Id("l4")>,
Button<Id("btn")>,
LabelText<Id("clicks")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

PageBase() {
page.setup("基础：wg2 直接 struct 控件",
"wg2 不再使用 Control<Self> 外壳和 Node 成员；控件就是 template struct，"
"静态树由 VBox<Id(\"...\"), ...> 编译期组合，字符串路径访问。",
snips::kBase);

root.at<Id("l1")>().text = "wg2：普通控件是直接模板 struct";
root.at<Id("l2")>().text = "VBox<Id(\"root\"), A, B, C> 编译期组合整棵树";
root.at<Id("l3")>().text = "at<Id(\"a\"), Id(\"b\")>() 字符串路径访问";
root.at<Id("l4")>().text = "动态能力仅在 RuntimeList/RuntimeTabs/RuntimeGrid 中 opt-in";
root.at<Id("btn")>().label = "点我看看行为";
root.at<Id("btn")>().onClick = [&] { ++clicks; };
root.at<Id("clicks")>().name = "点击次数";
root.at<Id("clicks")>().valueFn = [&] { return std::to_string(clicks); };

bindPage(page, root);
}

};

} // namespace gallery
