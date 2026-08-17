#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <iostream>
#include <string>
#include <vector>

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

} // namespace

// 批次3 结构容器验证：TabBar（仅激活页渲染）、TextTable（外部数据驱动）、
// FnTable（cellFn 泛型）、MenuBar/Menu/MenuItem、Panel 浮动面板。
// 逻辑侧每帧刷新表格首格文本；120 帧后自动关窗，确定性断言。
int main() {
	Mk::MainWindow<> mainWindow("函数式结构容器验证", 1280, 800);

	// —— 外置状态 ——
	int frames = 0;
	int menuClicks = 0, panelClicks = 0;
	int probeTabA = 0, probeTabB = 0, probePanel = 0;
	static constexpr const char* kHeaders[] = {"名称", "数值", "备注"};
	std::vector<std::vector<std::string>> rows{
		{"甲", "1", ""},
		{"乙", "2", ""},
		{"丙", "3", ""},
	};

	mainWindow.setUI([&] {
		++frames;
		rows[0][2] = "帧=" + std::to_string(frames);   // 数据驱动：表格内容每帧更新

		Mk::fui::draw(Mk::fui::vbox(6.0f,
			Mk::fui::tabBar("主标签栏", 0,
				Mk::fui::tab("页甲", RenderProbe{probeTabA}),
				Mk::fui::tab("页乙", RenderProbe{probeTabB})),
			Mk::fui::textTable("文本表", kHeaders, rows),
			Mk::fui::fnTable("函数表", kHeaders, 3,
				[](int r, int col) { return "r" + std::to_string(r) + "c" + std::to_string(col); }),
			Mk::fui::menuBar(
				Mk::fui::menu("文件",
					Mk::fui::menuItem("打开", [&] { ++menuClicks; }, "Ctrl+O"),
					Mk::fui::menuItem("退出", [&] { ++menuClicks; })),
				Mk::fui::menu("帮助",
					Mk::fui::menuItem("关于", [&] { ++menuClicks; }))),
			Mk::fui::panel("浮动面板", 0,
				Mk::fui::label("可在宿主内拖动的浮动窗口"),
				Mk::fui::button("面板按钮", [&] { ++panelClicks; }),
				RenderProbe{probePanel}),
			CloseSentinel{frames}
		));
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const bool okFrames = (frames == 120);
	const bool okTabA = (probeTabA == 120);   // 默认激活页每帧渲染
	const bool okTabB = (probeTabB == 0);     // 未激活页从不渲染（无法程序化切换）
	const bool okRows = (rows[0][2] == "帧=120");
	const bool okMenu = (menuClicks == 0);    // 菜单未点开：不误触发
	const bool okPanel = (probePanel == 120);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " tabA=" << probeTabA
		<< " tabB=" << probeTabB
		<< " rows=" << rows[0][2]
		<< " menu=" << menuClicks
		<< " panel=" << probePanel << std::endl;

	const bool pass = okFrames && okTabA && okTabB && okRows && okMenu && okPanel && okExit;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}