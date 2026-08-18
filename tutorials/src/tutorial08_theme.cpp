// 教程 08：主题与全动态自适应。文档：lessons/lesson-08-theme-adaptive.md
// §8.1 深/浅主题 + accent 实时换肤；§8.2 styled 作用域对照；
// §8.3 字号滑块（rescale 联动几何）；§8.4 自适应开关 + 手动档位。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <string>

namespace {

// §8.1 accent 编辑器（gallery 同款）：编辑即整窗换肤
struct AccentEdit {
	Mk::fui::Theme& theme;
	std::array<float, 3>& col;
	Mk::MainWindow<>& win;
};
void draw(const AccentEdit& e) {
	if(ImGui::ColorEdit3("主色 accent", e.col.data())) {
		e.theme = e.theme.withAccent(ImVec4(e.col[0], e.col[1], e.col[2], 1.0f));
		e.win.setTheme(e.theme);
	}
}

} // namespace

int main() {
	// §8.4 启动 DPI（opt-in）——让字号先跟随显示器缩放
	Mk::fui::setDefaultTheme(Mk::fui::scaledForMonitor(Mk::fui::Theme::dark()));
	const auto [w, h] = Mk::workAreaSize(0.9f, 0.9f, 1280, 900);
	Mk::MainWindow<> mainWindow("教程08：主题与全动态自适应", w, h);

	// §8.4 运行时自适应：跨屏 + 宽度档位（拖窗口跨 900/1400/2000 观察）
	mainWindow.getWindow().setAutoScaleFont(true);

	// 状态
	Mk::fui::Theme theme = Mk::fui::Theme::dark();
	std::array<float, 3> accent{0.26f, 0.59f, 0.98f};
	float fontSize = 18.0f;
	bool autoOn = true, styledOn = true;
	int frames = 0;

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		draw(vbox(8.0f,
			separatorText("§8.1 窗口级主题：预设 + 链式派生 + 实时换肤"),
			hbox(8.0f,
				button("深色", [&] { theme = Theme::dark(); mainWindow.setTheme(theme); }),
				button("浅色", [&] { theme = Theme::light(); mainWindow.setTheme(theme); }),
				AccentEdit{theme, accent, mainWindow}),
			labelText("当前字号", std::to_string((int)ImGui::GetFontSize())),
			labelText("帧", std::to_string(frames)),

			separatorText("§8.2 styled 作用域：局部紧凑（域外不受影响）"),
			checkbox("显示紧凑区", styledOn),
			when(styledOn,
				styled(theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f),
					vbox(2.0f,
						button("紧凑按钮 1", [] {}),
						button("紧凑按钮 2", [] {})))),
			hbox(8.0f, button("正常按钮 1", [] {}), button("正常按钮 2", [] {})),

			separatorText("§8.3 字号等比几何：拖滑块 → setTheme+rescale 联动"),
			hbox(8.0f,
				slider("主题字号", fontSize, 14.0f, 26.0f),
				button("应用字号", [&] {
					theme = theme.withFontSize(fontSize);
					mainWindow.setTheme(theme);
					mainWindow.getWindow().rescale(fontSize);   // 重建字体位图 + 几何等比
				})),
			textWrapped("观察：滑块应用后，文字/间距/内边距整体等比变化（控件高与字号同涨）"),

			separatorText("§8.4 全动态自适应"),
			checkbox("开启自动档位（拖窗口跨 900/1400/2000 宽）", autoOn),
			hbox(8.0f,
				button("手动 14px", [&] { mainWindow.getWindow().rescale(14.0f); }),
				button("手动 18px", [&] { mainWindow.getWindow().rescale(18.0f); }),
				button("手动 27px", [&] { mainWindow.getWindow().rescale(27.0f); })),
			textWrapped("重烘焙全量 CJK 图集约 0.3~0.5s 卡顿属预期（量化档位把频率降到最低）")));


		// §8.4 每帧同步开关（setAutoScaleFont 幂等，每帧调用无害——教学直白版）
		mainWindow.getWindow().setAutoScaleFont(autoOn);
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}