// 教程 02：立即模式与每帧 UI。文档：lessons/lesson-02-immediate-mode.md
// 四个实验区：计数器 / 条件渲染 / 正弦驱动 / 输入缓冲——全部状态外置。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <array>
#include <cmath>
#include <cstdio>

int main() {
	Mk::MainWindow<> mainWindow("教程02：立即模式", 1280, 800);

	// §2.2 全部状态外置：定义在 setUI 之外、活过 run() —— 本课唯一的规矩
	int frames = 0;
	int clicks = 0;
	bool showText = true;
	std::array<char, 128> buf{};
	std::snprintf(buf.data(), buf.size(), "初始内容（可编辑）");

	mainWindow.setUI([&] {
		++frames;
		const float t = (float)frames;
		ImGui::Text("第 %d 帧（%.1f FPS）", frames, ImGui::GetIO().Framerate);

		ImGui::SeparatorText("§2.5-1 计数器：点击的那帧 Button 才返回 true");
		if(ImGui::Button("点我"))
			++clicks;   // 只有点击的那一帧走进这里
		ImGui::SameLine();
		ImGui::Text("点击 %d 次", clicks);

		ImGui::SeparatorText("§2.5-2 条件渲染：if 决定这一帧控件是否存在");
		ImGui::Checkbox("显示下面这行文本", &showText);
		if(showText) {   // 不勾选时这行文本这一帧根本不存在（不是"隐藏"）
			ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.6f, 1), "我存在与否由每帧的 if 决定");
		}

		ImGui::SeparatorText("§2.5-3 正弦驱动：没人更新它，它自己会动");
		ImGui::ProgressBar(0.5f + 0.5f * std::sin(t * 0.05f), ImVec2(-FLT_MIN, 0), "自动动画");

		ImGui::SeparatorText("§2.5-4 输入缓冲：状态在你手里，跨帧保留");
		ImGui::InputText("可编辑", buf.data(), (int)buf.size());
		ImGui::TextDisabled("缓冲区是 main 栈上的 std::array，ImGui 每帧读写它");

		ImGui::SeparatorText("实验点");
		ImGui::TextDisabled("把 ++clicks 移到 if 外面？把 showText 改成 frames%%2？动手试试（§2.7）");
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}