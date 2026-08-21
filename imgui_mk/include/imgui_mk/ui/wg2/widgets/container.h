#pragma once
#include <algorithm>
#include <cstddef>
#include <string>
#include <tuple>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/static_layout.h>

// Mk::wg2 编译期容器/复合控件。
// 旧 wg 里这些控件通过 node.add 挂运行时子节点；这里改为模板参数 children。

namespace Mk::wg2 {

// ============ SubWindow（独立滚动的子窗口区域） ============

template<FixedString Id, class... Children>
struct SubWindow {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	float width = 0.0f;    // 0 = 填满可用宽度
	float height = 0.0f;   // 0 = 按内容
	bool border = true;
	float spacing = -1.0f;
	std::string idname;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float w = width > 0.0f ? width : avail.w;
		if (height > 0.0f) return Size{w, height};

		const float sp = spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.y;
		const float pad = ImGui::GetStyle().WindowPadding.y * 2.0f;
		float h = 0.0f;
		std::size_t n = 0;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				h += child.hint(Size{w, avail.h}).h;
				++n;
			};
			(one(c), ...);
		}, children);
		if (n > 1) h += sp * static_cast<float>(n - 1);
		return Size{w, n == 0 ? avail.h : h + pad};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const float hGuard = r.h > 1.0f ? r.h : ImGui::GetFrameHeight() * 2.0f;
		const char* cid = idname.empty() ? id.data : idname.c_str();
		if (!ImGui::BeginChild(cid, ImVec2(r.w, hGuard),
			border ? ImGuiChildFlags_Border : 0)) {
			return;
		}

		const float sp = spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.y;
		float y = 0.0f;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(Size{r.w, r.h - y});
				drawWidget(child, Rect{0.0f, y, r.w, s.h});
				y += s.h + sp;
			};
			(one(c), ...);
		}, children);
		ImGui::EndChild();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

template<FixedString Id, class... Children>
using Panel = SubWindow<Id, Children...>;

// ============ Tab / TabBar（编译期页签） ============

template<FixedString Id, class... Children>
struct Tab {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	std::string title;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				w = std::max(w, s.w);
				h = std::max(h, s.h);
			};
			(one(c), ...);
		}, children);
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		float y = r.y;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(Size{r.w, r.h - (y - r.y)});
				drawWidget(child, Rect{r.x, y, s.w, s.h});
				y += s.h;
			};
			(one(c), ...);
		}, children);
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

template<FixedString Id, class... Children>
struct TabBar {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	std::string idname;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				w = std::max(w, s.w);
				h = std::max(h, s.h);
			};
			(one(c), ...);
		}, children);
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const char* cid = idname.empty() ? id.data : idname.c_str();
		if (!ImGui::BeginTabBar(cid)) return;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible && ImGui::BeginTabItem(child.title.c_str())) {
					drawWidget(child, r);
					ImGui::EndTabItem();
				}
			};
			(one(c), ...);
		}, children);
		ImGui::EndTabBar();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

// ============ Collapsing（折叠面板，单编译期子控件） ============

template<FixedString Id, class Child>
struct Collapsing {
	static constexpr auto id = Id;

	WidgetState<> state;
	Child child;
	std::string title;
	bool defaultOpen = false;

	Size hint(const Size& avail) const {
		const float header = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		const Size cs = child.hint(avail);
		return Size{std::max(avail.w, cs.w), header + cs.h};
	}

	Policy policy() const { return child.policy(); }

	void draw(const Rect& r) {
		ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (ImGui::CollapsingHeader(title.empty() ? id.data : title.c_str(), flags)) {
			const float header = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
			const Rect inner{r.x, r.y + header, r.w, std::max(0.0f, r.h - header)};
			drawWidget(child, inner);
		}
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		static_assert(fixed_equal<Child::id, First>(), "wg2: Collapsing child id mismatch");
		if constexpr (sizeof...(Rest) == 0) return (child);
		else return child.template at<Rest...>();
	}
};

} // namespace Mk::wg2
