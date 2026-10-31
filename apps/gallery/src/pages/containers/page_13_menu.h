#pragma once
// p13 菜单面板：SubWindow 面板（wg2 编译期子节点）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageMenu {
using Root = VBox<
Id("root"),
SubWindow<Id("panel"),
SliderF<Id("slider")>,
LabelText<Id("frames")>
>,
Label<Id("note")>
>;

Page page;
Root root;
	Mk::window* win = nullptr;   // 形参存成员，避免 [&] 捕获形参后悬垂

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float volume = 0.6f;

PageMenu(Mk::window* w) : win(w) {
page.setup("菜单与面板",
"wg2 Panel = SubWindow 别名；面板内容经模板参数编译期传入。",
snips::kPanel);

auto& _w1 = root.at<Id("panel")>();
_w1.idname = "面板";
_w1.height = 110.0f;
auto& _w2 = root.at<Id("panel"), Id("slider")>();
_w2.name = "面板内滑块";
_w2.value = &volume;
auto& _w3 = root.at<Id("panel"), Id("frames")>();
_w3.name = "帧";
_w3.valueFn = [this] { return std::to_string(win->frameIndex()); };
root.at<Id("note")>().text = "Panel 是浮动窗口；菜单栏是常驻点（主窗口顶部）。";

bindPage(page, root);
}

};

} // namespace gallery
