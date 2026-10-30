#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <array>

// 第一层 · 颜色控件与位标志

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Component, ColorWidgetsDraw) {
	Harness h;

	std::array<float, 3> rgb{1.0f, 0.5f, 0.25f};
	std::array<float, 4> rgba{1.0f, 0.5f, 0.25f, 1.0f};
	int flags = 1;

	h.inFrame([&] {
		ColorEdit3<Id("ce3")> ce3; ce3.name = "RGB";  ce3.color = &rgb;
		ColorEdit4<Id("ce4")> ce4; ce4.name = "RGBA"; ce4.color = &rgba;
		ColorPicker3<Id("cp3")> cp3; cp3.name = "拾取"; cp3.color = &rgb;
		ColorPicker4<Id("cp4")> cp4; cp4.name = "拾取A"; cp4.color = &rgba;
		CheckboxFlags<Id("cf")> cf;  cf.name = "位A"; cf.flags = &flags; cf.mask = 1;

		EXPECT_GT(ce3.hint(Size{300.0f, 0.0f}).w, 0.0f);
		EXPECT_FLOAT_EQ(cp3.hint(Size{0.0f, 0.0f}).h, 320.0f);
		EXPECT_GT(cf.hint(Size{0.0f, 0.0f}).w, 0.0f);

		layout(ce3, Rect{10.0f, 10.0f, 300.0f, 24.0f});
		layout(ce4, Rect{10.0f, 40.0f, 300.0f, 24.0f});
		layout(cf, Rect{10.0f, 70.0f, 300.0f, 24.0f});
	});
}
