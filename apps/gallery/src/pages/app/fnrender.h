#pragma once
// p21 FnRender：裸 ImGui 绘制函数包装为 wg2 节点
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


// FnRender 页的裸绘制桥：自带演示值，不依赖任何外部状态
struct FnDrawerImpl {

	void operator()() const { ImGui::ProgressBar(0.42f, ImVec2(-FLT_MIN, 0), "FnRender 演示条"); }
};

struct PageFnRender {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "FnRender 裸绘制";
	static constexpr const char* desc  = "wg2 FnWidget 包装裸 ImGui 绘制函数（F 类型默认构造，状态用指针捕获）。";
	static constexpr std::string_view code = snips::kFnRender;
using Root = ui::VBox<
ui::Id("root"),
FnWidget<ui::Id("fnView"), FnDrawerImpl>,
ui::Label<ui::Id("note")>
>;

	Root root;

PageFnRender() {


root.at<ui::Id("fnView")>().fn = FnDrawerImpl{};   // 自带演示值，不再依赖共享状态
root.at<ui::Id("note")>().text = "上方进度条用裸 ImGui 函数绘制，由 FnWidget 节点挂入 wg2 布局。";

}

};

} // namespace gallery
