#include <gtest/gtest.h>

#include "wg2_test_harness.h"

// 第二层 · 嵌套组合与带内边距容器（SubWindow / TabBar / Collapsing / Tree / Offset / Indent）

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Layout, NestedContainersComposeWithoutOverflow) {
	Harness h;
	VBox<Id("root"),
		RectProbe<Id("head")>,
		HBox<Id("row"), RectProbe<Id("left")>, RectProbe<Id("right")>>,
		RectProbe<Id("tail")>>
		root;
	root.marginX = 0.0f;
	root.marginY = 0.0f;
	root.spacing = 0.0f;
	root.at<Id("head")>().h = 10.0f;
	root.at<Id("row")>().marginX = 0.0f;
	root.at<Id("row")>().marginY = 0.0f;
	root.at<Id("row")>().spacing = 0.0f;
	root.at<Id("row"), Id("left")>().w = 50.0f;
	root.at<Id("row"), Id("left")>().h = 20.0f;
	root.at<Id("row"), Id("right")>().w = 50.0f;
	root.at<Id("row"), Id("right")>().h = 20.0f;
	root.at<Id("tail")>().h = 10.0f;

	auto& left = root.at<Id("row"), Id("left")>();
	auto& right = root.at<Id("row"), Id("right")>();

	h.inFrame([&] {
		layout(root, Rect{0.0f, 0.0f, 100.0f, 40.0f});
		EXPECT_FLOAT_EQ(root.at<Id("head")>().last.y, 0.0f);
		EXPECT_FLOAT_EQ(left.last.y, 10.0f);        // HBox 行整体落在 head 之后
		EXPECT_FLOAT_EQ(right.last.x, 50.0f);
		EXPECT_FLOAT_EQ(root.at<Id("tail")>().last.y, 30.0f);
		EXPECT_FLOAT_EQ(root.at<Id("tail")>().last.y + root.at<Id("tail")>().last.h, 40.0f);
	});
}

TEST(Wg2Layout, SubWindowPadsItsChild) {
	Harness h;
	SubWindow<Id("sub"), RectProbe<Id("inner")>> sub;
	sub.idname = "sub-win";
	sub.height = 60.0f;
	sub.border = true;

	h.inFrame([&] {
		EXPECT_FLOAT_EQ(sub.hint(Size{200.0f, 0.0f}).h, 60.0f);
		layout(sub, Rect{10.0f, 20.0f, 200.0f, 60.0f});
		// 子项坐标是「子窗口内容坐标」：原点 = 子窗口左上角（不是父窗口绝对坐标）
		const Rect& inner = sub.at<Id("inner")>().last;
		const float pad = ImGui::GetStyle().WindowPadding.x;
		EXPECT_NEAR(inner.x, pad, 1.0f);
		EXPECT_NEAR(inner.w, 200.0f - pad * 2.0f, 2.0f);
	});
}

TEST(Wg2Layout, TabBarReservesRowAndTabContentBelowIt) {
	Harness h;
	TabBar<Id("tabs"),
		Tab<Id("t1"), RectProbe<Id("c1")>>,
		Tab<Id("t2"), RectProbe<Id("c2")>>
	> tabs;
	tabs.idname = "tabs-bar";
	tabs.at<Id("t1")>().title = "One";
	tabs.at<Id("t1"), Id("c1")>().h = 10.0f;
	tabs.at<Id("t2")>().title = "Two";
	tabs.at<Id("t2"), Id("c2")>().h = 10.0f;

	auto& tab1Content = tabs.at<Id("t1"), Id("c1")>();

	h.inFrame([&] {
		const Size s = tabs.hint(Size{200.0f, 0.0f});
		// 高度包含页签条那一行（回归：漏算会让后续控件压到页签内容上）
		EXPECT_GE(s.h, ImGui::GetFrameHeight() + 10.0f);
		layout(tabs, Rect{0.0f, 0.0f, 200.0f, 80.0f});
		const Rect& c1 = tab1Content.last;
		EXPECT_GE(c1.y, ImGui::GetFrameHeight() - 1.0f);   // 内容在页签条下方
		EXPECT_LE(c1.y + c1.h, 80.0f + 1.0f);
	});
}

TEST(Wg2Layout, CollapsingAndTreeIndentTheirChildren) {
	Harness h;
	Collapsing<Id("coll"), RectProbe<Id("cp")>> coll;
	coll.title = "Group";
	coll.defaultOpen = true;
	coll.at<Id("cp")>().h = 10.0f;

	Tree<Id("tree"), RectProbe<Id("tp")>> tree;
	tree.title = "Node";
	tree.defaultOpen = true;
	tree.at<Id("tp")>().h = 10.0f;

	h.inFrame([&] {
		EXPECT_GT(coll.hint(Size{200.0f, 0.0f}).h, 10.0f);   // 含表头
		EXPECT_GT(tree.hint(Size{200.0f, 0.0f}).h, 10.0f);
		layout(coll, Rect{0.0f, 0.0f, 200.0f, 60.0f});
		layout(tree, Rect{0.0f, 80.0f, 200.0f, 60.0f});
		EXPECT_GE(coll.at<Id("cp")>().last.y, ImGui::GetFrameHeight() - 1.0f);
		EXPECT_GE(tree.at<Id("tp")>().last.y, 80.0f + ImGui::GetFrameHeight() - 1.0f);
	});
}

TEST(Wg2Layout, OffsetShiftsByEmAndIndentShiftsByPixels) {
	Harness h;
	Offset<Id("off"), RectProbe<Id("op")>> off;
	off.dxEm = 1.0f;
	off.dyEm = 0.5f;

	Indent<Id("ind"), RectProbe<Id("ip")>> ind;
	ind.amount = 16.0f;

	h.inFrame([&] {
		const float em = ImGui::GetFontSize();
		layout(off, Rect{0.0f, 0.0f, 200.0f, 40.0f});
		EXPECT_FLOAT_EQ(off.at<Id("op")>().last.x, em);
		EXPECT_FLOAT_EQ(off.at<Id("op")>().last.y, em * 0.5f);

		layout(ind, Rect{10.0f, 50.0f, 200.0f, 40.0f});
		EXPECT_FLOAT_EQ(ind.at<Id("ip")>().last.x, 26.0f);
	});
}
