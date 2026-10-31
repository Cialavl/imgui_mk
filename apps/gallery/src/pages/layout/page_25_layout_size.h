#pragma once
// p25 布局·尺寸策略：fixed / pref / min / max / stretch / em（静态，随字号联动）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutSize {
	using Root = VBox<
		Id("root"),
		Label<Id("intro")>,
		HBox<Id("row1"),
			Button<Id("fixed")>,
			Button<Id("pref")>,
			Button<Id("stretch")>>,
		HBox<Id("row2"),
			Button<Id("em")>,
			Label<Id("fixedH")>,
			Button<Id("max")>>,
		Label<Id("note")>>;

	Page page;
	Root root;

	PageLayoutSize() {
		page.setup("布局·尺寸策略：fixed / pref / min / max / stretch / em",
			"state.size 覆盖控件尺寸：fixedW 钉死、prefW 期望值、stretch 分配剩余、"
			"prefWEm 按字号换算（字体缩放后宽度自动跟随）。",
			snips::kLayoutSize);

		root.at<Id("intro")>().text = "同一行三种策略：固定 170 / 期望 200 / 吃掉剩余";
		root.at<Id("row1"), Id("fixed")>().label = "fixedW = 170";
		root.at<Id("row1"), Id("fixed")>().state.size.fixedW = 170.0f;
		root.at<Id("row1"), Id("pref")>().label = "prefW = 200";
		root.at<Id("row1"), Id("pref")>().state.size.prefW = 200.0f;
		root.at<Id("row1"), Id("stretch")>().label = "stretch = 1";
		root.at<Id("row1"), Id("stretch")>().state.stretch = 1.0f;

		root.at<Id("row2"), Id("em")>().label = "prefWEm = 9（跟字号）";
		root.at<Id("row2"), Id("em")>().state.size.prefWEm = 9.0f;
		root.at<Id("row2"), Id("fixedH")>().text = "固定高 48";
		root.at<Id("row2"), Id("fixedH")>().state.size.fixedH = 48.0f;
		root.at<Id("row2"), Id("max")>().label = "maxW = 120（可被压缩）";
		root.at<Id("row2"), Id("max")>().state.size.maxW = 120.0f;
		root.at<Id("note")>().text = "在主题页切换字号 18/24，em 宽度会跟着变，像素值不变。";

		bindPage(page, root);
	}
};

} // namespace gallery
