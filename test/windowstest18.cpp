#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cmath>
#include <iostream>
#include <memory>

// test18：wg 双顶层窗口联动——两棵 wg 树绑定同一组外部状态，
// 主窗 120 帧自动关闭，Inspector 检测到主窗销毁后跟随退出。退出码 0 = 联动正确。
int main() {
	Mk::MainWindow<> mainWin("wg 双窗口联动", 900, 600);

	// —— 共享状态（两棵树绑定同一组变量 = 跨窗实时联动）——
	int frames = 0, insFrames = 0, clicks = 0;
	float volume = 0.5f;
	bool mute = false;

	Mk::window inspector("Inspector", 420, 480);

	// —— 主窗 wg 树 ——
	Mk::wg::Button mBtn;
	Mk::wg::SliderF mSl;
	Mk::wg::CheckBox mCb;
	Mk::wg::LabelText mLt1, mLt2;
	Mk::wg::VLayout mainTree;
	
	mBtn.self.setName("点我（两窗计数联动）");
	mBtn.self.onClick = [&] { ++clicks; };
	mSl.self.name = "音量（共享）";
	mSl.self.value = &volume;
	mCb.self.name = "静音（共享）";
	mCb.self.checked = &mute;
	mLt1.self.name = "主窗帧";   mLt1.self.value = std::to_string(frames);
	mLt2.self.name = "点击数";   mLt2.self.value = std::to_string(clicks);
	mainTree.addWidget(mBtn.node);
	mainTree.addWidget(mSl.node);
	mainTree.addWidget(mCb.node);
	mainTree.addWidget(mLt1.node);
	mainTree.addWidget(mLt2.node);

	// —— Inspector wg 树（同一组状态）——
	Mk::wg::LabelText iLt1, iLt2;
	Mk::wg::SliderF iSl;
	Mk::wg::CheckBox iCb;
	Mk::wg::VLayout insTree;
	
	iLt1.self.name = "主窗帧";   iLt1.self.value = std::to_string(frames);
	iSl.self.name = "音量（同源）"; iSl.self.value = &volume;
	iCb.self.name = "静音（同源）"; iCb.self.checked = &mute;
	iLt2.self.name = "Inspector 帧"; iLt2.self.value = std::to_string(insFrames);
	insTree.addWidget(iLt1.node);
	insTree.addWidget(iSl.node);
	insTree.addWidget(iCb.node);
	insTree.addWidget(iLt2.node);

	mainWin.setUI([&] {
		++frames;
		volume = 0.5f + 0.5f * std::sin(frames * 0.03f);
		mLt1.self.value = std::to_string(frames);
		mLt2.self.value = std::to_string(clicks);
		mainTree.layout(Mk::wg::Rect{20, 20, 500, 400});
		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWin.getHandle(), true);
		}
	});

	inspector.setUserUI([&] {
		if(mainWin.getHandle() == nullptr) {   // 主窗已销毁 → 跟随退出
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		++insFrames;
		iLt1.self.value = std::to_string(frames);
		iLt2.self.value = std::to_string(insFrames);
		insTree.layout(Mk::wg::Rect{20, 20, 360, 400});
	});

	Mk::mkui<Mk::window>::run();

	const bool okFollow = (inspector.getHandle() == nullptr);   // 双窗均已销毁
	std::cout << "frames=" << frames << " insFrames=" << insFrames
		<< " clicks=" << clicks
		<< " inspectorGone=" << (okFollow ? "yes" : "no") << std::endl;
	const bool pass = okFollow;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}

