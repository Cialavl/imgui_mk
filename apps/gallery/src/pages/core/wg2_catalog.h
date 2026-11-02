#pragma once
// p23 wg2 控件巡礼：输入 / 显示 / 装饰件 / 页签 在同一棵静态树中。
#include "../../page_frame.h"

#include <string>

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageWg2Catalog {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "wg2：控件巡礼";
	static constexpr const char* desc  = "输入、显示、装饰件、页签等控件全部是直接 struct，"
			"经 VBox/HBox/SubWindow 编译期组合，字符串路径访问。";
	static constexpr std::string_view code = R"(using Root = VBox<
    Id("root"),
    HBox<Id("row"),
        CheckBox<Id("check")>,
        SliderF<Id("volume")>
    >
>;)";

	using Root = ui::VBox<
		ui::Id("root"),
		ui::SeparatorText<ui::Id("input_head")>,
		ui::HBox<ui::Id("input_row"),
			ui::CheckBox<ui::Id("check")>,
			InputText<ui::Id("text"), 32>,
			ui::SliderF<ui::Id("volume")>
		>,
		ui::SeparatorText<ui::Id("display_head")>,
		ui::HBox<ui::Id("display_row"),
			ui::ProgressBar<ui::Id("bar")>,
			ui::LabelText<ui::Id("count")>
		>,
		ui::Colored<ui::Id("colored"),
			ui::Label<ui::Id("color_label")>
		>,
		ui::Tooltip<ui::Id("tip"),
			ui::Button<ui::Id("tip_btn")>
		>,
		ui::SubWindow<ui::Id("tabwin"),
			ui::TabBar<ui::Id("tabs"),
				ui::Tab<ui::Id("tab_a"),
					ui::Label<ui::Id("tab_a_text")>
				>,
				ui::Tab<ui::Id("tab_b"),
					ui::Label<ui::Id("tab_b_text")>
				>
			>
		>
	>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;

	float volume = 0.6f;
	bool checked = false;

	PageWg2Catalog() {
		

		root.at<ui::Id("input_head")>().name = "输入";
		auto& w_check = root.at<ui::Id("input_row"), ui::Id("check")>();
		w_check.name = "静音";
		w_check.checked = &checked;
		auto& w_text = root.at<ui::Id("input_row"), ui::Id("text")>();
		w_text.name = "文本";
		w_text.setName("hello wg2");
		auto& w_volume = root.at<ui::Id("input_row"), ui::Id("volume")>();
		w_volume.name = "音量";
		w_volume.value = &volume;

		root.at<ui::Id("display_head")>().name = "显示";
		root.at<ui::Id("display_row"), ui::Id("bar")>().fraction = &volume;
		root.at<ui::Id("display_row"), ui::Id("count")>().name = "clicks";

		root.at<ui::Id("colored")>().color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
		root.at<ui::Id("colored"), ui::Id("color_label")>().text = "Colored 装饰件";

		auto& w_tip_btn = root.at<ui::Id("tip"), ui::Id("tip_btn")>();
		w_tip_btn.label = "悬停看提示";
		w_tip_btn.onClick = [&] { ++clicks; };
		root.at<ui::Id("tip")>().text = "这是 Tooltip 装饰件";

		auto& w_tabwin = root.at<ui::Id("tabwin")>();
		w_tabwin.height = 170.0f;
		w_tabwin.idname = "wg2catalog:tabs";
		root.at<ui::Id("tabwin"), ui::Id("tabs"), ui::Id("tab_a")>().title = "页 A";
		root.at<ui::Id("tabwin"), ui::Id("tabs"), ui::Id("tab_a"), ui::Id("tab_a_text")>().text = "编译期 Tab A";
		root.at<ui::Id("tabwin"), ui::Id("tabs"), ui::Id("tab_b")>().title = "页 B";
		root.at<ui::Id("tabwin"), ui::Id("tabs"), ui::Id("tab_b"), ui::Id("tab_b_text")>().text = "编译期 Tab B";

	}

};

} // namespace gallery
