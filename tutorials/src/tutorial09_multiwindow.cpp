// 教程 09：多窗口编程。文档：lessons/lesson-09-multi-window.md
// 双顶层窗口（构造即注册）+ 共享状态（同一组外部变量）+ 否决模式（X 仅隐藏）+
// 跟随退出（主窗关闭后 run() 才返回）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <cmath>
#include <string>

int main() {
	// 构造即注册
	Mk::MainWindow<> main("主窗口", 1200, 800);
	Mk::window inspector("检查器", 420, 560);

	// 每窗口独立 ImGui 上下文 → 各持一套皮肤
	main.setTheme(Mk::wg::Theme::dark());
	inspector.setTheme(Mk::wg::Theme::light());

	// 共享状态（main 栈上，两棵 wg 树绑定同一组变量 = 实时联动）
	int mainFrames = 0, insFrames = 0;
	float volume = 0.6f;
	bool mute = false, veto = false;

	// —— 检查器窗口树 ——
	Mk::wg::SeparatorText iSec1, iSec2, iSec3;
	iSec1.self.name = "共享状态（与主窗口实时联动）";
	iSec2.self.name = "独立帧循环（帧号不同步）";
	iSec3.self.name = "否决模式";
	Mk::wg::SliderF iVol;
	iVol.self.name = "音量";
	iVol.self.value = &volume;
	Mk::wg::CheckBox iMute;
	iMute.self.name = "静音";
	iMute.self.checked = &mute;
	Mk::wg::ProgressBar iBar;
	iBar.self.fraction = &volume;
	Mk::wg::LabelText iMainF, iInsF;
	iMainF.self.name = "主窗帧";
	iInsF.self.name = "检查器帧";
	Mk::wg::CheckBox iVeto;
	iVeto.self.name = "启用否决（X 只隐藏）";
	iVeto.self.checked = &veto;
	Mk::wg::TextColored iVetoNote;
	iVetoNote.self.name = "已启用：右上角 X 只隐藏不退出（菜单可再开）";
	iVetoNote.self.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
	Mk::wg::When<Mk::wg::TextColored> iVetoWhen;
	Mk::wg::Layout insTree;
	insTree.self.mode = Mk::wg::LayoutMode::V;
	insTree.node.add(iSec1.node);
	insTree.node.add(iVol.node);
	insTree.node.add(iMute.node);
	insTree.node.add(iBar.node);
	insTree.node.add(iSec2.node);
	insTree.node.add(iMainF.node);
	insTree.node.add(iInsF.node);
	insTree.node.add(iSec3.node);
	insTree.node.add(iVeto.node);
	insTree.node.add(iVetoWhen.node);

	inspector.setUserUI([&] {
		++insFrames;
		if(main.getHandle() == nullptr) {   // 主窗已销毁 → 跟随退出
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		iMainF.self.value = std::to_string(mainFrames);
		iInsF.self.value = std::to_string(insFrames);
		iVetoWhen.self.show = veto;
		insTree.layout(Mk::wg::Rect{20, 20, 380, 520});
	});

	// —— 主窗口树 ——
	Mk::wg::SeparatorText mSec1, mSec2;
	mSec1.self.name = "共享状态（滑块在两侧同帧联动）";
	mSec2.self.name = "否决模式的安装";
	Mk::wg::SliderF mVol;
	mVol.self.name = "音量";
	mVol.self.value = &volume;
	Mk::wg::CheckBox mMute;
	mMute.self.name = "静音";
	mMute.self.checked = &mute;
	Mk::wg::LabelText mInsF, mMainF;
	mInsF.self.name = "检查器帧";
	mMainF.self.name = "主窗帧";
	Mk::wg::Button mShow;
	mShow.self.setName("显示检查器");
	mShow.self.onClick = [&] {
		if(inspector.getHandle()) glfwShowWindow(inspector.getHandle());
	};
	Mk::wg::TextWrapped mNote;
	mNote.self.name = "先勾选检查器里的“启用否决”，再点其右上角 X：窗口只隐藏；"
		"用本按钮唤回。主窗口关闭后（引擎销毁），检查器下一帧跟随退出，run() 才返回。";

	Mk::wg::Layout mainTree;
	mainTree.self.mode = Mk::wg::LayoutMode::V;
	mainTree.node.add(mSec1.node);
	mainTree.node.add(mVol.node);
	mainTree.node.add(mMute.node);
	mainTree.node.add(mInsF.node);
	mainTree.node.add(mMainF.node);
	mainTree.node.add(mSec2.node);
	mainTree.node.add(mShow.node);
	mainTree.node.add(mNote.node);

	main.setUI([&] {
		++mainFrames;
		volume = 0.5f + 0.5f * std::sin(mainFrames * 0.02f);   // 逻辑侧驱动

		// 每帧同步否决回调（setter 幂等）
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

		mInsF.self.value = std::to_string(insFrames);
		mMainF.self.value = std::to_string(mainFrames);
		mainTree.layout(Mk::wg::Rect{20, 20, 900, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
