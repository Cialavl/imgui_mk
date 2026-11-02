#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>

#include <string>

using namespace Mk::wg2;

// —— 主题密度 / 语义 token ——

TEST(Wg2P2, DensityScalesSpacing) {
	wg2test::Harness h;
	ImGuiContext* ctx = ImGui::GetCurrentContext();

	applyTheme(ctx, Theme::dark().withDensity(1.0f));
	const ImVec2 base = ImGui::GetStyle().FramePadding;

	applyTheme(ctx, Theme::dark().withDensity(0.5f));
	EXPECT_LT(ImGui::GetStyle().FramePadding.y, base.y);

	applyTheme(ctx, Theme::dark().withDensity(2.0f));
	EXPECT_GT(ImGui::GetStyle().FramePadding.y, base.y);
}

TEST(Wg2P2, ThemeTokensExposeRoles) {
	const Theme t = Theme::dark();
	const Theme::Tokens tk = t.tokens();
	EXPECT_FLOAT_EQ(tk.bg.x, t.windowBg.x);
	EXPECT_FLOAT_EQ(tk.surface.x, t.childBg.x);
	EXPECT_FLOAT_EQ(tk.accent.x, t.accent.x);
	EXPECT_GT(t.danger.x, t.danger.y);
	EXPECT_GT(t.success.y, t.success.x);
}

// —— 富文本解析 ——

TEST(Wg2P2, ParseRichTextSpans) {
	const auto spans = parseRichText("a **b** c `d` [e](f)");
	ASSERT_EQ(spans.size(), 6u);
	EXPECT_EQ(spans[0].kind, RichKind::Text);
	EXPECT_EQ(spans[0].text, "a ");
	EXPECT_EQ(spans[1].kind, RichKind::Bold);
	EXPECT_EQ(spans[1].text, "b");
	EXPECT_EQ(spans[2].text, " c ");
	EXPECT_EQ(spans[3].kind, RichKind::Code);
	EXPECT_EQ(spans[3].text, "d");
	EXPECT_EQ(spans[4].text, " ");
	EXPECT_EQ(spans[5].kind, RichKind::Link);
	EXPECT_EQ(spans[5].text, "e");
	EXPECT_EQ(spans[5].href, "f");
}

TEST(Wg2P2, RichTextUnclosedMarkerStaysPlain) {
	const auto spans = parseRichText("**oops");
	ASSERT_EQ(spans.size(), 1u);
	EXPECT_EQ(spans[0].kind, RichKind::Text);
	EXPECT_EQ(spans[0].text, "**oops");
}

TEST(Wg2P2, RichTextDrawsInFrame) {
	wg2test::Harness h;
	RichText<Id("rt")> rt;
	rt.text = "**bold** `code` [link](url)";
	h.layoutFrame(rt, Rect{0.0f, 0.0f, 400.0f, 24.0f});
}

// —— 图表 ——

TEST(Wg2P2, MapValueEndpointsAndClamp) {
	EXPECT_FLOAT_EQ(detail::mapValue(0.0f, 0.0f, 10.0f, 100.0f, 0.0f), 100.0f);
	EXPECT_FLOAT_EQ(detail::mapValue(10.0f, 0.0f, 10.0f, 100.0f, 0.0f), 0.0f);
	EXPECT_FLOAT_EQ(detail::mapValue(5.0f, 0.0f, 10.0f, 100.0f, 0.0f), 50.0f);
	EXPECT_FLOAT_EQ(detail::mapValue(-5.0f, 0.0f, 10.0f, 100.0f, 0.0f), 100.0f);
	EXPECT_FLOAT_EQ(detail::mapValue(50.0f, 0.0f, 10.0f, 100.0f, 0.0f), 0.0f);
	EXPECT_FLOAT_EQ(detail::mapValue(5.0f, 5.0f, 5.0f, 100.0f, 0.0f), 50.0f);
}

TEST(Wg2P2, ChartsDrawInFrame) {
	wg2test::Harness h;
	LineChart<Id("lc")> line;
	line.series.push_back(ChartSeries{{1.0f, 3.0f, 2.0f, 5.0f}, 0, "s"});
	h.layoutFrame(line, Rect{0.0f, 0.0f, 320.0f, 160.0f});

	BarChart<Id("bc")> bars;
	bars.values = {1.0f, 4.0f, 2.0f};
	h.layoutFrame(bars, Rect{0.0f, 0.0f, 320.0f, 140.0f});
}

// —— 交互测试夹具 ——

TEST(Wg2P2, ClickingButtonFiresCallback) {
	wg2test::Harness h;
	Button<Id("b")> btn;
	btn.label = "ok";
	int clicks = 0;
	btn.onClick = [&clicks] { ++clicks; };

	auto draw = [&] { layout(btn, Rect{10.0f, 10.0f, 80.0f, 24.0f}); };
	h.click(28.0f, 22.0f, draw);

	EXPECT_EQ(clicks, 1);
}
