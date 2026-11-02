#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第二层 · VBox：堆叠、外边距、固定/伸展/裁剪、hint 聚合

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Layout, VBoxStacksChildrenAndFillsRectExactly) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("a")>, RectProbe<Id("b")>, RectProbe<Id("c")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 10.0f;
	box.at<Id("a")>().w = 20.0f; box.at<Id("a")>().h = 10.0f;
	box.at<Id("b")>().w = 30.0f; box.at<Id("b")>().h = 15.0f;
	box.at<Id("c")>().w = 40.0f; box.at<Id("c")>().h = 25.0f;

	h.inFrame([&] {
		// 高度恰好等于 hint：10 + 15 + 25 + 2*10 = 70
		layout(box, Rect{0.0f, 0.0f, 100.0f, 70.0f});
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.y, 0.0f);
		EXPECT_FLOAT_EQ(box.at<Id("b")>().last.y, 20.0f);
		EXPECT_FLOAT_EQ(box.at<Id("c")>().last.y, 45.0f);
		// 回归：间距只计一次，末子项正好落在矩形底边（早前会多推 (n-1)*spacing）
		EXPECT_FLOAT_EQ(box.at<Id("c")>().last.y + box.at<Id("c")>().last.h, 70.0f);
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.w, 100.0f);
		EXPECT_FLOAT_EQ(box.at<Id("b")>().last.w, 100.0f);
	});
}

TEST(Wg2Layout, VBoxAppliesMarginsAndSpreadsExtraHeight) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("a")>, RectProbe<Id("b")>> box;
	box.marginX = 5.0f;
	box.marginY = 3.0f;
	box.spacing = 0.0f;
	box.at<Id("a")>().h = 10.0f;
	box.at<Id("b")>().h = 10.0f;

	h.inFrame([&] {
		layout(box, Rect{0.0f, 0.0f, 100.0f, 50.0f});
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.x, 5.0f);
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.w, 90.0f);
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.y, 3.0f);
		// 内高 44、hint 合计 20 → 富余 24 由两个 Preferred 子项均分（各 22 高）
		EXPECT_FLOAT_EQ(box.at<Id("a")>().last.h, 22.0f);
		EXPECT_FLOAT_EQ(box.at<Id("b")>().last.y, 25.0f);
		EXPECT_FLOAT_EQ(box.at<Id("b")>().last.y + box.at<Id("b")>().last.h, 47.0f);
	});
}

TEST(Wg2Layout, VBoxHonoursFixedAndStretch) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("fixed")>, RectProbe<Id("flex")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 0.0f;
	box.at<Id("fixed")>().h = 10.0f;
	sizeOf(box.at<Id("fixed")>()).fixedH = 30.0f;     // 固定高度优先于 hint
	box.at<Id("flex")>().h = 10.0f;
	setStretch(box.at<Id("flex")>(), 1.0f);            // 吃掉全部剩余空间

	h.inFrame([&] {
		layout(box, Rect{0.0f, 0.0f, 100.0f, 100.0f});
		EXPECT_FLOAT_EQ(box.at<Id("fixed")>().last.h, 30.0f);
		EXPECT_FLOAT_EQ(box.at<Id("flex")>().last.y, 30.0f);
		EXPECT_FLOAT_EQ(box.at<Id("flex")>().last.h, 70.0f);
	});
}

TEST(Wg2Layout, VBoxClampsHeightByMaxSize) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("clamped")>, RectProbe<Id("after")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 0.0f;
	box.at<Id("clamped")>().h = 30.0f;
	sizeOf(box.at<Id("clamped")>()).maxH = 10.0f;      // 上限裁剪
	box.at<Id("after")>().h = 10.0f;

	h.inFrame([&] {
		layout(box, Rect{0.0f, 0.0f, 100.0f, 20.0f});
		EXPECT_FLOAT_EQ(box.at<Id("clamped")>().last.h, 10.0f);
		EXPECT_FLOAT_EQ(box.at<Id("after")>().last.y, 10.0f);
	});
}

TEST(Wg2Layout, VBoxHintAggregatesVisibleChildren) {
	VBox<Id("box"), RectProbe<Id("a")>, RectProbe<Id("b")>> box;
	box.spacing = 8.0f;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.at<Id("a")>().w = 10.0f;
	box.at<Id("a")>().h = 20.0f;
	box.at<Id("b")>().w = 30.0f;
	box.at<Id("b")>().h = 40.0f;

	Harness h;
	h.inFrame([&] {
		const Size s = box.hint(Size{100.0f, 100.0f});
		EXPECT_FLOAT_EQ(s.w, 30.0f);
		EXPECT_FLOAT_EQ(s.h, 68.0f);   // 20 + 8 + 40

		setVisible(box.at<Id("a")>(), false);
		const Size s2 = box.hint(Size{100.0f, 100.0f});
		EXPECT_FLOAT_EQ(s2.w, 30.0f);
		EXPECT_FLOAT_EQ(s2.h, 40.0f);  // 不可见子项不占空间
	});
}
