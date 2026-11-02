#pragma once
// p2 布局：HBox 嵌 VBox（编译期布局组合演示）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名

// —— 一列音量 + 一列标签，组装成一行 ——
using VolumeColumn = ui::VBox<ui::Id("col"),
	ui::SliderF<ui::Id("volume")>,
	ui::CheckBox<ui::Id("mute")>>;

using RightColumn = ui::VBox<ui::Id("rightCol"),
	ui::Label<ui::Id("right")>>;   // 同套一层 VBox：与左列共享内边距，同一行才对得齐

using ControlRow = ui::HBox<ui::Id("row"), VolumeColumn, RightColumn>;

struct PageLayout {
	static constexpr const char* title = "布局：编译期 HBox/VBox 嵌套";
	static constexpr const char* desc  = "wg2 静态布局 = VBox/HBox/Grid/Wrap/SplitH/SplitV/Overlay 模板容器。"
		"本页 = HBox 嵌 VBox，子节点经模板参数传入，布局嵌布局自然递归。";
	static constexpr std::string_view code = snips::kLayout;

	// 根只列结构：说明 / 控制行 / 注解
	using Root = ui::VBox<ui::Id("root"),
		ui::Label<ui::Id("intro")>,
		ControlRow,
		ui::Label<ui::Id("note")>>;

	Root root;

	// 本页自己的数据
	float volume = 0.6f;
	bool mute = false;

	PageLayout() {
		root.at<ui::Id("intro")>().text = "布局步骤：hint（计算尺寸）→ draw（求解并递归子节点）";

		auto& w_volume = root.at<ui::Id("row"), ui::Id("col"), ui::Id("volume")>();
		w_volume.name = "音量";
		w_volume.value = &volume;

		auto& w_mute = root.at<ui::Id("row"), ui::Id("col"), ui::Id("mute")>();
		w_mute.name = "静音";
		w_mute.checked = &mute;

		root.at<ui::Id("row"), ui::Id("rightCol"), ui::Id("right")>().text = "H 行右格";
		root.at<ui::Id("note")>().text = "布局数据约束：子控件在父容器内，路径访问全程一致。";
	}
};

} // namespace gallery
