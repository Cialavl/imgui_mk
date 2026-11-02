#pragma once
// p17 装饰件：ui::Tooltip/ui::Colored/ui::Indent/ui::ItemWidth/When + state.enabled（wg2）
#include "../../page_frame.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageDeco {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "装饰件";
	static constexpr const char* desc  = "wg2 Tooltip/Colored/Indent/ItemWidth/When 均为编译期包装；禁用用 state.enabled=false。";
	static constexpr std::string_view code = snips::kDeco;

using Root = ui::VBox<
ui::Id("root"),
ui::Tooltip<ui::Id("tip"),
ui::Label<ui::Id("tipTarget")>
>,
ui::Colored<ui::Id("colored"),
ui::Label<ui::Id("coloredText")>
>,
ui::Indent<ui::Id("indent"),
ui::Label<ui::Id("indentText")>
>,
ui::ItemWidth<ui::Id("width"),
ui::DragF<ui::Id("drag")>
>,
ui::Label<ui::Id("whenText")>,
ui::Colored<ui::Id("styled"),
ui::Button<ui::Id("styledBtn")>
>,
ui::Button<ui::Id("disabled")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float drag = 0.5f;
	bool mute = false;

PageDeco() {


root.at<ui::Id("tip"), ui::Id("tipTarget")>().text = "悬停看我（tooltip 演示）";
root.at<ui::Id("tip")>().text = "提示文本";
root.at<ui::Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
root.at<ui::Id("colored"), ui::Id("coloredText")>().text = "彩色文字（colored 包裹）";
root.at<ui::Id("indent")>().amount = 24.0f;
root.at<ui::Id("indent"), ui::Id("indentText")>().text = "缩进的文本";
root.at<ui::Id("width")>().width = 180.0f;
auto& w_drag = root.at<ui::Id("width"), ui::Id("drag")>();
w_drag.name = "限宽拖拽";
w_drag.value = &drag;
root.at<ui::Id("whenText")>().text = "When 包裹的文本，show=false 时不渲染不占位。";
root.at<ui::Id("styled")>().color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
root.at<ui::Id("styled"), ui::Id("styledBtn")>().label = "Styled 主题按钮";
auto& w_disabled = root.at<ui::Id("disabled")>();
w_disabled.label = "禁用按钮（state.enabled=false）";
w_disabled.state.enabled = false;

	root.at<ui::Id("whenText")>().state.visibleFn = [&] { return !mute; };
}


};

} // namespace gallery
