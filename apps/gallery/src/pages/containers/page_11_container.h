#pragma once
// p11 容器：Overlay / Collapsing / Tree / SubWindow（wg2 编译期子节点）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageContainer {
using Root = VBox<
Id("root"),
Overlay<Id("overlay"),
Button<Id("overlayButton")>,
Offset<Id("overlayOffset"), Label<Id("overlayLabel")>>
>,
Collapsing<Id("collapsing"),
Label<Id("collLabel")>
>,
Tree<Id("tree"),
Label<Id("leaf1")>,
Label<Id("leaf2")>
>,
SubWindow<Id("sub"),
Label<Id("subLabel")>
>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

PageContainer() {
page.setup("容器控件",
"wg2 Overlay 叠放 / Collapsing / Tree / SubWindow；子节点经模板参数编译期传入。",
snips::kContainers);

root.at<Id("overlay"), Id("overlayOffset"), Id("overlayLabel")>().text = "Overlay 叠放（重叠渲染）";
	// 叠放时错开：标签浮到按钮右侧，避免两段文字完全叠在一起看不清
	auto& _w1 = root.at<Id("overlay"), Id("overlayOffset")>();
	_w1.dxEm = 7.5f;
	_w1.dyEm = 0.35f;
auto& _w2 = root.at<Id("overlay"), Id("overlayButton")>();
_w2.label = "叠放按钮";
_w2.onClick = [&] { ++clicks; };
auto& _w3 = root.at<Id("collapsing")>();
_w3.title = "折叠容器（默认展开）";
_w3.defaultOpen = true;
root.at<Id("collapsing"), Id("collLabel")>().text = "折叠内容";
auto& _w4 = root.at<Id("tree")>();
_w4.title = "树节点（默认展开）";
_w4.defaultOpen = true;
root.at<Id("tree"), Id("leaf1")>().text = "叶子一";
root.at<Id("tree"), Id("leaf2")>().text = "叶子二";
auto& _w5 = root.at<Id("sub")>();
_w5.idname = "subwin";
_w5.height = 70.0f;
root.at<Id("sub"), Id("subLabel")>().text = "SubWindow：嵌套滚动子窗口";

bindPage(page, root);
}

};

} // namespace gallery
