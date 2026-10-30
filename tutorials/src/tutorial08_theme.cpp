// 教程 08：主题与全局动态自适应。文档：lessons/lesson-08-theme-adaptive.md
// 深/浅/Accent 实时切换 + styled 局部换肤 + 字号重烘焙（rescale）+ 全动态自适应。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <array>
#include <string>

namespace {

// accent 编辑器（wg 控件）：ColorEdit3 编辑即整窗换肤
struct AccentEditImpl {
	Mk::wg::Theme* theme = nullptr;
	std::array<float, 3>* col = nullptr;
	Mk::MainWindow<>* win = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(theme && col && win) {
			if(ImGui::ColorEdit3("主色 accent", col->data())) {
				*theme = theme->withAccent(ImVec4((*col)[0], (*col)[1], (*col)[2], 1.0f));
				win->setTheme(*theme);
			}
		}
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) {
		return Mk::wg::Size{260.0f, 24.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using AccentEdit = Mk::wg::Control<AccentEditImpl, Mk::wg::DefaultStyle>;

} // namespace

int main() {
	// 启动 DPI（opt-in）：字号与几何随显示器缩放
	Mk::wg::setDefaultTheme(Mk::wg::scaledForMonitor(Mk::wg::Theme::dark()));
	const auto [w, h] = Mk::workAreaSize(0.9f, 0.9f, 1280, 900);
	Mk::MainWindow<> mainWindow("教程08：主题与全局动态自适应", w, h);
	mainWindow.setAutoScaleFont(true);

	// 状态
	Mk::wg::Theme theme = Mk::wg::Theme::dark();
	std::array<float, 3> accent{0.26f, 0.59f, 0.98f};
	float fontSize = 18.0f;
	bool autoOn = true, styledOn = true;
	int frames = 0;

	// —— 树组装（子先声明）——
	Mk::wg::SeparatorText sec1, sec2, sec3, sec4;
	sec1.self.name = "窗口级主题：预设 + 链式派生 + 实时换肤";
	sec2.self.name = "styled 作用域（局部换肤，外部不受影响）";
	sec3.self.name = "字号等比联动：滑动后应用（setTheme + rescale）";
	sec4.self.name = "全动态自适应";

	Mk::wg::Button bDark, bLight;
	bDark.self.setName("深色");
	bDark.self.onClick = [&] {
		theme = Mk::wg::Theme::dark();
		mainWindow.setTheme(theme);
	};
	bLight.self.setName("浅色");
	bLight.self.onClick = [&] {
		theme = Mk::wg::Theme::light();
		mainWindow.setTheme(theme);
	};
	AccentEdit accentEdit;
	accentEdit.self = AccentEditImpl{&theme, &accent, &mainWindow};
	Mk::wg::Layout themeRow;
	themeRow.self.mode = Mk::wg::LayoutMode::H;
	themeRow.node.add(bDark.node);
	themeRow.node.add(bLight.node);
	themeRow.node.add(accentEdit.node);

	Mk::wg::LabelText ltFont, ltFrame;
	ltFont.self.name = "当前字号";
	ltFrame.self.name = "帧";
	Mk::wg::Layout infoRow;
	infoRow.self.mode = Mk::wg::LayoutMode::H;
	infoRow.node.add(ltFont.node);
	infoRow.node.add(ltFrame.node);

	Mk::wg::CheckBox cbStyled, cbAuto;
	cbStyled.self.name = "显示紧凑区";
	cbStyled.self.checked = &styledOn;
	cbAuto.self.name = "自动跨屏/窗口比例";
	cbAuto.self.checked = &autoOn;

	Mk::wg::Button compact1, compact2;
	compact1.self.setName("紧凑按钮 1");
	compact2.self.setName("紧凑按钮 2");
	Mk::wg::Layout compactBox;
	compactBox.self.mode = Mk::wg::LayoutMode::V;
	compactBox.self.spacing = 2.0f;
	compactBox.node.add(compact1.node);
	compactBox.node.add(compact2.node);
	Mk::wg::Styled<Mk::wg::Layout> styledBox;
	styledBox.self.theme = theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f);
	styledBox.node.add(compactBox.node);
	Mk::wg::When<Mk::wg::Styled<Mk::wg::Layout>> styledWhen;

	Mk::wg::Button normal1, normal2;
	normal1.self.setName("普通按钮 1");
	normal2.self.setName("普通按钮 2");
	Mk::wg::Layout normalRow;
	normalRow.self.mode = Mk::wg::LayoutMode::H;
	normalRow.node.add(normal1.node);
	normalRow.node.add(normal2.node);

	Mk::wg::SliderF fontSlider;
	fontSlider.self.name = "目标字号";
	fontSlider.self.value = &fontSize;
	fontSlider.self.mn = 14.0f;
	fontSlider.self.mx = 26.0f;
	Mk::wg::Button applyFont;
	applyFont.self.setName("应用字号");
	applyFont.self.onClick = [&] {
		theme = theme.withFontSize(fontSize);
		mainWindow.setTheme(theme);
		mainWindow.rescale(fontSize);   // 重烘焙字体位图 + 几何等比
	};
	Mk::wg::Layout fontRow;
	fontRow.self.mode = Mk::wg::LayoutMode::H;
	fontRow.node.add(fontSlider.node);
	fontRow.node.add(applyFont.node);
	Mk::wg::TextWrapped fontNote;
	fontNote.self.name = "观察：应用后文字/边距/控件高/间距全部按字号等比变化（控件几何与字号同源）；"
		"全量 CJK 图集重烘焙约 0.3~0.5s 一次性卡顿，属预期。";

	Mk::wg::Button man14, man18, man27;
	man14.self.setName("手动 14px");
	man14.self.onClick = [&] { mainWindow.rescale(14.0f); };
	man18.self.setName("手动 18px");
	man18.self.onClick = [&] { mainWindow.rescale(18.0f); };
	man27.self.setName("手动 27px");
	man27.self.onClick = [&] { mainWindow.rescale(27.0f); };
	Mk::wg::Layout manualRow;
	manualRow.self.mode = Mk::wg::LayoutMode::H;
	manualRow.node.add(man14.node);
	manualRow.node.add(man18.node);
	manualRow.node.add(man27.node);
	Mk::wg::TextWrapped autoNote;
	autoNote.self.name = "跨屏拖动/系统缩放变化会立即重烘焙；窗口尺寸变化按比例无级缩放（拖动中不重建、稳定 200ms 且变化 >5% 才重建）。每帧 setAutoScaleFont 同步开关（幂等 setter）。";

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(sec1.node);
	root.node.add(themeRow.node);
	root.node.add(infoRow.node);
	root.node.add(sec2.node);
	root.node.add(cbStyled.node);
	root.node.add(styledWhen.node);
	root.node.add(normalRow.node);
	root.node.add(sec3.node);
	root.node.add(fontRow.node);
	root.node.add(fontNote.node);
	root.node.add(sec4.node);
	root.node.add(cbAuto.node);
	root.node.add(manualRow.node);
	root.node.add(autoNote.node);

	mainWindow.setUI([&] {
		++frames;
		ltFont.self.value = std::to_string((int)ImGui::GetFontSize());
		ltFrame.self.value = std::to_string(frames);
		styledWhen.self.show = styledOn;
		mainWindow.setAutoScaleFont(autoOn);   // 每帧同步（幂等）
		root.layout(Mk::wg::Rect{20, 20, 1240, 860});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
