// 教程 04：函数式核心。文档：lessons/lesson-04-functional-core.md
// 每帧重建整棵视图值；同一变量绑定多个控件实时联动；自定义 ADL 控件。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>

namespace {

// §4.2 自定义控件（ADL 三步法的雏形：结构体 + draw 重载，无需继承）：
// 放在自己的命名空间，容器/修饰器经依赖实参查找自动发现
struct FrameCounter {
	int& frames;
};
void draw(const FrameCounter& f) {
	ImGui::Text("帧 %d（自定义 ADL 控件 FrameCounter）", f.frames);
}

struct SineWave {
	int& frames;
	float& value;
};
void draw(const SineWave& s) {
	s.value = 0.5f + 0.5f * std::sin(s.frames * 0.05f);   // 逻辑侧驱动状态
	ImGui::Text("正弦值 %.3f（本控件每帧写回绑定变量）", s.value);
}

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程04：函数式核心", 1280, 800);

	// §4.3 状态全外置（main 栈上）；视图每帧重建，无生命周期负担
	int frames = 0;
	float volume = 0.6f;   // 同一变量将被两处控件绑定 → 实时联动
	int clicks = 0;

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		// 每帧构造整棵视图树（值语义临时值，帧末整体销毁）
		draw(vbox(8.0f,
			label("§4.1 控件是纯值：本树每帧从零构造，无基类无虚函数"),
			FrameCounter{frames},          // §4.2 自定义控件与库控件平权
			SineWave{frames, volume},
			separatorText("§4.3 同一变量绑定两个控件"),
			slider("音量 A（树上方）", volume, 0.0f, 1.0f),
			subwindow("联动子窗口", ImVec2(0, 90), true,
				slider("音量 B（子窗口内，同一 float&）", volume, 0.0f, 1.0f)),
			hbox(8.0f,
				button("点我", [&] { ++clicks; }),
				labelText("点击", std::to_string(clicks))),
			label("拖动任一音量滑块：另一处同帧跟随（读的是同一个变量）")));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}