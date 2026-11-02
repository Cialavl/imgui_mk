// mkui 控件画廊应用入口（wg-only）：双顶层窗口（Gallery + Inspector）+ 常驻层
// （菜单栏/浮动面板/三个弹窗）+ 文件对话框轮询。24 个 wg 演示页见 gallery_pages.h，
// 代码片段见 snippets.h。交互式应用：菜单 File/Exit 或直接关窗退出。
// 常驻层组装约定：命名控件 + 运行时 node.add；声明顺序 = 子先于父（析构安全）。
#include "gallery_app.h"
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/texture.h>




#include <windows.h>   // 崩溃自诊断写盘（调试用）
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace {

// —— 崩溃自诊断：未处理异常时把异常地址/调用栈/运行状态写盘（调试用；SEH 下 CRT IO 为 best-effort）——
static Mk::window* g_win = nullptr;   // 崩溃自诊断读窗口帧序号
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
	FILE* f = nullptr;
	if(fopen_s(&f, "C:\\Users\\26495\\AppData\\Local\\Temp\\opencode\\gallery_crash.txt", "w") == 0 && f) {
		std::fprintf(f, "exception=0x%08X addr=0x%p\n",
			ep->ExceptionRecord->ExceptionCode,
			ep->ExceptionRecord->ExceptionAddress);
		std::fprintf(f, "window frame=%llu\n", g_win ? g_win->frameIndex() : 0ull);
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
	// DPI + 全动态自适应（应用显式 opt-in）：
	//   字号 = 设计字号 × 窗口尺寸比例 × 显示器缩放（比例基准 = 本窗口构造尺寸，见 window::processAdaptiveScaling）
	Mk::wg2::setDefaultTheme(Mk::wg2::scaledForMonitor(Mk::wg2::Theme::dark()));
	// 默认尺寸上限同样按显示器缩放放大：GLFW 窗口尺寸是屏幕坐标，HiDPI 下不放大窗口会显得局促
	const float uiDpi = Mk::wg2::dpiScale();
	const auto [galW, galH] = Mk::workAreaSize(0.75f, 0.8f,
		static_cast<int>(1440.0f * uiDpi), static_cast<int>(880.0f * uiDpi));

	// 主窗口（组件化：数据归 GalleryWindow 所有，页面按引用取用）
	Mk::MainWindow<> gallery("mkui Widget Gallery", galW, galH);

	// 追加等宽代码字体（×0.85 随字号联动）：框架保证默认中文链 + 重烘焙后句柄写回 gMono
	gallery.setAutoScaleFont(true);

	// 应用组件：主壳 + 检查器子窗口（数据各页/各窗自己持有）
	gallery::GalleryWindow window(gallery);
	gallery.addAuxFont(0.85f, window.monoSlot());

	// 棋盘纹理（Image/ImageButton 页共用）：应用壳持有，页面按句柄取用
	static unsigned char pixels[64 * 64 * 4];
	makeChecker(pixels);
	window.tex = Mk::wg2::loadTextureRGBA(pixels, 64, 64);
	g_win = &gallery.getWindow();   // 崩溃诊断用
	if (const char* startPage = std::getenv("GALLERY_START_PAGE"))
		window.sel = std::atoi(startPage);   // 巡检/复现：直接打开指定页


	// —— Inspector 子窗口由主窗 GalleryWindow 构造时全权装配（窗口/界面/回调）——


	// 键盘翻页（PgUp/PgDn）：直接改应用组件的选中页
	const int pageCount = window.pageCount();
	gallery.addShortcut(ImGuiKey_PageDown, [&window, pageCount] {
		if (pageCount > 0) window.sel = (window.sel + 1) % pageCount;
	});
	gallery.addShortcut(ImGuiKey_PageUp, [&window, pageCount] {
		if (pageCount > 0) window.sel = (window.sel + pageCount - 1) % pageCount;
	});

	// Ctrl+O：发打开文件事件，由文件页自己处理（页面持有对话框）
	gallery.addShortcut(ImGuiKey_O, [] {
		Mk::mkui<Mk::window>::RegistrationWindow().emit(gallery::OpenFile{});
	}, true);

	SetUnhandledExceptionFilter(CrashHandler);

	// 视图直挂（Qt setCentralWidget 语义）：窗口每帧自动布局整棵树
	gallery.setCentralWidget(window.shell);


	Mk::mkui<Mk::window>::run();
	return 0;
}
