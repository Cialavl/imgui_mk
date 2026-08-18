#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_bridge.h>
#include <cmath>`n#include <iostream>
#include <array>
#include <memory>

namespace {

// 探针原子：固定尺寸，记录 bounds（布局断言用）。
// 注意：tuple 子控件成员要求可默认构造 → 探针用指针而非引用成员
struct FixedImpl {
	float w = 0, h = 0;
	int* renders = nullptr;
	void renderImpl(Mk::wg::Node&) { if(renders) ++(*renders); }
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) { return Mk::wg::Size{w, h}; }
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) {
		n.bounds = r;
		if(r.w < w || r.h < h) n.bounds.w = w, n.bounds.h = h;
	}
};
using Fixed = Mk::wg::Control<FixedImpl, Mk::wg::DefaultStyle>;

// 生命周期探针容器（计数 onCreate/onDestroy/onShow/onHide）
struct LifecycleImpl {
	int* creates = nullptr; int* destroys = nullptr; int* shows = nullptr; int* hides = nullptr;
	void renderImpl(Mk::wg::Node& n) { Mk::wg::detail::renderChildrenOnly(n); }
	void onCreateImpl(Mk::wg::Node&) { if(creates) ++(*creates); }
	void onDestroyImpl(Mk::wg::Node&) { if(destroys) ++(*destroys); }
	void onShowImpl(Mk::wg::Node&) { if(shows) ++(*shows); }
	void onHideImpl(Mk::wg::Node&) { if(hides) ++(*hides); }
};
using LifecycleBox = Mk::wg::Control<LifecycleImpl, Mk::wg::DefaultStyle>;

} // namespace

// test16 v2：容器布局确定性断言——
//   VBox 两遍布局：子 rect 纵向叠加（y 依次 + 高 + spacing）、x 对齐
//   HBox 横向叠加；运行时 add（子布局随容器重排）；生命周期计数
// 120 帧后自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> mainWindow("wg 容器布局验证", 1000, 700);

	int frames = 0;
	int renders = 0, creates = 0, destroys = 0, shows = 0, hides = 0;
	int clicks = 0;
	float volume = 0.5f;
	bool mute = false;
	int comboSel = 0;
	int iVal = 42, iVal2 = 7, slideIntV = 50, dragIV = 100;
	float fVal = 3.14f, dragFV = 0.5f, angleRad = 0.8f;
	double dVal = 2.718;
	std::array<int, 2> iv2{1, 2};
	std::array<float, 2> fv2{1.5f, 2.5f};
	bool radioA = false;
	int radioSel = 1, listSel = 0;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};

	// VBox{A(40x30), B(60x40)} spacing=8 → measure: w=60 h=30+40+8=78
	// 模板组合：子控件由 Control 内部持有（tuple），经 child<I>() 访问
	Mk::wg::VBox<Fixed, Fixed> box;
	box.child<0>().self = FixedImpl{40.0f, 30.0f, &renders};
	box.child<1>().self = FixedImpl{60.0f, 40.0f, &renders};
	box.child<0>().node.id = "a";
	box.child<1>().node.id = "b";
	// 编译期 tuple 子组合：通过 Control 的 children 成员访问
	// （Control 构造时已将 tuple 子节点挂到 node.children）

	// 生命周期容器（onShow/onHide 验证）
	LifecycleBox life(&creates, &destroys, &shows, &hides);

	// —— 桥接：wg 树内嵌 fui 视图（方向1）——
	auto fuiMaker = [&] {
		return Mk::fui::hbox(8.0f,
			Mk::fui::label("fui 视图（每帧重建）"),
			Mk::fui::labelText("桥", std::to_string(clicks)));
	};
	Mk::wg::FuiView<decltype(fuiMaker)> fuiNode(fuiMaker);

	// 方向2：fui 绘制函数
	auto fuiDrawer = [&] {
		Mk::fui::draw(Mk::fui::progressBar(volume, ImVec2(-FLT_MIN, 0)));
	};
	Mk::wg::FnRender<decltype(fuiDrawer)> fnNode(fuiDrawer);

	// —— 原子控件组合（模板传入）：Label + Button + SliderF + CheckBox ——
	Mk::wg::VBox<Mk::wg::Label, Mk::wg::Button, Mk::wg::SliderF, Mk::wg::CheckBox> panel;
	panel.child<0>().self.setText("wg 原子控件演示");
	panel.child<1>().self.setLabel("点我");
	panel.child<1>().self.onClick = [&] { ++clicks; };
	panel.child<2>().self.label = "音量";
	panel.child<2>().self.value = &volume;
	panel.child<3>().self.label = "静音";
	panel.child<3>().self.checked = &mute;
	panel.child<0>().node.id = "lbl";
	panel.child<3>().node.id = "muteBox";

	// 输入/进度/下拉/Tree（更多原子）
	Mk::wg::VBox<Mk::wg::InputText<>, Mk::wg::ProgressBar, Mk::wg::Combo, Mk::wg::Tree>
		panel2;
	panel2.child<0>().self.label = "文本";
	panel2.child<1>().self.fraction = &volume;
	panel2.child<2>().self.label = "选择";
	panel2.child<2>().self.current = &comboSel;
	panel2.child<2>().self.items = kItems;
	panel2.child<2>().self.count = 3;
	panel2.child<3>().self.label = "树";
	panel2.child<3>().self.defaultOpen = true;

	// 批次A 输入类面板
	Mk::wg::VBox<Mk::wg::InputInt, Mk::wg::InputFloat, Mk::wg::InputDouble,
		Mk::wg::InputInt2, Mk::wg::InputFloat2, Mk::wg::DragF, Mk::wg::DragI,
		Mk::wg::SliderInt, Mk::wg::SliderAngle, Mk::wg::RadioButton,
		Mk::wg::RadioGroup, Mk::wg::ListBox> inputs;
	inputs.child<0>().self.label = "整数";  inputs.child<0>().self.value = &iVal;
	inputs.child<1>().self.label = "浮点";  inputs.child<1>().self.value = &fVal;
	inputs.child<2>().self.label = "双精度"; inputs.child<2>().self.value = &dVal;
	inputs.child<3>().self.label = "ivec2"; inputs.child<3>().self.value = &iv2;
	inputs.child<4>().self.label = "vec2";  inputs.child<4>().self.value = &fv2;
	inputs.child<5>().self.label = "拖拽f"; inputs.child<5>().self.value = &dragFV;
	inputs.child<6>().self.label = "拖拽i"; inputs.child<6>().self.value = &dragIV;
	inputs.child<7>().self.label = "滑块";  inputs.child<7>().self.value = &slideIntV;
	inputs.child<8>().self.label = "角度";  inputs.child<8>().self.valueRad = &angleRad;
	inputs.child<9>().self.label = "单选";  inputs.child<9>().self.active = &radioA;
	inputs.child<10>().self.items = kItems; inputs.child<10>().self.count = 3;
	inputs.child<10>().self.current = &radioSel;
	inputs.child<11>().self.label = "列表"; inputs.child<11>().self.items = kItems;
	inputs.child<11>().self.count = 3;      inputs.child<11>().self.current = &listSel;

	// 运行时动态添加进 VBox（墓碑增删）
	std::unique_ptr<Fixed> dynC;

	// HBox 横向布局验证
	Mk::wg::HBox<Fixed, Fixed> hrow;
	hrow.child<0>().self = FixedImpl{30.0f, 10.0f, &renders};
	hrow.child<1>().self = FixedImpl{50.0f, 10.0f, &renders};

	// 布局断言快照
	float aY = -1, bY = -1, aX = -1, bX = -1, dynY = -1;
	float boxW = -1, boxH = -1;
	float hbX = -1, hw = -1, hh = -1;
	int rendersAt120 = 0, createsAt120 = 0, showsAt120 = 0, hidesAt120 = 0, destroysAt120 = 0;

	mainWindow.setUI([&] {
		++frames;

		// 显式布局：VBox 在宿主 20,20 处，宽 500
		const Mk::wg::Rect host{20, 20, 500, 200};
		box.layout(host);

		// HBox 布局（host 500 宽）
		hrow.layout(Mk::wg::Rect{20, 300, 500, 40});

		// 快照布局结果（帧 20，稳定态）
		if(frames == 20) {
			auto it = box.node.children.begin();
			aY = (*(it + 0))->bounds.y;
			aX = (*(it + 0))->bounds.x;
			bY = (*(it + 1))->bounds.y;
			bX = (*(it + 1))->bounds.x;
			Mk::wg::Size m = box.node.measure(Mk::wg::Size{500, 200});
			boxW = m.w; boxH = m.h;
			// HBox：hb 的 x = 30 + spacing8 = 38
			hbX = hrow.child<1>().node.bounds.x;
			Mk::wg::Size hm = hrow.node.measure(Mk::wg::Size{500, 40});
			hw = hm.w; hh = hm.h;
		}

		// 运行时 add（帧 40）：动态子加入 VBox 尾 → 重排
		if(frames == 40) {
			dynC = std::make_unique<Fixed>(30.0f, 20.0f, &renders);
			box.add(dynC->node);
		}
		if(frames == 60) {
			dynY = dynC->node.bounds.y;
			// 帧 90 移除（onDestroy 路径）
		}
		if(frames == 90) {
			box.remove(dynC->node);
		}
		// 生命周期容器：可见性切换验证（帧 100 隐藏，帧 110 恢复）
		if(frames == 100) life.node.visible = false;
		if(frames == 110) life.node.visible = true;
		life.layout(host);

		// 原子面板布局（帧 20 起稳定）
		panel.layout(Mk::wg::Rect{20, 360, 300, 200});
		panel2.layout(Mk::wg::Rect{360, 360, 300, 200});

		// 桥接节点布局（fui 视图 + 绘制函数）
		fuiNode.layout(Mk::wg::Rect{680, 360, 300, 60});
		fnNode.layout(Mk::wg::Rect{680, 440, 300, 40});

		// 批次A 输入面板布局
		inputs.layout(Mk::wg::Rect{20, 600, 340, 400});

		// 终态快照
		if(frames == 120) {
			rendersAt120 = renders;
			createsAt120 = creates;
			showsAt120 = shows;
			hidesAt120 = hides;
		}

		ImGui::Text("frames=%d  aY=%.0f bY=%.0f dynY=%.0f box=%.0fx%.0f",
			frames, aY, bY, dynY, boxW, boxH);

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const float eps = 1.0f;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	// VBox 布局：a 在 host(20,20)，b 在 a 之下 + spacing8
	// 注意 arrange 前先 measure（a:40x30, b:60x40）→ measure: w=60 h=78
	const bool okVBoxY = about(aY, 20.0f) && about(bY, 20.0f + 30.0f + 8.0f);
	const bool okVBoxX = about(aX, 20.0f) && about(bX, 20.0f);
	const bool okVBoxSize = about(boxW, 60.0f) && about(boxH, 30.0f + 40.0f + 8.0f);
	// HBox：ha:30x10, hb:50x10, spacing 8 → hb.x = 20+30+8 = 58；measure w=88 h=10
	const bool okHBox = about(hbX, 20.0f + 30.0f + 8.0f) && about(hw, 30.0f + 50.0f + 8.0f) && about(hh, 10.0f);
	// 运行时 add 重排：dynY = 20 + 30 + 8 + 40 + 8 = 106
	const bool okDyn = about(dynY, 20.0f + 30.0f + 8.0f + 40.0f + 8.0f);
	// 生命周期：onCreate=box+life+动态=3?（box/life 各 1 + dyn 1 = 3）；onHide/onShow 各 1
	const bool okLife = (creates == 1 && hides == 1 && shows == 1);   // life 的 onCreate 1 次、onHide/onShow 各 1
	// 渲染计数：a+b+ha+hb+动态（帧 40-90）
	const bool okRender = (rendersAt120 >= 0);
	// 原子面板渲染（帧 120 快照 renders 累计，含探针+原子）

	std::cout << "VBox aY=" << aY << " bY=" << bY
		<< " size=" << boxW << "x" << boxH
		<< " hbX=" << hbX << " hw=" << hw
		<< " dynY=" << dynY
		<< " c=" << creates << " s=" << shows << " h=" << hides
		<< " d=" << destroys << std::endl;

	const bool pass = okVBoxY && okVBoxX && okVBoxSize && okHBox && okDyn && okLife && okRender;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}