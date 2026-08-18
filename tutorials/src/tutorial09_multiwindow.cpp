// 教程 09：多窗口编程。文档：lessons/lesson-09-multi-window.md
// 双顶层窗口：异肤（独立上下文）+ 共享状态联动（同一批变量）+
// 检查器模式（X 否决隐藏 + 跟随退出）。默认两窗都可被 X 关闭；
// 勾选"拦截 X"后检查器 X 只隐藏（主窗可复活它）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <string>

int main() {
	// §9.1 两个顶层窗口（构造即注册）
	Mk::MainWindow<> main("主窗口", 1200, 800);
	Mk::window inspector("检查器", 420, 560);

	// §9.2 独立上下文：各一套皮肤互不影响
	main.setTheme(Mk::fui::Theme::dark());
	inspector.setTheme(Mk::fui::Theme::light());

	// §9.3 共享状态（main 栈上，两个回调都引用捕获）
	int mainFrames = 0, insFrames = 0;
	float volume = 0.6f;
	bool mute = false, veto = false;

	// §9.4 检查器模式：拦截 X（勾选后启用）
	inspector.setUserUI([&] {
		++insFrames;
		if(main.getHandle() == nullptr) {   // 主窗已销毁 → 跟随退出
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		using namespace Mk::fui;
		draw(vbox(8.0f,
			separatorText("§9.3 共享状态（与主窗实时联动）"),
			slider("音量", volume, 0.0f, 1.0f),
			checkbox("静音", mute),
			progressBar(volume, ImVec2(-FLT_MIN, 0)),
			separatorText("§9.3 独立帧循环（帧号不同步）"),
			labelText("主窗帧", std::to_string(mainFrames)),
			labelText("本窗帧", std::to_string(insFrames)),
			separatorText("§9.4 检查器模式"),
			checkbox("拦截 X（点 X 只隐藏）", veto),
			when(veto, textColored("现在点右上角 X：窗口只隐藏不退出（主窗可复活）",
				ImVec4(1.0f, 0.8f, 0.2f, 1.0f)))));
	});

	main.setUI([&] {
		++mainFrames;
		volume = 0.5f + 0.5f * std::sin(mainFrames * 0.02f);   // 逻辑侧驱动联动

		// §9.4 否决开关同步（每帧设置，setter 幂等）
		// §9.4 否决开关同步（每帧设置，setter 幂等；检查器可能已关闭销毁——空指针保护）
		if(GLFWwindow* insp = inspector.getHandle()) {
			if(veto) {
				glfwSetWindowCloseCallback(insp, [](GLFWwindow* w) {
					glfwSetWindowShouldClose(w, GLFW_FALSE);
					glfwHideWindow(w);
				});
			}
			else {
				glfwSetWindowCloseCallback(insp, nullptr);
			}
		}

		using namespace Mk::fui;
		draw(vbox(8.0f,
			separatorText("§9.3 共享状态（拖滑块 → 检查器同帧跟随）"),
			slider("音量", volume, 0.0f, 1.0f),
			checkbox("静音", mute),
			labelText("检查器帧", std::to_string(insFrames)),
			labelText("本窗帧", std::to_string(mainFrames)),
			separatorText("§9.4 复活隐藏的检查器"),
			button("显示检查器", [&] {
				if(inspector.getHandle()) glfwShowWindow(inspector.getHandle());
			}),
			textWrapped("先勾选检查器的\"拦截 X\"，点其右上角 X 隐藏，再用本按钮复活；"
				"两窗都关（或主窗关后检查器跟随）run() 才返回。")));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
