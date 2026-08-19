#include <gtest/gtest.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <string>

// wg 纯逻辑单元测试（不依赖 ImGui 帧）：Node 树语义、findById、布局求解数学、ItemRole 默认态。

namespace {

// 探针原子：固定尺寸、可记录渲染次数（tuple 子控件要求默认构造 + 指针成员）
struct ProbeImpl {
	float w = 0, h = 0;
	int* renders = nullptr;
	void renderImpl(Mk::wg::Node&) { if(renders) ++(*renders); }
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) { return Mk::wg::Size{w, h}; }
	Mk::wg::Size sizeHintImpl(Mk::wg::Node&) { return Mk::wg::Size{w, h}; }
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using Probe = Mk::wg::Control<ProbeImpl, Mk::wg::DefaultStyle>;

} // namespace

// ============ Node 树语义 ============

TEST(WgNode, BindFillsBehaviorTable) {
	Probe p(10.0f, 5.0f, nullptr);
	EXPECT_NE(p.node.owner, nullptr);
	const Mk::wg::Size h = p.node.hint();
	EXPECT_FLOAT_EQ(h.w, 10.0f);
	EXPECT_FLOAT_EQ(h.h, 5.0f);
}

TEST(WgNode, AddSetsParentAndChildren) {
	Probe a(1, 1, nullptr), b(1, 1, nullptr);
	Mk::wg::VLayout root;
	root.addWidget(a.node);
	root.addWidget(b.node);
	EXPECT_EQ(root.node.children.size(), 2u);
	EXPECT_EQ(a.node.parent, &root.node);
	EXPECT_EQ(b.node.parent, &root.node);
}

TEST(WgNode, RemoveIsTombstoneAndClearsParent) {
	Probe a(1, 1, nullptr), b(1, 1, nullptr);
	Mk::wg::VLayout root;
	root.addWidget(a.node);
	root.addWidget(b.node);
	root.node.remove(b.node);
	EXPECT_EQ(root.node.children[1], nullptr);   // 墓碑
	EXPECT_EQ(b.node.parent, nullptr);
}

TEST(WgNode, FindByIdRecursesAndCompacts) {
	Probe mid(1, 1, nullptr), leaf(1, 1, nullptr);
	Mk::wg::VLayout root;
	mid.node.idname = "mid";
	leaf.node.idname = "leaf";
	root.node.add(mid.node);
	root.node.add(leaf.node);
	root.node.remove(leaf.node);                          // 墓碑
	EXPECT_EQ(root.node.findByIdname("leaf"), nullptr);  // 墓碑不可达
	EXPECT_EQ(root.node.findByIdname("mid"), &mid.node); // 压缩后可找到
	EXPECT_EQ(root.node.findByIdname("nope"), nullptr);
}

TEST(WgNode, ClearNullsAllSlotsAndParents) {
	Probe a(1, 1, nullptr), b(1, 1, nullptr);
	Mk::wg::VLayout root;
	root.addWidget(a.node);
	root.addWidget(b.node);
	root.node.clear();
	EXPECT_EQ(root.node.children[0], nullptr);
	EXPECT_EQ(root.node.children[1], nullptr);
	EXPECT_EQ(a.node.parent, nullptr);
	EXPECT_EQ(b.node.parent, nullptr);
}

TEST(WgNode, ItemRoleDefaultsToNoRole) {
	Probe p(1, 1, nullptr);
	EXPECT_FALSE(p.node.role.hasRole);
	EXPECT_FLOAT_EQ(p.node.role.stretch, 0.0f);
}

// ============ 布局求解数学（Qt 式：fill + 富余均分/stretch 权重）============

TEST(WgLayout, VBoxMeasureAndArrange) {
	Mk::wg::VLayout box;
	box.setSpacing(8.0f); box.setContentsMargins(0.0f, 0.0f);   // 显式定值：纯逻辑断言不依赖样式
	Probe a(40.0f, 30.0f, nullptr), b(60.0f, 40.0f, nullptr);
	box.addWidget(a.node);
	box.addWidget(b.node);

	const Mk::wg::Size m = box.node.measure(Mk::wg::Size{500, 500});
	EXPECT_FLOAT_EQ(m.w, 500.0f);   // 桥：宽取容器可用
	EXPECT_FLOAT_EQ(m.h, 78.0f);    // 30+40+8

	box.node.arrange(Mk::wg::Rect{10, 10, 500, 500});
	EXPECT_FLOAT_EQ(a.node.bounds.x, 10.0f);
	EXPECT_FLOAT_EQ(a.node.bounds.y, 10.0f);
	EXPECT_FLOAT_EQ(b.node.bounds.y, 259.0f);   // a 富余后 241 + 8
}

TEST(WgLayout, HBoxMeasureAndArrange) {
	Mk::wg::HLayout box;
	box.setSpacing(8.0f); box.setContentsMargins(0.0f, 0.0f);
	Probe a(30.0f, 10.0f, nullptr), b(50.0f, 10.0f, nullptr);
	box.addWidget(a.node);
	box.addWidget(b.node);

	const Mk::wg::Size m = box.node.measure(Mk::wg::Size{500, 500});
	EXPECT_FLOAT_EQ(m.w, 500.0f);
	EXPECT_FLOAT_EQ(m.h, 10.0f);

	box.node.arrange(Mk::wg::Rect{0, 0, 500, 500});
	EXPECT_FLOAT_EQ(b.node.bounds.x, 244.0f);   // a 富余后 236 + 8
}

TEST(WgLayout, OverlayOverlapsChildren) {
	Mk::wg::OverlayLayout g;
	g.setContentsMargins(0.0f, 0.0f);
	Probe a(40.0f, 30.0f, nullptr), b(60.0f, 40.0f, nullptr);
	g.addWidget(a.node);
	g.addWidget(b.node);

	const Mk::wg::Size m = g.node.measure(Mk::wg::Size{500, 500});
	EXPECT_FLOAT_EQ(m.w, 500.0f);
	EXPECT_FLOAT_EQ(m.h, 40.0f);

	g.node.arrange(Mk::wg::Rect{5, 5, 200, 100});
	EXPECT_FLOAT_EQ(a.node.bounds.x, 5.0f);
	EXPECT_FLOAT_EQ(b.node.bounds.x, 5.0f);
}

TEST(WgLayout, VBoxDynamicAddRearranges) {
	Mk::wg::VLayout box;
	box.setSpacing(8.0f); box.setContentsMargins(0.0f, 0.0f);
	Probe a(40.0f, 30.0f, nullptr), b(60.0f, 40.0f, nullptr);
	Probe dyn(30.0f, 20.0f, nullptr);
	box.addWidget(a.node);
	box.addWidget(b.node);
	box.addWidget(dyn.node);

	box.node.arrange(Mk::wg::Rect{0, 0, 500, 500});
	EXPECT_GT(dyn.node.bounds.y, b.node.bounds.y + 39.0f);   // 富余均分后 dyn 在 b 之下
	EXPECT_EQ(dyn.node.parent, &box.node);
}

TEST(WgLayout, WrapMeasuresAndArranges) {
	Mk::wg::WrapLayout w;
	w.setSpacing(8.0f); w.setContentsMargins(0.0f, 0.0f);
	Probe a(40.0f, 30.0f, nullptr), b(60.0f, 40.0f, nullptr), c(30.0f, 20.0f, nullptr);
	w.addWidget(a.node);
	w.addWidget(b.node);
	w.addWidget(c.node);

	const Mk::wg::Size m = w.node.measure(Mk::wg::Size{100, 100});
	EXPECT_FLOAT_EQ(m.w, 100.0f);   // 桥：宽取容器
	EXPECT_FLOAT_EQ(m.h, 40.0f);    // 无界单行高 = 行内最大

	w.node.arrange(Mk::wg::Rect{0, 0, 100, 100});
	EXPECT_FLOAT_EQ(a.node.bounds.x, 0.0f);
	EXPECT_FLOAT_EQ(a.node.bounds.y, 0.0f);
	EXPECT_FLOAT_EQ(b.node.bounds.x, 0.0f);
	EXPECT_FLOAT_EQ(b.node.bounds.y, 38.0f);
	EXPECT_FLOAT_EQ(c.node.bounds.x, 68.0f);
	EXPECT_FLOAT_EQ(c.node.bounds.y, 38.0f);
}

TEST(WgLayout, WrapSingleRowWhenWide) {
	Mk::wg::WrapLayout w;
	w.setSpacing(8.0f); w.setContentsMargins(0.0f, 0.0f);
	Probe a(40.0f, 30.0f, nullptr), b(40.0f, 10.0f, nullptr);
	w.addWidget(a.node);
	w.addWidget(b.node);

	const Mk::wg::Size m = w.node.measure(Mk::wg::Size{300, 100});
	EXPECT_FLOAT_EQ(m.w, 300.0f);
	EXPECT_FLOAT_EQ(m.h, 30.0f);
}

TEST(WgLayout, GridMeasuresAndArranges) {
	Mk::wg::GridLayout g;
	g.setSpacing(8.0f); g.setContentsMargins(0.0f, 0.0f);
	g.setCols(3);
	Probe p[5];
	for(int i = 0; i < 5; ++i) {
		p[i].self = ProbeImpl{30.0f, 20.0f, nullptr};
		g.addWidget(p[i].node);
	}

	const Mk::wg::Size m = g.node.measure(Mk::wg::Size{300, 200});
	EXPECT_FLOAT_EQ(m.w, 300.0f);
	EXPECT_FLOAT_EQ(m.h, 48.0f);    // 20 + 8 + 20

	g.node.arrange(Mk::wg::Rect{0, 0, 300, 200});
	const float cellW = (300.0f - 16.0f) / 3.0f;
	EXPECT_FLOAT_EQ(p[0].node.bounds.x, 0.0f);
	EXPECT_FLOAT_EQ(p[1].node.bounds.x, cellW + 8.0f);
	EXPECT_FLOAT_EQ(p[2].node.bounds.x, (cellW + 8.0f) * 2.0f);
	EXPECT_FLOAT_EQ(p[3].node.bounds.y, 28.0f);
	EXPECT_FLOAT_EQ(p[4].node.bounds.x, cellW + 8.0f);
	EXPECT_FLOAT_EQ(p[4].node.bounds.y, 28.0f);
	EXPECT_FLOAT_EQ(p[0].node.bounds.w, cellW);
}

TEST(WgLayout, NestedContainerChain) {
	// VLayout > HLayout > VLayout 嵌套：Qt 式求解（fill + 富余均分）
	Mk::wg::VLayout root, col;
	root.setSpacing(8.0f); root.setContentsMargins(0.0f, 0.0f);
	col.setSpacing(8.0f); col.setContentsMargins(0.0f, 0.0f);
	Probe a(60.0f, 30.0f, nullptr), b(40.0f, 20.0f, nullptr);
	Probe c(50.0f, 10.0f, nullptr), d(30.0f, 10.0f, nullptr);
	col.addWidget(a.node);
	col.addWidget(b.node);
	Mk::wg::HLayout row;
	row.setSpacing(8.0f); row.setContentsMargins(0.0f, 0.0f);
	row.addWidget(col.node);
	row.addWidget(c.node);
	root.addWidget(row.node);
	root.addWidget(d.node);

	const Mk::wg::Size m = root.node.measure(Mk::wg::Size{300, 200});
	EXPECT_FLOAT_EQ(m.w, 300.0f);
	EXPECT_FLOAT_EQ(m.h, 76.0f);

	root.node.arrange(Mk::wg::Rect{20, 20, 300, 200});
	EXPECT_FLOAT_EQ(a.node.bounds.y, 20.0f);
	EXPECT_FLOAT_EQ(b.node.bounds.y, 89.0f);
	EXPECT_FLOAT_EQ(c.node.bounds.x, 179.0f);
	EXPECT_FLOAT_EQ(d.node.bounds.y, 148.0f);
}
