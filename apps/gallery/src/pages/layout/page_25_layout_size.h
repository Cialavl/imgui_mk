#pragma once
// p25 布局·尺寸策略：fixed / pref / min / max / stretch / em（静态，随字号联动）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutSize {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "布局·尺寸策略：fixed / pref / min / max / stretch / em";
	static constexpr const char* desc  = "state.size 覆盖控件尺寸：fixedW 钉死、prefW 期望值、stretch 分配剩余、"
			"prefWEm 按字号换算（字体缩放后宽度自动跟随）。";
	static constexpr std::string_view code = snips::kLayoutSize;

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

	Root root;

	PageLayoutSize() {
		

		root.at<Id("intro")>().text = "同一行三种策略：固定 170 / 期望 200 / 吃掉剩余";
		auto& w_fixed = root.at<Id("row1"), Id("fixed")>();
		w_fixed.label = "fixedW = 170";
		w_fixed.state.size.fixedW = 170.0f;
		auto& w_pref = root.at<Id("row1"), Id("pref")>();
		w_pref.label = "prefW = 200";
		w_pref.state.size.prefW = 200.0f;
		auto& w_stretch = root.at<Id("row1"), Id("stretch")>();
		w_stretch.label = "stretch = 1";
		w_stretch.state.stretch = 1.0f;

		auto& w_em = root.at<Id("row2"), Id("em")>();
		w_em.label = "prefWEm = 9（跟字号）";
		w_em.state.size.prefWEm = 9.0f;
		auto& w_fixedH = root.at<Id("row2"), Id("fixedH")>();
		w_fixedH.text = "固定高 48";
		w_fixedH.state.size.fixedH = 48.0f;
		auto& w_max = root.at<Id("row2"), Id("max")>();
		w_max.label = "maxW = 120（可被压缩）";
		w_max.state.size.maxW = 120.0f;
		root.at<Id("note")>().text = "在主题页切换字号 18/24，em 宽度会跟着变，像素值不变。";

	}
};

} // namespace gallery
