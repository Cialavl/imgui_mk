#pragma once
// gallery Inspector 子窗口：独立文件分层（装配模式与主窗 GalleryWindow 同构）。
// 子窗口挂载语义：构造即创建窗口并 mountChild 到父窗口（父关随关、点 X 隐藏）；
// 界面树经 buildIns 一次性装配，显示内容全部由共享状态绑定/拉取式 valueFn 自动更新。
#include "gallery_chrome.h"
#include <memory>

namespace gallery {

using namespace Mk::wg;

// ============ Inspector 窗口 UI ============

struct InsUi {
	SeparatorText head;
	SliderF volume;
	CheckBox mute;
	CheckboxFlags bit1, bit2, bit3;
	SeparatorText framesHead;
	LabelText galFrames, insFrames;
	TextWrapped note;
	VLayout root;
};

inline void buildIns(InsUi& u, State& s) {
	u.head.self.name = "Shared state (live link with Gallery)";
	u.volume.self.name = "Volume";
	u.volume.self.value = &s.volume;
	u.mute.self.name = "Mute";
	u.mute.self.checked = &s.mute;
	u.bit1.self.name = "Bit A"; u.bit1.self.flags = &s.bitFlags; u.bit1.self.mask = 1;
	u.bit2.self.name = "Bit B"; u.bit2.self.flags = &s.bitFlags; u.bit2.self.mask = 2;
	u.bit3.self.name = "Bit C"; u.bit3.self.flags = &s.bitFlags; u.bit3.self.mask = 4;
	u.framesHead.self.name = "Independent frame loops";
	u.galFrames.self.name = "Gallery frame";
	u.galFrames.self.valueFn = [&s] { return std::to_string(s.frames); };   // 拉取式绑定，免每帧赋值
	u.insFrames.self.name = "Inspector frame";
	u.insFrames.self.valueFn = [&s] { return std::to_string(s.insFrames); };
	u.note.self.name = "Two top-level windows each own an ImGui context and frame loop; "
		"binding the same external variables keeps them live-linked. "
		"Clicking X only hides the Inspector; it follows the Gallery out when the Gallery closes.";
	for(Node* nd : {&u.head.node, &u.volume.node, &u.mute.node, &u.bit1.node,
		&u.bit2.node, &u.bit3.node, &u.framesHead.node, &u.galFrames.node,
		&u.insFrames.node, &u.note.node}) {
		u.root.addWidget(*nd);
	}
}

// ============ Inspector 子窗口（Qt 风格装配 struct）============

struct InspectorWindow {
	InsUi ui;                             // Inspector 界面树（buildIns 装配）
	std::unique_ptr<Mk::window> win;      // 子窗口（父窗口经 mountChild 亦持有共享引用）

	// 构造即装配并挂载到父窗口：中央挂载 + 帧回调 + 父关随关/点 X 隐藏
	InspectorWindow(Mk::window& parent, State& s) {
		buildIns(ui, s);
		const auto [w, h] = Mk::workAreaSize(0.3f, 0.7f, 420, 640);
		win = std::make_unique<Mk::window>("Inspector", w, h);
		win->setAutoScaleFont(true);
		win->setCentralWidget(ui.root.node);   // 中央挂载（引擎每帧自动布局）
		win->setUserUI([&s] { ++s.insFrames; });   // 换肤由引擎广播、跟随退出/点 X 隐藏由挂载语义接管
		s.inspectorHandle = win->getHandle();     // GLFW 裸句柄：toggleInspector 用
		parent.mountChild(*win);                  // 挂载到父窗口：父关随关、子 X 隐藏
	}
};

} // namespace gallery
