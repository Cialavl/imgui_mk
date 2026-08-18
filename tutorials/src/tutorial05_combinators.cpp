// 教程 05：组合子与修饰器。文档：lessons/lesson-05-combinators.md
// 每个修饰器一个实验区；两个同名按钮独立计数；宽度三兄弟对照。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <string>

int main() {
	Mk::MainWindow<> mainWindow("教程05：组合子与修饰器", 1280, 800);

	// 状态全外置
	bool mute = false, showAdvanced = false, isAdmin = false;
	float volume = 0.6f, dragV = 1.0f;
	int clicksA = 0, clicksB = 0;
	int frames = 0;
	std::array<char, 64> inA{}, inB{}, inC{};
	const std::string longPath =
		"C:\\Users\\someone\\Documents\\projects\\very\\deep\\dir\\file.txt";

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		draw(vbox(8.0f,
			separatorText("§5.2 disabled vs when"),
			hbox(8.0f,
				checkbox("静音", mute),
				disabled(slider("音量（静音时禁用）", volume, 0.0f, 1.0f))),
			checkbox("管理员模式", isAdmin),
			when(isAdmin, textColored("危险区：删除全部（管理员才可见）",
				ImVec4(0.9f, 0.4f, 0.4f, 1.0f))),

			separatorText("§5.3 withId：两个同名按钮独立可点"),
			hbox(8.0f,
				withId("a", button("确定", [&] { ++clicksA; })),
				withId("b", button("确定", [&] { ++clicksB; }))),
			labelText("左", std::to_string(clicksA)),
			labelText("右", std::to_string(clicksB)),

			separatorText("§5.4 宽度三兄弟"),
			itemWidth(220.0f, inputText("定宽 220", inA)),
			widthFrac(0.5f, inputText("占可用宽 50%", inB)),
			withMinWidth(300.0f, inputText("至少 300（更窄就撑开）", inC)),

			separatorText("§5.1 更多修饰器叠加"),
			tooltip(colored(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.6f, 1.0f),
				indent(24.0f, label("悬停我看提示：colored+indent+tooltip"))),
				"三个修饰器的叠加，顺序即语义"),
			tooltip(labelEllipsis(longPath), longPath),

			separatorText("实验点"),
			checkbox("展开高级区", showAdvanced),
			when(showAdvanced, dragF("高级参数（when 控制显隐）", dragV, 0.01f, 0.0f, 10.0f)),
			labelText("帧", std::to_string(frames))));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}