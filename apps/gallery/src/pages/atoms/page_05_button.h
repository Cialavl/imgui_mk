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

root.at<Id("plain")>().label = "普通按钮";
root.at<Id("plain")>().onClick = [&] { ++clicks; };
root.at<Id("count")>().label = "计数按钮";
root.at<Id("count")>().onClick = [&] { clicks += 10; };
root.at<Id("imgbtn")>().tex = tex;
root.at<Id("imgbtn")>().size = ImVec2(96, 96);
root.at<Id("imgbtn")>().onClick = [&] { ++imgBtnClicks; };
root.at<Id("clicks")>().name = "图片按钮点击";
root.at<Id("clicks")>().valueFn = [&] { return std::to_string(imgBtnClicks); };
root.at<Id("imageRow"), Id("imageA")>().tex = tex;
root.at<Id("imageRow"), Id("imageB")>().tex = tex;

bindPage(page, root);
}

};

} // namespace gallery
