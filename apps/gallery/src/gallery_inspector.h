#pragma once
// gallery Inspector 子窗口：独立文件分层。
// 子窗口挂载语义保持不变（构造即创建窗口并 mountChild 到父窗口）；
#include "gallery_chrome.h"
#include <memory>

namespace gallery {


// ============ Inspector 窗口 UI ============

struct InsUi {
using Root = Mk::wg2::VBox<
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

inline void buildIns(InsUi& u, State& s) {
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
u.root.at<Mk::wg2::Id("galFrames")>().valueFn = [&s] { return std::to_string(s.frames); };
u.root.at<Mk::wg2::Id("insFrames")>().name = "Inspector frame";
u.root.at<Mk::wg2::Id("insFrames")>().valueFn = [&s] { return std::to_string(s.insFrames); };
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
buildIns(ui, s);
const auto [w, h] = Mk::workAreaSize(0.3f, 0.7f, 420, 640);
win = std::make_unique<Mk::window>("Inspector", w, h);
win->setAutoScaleFont(true);
win->setCentralWidget([&](const Mk::wg2::Rect& r) {
Mk::wg2::layout(ui.root, r);
});   // 中央挂载 wg2 回调
win->setUserUI([&s] { ++s.insFrames; });
s.inspectorHandle = win->getHandle();
parent.mountChild(*win);
}
};

} // namespace gallery
