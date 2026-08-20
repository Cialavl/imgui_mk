// mkui 控件画廊应用入口（wg-only）：双顶层窗口（Gallery + Inspector）+ 常驻层
// （菜单栏/浮动面板/三个弹窗）+ 文件对话框轮询。22 个 wg 演示页见 gallery_pages.h，
// 代码片段见 snippets.h。交互式应用：菜单 File/Exit 或直接关窗退出。
// 常驻层组装约定：命名控件 + 运行时 node.add；声明顺序 = 子先于父（析构安全）。
#include "gallery_app.h"
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>

#include <windows.h>   // 崩溃自诊断写盘（调试用）
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

// —— 崩溃自诊断：未处理异常时把异常地址/调用栈/运行状态写盘（调试用；SEH 下 CRT IO 为 best-effort）——
static volatile LONG g_crashFrame = 0;
static volatile LONG g_crashSel = -1;
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
	FILE* f = nullptr;
	if(fopen_s(&f, "C:\\Users\\26495\\AppData\\Local\\Temp\\opencode\\gallery_crash.txt", "w") == 0 && f) {
		std::fprintf(f, "exception=0x%08X addr=0x%p\n",
			ep->ExceptionRecord->ExceptionCode,
			ep->ExceptionRecord->ExceptionAddress);
		std::fprintf(f, "state: frame=%ld sel=%ld\n", g_crashFrame, g_crashSel);
		void* bt[40];
		const USHORT n = RtlCaptureStackBackTrace(0, 40, bt, nullptr);
		for(USHORT i = 0; i < n; ++i) {
			std::fprintf(f, "  [%u] 0x%p\n", i, bt[i]);
		}
		std::fclose(f);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

// 程序生成 64x64 棋盘格 RGBA 纹理（无外部文件依赖）
void makeChecker(unsigned char* px) {
	for(int y = 0; y < 64; ++y) {
		for(int x = 0; x < 64; ++x) {
			const bool odd = ((x / 8) + (y / 8)) % 2 == 1;
			unsigned char* p = &px[(y * 64 + x) * 4];
			p[0] = odd ? 220 : 60;
			p[1] = odd ? 160 : 120;
			p[2] = odd ? 60 : 220;
			p[3] = 255;
		}
	}
}

} // namespace

int main() {
	// DPI + 全动态自适应（应用显式 opt-in）：字号 = 基准 18 × 显示器缩放 × 宽度档位
	Mk::wg::setDefaultTheme(Mk::wg::scaledForMonitor(Mk::wg::Theme::dark()));
	const auto [galW, galH] = Mk::workAreaSize(0.75f, 0.8f, 1440, 880);

	Mk::MainWindow<> gallery("mkui Widget Gallery", galW, galH);

	// 自定义字体链：中文（默认链）+ 等宽代码字体（×0.85 随字号联动）
	gallery.getWindow().setFontLoader([](float fontSize) {
		if(!Mk::window::Impl::defaultFontLoader(fontSize)) return false;
		ImFontConfig cfg;
		cfg.SizePixels = fontSize * 0.85f;
		gallery::gMono = ImGui::GetIO().Fonts->AddFontDefault(&cfg);
		return true;
	});
	gallery.getWindow().setAutoScaleFont(true);

	gallery::State s;
	s.win = &gallery;

	// 棋盘纹理（Image/ImageButton 页共用）
	static unsigned char pixels[64 * 64 * 4];
	makeChecker(pixels);
	s.tex = Mk::wg::loadTextureRGBA(pixels, 64, 64);

	// —— 22 个演示页：main 组装一次，UI 回调按选中下标切换可见性 ——
	gallery::GalleryWindow window(s);

	gallery::PageBase      pageBase(s);
	gallery::PageLifecycle pageLifecycle(s);
	gallery::PageLayout    pageLayout(s);
	gallery::PageSplit     pageSplit(s);
	gallery::PageText      pageText(s);
	gallery::PageButton    pageButton(s);
	gallery::PageInput     pageInput(s);
	gallery::PageNumber    pageNumber(s);
	gallery::PageDrag      pageDrag(s);
	gallery::PageSelect    pageSelect(s);
	gallery::PageFeedback  pageFeedback(s);
	gallery::PageContainer pageContainer(s);
	gallery::PageTab       pageTab(s);
	gallery::PageMenu      pageMenu(s);
	gallery::PageTable     pageTable(s);
	gallery::PagePopup     pagePopup(s);
	gallery::PageColor     pageColor(s);
	gallery::PageDeco      pageDeco(s);
	gallery::PageTheme     pageTheme(s);
	gallery::PageWindow    pageWindow(s);
	gallery::PageFile      pageFile(s);
	gallery::PageFnRender  pageFnRender(s);

	window.addPage(pageBase,      gallery::kGroupWg);
	window.addPage(pageLifecycle);
	window.addPage(pageLayout);
	window.addPage(pageSplit);
	window.addPage(pageText,      gallery::kGroupAtoms);
	window.addPage(pageButton);
	window.addPage(pageInput);
	window.addPage(pageNumber);
	window.addPage(pageDrag);
	window.addPage(pageSelect);
	window.addPage(pageFeedback);
	window.addPage(pageContainer, gallery::kGroupContainers);
	window.addPage(pageTab);
	window.addPage(pageMenu);
	window.addPage(pageTable);
	window.addPage(pagePopup,     gallery::kGroupPopColor);
	window.addPage(pageColor);
	window.addPage(pageDeco,      gallery::kGroupDeco);
	window.addPage(pageTheme,     gallery::kGroupApp);
	window.addPage(pageWindow);
	window.addPage(pageFile);
	window.addPage(pageFnRender);

	// —— 第二顶层窗口：Inspector（与画廊共享全部外置状态；点 X 仅隐藏）——
	const auto [insW, insH] = Mk::workAreaSize(0.3f, 0.7f, 420, 640);
	Mk::window inspector("Inspector", insW, insH);
	inspector.setAutoScaleFont(true);
	s.inspectorWin = &inspector;
	s.inspectorHandle = inspector.getHandle();
	if(s.inspectorHandle) {
		glfwSetWindowCloseCallback(s.inspectorHandle, [](GLFWwindow* w) {
			glfwSetWindowShouldClose(w, GLFW_FALSE);
			glfwHideWindow(w);
		});
	}
	gallery::InsUi insUi;
	gallery::buildIns(insUi, s);
	inspector.setCentralWidget(insUi.root.node);   // 中央挂载：外边距/尺寸由引擎每帧驱动
	inspector.setUserUI([&] {
		if(gallery.getHandle() == nullptr) {   // 主窗已销毁 → 跟随退出
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		if(s.inspectorThemeDirty) {   // 延迟换肤：本帧上下文即 Inspector 自身，无跨窗口切换
			inspector.setTheme(s.pendingTheme);
			s.inspectorThemeDirty = false;
		}
		++s.insFrames;
		gallery::refreshIns(insUi, s);
	});

	// 中央挂载一次性完成（引擎每帧自动根布局；Qt setCentralWidget 语义）
	gallery.setCentralWidget(window.root.node);

	SetUnhandledExceptionFilter(CrashHandler);

	gallery.setUI([&] {
		g_crashFrame = s.frames;
		g_crashSel = s.sel;
		// 动画驱动
		++s.frames;
		s.volume = 0.5f + 0.5f * std::sin(s.frames * 0.02f);
		s.history.push_back(0.5f + 0.5f * std::sin(s.frames * 0.05f));
		if(s.history.size() > 300) s.history.erase(s.history.begin());

		// Ctrl+O 快捷键（与菜单 Open 同路径；wg 小助手免直连 ImGui IO）
		if(Mk::wg::detail::ctrlHeld() && Mk::wg::detail::keyPressed(ImGuiKey_O)) s.openFileDlg();

		// Inspector 句柄刷新（窗口关闭后失效防护）
		s.inspectorHandle = inspector.getHandle();

		// 文件对话框轮询：完成即读取（文本截前 64KB，二进制只记大小）
		std::string picked;
		if(s.fileDlg.poll(picked)) {
			s.filePath = picked;
			s.filePreview.clear();
			s.fileSize = 0;
			s.fileBinary = false;
			s.fileMissing = false;
			if(!s.filePath.empty()) {
				std::error_code ec;
				s.fileSize = std::filesystem::file_size(s.filePath, ec);
				if(ec) {
					s.fileMissing = true;
				}
				else {
					std::ifstream in(s.filePath, std::ios::binary);
					constexpr std::size_t kCap = 64 * 1024;
					std::string buf(kCap, '\0');
					in.read(buf.data(), (std::streamsize)kCap);
					buf.resize((size_t)in.gcount());
					s.fileBinary = buf.find('\0') != std::string::npos;
					if(!s.fileBinary) s.filePreview = std::move(buf);
				}
			}
		}

		// 每帧动态绑定刷新 + 状态栏
		window.refreshAll(s);
		window.drawPopups();   // 弹窗浮层（触发后自己显示，不占布局）
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}