#pragma once
// gallery wg2 基础设施：PageNavWg2 / PageHostWg2。

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

void addEntry(const char* label, bool header) {
if (header && !entries.empty() && entries.back().header) return;
entries.push_back({label, header});
}

Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
// 行高/组标题高都按当前字号度量：字号随窗口比例缩放时导航高度同步变化
const float rowH = ImGui::GetFrameHeight();
const float headH = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
float h = 0.0f;
for (const auto& e : entries) h += e.header ? headH : rowH;
return Mk::wg2::Size{avail.w > 0.0f ? avail.w : 12.0f * ImGui::GetFontSize(), h};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
// BeginChild 即使返回 false 也必须配对 EndChild（否则 ImGui 窗口栈失衡）
// 卡片化：与内容宿主/右侧面板一致，统一为带边框的圆角子窗（内部自动有 WindowPadding）
const bool open = ImGui::BeginChild("pagenav_wg2", ImVec2(r.w, r.h), ImGuiChildFlags_Border);
if (open) {
const ImGuiStyle& st = ImGui::GetStyle();
int idx = 0;
for (const auto& e : entries) {
if (e.header) {
// 组标题：与条目同左边界、暗色文字 + 全宽细分隔线；组间留一点呼吸空间
if (idx > 0) ImGui::Dummy(ImVec2(0.0f, st.ItemSpacing.y));
ImGui::TextDisabled("%s", e.label);
ImGui::Separator();
continue;
}
ImGui::PushID(idx);
const bool sel = current && (idx == *current);
const float avail = ImGui::GetContentRegionAvail().x - st.FramePadding.x * 2.0f;
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
};

std::vector<PageFn> pages;

template<class F, class G = std::nullptr_t>
void addPage(F&& render) {
		pages.push_back(PageFn{std::forward<F>(render)});
}


Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
return Mk::wg2::Size{avail.w, avail.h};
}

Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }

void draw(const Mk::wg2::Rect& r) {
const ImVec2 sz(r.w, r.h);
const bool open = ImGui::BeginChild("preview_wg2", sz, ImGuiChildFlags_Border,
ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
if (open) {
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
}
ImGui::EndChild();
}
};

} // namespace gallery
