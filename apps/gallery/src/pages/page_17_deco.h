#pragma once
// p17 装饰件：Tooltip/Colored/Indent/ItemWidth/When + state.enabled（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageDeco {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Tooltip<Mk::wg2::Id("tip"),
Mk::wg2::Label<Mk::wg2::Id("tipTarget")>
>,
Mk::wg2::Colored<Mk::wg2::Id("colored"),
Mk::wg2::Label<Mk::wg2::Id("coloredText")>
>,
Mk::wg2::Indent<Mk::wg2::Id("indent"),
Mk::wg2::Label<Mk::wg2::Id("indentText")>
>,
Mk::wg2::ItemWidth<Mk::wg2::Id("width"),
Mk::wg2::DragF<Mk::wg2::Id("drag")>
>,
Mk::wg2::When<Mk::wg2::Id("when"),
Mk::wg2::Label<Mk::wg2::Id("whenText")>
>,
Mk::wg2::Colored<Mk::wg2::Id("styled"),
Mk::wg2::Button<Mk::wg2::Id("styledBtn")>
>,
Mk::wg2::Button<Mk::wg2::Id("disabled")>
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

root.at<Mk::wg2::Id("tip"), Mk::wg2::Id("tipTarget")>().text = "悬停看我（tooltip 演示）";
root.at<Mk::wg2::Id("tip")>().text = "提示文本";
root.at<Mk::wg2::Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
root.at<Mk::wg2::Id("colored"), Mk::wg2::Id("coloredText")>().text = "彩色文字（colored 包裹）";
root.at<Mk::wg2::Id("indent")>().amount = 24.0f;
root.at<Mk::wg2::Id("indent"), Mk::wg2::Id("indentText")>().text = "缩进的文本";
root.at<Mk::wg2::Id("width")>().width = 180.0f;
root.at<Mk::wg2::Id("width"), Mk::wg2::Id("drag")>().name = "限宽拖拽";
root.at<Mk::wg2::Id("width"), Mk::wg2::Id("drag")>().value = &drag;
root.at<Mk::wg2::Id("when"), Mk::wg2::Id("whenText")>().text = "When 包裹的文本，show=false 时不渲染不占位。";
root.at<Mk::wg2::Id("styled")>().color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
root.at<Mk::wg2::Id("styled"), Mk::wg2::Id("styledBtn")>().label = "Styled 主题按钮";
root.at<Mk::wg2::Id("disabled")>().label = "禁用按钮（state.enabled=false）";
root.at<Mk::wg2::Id("disabled")>().state.enabled = false;

bindPage(page, root);
	root.at<Mk::wg2::Id("when")>().showFn = [&] { return !mute; };
}


};

} // namespace gallery
