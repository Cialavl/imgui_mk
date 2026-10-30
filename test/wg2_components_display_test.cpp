#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <cstdint>
#include <string>
#include <vector>

// 第一层 · 显示类控件：进度条 / 曲线 / 各类文本 / 图片 / 可选中行

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Component, DisplayWidgetsDraw) {
	Harness h;

	float fraction = 0.42f;
	std::vector<float> history(64, 0.5f);
	bool selected = true;
	std::string pulled = "42";

	h.inFrame([&] {
		ProgressBar<Id("pb")> pb;
		pb.fraction = &fraction;
		pb.overlay = "加载中";
		EXPECT_GT(pb.hint(Size{200.0f, 0.0f}).w, 0.0f);
		layout(pb, Rect{10.0f, 10.0f, 300.0f, 24.0f});

		PlotLines<Id("plot")> plot;
		plot.name = "曲线";
		plot.values = &history;
		layout(plot, Rect{10.0f, 40.0f, 300.0f, 60.0f});

		TextWrapped<Id("tw")> tw;
		tw.text = "自动换行的长文本，用于验证 hint 会按可用宽度换行计算高度。";
		const Size twh = tw.hint(Size{120.0f, 0.0f});
		EXPECT_FLOAT_EQ(twh.w, 120.0f);
		EXPECT_GT(twh.h, 0.0f);
		layout(tw, Rect{10.0f, 110.0f, 200.0f, 60.0f});

		BulletText<Id("bullet")> bullet;
		bullet.text = "圆点项";
		layout(bullet, Rect{10.0f, 175.0f, 200.0f, 24.0f});

		LabelText<Id("lt")> lt;
		lt.name = "帧";
		lt.valueFn = [&] { return pulled; };
		const Size lth = lt.hint(Size{0.0f, 0.0f});
		EXPECT_GT(lth.w, 0.0f);
		layout(lt, Rect{10.0f, 205.0f, 200.0f, 24.0f});

		Selectable<Id("sel")> sel;
		sel.name = "可选中行";
		sel.selected = &selected;
		layout(sel, Rect{10.0f, 235.0f, 200.0f, 24.0f});

		SeparatorText<Id("st")> st;
		st.name = "分组";
		layout(st, Rect{10.0f, 265.0f, 300.0f, 24.0f});

		Image<Id("img")> img;
		img.tex = static_cast<ImTextureID>(static_cast<intptr_t>(1));
		img.size = ImVec2(32.0f, 32.0f);
		EXPECT_FLOAT_EQ(img.hint(Size{0.0f, 0.0f}).w, 32.0f);
		layout(img, Rect{10.0f, 295.0f, 64.0f, 64.0f});

		int buttonClicks = 0;
		ImageButton<Id("imgbtn")> imgbtn;
		imgbtn.tex = static_cast<ImTextureID>(static_cast<intptr_t>(1));
		imgbtn.size = ImVec2(32.0f, 32.0f);
		imgbtn.onClick = [&] { ++buttonClicks; };
		layout(imgbtn, Rect{10.0f, 365.0f, 64.0f, 64.0f});
		EXPECT_EQ(buttonClicks, 0);

		EllipsisLabel<Id("ell")> ell;
		ell.text = "一段很长很长、需要被省略号截断的标题文本";
		layout(ell, Rect{10.0f, 435.0f, 120.0f, 24.0f});
	});
}
