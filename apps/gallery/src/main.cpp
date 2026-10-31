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
#include <filesystem>
#include <fstream>
#include <string>

namespace {

// —— 崩溃自诊断：未处理异常时把异常地址/调用栈/运行状态写盘（调试用；SEH 下 CRT IO 为 best-effort）——
static gallery::State* g_state = nullptr;   // 崩溃自诊断直接读状态，不做每帧快照
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
	FILE* f = nullptr;
	if(fopen_s(&f, "C:\\Users\\26495\\AppData\\Local\\Temp\\opencode\\gallery_crash.txt", "w") == 0 && f) {
		std::fprintf(f, "exception=0x%08X addr=0x%p\n",
			ep->ExceptionRecord->ExceptionCode,
			ep->ExceptionRecord->ExceptionAddress);
		std::fprintf(f, "state: frame=%llu sel=%d\n", g_state ? g_state->win->frameIndex() : 0ull, g_state ? g_state->sel : -1);
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

	// 主窗口创建即注册共享状态（引擎层数据联动：窗口/页面经引擎取用同一实例）
	Mk::MainWindow<Mk::window, gallery::State> gallery("mkui Widget Gallery", galW, galH);
	gallery::State& s = gallery.state<gallery::State>();

	// 追加等宽代码字体（×0.85 随字号联动）：框架保证默认中文链 + 重烘焙后句柄写回 gMono
	gallery.addAuxFont(0.85f, &gallery::gMono);
	gallery.setAutoScaleFont(true);

	// 棋盘纹理（Image/ImageButton 页共用）
	static unsigned char pixels[64 * 64 * 4];
	makeChecker(pixels);
	s.tex = Mk::wg2::loadTextureRGBA(pixels, 64, 64);

	// —— 29 个演示页：main 组装一次，UI 回调按选中下标切换可见性 ——
	gallery::GalleryWindow window(gallery, s);

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
	gallery::PageWg2       pageWg2(s);
	gallery::PageWg2Catalog pageWg2Catalog(s);
	gallery::PageLayoutBasic   pageLayoutBasic(s);
	gallery::PageLayoutSize    pageLayoutSize(s);
	gallery::PageLayoutGrid    pageLayoutGrid(s);
	gallery::PageLayoutRuntime pageLayoutRuntime(s);
	gallery::PageLayoutDynamic pageLayoutDynamic(s);

	// —— 布局大类：简单/复杂 × 静态/动态 ——
	window.addPage(pageLayoutBasic,   gallery::kGroupLayout);
	window.addPage(pageLayoutSize);
	window.addPage(pageLayoutGrid);
	window.addPage(pageLayoutRuntime);
	window.addPage(pageLayoutDynamic);

	window.addPage(pageBase,      gallery::kGroupWg);
	window.addPage(pageLifecycle);
	window.addPage(pageWg2);
	window.addPage(pageWg2Catalog);
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

	// —— Inspector 子窗口由主窗 GalleryWindow 构造时全权装配（窗口/界面/回调）——


	// 键盘翻页（PgUp/PgDn，与侧栏选择共用一个下标）：方便逐页巡查 UI
	const int pageCount = window.pageCount();
	gallery.addShortcut(ImGuiKey_PageDown, [&s, pageCount] {
		if(pageCount > 0) s.sel = (s.sel + 1) % pageCount;
	});
	gallery.addShortcut(ImGuiKey_PageUp, [&s, pageCount] {
		if(pageCount > 0) s.sel = (s.sel + pageCount - 1) % pageCount;
	});

	// 注册式每帧机制：快捷键 + 文件对话框托管（引擎在 user UI 前统一分发）
	gallery.addShortcut(ImGuiKey_O, [&] { s.openFileDlg(); }, true);
	gallery.watchFileDialog(s.fileDlg, [&](std::string picked) {
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
	});

	SetUnhandledExceptionFilter(CrashHandler);

	// 视图直挂（Qt setCentralWidget 语义）：窗口每帧自动布局整棵树
	gallery.setCentralWidget(window.shell);

	// 状态交给控件自己拉取：帧序号来自引擎，界面值由各控件 valueFn 取
	s.win = &gallery.getWindow();

	Mk::mkui<Mk::window>::run();
	return 0;
}
