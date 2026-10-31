#pragma once
// gallery Inspector 子窗口：独立文件分层。
// 子窗口挂载语义保持不变（构造即创建窗口并 mountChild 到父窗口）；
#include "gallery_chrome.h"
#include <memory>

namespace gallery {


// ============ Inspector 窗口 UI ============

struct InsUi {
// 根用 SubWindow：内容超出窗口时由子窗口滚动，不再被窗口下缘硬切
using Root = Mk::wg2::SubWindow<
Mk::wg2::Id("root"),
Mk::wg2::SeparatorText<Mk::wg2::Id("head")>,
Mk::wg2::SliderF<Mk::wg2::Id("volume")>,
Mk::wg2::CheckBox<Mk::wg2::Id("mute")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit1")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit2")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit3")>,
Mk::wg2::SeparatorText<Mk::wg2::Id("framesHead")>,
Mk::wg2::LabelText<Mk::wg2::Id("galFrames")>,
Mk::wg2::LabelText<Mk::wg2::Id("insFrames")>,
Mk::wg2::TextWrapped<Mk::wg2::Id("note")>
>;

Root root;
};

inline void buildIns(InsUi& u, State& s, Mk::window& w) {
u.root.border = false;
u.root.idname = "inspector_scroll";
u.root.at<Mk::wg2::Id("head")>().name = "Shared state (live link with Gallery)";
u.root.at<Mk::wg2::Id("volume")>().name = "Volume";
u.root.at<Mk::wg2::Id("volume")>().value = &s.volume;
u.root.at<Mk::wg2::Id("mute")>().name = "Mute";
u.root.at<Mk::wg2::Id("mute")>().checked = &s.mute;
u.root.at<Mk::wg2::Id("bit1")>().name = "Bit A"; u.root.at<Mk::wg2::Id("bit1")>().flags = &s.bitFlags; u.root.at<Mk::wg2::Id("bit1")>().mask = 1;
u.root.at<Mk::wg2::Id("bit2")>().name = "Bit B"; u.root.at<Mk::wg2::Id("bit2")>().flags = &s.bitFlags; u.root.at<Mk::wg2::Id("bit2")>().mask = 2;
u.root.at<Mk::wg2::Id("bit3")>().name = "Bit C"; u.root.at<Mk::wg2::Id("bit3")>().flags = &s.bitFlags; u.root.at<Mk::wg2::Id("bit3")>().mask = 4;
u.root.at<Mk::wg2::Id("framesHead")>().name = "Independent frame loops";
u.root.at<Mk::wg2::Id("galFrames")>().name = "Gallery frame";
u.root.at<Mk::wg2::Id("galFrames")>().valueFn = [&s] { return std::to_string(s.win->frameIndex()); };
u.root.at<Mk::wg2::Id("insFrames")>().name = "Inspector frame";
u.root.at<Mk::wg2::Id("insFrames")>().valueFn = [&w] { return std::to_string(w.frameIndex()); };
u.root.at<Mk::wg2::Id("note")>().text = "Two top-level windows each own an ImGui context and frame loop; "
"binding the same external variables keeps them live-linked. "
"Clicking X only hides the Inspector; it follows the Gallery out when the Gallery closes.";
}

// ============ Inspector 子窗口（Qt 风格装配 struct）============

struct InspectorWindow {
InsUi ui;                             // Inspector 界面树（buildIns 装配）
std::unique_ptr<Mk::window> win;      // 子窗口（父窗口经 mountChild 亦持有共享引用）

// 构造即装配并挂载到父窗口：中央挂载 + 帧回调 + 父关随关/点 X 隐藏
		InspectorWindow(Mk::window& parent, State& s) {
			const float uiDpi = Mk::wg2::dpiScale();
			const auto [w, h] = Mk::workAreaSize(0.3f, 0.7f,
				static_cast<int>(420.0f * uiDpi), static_cast<int>(640.0f * uiDpi));
			win = std::make_unique<Mk::window>("Inspector", w, h);
			buildIns(ui, s, *win);   // 装配界面树（帧序号等由窗口自己提供）
			win->setAutoScaleFont(true);
			win->setCentralWidget(ui.root);   // 静态直挂：窗口每帧自动布局
			s.inspectorHandle = win->getHandle();
			s.inspectorWin = win.get();
			parent.mountChild(*win);
}
};

} // namespace gallery
