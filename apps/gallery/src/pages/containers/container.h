#pragma once
// p11 容器：ui::Overlay / ui::Collapsing / ui::Tree / ui::SubWindow（wg2 编译期子节点）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageContainer {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "容器控件";
	static constexpr const char* desc  = "wg2 ui::Overlay 叠放 / ui::Collapsing / ui::Tree / ui::SubWindow；子节点经模板参数编译期传入。";
	static constexpr std::string_view code = snips::kContainers;

using Root = ui::VBox<
ui::Id("root"),
ui::Overlay<ui::Id("overlay"),
ui::Button<ui::Id("overlayButton")>,
ui::Offset<ui::Id("overlayOffset"), ui::Label<ui::Id("overlayLabel")>>
>,
ui::Collapsing<ui::Id("collapsing"),
ui::Label<ui::Id("collLabel")>
>,
ui::Tree<ui::Id("tree"),
ui::Label<ui::Id("leaf1")>,
ui::Label<ui::Id("leaf2")>
>,
ui::SubWindow<ui::Id("sub"),
ui::Label<ui::Id("subLabel")>
>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

PageContainer() {


root.at<ui::Id("overlay"), ui::Id("overlayOffset"), ui::Id("overlayLabel")>().text = "ui::Overlay 叠放（重叠渲染）";
	// 叠放时错开：标签浮到按钮右侧，避免两段文字完全叠在一起看不清
	auto& w_overlayOffset = root.at<ui::Id("overlay"), ui::Id("overlayOffset")>();
	w_overlayOffset.dxEm = 7.5f;
	w_overlayOffset.dyEm = 0.35f;
auto& w_overlayButton = root.at<ui::Id("overlay"), ui::Id("overlayButton")>();
w_overlayButton.label = "叠放按钮";
w_overlayButton.onClick = [&] { ++clicks; };
auto& w_collapsing = root.at<ui::Id("collapsing")>();
w_collapsing.title = "折叠容器（默认展开）";
w_collapsing.defaultOpen = true;
root.at<ui::Id("collapsing"), ui::Id("collLabel")>().text = "折叠内容";
auto& w_tree = root.at<ui::Id("tree")>();
w_tree.title = "树节点（默认展开）";
w_tree.defaultOpen = true;
root.at<ui::Id("tree"), ui::Id("leaf1")>().text = "叶子一";
root.at<ui::Id("tree"), ui::Id("leaf2")>().text = "叶子二";
auto& w_sub = root.at<ui::Id("sub")>();
w_sub.idname = "subwin";
w_sub.height = 70.0f;
root.at<ui::Id("sub"), ui::Id("subLabel")>().text = "ui::SubWindow：嵌套滚动子窗口";

}

};

} // namespace gallery
