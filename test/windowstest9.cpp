#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

// 渲染探针：统计自己被渲染的帧数
struct RenderProbe { int& renders; };
inline void draw(const RenderProbe& p) { ++p.renders; }

// 纹理释放哨兵：放树尾，第 120 帧当帧释放纹理（此刻 GL 上下文仍存活）
struct TextureReleaser {
	int& frames;
	ImTextureID tex;
};
inline void draw(const TextureReleaser& r) {
	if(r.frames >= 120) {
		glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		Mk::fui::deleteTexture(r.tex);
	}
}

} // namespace

// 批次2 显示反馈控件验证：TextColored/Wrapped/BulletText/LabelText、ProgressBar、
// Separator(+Text)、Selectable、Image/ImageButton（程序生成棋盘纹理）、PlotLines、
// TreeNode 容器（展开/收起门控）、tooltip/colored/indent 修饰器。
// 逻辑侧每帧驱动进度/历史/选中态；120 帧后自动关窗，确定性断言。
int main() {
	Mk::MainWindow<> mainWindow("函数式显示控件验证", 1280, 800);

	// —— 外置状态 ——
	int frames = 0;
	float progress = 0.0f;
	std::vector<float> history;
	history.reserve(256);
	bool sel = false;
	int imgBtnClicks = 0;
	int probeAll = 0, probeOpen = 0, probeClosed = 0, probeIndent = 0;

	// —— 程序生成 64x64 棋盘格 RGBA 纹理（无外部文件依赖）——
	static unsigned char pixels[64 * 64 * 4];
	for(int y = 0; y < 64; ++y) {
		for(int x = 0; x < 64; ++x) {
			const bool odd = ((x / 8) + (y / 8)) % 2 == 1;
			unsigned char* px = &pixels[(y * 64 + x) * 4];
			px[0] = odd ? 220 : 60;
			px[1] = odd ? 160 : 120;
			px[2] = odd ? 60 : 220;
			px[3] = 255;
		}
	}
	const ImTextureID tex = Mk::fui::loadTextureRGBA(pixels, 64, 64);

	mainWindow.setUI([&] {
		// 逻辑侧驱动（先自增再画树：树尾哨兵读到当帧计数，120 帧整关闭）
		++frames;
		progress = frames / 120.0f;
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.1f));
		if(frames == 60) sel = true;

		Mk::fui::draw(Mk::fui::vbox(6.0f,
			Mk::fui::textColored("彩色文本（金色）", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)),
			Mk::fui::textWrapped("自动换行的长文本：函数式控件库中视图是每帧构造的临时值，状态全部外置于被绑定的变量，视图本身无生命周期负担。"),
			Mk::fui::bulletText("列表项：函数式组合风格"),
			Mk::fui::labelText("标签", "值"),
			Mk::fui::progressBar(progress, ImVec2(-FLT_MIN, 0), "加载中"),
			Mk::fui::separator(),
			Mk::fui::separatorText("分隔标题"),
			Mk::fui::selectable("可选中行（第 60 帧被逻辑侧选中）", sel),
			Mk::fui::hbox(8.0f,
				Mk::fui::image(tex, ImVec2(64, 64)),
				Mk::fui::imageButton("棋盘按钮", tex, ImVec2(64, 64),
					[&] { ++imgBtnClicks; })),
			Mk::fui::plotLines("折线历史", history, ImVec2(0, 80)),
			Mk::fui::tooltip(Mk::fui::button("悬停看提示", [] {}), "这是提示文本"),
			Mk::fui::colored(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.6f, 1.0f),
				Mk::fui::label("被 colored 修饰的文本")),
			Mk::fui::indent(30.0f, RenderProbe{probeIndent}),
			Mk::fui::treeNode("默认展开的树节点", ImGuiTreeNodeFlags_DefaultOpen,
				RenderProbe{probeOpen}),
			Mk::fui::treeNode("默认收起的树节点", 0, RenderProbe{probeClosed}),
			RenderProbe{probeAll},
			TextureReleaser{frames, tex}
		));
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const bool okFrames = (frames == 120);
	const bool okProbe = (probeAll == 120);
	const bool okProgress = (std::abs(progress - 1.0f) < 1e-6f);
	const bool okHistory = (history.size() == 120);
	const bool okSelect = (sel == true);
	const bool okTreeOpen = (probeOpen == 120);
	const bool okTreeClosed = (probeClosed == 0);
	const bool okIndent = (probeIndent == 120);
	const bool okBtn = (imgBtnClicks == 0);   // 无法程序化点击：只验证不误触发
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " probe=" << probeAll
		<< " progress=" << progress
		<< " history=" << history.size()
		<< " sel=" << sel
		<< " treeOpen=" << probeOpen
		<< " treeClosed=" << probeClosed
		<< " indent=" << probeIndent << std::endl;

	const bool pass = okFrames && okProbe && okProgress && okHistory && okSelect
		&& okTreeOpen && okTreeClosed && okIndent && okBtn && okExit;
	std::cout << (pass ? "PASS" : "FAIL") << std::endl;
	return pass ? 0 : 1;
}