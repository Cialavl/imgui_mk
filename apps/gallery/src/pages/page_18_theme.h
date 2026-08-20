#pragma once
// p18 主题自适应：Theme dark/light/accent + Styled 局部主题 + rescale
#include "../gallery_chrome.h"

namespace gallery {

struct PageTheme {
	Button themeDark, themeLight, themeAccent;
	HLayout themeRow;
	Label themeScopeNote;
	Button themeCompact1, themeCompact2;
	VLayout themeCompactInner;
	Styled<VLayout> themeCompact;
	Label themeRescaleNote;
	Button themeRescale;

	Page page;
	PageTheme(State& s) {
		page.setup("主题与自适应",
			"Theme 值对象：dark/light 预设 + withAccent 链式修改；Styled 局部主题区换肤；"
			"rescale 重载字号（窗口等比缩放自适应）。",
			snips::kTheme);
		themeDark.self.setName("Dark");
		themeDark.self.onClick = [&] { s.applyTheme(Theme::dark()); };   // 引擎广播：双窗同步换肤
		themeLight.self.setName("Light");
		themeLight.self.onClick = [&] { s.applyTheme(Theme::light()); };
		themeAccent.self.setName("Accent 红");
		themeAccent.self.onClick = [&] {
			s.applyTheme(Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
		};

		themeRow.addWidget(themeDark.node);
		themeRow.addWidget(themeLight.node);
		themeRow.addWidget(themeAccent.node);
		themeScopeNote.self.setName("Styled 作用域：局部覆盖主题（全局不受影响）");
		themeCompact1.self.setName("紧凑按钮一");
		themeCompact2.self.setName("紧凑按钮二");

		themeCompactInner.addWidget(themeCompact1.node);
		themeCompactInner.addWidget(themeCompact2.node);
		themeCompact.self.theme = s.theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f);
		themeCompact.node.add(themeCompactInner.node);
		themeRescaleNote.self.setName("rescale(24)：重载字号图标集；布局/控件/行高/间距全等比");
		themeRescale.self.setName("rescale(24)");
		themeRescale.self.onClick = [&] {
			const float cur = engineFontSize();
			engineRescale(cur >= 23.5f ? 18.0f : 24.0f);   // 引擎广播：18↔24 双向切换
		};
		page.addWidget(themeRow);
		page.addWidget(themeScopeNote);
		page.addWidget(themeCompact);
		page.addWidget(themeRescaleNote);
		page.addWidget(themeRescale);
	}

	void refresh(State& s) {
		themeCompact.self.theme = s.theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f);   // 跟随当前主题（换肤后同步）
		const float cur = engineFontSize();
		themeRescale.self.setName(cur >= 23.5f ? "rescale(18)" : "rescale(24)");
	}
};

} // namespace gallery