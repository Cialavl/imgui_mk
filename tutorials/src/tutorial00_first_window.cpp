// 教程 00：最小窗口。文档：lessons/lesson-00-setup-first-window.md
// 运行后看到一个窗口，帧数在跳、鼠标坐标实时跟随——证明"每帧回调"在工作。
// 关闭窗口（点 X）即退出。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>

int main() {
	// §0.3 构造即注册：这行创建了 GLFW 窗口 + 独立 ImGui 上下文 + 中文字体，
	// 并把窗口自动注册进全局引擎（§0.4）
	Mk::MainWindow<> mainWindow("我的第一个 mkui 窗口", 1280, 800);

	int frames = 0;   // 状态定义在回调外面：每帧回调只是"使用"它（第 02 课主题）

	// §0.5 注入每帧 UI：这里的代码每帧从头执行一次
	mainWindow.setUI([&] {
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("第 %d 帧", ++frames);
		ImGui::Text("鼠标：%.0f, %.0f（移动鼠标试试）", io.MousePos.x, io.MousePos.y);
		ImGui::Text("窗口右上角 X 关闭退出。");
	});

	// §0.3 引擎循环：驱动所有已注册窗口，全部关闭后才返回
	Mk::mkui<Mk::window>::run();
	return 0;
}