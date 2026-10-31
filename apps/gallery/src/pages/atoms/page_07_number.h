#pragma once
// p7 数值输入：InputInt/Float/Double + vec2（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageNumber {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::InputInt<Mk::wg2::Id("i")>,
Mk::wg2::InputFloat<Mk::wg2::Id("f")>,
Mk::wg2::InputDouble<Mk::wg2::Id("d")>,
Mk::wg2::InputInt2<Mk::wg2::Id("iv2")>,
Mk::wg2::InputFloat2<Mk::wg2::Id("fv2")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int iVal = 42;
	float fVal = 3.14f;
	double dVal = 2.718;
	std::array<int, 2> iv{10, 20};
	std::array<float, 2> fv{1.5f, 2.5f};

PageNumber() {
page.setup("数值输入",
"wg2 InputInt/Float/Double 与 vec2 变体（std::array<T,2>* 绑定）。数值直接写回外部变量。",
snips::kInputNum);

root.at<Mk::wg2::Id("i")>().name = "整数";    root.at<Mk::wg2::Id("i")>().value = &iVal;
root.at<Mk::wg2::Id("f")>().name = "浮点";    root.at<Mk::wg2::Id("f")>().value = &fVal;
root.at<Mk::wg2::Id("d")>().name = "双精度";  root.at<Mk::wg2::Id("d")>().value = &dVal;
root.at<Mk::wg2::Id("iv2")>().name = "ivec2"; root.at<Mk::wg2::Id("iv2")>().value = &iv;
root.at<Mk::wg2::Id("fv2")>().name = "vec2";  root.at<Mk::wg2::Id("fv2")>().value = &fv;

bindPage(page, root);
}

};

} // namespace gallery
