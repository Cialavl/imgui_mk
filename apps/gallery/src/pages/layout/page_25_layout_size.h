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
		auto& _w1 = root.at<Id("row1"), Id("fixed")>();
		_w1.label = "fixedW = 170";
		_w1.state.size.fixedW = 170.0f;
		auto& _w2 = root.at<Id("row1"), Id("pref")>();
		_w2.label = "prefW = 200";
		_w2.state.size.prefW = 200.0f;
		auto& _w3 = root.at<Id("row1"), Id("stretch")>();
		_w3.label = "stretch = 1";
		_w3.state.stretch = 1.0f;

		auto& _w4 = root.at<Id("row2"), Id("em")>();
		_w4.label = "prefWEm = 9（跟字号）";
		_w4.state.size.prefWEm = 9.0f;
		auto& _w5 = root.at<Id("row2"), Id("fixedH")>();
		_w5.text = "固定高 48";
		_w5.state.size.fixedH = 48.0f;
		auto& _w6 = root.at<Id("row2"), Id("max")>();
		_w6.label = "maxW = 120（可被压缩）";
		_w6.state.size.maxW = 120.0f;
		root.at<Id("note")>().text = "在主题页切换字号 18/24，em 宽度会跟着变，像素值不变。";

		bindPage(page, root);
	}
};

} // namespace gallery
