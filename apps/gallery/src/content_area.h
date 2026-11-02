#pragma once
// 中间内容区：拥有页簿与 29 个演示页，对外是一个 wg2 控件
#include "demo_page.h"

#include "pages/core/base.h"
#include "pages/core/lifecycle.h"
#include "pages/core/layout.h"
#include "pages/core/split.h"
#include "pages/atoms/text.h"
#include "pages/atoms/button.h"
#include "pages/atoms/input.h"
#include "pages/atoms/number.h"
#include "pages/atoms/drag.h"
#include "pages/atoms/select.h"
#include "pages/atoms/feedback.h"
#include "pages/containers/container.h"
#include "pages/containers/tab.h"
#include "pages/containers/menu.h"
#include "pages/containers/table.h"
#include "pages/popup_color/popup.h"
#include "pages/popup_color/color.h"
#include "pages/deco/deco.h"
#include "pages/app/theme.h"
#include "pages/app/window.h"
#include "pages/app/file.h"
#include "pages/app/fnrender.h"
#include "pages/core/wg2.h"
#include "pages/core/wg2_catalog.h"
#include "pages/layout/layout_basic.h"
#include "pages/layout/layout_size.h"
#include "pages/layout/layout_grid.h"
#include "pages/layout/layout_runtime.h"
#include "pages/layout/layout_dynamic.h"
#include <imgui_mk/ui/wg2/widgets/nav.h>

namespace gallery {

// ============ 侧栏分组常量（addPage 组首传参用）============
inline constexpr const char* kGroupLayout     = "布局";
inline constexpr const char* kGroupWg          = "wg 体系";
inline constexpr const char* kGroupAtoms       = "原子控件";
inline constexpr const char* kGroupContainers  = "容器控件";
inline constexpr const char* kGroupPopColor    = "弹窗与颜色";
inline constexpr const char* kGroupDeco        = "装饰件";
inline constexpr const char* kGroupApp         = "应用";


struct ContentArea {
	static constexpr auto id = Mk::wg2::Id("content");

	Mk::wg2::WidgetState<> state;
	Mk::wg2::PageBook<Mk::wg2::Id("book")> book;
	std::function<ImFont*()> monoProvider;   // 等宽代码字体（应用注入）

	void setMonoProvider(std::function<ImFont*()> f) { monoProvider = std::move(f); }   // 在 init() 之前调用

	gallery::DemoPage<gallery::PageLayoutBasic>   pageLayoutBasic;
	gallery::DemoPage<gallery::PageLayoutSize>    pageLayoutSize;
	gallery::DemoPage<gallery::PageLayoutGrid>    pageLayoutGrid;
	gallery::DemoPage<gallery::PageLayoutRuntime> pageLayoutRuntime;
	gallery::DemoPage<gallery::PageLayoutDynamic> pageLayoutDynamic;
	gallery::DemoPage<gallery::PageBase>          pageBase;
	gallery::DemoPage<gallery::PageLifecycle>     pageLifecycle;
	gallery::DemoPage<gallery::PageWg2>           pageWg2;
	gallery::DemoPage<gallery::PageWg2Catalog>    pageWg2Catalog;
	gallery::DemoPage<gallery::PageLayout>        pageLayout;
	gallery::DemoPage<gallery::PageSplit>         pageSplit;
	gallery::DemoPage<gallery::PageText>          pageText;
	gallery::DemoPage<gallery::PageInput>         pageInput;
	gallery::DemoPage<gallery::PageNumber>        pageNumber;
	gallery::DemoPage<gallery::PageDrag>          pageDrag;
	gallery::DemoPage<gallery::PageSelect>        pageSelect;
	gallery::DemoPage<gallery::PageFeedback>      pageFeedback;
	gallery::DemoPage<gallery::PageContainer>     pageContainer;
	gallery::DemoPage<gallery::PageTab>           pageTab;
	gallery::DemoPage<gallery::PageMenu>          pageMenu;
	gallery::DemoPage<gallery::PageTable>         pageTable;
	gallery::DemoPage<gallery::PagePopup>         pagePopup;
	gallery::DemoPage<gallery::PageColor>         pageColor;
	gallery::DemoPage<gallery::PageDeco>          pageDeco;
	gallery::DemoPage<gallery::PageTheme>         pageTheme;
	gallery::DemoPage<gallery::PageWindow>        pageWindow;
	gallery::DemoPage<gallery::PageFile>          pageFile;
	gallery::DemoPage<gallery::PageFnRender>      pageFnRender;
	gallery::DemoPage<gallery::PageButton>        pageButton;

	// 依赖注入 + 注册（GalleryWindow 构造末尾调用一次）
	void init(ImTextureID tex, Mk::window* win, Mk::window* ins) {
		pageButton.content.root.at<Mk::wg2::Id("imgbtn")>().tex = tex;
		pageFeedback.content.win = win;
		pageMenu.content.win = win;
		pageWindow.content.win = win;
		pageWindow.content.inspectorWin = ins;
		pageFile.content.attach(win);
		load();
	}

	Mk::wg2::Size hint(const Mk::wg2::Size& a) const { return book.hint(a); }
	Mk::wg2::Policy policy() const { return book.policy(); }
	void draw(const Mk::wg2::Rect& r) { book.draw(r); }

private:
	template<class Content>
	void add(gallery::DemoPage<Content>& p, const char* group = nullptr) {
		p.setMonoProvider(monoProvider);   // 绘制时按需查询等宽字体
		p.setup();
		if (group && *group) book.addGroup(group);
		book.addPage(p.title, [&p](const Mk::wg2::Rect& r) { p.renderTo(r); });
	}

	void load() {
		add(pageLayoutBasic, "布局");
		add(pageLayoutSize);
		add(pageLayoutGrid);
		add(pageLayoutRuntime);
		add(pageLayoutDynamic);
		add(pageBase, "wg 体系");
		add(pageLifecycle);
		add(pageWg2);
		add(pageWg2Catalog);
		add(pageLayout);
		add(pageSplit);
		add(pageText, "原子控件");
		add(pageButton);
		add(pageInput);
		add(pageNumber);
		add(pageDrag);
		add(pageSelect);
		add(pageFeedback);
		add(pageContainer, "容器控件");
		add(pageTab);
		add(pageMenu);
		add(pageTable);
		add(pagePopup, "弹窗与颜色");
		add(pageColor);
		add(pageDeco, "装饰件");
		add(pageTheme, "应用");
		add(pageWindow);
		add(pageFile);
		add(pageFnRender);
	}
};

} // namespace gallery
