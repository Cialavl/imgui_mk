#pragma once
// content_area 层 · 界面：内容区（页簿 + 30 个演示页数据）
//   页簿是框架组件（HBox<NavTree, PageHost>），页面数据归内容区自己，不挂在应用壳上。
#include "demo_page.h"
#include "../status_bar.h"   // 页脚（页面正下方，与页面同列）
#include <imgui_mk/ui/wg2/widgets/nav.h>   // 框架页簿（NavTree + PageHost）
#include <imgui_mk/ui_context.h>

#include <cstdlib>

namespace gallery {

struct ContentArea {
	static constexpr auto id = Mk::wg2::Id("content");

	// 页簿：导航树 +（页面 + 页脚）；宽度切分由框架 HBox 完成
	using Book = Mk::wg2::PageBook<Mk::wg2::Id("book"), StatusBar>;
	Book book;
	// 演示页数据：30 页与注册表（归内容区）
	DemoPages pages;

	// 控件树初始化：页导航快捷键归内容区自己（页簿在本层）
	void setup(Mk::UiContext& ctx) {
		book.footer().setup(ctx);   // 页脚在页簿内部，随内容区一起初始化
		if (!ctx.window) return;
		ctx.window->addShortcut(ImGuiKey_PageDown, [this] { stepPage(1); });
		ctx.window->addShortcut(ImGuiKey_PageUp, [this] { stepPage(-1); });
	}

	// 控件树初始化（ready 阶段）：页面数据注入 + 注册，并把页标题服务登记给上下文
	void ready(Mk::UiContext& ctx) {
		pages.inject(ctx.window, ctx.inspectorWindow);
		pages.registerAll(book);
		ctx.pageTitle = [this] { return book.currentTitle(); };

		// 启动页也归内容区（巡检/复现）
		if (const char* startPage = std::getenv("GALLERY_START_PAGE"))
			selectPage(std::atoi(startPage));
	}

	// —— 页导航：归内容区（页簿）——
	int pageCount() const { return book.pageCount(); }
	int currentPage() const { return book.current; }
	void selectPage(int page) { book.select(page); }
	void stepPage(int delta) {
		const int n = pageCount();
		if (n > 0) selectPage((currentPage() + delta % n + n) % n);
	}

	StatusBar& statusBar() { return book.footer(); }

	// 框架识别 root()：尺寸/策略/绘制都由框架转发给页簿（对外 book 接口不变）
	auto& root() { return book; }
	const auto& root() const { return book; }
};

} // namespace gallery
