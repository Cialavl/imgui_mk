#pragma once
// Mk::wg2 图表：折线 / 柱状（draw list 自绘，无第三方依赖）
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

namespace Mk::wg2 {

namespace detail {
// 数值 → 像素（y 轴翻转，超出范围钳制；lo==hi 落在中间）
inline float mapValue(float v, float lo, float hi, float yBottom, float yTop) {
	if (hi <= lo) return (yBottom + yTop) * 0.5f;
	const float t = std::clamp((v - lo) / (hi - lo), 0.0f, 1.0f);
	return yBottom + (yTop - yBottom) * t;
}
} // namespace detail

struct ChartSeries {
	std::vector<float> values;
	ImU32 color = 0;          // 0 = 主题 PlotLines 色
	std::string name;
};

template<FixedString Id>
struct LineChart {
	static constexpr auto id = Id;

	std::vector<ChartSeries> series;
	bool grid = true;
	std::string idname;

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 320.0f, avail.h > 0.0f ? avail.h : 160.0f};
	}
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (r.w <= 16.0f || r.h <= 16.0f) return;
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const ImVec2 o = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 a(o.x + 36.0f, o.y + 8.0f);
		const ImVec2 b(o.x + r.w - 8.0f, o.y + r.h - 8.0f);

		float lo = 0.0f;
		float hi = 1.0f;
		bool any = false;
		for (const auto& s : series) {
			for (float v : s.values) {
				if (!any) { lo = hi = v; any = true; }
				else { lo = std::min(lo, v); hi = std::max(hi, v); }
			}
		}
		if (!any) { lo = 0.0f; hi = 1.0f; }
		if (hi <= lo) hi = lo + 1.0f;

		if (grid) {
			const ImU32 g = ImGui::GetColorU32(ImGuiCol_Separator);
			for (int i = 0; i <= 4; ++i) {
				const float y = a.y + (b.y - a.y) * static_cast<float>(i) / 4.0f;
				dl->AddLine(ImVec2(a.x, y), ImVec2(b.x, y), g);
			}
		}
		const ImU32 axis = ImGui::GetColorU32(ImGuiCol_TextDisabled);
		dl->AddLine(ImVec2(a.x, a.y), ImVec2(a.x, b.y), axis);
		dl->AddLine(ImVec2(a.x, b.y), ImVec2(b.x, b.y), axis);

		for (const auto& s : series) {
			if (s.values.size() < 2) continue;
			const ImU32 col = s.color ? s.color : ImGui::GetColorU32(ImGuiCol_PlotLines);
			const float n = static_cast<float>(s.values.size() - 1);
			for (std::size_t i = 1; i < s.values.size(); ++i) {
				const float x0 = a.x + (b.x - a.x) * static_cast<float>(i - 1) / n;
				const float x1 = a.x + (b.x - a.x) * static_cast<float>(i) / n;
				const float y0 = detail::mapValue(s.values[i - 1], lo, hi, b.y, a.y);
				const float y1 = detail::mapValue(s.values[i], lo, hi, b.y, a.y);
				dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), col, 1.5f);
			}
		}

		char buf[32];
		std::snprintf(buf, sizeof(buf), "%.2f", hi);
		dl->AddText(ImVec2(o.x + 2.0f, a.y - 6.0f), axis, buf);
		std::snprintf(buf, sizeof(buf), "%.2f", lo);
		dl->AddText(ImVec2(o.x + 2.0f, b.y - 6.0f), axis, buf);
	}
};

template<FixedString Id>
struct BarChart {
	static constexpr auto id = Id;

	std::vector<float> values;
	ImU32 color = 0;   // 0 = 主题 PlotHistogram 色
	float gap = 2.0f;
	std::string idname;

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 320.0f, avail.h > 0.0f ? avail.h : 140.0f};
	}
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (r.w <= 16.0f || r.h <= 16.0f || values.empty()) return;
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const ImVec2 o = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 a(o.x + 36.0f, o.y + 8.0f);
		const ImVec2 b(o.x + r.w - 8.0f, o.y + r.h - 8.0f);

		float lo = 0.0f;
		float hi = 0.0f;
		for (float v : values) { lo = std::min(lo, v); hi = std::max(hi, v); }
		if (hi <= lo) hi = lo + 1.0f;

		const ImU32 col = color ? color : ImGui::GetColorU32(ImGuiCol_PlotHistogram);
		const float step = (b.x - a.x) / static_cast<float>(values.size());
		const float barW = std::max(1.0f, step - gap);
		const float zeroY = detail::mapValue(0.0f, lo, hi, b.y, a.y);
		for (std::size_t i = 0; i < values.size(); ++i) {
			const float x = a.x + step * static_cast<float>(i);
			const float y = detail::mapValue(values[i], lo, hi, b.y, a.y);
			const float top = std::min(y, zeroY);
			const float bottom = std::max(y, zeroY);
			dl->AddRectFilled(ImVec2(x, top), ImVec2(x + barW, bottom), col);
		}
		dl->AddLine(ImVec2(a.x, zeroY), ImVec2(b.x, zeroY), ImGui::GetColorU32(ImGuiCol_TextDisabled));
	}
};

} // namespace Mk::wg2
