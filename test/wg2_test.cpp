#include <gtest/gtest.h>
#include <imgui_mk/ui/wg2/wg2.h>

#include <memory>
#include <string>

// wg2 静态控件体系单元测试：编译期字符串 ID、静态布局、动态层 opt-in。

using namespace Mk::wg2;

namespace {

template<FixedString Id>
struct Probe {
	static constexpr auto id = Id;

	WidgetState<> state;
	float w = 0.0f, h = 0.0f;

	Size hint(const Size&) const { return Size{w, h}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {}
};

} // namespace

TEST(Wg2FixedString, LiteralCreatesComparableId) {
	constexpr auto a = Id("save");
	constexpr auto b = Id("save");
	constexpr auto c = Id("cancel");
	EXPECT_EQ(a, b);
	EXPECT_NE(a, c);
	EXPECT_EQ(a.size, 4u);
	EXPECT_EQ(c.size, 6u);
}

TEST(Wg2StaticTree, StringPathReplacesNumericChildIndex) {
	VBox<Id("root"),
		HBox<Id("bar"), Probe<Id("save")>, Probe<Id("cancel")>>,
		Probe<Id("status")>
	> root;

	root.at<Id("bar"), Id("save")>().w = 11.0f;
	root.at<Id("bar"), Id("cancel")>().w = 22.0f;
	root.at<Id("status")>().w = 33.0f;

	EXPECT_FLOAT_EQ(root.child<0>().child<0>().w, 11.0f);
	EXPECT_FLOAT_EQ(root.child<0>().child<1>().w, 22.0f);
	EXPECT_FLOAT_EQ(root.child<1>().w, 33.0f);
}

TEST(Wg2StaticLayout, VBoxHintAggregatesVisibleChildren) {
	VBox<Id("box"), Probe<Id("a")>, Probe<Id("b")>> box;
	box.spacing = 8.0f;
	box.marginX = 0.0f;
	box.marginY = 0.0f;

	box.at<Id("a")>().w = 10.0f;
	box.at<Id("a")>().h = 20.0f;
	box.at<Id("b")>().w = 30.0f;
	box.at<Id("b")>().h = 40.0f;

	const Size s = box.hint(Size{100.0f, 100.0f});
	EXPECT_FLOAT_EQ(s.w, 30.0f);
	EXPECT_FLOAT_EQ(s.h, 68.0f);   // 20 + 8 + 40

	box.at<Id("a")>().state.visible = false;
	const Size s2 = box.hint(Size{100.0f, 100.0f});
	EXPECT_FLOAT_EQ(s2.w, 30.0f);
	EXPECT_FLOAT_EQ(s2.h, 40.0f);
}

TEST(Wg2StaticLayout, HBoxHintAggregatesVisibleChildren) {
	HBox<Id("row"), Probe<Id("a")>, Probe<Id("b")>> row;
	row.spacing = 8.0f;
	row.marginX = 0.0f;
	row.marginY = 0.0f;

	row.at<Id("a")>().w = 10.0f;
	row.at<Id("a")>().h = 20.0f;
	row.at<Id("b")>().w = 30.0f;
	row.at<Id("b")>().h = 40.0f;

	const Size s = row.hint(Size{100.0f, 100.0f});
	EXPECT_FLOAT_EQ(s.w, 48.0f);   // 10 + 8 + 30
	EXPECT_FLOAT_EQ(s.h, 40.0f);
}

TEST(Wg2Dynamic, AnyWidgetOwnsTypedWidgetAndFindsById) {
	RuntimeList<> list;
	list.spacing = 8.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	auto a = std::make_unique<Probe<Id("item_a")>>();
	a->w = 12.0f;
	a->h = 24.0f;
	AnyWidget any = AnyWidget::own(std::move(a));
	any.idname = "item_a";
	list.add(std::move(any));

	auto b = std::make_unique<Probe<Id("item_b")>>();
	b->w = 40.0f;
	b->h = 50.0f;
	AnyWidget anyb = AnyWidget::own(std::move(b));
	anyb.idname = "item_b";
	list.add(std::move(anyb));

	EXPECT_EQ(list.size(), 2u);
	EXPECT_NE(list.findById("item_b"), nullptr);
	EXPECT_EQ(list.findById("missing"), nullptr);

	const Size s = list.hint(Size{100.0f, 100.0f});
	EXPECT_FLOAT_EQ(s.w, 40.0f);
	EXPECT_FLOAT_EQ(s.h, 82.0f);   // 24 + 8 + 50
}

TEST(Wg2Dynamic, AddDuringRenderIsDeferredThenFlushed) {
	RuntimeList<> list;
	list.spacing = 0.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	auto p = std::make_unique<Probe<Id("late")>>();
	p->w = 5.0f;
	p->h = 6.0f;
	AnyWidget any = AnyWidget::own(std::move(p));
	any.idname = "late";

	++g_RenderDepth;
	list.add(std::move(any));
	--g_RenderDepth;

	EXPECT_EQ(list.size(), 1u);              // pending 计入
	EXPECT_EQ(list.findById("late"), nullptr); // 尚未 flush 到正式槽位

	const Size s = list.hint(Size{50.0f, 50.0f});
	EXPECT_FLOAT_EQ(s.h, 6.0f);
	EXPECT_NE(list.findById("late"), nullptr); // flush 后可查找
}

TEST(Wg2Dynamic, RemoveIsTombstoneAndSizeExcludesIt) {
	RuntimeList<> list;
	list.spacing = 0.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	auto p = std::make_unique<Probe<Id("gone")>>();
	p->w = 7.0f;
	p->h = 8.0f;
	AnyWidget any = AnyWidget::own(std::move(p));
	any.idname = "gone";
	list.add(std::move(any));
	EXPECT_EQ(list.size(), 1u);

	list.removeAt(0);
	EXPECT_EQ(list.size(), 0u);
	EXPECT_EQ(list.findById("gone"), nullptr);

	list.compact();
	EXPECT_EQ(list.size(), 0u);
}
