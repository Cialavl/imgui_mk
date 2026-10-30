#pragma once
// p11 容器：Overlay / Collapsing / Tree / SubWindow（wg2 编译期子节点）
#include "../gallery_chrome.h"

namespace gallery {

struct PageContainer {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Overlay<Mk::wg2::Id("overlay"),
Mk::wg2::Button<Mk::wg2::Id("overlayButton")>,
Mk::wg2::Offset<Mk::wg2::Id("overlayOffset"), Mk::wg2::Label<Mk::wg2::Id("overlayLabel")>>
>,
Mk::wg2::Collapsing<Mk::wg2::Id("collapsing"),
Mk::wg2::Label<Mk::wg2::Id("collLabel")>
>,
Mk::wg2::Tree<Mk::wg2::Id("tree"),
Mk::wg2::Label<Mk::wg2::Id("leaf1")>,
Mk::wg2::Label<Mk::wg2::Id("leaf2")>
>,
Mk::wg2::SubWindow<Mk::wg2::Id("sub"),
Mk::wg2::Label<Mk::wg2::Id("subLabel")>
>
>;

Page page;
Root root;

PageContainer(State& s) {
page.setup("容器控件",
"wg2 Overlay 叠放 / Collapsing / Tree / SubWindow；子节点经模板参数编译期传入。",
snips::kContainers);

root.at<Mk::wg2::Id("overlay"), Mk::wg2::Id("overlayOffset"), Mk::wg2::Id("overlayLabel")>().text = "Overlay 叠放（重叠渲染）";
	// 叠放时错开：标签浮到按钮右侧，避免两段文字完全叠在一起看不清
	root.at<Mk::wg2::Id("overlay"), Mk::wg2::Id("overlayOffset")>().dxEm = 7.5f;
	root.at<Mk::wg2::Id("overlay"), Mk::wg2::Id("overlayOffset")>().dyEm = 0.35f;
root.at<Mk::wg2::Id("overlay"), Mk::wg2::Id("overlayButton")>().label = "叠放按钮";
root.at<Mk::wg2::Id("overlay"), Mk::wg2::Id("overlayButton")>().onClick = [&] { ++s.clicks; };
root.at<Mk::wg2::Id("collapsing")>().title = "折叠容器（默认展开）";
root.at<Mk::wg2::Id("collapsing")>().defaultOpen = true;
root.at<Mk::wg2::Id("collapsing"), Mk::wg2::Id("collLabel")>().text = "折叠内容";
root.at<Mk::wg2::Id("tree")>().title = "树节点（默认展开）";
root.at<Mk::wg2::Id("tree")>().defaultOpen = true;
root.at<Mk::wg2::Id("tree"), Mk::wg2::Id("leaf1")>().text = "叶子一";
root.at<Mk::wg2::Id("tree"), Mk::wg2::Id("leaf2")>().text = "叶子二";
root.at<Mk::wg2::Id("sub")>().idname = "subwin";
root.at<Mk::wg2::Id("sub")>().height = 70.0f;
root.at<Mk::wg2::Id("sub"), Mk::wg2::Id("subLabel")>().text = "SubWindow：嵌套滚动子窗口";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
