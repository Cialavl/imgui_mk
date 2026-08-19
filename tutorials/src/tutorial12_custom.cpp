// 教程 12：扩展你自己的控件。文档：lessons/lesson-12-custom-widgets.md
// 模板 struct 自定义控件（成员函数行为）/ 自定义装饰件（样式作用域）/
// 自定义复合控件 / 自定义字体链与重烘焙。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <array>
#include <string>

namespace {

// 自定义控件 1：状态栏（纯显示，模板 struct + 成员函数行为）
struct StatusBarImpl {
	int* frames = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(frames) ImGui::Text("帧 %d（自定义 wg 控件 StatusBar）", *frames);
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size& avail) {
		return Mk::wg::Size{avail.w, 20.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using StatusBar = Mk::wg::Control<StatusBarImpl, Mk::wg::DefaultStyle>;

// 自定义控件 2：色块编辑器（交互控件，直接绑定外部数组）
struct ColorSwatchImpl {
	std::array<float, 3>* rgb = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(rgb) ImGui::ColorEdit3("色块（点击/输入）", rgb->data());
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) {
		return Mk::wg::Size{260.0f, 24.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using ColorSwatch = Mk::wg::Control<ColorSwatchImpl, Mk::wg::DefaultStyle>;

// 自定义装饰件：包裹子树加粗边框（Push/Pop 严格配对）
struct EmphasisImpl {
	float border = 2.0f;
	void renderImpl(Mk::wg::Node& n) {
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, border);
		for(auto* c : n.children) if(c) c->draw();
		ImGui::PopStyleVar();
	}
	Mk::wg::Size measureImpl(Mk::wg::Node& n, const Mk::wg::Size& avail) {
		Mk::wg::Size m{0, 0};
		for(auto* c : n.children) if(c) m = c->measure(avail);
		return m;
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) {
		for(auto* c : n.children) if(c) c->arrange(r);
	}
};
using Emphasis = Mk::wg::Control<EmphasisImpl, Mk::wg::DefaultStyle>;

// 自定义复合控件：自带状态引用 + 自有渲染
struct TogglePanelImpl {
	int* clicks = nullptr;
	float* vol = nullptr;
	void renderImpl(Mk::wg::Node&) {
		if(ImGui::CollapsingHeader("自定义复合面板（点击展开）")) {
			ImGui::Text("内部计数 %d（自持引用绑定）", clicks ? *clicks : 0);
			if(vol) ImGui::SliderFloat("音量", vol, 0.0f, 1.0f);
			if(ImGui::Button("+1")) { if(clicks) ++(*clicks); }
		}
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size& avail) {
		return Mk::wg::Size{avail.w, 40.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using TogglePanel = Mk::wg::Control<TogglePanelImpl, Mk::wg::DefaultStyle>;

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程12：扩展你自己的控件", 1280, 800);

	// 自定义字体链：中文（默认链）+ 等宽代码字体（×0.9，随字号联动）
	mainWindow.getWindow().setFontLoader([](float size) {
		if(!Mk::window::Impl::defaultFontLoader(size)) return false;
		ImFontConfig cfg;
		cfg.SizePixels = size * 0.9f;
		ImGui::GetIO().Fonts->AddFontDefault(&cfg);
		return true;
	});

	int frames = 0, clicks = 0;
	float volume = 0.5f;
	std::array<float, 3> rgb{0.9f, 0.3f, 0.2f};

	// —— 树组装（子先声明）——
	StatusBar statusBar;
	statusBar.self.frames = &frames;
	ColorSwatch swatch;
	swatch.self.rgb = &rgb;

	Mk::wg::SeparatorText sec1, sec2, sec3, sec4;
	sec1.self.name = "自定义控件（模板 struct + 成员函数行为）";
	sec2.self.name = "自定义装饰件 Emphasis（加粗边框）";
	sec3.self.name = "自定义复合控件（自有渲染）";
	sec4.self.name = "自定义字体链 + 重烘焙（等宽字随字号联动）";

	Mk::wg::Button emphBtn;
	emphBtn.self.setName("加粗边框按钮");
	emphBtn.self.onClick = [&] { ++clicks; };
	Mk::wg::SliderF emphSlider;
	emphSlider.self.name = "加粗滑块";
	emphSlider.self.value = &volume;
	Emphasis emphasis;
	emphasis.self.border = 2.0f;
	emphasis.node.add(emphBtn.node);
	emphasis.node.add(emphSlider.node);

	TogglePanel panel;
	panel.self.clicks = &clicks;
	panel.self.vol = &volume;

	Mk::wg::Button b18, b27;
	b18.self.setName("18px");
	b18.self.onClick = [&] { mainWindow.getWindow().rescale(18.0f); };
	b27.self.setName("27px");
	b27.self.onClick = [&] { mainWindow.getWindow().rescale(27.0f); };
	Mk::wg::Layout btnRow;
	btnRow.self.mode = Mk::wg::LayoutMode::H;
	btnRow.node.add(b18.node);
	btnRow.node.add(b27.node);
	Mk::wg::TextWrapped note;
	note.self.name = "扩展方式：任何模板 struct（数据 + renderImpl/measureImpl/arrangeImpl 成员函数）"
		"经 Control 组合即成控件；装饰件用 Push/Pop 样式作用域包裹子树。";

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(statusBar.node);
	root.node.add(swatch.node);
	root.node.add(sec1.node);
	root.node.add(emphasis.node);
	root.node.add(sec3.node);
	root.node.add(panel.node);
	root.node.add(sec4.node);
	root.node.add(btnRow.node);
	root.node.add(note.node);

	mainWindow.setUI([&] {
		++frames;
		root.layout(Mk::wg::Rect{20, 20, 900, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
