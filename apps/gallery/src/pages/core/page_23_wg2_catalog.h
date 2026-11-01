#pragma once
// p23 wg2 控件巡礼：输入 / 显示 / 装饰件 / 页签 在同一棵静态树中。
#include "../../gallery_chrome.h"

#include <string>

namespace gallery {

struct PageWg2Catalog {
	using Root = VBox<
		Id("root"),
		SeparatorText<Id("input_head")>,
		HBox<Id("input_row"),
			CheckBox<Id("check")>,
			InputText<Id("text"), 32>,
			SliderF<Id("volume")>
		>,
		SeparatorText<Id("display_head")>,
		HBox<Id("display_row"),
			ProgressBar<Id("bar")>,
			LabelText<Id("count")>
		>,
		Colored<Id("colored"),
			Label<Id("color_label")>
		>,
		Tooltip<Id("tip"),
			Button<Id("tip_btn")>
		>,
		SubWindow<Id("tabwin"),
			TabBar<Id("tabs"),
				Tab<Id("tab_a"),
					Label<Id("tab_a_text")>
				>,
				Tab<Id("tab_b"),
					Label<Id("tab_b_text")>
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
			R"(using Root = VBox<
    Id("root"),
    HBox<Id("row"),
        CheckBox<Id("check")>,
        SliderF<Id("volume")>
    >
>;)");

		root.at<Id("input_head")>().name = "输入";
		auto& w_check = root.at<Id("input_row"), Id("check")>();
		w_check.name = "静音";
		w_check.checked = &checked;
		auto& w_text = root.at<Id("input_row"), Id("text")>();
		w_text.name = "文本";
		w_text.setName("hello wg2");
		auto& w_volume = root.at<Id("input_row"), Id("volume")>();
		w_volume.name = "音量";
		w_volume.value = &volume;

		root.at<Id("display_head")>().name = "显示";
		root.at<Id("display_row"), Id("bar")>().fraction = &volume;
		root.at<Id("display_row"), Id("count")>().name = "clicks";

		root.at<Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
		root.at<Id("colored"), Id("color_label")>().text = "Colored 装饰件";

		auto& w_tip_btn = root.at<Id("tip"), Id("tip_btn")>();
		w_tip_btn.label = "悬停看提示";
		w_tip_btn.onClick = [&] { ++clicks; };
		root.at<Id("tip")>().text = "这是 Tooltip 装饰件";

		auto& w_tabwin = root.at<Id("tabwin")>();
		w_tabwin.height = 170.0f;
		w_tabwin.idname = "wg2catalog:tabs";
		root.at<Id("tabwin"), Id("tabs"), Id("tab_a")>().title = "页 A";
		root.at<Id("tabwin"), Id("tabs"), Id("tab_a"), Id("tab_a_text")>().text = "编译期 Tab A";
		root.at<Id("tabwin"), Id("tabs"), Id("tab_b")>().title = "页 B";
		root.at<Id("tabwin"), Id("tabs"), Id("tab_b"), Id("tab_b_text")>().text = "编译期 Tab B";

		bindPage(page, root);
	}

};

} // namespace gallery
