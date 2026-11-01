#pragma once
// 中间内容区：拥有页簿与 29 个演示页，对外是一个 wg2 控件
#include "gallery_pages.h"
#include <imgui_mk/ui/wg2/widgets/nav.h>

namespace gallery {

struct ContentArea {
	static constexpr auto id = Mk::wg2::Id("content");

	Mk::wg2::WidgetState<> state;
	Mk::wg2::PageBook<Mk::wg2::Id("book")> book;

	gallery::Page<gallery::PageLayoutBasic>   pageLayoutBasic;
	gallery::Page<gallery::PageLayoutSize>    pageLayoutSize;
	gallery::Page<gallery::PageLayoutGrid>    pageLayoutGrid;
	gallery::Page<gallery::PageLayoutRuntime> pageLayoutRuntime;
	gallery::Page<gallery::PageLayoutDynamic> pageLayoutDynamic;
	gallery::Page<gallery::PageBase>          pageBase;
	gallery::Page<gallery::PageLifecycle>     pageLifecycle;
	gallery::Page<gallery::PageWg2>           pageWg2;
	gallery::Page<gallery::PageWg2Catalog>    pageWg2Catalog;
	gallery::Page<gallery::PageLayout>        pageLayout;
	gallery::Page<gallery::PageSplit>         pageSplit;
	gallery::Page<gallery::PageText>          pageText;
	gallery::Page<gallery::PageInput>         pageInput;
	gallery::Page<gallery::PageNumber>        pageNumber;
	gallery::Page<gallery::PageDrag>          pageDrag;
	gallery::Page<gallery::PageSelect>        pageSelect;
	gallery::Page<gallery::PageFeedback>      pageFeedback;
	gallery::Page<gallery::PageContainer>     pageContainer;
	gallery::Page<gallery::PageTab>           pageTab;
	gallery::Page<gallery::PageMenu>          pageMenu;
	gallery::Page<gallery::PageTable>         pageTable;
	gallery::Page<gallery::PagePopup>         pagePopup;
	gallery::Page<gallery::PageColor>         pageColor;
	gallery::Page<gallery::PageDeco>          pageDeco;
	gallery::Page<gallery::PageTheme>         pageTheme;
	gallery::Page<gallery::PageWindow>        pageWindow;
	gallery::Page<gallery::PageFile>          pageFile;
	gallery::Page<gallery::PageFnRender>      pageFnRender;
	gallery::Page<gallery::PageButton>        pageButton;

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
	void add(gallery::Page<Content>& p, const char* group = nullptr) {
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
