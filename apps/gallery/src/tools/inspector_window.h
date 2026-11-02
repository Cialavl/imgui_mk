#pragma once
// gallery Inspector 子窗口：独立文件分层。
// 子窗口挂载语义保持不变（构造即创建窗口并 mountChild 到父窗口）；
#include "../content_area/demo_page.h"
#include "../menu_bar.h"
#include <imgui_mk/ui_context.h>
#include <imgui_mk/window.h>
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

	// 本窗口自己的演示数据（不与其它窗口共享）
	float volume = 0.6f;
	bool mute = false;
	int bitFlags = 0b101;
};

inline void buildIns(InsUi& u, Mk::window* parentWin, Mk::window& w) {
u.root.border = false;
u.root.idname = "inspector_scroll";
u.root.at<Mk::wg2::Id("head")>().name = "Independent window, own demo data";
u.root.at<Mk::wg2::Id("volume")>().name = "Volume";
u.root.at<Mk::wg2::Id("volume")>().value = &u.volume;
u.root.at<Mk::wg2::Id("mute")>().name = "Mute";
u.root.at<Mk::wg2::Id("mute")>().checked = &u.mute;
u.root.at<Mk::wg2::Id("bit1")>().name = "Bit A"; u.root.at<Mk::wg2::Id("bit1")>().flags = &u.bitFlags; u.root.at<Mk::wg2::Id("bit1")>().mask = 1;
u.root.at<Mk::wg2::Id("bit2")>().name = "Bit B"; u.root.at<Mk::wg2::Id("bit2")>().flags = &u.bitFlags; u.root.at<Mk::wg2::Id("bit2")>().mask = 2;
u.root.at<Mk::wg2::Id("bit3")>().name = "Bit C"; u.root.at<Mk::wg2::Id("bit3")>().flags = &u.bitFlags; u.root.at<Mk::wg2::Id("bit3")>().mask = 4;
u.root.at<Mk::wg2::Id("framesHead")>().name = "Independent frame loops";
u.root.at<Mk::wg2::Id("galFrames")>().name = "Gallery frame";
u.root.at<Mk::wg2::Id("galFrames")>().valueFn = [parentWin] { return parentWin ? std::to_string(parentWin->frameIndex()) : std::string("-"); };
u.root.at<Mk::wg2::Id("insFrames")>().name = "Inspector frame";
u.root.at<Mk::wg2::Id("insFrames")>().valueFn = [&w] { return std::to_string(w.frameIndex()); };
u.root.at<Mk::wg2::Id("note")>().text = "Two top-level windows each own an ImGui context and frame loop; "
"binding the same external variables keeps them live-linked. "
"Clicking X only hides the Inspector; it follows the Gallery out when the Gallery closes.";
}

// ============ Inspector 子窗口（Qt 风格装配 struct）============

struct InspectorWindow {
	static constexpr auto id = Mk::wg2::Id("inspector");   // 树上的节点 id

	InsUi ui;                             // Inspector 界面树（buildIns 装配）
	std::unique_ptr<Mk::window> win;      // 子窗口（父窗口经 mountChild 亦持有共享引用）

	InspectorWindow() = default;          // 窗口在 setup() 里创建（主窗口要先存在）

	// 控件树初始化：从上下文拿主窗口，创建自己并挂载，再把窗口登记回上下文
	void setup(Mk::UiContext& ctx) {
		if (!ctx.window) return;
		init(*ctx.window);
		ctx.inspectorWindow = win.get();
	}

	// 创建并挂载到主窗口：中央挂载 + 帧回调 + 父关随关/点 X 隐藏
	void init(Mk::window& parent) {
		const float uiDpi = Mk::wg2::dpiScale();
		const auto [w, h] = Mk::workAreaSize(0.3f, 0.7f,
			static_cast<int>(420.0f * uiDpi), static_cast<int>(640.0f * uiDpi));
		win = std::make_unique<Mk::window>("Inspector", w, h);
		buildIns(ui, &parent, *win);   // 装配界面树（帧序号等由窗口自己提供）
		win->setAutoScaleFont(true);
		win->setCentralWidget(ui.root);   // 静态直挂：窗口每帧自动布局
		// 订阅引擎事件：主题 / 显隐都由本组件处理
		auto& engine = Mk::mkui<Mk::window>::RegistrationWindow();
		engine.on<Mk::ThemeChanged>([this](const Mk::ThemeChanged& e) { if (win) win->setTheme(e.theme); });
		engine.on<ToggleInspector>([this](const ToggleInspector&) {
			if (!win) return;
			if (win->visible()) win->hide(); else win->show();
		});
		parent.mountChild(*win);
	}

	// 控件协议：它是独立窗口节点，不参与主窗口的布局与绘制
	Mk::wg2::Size hint(const Mk::wg2::Size&) const { return Mk::wg2::Size{0.0f, 0.0f}; }
	Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }
	void draw(const Mk::wg2::Rect&) {}
};

} // namespace gallery
