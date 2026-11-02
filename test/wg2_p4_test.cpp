#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>

#include <memory>
#include <tuple>

using namespace Mk::wg2;

namespace {

// 带渲染开关的探针：shouldRender 由自身条件决定
struct GateProbe {
	static constexpr auto id = Id("gate");
	bool render = true;
	int draws = 0;

	Size hint(const Size&) const { return Size{10.0f, 10.0f}; }
	Policy policy() const { return Policy{}; }
	bool shouldRender() const { return render; }
	void draw(const Rect&) { ++draws; }
};

// 组合控件：纯 struct（无基类、无 hint/policy/draw），只声明 root + overlays()
struct TestComposite {
	static constexpr auto id = Id("comp");
	wg2test::RectProbe<Id("rootProbe")> root;
	GateProbe overlay;
	auto overlays() { return std::tie(overlay); }
};

// 记录绘制时光标位置：验证页签内容画在页签条下方
struct CursorProbe {
	static constexpr auto id = Id("cursorProbe");
	float* y = nullptr;
	Size hint(const Size&) const { return Size{10.0f, 10.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) { if (y) *y = ImGui::GetCursorScreenPos().y; }
};

} // namespace

TEST(Wg2P4, FrameworkDrivesRootAndOverlays) {
	wg2test::Harness h;
	TestComposite c;
	c.root.w = 100.0f;
	c.root.h = 20.0f;

	h.inFrame([&] {
		EXPECT_FLOAT_EQ(hintOf(c, Size{200.0f, 50.0f}).w, 100.0f);   // 只由 root 决定
		EXPECT_FLOAT_EQ(hintOf(c, Size{200.0f, 50.0f}).h, 20.0f);
		layout(c, Rect{0.0f, 0.0f, 200.0f, 50.0f});                  // 框架画 root + overlays
	});

	EXPECT_EQ(c.root.drawCount, 1);
	EXPECT_EQ(c.overlay.draws, 1);
}

TEST(Wg2P4, OverlayVisibilityControlledByEngineState) {
	wg2test::Harness h;
	TestComposite c;

	setVisible(c.overlay, false);
	h.inFrame([&] { layout(c, Rect{0.0f, 0.0f, 200.0f, 50.0f}); });
	EXPECT_EQ(c.overlay.draws, 0);
	EXPECT_EQ(c.root.drawCount, 1);

	setVisible(c.overlay, true);
	h.inFrame([&] { layout(c, Rect{0.0f, 0.0f, 200.0f, 50.0f}); });
	EXPECT_EQ(c.overlay.draws, 1);
}

TEST(Wg2P4, ShouldRenderHookSkipsDrawing) {
	wg2test::Harness h;
	TestComposite c;
	c.overlay.render = false;   // 控件自身条件：不渲染

	h.inFrame([&] { layout(c, Rect{0.0f, 0.0f, 200.0f, 50.0f}); });
	EXPECT_EQ(c.overlay.draws, 0);

	c.overlay.render = true;
	h.inFrame([&] { layout(c, Rect{0.0f, 0.0f, 200.0f, 50.0f}); });
	EXPECT_EQ(c.overlay.draws, 1);
}

TEST(Wg2P4, HiddenRootCollapsesCompositeHint) {
	TestComposite c;
	c.root.w = 100.0f;
	c.root.h = 20.0f;
	EXPECT_FLOAT_EQ(hintOf(c, Size{200.0f, 50.0f}).w, 100.0f);

	setVisible(c.root, false);
	EXPECT_FLOAT_EQ(hintOf(c, Size{200.0f, 50.0f}).w, 0.0f);
	EXPECT_FLOAT_EQ(hintOf(c, Size{200.0f, 50.0f}).h, 0.0f);
}

// 页签内容必须画在页签条下方（曾经用矩形定位，画到了页签栏上重叠）
TEST(Wg2P4, RuntimeTabsContentDrawsBelowTabBar) {
	wg2test::Harness h;
	RuntimeTabs<Id("tabs")> tabs;
	float contentY = -1.0f;
	auto probe = std::make_unique<CursorProbe>();
	probe->y = &contentY;
	tabs.add("t1", AnyWidget::own(std::move(probe)));

	float top = 0.0f;
	h.inFrame([&] {
		top = ImGui::GetCursorScreenPos().y;
		tabs.draw(Rect{0.0f, 0.0f, 400.0f, 200.0f});
	});

	EXPECT_GT(contentY, top);          // 不是画在矩形顶端（页签栏位置）
	EXPECT_GT(contentY - top, 16.0f);  // 至少下移一个页签条高度
}
