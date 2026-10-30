#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第三层 · 输入事件派发：鼠标点击、勾选切换、键盘轮询

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Loop, ButtonClickFiresOnceOnRelease) {
	Harness h;
	int clicks = 0;
	Button<Id("btn")> btn;
	btn.label = "OK";
	btn.onClick = [&] { ++clicks; };
	const Rect r{20.0f, 20.0f, 160.0f, 30.0f};
	auto frame = [&] { h.inFrame([&] { layout(btn, r); }); };

	frame();                                     // 建立 item 状态
	h.mouseTo(r.x + 15.0f, r.y + 10.0f);
	frame();                                     // 悬停
	h.mouseDown();
	frame();                                     // 按下
	h.mouseUp();
	frame();                                     // 松开 → 触发
	EXPECT_EQ(clicks, 1);
	frame();
	EXPECT_EQ(clicks, 1);                        // 不会重复触发
}

TEST(Wg2Loop, CheckboxToggleChangesBoundStateOncePerClick) {
	Harness h;
	bool checked = false;
	CheckBox<Id("check")> check;
	check.name = "Mute";
	check.checked = &checked;
	const Rect r{10.0f, 10.0f, 160.0f, 24.0f};
	auto frame = [&] { h.inFrame([&] { layout(check, r); }); };

	frame();
	h.mouseTo(r.x + 8.0f, r.y + 8.0f);
	frame();
	h.mouseDown();
	frame();
	h.mouseUp();
	frame();
	EXPECT_TRUE(checked);
}

TEST(Wg2Loop, KeyPollingMatchesEngineShortcutContract) {
	Harness h;

	h.keyDown(ImGuiKey_PageDown);
	h.beginFrame();
	EXPECT_TRUE(ImGui::IsKeyPressed(ImGuiKey_PageDown, false));    // 按下的那一帧为真
	h.endFrame();

	h.beginFrame();
	EXPECT_FALSE(ImGui::IsKeyPressed(ImGuiKey_PageDown, false));   // 按住不重复触发
	h.endFrame();

	h.keyUp(ImGuiKey_PageDown);
	h.beginFrame();
	EXPECT_FALSE(ImGui::IsKeyPressed(ImGuiKey_PageDown, false));
	h.endFrame();
}
