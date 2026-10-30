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
		const ImVec2 wpad = ImGui::GetStyle().WindowPadding;
		const float pad = wpad.y * 2.0f;
		// 子窗口内容区比外框窄（左右 WindowPadding）：量子项时按内容宽，绘制时才能对齐
		const float innerW = std::max(0.0f, w - wpad.x * 2.0f);
		float h = 0.0f;
		std::size_t n = 0;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				h += child.hint(Size{innerW, avail.h}).h;
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
		const bool open = ImGui::BeginChild(cid, ImVec2(r.w, hGuard),
			border ? ImGuiChildFlags_Border : 0);
		if (open) {
			// 子项按内容区（去掉左右 WindowPadding）定位与定宽，否则右侧标签会被子窗口裁掉
			const ImVec2 origin = ImGui::GetCursorPos();
			const float innerW = std::max(0.0f, ImGui::GetContentRegionAvail().x);
			const float sp = spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.y;
			float y = 0.0f;
			std::apply([&](auto&... c) {
				auto one = [&](auto& child) {
					if (!child.state.visible) return;
					const Size s = child.hint(Size{innerW, r.h - y});
					drawWidget(child, Rect{origin.x, origin.y + y, innerW, s.h});
					y += s.h + sp;
				};
				(one(c), ...);
			}, children);
		}
		ImGui::EndChild();   // BeginChild 即使返回 false 也必须配对 EndChild
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
		// 页签内容在页签条之后流式排布：以当前光标为原点逐行下移。
		// （旧实现用 Rect{0,0,0,0} 里的绝对坐标定位，内容会被画到窗口左上角、压在页签条下面）
		const ImVec2 origin = ImGui::GetCursorPos();
		const float w = r.w > 0.0f ? r.w : ImGui::GetContentRegionAvail().x;
		float y = origin.y;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(Size{w, 0.0f});
				drawWidget(child, Rect{origin.x, y, w, s.h});
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
		// 页签条（页签名那一行）本身也占高度：只报内容高会让后续控件压到页签内容上
		const float barH = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		return Size{w, barH + h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const char* cid = idname.empty() ? id.data : idname.c_str();
		if (!ImGui::BeginTabBar(cid)) return;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				// 页签宽度取标签自然宽：容器统一设置的 SetNextItemWidth(r.w) 会把首个页签拉满整行
				const float tabW = ImGui::CalcTextSize(child.title.c_str()).x
					+ ImGui::GetStyle().FramePadding.x * 2.0f;
				ImGui::SetNextItemWidth(tabW);
				if (ImGui::BeginTabItem(child.title.c_str())) {
					drawFlow(child);   // 页签内容流式排布（位于 tab 条之后）
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
