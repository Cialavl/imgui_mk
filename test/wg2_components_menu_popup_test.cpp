#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第一层 · 菜单与弹窗

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Component, MenuBarMenuAndMenuItemDraw) {
	Harness h;
	int clicks = 0;
	MenuBar<Id("bar"),
		Menu<Id("file"),
			MenuItem<Id("open")>,
			MenuItem<Id("quit")>>>
		bar;
	bar.at<Id("file")>().name = "File";
	bar.at<Id("file"), Id("open")>().name = "Open";
	bar.at<Id("file"), Id("open")>().shortcut = "Ctrl+O";
	bar.at<Id("file"), Id("open")>().onClick = [&] { ++clicks; };
	bar.at<Id("file"), Id("quit")>().name = "Quit";

	h.inFrame([&] {
		EXPECT_GT(bar.hint(Size{0.0f, 0.0f}).h, 0.0f);
		layout(bar, Rect{0.0f, 0.0f, 300.0f, 30.0f});   // 菜单未展开：只画菜单栏
	});
	EXPECT_EQ(clicks, 0);
}

TEST(Wg2Component, PopupAndModalOpenThroughStateBinding) {
	Harness h;
	bool open = false;
	Popup<Id("pop"), RectProbe<Id("pl")>> pop;
	pop.idname = "wg2-test-popup";
	pop.open = &open;

	// 关闭状态：BeginPopup 返回 false，子项不绘制
	h.inFrame([&] { layout(pop, Rect{0.0f, 0.0f, 100.0f, 40.0f}); });
	EXPECT_EQ(pop.at<Id("pl")>().drawCount, 0);

	open = true;   // 外部状态置位 → 本帧 OpenPopup
	h.inFrame([&] { layout(pop, Rect{0.0f, 0.0f, 100.0f, 40.0f}); });
	EXPECT_EQ(pop.at<Id("pl")>().drawCount, 1);
	EXPECT_FALSE(open);   // 一次性标志已被消费

	bool modalOpen = false;
	PopupModal<Id("modal"), RectProbe<Id("ml")>> modal;
	modal.idname = "wg2-test-modal";
	modal.open = &modalOpen;
	modalOpen = true;
	h.inFrame([&] { layout(modal, Rect{0.0f, 0.0f, 100.0f, 40.0f}); });
	EXPECT_EQ(modal.at<Id("ml")>().drawCount, 1);
}
