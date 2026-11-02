#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <memory>

// 第四层 · 动态容器：RuntimeList / RuntimeTabs / RuntimeGrid / AnyWidget

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;
using wg2test::CaptureProbe;

TEST(Wg2Comm, RuntimeListExposesTypedChildrenById) {
	Harness h;
	RuntimeList<Id("list")> list;
	list.spacing = 0.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	auto make = [](const char* idname, float w, float hgt) {
		auto p = std::make_unique<RectProbe<Id("item")>>();
		p->w = w;
		p->h = hgt;
		AnyWidget a = AnyWidget::own(std::move(p));
		a.idname = idname;
		return a;
	};

	list.add(make("a", 10.0f, 5.0f));
	list.add(make("b", 20.0f, 7.0f));
	EXPECT_EQ(list.size(), 2u);
	ASSERT_NE(list.findById("b"), nullptr);
	EXPECT_FLOAT_EQ(list.findById("b")->hint(Size{100.0f, 0.0f}).h, 7.0f);

	h.inFrame([&] {
		EXPECT_FLOAT_EQ(list.hint(Size{100.0f, 0.0f}).h, 12.0f);
		layout(list, Rect{0.0f, 0.0f, 100.0f, 40.0f});
	});

	list.removeAt(0);
	EXPECT_EQ(list.size(), 1u);
	EXPECT_EQ(list.findById("a"), nullptr);
	list.clear();
	EXPECT_EQ(list.size(), 0u);
}

TEST(Wg2Comm, RuntimeTabsAreFoundByTitleOrId) {
	Harness h;
	RuntimeTabs<Id("tabs")> tabs;
	tabs.idname = "dyn-tabs";

	auto p1 = std::make_unique<RectProbe<Id("p1")>>();
	p1->h = 6.0f;
	AnyWidget w1 = AnyWidget::own(std::move(p1));
	w1.idname = "page_a";
	tabs.add("Page A", std::move(w1));

	auto p2 = std::make_unique<RectProbe<Id("p2")>>();
	p2->h = 8.0f;
	AnyWidget w2 = AnyWidget::own(std::move(p2));
	w2.idname = "page_b";
	tabs.add("Page B", std::move(w2));

	EXPECT_EQ(tabs.size(), 2u);
	EXPECT_NE(tabs.findById("Page A"), nullptr);   // 按标题查
	EXPECT_NE(tabs.findById("page_b"), nullptr);   // 按 idname 查

	h.inFrame([&] { layout(tabs, Rect{0.0f, 0.0f, 200.0f, 80.0f}); });
	tabs.removeLast();
	EXPECT_EQ(tabs.size(), 1u);
}

TEST(Wg2Comm, AnyWidgetMovesOwnershipAndKeepsBehaviour) {
	auto p = std::make_unique<RectProbe<Id("x")>>();
	p->h = 5.0f;
	AnyWidget a = AnyWidget::own(std::move(p));
	EXPECT_TRUE(a.valid());
	EXPECT_FLOAT_EQ(a.hint(Size{10.0f, 0.0f}).h, 5.0f);

	AnyWidget b = std::move(a);
	EXPECT_FALSE(a.valid());
	EXPECT_TRUE(b.valid());
	EXPECT_FLOAT_EQ(b.hint(Size{10.0f, 0.0f}).h, 5.0f);

	AnyWidget empty;
	EXPECT_FALSE(empty.valid());
	EXPECT_FLOAT_EQ(empty.hint(Size{10.0f, 0.0f}).h, 0.0f);
}

TEST(Wg2Comm, RuntimeGridArrangesDynamicChildren) {
	Harness h;
	RuntimeGrid<Id("grid")> grid;
	grid.cols = 2;
	grid.spacing = 10.0f;
	grid.marginX = 0.0f;
	grid.marginY = 0.0f;

	Mk::wg2::Rect a{}, b{}, c{};
	auto add = [&](Mk::wg2::Rect* out, const char* idname) {
		auto p = std::make_unique<CaptureProbe>();
		p->out = out;
		AnyWidget w = AnyWidget::own(std::move(p));
		w.idname = idname;
		grid.add(std::move(w));
	};
	add(&a, "a");
	add(&b, "b");
	add(&c, "c");
	EXPECT_EQ(grid.size(), 3u);
	EXPECT_NE(grid.findById("c"), nullptr);

	h.inFrame([&] { layout(grid, Mk::wg2::Rect{0.0f, 0.0f, 110.0f, 100.0f}); });

	// 列宽 = (110 - 10) / 2 = 50；第二行 y = 行高 10 + 间距 10
	EXPECT_FLOAT_EQ(a.x, 0.0f);
	EXPECT_FLOAT_EQ(a.w, 50.0f);
	EXPECT_FLOAT_EQ(b.x, 60.0f);
	EXPECT_FLOAT_EQ(c.y, 20.0f);

	grid.removeLast();
	EXPECT_EQ(grid.size(), 2u);
}

TEST(Wg2Comm, DynamicContainersAggregateHint) {
	Harness h;
	auto make = [](const char* idname, float w, float hgt) {
		auto p = std::make_unique<RectProbe<Id("item")>>();
		p->w = w;
		p->h = hgt;
		AnyWidget a = AnyWidget::own(std::move(p));
		a.idname = idname;
		return a;
	};

	RuntimeTabs<Id("tabs")> tabs;
	tabs.add("Tab A", make("t1", 10.0f, 20.0f));
	tabs.add("Tab B", make("t2", 30.0f, 40.0f));

	RuntimeList<Id("list")> list;
	list.spacing = 8.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;
	list.add(make("a", 12.0f, 24.0f));
	list.add(make("b", 40.0f, 50.0f));

	RuntimeGrid<Id("grid")> grid;
	grid.cols = 2;
	grid.spacing = 8.0f;
	grid.marginX = 0.0f;
	grid.marginY = 0.0f;
	grid.add(make("g1", 10.0f, 20.0f));
	grid.add(make("g2", 30.0f, 40.0f));
	grid.add(make("g3", 50.0f, 60.0f));

	h.inFrame([&] {
		const Size ts = tabs.hint(Size{100.0f, 0.0f});
		EXPECT_FLOAT_EQ(ts.w, 30.0f);       // 取子项最大值
		EXPECT_FLOAT_EQ(ts.h, 40.0f);

		const Size ls = list.hint(Size{100.0f, 0.0f});
		EXPECT_FLOAT_EQ(ls.w, 40.0f);
		EXPECT_FLOAT_EQ(ls.h, 82.0f);       // 24 + 8 + 50

		const Size gs = grid.hint(Size{100.0f, 0.0f});
		EXPECT_FLOAT_EQ(gs.w, 108.0f);      // 50*2 + 8
		EXPECT_FLOAT_EQ(gs.h, 108.0f);      // 40 + 8 + 60
	});
}

TEST(Wg2Comm, AnyWidgetDestructionRecyclesChrome) {
	Harness h;
	const auto baseline = chromeCount();
	{
		AnyWidget a = AnyWidget::own(std::make_unique<RectProbe<Id("cleanup")>>());
		setStretch(a, 2.0f);
		h.inFrame([&] { layout(a, Rect{0.0f, 0.0f, 80.0f, 40.0f}); });
		EXPECT_EQ(chromeCount(), baseline + 2);   // AnyWidget 自身 + 内部控件各一条
	}
	EXPECT_EQ(chromeCount(), baseline);           // 析构后两条都回收
}

TEST(Wg2Comm, AnyWidgetMoveCarriesChrome) {
	Harness h;
	AnyWidget a = AnyWidget::own(std::make_unique<RectProbe<Id("move")>>());
	setVisible(a, false);

	AnyWidget b = std::move(a);
	EXPECT_FALSE(widgetChrome(b).visible);        // 状态随对象迁移
	EXPECT_EQ(chromeCount(), std::size_t{1});     // 旧地址不留残余
}
