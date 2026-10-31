#pragma once
// p18 主题自适应：全局 Theme 切换 + rescale（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageTheme {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::HBox<Mk::wg2::Id("row"),
Mk::wg2::Button<Mk::wg2::Id("dark")>,
Mk::wg2::Button<Mk::wg2::Id("light")>,
Mk::wg2::Button<Mk::wg2::Id("accent")>
>,
Mk::wg2::Label<Mk::wg2::Id("scopeNote")>,
Mk::wg2::VBox<Mk::wg2::Id("compact"),
Mk::wg2::Button<Mk::wg2::Id("compact1")>,
Mk::wg2::Button<Mk::wg2::Id("compact2")>
>,
Mk::wg2::Label<Mk::wg2::Id("rescaleNote")>,
Mk::wg2::Button<Mk::wg2::Id("rescale")>
>;

Page page;
Root root;

PageTheme(std::function<void(const Mk::wg2::Theme&)> applyTheme) {
page.setup("主题与自适应",
"wg2 全局 Theme 切换与 rescale 重载字号；局部主题可用 Colored/样式作用域代替。",
snips::kTheme);

root.at<Mk::wg2::Id("row"), Mk::wg2::Id("dark")>().label = "Dark";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("dark")>().onClick = [&] { applyTheme(Theme::dark()); };
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("light")>().label = "Light";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("light")>().onClick = [&] { applyTheme(Theme::light()); };
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("accent")>().label = "Accent 红";
root.at<Mk::wg2::Id("row"), Mk::wg2::Id("accent")>().onClick = [&] {
applyTheme(Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
};
root.at<Mk::wg2::Id("scopeNote")>().text = "局部主题：wg2 当前使用 Colored 装饰件替代 Styled";
root.at<Mk::wg2::Id("compact")>().spacing = 2.0f;
root.at<Mk::wg2::Id("compact"), Mk::wg2::Id("compact1")>().label = "紧凑按钮一";
root.at<Mk::wg2::Id("compact"), Mk::wg2::Id("compact2")>().label = "紧凑按钮二";
root.at<Mk::wg2::Id("rescaleNote")>().text = "rescale(24)：重载字号图标集；布局/控件/行高/间距全等比";
	root.at<Mk::wg2::Id("rescale")>().label = "切换字号 18 / 24";
root.at<Mk::wg2::Id("rescale")>().onClick = [&] {
const float cur = engineFontSize();
engineRescale(cur >= 23.5f ? 18.0f : 24.0f);
};

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}


};

} // namespace gallery
