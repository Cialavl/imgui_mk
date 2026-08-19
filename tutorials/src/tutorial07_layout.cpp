// 教程 07：布局系统。文档：lessons/lesson-07-layout.md
// 唯一布局控件 Layout（mode 切换 V/H/Wrap/Grid/SplitH/SplitV/Overlay）——
// 四个版本对照：原始固定宽 → SplitH 权重分格 → 字号等比（em）→ 约束分格。
// 分格是填充型容器：layout(host) 保留 host 尺寸；setFixed 尺寸为设计像素（em 缩放）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <array>
#include <string>

int main() {
	// 应用级尺寸 + 字号/间距双动态自适应（"实操窗口"）
	Mk::wg::setDefaultTheme(Mk::wg::scaledForMonitor(Mk::wg::Theme::dark()));
	const auto [w, h] = Mk::workAreaSize(0.9f, 0.9f, 1280, 900);
	Mk::MainWindow<> mainWindow("教程07：布局系统（拖大窗口宽度做实验！）", w, h);
	mainWindow.getWindow().setAutoScaleFont(true);

	static constexpr const char* kVersions[] = {
		"v1 原始固定宽", "v2 Layout SplitH", "v3 字号等比（em）", "v4 约束分格",
	};
	int sel = 0;
	std::array<char, 64> v4buf{};
	int frames = 0;

	// —— 四个版本的演示树（When 切换可见性，全部常驻内存；子先声明）——
	// v1：固定宽度子窗口
	Mk::wg::SubWindow<> v1a, v1b, v1c;
	Mk::wg::Label l1a, l1b, l1c;
	l1a.self.setName("固定 250 宽");
	l1b.self.setName("自适应（挤压后文字溢出）");
	l1c.self.setName("固定 200");
	v1a.node.idname = "v1a";  v1a.self.width = 250.0f;  v1a.self.height = 200.0f;
	v1a.node.add(l1a.node);
	v1b.node.idname = "v1b";  v1b.self.width = 0.0f;    v1b.self.height = 200.0f;
	v1b.node.add(l1b.node);
	v1c.node.idname = "v1c";  v1c.self.width = 200.0f;  v1c.self.height = 200.0f;
	v1c.node.add(l1c.node);
	Mk::wg::Layout v1;
	v1.self.mode = Mk::wg::LayoutMode::H;
	v1.node.add(v1a.node);
	v1.node.add(v1b.node);
	v1.node.add(v1c.node);

	// v2：SplitH 权重分格
	Mk::wg::Label l2a, l2b, l2c;
	l2a.self.setName("导航\nfixed 150");
	l2b.self.setName("主区 stretch 权重 2");
	l2c.self.setName("侧区 stretch 权重 1");
	Mk::wg::setFixed(l2a.node, 150.0f);
	Mk::wg::setStretch(l2b.node, 2.0f);
	Mk::wg::setStretch(l2c.node, 1.0f);
	Mk::wg::Layout v2;
	v2.self.mode = Mk::wg::LayoutMode::SplitH;
	v2.node.add(l2a.node);
	v2.node.add(l2b.node);
	v2.node.add(l2c.node);

	// v3：SplitV + em 字号等比工具栏
	Mk::wg::Button tbA1, tbA2, tbA3, tbB1, tbB2, tbB3;
	tbA1.self.setName("新");  tbA2.self.setName("开");  tbA3.self.setName("存");
	tbB1.self.setName("新");  tbB2.self.setName("开");  tbB3.self.setName("存");
	Mk::wg::Layout barA, barB;
	barA.self.mode = Mk::wg::LayoutMode::H;
	barA.node.add(tbA1.node);
	barA.node.add(tbA2.node);
	barA.node.add(tbA3.node);
	barB.self.mode = Mk::wg::LayoutMode::H;
	barB.node.add(tbB1.node);
	barB.node.add(tbB2.node);
	barB.node.add(tbB3.node);
	Mk::wg::Label l3c;
	l3c.self.setName("主区内容");
	Mk::wg::setFixed(barA.node, 44.0f);   // 设计像素：随字号等比缩放
	Mk::wg::setFixed(barB.node, 44.0f);
	Mk::wg::setStretch(l3c.node, 1.0f);
	Mk::wg::Layout v3;
	v3.self.mode = Mk::wg::LayoutMode::SplitV;
	v3.node.add(barA.node);
	v3.node.add(barB.node);
	v3.node.add(l3c.node);

	// v4：min/maxFrac 约束分格
	Mk::wg::Label l4a;
	l4a.self.setName("侧栏\npreferred 250\nmin 150\natMost 35%");
	Mk::wg::Label l4main;
	l4main.self.setName("主区 weight 1");
	Mk::wg::InputText<64> v4input;
	v4input.self.name = "占满输入";
	Mk::wg::Layout v4main;
	v4main.self.mode = Mk::wg::LayoutMode::V;
	v4main.node.add(l4main.node);
	v4main.node.add(v4input.node);
	Mk::wg::setFixed(l4a.node, 250.0f, 150.0f, 0.35f);   // 收缩下限 150 / 上限 35%
	Mk::wg::setStretch(v4main.node, 1.0f);
	Mk::wg::Layout v4;
	v4.self.mode = Mk::wg::LayoutMode::SplitH;
	v4.node.add(l4a.node);
	v4.node.add(v4main.node);

	// 顶部仪表 + 版本切换（When 包裹版本树，隐藏时不布局不渲染）
	Mk::wg::SeparatorText gauge;
	gauge.self.name = "实验仪表：拖窗口右边缘变宽，观察各版本行为";
	Mk::wg::LabelText ltW, ltFont, ltFrame;
	ltW.self.name = "窗口宽";
	ltFont.self.name = "字号";
	ltFrame.self.name = "帧";
	Mk::wg::Layout gaugeRow;
	gaugeRow.self.mode = Mk::wg::LayoutMode::H;
	gaugeRow.node.add(ltW.node);
	gaugeRow.node.add(ltFont.node);
	gaugeRow.node.add(ltFrame.node);
	Mk::wg::RadioGroup verSel;
	verSel.self.items = kVersions;
	verSel.self.count = 4;
	verSel.self.current = &sel;

	Mk::wg::When<Mk::wg::Layout> w1, w2, w3, w4;
	w1.node.add(v1.node);
	w2.node.add(v2.node);
	w3.node.add(v3.node);
	w4.node.add(v4.node);

	Mk::wg::Label vNote;
	vNote.self.setName("v1 固定宽在窄窗口溢出；v2 分格按权重分配；v3 尺寸随字号等比（em）；"
		"v4 有 min/maxFrac 约束");

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(gauge.node);
	root.node.add(gaugeRow.node);
	root.node.add(verSel.node);
	root.node.add(w1.node);
	root.node.add(w2.node);
	root.node.add(w3.node);
	root.node.add(w4.node);
	root.node.add(vNote.node);

	mainWindow.setUI([&] {
		++frames;
		ltW.self.value = std::to_string((int)ImGui::GetMainViewport()->Size.x);
		ltFont.self.value = std::to_string((int)ImGui::GetFontSize());
		ltFrame.self.value = std::to_string(frames);
		w1.self.show = (sel == 0);
		w2.self.show = (sel == 1);
		w3.self.show = (sel == 2);
		w4.self.show = (sel == 3);
		root.layout(Mk::wg::Rect{20, 20, 1240, 860});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
