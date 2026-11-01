#pragma once
// p5 按钮图片：Button/ImageButton/Image（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageButton {
using Root = VBox<
Id("root"),
Button<Id("plain")>,
Button<Id("count")>,
ImageButton<Id("imgbtn")>,
LabelText<Id("clicks")>,
HBox<Id("imageRow"),
Image<Id("imageA")>,
Image<Id("imageB")>
>
>;

Page<Root> page;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;
	int imgBtnClicks = 0;

PageButton(ImTextureID tex) {
page.setup("按钮与图片",
"wg2 Button（onClick）/ ImageButton / Image（loadTextureRGBA 生成纹理）。",
snips::kButton);

auto& w_plain = page.root.at<Id("plain")>();
w_plain.label = "普通按钮";
w_plain.onClick = [&] { ++clicks; };
auto& w_count = page.root.at<Id("count")>();
w_count.label = "计数按钮";
w_count.onClick = [&] { clicks += 10; };
auto& w_imgbtn = page.root.at<Id("imgbtn")>();
w_imgbtn.tex = tex;
w_imgbtn.size = ImVec2(96, 96);
w_imgbtn.onClick = [&] { ++imgBtnClicks; };
auto& w_clicks = page.root.at<Id("clicks")>();
w_clicks.name = "图片按钮点击";
w_clicks.valueFn = [&] { return std::to_string(imgBtnClicks); };
page.root.at<Id("imageRow"), Id("imageA")>().tex = tex;
page.root.at<Id("imageRow"), Id("imageB")>().tex = tex;

}

};

} // namespace gallery
