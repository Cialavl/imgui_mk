#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <iostream>
#include <string>

namespace {

// 控件宽度探针：测自己所在子窗口宽（fixed/fixedPx em 化验证）
struct WidthProbe { float& out; };
inline void draw(const WidthProbe& p) { p.out = ImGui::GetWindowSize().x; }

// em 探针：记录当前字号与 em(100) 换算值
struct EmProbe { float& fontSize; float& em100; };
inline void draw(const EmProbe& p) {
	p.fontSize = ImGui::GetFontSize();
	p.em100 = Mk::fui::em(100.0f);
}

// item 宽度探针：InputText 测实宽（withMinWidth 验证；PushItemWidth 只作用于输入/按钮类 item）
struct ItemWidthProbe { float& out; };
inline void draw(const ItemWidthProbe& p) {
	static char buf[8] = {};
	ImGui::InputText("##mw", buf, sizeof buf);
	p.out = ImGui::GetItemRectSize().x;
}

// 全文测量探针：测长串完整渲染宽（截断对照）
struct FullTextProbe { float& out; std::string text; };
inline void draw(const FullTextProbe& p) {
	p.out = ImGui::CalcTextSize(p.text.c_str()).x;
	ImGui::TextUnformatted(p.text.c_str());
}

// 省略文本测量探针：画 labelEllipsis 并记录其 item 实宽
struct EllipsisMeasure { float& out; std::string text; };
inline void draw(const EllipsisMeasure& m) {
	Mk::fui::draw(Mk::fui::labelEllipsis(std::move(m.text)));
	m.out = ImGui::GetItemRectSize().x;
}

} // namespace

// 文字溢出自适应验证（按字号相位快照，帧号仅驱动 rescale）：
//   fixed em 化：18px 相位 fixed(44) 实测 44；27px 相位实测 66（×1.5）
//   fixedPx：两相位都实测 44（真像素不随字号缩放）
//   em：27px 相位 em(100)=150
//   labelEllipsis：长路径在 150 设计像素格内——显示宽 < 全文宽（已截断）
//   withMinWidth(300)：窄内容 item 实测宽 == em(300)
// 120 帧后自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> mainWindow("文字溢出自适应验证", 1000, 600);

	int frames = 0;
	// 每帧实时值（树内探针更新）
	float wFixed = 0, wFixedPx = 0, curFont = 0, curEm100 = 0;
	float ellW = 0, ellFullW = 0, minW = 0;
	// 相位快照（树画完后抄录）
	float wFixed18 = 0, wFixed27 = 0, wFixedPx18 = 0, wFixedPx27 = 0;
	float em100at18 = 0, em100at27 = 0;
	float ellW18 = 0, ellFull18 = 0, minW18 = 0;

	const std::string longPath =
		"C:\\Users\\someone\\Documents\\projects\\very\\deep\\directory\\structure\\file_name.txt";

	mainWindow.setUI([&] {
		using namespace Mk::fui;

		// —— 树：各测量组（树内探针写实时值）——
		draw(vbox(6.0f,
			hsplit(
				fixed("fa", 44.0f, WidthProbe{wFixed}),
				fixedPx("fp", 44.0f, WidthProbe{wFixedPx}),
				stretch("mid", 1.0f, label(" "))),

			// 省略文本（150 设计像素格）与全文对照（stretch 格）
			hsplit(
				fixed("ell", 150.0f, EllipsisMeasure{ellW, longPath}),
				stretch("full", 1.0f, FullTextProbe{ellFullW, longPath})),

			// withMinWidth(200)：放进 260 设计像素窄格——默认宽(65%×~240≈156) < 200，item 应被撑到 em(200)=200
			hsplit(fixed("mw", 260.0f, withMinWidth(200.0f, ItemWidthProbe{minW})),
				stretch("mwr", 1.0f, label(" "))),

			EmProbe{curFont, curEm100}));

		// —— 驱动（树后）：帧 30 请求 27px、帧 60 回 18px（下一帧生效）——
		++frames;
		if(frames == 30) mainWindow.getWindow().rescale(27.0f);
		if(frames == 60) mainWindow.getWindow().rescale(18.0f);

		// 相位快照：20 帧（稳定 18px）、50 帧（稳定 27px）
		if(frames == 20) {
			wFixed18 = wFixed;
			wFixedPx18 = wFixedPx;
			em100at18 = curEm100;
			ellW18 = ellW;
			ellFull18 = ellFullW;
			minW18 = minW;
		}
		if(frames == 50) {
			wFixed27 = wFixed;
			wFixedPx27 = wFixedPx;
			em100at27 = curEm100;
		}

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 断言（±1.5px 容差）——
	const float eps = 1.5f;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	const bool okFrames = (frames == 120);
	const bool okFixed18 = about(wFixed18, 44.0f);              // 18px：em 因子 1.0
	const bool okFixed27 = about(wFixed27, 66.0f);              // 27px：44×1.5
	const bool okFixedPx = about(wFixedPx18, 44.0f) && about(wFixedPx27, 44.0f);
	const bool okEm18 = about(em100at18, 100.0f);
	const bool okEm27 = about(em100at27, 150.0f);
	// 18px 相位 em 因子 1.0：格宽 150；run() 后上下文已销毁，不能用 em() 动态算
	const bool okEllip = (ellW18 < ellFull18 && ellW18 <= 158.0f);
	const bool okMinW = about(minW18, 200.0f);              // 18px 下 em(200)=200，默认宽 156 被撑起
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " fixed18=" << wFixed18 << " fixed27=" << wFixed27
		<< " px18=" << wFixedPx18 << " px27=" << wFixedPx27
		<< " em100@18=" << em100at18 << " em100@27=" << em100at27
		<< " ellW=" << ellW18 << " ellFull=" << ellFull18
		<< " minW=" << minW18 << std::endl;

	const bool pass = okFrames && okFixed18 && okFixed27 && okFixedPx && okEm18
		&& okEm27 && okEllip && okMinW && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", fixed@18 " << okFixed18
		<< ", fixed@27 " << okFixed27
		<< ", fixedPx 恒定 " << okFixedPx
		<< ", em@18 " << okEm18
		<< ", em@27 " << okEm27
		<< ", 省略截断 " << okEllip
		<< ", 最小宽 " << okMinW
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}