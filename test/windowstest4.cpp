#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

// 帧驱动器：每帧推进逻辑侧状态（放在树尾，保证其余控件已渲染完本帧）
struct FrameDriver : Mk::ui::Widget {
	int& frames;
	float& volume;
	Mk::ui::Label& status;
	Mk::ui::Widget& hideAt30;   // 第 30 帧隐藏（其后置于此驱动器）
	Mk::ui::VBox& dynZone;
	Mk::window& win;

	FrameDriver(int& f, float& v, Mk::ui::Label& s, Mk::ui::Widget& h, Mk::ui::VBox& z, Mk::window& w)
		: frames(f), volume(v), status(s), hideAt30(h), dynZone(z), win(w) {}

protected:
	void render() override {
		++frames;
		// 逻辑侧改绑定变量，UI 实时跟随（数据绑定验证）
		volume = 0.5f + 0.5f * std::sin(frames * 0.05f);
		// setText 传临时字符串：控件持副本，悬垂修复验证
		status.setText(
			"帧=" + std::to_string(frames)
			+ "  音量=" + std::to_string(volume)
			+ "  动态子控件=" + std::to_string(dynZone.childCount()));

		if(frames == 30) {
			hideAt30.setVisible(false);
		}
		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	}
};

// 渲染探针：统计自己 render() 被调用的次数（可见性/折叠门控验证）
struct Probe : Mk::ui::Widget {
	int renders = 0;

protected:
	void render() override { ++renders; }
};

// 动态树驱动器：渲染回调内增删子控件（墓碑与快照迭代验证）
struct DynDriver : Mk::ui::Widget {
	int& frames;
	Mk::ui::VBox& zone;
	std::vector<std::unique_ptr<Mk::ui::Label>>& pool;
	int& adds;
	int& removes;

	DynDriver(int& f, Mk::ui::VBox& z, std::vector<std::unique_ptr<Mk::ui::Label>>& p, int& a, int& r)
		: frames(f), zone(z), pool(p), adds(a), removes(r) {}

protected:
	void render() override {
		if(frames % 10 == 5) {
			pool.push_back(std::make_unique<Mk::ui::Label>(
				("动态#" + std::to_string(++adds)).c_str()));
			zone.add(*pool.back());   // 渲染中 add：本帧不画，下一帧生效
		}
		if(frames >= 60 && frames % 10 == 0 && !pool.empty()) {
			if(zone.remove(*pool.back())) {  // 渲染中 remove：墓碑，立即跳过
				pool.pop_back();
				++removes;
			}
		}
	}
};

} // namespace

// 控件树优化验证：统一 draw() 入口（可见性/禁用/ID 作用域）、动态增删子控件、
// 布局容器（VBox/HBox 间距、Group、CollapsibleHeader）、字符串副本、setWidget 挂载。
// 渲染 120 帧后自动请求关闭，全部断言通过输出 PASS 并以退出码 0 结束。
int main() {
	Mk::MainWindow<> mainWindow("控件树优化验证", 1280, 800);

	// —— 被绑定的状态（存活期覆盖 run()，与控件树同作用域）——
	float volume = 0.8f;
	bool  mute = false;
	int   frames = 0;
	int   adds = 0, removes = 0;
	std::vector<std::unique_ptr<Mk::ui::Label>> pool;   // 动态子控件所有权池

	// —— 组装控件树 ——
	Mk::ui::VBox root(8.0f);

	Mk::ui::Label status("状态文本");
	Mk::ui::Label probeVisibleHint("本行第 30 帧后隐藏");

	Mk::ui::CollapsibleHeader headerOpen("折叠容器(默认展开)", ImGuiTreeNodeFlags_DefaultOpen);
	Mk::ui::Label openHint("展开时才渲染的子控件");
	Probe probeOpen;

	Mk::ui::CollapsibleHeader headerClosed("折叠容器(默认收起)");
	Probe probeClosed;   // 收起状态：render 永远不会被调用

	// 两个同 label 按钮：PushID(this) 作用域隔离验证（不再 ID 冲突）
	Mk::ui::HBox buttonRow(8.0f);
	Mk::ui::Button sameA("同按钮");
	Mk::ui::Button sameB("同按钮");

	// 分组：多控件打包成一个整体参与外部布局
	Mk::ui::Group group;
	Mk::ui::Label groupHint("分组内：滑块与复选框绑定外部变量");
	Mk::ui::SliderF vol("音量", volume, 0.0f, 1.0f);
	Mk::ui::CheckBox muteBox("静音", mute);

	// 嵌套子窗口：内部控件与外部滑块绑定同一变量
	Mk::ui::SubWindow panel("嵌套子窗口", ImVec2(520, 120));
	Mk::ui::Label panelText("子窗口内的滑块与外部绑定同一变量");
	Mk::ui::SliderF innerVol("内部音量", volume, 0.0f, 1.0f);

	// 动态增删区
	Mk::ui::VBox dynZone(0.0f);

	Probe probeVisible;   // 可见性门控探针（放在 FrameDriver 之前）
	FrameDriver frameDriver(frames, volume, status, probeVisible, dynZone, mainWindow.getWindow());
	DynDriver dynDriver(frames, dynZone, pool, adds, removes);

	headerOpen.add(openHint);
	headerOpen.add(probeOpen);
	headerClosed.add(probeClosed);
	buttonRow.add(sameA);
	buttonRow.add(sameB);
	group.add(groupHint);
	group.add(vol);
	group.add(muteBox);
	panel.add(panelText);
	panel.add(innerVol);

	root.add(headerOpen);
	root.add(headerClosed);
	root.add(buttonRow);
	root.add(group);
	root.add(panel);
	root.add(dynZone);
	root.add(probeVisibleHint);
	root.add(probeVisible);
	root.add(status);
	root.add(frameDriver);
	root.add(dynDriver);

	// 挂载控件树（新接口：替代手写 setUserUI 桥接）
	mainWindow.setWidget(root);

	Mk::mkui<Mk::window>::run();

	// —— 验证结论（全部为帧计数确定性断言）——
	const bool okFrames = (frames == 120);
	const bool okVisible = (probeVisible.renders == 30);        // 第 30 帧后隐藏
	const bool okOpen = (probeOpen.renders == frames);          // 默认展开：每帧渲染
	const bool okClosed = (probeClosed.renders == 0);           // 默认收起：从不渲染
	const bool okDyn = (adds == 12 && removes == 7 && dynZone.childCount() == 5);
	const bool okExit = (mainWindow.getHandle() == nullptr);    // 引擎真正关闭并销毁窗口

	std::cout << "frames=" << frames
		<< " probeVisible=" << probeVisible.renders
		<< " probeOpen=" << probeOpen.renders
		<< " probeClosed=" << probeClosed.renders
		<< " adds=" << adds
		<< " removes=" << removes
		<< " dynChilds=" << dynZone.childCount() << std::endl;

	const bool pass = okFrames && okVisible && okOpen && okClosed && okDyn && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", 可见性门控 " << okVisible
		<< ", 展开渲染 " << okOpen
		<< ", 收起不渲染 " << okClosed
		<< ", 动态增删 " << okDyn
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}