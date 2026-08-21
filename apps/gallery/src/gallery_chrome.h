#pragma once
// gallery 页架（wg2 迁移版）：
//   SectionHead + PageDemo(demo) + CodeBlock 编译期组成页面根。
//   每个页面把自己的 wg2 root 渲染/测量回调 addWidget 进 demo 槽位。
#include "gallery_state.h"
#include "snippets.h"
#include <functional>
#include <imgui_mk/ui/wg2/wg2.h>

#include <string_view>

namespace gallery {

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

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
std::size_t lines = 1;
for (char c : code) if (c == '\n') ++lines;
const float lineH = ImGui::GetTextLineHeightWithSpacing();
return Mk::wg2::Size{avail.w, ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + static_cast<float>(lines) * lineH};
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

ImFont* cm = activeMono();
if (cm) ImGui::PushFont(cm);
const float copyRow = ImGui::GetFrameHeight() + st.ItemSpacing.y;
const float childH = r.h > copyRow ? r.h - copyRow : 0.0f;
if (ImGui::BeginChild("code_wg2", ImVec2(r.w, childH), ImGuiChildFlags_Border)) {
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

float contentH = r.h;
if (measure) {
const Mk::wg2::Size desired = measure(Mk::wg2::Size{r.w, 0.0f});
contentH = std::max(r.h, desired.h);
}

if (ImGui::BeginChild("page_demo_scroll", ImVec2(r.w, r.h), false,
ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
const ImVec2 cp = ImGui::GetCursorPos();
render(Mk::wg2::Rect{cp.x, cp.y, r.w, contentH});
}
ImGui::EndChild();
}
};

// ============ 页面架 ============

struct Page {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("page"),
SectionHead<Mk::wg2::Id("head")>,
PageDemo<Mk::wg2::Id("demo")>,
Mk::wg2::SeparatorText<Mk::wg2::Id("codeTitle")>,
CodeBlock<Mk::wg2::Id("code")>
>;

Root root;
const char* title = "";

void setup(const char* pageTitle, const char* desc, std::string_view snippet) {
title = pageTitle;
root.at<Mk::wg2::Id("head")>().title = pageTitle;
root.at<Mk::wg2::Id("head")>().desc = desc;
root.at<Mk::wg2::Id("codeTitle")>().name = "Code";
root.at<Mk::wg2::Id("code")>().code = snippet;
root.at<Mk::wg2::Id("code")>().monoFn = [] { return gMono; };
root.at<Mk::wg2::Id("demo")>().state.stretch = 1.0f;
root.marginX = 0.0f;
root.marginY = 0.0f;
}

void addWidget(std::function<void(const Mk::wg2::Rect&)> render,
std::function<Mk::wg2::Size(const Mk::wg2::Size&)> measure = nullptr) {
root.at<Mk::wg2::Id("demo")>().render = std::move(render);
root.at<Mk::wg2::Id("demo")>().measure = std::move(measure);
}

void renderTo(const Mk::wg2::Rect& r) {
Mk::wg2::layout(root, r);
}
};

} // namespace gallery
