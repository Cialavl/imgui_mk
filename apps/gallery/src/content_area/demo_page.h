#pragma once
// content_area 层 · 演示界面：页面骨架 + 30 个演示页与注册表
//   骨架：SectionHead + PageDemo + CodeBlock 编译期组成 DemoPage<Content>（页头 / 演示场 / 代码段）。
//   注册表：DemoPages 持有 30 页，做依赖注入，并按分组注册进侧栏与页簿。
//   具体页面在 pages/ 下，由本文件统一包含；页面头不要单独 include（它会回指本文件）。
#include "../data/snippets.h"
#include <imgui_mk/ui/wg2/font.h>   // 引擎等宽字体（monoFont）

#include <functional>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>

#include <algorithm>
#include <string_view>
#include <utility>

namespace gallery {

using namespace Mk::wg2;   // 页面代码沿用 wg2 名字（pages/ 里的内容 struct 依赖）

} // namespace gallery

// —— 演示页：只依赖上面的 wg2 名字与 snippets，先于骨架定义包含 ——
#include "pages/wg/base.h"
#include "pages/wg/lifecycle.h"
#include "pages/layout/layout.h"
#include "pages/layout/split.h"
#include "pages/layout/dock.h"
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
#include "pages/wg/wg2.h"
#include "pages/wg/wg2_catalog.h"
#include "pages/layout/layout_basic.h"
#include "pages/layout/layout_size.h"
#include "pages/layout/layout_grid.h"
#include "pages/layout/layout_runtime.h"
#include "pages/layout/layout_dynamic.h"

namespace gallery {

// ============ 节标题 ============

template<Mk::wg2::FixedString Id>
struct SectionHead {
	static constexpr auto id = Id;

	const char* title = "";
	const char* desc = "";

	// 组合式：标题 + 灰色换行描述，高度由子控件汇总
	using Root = Mk::wg2::VBox<Mk::wg2::Id("headCol"),
		Mk::wg2::Label<Mk::wg2::Id("headTitle")>,
		Mk::wg2::Colored<Mk::wg2::Id("headDescColor"),
			Mk::wg2::TextWrapped<Mk::wg2::Id("headDesc")>>,
		Mk::wg2::Rule<Mk::wg2::Id("headRule")>>;
	Root root;

	// hint/policy 由框架转发给 root；draw 保留（先填标题/描述再布局）

	void draw(const Mk::wg2::Rect& r) {
		root.spacing = 4.0f;   // 收紧档：与全局 4/8/16 刻度一致
		root.at<Mk::wg2::Id("headTitle")>().text = title;
		root.at<Mk::wg2::Id("headDescColor"), Mk::wg2::Id("headDesc")>().text = desc;
		root.at<Mk::wg2::Id("headDescColor")>().color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		Mk::wg2::layout(root, Mk::wg2::Rect{r.x, r.y, r.w, r.h});
	}
};

// ============ 代码块（简化：等宽 + 行号 + Copy） ============

template<Mk::wg2::FixedString Id>
struct CodeBlock {
static constexpr auto id = Id;

std::string_view code;
ImU32 copiedFrame = 0;

// 代码区内边距（em，随字号）：顶部留白比全局控件间距更松，代码不贴边框
static ImVec2 innerPad() {
const float em = Mk::wg2::metrics::fontSize();
return ImVec2(0.55f * em, 0.75f * em);
}

	// 组合式：复制行 + 代码区
	using Root = Mk::wg2::VBox<Mk::wg2::Id("codeCol"),
		Mk::wg2::HBox<Mk::wg2::Id("copyRow"),
			Mk::wg2::Spacer<Mk::wg2::Id("grow")>,
			Mk::wg2::SmallButton<Mk::wg2::Id("copyBtn")>>,
		Mk::wg2::ChildRegion<Mk::wg2::Id("codeRegion")>>;
	Root root;

	Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
		// 高度 = 复制行 + 代码区（行数 × 行高 + 内边距）
		const std::size_t linesN = 1 + static_cast<std::size_t>(std::count(code.begin(), code.end(), '\n'));
		return Mk::wg2::Size{avail.w,
			Mk::wg2::metrics::frameHeight() + Mk::wg2::metrics::itemSpacing().y
			+ static_cast<float>(linesN) * Mk::wg2::metrics::lineHeight() + innerPad().y * 2.0f};
	}

	void draw(const Mk::wg2::Rect& r) {
		auto& btn = root.at<Mk::wg2::Id("copyRow"), Mk::wg2::Id("copyBtn")>();
		btn.label = (Mk::wg2::metrics::frameCount() - copiedFrame < 60) ? "已复制" : "Copy";
		btn.onClick = [this] {
			Mk::wg2::setClipboard(std::string(code));
			copiedFrame = Mk::wg2::metrics::frameCount();
		};

		auto& region = root.at<Mk::wg2::Id("codeRegion")>();
		region.idname = "code_wg2";
		region.padding = innerPad();
		region.border = true;
		region.flags = ImGuiWindowFlags_HorizontalScrollbar;
		region.height = hint(Mk::wg2::Size{r.w, 0.0f}).h
			- Mk::wg2::metrics::frameHeight() - Mk::wg2::metrics::itemSpacing().y;
		region.body = [this](const ImVec2&) {
			const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_TextDisabled);
			int lineNo = 1;
			std::size_t pos = 0;
			while (pos <= code.size()) {
				std::size_t end = code.find('\n', pos);
				if (end == std::string_view::npos) end = code.size();
				ImGui::PushStyleColor(ImGuiCol_Text, lineCol);
				ImGui::Text("%4d  ", lineNo);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				ImGui::TextUnformatted(code.data() + pos, code.data() + end);
				++lineNo;
				if (end == code.size()) break;
				pos = end + 1;
			}
		};

		Mk::wg2::FontScope font(Mk::wg2::monoFont());   // 引擎字体：有等宽字体用它，否则回退默认字体
		Mk::wg2::layout(root, r);
	}
};

// ============ 页面演示槽（可滚动） ============

template<Mk::wg2::FixedString Id>
struct PageDemo {
static constexpr auto id = Id;

std::function<void(const Mk::wg2::Rect&)> render;
std::function<Mk::wg2::Size(const Mk::wg2::Size&)> measure;

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
		if (measure) {
			// 只占内容需要的高度：演示区不把剩余空间全吃掉
			const float h = measure(Mk::wg2::Size{avail.w > 0.0f ? avail.w : 0.0f, 0.0f}).h;
			return Mk::wg2::Size{avail.w, h + 6.0f};
		}
		return Mk::wg2::Size{avail.w, avail.h};
	}

void draw(const Mk::wg2::Rect& r) {
	if (!render) return;
	// 演示区滚动交给框架 ChildRegion：无边框、零内边距（页面层已提供滚动与留白）
	Mk::wg2::ChildRegion<Mk::wg2::Id("demoRegion")> region;
	region.idname = "page_demo_scroll";
	region.padding = ImVec2(0.0f, 0.0f);
	region.border = false;
	region.body = [&](const ImVec2& cp) {
		const float innerW = std::max(0.0f, Mk::wg2::metrics::availRegion().x);
		// 按自然高度排版：demo 格被撑高时多出的高度留白，避免 VBox 把子项拉散
		float contentH = r.h;
		if (measure) contentH = measure(Mk::wg2::Size{innerW, 0.0f}).h;
		render(Mk::wg2::Rect{cp.x, cp.y, innerW, contentH});
	};
	region.draw(r);
}
};

// ============ 页面架 ============

// 演示页 = 三个固定区块 + 具体演示内容
//   内容类约定：static constexpr title/desc/code + 名为 root 的控件树
template<class Content>
struct DemoPage {
	using Chrome = Mk::wg2::VBox<
		Mk::wg2::Id("page"),
		SectionHead<Mk::wg2::Id("head")>,
		PageDemo<Mk::wg2::Id("demo")>,
		Mk::wg2::SeparatorText<Mk::wg2::Id("codeTitle")>,
		CodeBlock<Mk::wg2::Id("code")>>;

	Chrome chrome;
	Content content;
	const char* title = "";

	DemoPage() = default;
	template<class... Args>
	explicit DemoPage(Args&&... args) : content(std::forward<Args>(args)...) {}

	void setup() {
		title = Content::title;
		chrome.at<Mk::wg2::Id("head")>().title = Content::title;
		chrome.at<Mk::wg2::Id("head")>().desc = Content::desc;
		chrome.at<Mk::wg2::Id("codeTitle")>().name = "Code";
		chrome.at<Mk::wg2::Id("code")>().code = Content::code;
		chrome.marginX = 0.0f;
		chrome.marginY = 0.0f;
		chrome.spacing = 8.0f;   // 常规档

		chrome.at<Mk::wg2::Id("demo")>().render  = [this](const Mk::wg2::Rect& r) { Mk::wg2::layout(content.root, r); };
		chrome.at<Mk::wg2::Id("demo")>().measure = [this](const Mk::wg2::Size& a) { return content.root.hint(a); };
	}

	void renderTo(const Mk::wg2::Rect& r) {
		if (r.w <= 1.0f || r.h <= 1.0f) return;
		// 位置由 ChildRegion 统一负责（它按矩形 SetCursorPos），这里只透传
		Mk::wg2::ChildRegion<Mk::wg2::Id("pageRegion")> region;
		region.idname = "page_body";
		region.padding = ImVec2(12.0f, 8.0f);   // 页面四周留白（与全局间距同一刻度）
		// 铺满页面可用宽度：留白只来自页边距
		Mk::wg2::Rect box = r;
		region.body = [&](const ImVec2& cp) {
			const ImVec2 raw = Mk::wg2::metrics::availRegion();
			// 宽度不超过窗口可视区：父级可用宽度可能因滚动内容偏大
			float w = raw.x;
			if (const ImGuiViewport* vp = ImGui::GetMainViewport()) {
				const ImVec2 sp = ImGui::GetCursorScreenPos();
				w = std::min(w, vp->Pos.x + vp->Size.x - sp.x - ImGui::GetStyle().WindowPadding.x);
			}
			const Mk::wg2::Size want = chrome.hint(Mk::wg2::Size{std::max(0.0f, w), 0.0f});
			const float h = want.h > 1.0f ? want.h : raw.y;
			Mk::wg2::layout(chrome, Mk::wg2::Rect{cp.x, cp.y, std::max(0.0f, w), h});
		};
		region.draw(box);
	}
};

// ============ 演示界面：30 页与注册表 ============

// —— 侧栏分组（与 pages/ 子目录一一对应；组是可展开的树节点）——
inline constexpr const char* kGroupLayout     = "布局";
inline constexpr const char* kGroupWg         = "wg 体系";
inline constexpr const char* kGroupAtoms      = "原子控件";
inline constexpr const char* kGroupContainers = "容器控件";
inline constexpr const char* kGroupPopColor   = "弹窗与颜色";
inline constexpr const char* kGroupDeco       = "装饰件";
inline constexpr const char* kGroupApp        = "应用";

// 演示界面：声明顺序 = 侧栏顺序
struct DemoPages {

	DemoPage<PageLayoutBasic>   pageLayoutBasic;
	DemoPage<PageLayoutSize>    pageLayoutSize;
	DemoPage<PageLayoutGrid>    pageLayoutGrid;
	DemoPage<PageLayoutRuntime> pageLayoutRuntime;
	DemoPage<PageLayoutDynamic> pageLayoutDynamic;
	DemoPage<PageDock>          pageDock;
	DemoPage<PageBase>          pageBase;
	DemoPage<PageLifecycle>     pageLifecycle;
	DemoPage<PageWg2>           pageWg2;
	DemoPage<PageWg2Catalog>    pageWg2Catalog;
	DemoPage<PageLayout>        pageLayout;
	DemoPage<PageSplit>         pageSplit;
	DemoPage<PageText>          pageText;
	DemoPage<PageInput>         pageInput;
	DemoPage<PageNumber>        pageNumber;
	DemoPage<PageDrag>          pageDrag;
	DemoPage<PageSelect>        pageSelect;
	DemoPage<PageFeedback>      pageFeedback;
	DemoPage<PageContainer>     pageContainer;
	DemoPage<PageTab>           pageTab;
	DemoPage<PageMenu>          pageMenu;
	DemoPage<PageTable>         pageTable;
	DemoPage<PagePopup>         pagePopup;
	DemoPage<PageColor>         pageColor;
	DemoPage<PageDeco>          pageDeco;
	DemoPage<PageTheme>         pageTheme;
	DemoPage<PageWindow>        pageWindow;
	DemoPage<PageFile>          pageFile;
	DemoPage<PageFnRender>      pageFnRender;
	DemoPage<PageButton>        pageButton;

	// 依赖注入：演示页只认识自己需要的句柄（纹理 / 窗口 / 检查器）
	void inject(Mk::window* win, Mk::window* ins) {
		pageFeedback.content.win = win;
		pageMenu.content.win = win;
		pageWindow.content.win = win;
		pageWindow.content.inspectorWin = ins;
		pageFile.content.attach(win);
	}

	// 注册：分组进导航树，页条目 + 渲染回调进页簿
	template<class Book>
	void registerAll(Book& book) {
		auto add = [&](auto& page, const char* group = nullptr) {
			page.setup();
			if (group && *group) book.addGroup(group);
			book.addPage(page.title, [&page](const Mk::wg2::Rect& r) { page.renderTo(r); });
		};
		add(pageLayoutBasic, kGroupLayout);
		add(pageLayoutSize);
		add(pageLayoutGrid);
		add(pageLayoutRuntime);
		add(pageLayoutDynamic);
		add(pageDock);   // 停靠布局
		add(pageBase, kGroupWg);
		add(pageLifecycle);
		add(pageWg2);
		add(pageWg2Catalog);
		add(pageLayout);
		add(pageSplit);
		add(pageText, kGroupAtoms);
		add(pageButton);
		book.beginGroup("输入");   // 子分组：树形展开演示
		add(pageInput);
		add(pageNumber);
		add(pageDrag);
		add(pageSelect);
		book.endGroup();
		add(pageFeedback);
		add(pageContainer, kGroupContainers);
		add(pageTab);
		add(pageMenu);
		add(pageTable);
		add(pagePopup, kGroupPopColor);
		add(pageColor);
		add(pageDeco, kGroupDeco);
		add(pageTheme, kGroupApp);
		add(pageWindow);
		add(pageFile);
		add(pageFnRender);
	}
};

} // namespace gallery
