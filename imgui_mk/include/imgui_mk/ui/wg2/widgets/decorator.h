#pragma once
#include <type_traits>
#include <utility>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 静态装饰件：包装一个编译期子控件，不再使用运行时 node.add。

namespace Mk::wg2 {

template<FixedString Id, class Child>
struct ItemWidth {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	float width = 0.0f;

	Size hint(const Size& avail) const { return child.hint(avail); }
	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		// 用子项自身宽度作为绘制矩形：drawWidget 会按 r.w 设置 SetNextItemWidth，
		// 否则外层容器的整行宽度会覆盖这里的限宽意图（PushItemWidth 被 next-item 宽度覆盖）
		const float w = width > 0.0f ? width : r.w;
		drawWidget(child, Rect{r.x, r.y, w, r.h});
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

template<FixedString Id, class Child>
struct Indent {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	float amount = 0.0f;

	Size hint(const Size& avail) const {
		const Size s = child.hint(Size{std::max(0.0f, avail.w - amount), avail.h});
		return Size{s.w + amount, s.h};
	}

	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		const Rect inner{r.x + amount, r.y, std::max(0.0f, r.w - amount), r.h};
		ImGui::Indent(amount);
		drawWidget(child, inner);
		ImGui::Unindent(amount);
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

// 偏移装饰器：整体平移子控件（em 单位随字号缩放），用于 Overlay 叠放时错开元素，
// 避免两个子控件文字完全叠在一起。
template<FixedString Id, class Child>
struct Offset {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	float dxEm = 0.0f, dyEm = 0.0f;   // 以当前字号为单位
	float dx = 0.0f, dy = 0.0f;       // 额外像素

	Size hint(const Size& avail) const {
		const float ox = dxEm * ImGui::GetFontSize() + dx;
		const float oy = dyEm * ImGui::GetFontSize() + dy;
		const Size s = child.hint(Size{std::max(0.0f, avail.w - ox), std::max(0.0f, avail.h - oy)});
		return Size{s.w + std::max(0.0f, ox), s.h + std::max(0.0f, oy)};
	}

	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		const float ox = dxEm * ImGui::GetFontSize() + dx;
		const float oy = dyEm * ImGui::GetFontSize() + dy;
		drawWidget(child, Rect{r.x + ox, r.y + oy,
			std::max(1.0f, r.w - ox), std::max(1.0f, r.h - oy)});
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

template<FixedString Id, class Child>
struct Colored {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};

	Size hint(const Size& avail) const { return child.hint(avail); }
	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		drawWidget(child, r);
		ImGui::PopStyleColor();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

template<FixedString Id, class Child>
struct Tooltip {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	std::string text;

	Size hint(const Size& avail) const { return child.hint(avail); }
	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		drawWidget(child, r);
		if (!text.empty() && ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", text.c_str());
		}
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

} // namespace Mk::wg2
