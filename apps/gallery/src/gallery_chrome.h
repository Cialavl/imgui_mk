#pragma once
// gallery 页架（wg2 迁移版）：
//   SectionHead + PageDemo(demo) + CodeBlock 编译期组成页面根。
//   每个页面把自己的 wg2 root 渲染/测量回调 addWidget 进 demo 槽位。
#include "snippets.h"
#include "gallery_env.h"
#include "gallery_events.h"
#include <functional>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>

#include <string_view>

namespace gallery {


using namespace Mk::wg2;   // 页面代码沿用 wg2 名字


// ============ 引擎访问助手（页面零窗口依赖）============


// ============ 节标题 ============

template<Mk::wg2::FixedString Id>
struct SectionHead {
static constexpr auto id = Id;

Mk::wg2::WidgetState<> state;
const char* title = "";
const char* desc = "";

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
const ImVec2 d = ImGui::CalcTextSize(desc, nullptr, true, avail.w);
const ImVec2 t = ImGui::CalcTextSize(title);
return Mk::wg2::Size{avail.w, t.y + d.y + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
ImGui::TextUnformatted(title);
ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + r.w);
ImGui::TextDisabled("%s", desc);
ImGui::PopTextWrapPos();
ImGui::Separator();
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
const float em = ImGui::GetFontSize();
return ImVec2(0.55f * em, 0.75f * em);
}

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
std::size_t lines = 1;
for (char c : code) if (c == '\n') ++lines;
const float lineH = ImGui::GetTextLineHeightWithSpacing();
const ImVec2 pad = innerPad();
return Mk::wg2::Size{avail.w, ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + static_cast<float>(lines) * lineH + pad.y * 2.0f};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
const ImGuiStyle& st = ImGui::GetStyle();
const bool justCopied = ImGui::GetFrameCount() - copiedFrame < 60;
const char* copyLabel = justCopied ? "已复制" : "Copy";
const float w = ImGui::CalcTextSize(copyLabel).x + st.FramePadding.x * 2.0f;
const float availX = ImGui::GetContentRegionAvail().x;
ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availX - w);
if (ImGui::SmallButton(copyLabel)) {
ImGui::SetClipboardText(std::string(code).c_str());
copiedFrame = ImGui::GetFrameCount();
}

const ImVec2 pad = innerPad();   // 须在 PushFont 之前：按主字体字号算 em
ImFont* cm = activeMono();
if (cm) ImGui::PushFont(cm);
const float copyRow = ImGui::GetFrameHeight() + st.ItemSpacing.y;
const float childH = r.h > copyRow ? r.h - copyRow : 0.0f;
const float codeW = std::max(0.0f, ImGui::GetContentRegionAvail().x);
ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, pad);   // 代码块内边距：上下更松，避免贴边
if (ImGui::BeginChild("code_wg2", ImVec2(codeW, childH), ImGuiChildFlags_Border,
ImGuiWindowFlags_HorizontalScrollbar)) {
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
}
ImGui::EndChild();
ImGui::PopStyleVar();
if (cm) ImGui::PopFont();
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

// 页面层已提供整体滚动：这里按内容区宽度渲染，不再强制自带滚动条
// 同上：页面滚动层已提供内边距，演示区不再叠加
ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
if (!ImGui::BeginChild("page_demo_scroll", ImVec2(r.w, r.h), false, 0)) {
ImGui::EndChild();
ImGui::PopStyleVar();
return;
}
const ImVec2 cp = ImGui::GetCursorPos();
const float innerW = std::max(0.0f, ImGui::GetContentRegionAvail().x);
// 按自然高度排版：demo 格被 stretch 撑高时，多出的高度留白，
// 否则 VBox 会把子项拉成不均匀的大间距（页面看起来"散架"）
float contentH = r.h;
if (measure) contentH = measure(Mk::wg2::Size{innerW, 0.0f}).h;
render(Mk::wg2::Rect{cp.x, cp.y, innerW, contentH});
ImGui::EndChild();
ImGui::PopStyleVar();
}
};

// ============ 页面架 ============

// 页面基类：类型擦除，注册表/宿主统一持有（Page<DemoRoot> 各不相同）
struct PageCore {
	const char* title = "";
	virtual ~PageCore() = default;
	virtual void renderTo(const Mk::wg2::Rect& r) = 0;
};

// 页面 = 外壳（标题/说明/演示区/代码块）+ 内容树
//   page.setup(标题, 描述, 代码) 填外壳，并把演示槽挂到自己的 root 上
//   page.root.xxx              绑内容控件
template<class DemoRoot>
struct Page : PageCore {
	using Chrome = Mk::wg2::VBox<
		Mk::wg2::Id("page"),
		SectionHead<Mk::wg2::Id("head")>,
		PageDemo<Mk::wg2::Id("demo")>,
		Mk::wg2::SeparatorText<Mk::wg2::Id("codeTitle")>,
		CodeBlock<Mk::wg2::Id("code")>>;

	Chrome chrome;   // 外壳
	DemoRoot root;   // 内容（页面自己的控件树）

	void setup(const char* pageTitle, const char* desc, std::string_view snippet) {
		title = pageTitle;
		chrome.at<Mk::wg2::Id("head")>().title = pageTitle;
		chrome.at<Mk::wg2::Id("head")>().desc = desc;
		chrome.at<Mk::wg2::Id("codeTitle")>().name = "Code";
		chrome.at<Mk::wg2::Id("code")>().code = snippet;
		chrome.at<Mk::wg2::Id("code")>().monoFn = [] { return gMono; };
		// demo 不参与伸展：按自然高度自上而下排列
		chrome.marginX = 0.0f;
		chrome.marginY = 0.0f;
		// 挂载：演示槽直接渲染本页的内容树（Page 拥有 root，页面无需再 bind）
		chrome.at<Mk::wg2::Id("demo")>().render  = [this](const Mk::wg2::Rect& r) { Mk::wg2::layout(root, r); };
		chrome.at<Mk::wg2::Id("demo")>().measure = [this](const Mk::wg2::Size& a) { return root.hint(a); };
	}

	void renderTo(const Mk::wg2::Rect& r) override {
		if (r.w <= 1.0f || r.h <= 1.0f) return;
		// 整页滚动：本层接管滚动条（子页面不再各自滚动）
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (!ImGui::BeginChild("page_body", ImVec2(r.w, r.h), false, 0)) {
			ImGui::EndChild();
			ImGui::PopStyleVar();
			return;
		}
		const ImVec2 cp = ImGui::GetCursorPos();
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		// 按内容自然高度布局：剩余空间留在底部，避免 VBox 撑开出现大间隙
		const Mk::wg2::Size want = chrome.hint(Mk::wg2::Size{avail.x, 0.0f});
		const float h = want.h > 1.0f ? want.h : avail.y;
		Mk::wg2::layout(chrome, Mk::wg2::Rect{cp.x, cp.y, avail.x, h});
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
};

} // namespace gallery
