// 框架层导航/页面宿主：不依赖 ImGui 上下文的那部分逻辑
#include <gtest/gtest.h>

#include <imgui_mk/ui/wg2/widgets/nav.h>
#include "wg2_test_harness.h"

using namespace Mk::wg2;

namespace {

using Nav = NavList<Id("nav")>;
using Host = PageHost<Id("host")>;

} // namespace

TEST(Wg2Nav, NavEntryDeduplicatesConsecutiveHeaders) {
	Nav nav;
	nav.addEntry("分组 A", true);
	nav.addEntry("分组 A", true);   // 连续同名组标题只保留一个
	nav.addEntry("页 1", false);
	nav.addEntry("页 2", false);
	EXPECT_EQ(nav.entries.size(), std::size_t{3});
	EXPECT_TRUE(nav.entries[0].header);
}

TEST(Wg2Nav, PageCountSkipsHeaders) {
	Nav nav;
	nav.addEntry("分组 A", true);
	nav.addEntry("页 1", false);
	nav.addEntry("页 2", false);
	nav.addEntry("分组 B", true);
	nav.addEntry("页 3", false);
	EXPECT_EQ(nav.pageCount(), 3);
}

TEST(Wg2Nav, PageHostTracksRegisteredPages) {
	Host host;
	EXPECT_EQ(host.pageCount(), 0);
	host.addPage([](const Rect&) {});
	host.addPage([](const Rect&) {});
	EXPECT_EQ(host.pageCount(), 2);
}

TEST(Wg2Nav, PageBookRegistersPages) {
	PageBook<Id("book")> book;
	book.addGroup("分组 A");
	book.addPage("页 1", [](const Rect&) {});
	book.addPage("页 2", [](const Rect&) {});
	EXPECT_EQ(book.pageCount(), 2);
	EXPECT_EQ(book.nav.entries.size(), std::size_t{3});   // 1 组标题 + 2 页
	EXPECT_EQ(book.current, 0);
}

TEST(Wg2Nav, PageBookCurrentTitleFollowsSelection) {
	PageBook<Id("book2")> book;
	book.addGroup("分组 A");
	book.addPage("第一页", [](const Rect&) {});
	book.addPage("第二页", [](const Rect&) {});
	EXPECT_STREQ(book.currentTitle(), "第一页");
	book.current = 1;
	EXPECT_STREQ(book.currentTitle(), "第二页");
	book.current = 99;                 // 越界返回空串，不越界读
	EXPECT_STREQ(book.currentTitle(), "");
}

// ============ 绘制路径（需要 ImGui 上下文，走 harness）============

TEST(Wg2Nav, DrawRendersWithoutCrashAndHeightMatchesEntries) {
	wg2test::Harness h;
	NavList<Id("nav")> nav;
	int sel = 0;
	nav.current = &sel;
	nav.addEntry("分组 A", true);
	nav.addEntry("页 1", false);
	nav.addEntry("页 2", false);

	h.inFrame([&] { Mk::wg2::layout(nav, Rect{0.0f, 0.0f, 300.0f, 200.0f}); });

	// 高度 = 2 行 + 1 个组标题（行高/标题高都按当前字号度量）
	const float expected = ImGui::GetFrameHeight() * 2.0f
		+ ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
	EXPECT_FLOAT_EQ(nav.hint(Size{300.0f, 0.0f}).h, expected);
}

TEST(Wg2Nav, PageBookDrawsCurrentPageOnly) {
	wg2test::Harness h;
	PageBook<Id("book")> book;
	int first = 0, second = 0;
	book.addGroup("分组 A");
	book.addPage("页 1", [&](const Rect&) { ++first; });
	book.addPage("页 2", [&](const Rect&) { ++second; });

	h.inFrame([&] { Mk::wg2::layout(book, Rect{0.0f, 0.0f, 600.0f, 300.0f}); });
	EXPECT_EQ(first, 1);
	EXPECT_EQ(second, 0);

	book.current = 1;                                   // 切到第二页
	h.inFrame([&] { Mk::wg2::layout(book, Rect{0.0f, 0.0f, 600.0f, 300.0f}); });
	EXPECT_EQ(first, 1);
	EXPECT_EQ(second, 1);
}

