#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第二层 · HBox：列排布不溢出、收缩、hint 聚合

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Layout, HBoxPlacesChildrenAndNeverOverflowsRightEdge) {
	Harness h;
	HBox<Id("row"), RectProbe<Id("a")>, RectProbe<Id("b")>, RectProbe<Id("c")>> row;
	row.marginX = 0.0f;
	row.marginY = 0.0f;
	row.spacing = 10.0f;
	row.at<Id("a")>().w = 20.0f;
	row.at<Id("b")>().w = 30.0f;
	row.at<Id("c")>().w = 40.0f;

	h.inFrame([&] {
		// 宽度正好等于 hint 合计：20 + 30 + 40 + 2*10 = 110（多给会被 canGrow 均分，断言就不精确了）
		layout(row, Rect{0.0f, 0.0f, 110.0f, 30.0f});
		EXPECT_FLOAT_EQ(row.at<Id("a")>().last.x, 0.0f);
		EXPECT_FLOAT_EQ(row.at<Id("b")>().last.x, 30.0f);
		EXPECT_FLOAT_EQ(row.at<Id("c")>().last.x, 70.0f);
		// 回归：末列右缘 == 矩形右缘（早前会多推 (n-1)*spacing 而溢出）
		EXPECT_FLOAT_EQ(row.at<Id("c")>().last.x + row.at<Id("c")>().last.w, 110.0f);
	});
}

TEST(Wg2Layout, HBoxShrinksWhenHintsExceedRoom) {
	Harness h;
	HBox<Id("row"), RectProbe<Id("a")>, RectProbe<Id("b")>> row;
	row.marginX = 0.0f;
	row.marginY = 0.0f;
	row.spacing = 0.0f;
	row.at<Id("a")>().w = 100.0f;
	row.at<Id("b")>().w = 100.0f;

	h.inFrame([&] {
		layout(row, Rect{0.0f, 0.0f, 100.0f, 30.0f});
		EXPECT_NEAR(row.at<Id("a")>().last.w, 50.0f, 0.5f);
		EXPECT_NEAR(row.at<Id("b")>().last.w, 50.0f, 0.5f);
		EXPECT_FLOAT_EQ(row.at<Id("b")>().last.x + row.at<Id("b")>().last.w, 100.0f);
	});
}

TEST(Wg2Layout, HBoxHintAggregatesVisibleChildren) {
	HBox<Id("row"), RectProbe<Id("a")>, RectProbe<Id("b")>> row;
	row.spacing = 8.0f;
	row.marginX = 0.0f;
	row.marginY = 0.0f;
	row.at<Id("a")>().w = 10.0f;
	row.at<Id("a")>().h = 20.0f;
	row.at<Id("b")>().w = 30.0f;
	row.at<Id("b")>().h = 40.0f;

	Harness h;
	h.inFrame([&] {
		const Size s = row.hint(Size{100.0f, 100.0f});
		EXPECT_FLOAT_EQ(s.w, 48.0f);   // 10 + 8 + 30
		EXPECT_FLOAT_EQ(s.h, 40.0f);
	});
}
