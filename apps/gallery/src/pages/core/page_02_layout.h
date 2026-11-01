#pragma once
// p2 布局：HBox 嵌 VBox（编译期布局组合演示）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayout {
using Root = VBox<
Id("root"),
Label<Id("intro")>,
HBox<Id("row"),
VBox<Id("col"),
SliderF<Id("volume")>,
CheckBox<Id("mute")>
>,
// 右侧标签同样套一层 VBox：与左列共享 WindowPadding 内边距，同一行才对得齐
VBox<Id("rightCol"),
Label<Id("right")>
>
>,
Label<Id("note")>
>;

Page<Root> page;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float volume = 0.6f;
	bool mute = false;

PageLayout() {
page.setup("布局：编译期 HBox/VBox 嵌套",
"wg2 静态布局 = VBox/HBox/Grid/Wrap/SplitH/SplitV/Overlay 模板容器。"
"本页 = HBox 嵌 VBox，子节点经模板参数传入，布局嵌布局自然递归。",
snips::kLayout);

page.root.at<Id("intro")>().text = "布局步骤：hint（计算尺寸）→ draw（求解并递归子节点）";
auto& w_volume = page.root.at<Id("row"), Id("col"), Id("volume")>();
w_volume.name = "音量";
w_volume.value = &volume;
auto& w_mute = page.root.at<Id("row"), Id("col"), Id("mute")>();
w_mute.name = "静音";
w_mute.checked = &mute;
page.root.at<Id("row"), Id("rightCol"), Id("right")>().text = "H 行右格";
page.root.at<Id("note")>().text = "布局数据约束：子控件在父容器内，路径访问全程一致。";

}

};

} // namespace gallery
