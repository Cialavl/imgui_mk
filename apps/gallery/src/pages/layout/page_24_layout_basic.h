#pragma once
// p24 布局·基础：VBox 堆叠 / HBox 横排 / 间距与边距（简单静态）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutBasic {
	using Root = VBox<
		Id("root"),
		Label<Id("intro")>,
		HBox<Id("row1"),
			Button<Id("a1")>,
			Button<Id("a2")>,
			Button<Id("a3")>>,
		HBox<Id("row2"),
			Button<Id("b1")>,
			Button<Id("b2")>>,
		VBox<Id("stack"),
			Label<Id("s1")>,
			Label<Id("s2")>,
			Label<Id("s3")>>,
		Label<Id("note")>>;

	Page<Root> page;

	PageLayoutBasic() {
		page.setup("布局·基础：VBox 堆叠 / HBox 横排",
			"最简单的两种组合：VBox 按行堆叠、HBox 按列排布；"
			"子项间距由容器 spacing、整体边距由 marginX/marginY 控制。",
			snips::kLayoutBasic);

		page.root.at<Id("intro")>().text = "HBox：三个按钮自动均分宽度（都可增长）";

		auto& row1 = page.root.at<Id("row1")>();
		row1.at<Id("a1")>().label = "左";
		row1.at<Id("a2")>().label = "中";
		row1.at<Id("a3")>().label = "右";

		auto& b1 = page.root.at<Id("row2"), Id("b1")>();
		b1.label = "固定 160";
		b1.state.size.fixedW = 160.0f;

		auto& b2 = page.root.at<Id("row2"), Id("b2")>();
		b2.label = "吃掉剩余宽度（stretch = 1）";
		b2.state.stretch = 1.0f;

		auto& stack = page.root.at<Id("stack")>();
		stack.at<Id("s1")>().text = "VBox 第一行";
		stack.at<Id("s2")>().text = "VBox 第二行";
		stack.at<Id("s3")>().text = "VBox 第三行";
		page.root.at<Id("note")>().text = "把窗口拖宽：HBox 跟着变宽，VBox 只吃高度。";

	}
};

} // namespace gallery
