// 诊断：wg 体系 UI 重叠检测——渲染 gallery 同款布局，逐子记录实际 item 矩形，
// 帧 60 时两两碰撞检测，自动关窗打印。退出码 0 = 无重叠。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
struct RectRec { const char* name; float x, y, w, h; };
// 记录容器：逐子定位渲染，渲染后读 GetItemRect 记录
struct RecBoxImpl {
	std::vector<RectRec>* recs = nullptr;
	void renderImpl(Mk::wg::Node& n) {
		for(auto* c : n.children) {
			if(!c) continue;
			Mk::wg::detail::drawChildAt(n, *c);
			const ImVec2 mn = ImGui::GetItemRectMin();
			const ImVec2 mx = ImGui::GetItemRectMax();
			recs->push_back({c->idname.c_str(), mn.x, mn.y, mx.x - mn.x, mx.y - mn.y});
		}
	}
	Mk::wg::Size measureImpl(Mk::wg::Node& n, const Mk::wg::Size& avail) {
		Mk::wg::Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Mk::wg::Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h += s.h;
		}
		if(n.children.size() > 1) total.h += 8.0f * (float)(n.children.size() - 1);
		return total;
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Mk::wg::Size s = c->measure(Mk::wg::Size{r.w, r.h - (y - r.y)});
			c->arrange(Mk::wg::Rect{r.x, y, s.w, s.h});
			y += s.h + 8.0f;
		}
	}
};
using RecBox = Mk::wg::Control<RecBoxImpl, Mk::wg::DefaultStyle>;
}

int main() {
	Mk::MainWindow<> win("wg 重叠诊断", 1200, 900);
	int frames = 0;
	std::vector<RectRec> recs;
	static float volume = 0.5f;
	static bool mute = false;
	static int comboSel = 0, radioSel = 1, listSel = 0, iVal = 42;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};
	static constexpr const char* kCols[] = {"名称", "数值"};
	std::vector<std::vector<std::string>> rows{{"甲", "1"}, {"乙", "2"}};

	// —— gallery 同款布局：V 列 + 各类控件 ——
	Mk::wg::SeparatorText st1;  st1.self.name = "输入区";
	Mk::wg::InputText<> inT;    inT.self.name = "文本";  inT.self.setName("hello");
	Mk::wg::SliderF slF;        slF.self.name = "滑块f";  slF.self.value = &volume;
	Mk::wg::SliderInt slI;      slI.self.name = "滑块i";  slI.self.value = &iVal;
	Mk::wg::CheckBox cb;        cb.self.name = "开关";    cb.self.checked = &mute;
	Mk::wg::RadioGroup rg;      rg.self.items = kItems;  rg.self.count = 3;  rg.self.current = &radioSel;
	Mk::wg::Combo co;           co.self.name = "下拉";    co.self.current = &comboSel;  co.self.items = kItems;  co.self.count = 3;
	Mk::wg::ListBox lb;         lb.self.name = "列表";    lb.self.current = &listSel;    lb.self.items = kItems;  lb.self.count = 3;
	Mk::wg::ProgressBar pb;     pb.self.fraction = &volume;
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	Mk::wg::ColorEdit3 ce;      ce.self.name = "RGB";     ce.self.color = &col3;
	Mk::wg::DragF df;           df.self.name = "拖拽";    df.self.value = &volume;
	Mk::wg::CheckboxFlags cf;   cf.self.name = "位A";     cf.self.flags = &iVal;  cf.self.mask = 1;

	// H 行（两列）
	Mk::wg::Label hL1, hL2;     hL1.self.setName("H 左");  hL2.self.setName("H 右");
	Mk::wg::Layout hRow;
	hRow.self.mode = Mk::wg::LayoutMode::H;
	hRow.node.add(hL1.node);
	hRow.node.add(hL2.node);

	// SplitH
	Mk::wg::Label sp1, sp2, sp3;
	sp1.self.setName("fixed 160");  sp2.self.setName("stretch 1");  sp3.self.setName("stretch 2");
	Mk::wg::setFixed(sp1.node, 160.0f);
	Mk::wg::setStretch(sp2.node, 1.0f);
	Mk::wg::setStretch(sp3.node, 2.0f);
	Mk::wg::Layout split;
	split.self.mode = Mk::wg::LayoutMode::SplitH;
	split.node.add(sp1.node);
	split.node.add(sp2.node);
	split.node.add(sp3.node);

	// Wrap / Grid
	Mk::wg::Label w1, w2, w3, w4;
	w1.self.setName("流式一");  w2.self.setName("流式二");  w3.self.setName("流式三");  w4.self.setName("流式四");
	Mk::wg::Layout wrap;
	wrap.self.mode = Mk::wg::LayoutMode::Wrap;
	wrap.node.add(w1.node);  wrap.node.add(w2.node);  wrap.node.add(w3.node);  wrap.node.add(w4.node);
	Mk::wg::Label g1, g2, g3, g4;
	g1.self.setName("格0-0");  g2.self.setName("格0-1");  g3.self.setName("格0-2");  g4.self.setName("格1-0");
	Mk::wg::Layout grid;
	grid.self.mode = Mk::wg::LayoutMode::Grid;
	grid.self.cols = 3;
	grid.node.add(g1.node);  grid.node.add(g2.node);  grid.node.add(g3.node);  grid.node.add(g4.node);

	// SubWindow / Collapsing / TabBar
	Mk::wg::Label subL;
	subL.self.setName("子窗口内容");
	Mk::wg::SubWindow<> sub;
	sub.node.idname = "sub";
	sub.self.height = 60.0f;
	sub.node.add(subL.node);
	Mk::wg::Label collL;
	collL.self.setName("折叠内容");
	Mk::wg::Collapsing coll;
	coll.self.name = "折叠区";
	coll.self.flags = ImGuiTreeNodeFlags_DefaultOpen;
	coll.node.add(collL.node);
	Mk::wg::Label tab1L, tab2L;
	tab1L.self.setName("第一页");  tab2L.self.setName("第二页");
	Mk::wg::Tab<> t1, t2;
	t1.node.idname = "页一";  t1.node.add(tab1L.node);
	t2.node.idname = "页二";  t2.node.add(tab2L.node);
	Mk::wg::TabBar<> tabs;
	tabs.node.idname = "tabs";
	tabs.node.add(t1.node);  tabs.node.add(t2.node);

	// —— 记录容器（宿主窗口空间直接子项碰撞检测）——
	RecBox box;
	box.self.recs = &recs;
	for(Mk::wg::Node* nd : {&st1.node, &inT.node, &slF.node, &slI.node, &cb.node,
			&rg.node, &co.node, &lb.node, &pb.node, &ce.node, &df.node, &cf.node,
			&hRow.node, &split.node, &wrap.node, &grid.node, &sub.node, &coll.node, &tabs.node}) {
		box.node.add(*nd);
	}

	win.setUI([&] {
		++frames;
		recs.clear();
		box.layout(Mk::wg::Rect{20, 40, 900, 800});
		if(frames >= 120) glfwSetWindowShouldClose(win.getHandle(), true);
	});

	Mk::mkui<Mk::window>::run();

	// —— 碰撞检测（帧 120 的最终记录）——
	int overlap = 0;
	for(size_t i = 0; i < recs.size(); ++i) {
		for(size_t j = i + 1; j < recs.size(); ++j) {
			const RectRec& a = recs[i];
			const RectRec& b = recs[j];
			const bool hit = a.x < b.x + b.w && a.x + a.w > b.x
				&& a.y < b.y + b.h && a.y + a.h > b.y;
			if(hit) {
				std::printf("OVERLAP: %s(%.1f,%.1f %.1fx%.1f) <-> %s(%.1f,%.1f %.1fx%.1f)\n",
					a.name, a.x, a.y, a.w, a.h, b.name, b.x, b.y, b.w, b.h);
				++overlap;
			}
		}
	}
	for(size_t i = 0; i < recs.size(); ++i) {
		const RectRec& r = recs[i];
		std::printf("  [%2zu] %-12s (%.1f,%.1f %.1fx%.1f)\n", i, r.name, r.x, r.y, r.w, r.h);
	}
	std::printf("diag: items=%zu overlaps=%d\n", recs.size(), overlap);
	const bool pass = overlap == 0;
	std::printf("%s\n", pass ? "PASS" : "FAIL");
	return pass ? 0 : 1;
}

