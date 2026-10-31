#pragma once
// p23 wg2 控件巡礼：输入 / 显示 / 装饰件 / 页签 在同一棵静态树中。
#include "../gallery_chrome.h"

#include <string>

namespace gallery {

struct PageWg2Catalog {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::SeparatorText<Mk::wg2::Id("input_head")>,
		Mk::wg2::HBox<Mk::wg2::Id("input_row"),
			Mk::wg2::CheckBox<Mk::wg2::Id("check")>,
			Mk::wg2::InputText<Mk::wg2::Id("text"), 32>,
			Mk::wg2::SliderF<Mk::wg2::Id("volume")>
		>,
		Mk::wg2::SeparatorText<Mk::wg2::Id("display_head")>,
		Mk::wg2::HBox<Mk::wg2::Id("display_row"),
			Mk::wg2::ProgressBar<Mk::wg2::Id("bar")>,
			Mk::wg2::LabelText<Mk::wg2::Id("count")>
		>,
		Mk::wg2::Colored<Mk::wg2::Id("colored"),
			Mk::wg2::Label<Mk::wg2::Id("color_label")>
		>,
		Mk::wg2::Tooltip<Mk::wg2::Id("tip"),
			Mk::wg2::Button<Mk::wg2::Id("tip_btn")>
		>,
		Mk::wg2::SubWindow<Mk::wg2::Id("tabwin"),
			Mk::wg2::TabBar<Mk::wg2::Id("tabs"),
				Mk::wg2::Tab<Mk::wg2::Id("tab_a"),
					Mk::wg2::Label<Mk::wg2::Id("tab_a_text")>
				>,
				Mk::wg2::Tab<Mk::wg2::Id("tab_b"),
					Mk::wg2::Label<Mk::wg2::Id("tab_b_text")>
				>
			>
		>
	>;

	Page page;
	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

	float volume = 0.6f;
	bool checked = false;

	PageWg2Catalog() {
		page.setup("wg2：控件巡礼",
			"输入、显示、装饰件、页签等控件全部是直接 struct，"
			"经 VBox/HBox/SubWindow 编译期组合，字符串路径访问。",
			R"(using Root = Mk::wg2::VBox<
    Mk::wg2::Id("root"),
    Mk::wg2::HBox<Mk::wg2::Id("row"),
        Mk::wg2::CheckBox<Mk::wg2::Id("check")>,
        Mk::wg2::SliderF<Mk::wg2::Id("volume")>
    >
>;)");

		root.at<Mk::wg2::Id("input_head")>().name = "输入";
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("check")>().name = "静音";
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("check")>().checked = &checked;
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("text")>().name = "文本";
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("text")>().setName("hello wg2");
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("volume")>().name = "音量";
		root.at<Mk::wg2::Id("input_row"), Mk::wg2::Id("volume")>().value = &volume;

		root.at<Mk::wg2::Id("display_head")>().name = "显示";
		root.at<Mk::wg2::Id("display_row"), Mk::wg2::Id("bar")>().fraction = &volume;
		root.at<Mk::wg2::Id("display_row"), Mk::wg2::Id("count")>().name = "clicks";

		root.at<Mk::wg2::Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
		root.at<Mk::wg2::Id("colored"), Mk::wg2::Id("color_label")>().text = "Colored 装饰件";

		root.at<Mk::wg2::Id("tip"), Mk::wg2::Id("tip_btn")>().label = "悬停看提示";
		root.at<Mk::wg2::Id("tip"), Mk::wg2::Id("tip_btn")>().onClick = [&] { ++clicks; };
		root.at<Mk::wg2::Id("tip")>().text = "这是 Tooltip 装饰件";

		root.at<Mk::wg2::Id("tabwin")>().height = 170.0f;
		root.at<Mk::wg2::Id("tabwin")>().idname = "wg2catalog:tabs";
		root.at<Mk::wg2::Id("tabwin"), Mk::wg2::Id("tabs"), Mk::wg2::Id("tab_a")>().title = "页 A";
		root.at<Mk::wg2::Id("tabwin"), Mk::wg2::Id("tabs"), Mk::wg2::Id("tab_a"), Mk::wg2::Id("tab_a_text")>().text = "编译期 Tab A";
		root.at<Mk::wg2::Id("tabwin"), Mk::wg2::Id("tabs"), Mk::wg2::Id("tab_b")>().title = "页 B";
		root.at<Mk::wg2::Id("tabwin"), Mk::wg2::Id("tabs"), Mk::wg2::Id("tab_b"), Mk::wg2::Id("tab_b_text")>().text = "编译期 Tab B";

		page.addWidget([&](const Mk::wg2::Rect& r) {
			root.at<Mk::wg2::Id("display_row"), Mk::wg2::Id("count")>().value =
				std::to_string(clicks);
			Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
	}

};

} // namespace gallery
