#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第一层 · 装饰器：When / ItemWidth / Indent / Offset / Colored / Tooltip

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Component, DecoratorsDraw) {
	Harness h;

	bool show = true;
	RectProbe<Id("wp")> when;              // 探针放闭包外：闭包每帧执行
	when.h = 10.0f;
	when.state.visibleFn = [&] { return show; };
	RectProbe<Id("probe")> probe;
	probe.w = 20.0f;
	probe.h = 10.0f;

	h.inFrame([&] {
		const int before = when.drawCount;
		EXPECT_FLOAT_EQ(when.hint(Size{100.0f, 0.0f}).h, 10.0f);
		layout(when, Rect{10.0f, 10.0f, 100.0f, 30.0f});
		EXPECT_EQ(when.drawCount, before + 1);
		show = false;
		const int mid = when.drawCount;
		layout(when, Rect{10.0f, 10.0f, 100.0f, 30.0f});
		EXPECT_EQ(when.drawCount, mid);

		ItemWidth<Id("iw"), RectProbe<Id("iwp")>> iw;
		iw.width = 120.0f;
		layout(iw, Rect{10.0f, 50.0f, 300.0f, 30.0f});

		Indent<Id("ind"), RectProbe<Id("indp")>> ind;
		ind.amount = 24.0f;
		layout(ind, Rect{10.0f, 90.0f, 300.0f, 30.0f});
		EXPECT_FLOAT_EQ(ind.at<Id("indp")>().last.x, 34.0f);

		Offset<Id("off"), RectProbe<Id("offp")>> off;
		off.dx = 12.0f;
		off.dy = 6.0f;
		layout(off, Rect{10.0f, 130.0f, 300.0f, 30.0f});
		EXPECT_FLOAT_EQ(off.at<Id("offp")>().last.x, 22.0f);
		EXPECT_FLOAT_EQ(off.at<Id("offp")>().last.y, 136.0f);

		Colored<Id("col"), RectProbe<Id("colp")>> col;
		col.color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		layout(col, Rect{10.0f, 170.0f, 300.0f, 30.0f});

		Tooltip<Id("tip"), RectProbe<Id("tipp")>> tip;
		tip.text = "提示";
		layout(tip, Rect{10.0f, 210.0f, 300.0f, 30.0f});
	});
}
