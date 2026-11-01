#pragma once
// p17 装饰件：Tooltip/Colored/Indent/ItemWidth/When + state.enabled（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageDeco {
using Root = VBox<
Id("root"),
Tooltip<Id("tip"),
Label<Id("tipTarget")>
>,
Colored<Id("colored"),
Label<Id("coloredText")>
>,
Indent<Id("indent"),
Label<Id("indentText")>
>,
ItemWidth<Id("width"),
DragF<Id("drag")>
>,
Label<Id("whenText")>,
Colored<Id("styled"),
Button<Id("styledBtn")>
>,
Button<Id("disabled")>
>;

Page<Root> page;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	bool mute = false;

PageDeco() {
page.setup("装饰件",
"wg2 Tooltip/Colored/Indent/ItemWidth/When 均为编译期包装；禁用用 state.enabled=false。",
snips::kDeco);

page.root.at<Id("tip"), Id("tipTarget")>().text = "悬停看我（tooltip 演示）";
page.root.at<Id("tip")>().text = "提示文本";
page.root.at<Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
page.root.at<Id("colored"), Id("coloredText")>().text = "彩色文字（colored 包裹）";
page.root.at<Id("indent")>().amount = 24.0f;
page.root.at<Id("indent"), Id("indentText")>().text = "缩进的文本";
page.root.at<Id("width")>().width = 180.0f;
auto& w_drag = page.root.at<Id("width"), Id("drag")>();
w_drag.name = "限宽拖拽";
w_drag.value = &drag;
page.root.at<Id("whenText")>().text = "When 包裹的文本，show=false 时不渲染不占位。";
page.root.at<Id("styled")>().color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
page.root.at<Id("styled"), Id("styledBtn")>().label = "Styled 主题按钮";
auto& w_disabled = page.root.at<Id("disabled")>();
w_disabled.label = "禁用按钮（state.enabled=false）";
w_disabled.state.enabled = false;

	page.root.at<Id("whenText")>().state.visibleFn = [&] { return !mute; };
}


};

} // namespace gallery
