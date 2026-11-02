#pragma once
// 演示页（DemoPage<Content>）：页头 + 演示场 + 代码段，三段固定
//   SectionHead + PageDemo(demo) + CodeBlock 编译期组成页面根。
//   每个页面把自己的 wg2 root 渲染/测量回调 addWidget 进 demo 槽位。
#include "snippets.h"
#include <functional>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>

#include <algorithm>
#include <string_view>

namespace gallery {



struct ThemeChanged {          // 换肤：各窗口自己 setTheme
	Mk::wg2::Theme theme;
};




using namespace Mk::wg2;   // 页面代码沿用 wg2 名字


// ============ 引擎访问助手（页面零窗口依赖）============


// ============ 节标题 ============

template<Mk::wg2::FixedString Id>
struct SectionHead {
	static constexpr auto id = Id;

	Mk::wg2::WidgetState<> state;
	const char* title = "";
	const char* desc = "";

	// 组合式：标题 + 灰色换行描述，高度由子控件汇总
	using Root = Mk::wg2::VBox<Mk::wg2::Id("headCol"),
		Mk::wg2::Label<Mk::wg2::Id("headTitle")>,
		Mk::wg2::Colored<Mk::wg2::Id("headDescColor"),
			Mk::wg2::TextWrapped<Mk::wg2::Id("headDesc")>>,
		Mk::wg2::Rule<Mk::wg2::Id("headRule")>>;
	Root root;

	Mk::wg2::Size hint(const Mk::wg2::Size& avail) const { return root.hint(avail); }
	Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

	void draw(const Mk::wg2::Rect& r) {
		root.at<Mk::wg2::Id("headTitle")>().text = title;
		root.at<Mk::wg2::Id("headDescColor"), Mk::wg2::Id("headDesc")>().text = desc;
		root.at<Mk::wg2::Id("headDescColor")>().color = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
		Mk::wg2::layout(root, Mk::wg2::Rect{r.x, r.y, r.w, r.h});
	}
};

// ============ 代码块（简化：等宽 + 行号 + Copy） ============

template<Mk::wg2::FixedString Id>
struct CodeBlock {
static constexpr auto id = Id;

Mk::wg2::WidgetState<> state;
std::string_view code;
ImFont* mono = nullptr;
std::function<ImFont*()> monoFn;
ImU32 copiedFrame = 0;

ImFont* activeMono() const { return monoFn ? monoFn() : mono; }

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

	Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

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
			int lineNo = 1;
			std::size_t pos = 0;
			while (pos <= code.size()) {
				std::size_t end = code.find('\n', pos);
				if (end == std::string_view::npos) end = code.size();
				ImGui::Text("%4d  ", lineNo);
				ImGui::SameLine();
				ImGui::TextUnformatted(code.data() + pos, code.data() + end);
				++lineNo;
				if (end == code.size()) break;
				pos = end + 1;
			}
		};

		Mk::wg2::FontScope font(activeMono());
		Mk::wg2::layout(root, r);
	}
};

// ============ 页面演示槽（可滚动） ============

template<Mk::wg2::FixedString Id>
struct PageDemo {
static constexpr auto id = Id;

Mk::wg2::WidgetState<> state;
std::function<void(const Mk::wg2::Rect&)> render;
std::function<Mk::wg2::Size(const Mk::wg2::Size&)> measure;

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
if (measure) return measure(avail);
return Mk::wg2::Size{avail.w, avail.h};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

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
	std::function<ImFont*()> monoProvider;   // 等宽代码字体（内容区注入，绘制时查询）

	void setMonoProvider(std::function<ImFont*()> f) { monoProvider = std::move(f); }

	DemoPage() = default;
	template<class... Args>
	explicit DemoPage(Args&&... args) : content(std::forward<Args>(args)...) {}

	void setup() {
		title = Content::title;
		chrome.at<Mk::wg2::Id("head")>().title = Content::title;
		chrome.at<Mk::wg2::Id("head")>().desc = Content::desc;
		chrome.at<Mk::wg2::Id("codeTitle")>().name = "Code";
		chrome.at<Mk::wg2::Id("code")>().code = Content::code;
		chrome.at<Mk::wg2::Id("code")>().monoFn = [this] { return monoProvider ? monoProvider() : nullptr; };
		chrome.marginX = 0.0f;
		chrome.marginY = 0.0f;
		chrome.at<Mk::wg2::Id("demo")>().render  = [this](const Mk::wg2::Rect& r) { Mk::wg2::layout(content.root, r); };
		chrome.at<Mk::wg2::Id("demo")>().measure = [this](const Mk::wg2::Size& a) { return content.root.hint(a); };
	}

	void renderTo(const Mk::wg2::Rect& r) {
		if (r.w <= 1.0f || r.h <= 1.0f) return;
		// 整页滚动：同样是框架 ChildRegion，页面里不再手写 BeginChild/PushStyleVar
		Mk::wg2::ChildRegion<Mk::wg2::Id("pageRegion")> region;
		region.idname = "page_body";
		region.padding = ImVec2(0.0f, 0.0f);
		region.body = [&](const ImVec2& cp) {
			const ImVec2 avail = Mk::wg2::metrics::availRegion();
			const Mk::wg2::Size want = chrome.hint(Mk::wg2::Size{avail.x, 0.0f});
			const float h = want.h > 1.0f ? want.h : avail.y;
			Mk::wg2::layout(chrome, Mk::wg2::Rect{cp.x, cp.y, avail.x, h});
		};
		region.draw(r);
	}
};

} // namespace gallery
