#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cmath>
#include <cstdio>

namespace {

struct PosProbeImpl {
	float w = 0, h = 0;
	ImVec2* lastPos = nullptr;
	int* renders = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(renders) ++(*renders);
		if(lastPos) *lastPos = ImGui::GetCursorPos();
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) { return Mk::wg::Size{w, h}; }
	Mk::wg::Size sizeHintImpl(Mk::wg::Node&) { return Mk::wg::Size{w, h}; }
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using PosProbe = Mk::wg::Control<PosProbeImpl, Mk::wg::DefaultStyle>;

} // namespace

// test21：Qt 式布局嵌套全验证（VLayout/HLayout/SplitH/SplitV/Wrap/Grid 嵌套 + 容器嵌套 + origin 传播）。
// 全程每帧断言：每个探针渲染时 GetCursorPos() == node.origin。
int main() {
	Mk::MainWindow<> win("wg 嵌套布局验证", 1280, 1100);

	int frames = 0;
	int renders = 0;
	ImVec2 last[32];

	// 组 A. 布局嵌套：VLayout > HLayout > VLayout
	PosProbe a(60, 30, &last[0], &renders), b(40, 20, &last[1], &renders);
	PosProbe c(50, 10, &last[2], &renders), d(30, 10, &last[3], &renders);
	Mk::wg::VLayout innerV;
	innerV.addWidget(a.node);
	innerV.addWidget(b.node);
	Mk::wg::HLayout midH;
	midH.addWidget(innerV.node);
	midH.addWidget(c.node);
	Mk::wg::VLayout nestBox;
	nestBox.addWidget(midH.node);
	nestBox.addWidget(d.node);

	// 组 B. 分格嵌套：SplitV > SplitH > V
	PosProbe top(100, 20, &last[4], &renders), cellA(80, 20, &last[5], &renders);
	PosProbe p(100, 30, &last[6], &renders), q(100, 30, &last[7], &renders);
	PosProbe cellC(60, 20, &last[8], &renders);
	top.setFixedHeight(60.0f);
	cellA.setFixedWidth(120.0f);
	Mk::wg::VLayout vInner;
	vInner.addWidget(p.node);
	vInner.addWidget(q.node);
	Mk::wg::SplitHLayout splitH;
	splitH.addWidget(cellA.node);
	splitH.addWidget(vInner.node, 1.0f);
	splitH.addWidget(cellC.node, 1.0f);
	Mk::wg::SplitVLayout splitNest;
	splitNest.addWidget(top.node);
	splitNest.addWidget(splitH.node, 1.0f);

	// 组 C. 窗口嵌套：SubWindow > SubWindow
	PosProbe probeA(380, 40, &last[9], &renders);
	PosProbe probeB(300, 30, &last[10], &renders), probeC(50, 20, &last[11], &renders);
	Mk::wg::SubWindow<> inner;
	inner.node.idname = "nestInner";
	inner.self.height = 120.0f;
	inner.node.add(probeB.node);
	inner.node.add(probeC.node);
	Mk::wg::SubWindow<> winNest;
	winNest.node.idname = "nestOuter";
	winNest.self.width = 400.0f;
	winNest.self.height = 300.0f;
	winNest.node.add(probeA.node);
	winNest.node.add(inner.node);

	// 组 D. 分格嵌窗口：SplitH in SubWindow
	PosProbe dA(60, 20, &last[12], &renders), dB(100, 20, &last[13], &renders);
	dA.setFixedWidth(80.0f);
	Mk::wg::SplitHLayout swSplit;
	swSplit.addWidget(dA.node);
	swSplit.addWidget(dB.node, 1.0f);
	Mk::wg::SubWindow<> splitInWin;
	splitInWin.node.idname = "swsplit";
	splitInWin.self.width = 300.0f;
	splitInWin.self.height = 200.0f;
	splitInWin.node.add(swSplit.node);

	// 组 E. Wrap 换行
	PosProbe e1(40, 30, &last[14], &renders), e2(60, 40, &last[15], &renders), e3(30, 20, &last[16], &renders);
	Mk::wg::WrapLayout wrapRow;
	wrapRow.addWidget(e1.node);
	wrapRow.addWidget(e2.node);
	wrapRow.addWidget(e3.node);

	// 组 F. Grid 网格（3 列 5 格 → 2 行）
	PosProbe g[5];
	for(int i = 0; i < 5; ++i) {
		g[i].self = PosProbeImpl{30, 20, &last[17 + i], &renders};
	}
	Mk::wg::GridLayout grid;
	grid.setCols(3);
	for(int i = 0; i < 5; ++i) grid.addWidget(g[i].node);

	// 组 G. Panel 嵌套 VLayout
	PosProbe ga(100, 30, &last[22], &renders), gb(100, 30, &last[23], &renders);
	Mk::wg::VLayout gv;
	gv.addWidget(ga.node);
	gv.addWidget(gb.node);
	Mk::wg::Panel<> panelTree;
	panelTree.node.idname = "嵌套面板";
	panelTree.node.add(gv.node);

	// 组 H. Collapsing 嵌套 VLayout
	PosProbe ha(80, 20, &last[24], &renders), hb(80, 20, &last[25], &renders);
	Mk::wg::VLayout collapsBody;
	collapsBody.addWidget(ha.node);
	collapsBody.addWidget(hb.node);
	Mk::wg::Collapsing collaps;
	collaps.self.name = "折叠容器（默认展开）";
	collaps.self.flags = ImGuiTreeNodeFlags_DefaultOpen;
	collaps.node.add(collapsBody.node);

	// 组 I. TabBar 页签嵌套
	PosProbe tabProbe(80, 20, &last[26], &renders);
	Mk::wg::Tab<> tab1;
	tab1.node.idname = "页一";
	tab1.node.add(tabProbe.node);
	Mk::wg::TabBar<> tabs;
	tabs.node.idname = "t21";
	tabs.node.add(tab1.node);

	// 组 J. Popup 内容（帧 15 打开）
	PosProbe popProbe(100, 30, &last[27], &renders);
	bool popupOpen = false;
	Mk::wg::Popup<> popupTree;
	popupTree.node.idname = "p21";
	popupTree.self.open = &popupOpen;
	popupTree.node.add(popProbe.node);

	win.setUI([&] {
		++frames;
		if(frames == 15) popupOpen = true;

		nestBox.layout(Mk::wg::Rect{20, 20, 300, 200});
		splitNest.layout(Mk::wg::Rect{20, 140, 500, 300});
		winNest.layout(Mk::wg::Rect{20, 460, 400, 300});
		splitInWin.layout(Mk::wg::Rect{440, 460, 300, 200});
		wrapRow.layout(Mk::wg::Rect{20, 780, 300, 100});
		grid.layout(Mk::wg::Rect{340, 780, 300, 100});
		panelTree.layout(Mk::wg::Rect{0, 0, 200, 100});
		collaps.layout(Mk::wg::Rect{20, 900, 300, 100});
		tabs.layout(Mk::wg::Rect{440, 780, 300, 80});
		popupTree.layout(Mk::wg::Rect{0, 0, 200, 80});

		ImGui::Text("frames=%d", frames);
		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const float eps = 1.0f;
	int failures = 0;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };
	const auto chk = [&](const char* name, float actual, float expect) {
		if(!about(actual, expect)) {
			std::printf("FAIL %s: actual=%.2f expect=%.2f\n", name, actual, expect);
			++failures;
		}
	};
	const auto chkInv = [&](const char* name, Mk::wg::Node& n, const ImVec2& lp) {
		chk(name, n.origin.x, lp.x);
		chk(name, n.origin.y, lp.y);
	};

	// 组 A（布局嵌套；nestBox fill → 300×200；Preferred 无 stretch 时富余均分）
	chk("A.root.w", nestBox.node.bounds.w, 300.0f);
	chk("A.root.h", nestBox.node.bounds.h, 200.0f);
	chk("A.a.x", a.node.bounds.x, 20.0f);
	chk("A.a.y", a.node.bounds.y, 20.0f);
	chk("A.b.y", b.node.bounds.y, 89.0f);   // midH 富余后高 120 → innerV 内 a=61
	chk("A.c.x", c.node.bounds.x, 179.0f);   // 20 + (60+91) + 8
	chk("A.d.y", d.node.bounds.y, 148.0f);   // 20 + (58+62) + 8
	chkInv("A.a.inv", a.node, last[0]);
	chkInv("A.b.inv", b.node, last[1]);
	chkInv("A.c.inv", c.node, last[2]);
	chkInv("A.d.inv", d.node, last[3]);

	// 组 B（分格嵌套：top 固定 60；splitH stretch 吃剩余 232；splitH 内 stretch 1:1 分富余）
	chk("B.top.h", top.node.bounds.h, 60.0f);
	chk("B.hs.y", splitH.node.bounds.y, 208.0f);
	chk("B.A.w", cellA.node.bounds.w, 120.0f);
	chk("B.B.x", vInner.node.bounds.x, 148.0f);
	chk("B.C.x", cellC.node.bounds.x, 358.0f);   // 148 + 202 + 8
	chk("B.Bw", vInner.node.bounds.w, 202.0f);
	chk("B.q.y", q.node.bounds.y, 328.0f);       // 208 + 112 + 8
	chkInv("B.top.inv", top.node, last[4]);
	chkInv("B.A.inv", cellA.node, last[5]);
	chkInv("B.p.inv", p.node, last[6]);
	chkInv("B.q.inv", q.node, last[7]);
	chkInv("B.C.inv", cellC.node, last[8]);

	// 组 C（SubWindow 嵌套；子窗口局部坐标）
	chk("C.probeA.bx", probeA.node.bounds.x, 20.0f);
	chk("C.inner.by", inner.node.bounds.y, 500.0f);
	chk("C.probeC.by", probeC.node.bounds.y, 530.0f);
	chk("C.probeA.ox", probeA.node.origin.x, 0.0f);
	chk("C.probeA.oy", probeA.node.origin.y, 0.0f);
	chk("C.inner.oy", inner.node.origin.y, 40.0f);
	chk("C.probeB.oy", probeB.node.origin.y, 0.0f);
	chk("C.probeC.oy", probeC.node.origin.y, 30.0f);
	chkInv("C.A.inv", probeA.node, last[9]);
	chkInv("C.B.inv", probeB.node, last[10]);
	chkInv("C.C.inv", probeC.node, last[11]);

	// 组 D（SplitH 嵌 SubWindow）
	chk("D.cellA.w", dA.node.bounds.w, 80.0f);
	chk("D.cellB.x", dB.node.bounds.x, 528.0f);
	chk("D.cellB.w", dB.node.bounds.w, 212.0f);
	chk("D.split.ox", swSplit.node.origin.x, 0.0f);
	chk("D.pA.ox", dA.node.origin.x, 0.0f);
	chk("D.pB.ox", dB.node.origin.x, 88.0f);
	chk("D.pB.oy", dB.node.origin.y, 0.0f);
	chkInv("D.pA.inv", dA.node, last[12]);
	chkInv("D.pB.inv", dB.node, last[13]);

	// 组 E（Wrap：100 宽时换行）
	wrapRow.node.arrange(Mk::wg::Rect{0, 0, 100, 100});
	chk("E.c0.x", e1.node.bounds.x, 0.0f);
	chk("E.c0.y", e1.node.bounds.y, 0.0f);
	chk("E.c1.y", e2.node.bounds.y, 38.0f);
	chk("E.c1.x", e2.node.bounds.x, 0.0f);
	chk("E.c2.x", e3.node.bounds.x, 68.0f);
	chk("E.c2.y", e3.node.bounds.y, 38.0f);

	// 组 F（Grid：3 列均分 94.667；2 行）
	chk("F.g.w", grid.node.bounds.w, 300.0f);
	chk("F.g.h", grid.node.bounds.h, 100.0f);   // 布局恒填满宿主（Qt 语义）
	chk("F.i1.x", g[1].node.bounds.x, 340.0f + 102.6667f);
	chk("F.i2.x", g[2].node.bounds.x, 340.0f + 205.3333f);
	chk("F.i3.y", g[3].node.bounds.y, 808.0f);
	chk("F.i4.x", g[4].node.bounds.x, 340.0f + 102.6667f);
	chk("F.cw", g[0].node.bounds.w, 94.6667f);
	chkInv("F.i0.inv", g[0].node, last[17]);
	chkInv("F.i4.inv", g[4].node, last[21]);

	// 组 G（Panel 嵌套 VLayout）
	chk("G.p1.oy", gb.node.origin.y, 38.0f);
	chkInv("G.p1.inv", gb.node, last[23]);

	// 组 H（Collapsing 嵌套 VLayout）
	chk("H.vbox.by", collapsBody.node.bounds.y, 932.04f);
	chk("H.p0.oy", ha.node.origin.y, 932.04f);
	chk("H.p1.oy", hb.node.origin.y, 960.04f);
	chkInv("H.p0.inv", ha.node, last[24]);
	chkInv("H.p1.inv", hb.node, last[25]);

	// 组 I/J
	chkInv("I.tab.inv", tabProbe.node, last[26]);
	chkInv("J.pop.inv", popProbe.node, last[27]);

	if(renders < 27 * 120) {
		std::printf("FAIL renders: %d < %d\n", renders, 27 * 120);
		++failures;
	}

	std::printf("test21 嵌套布局: renders=%d failures=%d\n", renders, failures);
	const bool pass = (failures == 0) && (renders >= 27 * 120);
	std::printf("%s\n", pass ? "PASS" : "FAIL");
	return pass ? 0 : 1;
}
