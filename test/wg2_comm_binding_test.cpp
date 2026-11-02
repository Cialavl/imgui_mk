#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <string>

// 第四层 · 控件间共享绑定：同一变量多控件读写、位标志、显隐联动

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Comm, SharedValueIsPulledByLabelTextEveryDraw) {
	Harness h;
	float shared = 0.25f;
	int pulls = 0;

	SliderF<Id("slider")> slider;
	slider.name = "音量";
	slider.value = &shared;
	slider.mn = 0.0f;
	slider.mx = 1.0f;

	LabelText<Id("label")> label;
	label.name = "音量";
	label.valueFn = [&] { ++pulls; return std::to_string(shared); };

	h.inFrame([&] {
		layout(slider, Rect{0.0f, 0.0f, 200.0f, 24.0f});
		layout(label, Rect{0.0f, 30.0f, 200.0f, 24.0f});
		EXPECT_EQ(pulls, 1);                     // 每帧拉取一次
		shared = 0.9f;                           // 模拟别处写入共享状态
	});

	h.inFrame([&] {
		const std::string shown = label.valueFn();
		EXPECT_NE(shown.find("0.9"), std::string::npos);
		EXPECT_EQ(pulls, 2);
	});
}

TEST(Wg2Comm, CheckboxFlagsShareOneBitMask) {
	Harness h;
	int flags = 0b101;

	CheckboxFlags<Id("a")> a; a.name = "A"; a.flags = &flags; a.mask = 1;
	CheckboxFlags<Id("b")> b; b.name = "B"; b.flags = &flags; b.mask = 2;
	CheckboxFlags<Id("c")> c; c.name = "C"; c.flags = &flags; c.mask = 4;

	h.inFrame([&] {
		layout(a, Rect{0.0f, 0.0f, 200.0f, 24.0f});
		layout(b, Rect{0.0f, 30.0f, 200.0f, 24.0f});
		layout(c, Rect{0.0f, 60.0f, 200.0f, 24.0f});
	});
	EXPECT_EQ(flags & 1, 1);
	EXPECT_EQ(flags & 2, 0);
	EXPECT_EQ(flags & 4, 4);
	EXPECT_EQ(flags, 0b101);                    // 纯绘制不改状态
}

TEST(Wg2Comm, WhenFollowsExternalFlagAndReleasesSpace) {
	Harness h;
	bool whenShown = true;
	RectProbe<Id("p")> when;
	setVisibleFn(when, [&] { return whenShown; });
	whenShown = true;
	when.h = 12.0f;

	h.inFrame([&] {
		EXPECT_FLOAT_EQ(when.hint(Size{100.0f, 0.0f}).h, 12.0f);
	});
	whenShown = false;                          // 外部状态变化 → 下一帧不再占位
	h.inFrame([&] {
		layout(when, Rect{0.0f, 0.0f, 100.0f, 40.0f});
	});
	EXPECT_EQ(when.drawCount, 0);
}
