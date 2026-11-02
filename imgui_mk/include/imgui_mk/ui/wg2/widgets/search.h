#pragma once
// Mk::wg2 可搜索下拉：输入过滤 + 列表选择
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

namespace Mk::wg2 {

namespace detail {
inline bool containsNoCase(const std::string& hay, const std::string& needle) {
	if (needle.empty()) return true;
	auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
	const std::size_t n = needle.size();
	if (hay.size() < n) return false;
	for (std::size_t i = 0; i + n <= hay.size(); ++i) {
		bool ok = true;
		for (std::size_t k = 0; k < n; ++k) {
			if (lower(static_cast<unsigned char>(hay[i + k])) != lower(static_cast<unsigned char>(needle[k]))) {
				ok = false;
				break;
			}
		}
		if (ok) return true;
	}
	return false;
}

// 过滤后的索引表（大小写不敏感子串）
inline std::vector<int> filterItems(const std::vector<std::string>& items, const std::string& query) {
	std::vector<int> out;
	out.reserve(items.size());
	for (int i = 0; i < static_cast<int>(items.size()); ++i) {
		if (containsNoCase(items[static_cast<std::size_t>(i)], query)) out.push_back(i);
	}
	return out;
}
} // namespace detail

template<FixedString Id>
struct SearchCombo {
	static constexpr auto id = Id;

	std::string name;
	std::vector<std::string> items;
	int* selected = nullptr;                 // 选中索引（可空）
	std::string placeholder = "搜索...";
	std::function<void(int)> onChange;
	char query[128]{};

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 220.0f, ImGui::GetFrameHeight()};
	}
	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		const char* preview = placeholder.c_str();
		if (selected && *selected >= 0 && *selected < static_cast<int>(items.size())) {
			preview = items[static_cast<std::size_t>(*selected)].c_str();
		}
		const char* cid = name.empty() ? id.data : name.c_str();
		if (ImGui::BeginCombo(cid, preview)) {
			ImGui::InputTextWithHint("##filter", placeholder.c_str(), query, sizeof(query));
			ImGui::Separator();
			const std::vector<int> shown = detail::filterItems(items, query);
			if (shown.empty()) ImGui::TextDisabled("（无匹配）");
			for (int idx : shown) {
				const bool cur = selected && *selected == idx;
				if (ImGui::Selectable(items[static_cast<std::size_t>(idx)].c_str(), cur)) {
					if (selected) *selected = idx;
					if (onChange) onChange(idx);
				}
				if (cur) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
};

} // namespace Mk::wg2
