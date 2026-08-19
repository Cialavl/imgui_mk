#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cmath>
#include <iostream>

// test20：wg Qt 式分格布局（SplitH/SplitV + fixed 覆盖/stretch/em）确定性断言。
//   A: fixed(200)+stretch(1)+stretch(2) @宽500 spacing8 → 284 按 1:2 分
//   B: fixed(400,min100)+fixed(300,min100) @宽500 → 按可缩比例压缩 → 275.2/216.8
//   C: fixed(300)+max(250)+stretch(1) @宽500 → 250/242
//   D: SplitV fixed(50)+stretch(1) @高200 → 50/142；em(100)==100（字号 18）
// 120 帧自动关窗；确定性断言 PASS。
int main() {
	Mk::MainWindow<> win("wg 分格布局验证", 900, 700);

	int frames = 0;
	float a0x = 0, a0w = 0, a1x = 0, a1w = 0, a2x = 0, a2w = 0, aH = 0;
	float b0w = 0, b1w = 0, b1x = 0;
	float c0w = 0, c1w = 0, c1x = 0;
	float d0y = 0, d0h = 0, d1y = 0, d1h = 0;
	float emProbe = 0, em2w = 0, em2h = 0, fontAt = 0;

	int snap = 0;

	// A：固定 200 + 权重 1:2
	Mk::wg::Label aL0, aL1, aL2;
	aL0.self.setName("A0");
	aL1.self.setName("A1");
	aL2.self.setName("A2");
	aL0.setFixedWidth(200.0f);
	aL1.setPrefSize(30.0f, 0.0f);
	aL2.setPrefSize(30.0f, 0.0f);
	Mk::wg::SplitHLayout splitA;
	splitA.setSpacing(8.0f); splitA.setContentsMargins(0.0f, 0.0f);   // 显式定值：断言确定性
	splitA.addWidget(aL0.node);
	splitA.addWidget(aL1.node, 1.0f);
	splitA.addWidget(aL2.node, 2.0f);

	// B：双固定超宽（shrink 语义）
	Mk::wg::Label bL0, bL1;
	bL0.self.setName("B0");
	bL1.self.setName("B1");
	bL0.setPrefSize(400.0f, 0.0f); bL0.setMinSize(100.0f, 0.0f);
	bL1.setPrefSize(300.0f, 0.0f); bL1.setMinSize(100.0f, 0.0f);
	Mk::wg::SplitHLayout splitB;
	splitB.setSpacing(8.0f); splitB.setContentsMargins(0.0f, 0.0f);
	splitB.addWidget(bL0.node);
	splitB.addWidget(bL1.node);

	// C：max 钳制
	Mk::wg::Label cL0, cL1;
	cL0.self.setName("C0");
	cL1.self.setName("C1");
	cL0.setPrefSize(300.0f, 0.0f); cL0.setMaxSize(250.0f, 0.0f);
	cL1.setPrefSize(30.0f, 0.0f);
	Mk::wg::SplitHLayout splitC;
	splitC.setSpacing(8.0f); splitC.setContentsMargins(0.0f, 0.0f);
	splitC.addWidget(cL0.node);
	splitC.addWidget(cL1.node, 1.0f);

	// D：垂直分格
	Mk::wg::Label dL0, dL1;
	dL0.self.setName("D0");
	dL1.self.setName("D1");
	dL0.setFixedHeight(50.0f);
	Mk::wg::SplitVLayout splitD;
	splitD.setSpacing(8.0f); splitD.setContentsMargins(0.0f, 0.0f);
	splitD.addWidget(dL0.node);
	splitD.addWidget(dL1.node, 1.0f);

	win.setUI([&] {
		++frames;

		splitA.layout(Mk::wg::Rect{20, 20, 500, 60});
		splitB.layout(Mk::wg::Rect{20, 100, 500, 60});
		splitC.layout(Mk::wg::Rect{20, 180, 500, 60});
		splitD.layout(Mk::wg::Rect{20, 260, 400, 200});

		if(frames == 20) {
			snap = 1;
			auto itA = splitA.node.children.begin();
			a0x = (*itA)->bounds.x; a0w = (*itA)->bounds.w;
			a1x = (*(itA + 1))->bounds.x; a1w = (*(itA + 1))->bounds.w;
			a2x = (*(itA + 2))->bounds.x; a2w = (*(itA + 2))->bounds.w;

			auto itB = splitB.node.children.begin();
			b0w = (*itB)->bounds.w;
			b1x = (*(itB + 1))->bounds.x; b1w = (*(itB + 1))->bounds.w;
			auto itC = splitC.node.children.begin();
			c0w = (*itC)->bounds.w;
			c1x = (*(itC + 1))->bounds.x; c1w = (*(itC + 1))->bounds.w;
			auto itD = splitD.node.children.begin();
			d0y = (*itD)->bounds.y; d0h = (*itD)->bounds.h;
			d1y = (*(itD + 1))->bounds.y; d1h = (*(itD + 1))->bounds.h;
			emProbe = Mk::wg::em(100.0f);
			ImVec2 e2 = Mk::wg::em2(100.0f, 50.0f);
			em2w = e2.x; em2h = e2.y;
			fontAt = ImGui::GetFontSize();
			aH = (*itA)->bounds.h;
		}

		ImGui::Text("frames=%d A:%.0f/%.0f/%.0f", frames, a0w, a1w, a2w);

		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	const float eps = 1.0f;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	// A：固定 200；剩余 500-200-16=284 按 1:2 → 94.67 / 189.33
	const bool okA = snap == 1
		&& about(a0x, 20.0f) && about(a0w, 200.0f)
		&& about(a1x, 20.0f + 200.0f + 8.0f) && about(a1w, 30.0f + 224.0f / 3.0f)
		&& about(a2x, 20.0f + 200.0f + 8.0f + 30.0f + 224.0f / 3.0f + 8.0f) && about(a2w, 30.0f + 224.0f * 2.0f / 3.0f)
		&& about(aH, 60.0f);
	// B：hint 700 min 200 → 压缩 275.2 / 216.8
	const bool okB = about(b0w, 275.2f) && about(b1x, 20.0f + 275.2f + 8.0f) && about(b1w, 216.8f);
	// C：max 钳到 250；剩余 242
	const bool okC = about(c0w, 250.0f) && about(c1x, 20.0f + 250.0f + 8.0f) && about(c1w, 192.0f);
	// D：固定 50 + 剩余 200-50-8=142
	const bool okD = about(d0y, 260.0f) && about(d0h, 50.0f)
		&& about(d1y, 260.0f + 50.0f + 8.0f) && about(d1h, 142.0f);
	// em：字号 18 → 缩放 1:1
	const bool okEm = about(fontAt, 18.0f) && about(emProbe, 100.0f)
		&& about(em2w, 100.0f) && about(em2h, 50.0f);

	std::cout << "A: " << a0w << "/" << a1w << "/" << a2w
		<< " B: " << b0w << "/" << b1w
		<< " C: " << c0w << "/" << c1w
		<< " D: " << d0h << "/" << d1h
		<< " em=" << emProbe << std::endl;

	const bool pass = okA && okB && okC && okD && okEm;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}
