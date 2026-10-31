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

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int clicks = 0;
	int imgBtnClicks = 0;

PageButton(ImTextureID tex) {
page.setup("按钮与图片",
"wg2 Button（onClick）/ ImageButton / Image（loadTextureRGBA 生成纹理）。",
snips::kButton);

auto& _w1 = root.at<Id("plain")>();
_w1.label = "普通按钮";
_w1.onClick = [&] { ++clicks; };
auto& _w2 = root.at<Id("count")>();
_w2.label = "计数按钮";
_w2.onClick = [&] { clicks += 10; };
auto& _w3 = root.at<Id("imgbtn")>();
_w3.tex = tex;
_w3.size = ImVec2(96, 96);
_w3.onClick = [&] { ++imgBtnClicks; };
auto& _w4 = root.at<Id("clicks")>();
_w4.name = "图片按钮点击";
_w4.valueFn = [&] { return std::to_string(imgBtnClicks); };
root.at<Id("imageRow"), Id("imageA")>().tex = tex;
root.at<Id("imageRow"), Id("imageB")>().tex = tex;

bindPage(page, root);
}

};

} // namespace gallery
