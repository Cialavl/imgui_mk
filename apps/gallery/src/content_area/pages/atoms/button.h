#pragma once
// p5 按钮图片：ui::Button/ui::ImageButton/ui::Image（wg2）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageButton {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "按钮与图片";
	static constexpr const char* desc  = "wg2 Button（onClick）/ ImageButton / Image（不设纹理时用引擎默认棋盘）。";
	static constexpr std::string_view code = snips::kButton;

using Root = ui::VBox<
ui::Id("root"),
ui::Button<ui::Id("plain")>,
ui::Button<ui::Id("count")>,
ui::ImageButton<ui::Id("imgbtn")>,
ui::LabelText<ui::Id("clicks")>,
ui::HBox<ui::Id("imageRow"),
ui::Image<ui::Id("imageA")>,
ui::Image<ui::Id("imageB")>
>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;
	int imgBtnClicks = 0;

PageButton() {


auto& w_plain = root.at<ui::Id("plain")>();
w_plain.label = "普通按钮";
w_plain.onClick = [&] { ++clicks; };
auto& w_count = root.at<ui::Id("count")>();
w_count.label = "计数按钮";
w_count.onClick = [&] { clicks += 10; };
auto& w_imgbtn = root.at<ui::Id("imgbtn")>();
w_imgbtn.size = ImVec2(96, 96);   // 纹理留空 → 引擎默认棋盘
w_imgbtn.onClick = [&] { ++imgBtnClicks; };
auto& w_clicks = root.at<ui::Id("clicks")>();
w_clicks.name = "图片按钮点击";
w_clicks.valueFn = [&] { return std::to_string(imgBtnClicks); };

}

};

} // namespace gallery
