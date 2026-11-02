#pragma once
// p24 布局·基础：ui::VBox 堆叠 / ui::HBox 横排 / 间距与边距（简单静态）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageLayoutBasic {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "布局·基础：VBox 堆叠 / HBox 横排";
	static constexpr const char* desc  = "最简单的两种组合：VBox 按行堆叠、HBox 按列排布；"
			"子项间距由容器 spacing、整体边距由 marginX/marginY 控制。";
	static constexpr std::string_view code = snips::kLayoutBasic;

	using Root = ui::VBox<
		ui::Id("root"),
		ui::Label<ui::Id("intro")>,
		ui::HBox<ui::Id("row1"),
			ui::Button<ui::Id("a1")>,
			ui::Button<ui::Id("a2")>,
			ui::Button<ui::Id("a3")>>,
		ui::HBox<ui::Id("row2"),
			ui::Button<ui::Id("b1")>,
			ui::Button<ui::Id("b2")>>,
		ui::VBox<ui::Id("stack"),
			ui::Label<ui::Id("s1")>,
			ui::Label<ui::Id("s2")>,
			ui::Label<ui::Id("s3")>>,
		ui::Label<ui::Id("note")>>;

	Root root;

	PageLayoutBasic() {
		

		root.at<ui::Id("intro")>().text = "HBox：三个按钮自动均分宽度（都可增长）";

		auto& row1 = root.at<ui::Id("row1")>();
		row1.at<ui::Id("a1")>().label = "左";
		row1.at<ui::Id("a2")>().label = "中";
		row1.at<ui::Id("a3")>().label = "右";

		auto& b1 = root.at<ui::Id("row2"), ui::Id("b1")>();
		b1.label = "固定 160";
		b1.state.size.fixedW = 160.0f;

		auto& b2 = root.at<ui::Id("row2"), ui::Id("b2")>();
		b2.label = "吃掉剩余宽度（stretch = 1）";
		b2.state.stretch = 1.0f;

		auto& stack = root.at<ui::Id("stack")>();
		stack.at<ui::Id("s1")>().text = "VBox 第一行";
		stack.at<ui::Id("s2")>().text = "VBox 第二行";
		stack.at<ui::Id("s3")>().text = "VBox 第三行";
		root.at<ui::Id("note")>().text = "把窗口拖宽：HBox 跟着变宽，VBox 只吃高度。";

	}
};

} // namespace gallery
