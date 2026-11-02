#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第一层 · 基础原子控件：hint、绘制、可见/禁用门控、尺寸覆盖

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;
using wg2test::LambdaDrawer;

TEST(Wg2Component, BasicWidgetsHintAndDraw) {
	Harness h;
	h.inFrame([&] {
		Label<Id("label")> label;
		label.text = "Hello wg2";
		const Size lh = label.hint(Size{200.0f, 0.0f});
		EXPECT_GT(lh.w, 0.0f);
		EXPECT_GT(lh.h, 0.0f);
		EXPECT_FLOAT_EQ(lh.h, ImGui::GetTextLineHeight());
		layout(label, Rect{10.0f, 10.0f, 200.0f, 24.0f});

		int clicks = 0;
		Button<Id("button")> button;
		button.label = "OK";
		button.onClick = [&] { ++clicks; };
		const Size bh = button.hint(Size{0.0f, 0.0f});
		EXPECT_GT(bh.w, ImGui::CalcTextSize("OK").x);   // 按钮 = 文本 + 内边距
		EXPECT_FLOAT_EQ(bh.h, ImGui::GetFrameHeight());
		layout(button, Rect{10.0f, 40.0f, 120.0f, 30.0f});
		EXPECT_EQ(clicks, 0);    // 没有鼠标输入时不会被点击

		TextColored<Id("tc")> tc;
		tc.text = "colored";
		tc.color = ImVec4(0.2f, 0.9f, 0.4f, 1.0f);
		layout(tc, Rect{10.0f, 80.0f, 200.0f, 24.0f});

		Separator<Id("sep")> sep;
		EXPECT_GT(sep.hint(Size{100.0f, 0.0f}).h, 0.0f);
		layout(sep, Rect{10.0f, 110.0f, 300.0f, 20.0f});

		int fnCalls = 0;
		LambdaDrawer drawer{&fnCalls};
		FnWidget<Id("fn"), LambdaDrawer> fn(drawer);
		const Size fh = fn.hint(Size{300.0f, 0.0f});
		EXPECT_FLOAT_EQ(fh.w, 300.0f);
		EXPECT_FLOAT_EQ(fh.h, ImGui::GetFrameHeight());   // 裸绘制默认占一行高
		layout(fn, Rect{10.0f, 140.0f, 300.0f, fh.h});
		EXPECT_EQ(fnCalls, 1);
	});
}

TEST(Wg2Component, VisibleAndEnabledGateDrawing) {
	Harness h;

	RectProbe<Id("p")> probe;
	probe.w = 10.0f;
	probe.h = 10.0f;

	setVisible(probe, false);
	h.inFrame([&] { drawWidget(probe, Rect{0.0f, 0.0f, 50.0f, 50.0f}); });
	EXPECT_EQ(probe.drawCount, 0);

	setVisible(probe, true);
	setEnabled(probe, false);
	h.inFrame([&] { drawWidget(probe, Rect{0.0f, 0.0f, 50.0f, 50.0f}); });
	EXPECT_EQ(probe.drawCount, 1);   // 禁用仍绘制（BeginDisabled 包裹），只是交互被禁

	VBox<Id("v"), RectProbe<Id("a")>, RectProbe<Id("b")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 0.0f;
	box.at<Id("a")>().w = 10.0f;
	box.at<Id("a")>().h = 5.0f;
	box.at<Id("b")>().w = 10.0f;
	box.at<Id("b")>().h = 7.0f;
	setVisible(box.at<Id("a")>(), false);
	h.inFrame([&] {
		EXPECT_FLOAT_EQ(box.hint(Size{100.0f, 0.0f}).h, 7.0f);   // 不可见子项不占高
		layout(box, Rect{0.0f, 0.0f, 100.0f, 40.0f});
	});
	EXPECT_EQ(box.at<Id("a")>().drawCount, 0);
	EXPECT_EQ(box.at<Id("b")>().drawCount, 1);
}

TEST(Wg2Component, SizeHelperSettersApplyToLayout) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("fixed")>, RectProbe<Id("pref")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 0.0f;
	setFixedSize(box.at<Id("fixed")>(), 40.0f, 20.0f);   // 高度固定 20
	setPrefSize(box.at<Id("pref")>(), 0.0f, 15.0f);      // 期望高度 15
	box.at<Id("pref")>().h = 5.0f;

	HBox<Id("h"), RectProbe<Id("wide")>> row;
	row.marginX = 0.0f;
	row.marginY = 0.0f;
	row.spacing = 0.0f;
	setFixedSize(row.at<Id("wide")>(), 30.0f, 0.0f);     // 宽度固定 30

	h.inFrame([&] {
		// 高度正好等于 hint 合计（20 + 15），避免富余高度被 canGrow 均分
		layout(box, Rect{0.0f, 0.0f, 100.0f, 35.0f});
		EXPECT_FLOAT_EQ(box.at<Id("fixed")>().last.h, 20.0f);
		EXPECT_FLOAT_EQ(box.at<Id("pref")>().last.h, 15.0f);
		EXPECT_FLOAT_EQ(box.at<Id("pref")>().last.y, 20.0f);

		layout(row, Rect{0.0f, 80.0f, 100.0f, 20.0f});
		EXPECT_FLOAT_EQ(row.at<Id("wide")>().last.w, 30.0f);
	});
}
