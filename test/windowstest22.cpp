#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cmath>
#include <cstdio>
#include <array>

namespace {

// 实测几何记录：布局 bounds vs ImGui 实际提交的 item 矩形
struct ItemRec {
	Mk::wg::Rect bounds;                    // 布局分配
	struct { float x, y, w, h; } actual;    // 实际 item 矩形
};

// 记录型 VBox：逐子定位渲染，渲染后立即读 GetItemRectMin/Size（该子最后一个 item）
struct RecBoxImpl {
	std::vector<ItemRec>* recs = nullptr;
	void renderImpl(Mk::wg::Node& n) {
		for(auto* c : n.children) {
			if(!c) continue;
			Mk::wg::detail::drawChildAt(n, *c);
			ItemRec r;
			r.bounds = c->bounds;
			const ImVec2 mn = ImGui::GetItemRectMin();
			const ImVec2 sz = ImGui::GetItemRectSize();
			r.actual = {mn.x, mn.y, sz.x, sz.y};
			recs->push_back(r);
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

} // namespace

// test22：原子控件实测几何——真实控件（非探针）在 VBox 列中渲染，
//   读回 ImGui 实际 item 矩形断言：
//   a) 位置精确：actual.min == bounds.xy（±1.5px）
//   b) 高度一致：bounds.h == actual.h（±1.5px）
//   c) 宽度受控：actual.w ≤ bounds.w + 2（输入类 frame+label 恰填满格子）
//   d) 无重叠：下一项 min.y ≥ 上一项 bottom − 1
// 120 帧自动关窗，PASS 退出码 0。
int main() {
	Mk::MainWindow<> win("wg 原子实测几何", 900, 1100);

	int frames = 0;
	std::vector<ItemRec> recs;

	static float f = 0.5f;
	static int i = 42;
	static double d = 3.14;
	static std::array<int, 2> iv{1, 2};
	static std::array<float, 2> fv{1.5f, 2.5f};
	static bool b1 = false, b2 = true, sel = false;
	static int radio = 1, comboSel = 0, listSel = 0, flags = 0b101;
	static std::array<float, 3> c3{0.9f, 0.3f, 0.2f};
	static std::vector<float> history;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};

	// 真实原子控件（运行时挂入 RecBox）
	Mk::wg::Label lbl;        lbl.self.setName("文本标签 Label");
	Mk::wg::Button btn;       btn.self.setName("按钮 Button");
	Mk::wg::SliderF sl;       sl.self.name = "滑块";        sl.self.value = &f;
	Mk::wg::InputText<64> it; it.self.name = "文本输入";    it.self.setName("hello");
	Mk::wg::InputInt ii;      ii.self.name = "整数";        ii.self.value = &i;
	Mk::wg::InputFloat fi;    fi.self.name = "浮点";        fi.self.value = &f;
	Mk::wg::InputDouble di;   di.self.name = "双精度";      di.self.value = &d;
	Mk::wg::InputInt2 i2;     i2.self.name = "ivec2";       i2.self.value = &iv;
	Mk::wg::InputFloat2 f2;   f2.self.name = "vec2";        f2.self.value = &fv;
	Mk::wg::DragF df;         df.self.name = "拖拽f";       df.self.value = &f;
	Mk::wg::DragI dint;       dint.self.name = "拖拽i";     dint.self.value = &i;
	Mk::wg::SliderInt si;     si.self.name = "滑块i";       si.self.value = &i;
	Mk::wg::SliderAngle sa;   sa.self.name = "角度";        sa.self.valueRad = &f;
	Mk::wg::CheckBox cb;      cb.self.name = "复选";        cb.self.checked = &b1;
	Mk::wg::RadioButton rb;   rb.self.name = "单选";        rb.self.active = &b2;
	Mk::wg::Combo co;         co.self.name = "下拉";        co.self.current = &comboSel;
	                           co.self.items = kItems;       co.self.count = 3;
	Mk::wg::Selectable st;    st.self.name = "可选行";      st.self.selected = &sel;
	Mk::wg::ProgressBar pb;   pb.self.fraction = &f;
	Mk::wg::LabelText lt;     lt.self.name = "属性";      lt.self.value = "值";
	Mk::wg::CheckboxFlags cf; cf.self.name = "位标志";      cf.self.flags = &flags; cf.self.mask = 1;
	Mk::wg::ColorEdit3 ce;    ce.self.name = "RGB";         ce.self.color = &c3;
	Mk::wg::PlotLines pl;     pl.self.name = "曲线";        pl.self.values = &history;

	RecBox box;
	box.self = RecBoxImpl{&recs};
	for(Mk::wg::Node* nd : {&lbl.node, &btn.node, &sl.node, &it.node, &ii.node,
			&fi.node, &di.node, &i2.node, &f2.node, &df.node, &dint.node, &si.node,
			&sa.node, &cb.node, &rb.node, &co.node, &st.node, &pb.node, &lt.node,
			&cf.node, &ce.node, &pl.node}) {
		box.node.add(*nd);
	}
	constexpr int kN = 22;
	const char* names[kN] = {"Label", "Button", "SliderF", "InputText", "InputInt",
		"InputFloat", "InputDouble", "InputInt2", "InputFloat2", "DragF", "DragI",
		"SliderInt", "SliderAngle", "CheckBox", "RadioButton", "Combo", "Selectable",
		"ProgressBar", "LabelText", "CheckboxFlags", "ColorEdit3", "PlotLines"};

	win.setUI([&] {
		++frames;
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 100) history.erase(history.begin());
		recs.clear();
		box.layout(Mk::wg::Rect{30, 30, 420, 1000});
		ImGui::Text("frames=%d", frames);
		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 断言 ——
	const float eps = 1.5f;
	int failures = 0;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };
	float prevBottom = -1e9;
	for(int k = 0; k < (int)recs.size() && k < kN; ++k) {
		const ItemRec& r = recs[k];
		const char* nm = names[k];
		const std::string base = nm;
		// a) 位置精确
		if(k != 18 && !about(r.actual.x, r.bounds.x)) { std::printf("FAIL %s.x: %.2f != %.2f\n", base.c_str(), r.actual.x, r.bounds.x); ++failures; }   // LabelText(18) 双 item：末 item 起点含 value+间距，跳过位置断言
		if(!about(r.actual.y, r.bounds.y)) { std::printf("FAIL %s.y: %.2f != %.2f\n", base.c_str(), r.actual.y, r.bounds.y); ++failures; }
		// b) 高度一致
		if(r.actual.w > 0.0f && r.actual.h > 0.0f && !about(r.bounds.h, r.actual.h)) {
			std::printf("FAIL %s.h: bounds=%.2f actual=%.2f\n", base.c_str(), r.bounds.h, r.actual.h); ++failures;
		}
		// c) 宽度受控（不超格）
		if(r.actual.w > r.bounds.w + 2.0f) {
			std::printf("FAIL %s.w: %.2f > bounds=%.2f\n", base.c_str(), r.actual.w, r.bounds.w); ++failures;
		}
		// d) 无重叠
		if(r.actual.y < prevBottom - 1.0f) {
			std::printf("FAIL %s.overlap: top=%.2f < prevBottom=%.2f\n", base.c_str(), r.actual.y, prevBottom); ++failures;
		}
		prevBottom = r.actual.y + r.actual.h;
	}

	std::printf("test22 原子实测几何: items=%zu failures=%d\n", recs.size(), failures);
	const bool pass = (failures == 0) && (recs.size() == kN);
	std::printf("%s\n", pass ? "PASS" : "FAIL");
	return pass ? 0 : 1;
}

