#pragma once
// p7 数值输入：ui::InputInt/Float/Double + vec2（wg2）
#include "../../page.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageNumber {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "数值输入";
	static constexpr const char* desc  = "wg2 InputInt/Float/Double 与 vec2 变体（std::array<T,2>* 绑定）。数值直接写回外部变量。";
	static constexpr std::string_view code = snips::kInputNum;

using Root = ui::VBox<
ui::Id("root"),
ui::InputInt<ui::Id("i")>,
ui::InputFloat<ui::Id("f")>,
ui::InputDouble<ui::Id("d")>,
ui::InputInt2<ui::Id("iv2")>,
ui::InputFloat2<ui::Id("fv2")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	int iVal = 42;
	float fVal = 3.14f;
	double dVal = 2.718;
	std::array<int, 2> iv{10, 20};
	std::array<float, 2> fv{1.5f, 2.5f};

PageNumber() {


root.at<ui::Id("i")>().name = "整数";    root.at<ui::Id("i")>().value = &iVal;
root.at<ui::Id("f")>().name = "浮点";    root.at<ui::Id("f")>().value = &fVal;
root.at<ui::Id("d")>().name = "双精度";  root.at<ui::Id("d")>().value = &dVal;
root.at<ui::Id("iv2")>().name = "ivec2"; root.at<ui::Id("iv2")>().value = &iv;
root.at<ui::Id("fv2")>().name = "vec2";  root.at<ui::Id("fv2")>().value = &fv;

}

};

} // namespace gallery
