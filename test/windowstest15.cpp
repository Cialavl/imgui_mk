#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <iostream>

namespace {

// 格宽探针：测自己所在子窗口宽
struct WidthProbe { float& out; };
inline void draw(const WidthProbe& p) { p.out = ImGui::GetWindowSize().x; }

// 宿主内容宽探针：读宿主窗口可用宽（分格分配的 avail 基准）
struct AvailProbe { float& out; };
inline void draw(const AvailProbe& p) { p.out = ImGui::GetContentRegionAvail().x; }

} // namespace

// flexbox 约束分格验证：hsplit(fixed(250).min(150).atMost(0.35), stretch(1))
//   宽 1000 → 250（preferred）；600 → 0.35×avail（上限生效）；
//   380 → 150（min 兜底）；1400 → 250（回落 preferred）
//   账目平衡回归（Bug1）：任意字号/宽度下 fixed + stretch + 间距 == avail
//   rescale(27)：fixed 375（250×1.5），stretch 右缘不越界
// 120 帧后自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> mainWindow("约束分格验证", 1000, 600);

	int frames = 0;
	// 实时探针值
	float sbW = 0, stW = 0, availW = 0, gapX = 0;
	// 相位快照
	float sb1000 = 0, sb600 = 0, av600 = 0, sb380 = 0, sb1400 = 0;
	float sb27 = 0, st27 = 0, av27 = 0, gap27 = 0;

	mainWindow.setUI([&] {
		using namespace Mk::fui;

		gapX = ImGui::GetStyle().ItemSpacing.x;
		availW = ImGui::GetContentRegionAvail().x;

		draw(hsplit(
			fixed("sb", 250.0f, WidthProbe{sbW}).min(150.0f).atMost(0.35f),
			stretch("st", 1.0f, WidthProbe{stW})));
		draw(AvailProbe{availW});

		++frames;
		// 阶段驱动（setWindowSize 下一帧起生效，快照留 10 帧余量）
		if(frames == 20) { sb1000 = sbW; }
		if(frames == 40) { glfwSetWindowSize(mainWindow.getHandle(), 600, 600); }
		if(frames == 50) { sb600 = sbW; av600 = availW; }
		if(frames == 60) { glfwSetWindowSize(mainWindow.getHandle(), 380, 600); }
		if(frames == 70) { sb380 = sbW; }
		if(frames == 80) { glfwSetWindowSize(mainWindow.getHandle(), 1400, 600); }
		if(frames == 90) { sb1400 = sbW; }
		if(frames == 95) { mainWindow.getWindow().rescale(27.0f); }
		if(frames == 110) { sb27 = sbW; st27 = stW; av27 = availW; gap27 = gapX; }

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 断言 ——
	const float eps = 2.5f;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	const bool okFrames = (frames == 120);
	const bool okPref = about(sb1000, 250.0f);                       // preferred
	const bool okMax = about(sb600, 0.35f * av600) && sb600 < 248.0f; // maxFrac 生效
	const bool okMin = about(sb380, 150.0f);                          // min 兜底
	const bool okBack = about(sb1400, 250.0f);                        // 回落 preferred
	const bool okEm = about(sb27, 375.0f);                            // 250×1.5
	// 账目平衡（Bug1 回归）：fixed + stretch + 1×gap == avail
	const bool okBalance = about(sb27 + st27 + gap27, av27);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " sb@1000=" << sb1000
		<< " sb@600=" << sb600 << " (0.35*av=" << 0.35f * av600 << ")"
		<< " sb@380=" << sb380
		<< " sb@1400=" << sb1400
		<< " sb@27=" << sb27
		<< " balance=" << (sb27 + st27 + gap27) << " av@27=" << av27 << std::endl;

	const bool pass = okFrames && okPref && okMax && okMin && okBack && okEm
		&& okBalance && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", preferred " << okPref
		<< ", maxFrac " << okMax
		<< ", min " << okMin
		<< ", 回落 " << okBack
		<< ", em 联动 " << okEm
		<< ", 账目平衡 " << okBalance
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}