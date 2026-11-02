#pragma once
// p3 分格模式：ui::SplitH/ui::SplitV/ui::Wrap/ui::Grid（wg2 编译期布局）
#include "../../demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageSplit {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "分格模式";
	static constexpr const char* desc  = "wg2 SplitH/SplitV 是 HBox/VBox 别名；stretch 与 fixed size 由 setStretch/sizeOf 设置。"
"Wrap 换行、Grid 固定列网格。";
	static constexpr std::string_view code = snips::kSplit;

using Root = ui::VBox<
ui::Id("root"),
ui::Label<ui::Id("intro")>,
ui::SplitH<ui::Id("splitH"),
ui::Label<ui::Id("sfA")>,
ui::Label<ui::Id("sfB")>,
ui::Label<ui::Id("sfC")>
>,
ui::SplitV<ui::Id("splitV"),
ui::Label<ui::Id("svA")>,
ui::Label<ui::Id("svB")>
>,
ui::Wrap<ui::Id("wrap"),
ui::Label<ui::Id("wA")>,
ui::Label<ui::Id("wB")>,
ui::Label<ui::Id("wC")>,
ui::Label<ui::Id("wD")>
>,
ui::Grid<ui::Id("grid"),
ui::Label<ui::Id("g00")>,
ui::Label<ui::Id("g01")>,
ui::Label<ui::Id("g02")>,
ui::Label<ui::Id("g10")>,
ui::Label<ui::Id("g11")>,
ui::Label<ui::Id("g12")>
>
>;

	Root root;

PageSplit() {


root.at<ui::Id("intro")>().text = "分格演示：SplitH 权重分割 / SplitV 垂直分割 / Wrap 换行 / Grid 网格";
auto& w_sfA = root.at<ui::Id("splitH"), ui::Id("sfA")>();
w_sfA.text = "fixed 160";
Mk::wg2::sizeOf(w_sfA).fixedW = 160.0f;
auto& w_sfB = root.at<ui::Id("splitH"), ui::Id("sfB")>();
w_sfB.text = "stretch 1";
Mk::wg2::setStretch(w_sfB, 1.0f);
auto& w_sfC = root.at<ui::Id("splitH"), ui::Id("sfC")>();
w_sfC.text = "stretch 2";
Mk::wg2::setStretch(w_sfC, 2.0f);

auto& w_svA = root.at<ui::Id("splitV"), ui::Id("svA")>();
w_svA.text = "fixed 44";
Mk::wg2::sizeOf(w_svA).fixedH = 44.0f;
auto& w_svB = root.at<ui::Id("splitV"), ui::Id("svB")>();
w_svB.text = "VSplit stretch";
Mk::wg2::setStretch(w_svB, 1.0f);

root.at<ui::Id("wrap"), ui::Id("wA")>().text = "换行一";
root.at<ui::Id("wrap"), ui::Id("wB")>().text = "换行二（较长项）";
root.at<ui::Id("wrap"), ui::Id("wC")>().text = "换行三";
root.at<ui::Id("wrap"), ui::Id("wD")>().text = "换行四";

root.at<ui::Id("grid")>().cols = 3;
root.at<ui::Id("grid"), ui::Id("g00")>().text = "格 0-0";
root.at<ui::Id("grid"), ui::Id("g01")>().text = "格 0-1";
root.at<ui::Id("grid"), ui::Id("g02")>().text = "格 0-2";
root.at<ui::Id("grid"), ui::Id("g10")>().text = "格 1-0";
root.at<ui::Id("grid"), ui::Id("g11")>().text = "格 1-1";
root.at<ui::Id("grid"), ui::Id("g12")>().text = "格 1-2";

}

};

} // namespace gallery
