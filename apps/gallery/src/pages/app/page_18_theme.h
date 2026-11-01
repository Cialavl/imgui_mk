#pragma once
// p18 主题自适应：全局 Theme 切换 + rescale（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageTheme {
using Root = VBox<
Id("root"),
HBox<Id("row"),
Button<Id("dark")>,
Button<Id("light")>,
Button<Id("accent")>
>,
Label<Id("scopeNote")>,
VBox<Id("compact"),
Button<Id("compact1")>,
Button<Id("compact2")>
>,
Label<Id("rescaleNote")>,
Button<Id("rescale")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	Theme theme = Theme::dark();

PageTheme() {
page.setup("主题与自适应",
"wg2 全局 Theme 切换与 rescale 重载字号；局部主题可用 Colored/样式作用域代替。",
snips::kTheme);

auto& w_dark = root.at<Id("row"), Id("dark")>();
w_dark.label = "Dark";
w_dark.onClick = [&] { Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::dark()}); };
auto& w_light = root.at<Id("row"), Id("light")>();
w_light.label = "Light";
w_light.onClick = [&] { Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::light()}); };
auto& w_accent = root.at<Id("row"), Id("accent")>();
w_accent.label = "Accent 红";
w_accent.onClick = [&] {
Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f))});
};
root.at<Id("scopeNote")>().text = "局部主题：wg2 当前使用 Colored 装饰件替代 Styled";
root.at<Id("compact")>().spacing = 2.0f;
root.at<Id("compact"), Id("compact1")>().label = "紧凑按钮一";
root.at<Id("compact"), Id("compact2")>().label = "紧凑按钮二";
root.at<Id("rescaleNote")>().text = "rescale(24)：重载字号图标集；布局/控件/行高/间距全等比";
	auto& w_rescale = root.at<Id("rescale")>();
	w_rescale.label = "切换字号 18 / 24";
	w_rescale.onClick = [&] {
const float cur = engineFontSize();
engineRescale(cur >= 23.5f ? 18.0f : 24.0f);
};

	bindPage(page, root);
}


};

} // namespace gallery
