#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <memory>

// 第三层 · 帧循环：跨帧稳定、运行中换肤、尺寸变化、渲染期增删延迟

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Loop, FrameLoopIsStableAcrossFrames) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("a")>, RectProbe<Id("b")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 4.0f;
	box.at<Id("a")>().h = 10.0f;
	box.at<Id("b")>().h = 10.0f;

	for (int i = 0; i < 20; ++i) {
		h.inFrame([&] { layout(box, Rect{0.0f, 0.0f, 100.0f, 100.0f}); });
	}
	EXPECT_EQ(h.frameCount(), 20);
	EXPECT_EQ(box.at<Id("a")>().drawCount, 20);
	EXPECT_EQ(box.at<Id("b")>().drawCount, 20);
}

TEST(Wg2Loop, ThemeSwitchMidLoopAppliesNextFrame) {
	Harness h;
	RectProbe<Id("p")> probe;
	probe.w = 10.0f;
	probe.h = 10.0f;

	h.inFrame([&] { layout(probe, Rect{0.0f, 0.0f, 50.0f, 50.0f}); });
	EXPECT_EQ(probe.drawCount, 1);

	const Theme light = Theme::light();
	applyTheme(ImGui::GetCurrentContext(), light);
	h.inFrame([&] { layout(probe, Rect{0.0f, 0.0f, 50.0f, 50.0f}); });
	EXPECT_EQ(probe.drawCount, 2);
	EXPECT_NEAR(ImGui::GetStyle().Colors[ImGuiCol_WindowBg].x, light.windowBg.x, 1e-4f);

	applyTheme(ImGui::GetCurrentContext(), Theme::dark());
}

TEST(Wg2Loop, ResizeTakesEffectNextFrame) {
	Harness h;
	VBox<Id("v"), RectProbe<Id("a")>> box;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.spacing = 0.0f;
	box.at<Id("a")>().h = 10.0f;

	h.inFrame([&] { layout(box, Rect{0.0f, 0.0f, 100.0f, 40.0f}); });
	EXPECT_FLOAT_EQ(box.at<Id("a")>().last.w, 100.0f);

	h.inFrame([&] { layout(box, Rect{0.0f, 0.0f, 240.0f, 40.0f}); });
	EXPECT_FLOAT_EQ(box.at<Id("a")>().last.w, 240.0f);
}

TEST(Wg2Loop, RuntimeListAddDuringRenderIsDeferred) {
	Harness h;
	RuntimeList<Id("list")> list;
	list.spacing = 0.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	int adds = 0;
	auto drawer = [&] {
		if (adds > 0) return;
		auto item = std::make_unique<RectProbe<Id("item")>>();
		item->h = 8.0f;
		AnyWidget w = AnyWidget::own(std::move(item));
		w.idname = "item";
		list.add(std::move(w));
		++adds;
	};

	AnyWidget seed = AnyWidget::own(
		std::make_unique<FnWidget<Id("fn"), decltype(drawer)>>(drawer));
	seed.idname = "fn";
	list.add(std::move(seed));
	ASSERT_EQ(list.size(), 1u);

	h.inFrame([&] { layout(list, Rect{0.0f, 0.0f, 100.0f, 80.0f}); });
	EXPECT_EQ(adds, 1);
	EXPECT_EQ(list.size(), 2u);                 // pending 也计入 size
	EXPECT_EQ(list.findById("item"), nullptr);  // 尚未 flush 到正式槽位

	h.inFrame([&] { layout(list, Rect{0.0f, 0.0f, 100.0f, 80.0f}); });
	EXPECT_NE(list.findById("item"), nullptr);  // 下一帧已 flush
}
