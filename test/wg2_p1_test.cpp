#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/window.h>

#include <cstdio>
#include <string>
#include <vector>

using namespace Mk::wg2;

// —— 多选模型 ——

TEST(Wg2P1, SelectionModelClickSemantics) {
	SelectionModel m;
	m.click(2, false, false);            // 单选
	EXPECT_EQ(m.items, (std::vector<int>{2}));
	m.click(5, true, false);             // Ctrl 加选
	EXPECT_EQ(m.items, (std::vector<int>{2, 5}));
	m.click(7, false, true);             // Shift：从锚点 5 连选到 7
	EXPECT_EQ(m.items, (std::vector<int>{5, 6, 7}));
	m.click(1, false, false);            // 普通点击清空重选
	EXPECT_EQ(m.items, (std::vector<int>{1}));
	m.click(1, true, false);             // Ctrl 取消
	EXPECT_TRUE(m.empty());
}

// —— 可搜索下拉 ——

TEST(Wg2P1, SearchFilterIsCaseInsensitiveSubstring) {
	const std::vector<std::string> items{"Alpha", "beta", "GAMMA", "alphabet"};
	EXPECT_EQ(detail::filterItems(items, ""), (std::vector<int>{0, 1, 2, 3}));
	EXPECT_EQ(detail::filterItems(items, "alpha"), (std::vector<int>{0, 3}));
	EXPECT_EQ(detail::filterItems(items, "BET"), (std::vector<int>{1, 3}));   // beta / alphabet
	EXPECT_TRUE(detail::filterItems(items, "zzz").empty());
}

TEST(Wg2P1, SearchComboDrawsInFrame) {
	wg2test::Harness h;
	SearchCombo<Id("sc")> combo;
	combo.name = "search";
	combo.items = {"a", "b"};
	int sel = 0;
	combo.selected = &sel;
	h.inFrame([&] {
		EXPECT_FLOAT_EQ(combo.hint(Size{240.0f, 0.0f}).w, 240.0f);
		combo.draw(Rect{0.0f, 0.0f, 240.0f, 24.0f});
	});
}

// —— 图标 ——

TEST(Wg2P1, IconGlyphEncodesUtf8) {
	EXPECT_EQ(iconGlyph(0x41), "A");
	EXPECT_EQ(iconGlyph(0xE001), std::string("\xEE\x80\x81"));
	EXPECT_EQ(iconGlyph(0x1F600), std::string("\xF0\x9F\x98\x80"));
}

TEST(Wg2P1, IconTextDrawsInFrame) {
	wg2test::Harness h;
	IconText<Id("ic")> icon;
	icon.glyph = 0xF001;
	h.layoutFrame(icon, Rect{0.0f, 0.0f, 20.0f, 20.0f});
}

// —— i18n ——

TEST(Wg2P1, TranslationFallsBackToKey) {
	clearTranslations();
	EXPECT_STREQ(tr("hello"), "hello");
	setTranslation("hello", "你好");
	EXPECT_STREQ(tr("hello"), "你好");
	setTranslations({{"a", "A"}, {"b", "B"}});
	EXPECT_STREQ(tr("b"), "B");
	clearTranslations();
	EXPECT_STREQ(tr("a"), "a");
	EXPECT_STREQ(tr(nullptr), "");
}

// —— 动效 ——

TEST(Wg2P1, EasingAndMotionConverge) {
	EXPECT_FLOAT_EQ(easeOutCubic(0.0f), 0.0f);
	EXPECT_FLOAT_EQ(easeOutCubic(1.0f), 1.0f);
	EXPECT_NEAR(easeOutCubic(0.5f), 0.875f, 1e-4f);

	Motion m;
	m.snap(0.0f);
	for (int i = 0; i < 60; ++i) m.toward(10.0f, 8.0f, 1.0f / 60.0f);
	EXPECT_GT(m.value, 9.0f);
}

TEST(Wg2P1, TransitionRunsToCompletion) {
	Transition t;
	t.start(0.0f, 1.0f, 0.2f);
	t.update(0.1f);
	EXPECT_TRUE(t.running);
	t.update(0.2f);
	EXPECT_FALSE(t.running);
	EXPECT_FLOAT_EQ(t.value(), 1.0f);
}

// —— 图片等比 ——

TEST(Wg2P1, FitRectKeepsAspectAndCenters) {
	const auto wide = fitRect(200.0f, 100.0f, 100.0f, 100.0f);
	EXPECT_FLOAT_EQ(wide.w, 100.0f);
	EXPECT_FLOAT_EQ(wide.h, 50.0f);
	EXPECT_FLOAT_EQ(wide.y, 25.0f);

	const auto tall = fitRect(100.0f, 200.0f, 100.0f, 100.0f);
	EXPECT_FLOAT_EQ(tall.h, 100.0f);
	EXPECT_FLOAT_EQ(tall.w, 50.0f);
	EXPECT_FLOAT_EQ(tall.x, 25.0f);
}

// —— 窗口状态持久化 ——

TEST(Wg2P1, WindowLayoutRoundTrip) {
	const std::string path = "wg2_layout_test.txt";
	Mk::WindowLayout in{10, 20, 800, 600, true};
	ASSERT_TRUE(Mk::saveWindowLayout(path, in));

	Mk::WindowLayout out;
	ASSERT_TRUE(Mk::loadWindowLayout(path, out));
	EXPECT_EQ(out.x, 10);
	EXPECT_EQ(out.y, 20);
	EXPECT_EQ(out.width, 800);
	EXPECT_EQ(out.height, 600);
	EXPECT_TRUE(out.maximized);

	std::remove(path.c_str());

	Mk::WindowLayout missing;
	EXPECT_FALSE(Mk::loadWindowLayout("wg2_no_such_layout.txt", missing));
}

// —— 表格多选集成 ——

TEST(Wg2P1, DataTableWithSelectionDrawsInFrame) {
	wg2test::Harness h;
	DataTable<Id("dt")> t;
	t.headers = {"x"};
	t.rowCount = 50;
	t.cell = [](int r, int) { return std::to_string(r); };
	SelectionModel sel;
	t.selection = &sel;
	h.layoutFrame(t, Rect{0.0f, 0.0f, 300.0f, 200.0f});
	EXPECT_TRUE(t.selection == &sel);
}
