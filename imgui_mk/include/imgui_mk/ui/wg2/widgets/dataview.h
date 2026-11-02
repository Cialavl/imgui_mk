#pragma once
// Mk::wg2 数据视图：虚拟列表 / 数据表格（ImGuiListClipper 只画可见行）
//   列表：行文本可用静态数组或 text(i) 回调；支持单选、双击激活、滚动定位。
//   表格：可排序表头、可拖列宽、行选中、双击激活；行数很大也只画可见行。
#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/widgets/selection.h>

namespace Mk::wg2 {

namespace detail {

// 数字感知比较："file2" < "file10"，"2" < "10"；其余按字节序
inline bool cellLess(const std::string& a, const std::string& b) {
	std::size_t i = 0;
	std::size_t j = 0;
	while (i < a.size() && j < b.size()) {
		const unsigned char ca = static_cast<unsigned char>(a[i]);
		const unsigned char cb = static_cast<unsigned char>(b[j]);
		if (std::isdigit(ca) && std::isdigit(cb)) {
			std::size_t ia = i;
			std::size_t jb = j;
			while (ia < a.size() && std::isdigit(static_cast<unsigned char>(a[ia]))) ++ia;
			while (jb < b.size() && std::isdigit(static_cast<unsigned char>(b[jb]))) ++jb;
			// 去前导零后按数值比较
			std::size_t za = i;
			std::size_t zb = j;
			while (za + 1 < ia && a[za] == '0') ++za;
			while (zb + 1 < jb && b[zb] == '0') ++zb;
			const std::size_t la = ia - za;
			const std::size_t lb = jb - zb;
			if (la != lb) return la < lb;
			const int c = a.compare(za, la, b, zb, lb);
			if (c != 0) return c < 0;
			i = ia;
			j = jb;
			continue;
		}
		if (ca != cb) return ca < cb;
		++i;
		++j;
	}
	return (a.size() - i) < (b.size() - j);
}

} // namespace detail

// ============ 虚拟列表 ============
template<FixedString Id>
struct VirtualList {
	static constexpr auto id = Id;

	std::string idname;
	int rowCount = 0;
	float rowHeight = 0.0f;                  // <=0：按字高 + 行距
	const char* const* items = nullptr;      // 静态行文本（与 text 二选一）
	std::function<std::string(int)> text;    // 行文本回调
	int* selected = nullptr;                 // 选中行（可空）
	SelectionModel* selection = nullptr;     // 多选模型（优先于 selected）
	std::function<void(int)> onSelect;       // 单击
	std::function<void(int)> onActivate;     // 双击
	int scrollTo = -1;                       // >=0：本帧滚动到该行（一次性）
	bool border = true;
	float height = 0.0f;                     // <=0：填满可用高度

	Size hint(const Size& avail) const {
		const float h = height > 0.0f ? height : (avail.h > 0.0f ? avail.h : 240.0f);
		return Size{avail.w > 0.0f ? avail.w : 260.0f, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const float rh = rowHeight > 0.0f ? rowHeight : ImGui::GetTextLineHeightWithSpacing();
		const char* cid = idname.empty() ? id.data : idname.c_str();
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const ImGuiChildFlags childFlags = border ? ImGuiChildFlags_Border : 0;
		if (ImGui::BeginChild(cid, ImVec2(r.w, r.h), childFlags, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
			ImGuiListClipper clipper;
			clipper.Begin(rowCount, rh);
			while (clipper.Step()) {
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
					const std::string label = text ? text(i)
						: (items && i >= 0 && i < rowCount ? std::string(items[i]) : std::string());
					const bool sel = selection ? selection->contains(i) : (selected && *selected == i);
					ImGui::PushID(i);
					if (ImGui::Selectable(label.c_str(), sel,
							ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, rh))) {
						if (selection) selection->click(i, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
						else if (selected) *selected = i;
						if (onSelect) onSelect(i);
						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && onActivate) onActivate(i);
					}
					if (i == scrollTo) ImGui::SetScrollHereY(0.5f);
					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
		scrollTo = -1;
	}
};

// ============ 数据表格 ============
template<FixedString Id>
struct DataTable {
	static constexpr auto id = Id;

	std::string idname;
	std::vector<std::string> headers;
	int rowCount = 0;
	std::function<std::string(int row, int col)> cell;   // 单元格文本
	int* selected = nullptr;                             // 选中行（可空）
	SelectionModel* selection = nullptr;                 // 多选模型（优先于 selected）
	std::function<void(int)> onActivate;                 // 双击
	float height = 0.0f;                                 // <=0：按可用高度（有上限）
	bool zebra = true;

	Size hint(const Size& avail) const {
		const float line = ImGui::GetTextLineHeightWithSpacing();
		const float wanted = (static_cast<float>(rowCount) + 1.0f) * line + 10.0f;
		const float h = height > 0.0f ? height
			: (avail.h > 0.0f ? std::min(wanted, avail.h) : wanted);
		return Size{avail.w > 0.0f ? avail.w : 480.0f, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (headers.empty() || !cell) return;
		ensureOrder();

		ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable
			| ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY;
		if (zebra) flags |= ImGuiTableFlags_RowBg;
		const char* tid = idname.empty() ? id.data : idname.c_str();
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const int cols = static_cast<int>(headers.size());
		if (!ImGui::BeginTable(tid, cols, flags, ImVec2(r.w, r.h))) return;

		ImGui::TableSetupScrollFreeze(0, 1);
		for (int c = 0; c < cols; ++c) {
			ImGui::TableSetupColumn(headers[static_cast<std::size_t>(c)].c_str(),
				ImGuiTableColumnFlags_DefaultSort, 0.0f, static_cast<ImGuiID>(c + 1));
		}
		ImGui::TableHeadersRow();

		ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
		if (specs && specs->SpecsDirty) {
			rebuildOrder(specs);
			specs->SpecsDirty = false;
		}

		const float line = ImGui::GetTextLineHeightWithSpacing();
		ImGuiListClipper clipper;
		clipper.Begin(rowCount, line);
		while (clipper.Step()) {
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
				const int row = order[static_cast<std::size_t>(i)];
				ImGui::TableNextRow();
				const bool sel = selection ? selection->contains(row) : (selected && *selected == row);
				for (int c = 0; c < cols; ++c) {
					ImGui::TableSetColumnIndex(c);
					const std::string val = cell(row, c);
					if (c == 0) {
						ImGui::PushID(row);
						if (ImGui::Selectable(val.c_str(), sel,
								ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick,
								ImVec2(0.0f, line))) {
							if (selection) selection->click(row, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
							else if (selected) *selected = row;
							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && onActivate) onActivate(row);
						}
						ImGui::PopID();
					}
					else {
						ImGui::TextUnformatted(val.c_str());
					}
				}
			}
		}
		ImGui::EndTable();
	}

	// 排序后的行索引（测试可直接用）
	void ensureOrder() {
		if (static_cast<int>(order.size()) != rowCount) {
			order.resize(static_cast<std::size_t>(rowCount));
			for (int i = 0; i < rowCount; ++i) order[static_cast<std::size_t>(i)] = i;
		}
	}

	void rebuildOrder(const ImGuiTableSortSpecs* specs) {
		ensureOrder();
		if (!specs || specs->SpecsCount == 0) return;
		const ImGuiTableColumnSortSpecs& spec = specs->Specs[0];
		const int col = static_cast<int>(spec.ColumnUserID) - 1;
		if (col < 0 || col >= static_cast<int>(headers.size())) return;
		const bool asc = spec.SortDirection != ImGuiSortDirection_Descending;
		std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
			const std::string va = cell(a, col);
			const std::string vb = cell(b, col);
			return asc ? detail::cellLess(va, vb) : detail::cellLess(vb, va);
		});
	}

	std::vector<int> order;
};

} // namespace Mk::wg2
