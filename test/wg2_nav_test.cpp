// 框架层导航/页面宿主：不依赖 ImGui 上下文的那部分逻辑
#include <gtest/gtest.h>

#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/ui/wg2/widgets/nav.h>
#include "wg2_test_harness.h"

using namespace Mk::wg2;

namespace {

using Nav = NavTree<Id("nav")>;
using Host = PageHost<Id("host")>;

} // namespace

TEST(Wg2Nav, NavTreeGroupsOwnTheirPages) {
	Nav nav;
	nav.addGroup("分组 A");
	nav.addPage("页 1");
	nav.addPage("页 2");
	nav.addGroup("分组 B");   // 结束上一个顶层分组
	nav.addPage("页 3");

	ASSERT_EQ(nav.roots.size(), std::size_t{2});
	EXPECT_TRUE(nav.roots[0].group);
	EXPECT_EQ(nav.roots[0].children.size(), std::size_t{2});
	EXPECT_EQ(nav.roots[1].children.size(), std::size_t{1});
	EXPECT_EQ(nav.pageCount(), 3);
	EXPECT_STREQ(nav.pageTitle(0), "页 1");
	EXPECT_STREQ(nav.pageTitle(2), "页 3");
	EXPECT_STREQ(nav.pageTitle(3), "");       // 越界返回空串
}

TEST(Wg2Nav, NavTreeNestsSubGroupsInDfsOrder) {
	Nav nav;
	nav.addGroup("A");
	nav.addPage("a1");
	nav.beginGroup("A-1");
	nav.addPage("a1x");
	nav.endGroup();
	nav.addPage("a2");
	nav.addGroup("B");
	nav.addPage("b1");

	ASSERT_EQ(nav.roots[0].children.size(), std::size_t{3});
	EXPECT_TRUE(nav.roots[0].children[1].group);
	EXPECT_EQ(nav.pageCount(), 4);
	EXPECT_STREQ(nav.pageTitle(0), "a1");
	EXPECT_STREQ(nav.pageTitle(1), "a1x");     // 叶子按 DFS 顺序编号
	EXPECT_STREQ(nav.pageTitle(2), "a2");
	EXPECT_STREQ(nav.pageTitle(3), "b1");
}

TEST(Wg2Nav, NavTreeCollapsedGroupHidesRowsButKeepsPageIndex) {
	Nav nav;
	nav.addGroup("A");
	nav.addPage("a1");
	nav.addPage("a2");
	nav.addGroup("B");
	nav.addPage("b1");

	EXPECT_EQ(nav.visibleRows(), 5);   // 2 个分组 + 3 页
	nav.roots[0].expanded = false;
	EXPECT_EQ(nav.visibleRows(), 3);   // 收起 A：A、B、b1
	EXPECT_EQ(nav.pageCount(), 3);     // 页序号不变（收起只影响显示）
	EXPECT_STREQ(nav.pageTitle(2), "b1");
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
	ASSERT_EQ(book.nav().roots.size(), std::size_t{1});
	EXPECT_EQ(book.nav().roots[0].children.size(), std::size_t{2});   // 1 个分组下 2 页
	EXPECT_EQ(book.current, 0);

	// 组合即布局：HBox 切片，导航固定 em 宽度、宿主 stretch 吃掉剩余
	EXPECT_FLOAT_EQ(widgetChrome(book.nav()).size.fixedWEm, PageBook<Id("book")>::kNavWidthEm);
	EXPECT_FLOAT_EQ(widgetChrome(book.host()).stretch, 1.0f);
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

TEST(Wg2Nav, PageBookSelectClampsToRange) {
	PageBook<Id("book3")> book;
	book.addPage("A", [](const Rect&) {});
	book.addPage("B", [](const Rect&) {});

	book.select(1);
	EXPECT_EQ(book.current, 1);
	book.select(99);           // 越界夹到最后一页
	EXPECT_EQ(book.current, 1);
	book.select(-5);           // 负数夹到第一页
	EXPECT_EQ(book.current, 0);
}

// ============ 绘制路径（需要 ImGui 上下文，走 harness）============

TEST(Wg2Nav, DrawRendersWithoutCrashAndHeightMatchesVisibleRows) {
	wg2test::Harness h;
	NavTree<Id("nav")> nav;
	nav.addGroup("分组 A");
	nav.addPage("页 1");
	nav.addPage("页 2");

	h.inFrame([&] { Mk::wg2::layout(nav, Rect{0.0f, 0.0f, 300.0f, 200.0f}); });

	// 统一行高：1 个分组 + 2 页（组与页同高，WinUI NavigationView 风格）
	EXPECT_FLOAT_EQ(nav.hint(Size{300.0f, 0.0f}).h, ImGui::GetFrameHeight() * 3.0f);

	nav.roots[0].expanded = false;   // 收起后高度跟着可见行数走
	EXPECT_FLOAT_EQ(nav.hint(Size{300.0f, 0.0f}).h, ImGui::GetFrameHeight());

	h.inFrame([&] { Mk::wg2::layout(nav, Rect{0.0f, 0.0f, 300.0f, 200.0f}); });   // 收起状态绘制不发散
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
	EXPECT_EQ(book.nav().selected, 1);                  // 页簿每帧下发给导航
	EXPECT_EQ(book.host().page, 1);                     // 页簿每帧下发给宿主
}


TEST(Wg2Nav, ChildRegionDrawsBody) {
	wg2test::Harness h;
	ChildRegion<Id("region")> region;
	region.border = true;
	region.padding = ImVec2(0.0f, 0.0f);
	int calls = 0;
	region.body = [&](const ImVec2&) { ++calls; };

	h.inFrame([&] { Mk::wg2::layout(region, Rect{0.0f, 0.0f, 120.0f, 60.0f}); });
	EXPECT_EQ(calls, 1);
}
