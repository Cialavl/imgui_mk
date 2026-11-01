#pragma once
// p18 主题自适应：全局 Theme 切换 + rescale（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageTheme {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "主题与自适应";
	static constexpr const char* desc  = "wg2 全局 Theme 切换与 rescale 重载字号；局部主题可用 ui::Colored/样式作用域代替。";
	static constexpr std::string_view code = snips::kTheme;

using Root = ui::VBox<
ui::Id("root"),
ui::HBox<ui::Id("row"),
ui::Button<ui::Id("dark")>,
ui::Button<ui::Id("light")>,
ui::Button<ui::Id("accent")>
>,
ui::Label<ui::Id("scopeNote")>,
ui::VBox<ui::Id("compact"),
ui::Button<ui::Id("compact1")>,
ui::Button<ui::Id("compact2")>
>,
ui::Label<ui::Id("rescaleNote")>,
ui::Button<ui::Id("rescale")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	Theme theme = Theme::dark();

PageTheme() {


auto& w_dark = root.at<ui::Id("row"), ui::Id("dark")>();
w_dark.label = "Dark";
w_dark.onClick = [&] { Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::dark()}); };
auto& w_light = root.at<ui::Id("row"), ui::Id("light")>();
w_light.label = "Light";
w_light.onClick = [&] { Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::light()}); };
auto& w_accent = root.at<ui::Id("row"), ui::Id("accent")>();
w_accent.label = "Accent 红";
w_accent.onClick = [&] {
Mk::mkui<Mk::window>::RegistrationWindow().emit(ThemeChanged{Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f))});
};
root.at<ui::Id("scopeNote")>().text = "局部主题：wg2 当前使用 ui::Colored 装饰件替代 Styled";
root.at<ui::Id("compact")>().spacing = 2.0f;
root.at<ui::Id("compact"), ui::Id("compact1")>().label = "紧凑按钮一";
root.at<ui::Id("compact"), ui::Id("compact2")>().label = "紧凑按钮二";
root.at<ui::Id("rescaleNote")>().text = "rescale(24)：重载字号图标集；布局/控件/行高/间距全等比";
	auto& w_rescale = root.at<ui::Id("rescale")>();
	w_rescale.label = "切换字号 18 / 24";
	w_rescale.onClick = [&] {
const float cur = engineFontSize();
engineRescale(cur >= 23.5f ? 18.0f : 24.0f);
};

}


};

} // namespace gallery
