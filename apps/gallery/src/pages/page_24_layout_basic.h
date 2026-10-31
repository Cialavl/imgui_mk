#pragma once
// p24 布局·基础：VBox 堆叠 / HBox 横排 / 间距与边距（简单静态）
#include "../gallery_chrome.h"

namespace gallery {

struct PageLayoutBasic {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("intro")>,
		Mk::wg2::HBox<Mk::wg2::Id("row1"),
			Mk::wg2::Button<Mk::wg2::Id("a1")>,
			Mk::wg2::Button<Mk::wg2::Id("a2")>,
			Mk::wg2::Button<Mk::wg2::Id("a3")>>,
		Mk::wg2::HBox<Mk::wg2::Id("row2"),
			Mk::wg2::Button<Mk::wg2::Id("b1")>,
			Mk::wg2::Button<Mk::wg2::Id("b2")>>,
		Mk::wg2::VBox<Mk::wg2::Id("stack"),
			Mk::wg2::Label<Mk::wg2::Id("s1")>,
			Mk::wg2::Label<Mk::wg2::Id("s2")>,
			Mk::wg2::Label<Mk::wg2::Id("s3")>>,
		Mk::wg2::Label<Mk::wg2::Id("note")>>;

	Page page;
	Root root;

	PageLayoutBasic() {
		page.setup("布局·基础：VBox 堆叠 / HBox 横排",
			"最简单的两种组合：VBox 按行堆叠、HBox 按列排布；"
			"子项间距由容器 spacing、整体边距由 marginX/marginY 控制。",
			snips::kLayoutBasic);

		root.at<Mk::wg2::Id("intro")>().text = "HBox：三个按钮自动均分宽度（都可增长）";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("a1")>().label = "左";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("a2")>().label = "中";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("a3")>().label = "右";

		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("b1")>().label = "固定 160";
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("b1")>().state.size.fixedW = 160.0f;
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("b2")>().label = "吃掉剩余宽度（stretch = 1）";
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("b2")>().state.stretch = 1.0f;

		root.at<Mk::wg2::Id("stack"), Mk::wg2::Id("s1")>().text = "VBox 第一行";
		root.at<Mk::wg2::Id("stack"), Mk::wg2::Id("s2")>().text = "VBox 第二行";
		root.at<Mk::wg2::Id("stack"), Mk::wg2::Id("s3")>().text = "VBox 第三行";
		root.at<Mk::wg2::Id("note")>().text = "把窗口拖宽：HBox 跟着变宽，VBox 只吃高度。";

		bindPage(page, root);
	}
};

} // namespace gallery
