#pragma once
// Mk::wg2 选择模型：单选 / Ctrl 多选 / Shift 连选（列表、表格共用）
#include <algorithm>
#include <cstddef>
#include <vector>

namespace Mk::wg2 {

struct SelectionModel {
	std::vector<int> items;   // 选中项（升序去重）
	int anchor = -1;          // 连选锚点（上一次点击）
	int primary = -1;         // 最近选中项（"当前项"）

	bool empty() const { return items.empty(); }
	std::size_t size() const { return items.size(); }
	bool contains(int i) const { return std::binary_search(items.begin(), items.end(), i); }

	void clear() {
		items.clear();
		anchor = -1;
		primary = -1;
	}

	void selectOnly(int i) {
		items.assign(1, i);
		anchor = i;
		primary = i;
	}

	void add(int i) {
		if (!contains(i)) {
			items.insert(std::lower_bound(items.begin(), items.end(), i), i);
		}
		primary = i;
	}

	void remove(int i) {
		auto it = std::lower_bound(items.begin(), items.end(), i);
		if (it != items.end() && *it == i) items.erase(it);
		if (primary == i) primary = items.empty() ? -1 : items.back();
	}

	void toggle(int i) {
		if (contains(i)) remove(i);
		else add(i);
		anchor = i;
	}

	// 连选：包含 a..b 的闭区间
	void selectRange(int a, int b) {
		const int lo = std::min(a, b);
		const int hi = std::max(a, b);
		items.clear();
		for (int i = lo; i <= hi; ++i) items.push_back(i);
		primary = b;
	}

	// 点击行：ctrl = 切换，shift = 从锚点连选，否则单选
	void click(int i, bool ctrl, bool shift) {
		if (shift && anchor >= 0) {
			selectRange(anchor, i);
			return;   // 锚点保持
		}
		if (ctrl) {
			toggle(i);
			return;
		}
		selectOnly(i);
	}
};

} // namespace Mk::wg2
