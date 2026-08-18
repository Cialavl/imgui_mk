// 教程 12：扩展你自己的控件。文档：lessons/lesson-12-custom-widgets.md
// ADL 三步法（数据型/交互型）+ 自定义修饰器 + OOP 复合控件 + 自定义字体链。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <string>
#include <utility>

namespace {

// §12.1 第一步/第二步：数据型自定义控件（ADL 三步法）
struct StatusBar { int& frames; };
void draw(const StatusBar& s) {
	ImGui::Text("帧 %d（自定义 StatusBar，数据型）", s.frames);
}

// §12.1 交互型自定义控件：渲染 + 点击检测（点击循环切换颜色）
struct ColorSwatch {
	std::array<float, 3>& rgb;
};
void draw(const ColorSwatch& c) {
	if(ImGui::ColorEdit3("色块（点击/输入）", c.rgb.data())) {
		// ColorEdit3 内部已写回 c.rgb——交互型控件"驱动状态"就靠返回值/写回
	}
}

// §12.2 自定义修饰器：加粗边框（Push/Pop 严格配对）
template<typename T>
struct Emphasis { T child; };
template<typename T>
void draw(const Emphasis<T>& e) {
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	draw(e.child);
	ImGui::PopStyleVar();
}
template<typename T>
auto emphasis(T child) { return Emphasis<std::decay_t<T>>{std::move(child)}; }

// §12.3 OOP 复合控件：长期存活 + 自持状态
class TogglePanel : public Mk::ui::Widget {
public:
	TogglePanel(int& clicks, float& v) : m_Clicks(clicks), m_Vol(v) {}
protected:
	void render() override {
		ImGui::CollapsingHeader("OOP 复合面板（点标题折叠）");
		if(ImGui::CollapsingHeader("内容区（可折叠）")) {
			ImGui::Text("点击次数 %d（自持子控件引用绑定）", m_Clicks);
			ImGui::SliderFloat("音量", &m_Vol, 0.0f, 1.0f);
			if(ImGui::Button("+1")) ++m_Clicks;
		}
	}
private:
	int& m_Clicks;
	float& m_Vol;
};

} // namespace

int main() {
	// §12.4 自定义字体链：中文 + 大号等宽（重烘焙时一起重挂）
	Mk::MainWindow<> mainWindow("教程12：扩展你自己的控件", 1280, 800);
	mainWindow.getWindow().setFontLoader([](float size) {
		if(!Mk::window::Impl::defaultFontLoader(size)) return false;  // 中文链
		ImFontConfig cfg;
		cfg.SizePixels = size * 0.9f;   // 等宽（这里用 0.9× 展示自定义）
		ImGui::GetIO().Fonts->AddFontDefault(&cfg);
		return true;
	});

	int frames = 0, clicks = 0;
	float volume = 0.5f;
	std::array<float, 3> rgb{0.9f, 0.3f, 0.2f};
	TogglePanel panel(clicks, volume);   // §12.3 长期存活

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		draw(vbox(8.0f,
			separatorText("§12.1 ADL 三步法自定义控件"),
			StatusBar{frames},                     // 数据型（直接进容器）
			ColorSwatch{rgb},                      // 交互型
			separatorText("§12.2 自定义修饰器：Emphasis（包任意控件）"),
			emphasis(button("加粗边框按钮", [&] { ++clicks; })),
			emphasis(slider("加粗滑块", volume, 0.0f, 1.0f)),
			labelText("点击", std::to_string(clicks)),
			separatorText("§12.3 OOP 复合控件（对象路线）"),
			dyn(panel),
			separatorText("§12.4 自定义字体链：按钮下重烘焙 18/27px 观察等宽字体"),
			hbox(8.0f,
				button("18px", [&] { mainWindow.getWindow().rescale(18.0f); }),
				button("27px", [&] { mainWindow.getWindow().rescale(27.0f); })),
			textWrapped("§12.5 自问：库内置组合能否替代自定义？能——用库的。")));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}