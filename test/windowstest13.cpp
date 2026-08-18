#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <iostream>
#include <string>

namespace {

// 宽度测量哨兵：在格子里测自己所在子窗口的宽度（hsplit 权重断言）
struct WidthProbe { float& out; };
inline void draw(const WidthProbe& p) { p.out = ImGui::GetWindowSize().x; }

// 项间距测量哨兵：读当前 style 的 ItemSpacing（styled 作用域断言）
struct SpacingProbe { float& outX; };
inline void draw(const SpacingProbe& p) { p.outX = ImGui::GetStyle().ItemSpacing.x; }

// 控件宽度测量哨兵：画一个占满 item 宽度的空文本，测 item 实宽（widthFrac 断言）
struct ItemWidthProbe { float& out; };
inline void draw(const ItemWidthProbe& p) {
	ImGui::TextUnformatted("");
	p.out = ImGui::GetItemRectSize().x;
}

// 窗口底色测量哨兵：读当前 style 的 WindowBg（setTheme 断言）
struct BgProbe { ImVec4& out; };
inline void draw(const BgProbe& p) { p.out = ImGui::GetStyle().Colors[ImGuiCol_WindowBg]; }

// 菜单栏偏移探针：记录画完菜单栏后的宿主光标 Y 与菜单栏高度
//（MenuBar 应把宿主光标推到菜单栏底边之下，否则内容与菜单栏重叠）
struct MenuOffsetProbe { float& cursorY; float& menuH; };
inline void draw(const MenuOffsetProbe& p) {
	p.cursorY = ImGui::GetCursorScreenPos().y;
	p.menuH = ImGui::GetFrameHeight();
}

// 样式实测探针：捕获宿主 WindowPadding.x 与 ItemSpacing.x（公式推导断言用，
// 期望值与实现同源，主题比例再调也无需改测试）
struct StyleProbe { float& padX; float& spX; };
inline void draw(const StyleProbe& p) {
	p.padX = ImGui::GetStyle().WindowPadding.x;
	p.spX = ImGui::GetStyle().ItemSpacing.x;
}

// 输入框宽度探针：InputText 后读 item 实宽（widthFrac 1.0 断言）
struct InputWidthProbe { float& out; };
inline void draw(const InputWidthProbe& p) {
	char buf[8] = {};
	ImGui::InputText("##iw", buf, sizeof(buf));
	p.out = ImGui::GetItemRectSize().x;
}

} // namespace

// 主题与自适应布局验证：
//   hsplit 权重分配（fixed/stretch 实测宽度 == 计算值）
//   widthFrac(1.0) 输入框占满可用宽
//   styled(withItemSpacing) 作用域内间距生效、域外回落
//   setTheme(light) 窗口底色 == 主题色
// 120 帧后自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> mainWindow("主题与自适应验证", 1000, 600);

	int frames = 0;
	float wFixedL = 0, wStretch = 0, wFixedR = 0;
	float spInStyled = 0, spOutside = 0;
	float inputW = 0;
	ImVec4 bg{0, 0, 0, 0};
	float menuCursorY = 0, menuH = 0;
	float hostPadX = 0, styleSpX = 0;

	// 换浅色主题：验证窗口级应用
	mainWindow.setTheme(Mk::fui::Theme::light());

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		// 菜单栏（浮在视口顶部的独立窗口）：画完后宿主光标应被推到菜单栏底边之下
		draw(menuBar(menu("文件", menuItem("条目", [] {}))));
		draw(MenuOffsetProbe{menuCursorY, menuH});
		draw(StyleProbe{hostPadX, styleSpX});

		// hsplit：fixed(240) + stretch(1) + fixed(200)，间距 8
		draw(hsplit(
			fixed("L", 240.0f, WidthProbe{wFixedL}),
			stretch("M", 1.0f, vbox(6.0f,
				// widthFrac(1.0)：输入框占满本子窗口可用宽
				WidthProbe{wStretch},
			widthFrac(1.0f, InputWidthProbe{inputW}),
				// styled 作用域：itemSpacing 20；域外回落
				styled(Theme::dark().withItemSpacing(20.0f, 20.0f),
					SpacingProbe{spInStyled}),
				SpacingProbe{spOutside},
				BgProbe{bg})),
			fixed("R", 200.0f, WidthProbe{wFixedR})));

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 断言（±1px 容差，浮点分配）——
	const float eps = 1.5f;
	const auto about = [eps](float a, float b) { return std::abs(a - b) <= eps; };

	const bool okFrames = (frames == 120);
	const bool okFixedL = about(wFixedL, 240.0f);
	const bool okFixedR = about(wFixedR, 200.0f);
	// 中格 = 宿主可用宽(1000−2×pad) − 240 − 200 − 2×间距：用当帧实测样式值推导（±4 容差）
	const float expectStretch = 1000.0f - 2.0f * hostPadX - 240.0f - 200.0f - 2.0f * styleSpX;
	const bool okStretchW = std::abs(wStretch - expectStretch) <= 4.0f;
	const bool okStyledSp = about(spInStyled, 20.0f);
	// 主题默认间距 = 0.58×fontSize（applyTheme 比例推导的联动验证）
	const bool okOutsideSp = about(spOutside, 0.58f * Mk::fui::Theme::dark().fontSize);
	const bool okBg = about(bg.x, 0.95f) && about(bg.y, 0.95f) && about(bg.z, 0.95f);
	const bool okInput = inputW >= 400.0f;   // 占满 ≈ 中格可用宽
	// 菜单栏底边 = 视口顶(0) + menuH；宿主光标必须在底边之下（0.5px 容差）
	const bool okMenu = (menuCursorY >= menuH - 0.5f);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " fixedL=" << wFixedL
		<< " stretch=" << wStretch
		<< " fixedR=" << wFixedR
		<< " spIn=" << spInStyled
		<< " spOut=" << spOutside
		<< " inputW=" << inputW
		<< " bg=(" << bg.x << "," << bg.y << "," << bg.z << ")"
		<< " menuY=" << menuCursorY << " menuH=" << menuH << std::endl;

	const bool pass = okFrames && okFixedL && okFixedR && okStretchW && okStyledSp
		&& okOutsideSp && okBg && okInput && okMenu && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", 左定宽 " << okFixedL
		<< ", 中弹性 " << okStretchW
		<< ", 右定宽 " << okFixedR
		<< ", 作用域间距 " << okStyledSp
		<< ", 域外回落 " << okOutsideSp
		<< ", 浅色主题 " << okBg
		<< ", 输入占满 " << okInput
		<< ", 菜单不重叠 " << okMenu
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}