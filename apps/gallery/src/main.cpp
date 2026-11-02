// mkui 控件画廊应用入口：
//   GalleryWindow 就是主窗口：构造即建窗，并一行挂载组合界面；随后跑框架主循环。
#include "gallery_app.h"
#include <imgui_mk/ui/wg2/style.h>

#include <windows.h>   // 崩溃自诊断写盘（调试用）
#include <cstdio>

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

} // namespace

int main() {
	// 全局主题必须在窗口创建前设置（框架默认主题，进程级）
	Mk::wg2::setDefaultTheme(Mk::wg2::scaledForMonitor(Mk::wg2::Theme::dark()));

	// gallery 主窗口：构造即建窗（框架默认标题/尺寸/字体），并一行挂载组合界面
	gallery::GalleryWindow window;

	g_win = &window.getWindow();   // 崩溃诊断用
	SetUnhandledExceptionFilter(CrashHandler);

	window.run();   // 框架主循环
	return 0;
}
