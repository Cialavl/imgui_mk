#pragma once
// p25 布局·尺寸策略：fixed / pref / min / max / stretch / em（静态，随字号联动）
#include "../gallery_chrome.h"

namespace gallery {

struct PageLayoutSize {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("intro")>,
		Mk::wg2::HBox<Mk::wg2::Id("row1"),
			Mk::wg2::Button<Mk::wg2::Id("fixed")>,
			Mk::wg2::Button<Mk::wg2::Id("pref")>,
			Mk::wg2::Button<Mk::wg2::Id("stretch")>>,
		Mk::wg2::HBox<Mk::wg2::Id("row2"),
			Mk::wg2::Button<Mk::wg2::Id("em")>,
			Mk::wg2::Label<Mk::wg2::Id("fixedH")>,
			Mk::wg2::Button<Mk::wg2::Id("max")>>,
		Mk::wg2::Label<Mk::wg2::Id("note")>>;

	Page page;
	Root root;

	PageLayoutSize() {
		page.setup("布局·尺寸策略：fixed / pref / min / max / stretch / em",
			"state.size 覆盖控件尺寸：fixedW 钉死、prefW 期望值、stretch 分配剩余、"
			"prefWEm 按字号换算（字体缩放后宽度自动跟随）。",
			snips::kLayoutSize);

		root.at<Mk::wg2::Id("intro")>().text = "同一行三种策略：固定 170 / 期望 200 / 吃掉剩余";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("fixed")>().label = "fixedW = 170";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("fixed")>().state.size.fixedW = 170.0f;
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("pref")>().label = "prefW = 200";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("pref")>().state.size.prefW = 200.0f;
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("stretch")>().label = "stretch = 1";
		root.at<Mk::wg2::Id("row1"), Mk::wg2::Id("stretch")>().state.stretch = 1.0f;

		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("em")>().label = "prefWEm = 9（跟字号）";
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("em")>().state.size.prefWEm = 9.0f;
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("fixedH")>().text = "固定高 48";
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("fixedH")>().state.size.fixedH = 48.0f;
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("max")>().label = "maxW = 120（可被压缩）";
		root.at<Mk::wg2::Id("row2"), Mk::wg2::Id("max")>().state.size.maxW = 120.0f;
		root.at<Mk::wg2::Id("note")>().text = "在主题页切换字号 18/24，em 宽度会跟着变，像素值不变。";

		page.addWidget([&](const Mk::wg2::Rect& r) { Mk::wg2::layout(root, r); },
			[&](const Mk::wg2::Size& avail) { return root.hint(avail); });
	}
};

} // namespace gallery
