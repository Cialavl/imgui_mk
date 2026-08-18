// 教程 01：窗口生命周期。文档：lessons/lesson-01-window-lifecycle.md
// 实验区：手动触发关闭请求观察状态表；按 V 切换 X 否决模式（检查器模式）。
// 默认 X 直接退出（否决模式关）；开启否决后 X 只隐藏，S 复活，再关否决或
// 用按钮请求关闭才真正退出。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <cstdio>

int main() {
	Mk::MainWindow<> mainWindow("教程01：窗口生命周期", 1280, 800);
	GLFWwindow* handle = mainWindow.getHandle();

	int frames = 0;
	bool vetoMode = false;    // X 否决模式（检查器模式）
	bool hidden = false;

	// §1.4 否决回调：只否决 + 隐藏，不做重活。切换开关时安装/恢复
	auto installVeto = [&](bool on) {
		if(on) {
			glfwSetWindowCloseCallback(handle, [](GLFWwindow* w) {
				glfwSetWindowShouldClose(w, GLFW_FALSE);   // 否决：不进引擎销毁流程
				glfwHideWindow(w);                          // 只是隐藏
			});
		}
		else {
			glfwSetWindowCloseCallback(handle, nullptr);   // 恢复默认：X = 请求关闭
		}
	};

	mainWindow.setUI([&] {
		++frames;
		ImGui::Text("第 %d 帧", frames);
		ImGui::SeparatorText("§1.3 状态表（点按钮前后对比）");
		ImGui::Text("getHandle()   = %p", (void*)mainWindow.getHandle());
		ImGui::Text("shouldClose() = %s", mainWindow.shouldClose() ? "true" : "false");
		ImGui::Text("（把手仍然有效：关闭后查询类调用是安全的空操作）");

		ImGui::SeparatorText("§1.5 正确姿势：回调里只说想关，引擎来真关");
		if(ImGui::Button("手动请求关闭（glfwSetWindowShouldClose）")) {
			// 千万不要在这里调 mainWindow.close()——本帧渲染还没结束（§1.5）
			glfwSetWindowShouldClose(handle, true);
		}

		ImGui::SeparatorText("§1.4 X 否决模式（检查器模式）");
		if(ImGui::Button(vetoMode ? "关闭否决模式 (V)" : "开启否决模式 (V)")) {
			vetoMode = !vetoMode;
			installVeto(vetoMode);
		}
		if(vetoMode) {
			ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1),
				"否决已开启：现在点 X 只会隐藏窗口（试试！），按 S 复活");
			if(hidden) {
				if(ImGui::Button("显示窗口 (S)") || ImGui::IsKeyPressed(ImGuiKey_S)) {
					glfwShowWindow(handle);
					hidden = false;
				}
			}
			// 我们无法在否决回调里直接改 hidden（简单起见每帧查可见性）
			hidden = (glfwGetWindowAttrib(handle, GLFW_VISIBLE) == 0);
		}
		else {
			ImGui::TextDisabled("否决模式关闭：点 X 程序直接退出");
		}
		ImGui::SeparatorText("练习/实验");
		ImGui::TextDisabled("试着把 §1.5 的按钮改成直接调 close() 看看会怎样（会崩溃，读文档 §1.5）");
	});

	Mk::mkui<Mk::window>::run();
	std::puts("run() 已返回：所有窗口关闭，程序干净退出");
	return 0;
}