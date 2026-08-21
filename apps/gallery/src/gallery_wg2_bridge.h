#pragma once
// gallery wg2 基础设施：PageNavWg2 / PageHostWg2。

#include "gallery_state.h"
#include <imgui_mk/ui/wg2/wg2.h>

#include <functional>
#include <string>
#include <vector>

namespace gallery {

// wg2 侧栏页面导航（替代旧 PageNav）
struct Wg2PageEntry { const char* label; bool header; };

template<Mk::wg2::FixedString Id>
struct PageNavWg2 {
static constexpr auto id = Id;

Mk::wg2::WidgetState<> state;
std::vector<Wg2PageEntry> entries;
int* current = nullptr;
float itemH = 22.0f;

void addEntry(const char* label, bool header) {
if (header && !entries.empty() && entries.back().header) return;
entries.push_back({label, header});
}

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
float h = 0.0f;
for (const auto& e : entries) h += e.header ? 26.0f : itemH;
return Mk::wg2::Size{avail.w > 0.0f ? avail.w : 220.0f, h};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
if (!ImGui::BeginChild("pagenav_wg2", ImVec2(r.w, r.h), 0)) return;
int idx = 0;
for (const auto& e : entries) {
if (e.header) {
ImGui::SeparatorText(e.label);
continue;
}
ImGui::PushID(idx);
const bool sel = current && (idx == *current);
const float avail = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
std::string clipped;
const char* label = e.label;
if (ImGui::CalcTextSize(e.label).x > avail) {
clipped = Mk::wg2::detail::ellipsize(e.label, avail);
label = clipped.c_str();
}
if (ImGui::Selectable(label, sel) && current) *current = idx;
ImGui::PopID();
++idx;
}
ImGui::EndChild();
}
};

// wg2 页面宿主：替代旧 PageHost，保存类型擦除的页面渲染回调。
template<Mk::wg2::FixedString Id>
struct PageHostWg2 {
static constexpr auto id = Id;

Mk::wg2::WidgetState<> state;
int* current = nullptr;

struct PageFn {
std::function<void(const Mk::wg2::Rect&)> render;
std::function<void(State&)> refresh;
};

std::vector<PageFn> pages;

void addPage(std::function<void(const Mk::wg2::Rect&)> render,
std::function<void(State&)> refresh = nullptr) {
pages.push_back(PageFn{std::move(render), std::move(refresh)});
}

void refreshAll(State& s) {
for (auto& p : pages) if (p.refresh) p.refresh(s);
}

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
return Mk::wg2::Size{avail.w, avail.h};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
const ImVec2 sz(r.w, r.h);
if (!ImGui::BeginChild("preview_wg2", sz, ImGuiChildFlags_Border,
ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
return;
}

const int cnt = static_cast<int>(pages.size());
if (current && *current >= 0 && *current < cnt && pages[*current].render) {
const ImVec2 cp = ImGui::GetCursorPos();
const ImVec2 avail = ImGui::GetContentRegionAvail();
pages[*current].render(Mk::wg2::Rect{
cp.x, cp.y,
avail.x > 0.0f ? avail.x : 0.0f,
avail.y > 0.0f ? avail.y : 0.0f
});
}
ImGui::EndChild();
}
};

} // namespace gallery
