#pragma once
#include <type_traits>
#include <utility>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 静态装饰件：包装一个编译期子控件，不再使用运行时 node.add。

namespace Mk::wg2 {

template<FixedString Id, class Child>
struct When {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	bool show = false;

	Size hint(const Size& avail) const {
		if (!show) return Size{0.0f, 0.0f};
		return child.hint(avail);
	}

	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		if (show) drawWidget(child, r);
	}

	constexpr Child& childRef() & { return child; }
	constexpr const Child& childRef() const & { return child; }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: decorator child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

template<FixedString Id, class Child>
struct ItemWidth {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	float width = 0.0f;

	Size hint(const Size& avail) const { return child.hint(avail); }
	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		ImGui::PushItemWidth(width);
		drawWidget(child, r);
		ImGui::PopItemWidth();
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
