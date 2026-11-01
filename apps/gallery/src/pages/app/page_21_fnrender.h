#pragma once
// p21 FnRender：裸 ImGui 绘制函数包装为 wg2 节点
#include "../../gallery_chrome.h"

namespace gallery {

// FnRender 页的裸绘制桥：自带演示值，不依赖任何外部状态
struct FnDrawerImpl {
	void operator()() const { ImGui::ProgressBar(0.42f, ImVec2(-FLT_MIN, 0), "FnRender 演示条"); }
};

struct PageFnRender {
using Root = VBox<
Id("root"),
FnWidget<Id("fnView"), FnDrawerImpl>,
Label<Id("note")>
>;

Page<Root> page;

PageFnRender() {
page.setup("FnRender 裸绘制",
"wg2 FnWidget 包装裸 ImGui 绘制函数（F 类型默认构造，状态用指针捕获）。",
snips::kFnRender);

page.root.at<Id("fnView")>().fn = FnDrawerImpl{};   // 自带演示值，不再依赖共享状态
page.root.at<Id("note")>().text = "上方进度条用裸 ImGui 函数绘制，由 FnWidget 节点挂入 wg2 布局。";

}

};

} // namespace gallery
