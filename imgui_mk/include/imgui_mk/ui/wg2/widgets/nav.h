#pragma once
// 导航 + 页面宿主（框架层）
//   NavList  : 侧栏页列表（组标题 + 可选中条目），选中下标写回 current
//   PageHost : 按选中下标渲染一页（类型擦除的渲染回调）
#include "../core.h"
#include "../static_layout.h"

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace Mk::wg2 {

namespace detail {

// 按可用宽度截断并加省略号（UTF-8 安全，按当前字体度量）
inline std::string ellipsizeToWidth(const char* text, float avail) {
	std::string s(text ? text : "");
	if (avail <= 0.0f || ImGui::CalcTextSize(s.c_str()).x <= avail) return s;
	const float ellW = ImGui::CalcTextSize("...").x;
	std::string out;
	for (std::size_t i = 0; i < s.size();) {
		const unsigned char c = static_cast<unsigned char>(s[i]);
		const std::size_t len = c < 0x80 ? 1 : (c >> 5) == 0x06 ? 2 : (c >> 4) == 0x0E ? 3 : 4;
		std::string next = out + s.substr(i, len);
		if (ImGui::CalcTextSize(next.c_str()).x + ellW > avail) break;
		out = std::move(next);
		i += len;
	}
	return out + "...";
}

} // namespace detail

// ---------- 侧栏导航 ----------

struct NavEntry {
	std::string label;
	bool header = false;
};

template<FixedString Id>
struct NavList {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::vector<NavEntry> entries;
	int* current = nullptr;          // 选中下标（页序号，组标题不计）

	void addEntry(std::string label, bool header = false) {
		if (header && !entries.empty() && entries.back().header) return;   // 连续组标题去重
		entries.push_back(NavEntry{std::move(label), header});
	}

	int pageCount() const {
		int n = 0;
		for (const auto& e : entries) if (!e.header) ++n;
		return n;
	}

	Size hint(const Size& avail) const {
		// 行高/组标题高按当前字号度量：字号缩放时导航高度同步变化
		const float rowH = ImGui::GetFrameHeight();
		const float headH = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
		float h = 0.0f;
		for (const auto& e : entries) h += e.header ? headH : rowH;
		return Size{avail.w > 0.0f ? avail.w : 12.0f * ImGui::GetFontSize(), h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		// BeginChild 即使返回 false 也必须配对 EndChild
		const bool open = ImGui::BeginChild("wg2_navlist", ImVec2(r.w, r.h), ImGuiChildFlags_Border);
		if (open) {
			const ImGuiStyle& st = ImGui::GetStyle();
			int idx = 0;
			for (const auto& e : entries) {
				if (e.header) {
					if (idx > 0) ImGui::Dummy(ImVec2(0.0f, st.ItemSpacing.y));
					ImGui::TextDisabled("%s", e.label.c_str());
					ImGui::Separator();
					continue;
				}
				ImGui::PushID(idx);
				const bool sel = current && idx == *current;
				const float avail = ImGui::GetContentRegionAvail().x - st.FramePadding.x * 2.0f;
				const bool needClip = ImGui::CalcTextSize(e.label.c_str()).x > avail;
				const std::string clipped = needClip ? detail::ellipsizeToWidth(e.label.c_str(), avail) : e.label;
				if (ImGui::Selectable(clipped.c_str(), sel) && current) *current = idx;
				ImGui::PopID();
				++idx;
			}
		}
		ImGui::EndChild();
	}
};

// ---------- 页面宿主 ----------

template<FixedString Id>
struct PageHost {
	static constexpr auto id = Id;

	WidgetState<> state;
	int* current = nullptr;
	std::vector<std::function<void(const Rect&)>> pages;

	template<class F>
	void addPage(F&& render) { pages.push_back(std::forward<F>(render)); }

	int pageCount() const { return static_cast<int>(pages.size()); }

	Size hint(const Size& avail) const { return Size{avail.w, avail.h}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const bool open = ImGui::BeginChild("wg2_pagehost", ImVec2(r.w, r.h), ImGuiChildFlags_Border,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		if (open) {
			const int cnt = pageCount();
			if (current && *current >= 0 && *current < cnt && pages[*current]) {
				const ImVec2 cp = ImGui::GetCursorPos();
				const ImVec2 avail = ImGui::GetContentRegionAvail();
				pages[*current](Rect{cp.x, cp.y,
					avail.x > 0.0f ? avail.x : 0.0f,
					avail.y > 0.0f ? avail.y : 0.0f});
			}
		}
		ImGui::EndChild();
	}
};

// ---------- 页簿：导航 + 宿主合成一个节点 ----------

template<FixedString Id>
struct PageBook {
	static constexpr auto id = Id;

	WidgetState<> state;

	NavList<Id> nav;
	PageHost<Id> host;
	int current = 0;              // 当前页序号（组标题不计）
	float navWidthEm = 9.3f;      // 侧栏宽度（按字号换算）

	template<class F>
	void addPage(std::string title, F&& render) {
		nav.addEntry(title, false);
		host.addPage(std::forward<F>(render));
	}
	void addGroup(std::string title) { nav.addEntry(std::move(title), true); }

	int pageCount() const { return host.pageCount(); }
	const char* currentTitle() const {
		return current >= 0 && current < static_cast<int>(nav.entries.size())
			? nav.entries[static_cast<std::size_t>(current)].label.c_str() : "";
	}

	Size hint(const Size& avail) const { return Size{avail.w, avail.h}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		nav.current = &current;
		host.current = &current;
		const float gap = ImGui::GetStyle().ItemSpacing.x;
		const float navW = navWidthEm * ImGui::GetFontSize();
		const float hostW = r.w - navW - gap > 1.0f ? r.w - navW - gap : 1.0f;
		Mk::wg2::layout(nav, Rect{r.x, r.y, navW, r.h});
		Mk::wg2::layout(host, Rect{r.x + navW + gap, r.y, hostW, r.h});
	}
};

} // namespace Mk::wg2
