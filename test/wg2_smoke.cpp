// wg2 无 gtest 冒烟检查：可在离线/未配置 BUILD_TESTING 的环境编译运行。
// 覆盖：编译期 ID、静态路径访问、VBox/HBox hint、动态 add/find/墓碑。
#include <imgui_mk/ui/wg2/wg2.h>

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

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

#define CHECK(cond) do { if(!(cond)) { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); return 1; } } while(0)
#define CHECK_FLOAT(a,b) do { if(std::fabs((a)-(b)) > 0.001f) { std::fprintf(stderr, "FAIL %s:%d: %f != %f\n", __FILE__, __LINE__, (float)(a), (float)(b)); return 1; } } while(0)

// 仅编译实例化检查：强制生成 drawWidget / layout / 基础控件的渲染路径。
// 本函数不运行，只用于在链接前暴露模板错误。
void compileOnlyTouch() {
	Button<Id("b")> b;
	Label<Id("l")> l;
	VBox<Id("v"), Button<Id("b")>, Label<Id("l")>> box;
	HBox<Id("h"), Label<Id("l")>, Button<Id("b")>> row;
	Grid<Id("g"), Label<Id("l")>, Button<Id("b")>, Label<Id("l2")>> grid;
	Wrap<Id("w"), Label<Id("l")>, Button<Id("b")>> wrap;
	Overlay<Id("o"), Label<Id("l")>, Button<Id("b")>> overlay;

	int ival = 0;
	float fval = 0.0f;
	bool bval = false;
	CheckBox<Id("chk")> check;
	check.checked = &bval;
	InputText<Id("txt"), 64> input;
	input.name = "text";
	SliderF<Id("sld")> slider;
	slider.value = &fval;
	Combo<Id("cmb")> combo;
	combo.current = &ival;
	ProgressBar<Id("pbar")> bar;
	bar.fraction = &fval;
	TextWrapped<Id("tw")> tw;
	tw.text = "hello";

	When<Id("when"), Label<Id("wl")>> when;
	when.show = true;
	ItemWidth<Id("iw"), SliderF<Id("iws")>> iw;
	iw.width = 120.0f;
	Colored<Id("col"), Label<Id("cl")>> colored;
	Indent<Id("ind"), Label<Id("il")>> indented;
	Tooltip<Id("tip"), Label<Id("tl")>> tooltip;

	SubWindow<Id("sw"), Label<Id("sl")>> sub;
	Panel<Id("panel"), Label<Id("pl")>> panel;
	RuntimeTabs<Id("rtabs")> runtimeTabs;
	RuntimeGrid<Id("rgrid")> runtimeGrid;
	TabBar<Id("tabs"),
		Tab<Id("tab1"), Label<Id("tab1l")>>,
		Tab<Id("tab2"), Label<Id("tab2l")>>
	> tabs;
	tabs.at<Id("tab1")>().title = "One";
	tabs.at<Id("tab2")>().title = "Two";
	Collapsing<Id("coll"), Label<Id("coll")>> collapsing;

	Tree<Id("tree"), Label<Id("treel")>> tree;
	tree.title = "Tree";
	MenuItem<Id("mi")> mi;
	mi.name = "Item";
	Menu<Id("menu"), MenuItem<Id("mi")>> menu;
	menu.name = "Menu";
	bool popupOpen = false;
	Popup<Id("pop"), Label<Id("popl")>> popup;
	popup.open = &popupOpen;
	PopupModal<Id("modal"), Label<Id("modall")>> modal;
	modal.open = &popupOpen;
	TextTable<Id("table")> table;
	FnTable<Id("fntable")> fnTable;
	MenuBar<Id("menubar"), Menu<Id("menu"), MenuItem<Id("mi")>>> menuBar;
	std::array<float, 3> rgb{};
	ColorEdit3<Id("ce3")> ce3;
	ce3.color = &rgb;
	CheckboxFlags<Id("flags")> flags;
	flags.flags = &ival;

	(void)b.hint(Size{100.0f, 100.0f});
	(void)l.hint(Size{100.0f, 100.0f});
	(void)check.hint(Size{100.0f, 100.0f});
	(void)input.hint(Size{100.0f, 100.0f});
	(void)slider.hint(Size{100.0f, 100.0f});
	(void)combo.hint(Size{100.0f, 100.0f});
	(void)bar.hint(Size{100.0f, 100.0f});
	(void)tw.hint(Size{100.0f, 100.0f});
	drawWidget(b, Rect{0.0f, 0.0f, 100.0f, 30.0f});
	layout(box, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(row, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(grid, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(wrap, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(overlay, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(when, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(iw, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(colored, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(indented, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(tooltip, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(sub, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(panel, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(runtimeTabs, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(runtimeGrid, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(tabs, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(collapsing, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(tree, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(mi, Rect{0.0f, 0.0f, 100.0f, 20.0f});
	layout(menu, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(popup, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(modal, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(table, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(fnTable, Rect{0.0f, 0.0f, 100.0f, 100.0f});
	layout(menuBar, Rect{0.0f, 0.0f, 100.0f, 30.0f});
	layout(ce3, Rect{0.0f, 0.0f, 260.0f, 20.0f});
	layout(flags, Rect{0.0f, 0.0f, 100.0f, 20.0f});
}

} // namespace

int main() {
	// FixedString
	constexpr auto a = Id("save");
	constexpr auto b = Id("save");
	constexpr auto c = Id("cancel");
	CHECK(a == b);
	CHECK(!(a == c));
	CHECK(a.size == 4u);

	// 静态字符串路径替代 child<I>
	VBox<Id("root"),
		HBox<Id("bar"), Probe<Id("save")>, Probe<Id("cancel")>>,
		Probe<Id("status")>
	> root;

	root.at<Id("bar"), Id("save")>().w = 11.0f;
	root.at<Id("bar"), Id("cancel")>().w = 22.0f;
	root.at<Id("status")>().w = 33.0f;

	CHECK_FLOAT(root.child<0>().child<0>().w, 11.0f);
	CHECK_FLOAT(root.child<0>().child<1>().w, 22.0f);
	CHECK_FLOAT(root.child<1>().w, 33.0f);

	// VBox
	VBox<Id("box"), Probe<Id("a")>, Probe<Id("b")>> box;
	box.spacing = 8.0f;
	box.marginX = 0.0f;
	box.marginY = 0.0f;
	box.at<Id("a")>().w = 10.0f;
	box.at<Id("a")>().h = 20.0f;
	box.at<Id("b")>().w = 30.0f;
	box.at<Id("b")>().h = 40.0f;
	const Size vs = box.hint(Size{100.0f, 100.0f});
	CHECK_FLOAT(vs.w, 30.0f);
	CHECK_FLOAT(vs.h, 68.0f);

	// HBox
	HBox<Id("row"), Probe<Id("a")>, Probe<Id("b")>> row;
	row.spacing = 8.0f;
	row.marginX = 0.0f;
	row.marginY = 0.0f;
	row.at<Id("a")>().w = 10.0f;
	row.at<Id("a")>().h = 20.0f;
	row.at<Id("b")>().w = 30.0f;
	row.at<Id("b")>().h = 40.0f;
	const Size hs = row.hint(Size{100.0f, 100.0f});
	CHECK_FLOAT(hs.w, 48.0f);
	CHECK_FLOAT(hs.h, 40.0f);

	// Grid
	Grid<Id("grid"), Probe<Id("g0")>, Probe<Id("g1")>, Probe<Id("g2")>, Probe<Id("g3")>> grid;
	grid.cols = 2;
	grid.spacing = 8.0f;
	grid.marginX = 0.0f;
	grid.marginY = 0.0f;
	for (std::size_t i = 0; i < 4; ++i) {
		auto set = [&](auto& p) { p.w = 10.0f; p.h = 20.0f; };
		if (i == 0) set(grid.child<0>());
		if (i == 1) set(grid.child<1>());
		if (i == 2) set(grid.child<2>());
		if (i == 3) set(grid.child<3>());
	}
	const Size gs = grid.hint(Size{100.0f, 100.0f});
	CHECK_FLOAT(gs.w, 28.0f);   // 10 + 8 + 10
	CHECK_FLOAT(gs.h, 48.0f);   // 20 + 8 + 20

	// Wrap：可用宽 28 时两项同一行
	Wrap<Id("wrap"), Probe<Id("w0")>, Probe<Id("w1")>> wrap;
	wrap.spacing = 8.0f;
	wrap.marginX = 0.0f;
	wrap.marginY = 0.0f;
	wrap.child<0>().w = 10.0f;
	wrap.child<0>().h = 20.0f;
	wrap.child<1>().w = 10.0f;
	wrap.child<1>().h = 20.0f;
	const Size ws1 = wrap.hint(Size{28.0f, 100.0f});
	CHECK_FLOAT(ws1.w, 28.0f);
	CHECK_FLOAT(ws1.h, 20.0f);
	// 可用宽 20 时换行
	const Size ws2 = wrap.hint(Size{20.0f, 100.0f});
	CHECK_FLOAT(ws2.w, 10.0f);
	CHECK_FLOAT(ws2.h, 48.0f);

	// 动态层 opt-in
	VBox<Id("dynroot"), RuntimeList<Id("dynlist")>> dynRoot;
	dynRoot.at<Id("dynlist")>().idname = "dynlist";
	CHECK(dynRoot.at<Id("dynlist")>().idname == "dynlist");

	RuntimeList<> list;
	list.spacing = 8.0f;
	list.marginX = 0.0f;
	list.marginY = 0.0f;

	auto p1 = std::make_unique<Probe<Id("item_a")>>();
	p1->w = 12.0f;
	p1->h = 24.0f;
	AnyWidget any1 = AnyWidget::own(std::move(p1));
	any1.idname = "item_a";
	list.add(std::move(any1));

	auto p2 = std::make_unique<Probe<Id("item_b")>>();
	p2->w = 40.0f;
	p2->h = 50.0f;
	AnyWidget any2 = AnyWidget::own(std::move(p2));
	any2.idname = "item_b";
	list.add(std::move(any2));

	CHECK(list.size() == 2u);
	CHECK(list.findById("item_b") != nullptr);
	CHECK(list.findById("missing") == nullptr);

	const Size ds = list.hint(Size{100.0f, 100.0f});
	CHECK_FLOAT(ds.w, 40.0f);
	CHECK_FLOAT(ds.h, 82.0f);

	// 渲染期 add 延迟
	RuntimeList<> late;
	late.spacing = 0.0f;
	late.marginX = 0.0f;
	late.marginY = 0.0f;
	auto p3 = std::make_unique<Probe<Id("late")>>();
	p3->w = 5.0f;
	p3->h = 6.0f;
	AnyWidget any3 = AnyWidget::own(std::move(p3));
	any3.idname = "late";

	++g_RenderDepth;
	late.add(std::move(any3));
	--g_RenderDepth;
	CHECK(late.size() == 1u);
	CHECK(late.findById("late") == nullptr);

	const Size ls = late.hint(Size{50.0f, 50.0f});
	CHECK_FLOAT(ls.h, 6.0f);
	CHECK(late.findById("late") != nullptr);

	late.removeAt(0);
	CHECK(late.size() == 0u);
	CHECK(late.findById("late") == nullptr);

	// 动态页签
	RuntimeTabs<> tabs;
	auto t1 = std::make_unique<Probe<Id("tab_a")>>();
	t1->w = 10.0f;
	t1->h = 20.0f;
	AnyWidget ta = AnyWidget::own(std::move(t1));
	ta.idname = "tab_a";
	tabs.add("Tab A", std::move(ta));

	auto t2 = std::make_unique<Probe<Id("tab_b")>>();
	t2->w = 30.0f;
	t2->h = 40.0f;
	AnyWidget tb = AnyWidget::own(std::move(t2));
	tb.idname = "tab_b";
	tabs.add("Tab B", std::move(tb));

	CHECK(tabs.size() == 2u);
	CHECK(tabs.findById("tab_b") != nullptr);
	CHECK(tabs.findById("Tab A") != nullptr);
	const Size ts = tabs.hint(Size{100.0f, 100.0f});
	CHECK_FLOAT(ts.w, 30.0f);
	CHECK_FLOAT(ts.h, 40.0f);

	// 动态网格
	RuntimeGrid<> dgrid;
	dgrid.cols = 2;
	dgrid.spacing = 8.0f;
	dgrid.marginX = 0.0f;
	dgrid.marginY = 0.0f;
	auto g1 = std::make_unique<Probe<Id("g_a")>>();
	g1->w = 10.0f; g1->h = 20.0f;
	AnyWidget ga = AnyWidget::own(std::move(g1));
	ga.idname = "g_a";
	dgrid.add(std::move(ga));
	auto g2 = std::make_unique<Probe<Id("g_b")>>();
	g2->w = 30.0f; g2->h = 40.0f;
	AnyWidget gb = AnyWidget::own(std::move(g2));
	gb.idname = "g_b";
	dgrid.add(std::move(gb));
	auto g3 = std::make_unique<Probe<Id("g_c")>>();
	g3->w = 50.0f; g3->h = 60.0f;
	AnyWidget gc = AnyWidget::own(std::move(g3));
	gc.idname = "g_c";
	dgrid.add(std::move(gc));

	CHECK(dgrid.size() == 3u);
	CHECK(dgrid.findById("g_c") != nullptr);
	const Size dgs = dgrid.hint(Size{200.0f, 200.0f});
	CHECK_FLOAT(dgs.w, 108.0f);   // 50*2 + 8
	CHECK_FLOAT(dgs.h, 108.0f);   // 40 + 8 + 60

	std::puts("wg2_smoke OK");
	return 0;
}
