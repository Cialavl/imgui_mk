#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>

#include <string>
#include <vector>

using namespace Mk::wg2;

// —— 数据视图 ——

TEST(Wg2P0, NaturalCellCompare) {
	EXPECT_TRUE(detail::cellLess("2", "10"));
	EXPECT_TRUE(detail::cellLess("file2", "file10"));
	EXPECT_FALSE(detail::cellLess("file10", "file2"));
	EXPECT_TRUE(detail::cellLess("a", "b"));
	EXPECT_FALSE(detail::cellLess("b", "a"));
	EXPECT_TRUE(detail::cellLess("a1b", "a1c"));
	EXPECT_FALSE(detail::cellLess("same", "same"));
}

TEST(Wg2P0, VirtualListHintUsesAvailableOrFixedHeight) {
	VirtualList<Id("vl")> list;
	list.rowCount = 5000;
	EXPECT_FLOAT_EQ(list.hint(Size{320.0f, 200.0f}).w, 320.0f);
	EXPECT_FLOAT_EQ(list.hint(Size{320.0f, 200.0f}).h, 200.0f);
	list.height = 80.0f;
	EXPECT_FLOAT_EQ(list.hint(Size{320.0f, 200.0f}).h, 80.0f);
}

TEST(Wg2P0, VirtualListDrawsLargeDataWithoutCrash) {
	wg2test::Harness h;
	VirtualList<Id("vl")> list;
	list.rowCount = 100000;
	list.text = [](int i) { return "row " + std::to_string(i); };
	h.layoutFrame(list, Rect{0.0f, 0.0f, 400.0f, 200.0f});
	EXPECT_GT(h.frameCount(), 0);
}

TEST(Wg2P0, DataTableSortsByColumn) {
	DataTable<Id("dt")> t;
	t.headers = {"name", "count"};
	t.rowCount = 3;
	const char* names[] = {"b", "a", "c"};
	const char* counts[] = {"10", "2", "1"};
	t.cell = [&](int r, int c) { return std::string(c == 0 ? names[r] : counts[r]); };

	ImGuiTableColumnSortSpecs spec{};
	spec.ColumnUserID = 1;   // 第一列
	spec.SortDirection = ImGuiSortDirection_Ascending;
	ImGuiTableSortSpecs specs{};
	specs.Specs = &spec;
	specs.SpecsCount = 1;
	specs.SpecsDirty = true;

	t.rebuildOrder(&specs);
	EXPECT_EQ(t.order, (std::vector<int>{1, 0, 2}));   // a, b, c

	spec.ColumnUserID = 2;   // 第二列：1, 2, 10
	t.rebuildOrder(&specs);
	EXPECT_EQ(t.order, (std::vector<int>{2, 1, 0}));

	spec.SortDirection = ImGuiSortDirection_Descending;
	t.rebuildOrder(&specs);
	EXPECT_EQ(t.order, (std::vector<int>{0, 1, 2}));
}

TEST(Wg2P0, DataTableDrawsInFrame) {
	wg2test::Harness h;
	DataTable<Id("dt")> t;
	t.headers = {"a", "b"};
	t.rowCount = 2000;
	t.cell = [](int r, int c) { return std::to_string(c == 0 ? r : r * 2); };
	h.layoutFrame(t, Rect{0.0f, 0.0f, 500.0f, 300.0f});
	EXPECT_GT(h.frameCount(), 0);
}

// —— 拖放 ——

TEST(Wg2P0, DragPayloadTypeIsStablePerType) {
	const std::string a = payloadType<int>();
	const std::string b = payloadType<int>();
	const std::string c = payloadType<float>();
	EXPECT_EQ(a, b);
	EXPECT_NE(a, c);
	EXPECT_LT(a.size(), 32u);   // ImGui 负载类型名长度上限
}

// —— 对话框族 ——

TEST(Wg2P0, ToastExpiresAfterTtl) {
	ToastHost t;
	t.toast("a", 1.0f);
	t.toast("b", 5.0f);
	t.advance(0.5f);
	EXPECT_EQ(t.items.size(), 2u);
	t.advance(0.6f);
	ASSERT_EQ(t.items.size(), 1u);
	EXPECT_EQ(t.items[0].text, "b");
	t.advance(10.0f);
	EXPECT_TRUE(t.items.empty());
}

TEST(Wg2P0, ConfirmDialogShowSetsContent) {
	ConfirmDialog d;
	d.show("标题", "内容");
	EXPECT_TRUE(d.open);
	EXPECT_EQ(d.title, "标题");
	EXPECT_EQ(d.text, "内容");
}

TEST(Wg2P0, SpinnerAndToastsDrawInFrame) {
	wg2test::Harness h;
	ToastHost toasts;
	toasts.toast("hello");
	Spinner<Id("spin")> spinner;
	h.inFrame([&] {
		toasts.draw(Rect{0.0f, 0.0f, 800.0f, 600.0f});
		spinner.draw(Rect{0.0f, 0.0f, 24.0f, 24.0f});
	});
	EXPECT_GT(h.frameCount(), 0);
}

// —— 输入 ——

TEST(Wg2P0, MultilineInputBufferAndHint) {
	wg2test::Harness h;
	InputTextMultiline<Id("ml"), 128> in;
	in.setText("hello\nworld");
	EXPECT_STREQ(in.c_str(), "hello\nworld");
	h.inFrame([&] {
		const Size s = in.hint(Size{300.0f, 0.0f});
		EXPECT_FLOAT_EQ(s.w, 300.0f);
		EXPECT_GT(s.h, 0.0f);
	});
}

// —— 焦点 / 指针 ——

TEST(Wg2P0, FocusRequestConsumedByDraw) {
	wg2test::Harness h;
	Button<Id("b")> btn;
	btn.label = "ok";
	requestFocus(btn);
	EXPECT_TRUE(widgetChrome(btn).focusRequest);
	h.layoutFrame(btn, Rect{10.0f, 10.0f, 80.0f, 24.0f});
	EXPECT_FALSE(widgetChrome(btn).focusRequest);
}

TEST(Wg2P0, CursorMapping) {
	EXPECT_EQ(toImGuiCursor(Cursor::Arrow), ImGuiMouseCursor_Arrow);
	EXPECT_EQ(toImGuiCursor(Cursor::TextInput), ImGuiMouseCursor_TextInput);
	EXPECT_EQ(toImGuiCursor(Cursor::Hand), ImGuiMouseCursor_Hand);
	EXPECT_EQ(toImGuiCursor(Cursor::ResizeEW), ImGuiMouseCursor_ResizeEW);
}
