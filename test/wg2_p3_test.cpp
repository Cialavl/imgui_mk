#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>

#include <string>
#include <vector>

using namespace Mk::wg2;

TEST(Wg2P3, PanelsLandInRootGroup) {
	DockSpace dock;
	const int a = dock.addPanel("A", [](const Rect&) {});
	const int b = dock.addPanel("B", [](const Rect&) {});

	EXPECT_EQ(dock.root(), 0);
	EXPECT_TRUE(dock.isLeaf(0));
	EXPECT_EQ(dock.leafPanels(0), (std::vector<int>{a, b}));
	EXPECT_EQ(dock.leafOf(b), 0);
	EXPECT_EQ(dock.leafCount(), 1);
	EXPECT_EQ(dock.panelCount(), 2);
}

TEST(Wg2P3, DockRightSplitsLeaf) {
	DockSpace dock;
	const int a = dock.addPanel("A", [](const Rect&) {});
	const int b = dock.addPanel("B", [](const Rect&) {});

	ASSERT_TRUE(dock.dockPanel(b, 0, DockEdge::Right));
	EXPECT_FALSE(dock.isLeaf(0));
	EXPECT_EQ(dock.leafCount(), 2);
	EXPECT_EQ(dock.leafOf(a), 1);
	EXPECT_EQ(dock.leafOf(b), 2);
	EXPECT_EQ(dock.leafPanels(1), (std::vector<int>{a}));
	EXPECT_EQ(dock.leafPanels(2), (std::vector<int>{b}));
}

TEST(Wg2P3, DockTopPutsPanelFirst) {
	DockSpace dock;
	const int a = dock.addPanel("A", [](const Rect&) {});
	const int b = dock.addPanel("B", [](const Rect&) {});
	ASSERT_TRUE(dock.dockPanel(b, 0, DockEdge::Top));
	EXPECT_EQ(dock.leafPanels(1), (std::vector<int>{b}));   // 新面板在上
	EXPECT_EQ(dock.leafPanels(2), (std::vector<int>{a}));
}

TEST(Wg2P3, MovePanelBetweenLeaves) {
	DockSpace dock;
	const int a = dock.addPanel("A", [](const Rect&) {});
	const int b = dock.addPanel("B", [](const Rect&) {});
	ASSERT_TRUE(dock.dockPanel(b, 0, DockEdge::Right));

	ASSERT_TRUE(dock.movePanel(b, 1));
	EXPECT_EQ(dock.leafPanels(1), (std::vector<int>{a, b}));
	EXPECT_TRUE(dock.leafPanels(2).empty());
	EXPECT_EQ(dock.leafOf(b), 1);
}

TEST(Wg2P3, ThreePanelSequenceStructure) {
	DockSpace dock;
	const int v = dock.addPanel("V", [](const Rect&) {});
	const int s = dock.addPanel("S", [](const Rect&) {});
	const int p = dock.addPanel("P", [](const Rect&) {});
	ASSERT_TRUE(dock.dockPanel(s, dock.root(), DockEdge::Left));
	ASSERT_TRUE(dock.dockPanel(p, dock.leafOf(v), DockEdge::Right));
	EXPECT_EQ(dock.leafCount(), 3);
	EXPECT_EQ(dock.leafOf(s), 1);
	EXPECT_EQ(dock.leafOf(v), 3);
	EXPECT_EQ(dock.leafOf(p), 4);
	EXPECT_EQ(dock.leafPanels(3), (std::vector<int>{v}));
	EXPECT_EQ(dock.leafPanels(4), (std::vector<int>{p}));
}

TEST(Wg2P3, PanelsDrawInsideTheirOwnChildWindows) {
	wg2test::Harness h;
	DockSpace dock;
	bool drawn[3] = {false, false, false};
	float widths[3] = {0.0f, 0.0f, 0.0f};
	float xs[3] = {-1.0f, -1.0f, -1.0f};
	const int v = dock.addPanel("V", [&](const Rect& r) { drawn[0] = true; widths[0] = r.w; xs[0] = r.x; });
	const int s = dock.addPanel("S", [&](const Rect& r) { drawn[1] = true; widths[1] = r.w; xs[1] = r.x; });
	const int p = dock.addPanel("P", [&](const Rect& r) { drawn[2] = true; widths[2] = r.w; xs[2] = r.x; });
	ASSERT_TRUE(dock.dockPanel(s, dock.root(), DockEdge::Left));
	ASSERT_TRUE(dock.dockPanel(p, dock.leafOf(v), DockEdge::Right));

	h.layoutFrame(dock, Rect{0.0f, 0.0f, 600.0f, 300.0f});

	EXPECT_TRUE(drawn[0]);
	EXPECT_TRUE(drawn[1]);
	EXPECT_TRUE(drawn[2]);
	EXPECT_GT(widths[1], 0.0f);   // 层级列
	EXPECT_GT(widths[0], 0.0f);   // 视口列
	EXPECT_GT(widths[2], 0.0f);   // 属性列
	EXPECT_FLOAT_EQ(xs[0], 0.0f); // 面板内容在各自子窗口内，坐标从 0 起
	EXPECT_FLOAT_EQ(xs[2], 0.0f);
}

TEST(Wg2P3, SerializeRoundTrip) {
	DockSpace a;
	a.addPanel("A", [](const Rect&) {});
	a.addPanel("B", [](const Rect&) {});
	ASSERT_TRUE(a.dockPanel(1, 0, DockEdge::Right));
	const std::string text = a.serialize();

	DockSpace b;
	b.addPanel("A", [](const Rect&) {});
	b.addPanel("B", [](const Rect&) {});
	ASSERT_TRUE(b.deserialize(text));

	EXPECT_EQ(b.nodeCount(), a.nodeCount());
	EXPECT_EQ(b.leafCount(), a.leafCount());
	EXPECT_EQ(b.leafOf(0), a.leafOf(0));
	EXPECT_EQ(b.leafOf(1), a.leafOf(1));

	EXPECT_FALSE(b.deserialize("garbage"));
}

TEST(Wg2P3, DrawsPanelsInFrame) {
	wg2test::Harness h;
	DockSpace dock;
	bool drawn = false;
	dock.addPanel("A", [&drawn](const Rect&) { drawn = true; });
	h.layoutFrame(dock, Rect{0.0f, 0.0f, 600.0f, 400.0f});
	EXPECT_TRUE(drawn);
}

TEST(Wg2P3, SplitCreatesTwoLeaves) {
	DockSpace dock;
	const int a = dock.addPanel("A", [](const Rect&) {});
	ASSERT_TRUE(dock.split(0, DockAxis::Vertical, 0.5f));
	EXPECT_EQ(dock.leafCount(), 2);
	EXPECT_EQ(dock.leafOf(a), 1);
	EXPECT_TRUE(dock.leafPanels(2).empty());
}
