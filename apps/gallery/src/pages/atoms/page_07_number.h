#pragma once
// p7 数值输入：InputInt/Float/Double + vec2（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageNumber {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "数值输入";
	static constexpr const char* desc  = "wg2 InputInt/Float/Double 与 vec2 变体（std::array<T,2>* 绑定）。数值直接写回外部变量。";
	static constexpr std::string_view code = snips::kInputNum;

using Root = VBox<
Id("root"),
InputInt<Id("i")>,
InputFloat<Id("f")>,
InputDouble<Id("d")>,
InputInt2<Id("iv2")>,
InputFloat2<Id("fv2")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int iVal = 42;
	float fVal = 3.14f;
	double dVal = 2.718;
	std::array<int, 2> iv{10, 20};
	std::array<float, 2> fv{1.5f, 2.5f};

PageNumber() {


root.at<Id("i")>().name = "整数";    root.at<Id("i")>().value = &iVal;
root.at<Id("f")>().name = "浮点";    root.at<Id("f")>().value = &fVal;
root.at<Id("d")>().name = "双精度";  root.at<Id("d")>().value = &dVal;
root.at<Id("iv2")>().name = "ivec2"; root.at<Id("iv2")>().value = &iv;
root.at<Id("fv2")>().name = "vec2";  root.at<Id("fv2")>().value = &fv;

}

};

} // namespace gallery
