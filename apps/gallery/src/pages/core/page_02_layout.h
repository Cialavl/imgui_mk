#pragma once
// p2 布局：HBox 嵌 VBox（编译期布局组合演示）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayout {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Label<Mk::wg2::Id("intro")>,
Mk::wg2::HBox<Mk::wg2::Id("row"),
Mk::wg2::VBox<Mk::wg2::Id("col"),
Mk::wg2::SliderF<Mk::wg2::Id("volume")>,
Mk::wg2::CheckBox<Mk::wg2::Id("mute")>
>,
// 右侧标签同样套一层 VBox：与左列共享 WindowPadding 内边距，同一行才对得齐
Mk::wg2::VBox<Mk::wg2::Id("rightCol"),
Mk::wg2::Label<Mk::wg2::Id("right")>
>
>,
Mk::wg2::Label<Mk::wg2::Id("note")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float volume = 0.6f;
	bool mute = false;

PageLayout() {
page.setup("布局：编译期 HBox/VBox 嵌套",
"wg2 静态布局 = VBox/HBox/Grid/Wrap/SplitH/SplitV/Overlay 模板容器。"
"本页 = HBox 嵌 VBox，子节点经模板参数传入，布局嵌布局自然递归。",
snips::kLayout);

root.at<Mk::wg2::Id("intro")>().text = "布局步骤：hint（计算尺寸）→ draw（求解并递归子节点）";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("col"), Mk::wg2::Id("volume")>().name = "音量";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("col"), Mk::wg2::Id("volume")>().value = &volume;
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("col"), Mk::wg2::Id("mute")>().name = "静音";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("col"), Mk::wg2::Id("mute")>().checked = &mute;
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("rightCol"), Mk::wg2::Id("right")>().text = "H 行右格";
root.at<Mk::wg2::Id("note")>().text = "布局数据约束：子控件在父容器内，路径访问全程一致。";

bindPage(page, root);
}

};

} // namespace gallery
