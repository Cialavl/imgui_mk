// 教程 07：布局系统演进。文档：lessons/lesson-07-layout.md
// 四个版本并排对照：绝对像素 → hsplit/vsplit → em 设计像素 → min/atMost 约束。
// 顶栏实时显示窗口宽度；§7.3 需要拖窗口跨宽度档位观察字号变化（本程序开启自适应）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <string>

int main() {
	// 工作区比例尺寸 + 字号/宽度双自适应：本课的"实验器材"
	Mk::fui::setDefaultTheme(Mk::fui::scaledForMonitor(Mk::fui::Theme::dark()));
	const auto [w, h] = Mk::workAreaSize(0.9f, 0.9f, 1280, 900);
	Mk::MainWindow<> mainWindow("教程07：布局系统演进（拖窗口宽度做实验！）", w, h);
	mainWindow.getWindow().setAutoScaleFont(true);

	static constexpr const char* kVersions[] = {
		"v1 绝对像素", "v2 hsplit", "v3 em/fixedPx 对照", "v4 约束三栏",
	};   // §6.6-1 items 须长期存活（不能用临时 init-list）
	int sel = 0;    // 版本切换（0..3 对应 §7.1..§7.4）
	std::array<char, 64> v4buf{};
	int frames = 0;

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		// 顶栏：实验仪表（当前窗口宽度——拖动时观察）
		draw(vbox(6.0f,
			separatorText("实验仪表：拖动窗口右边缘改变宽度，观察下面各区"),
			hbox(16.0f,
				labelText("窗口宽", std::to_string((int)ImGui::GetMainViewport()->Size.x)),
				labelText("字号", std::to_string((int)ImGui::GetFontSize())),
				labelText("帧", std::to_string(frames))),
			radioGroup(kVersions, sel),

			// —— §7.1 第一版：绝对像素（暴露：拖窄窗口侧栏吃屏）——
			when(sel == 0, vbox(4.0f,
				textColored("拖窄窗口：侧栏 250px 纹丝不动，内容区被挤没",
					ImVec4(0.9f, 0.5f, 0.3f, 1)),
				hbox(0.0f,
					subwindow("v1 侧栏(绝对250)", ImVec2(250, 200), true, label("固定 250 像素")),
					subwindow("v1 内容", ImVec2(0, 200), true, label("自适应（被挤压的受害者）")),
					subwindow("v1 右栏(绝对200)", ImVec2(200, 200), true, label("固定 200"))))),

			// —— §7.2 第二版：hsplit/stretch（改善：不再溢出；遗留：侧栏占比仍涨）——
			when(sel == 1, vbox(4.0f,
				textColored("拖窄窗口：内容区优雅收缩，但侧栏占比越来越大 → 引出 v4",
					ImVec4(0.9f, 0.7f, 0.3f, 1)),
				hsplit(
					fixed("v2nav", 150.0f, label("导航\nfixed 150")),
					stretch("v2main", 2.0f, label("主区 stretch 权重 2")),
					stretch("v2side", 1.0f, label("侧 stretch 权重 1"))))),

			// —— §7.3 第三版：em vs fixedPx（拖窗口跨 2000 档位看字号变化）——
			when(sel == 2, vbox(4.0f,
				textColored("拖窗口跨过 2000px（字号档位切换）：上排 fixed 跟随字号，下排 fixedPx 被裁",
					ImVec4(0.7f, 0.9f, 0.5f, 1)),
				vsplit(
					fixed("v3a", 44.0f, hbox(4.0f,   // 设计像素：随字号
						button("新", [] {}), button("打", [] {}), button("存", [] {}))),
					fixedPx("v3b", 44.0f, hbox(4.0f,  // 真像素：字号一大按钮被裁
						button("新", [] {}), button("打", [] {}), button("存", [] {}))),
					stretch("v3c", 1.0f, label("对照区"))))),

			// —— §7.4 第四版：min/atMost 约束三栏（终版：三层各司其职）——
			when(sel == 3, vbox(4.0f,
				textColored("侧栏带约束：宽窗 250 / 中窗至多 35% / 极窄 150 兜底",
					ImVec4(0.5f, 0.9f, 0.7f, 1)),
				hsplit(
					fixed("v4sb", 250.0f, label("侧栏\npreferred 250\nmin 150\natMost 35%"))
						.min(150.0f).atMost(0.35f),
					stretch("v4main", 1.0f, vbox(4.0f,
						label("主区（weight 1）"),
						widthFrac(1.0f, inputText("占满内容区", v4buf)))))))));

	});

	Mk::mkui<Mk::window>::run();
	return 0;
}