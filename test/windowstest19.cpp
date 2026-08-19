#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <cmath>
#include <iostream>
#include <array>

// test19：wg 主题与自适应验证——
//   帧 30 setTheme(light) → 帧 50 断言按钮色为浅色
//   帧 60 setTheme(dark.withAccent(红)) → 帧 80 断言 CheckMark = accent
//   帧 90 rescale(24) → 帧 110 断言字号 == 24
//   Styled 主题作用域：作用域内按钮色应为 light（FnRender 探针捕获）
// 120 帧自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> win("wg 主题与自适应", 900, 700);

	int frames = 0;
	int snapDarkBtn = 0, snapLightBtn = 0, snapAccent = 0, snapFont = 0;
	float btnDark = 0, btnLight = 0, accentR = 0, fontAt110 = 0, styledBtnR = 0;

	// Styled 作用域探针（可默认构造的结构体 + 指针成员）：作用域内捕获按钮色
	struct StyledProbe {
		float* out = nullptr;
		void operator()() { if(out) *out = ImGui::GetStyle().Colors[ImGuiCol_Button].x; }
	};
	Mk::wg::Styled<Mk::wg::FnRender<StyledProbe>> styledBox;
	styledBox.self.theme = Mk::wg::Theme::light();
	styledBox.child<0>().self.fn = StyledProbe{&styledBtnR};

	win.setUI([&] {
		++frames;

		if(frames == 20) {
			snapDarkBtn = 1;
			btnDark = ImGui::GetStyle().Colors[ImGuiCol_Button].x;
		}
		if(frames == 30) {
			win.setTheme(Mk::wg::Theme::light());
		}
		if(frames == 50) {
			snapLightBtn = 1;
			btnLight = ImGui::GetStyle().Colors[ImGuiCol_Button].x;
		}
		if(frames == 60) {
			win.setTheme(Mk::wg::Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
		}
		if(frames == 80) {
			snapAccent = 1;
			accentR = ImGui::GetStyle().Colors[ImGuiCol_CheckMark].x;
		}
		if(frames == 90) {
			win.getWindow().rescale(24.0f);   // 重烘焙：下一帧 render 顶部生效
		}
		if(frames == 110) {
			snapFont = 1;
			fontAt110 = ImGui::GetFontSize();
		}

		// Styled 作用域绘制（作用域内探针捕获按钮色）
		styledBox.layout(Mk::wg::Rect{20, 60, 300, 40});

		ImGui::Text("frames=%d font=%.0f", frames, ImGui::GetFontSize());

		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const float eps = 0.01f;
	const Mk::wg::Theme dark = Mk::wg::Theme::dark();
	const Mk::wg::Theme light = Mk::wg::Theme::light();
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	const bool okDarkBtn = snapDarkBtn == 1 && about(btnDark, dark.button.x);
	const bool okLightBtn = snapLightBtn == 1 && about(btnLight, light.button.x);
	const bool okAccent = snapAccent == 1 && about(accentR, 1.0f);
	const bool okFont = snapFont == 1 && about(fontAt110, 24.0f);
	const bool okStyled = about(styledBtnR, light.button.x);

	std::cout << "btnDark=" << btnDark << " btnLight=" << btnLight
		<< " accentR=" << accentR << " font@110=" << fontAt110
		<< " styledBtnR=" << styledBtnR << std::endl;

	const bool pass = okDarkBtn && okLightBtn && okAccent && okFont && okStyled;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}



