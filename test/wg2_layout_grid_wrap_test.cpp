#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第二层 · Grid / Wrap / Overlay：网格、换行、叠放与 hint 聚合

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Layout, GridPlacesCellsInColumns) {
	Harness h;
	Grid<Id("grid"), RectProbe<Id("g0")>, RectProbe<Id("g1")>, RectProbe<Id("g2")>, RectProbe<Id("g3")>> grid;
	grid.cols = 2;
	grid.marginX = 0.0f;
	grid.marginY = 0.0f;
	grid.spacing = 10.0f;
	grid.at<Id("g0")>().w = 10.0f; grid.at<Id("g0")>().h = 10.0f;
	grid.at<Id("g1")>().w = 10.0f; grid.at<Id("g1")>().h = 20.0f;
	grid.at<Id("g2")>().w = 10.0f; grid.at<Id("g2")>().h = 10.0f;
	grid.at<Id("g3")>().w = 10.0f; grid.at<Id("g3")>().h = 10.0f;

	h.inFrame([&] {
		layout(grid, Rect{0.0f, 0.0f, 110.0f, 100.0f});
		EXPECT_FLOAT_EQ(grid.at<Id("g0")>().last.x, 0.0f);
		EXPECT_FLOAT_EQ(grid.at<Id("g1")>().last.x, 60.0f);   // 第二列 = 50 + 10
		EXPECT_FLOAT_EQ(grid.at<Id("g2")>().last.y, 30.0f);   // 第二行 = 行高 20 + 间距 10
		EXPECT_FLOAT_EQ(grid.at<Id("g0")>().last.w, 50.0f);
	});
}

TEST(Wg2Layout, WrapBreaksToNextRowWhenNarrow) {
	Harness h;
	Wrap<Id("wrap"), RectProbe<Id("w0")>, RectProbe<Id("w1")>, RectProbe<Id("w2")>> wrap;
	wrap.marginX = 0.0f;
	wrap.marginY = 0.0f;
	wrap.spacing = 10.0f;
	wrap.at<Id("w0")>().w = 30.0f; wrap.at<Id("w0")>().h = 10.0f;
	wrap.at<Id("w1")>().w = 30.0f; wrap.at<Id("w1")>().h = 10.0f;
	wrap.at<Id("w2")>().w = 30.0f; wrap.at<Id("w2")>().h = 10.0f;

	h.inFrame([&] {
		layout(wrap, Rect{0.0f, 0.0f, 80.0f, 100.0f});   // 一行只放得下 2 个
		EXPECT_FLOAT_EQ(wrap.at<Id("w0")>().last.y, 0.0f);
		EXPECT_FLOAT_EQ(wrap.at<Id("w1")>().last.y, 0.0f);
		EXPECT_FLOAT_EQ(wrap.at<Id("w2")>().last.y, 20.0f);
	});
}

TEST(Wg2Layout, OverlayStacksAllChildrenOnSameRect) {
	Harness h;
	Overlay<Id("ov"), RectProbe<Id("a")>, RectProbe<Id("b")>> ov;
	ov.marginX = 0.0f;
	ov.marginY = 0.0f;
	h.inFrame([&] {
		layout(ov, Rect{5.0f, 6.0f, 100.0f, 40.0f});
		EXPECT_FLOAT_EQ(ov.at<Id("a")>().last.x, 5.0f);
		EXPECT_FLOAT_EQ(ov.at<Id("b")>().last.x, 5.0f);
		EXPECT_FLOAT_EQ(ov.at<Id("a")>().last.w, 100.0f);
		EXPECT_FLOAT_EQ(ov.at<Id("b")>().last.h, 40.0f);
	});
}

TEST(Wg2Layout, GridWrapOverlayHintAggregate) {
	Harness h;

	Grid<Id("grid"), RectProbe<Id("g0")>, RectProbe<Id("g1")>, RectProbe<Id("g2")>, RectProbe<Id("g3")>> grid;
	grid.cols = 2;
	grid.spacing = 8.0f;
	grid.marginX = 0.0f;
	grid.marginY = 0.0f;
	grid.at<Id("g0")>().w = 10.0f; grid.at<Id("g0")>().h = 20.0f;
	grid.at<Id("g1")>().w = 10.0f; grid.at<Id("g1")>().h = 20.0f;
	grid.at<Id("g2")>().w = 10.0f; grid.at<Id("g2")>().h = 20.0f;
	grid.at<Id("g3")>().w = 10.0f; grid.at<Id("g3")>().h = 20.0f;

	Wrap<Id("wrap"), RectProbe<Id("w0")>, RectProbe<Id("w1")>> wrap;
	wrap.spacing = 8.0f;
	wrap.marginX = 0.0f;
	wrap.marginY = 0.0f;
	wrap.at<Id("w0")>().w = 10.0f; wrap.at<Id("w0")>().h = 20.0f;
	wrap.at<Id("w1")>().w = 10.0f; wrap.at<Id("w1")>().h = 20.0f;

	Overlay<Id("ov"), RectProbe<Id("o0")>, RectProbe<Id("o1")>> ov;
	ov.marginX = 0.0f;
	ov.marginY = 0.0f;
	ov.at<Id("o0")>().w = 10.0f; ov.at<Id("o0")>().h = 20.0f;
	ov.at<Id("o1")>().w = 30.0f; ov.at<Id("o1")>().h = 15.0f;

	h.inFrame([&] {
		// 2 列 × 2 行：宽 10 + 8 + 10，高 20 + 8 + 20
		EXPECT_FLOAT_EQ(grid.hint(Size{100.0f, 0.0f}).w, 28.0f);
		EXPECT_FLOAT_EQ(grid.hint(Size{100.0f, 0.0f}).h, 48.0f);

		// 宽 28 时同一行、宽 20 时换行
		EXPECT_FLOAT_EQ(wrap.hint(Size{28.0f, 0.0f}).w, 28.0f);
		EXPECT_FLOAT_EQ(wrap.hint(Size{28.0f, 0.0f}).h, 20.0f);
		EXPECT_FLOAT_EQ(wrap.hint(Size{20.0f, 0.0f}).w, 10.0f);
		EXPECT_FLOAT_EQ(wrap.hint(Size{20.0f, 0.0f}).h, 48.0f);

		// 叠放取子项最大值
		EXPECT_FLOAT_EQ(ov.hint(Size{100.0f, 0.0f}).w, 30.0f);
		EXPECT_FLOAT_EQ(ov.hint(Size{100.0f, 0.0f}).h, 20.0f);
	});
}
