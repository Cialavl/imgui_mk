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
When<Id("when"),
Label<Id("whenText")>
>,
Colored<Id("styled"),
Button<Id("styledBtn")>
>,
Button<Id("disabled")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	bool mute = false;

PageDeco() {
page.setup("装饰件",
"wg2 Tooltip/Colored/Indent/ItemWidth/When 均为编译期包装；禁用用 state.enabled=false。",
snips::kDeco);

root.at<Id("tip"), Id("tipTarget")>().text = "悬停看我（tooltip 演示）";
root.at<Id("tip")>().text = "提示文本";
root.at<Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
root.at<Id("colored"), Id("coloredText")>().text = "彩色文字（colored 包裹）";
root.at<Id("indent")>().amount = 24.0f;
root.at<Id("indent"), Id("indentText")>().text = "缩进的文本";
root.at<Id("width")>().width = 180.0f;
auto& _w1 = root.at<Id("width"), Id("drag")>();
_w1.name = "限宽拖拽";
_w1.value = &drag;
root.at<Id("when"), Id("whenText")>().text = "When 包裹的文本，show=false 时不渲染不占位。";
root.at<Id("styled")>().color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
root.at<Id("styled"), Id("styledBtn")>().label = "Styled 主题按钮";
auto& _w2 = root.at<Id("disabled")>();
_w2.label = "禁用按钮（state.enabled=false）";
_w2.state.enabled = false;

bindPage(page, root);
	root.at<Id("when")>().showFn = [&] { return !mute; };
}


};

} // namespace gallery
