#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <cmath>
#include <iostream>

namespace {

// 渲染探针：统计自己被渲染的帧数
struct RenderProbe { int& renders; };
inline void draw(const RenderProbe& p) { ++p.renders; }

// 关窗哨兵：放树尾，读当帧计数
struct CloseSentinel { int& frames; };
inline void draw(const CloseSentinel& s) {
	if(s.frames >= 120) {
		glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
	}
}

// 模态关闭哨兵：放在模态弹窗内部，第 90 帧起主动关闭模态
//（CloseCurrentPopup 必须在 Begin/EndPopup 作用域内调用）
struct ModalCloser { int& frames; };
inline void draw(const ModalCloser& m) {
	if(m.frames >= 90) {
		ImGui::CloseCurrentPopup();
	}
}

} // namespace

// 批次4 弹窗与高级控件验证：Popup/PopupModal（标志触发式，从首帧打开存活到结束）、
// ColorEdit3/4、ColorPicker3/4（正弦驱动）、CheckboxFlags（第 50 帧置位）。
// 120 帧后自动关窗，确定性断言。
int main() {
	Mk::MainWindow<> mainWindow("函数式弹窗与高级控件验证", 1280, 800);

	// —— 外置状态 ——
	int frames = 0;
	bool popupOpen = true;    // 首帧即打开：内容应渲染 120 帧
	bool modalOpen = false;   // 第 60 帧由逻辑侧打开（模态会独占，不能与普通弹窗同时常驻）
	std::array<float, 3> col3{1.0f, 0.0f, 0.0f};
	std::array<float, 4> col4{0.0f, 1.0f, 0.0f, 1.0f};
	std::array<float, 3> pick3{0.2f, 0.4f, 0.6f};
	std::array<float, 4> pick4{0.6f, 0.4f, 0.2f, 1.0f};
	int bitFlags = 0;
	int probeAll = 0, probePopup = 0, probeModal = 0;

	mainWindow.setUI([&] {
		++frames;
		// 逻辑侧驱动颜色（正弦往返）与位标志
		col3[0] = 0.5f + 0.5f * std::sin(frames * 0.05f);
		col4[1] = 0.5f + 0.5f * std::cos(frames * 0.05f);
		if(frames == 50) bitFlags |= 2;
		if(frames == 60) modalOpen = true;

		Mk::fui::draw(Mk::fui::vbox(6.0f,
			Mk::fui::colorEdit3("RGB 编辑", col3),
			Mk::fui::colorEdit4("RGBA 编辑", col4),
			Mk::fui::colorPicker3("RGB 拾取", pick3),
			Mk::fui::colorPicker4("RGBA 拾取", pick4),
			Mk::fui::checkboxFlags("位 B", bitFlags, 2),
			Mk::fui::popup("普通弹窗", popupOpen,
				Mk::fui::label("弹窗内容（从首帧存活）"),
				RenderProbe{probePopup}),
			Mk::fui::popupModal("模态弹窗", modalOpen, 0,
				Mk::fui::label("模态内容（60 帧开、90 帧关）"),
				RenderProbe{probeModal},
				ModalCloser{frames}),
			RenderProbe{probeAll},
			CloseSentinel{frames}
		));
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const float expectR = 0.5f + 0.5f * std::sin(120 * 0.05f);
	const float expectG = 0.5f + 0.5f * std::cos(120 * 0.05f);
	const bool okFrames = (frames == 120);
	const bool okProbe = (probeAll == 120);
	// 普通弹窗常驻 1..60 帧；第 60 帧模态打开将其强制关闭（ImGui 模态独占语义）
	const bool okPopup = (probePopup == 60);
	const bool okModal = (probeModal == 31);   // 60..90 帧含两端
	const bool okColor = (std::abs(col3[0] - expectR) < 1e-4f
		&& std::abs(col4[1] - expectG) < 1e-4f);
	const bool okFlags = (bitFlags == 2);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " probe=" << probeAll
		<< " popup=" << probePopup
		<< " modal=" << probeModal
		<< " r=" << col3[0]
		<< " g=" << col4[1]
		<< " flags=" << bitFlags << std::endl;

	const bool pass = okFrames && okProbe && okPopup && okModal && okColor && okFlags && okExit;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}