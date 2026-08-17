#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <cmath>
#include <cstdio>
#include <iostream>

namespace {

// 渲染探针：统计自己被渲染的帧数
struct RenderProbe { int& renders; };
inline void draw(const RenderProbe& p) { ++p.renders; }

// 帧驱动器：每帧推进逻辑侧状态（放在树尾，当帧树已画完）
struct FrameDriver {
	int& frames;
	int& iVal;
	float& drag;
	int& dragI;
	int& radio;
	int& comboSel;
	int& listSel;
	std::array<char, 64>& buf;

	void tick() const {
		++frames;
		if(frames % 10 == 0) ++iVal;               // 120 帧共 12 次
		drag = 0.5f + frames * 0.01f;              // 帧末 = 1.7f
		dragI = frames;                            // 帧末 = 120
		radio = frames % 3;                        // 帧末 = 0
		comboSel = (frames / 20) % 3;              // 帧末 = 0
		listSel = (frames / 30) % 3;               // 帧末 = 1
		if(frames % 30 == 0) {
			std::snprintf(buf.data(), buf.size(), "frame#%d", frames);
		}
	}
};
inline void draw(const FrameDriver& d) { d.tick(); }

} // namespace

// 批次1 输入控件验证：InputText/Int/Float/Double/Int2/Float2、Drag、SliderInt/Angle、
// Radio/RadioGroup、Combo、ListBox、itemWidth 修饰器。
// 逻辑侧每帧驱动绑定变量，验证渲染稳定性与联动；120 帧后自动关窗，确定性断言。
int main() {
	Mk::MainWindow<> mainWindow("函数式输入控件验证", 1280, 800);

	// —— 外置状态 ——
	std::array<char, 64> buf{};
	std::snprintf(buf.data(), buf.size(), "hello");
	int iVal = 5;
	float fVal = 1.5f;
	double dVal = 2.5;
	std::array<int, 2> iv{1, 2};
	std::array<float, 2> fv{3.5f, 4.5f};
	float drag = 0.5f;
	int dragI = 0;
	int slideI = 50;
	float angle = 0.785f;
	bool radioBool = false;
	int radio = 1, comboSel = 0, listSel = 0;
	int frames = 0, probeAll = 0;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};

	FrameDriver driver{frames, iVal, drag, dragI, radio, comboSel, listSel, buf};

	mainWindow.setUI([&] {
		Mk::fui::draw(Mk::fui::vbox(6.0f,
			Mk::fui::inputText("文本", buf),
			Mk::fui::itemWidth(120.0f, Mk::fui::inputInt("整数", iVal)),
			Mk::fui::inputFloat("浮点", fVal),
			Mk::fui::inputDouble("双精度", dVal),
			Mk::fui::hbox(8.0f,
				Mk::fui::inputInt2("ivec2", iv),
				Mk::fui::inputFloat2("vec2", fv)),
			Mk::fui::hbox(8.0f,
				Mk::fui::dragF("拖拽浮点", drag, 0.01f, 0.0f, 10.0f),
				Mk::fui::dragI("拖拽整数", dragI, 1.0f, 0, 1000)),
			Mk::fui::hbox(8.0f,
				Mk::fui::sliderInt("整数滑块", slideI, 0, 100),
				Mk::fui::sliderAngle("角度", angle)),
			Mk::fui::radioButton("独立单选", radioBool),
			Mk::fui::radioGroup(kItems, radio),
			Mk::fui::combo("下拉", comboSel, kItems),
			Mk::fui::listBox("列表", listSel, kItems),
			RenderProbe{probeAll},
			driver
		));

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const bool okFrames = (frames == 120);
	const bool okProbe = (probeAll == 120);
	const bool okI = (iVal == 5 + 12);
	const bool okDrag = (std::abs(drag - 1.7f) < 1e-4f);
	const bool okSel = (radio == 0 && comboSel == 0 && listSel == 1);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " probe=" << probeAll
		<< " iVal=" << iVal
		<< " drag=" << drag
		<< " radio=" << radio
		<< " combo=" << comboSel
		<< " list=" << listSel << std::endl;

	const bool pass = okFrames && okProbe && okI && okDrag && okSel && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", 探针 " << okProbe
		<< ", 输入联动 " << okI
		<< ", 拖拽联动 " << okDrag
		<< ", 选择联动 " << okSel
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}