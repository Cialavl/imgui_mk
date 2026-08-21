#pragma once
// p5 按钮图片：Button/ImageButton/Image（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageButton {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Button<Mk::wg2::Id("plain")>,
Mk::wg2::Button<Mk::wg2::Id("count")>,
Mk::wg2::ImageButton<Mk::wg2::Id("imgbtn")>,
Mk::wg2::LabelText<Mk::wg2::Id("clicks")>,
Mk::wg2::HBox<Mk::wg2::Id("imageRow"),
Mk::wg2::Image<Mk::wg2::Id("imageA")>,
Mk::wg2::Image<Mk::wg2::Id("imageB")>
>
>;

Page page;
Root root;

PageButton(State& s) {
page.setup("按钮与图片",
"wg2 Button（onClick）/ ImageButton / Image（loadTextureRGBA 生成纹理）。",
snips::kButton);

root.at<Mk::wg2::Id("plain")>().label = "普通按钮";
root.at<Mk::wg2::Id("plain")>().onClick = [&] { ++s.clicks; };
root.at<Mk::wg2::Id("count")>().label = "计数按钮";
root.at<Mk::wg2::Id("count")>().onClick = [&] { s.clicks += 10; };
root.at<Mk::wg2::Id("imgbtn")>().tex = s.tex;
root.at<Mk::wg2::Id("imgbtn")>().size = ImVec2(96, 96);
root.at<Mk::wg2::Id("imgbtn")>().onClick = [&] { ++s.imgBtnClicks; };
root.at<Mk::wg2::Id("clicks")>().name = "图片按钮点击";
root.at<Mk::wg2::Id("clicks")>().valueFn = [&s] { return std::to_string(s.imgBtnClicks); };
root.at<Mk::wg2::Id("imageRow"), Mk::wg2::Id("imageA")>().tex = s.tex;
root.at<Mk::wg2::Id("imageRow"), Mk::wg2::Id("imageB")>().tex = s.tex;

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
