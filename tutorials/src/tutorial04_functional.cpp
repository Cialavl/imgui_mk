// 教程 04：wg 原子控件。文档：lessons/lesson-04-functional-core.md
// 控件 = 模板 struct（数据 + 功能函数），组合成树后长期存活；
// 状态外置绑定（指针成员），同一变量被多个控件绑定 = 实时联动。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cmath>
#include <string>

namespace {

// 自定义控件 1：帧计数器（纯显示，数据 = 指针绑定）
struct FrameCounterImpl {
	int* frames = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(frames) ImGui::Text("帧 %d（自定义 wg 控件 FrameCounter）", *frames);
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) {
		return Mk::wg::Size{ImGui::CalcTextSize("帧 999999（自定义 wg 控件 FrameCounter）").x, 20.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using FrameCounter = Mk::wg::Control<FrameCounterImpl, Mk::wg::DefaultStyle>;

// 自定义控件 2：正弦驱动器（渲染时写回绑定变量 = 逻辑与视图同帧）
struct SineWaveImpl {
	int* frames = nullptr;
	float* value = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(frames && value) {
			*value = 0.5f + 0.5f * std::sin(*frames * 0.05f);   // 逻辑侧驱动状态
			ImGui::Text("正弦值 %.3f（控件每帧写回绑定变量）", *value);
		}
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) {
		return Mk::wg::Size{ImGui::CalcTextSize("正弦值 0.000（控件每帧写回绑定变量）").x, 20.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using SineWave = Mk::wg::Control<SineWaveImpl, Mk::wg::DefaultStyle>;

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程04：wg 原子控件", 1280, 800);

	// 状态全部外置；视图树在 main 组装一次，长期存活
	int frames = 0;
	float volume = 0.6f;   // 同一变量被两个滑块绑定 → 实时联动
	int clicks = 0;

	FrameCounter counter;
	counter.self.frames = &frames;
	SineWave wave;
	wave.self.frames = &frames;
	wave.self.value = &volume;

	Mk::wg::SeparatorText sep;
	sep.self.name = "同一变量绑两个滑块";

	Mk::wg::SliderF volA, volB;
	volA.self.name = "音量 A（树上部）";
	volA.self.value = &volume;
	volB.self.name = "音量 B（子窗口内，同一 float&）";
	volB.self.value = &volume;

	Mk::wg::SubWindow<> sub;
	sub.node.idname = "sub4";
	sub.self.height = 90.0f;
	sub.node.add(volB.node);

	Mk::wg::Button btn;
	btn.self.setName("计数");
	btn.self.onClick = [&] { ++clicks; };
	Mk::wg::LabelText clicksLt;
	clicksLt.self.name = "点击";
	Mk::wg::Layout btnRow;
	btnRow.self.mode = Mk::wg::LayoutMode::H;
	btnRow.node.add(btn.node);
	btnRow.node.add(clicksLt.node);

	Mk::wg::Label note;
	note.self.setName("拖动任意滑块：另一处同帧跟随（绑定同一变量）");

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(counter.node);
	root.node.add(wave.node);
	root.node.add(sep.node);
	root.node.add(volA.node);
	root.node.add(sub.node);
	root.node.add(btnRow.node);
	root.node.add(note.node);

	mainWindow.setUI([&] {
		++frames;
		clicksLt.self.value = std::to_string(clicks);
		root.layout(Mk::wg::Rect{20, 20, 900, 700});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
