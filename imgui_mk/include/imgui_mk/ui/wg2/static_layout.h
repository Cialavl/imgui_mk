#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 静态布局容器。
//
// 子节点经模板参数传入：VBox<Id("id"), A, B, C>。
// 路径访问：box.at<Id("a"), Id("b")>()；重复/缺失 ID 在编译期报错。
// 动态子集合请使用 dynamic.h 的 RuntimeList。

namespace Mk::wg2 {

namespace detail {

inline float spacingX(float s) { return s >= 0.0f ? s : ImGui::GetStyle().ItemSpacing.x; }
inline float spacingY(float s) { return s >= 0.0f ? s : ImGui::GetStyle().ItemSpacing.y; }
inline float marginX(float m) { return m >= 0.0f ? m : ImGui::GetStyle().WindowPadding.x; }
inline float marginY(float m) { return m >= 0.0f ? m : ImGui::GetStyle().WindowPadding.y; }
template<class W>
LayoutItem makeLayoutItem(const W& w, bool axis, float availCross) {
	const auto& ov = w.state.size;
	const Size hs = w.hint(Size{axis ? 0.0f : availCross, axis ? availCross : 0.0f});

	// em 覆盖换算：字号变化时自动跟随（字体缩放后无需应用层重设数值）
	auto emOr = [](float px, float rel) { return rel >= 0.0f ? rel * ImGui::GetFontSize() : px; };
	const float prefW = emOr(ov.prefW, ov.prefWEm), fixedW = emOr(ov.fixedW, ov.fixedWEm);
	const float minWv = emOr(ov.minW, ov.minWEm), maxWv = emOr(ov.maxW, ov.maxWEm);
	const float prefH = emOr(ov.prefH, ov.prefHEm), fixedH = emOr(ov.fixedH, ov.fixedHEm);
	const float minHv = emOr(ov.minH, ov.minHEm), maxHv = emOr(ov.maxH, ov.maxHEm);

	LayoutItem it;
	it.hint = axis ? hs.w : hs.h;
	it.mn = axis ? 0.0f : 0.0f;
	it.mx = 1.0e9f;

	if (axis) {
		if (prefW >= 0.0f) it.hint = prefW;
		if (fixedW >= 0.0f) { it.hint = fixedW; it.mn = fixedW; it.mx = fixedW; }
		if (minWv >= 0.0f) it.mn = std::max(it.mn, minWv);
		if (maxWv >= 0.0f) it.mx = std::min(it.mx, maxWv);
	} else {
		if (prefH >= 0.0f) it.hint = prefH;
		if (fixedH >= 0.0f) { it.hint = fixedH; it.mn = fixedH; it.mx = fixedH; }
		if (minHv >= 0.0f) it.mn = std::max(it.mn, minHv);
		if (maxHv >= 0.0f) it.mx = std::min(it.mx, maxHv);
	}
	it.mn = std::min(it.mn, it.hint);
	it.mn = std::min(it.mn, it.mx);

	const Policy p = w.policy();
	const Policy::Kind kind = axis ? p.h : p.v;
	it.stretch = w.state.stretch;
	it.canGrow = p.canGrow(kind) || it.stretch > 0.0f;
	it.canShrink = p.canShrink(kind);
	it.wantsExpand = p.wantsExpand(kind) || it.stretch > 0.0f;
	it.cross = axis ? hs.h : hs.w;
	return it;
}

template<class W>
void collectOne(const W& w, std::vector<LayoutItem>& out, bool axis, float availCross) {
	if (!w.state.visible) return;
	out.push_back(makeLayoutItem(w, axis, availCross));
}

template<class... Cs>
std::vector<LayoutItem> collectItems(const std::tuple<Cs...>& children, bool axis, float availCross) {
	std::vector<LayoutItem> out;
	std::apply([&](const auto&... c) { (collectOne(c, out, axis, availCross), ...); }, children);
	return out;
}

inline std::vector<float> solveRow(std::vector<LayoutItem>& items, float room) {
	const int cnt = static_cast<int>(items.size());
	std::vector<float> out(cnt, 0.0f);
	if (cnt == 0) return out;

	float sumHint = 0.0f, sumShrink = 0.0f, sumStretch = 0.0f, sumExpand = 0.0f;
	for (int i = 0; i < cnt; ++i) {
		out[i] = items[i].hint;
		sumHint += items[i].hint;
		sumShrink += items[i].canShrink ? (items[i].hint - items[i].mn) : 0.0f;
		if (items[i].canGrow && items[i].stretch > 0.0f) sumStretch += items[i].stretch;
		if (items[i].canGrow && items[i].wantsExpand) sumExpand += 1.0f;
	}

	if (sumHint > room && sumShrink > 0.0f) {
		const float k = std::min(1.0f, (sumHint - room) / sumShrink);
		for (int i = 0; i < cnt; ++i) {
			if (!items[i].canShrink) continue;
			out[i] = items[i].hint - (items[i].hint - items[i].mn) * k;
		}
	}

	float used = 0.0f;
	for (float v : out) used += v;

	const float extra = room - used;
	if (extra > 0.01f) {
		if (sumStretch > 0.0f) {
			for (int i = 0; i < cnt; ++i) {
				if (items[i].canGrow && items[i].stretch > 0.0f)
					out[i] += extra * (items[i].stretch / sumStretch);
			}
		} else if (sumExpand > 0.0f) {
			for (int i = 0; i < cnt; ++i) {
				if (items[i].canGrow && items[i].wantsExpand)
					out[i] += extra / sumExpand;
			}
		} else {
			int g = 0;
			for (int i = 0; i < cnt; ++i) if (items[i].canGrow) ++g;
			if (g > 0) {
				for (int i = 0; i < cnt; ++i) if (items[i].canGrow) out[i] += extra / static_cast<float>(g);
			}
		}
	}

	for (int i = 0; i < cnt; ++i) out[i] = std::clamp(out[i], items[i].mn, items[i].mx);
	return out;
}

template<class W, FixedString Key>
consteval bool childIdMatches() {
	if constexpr (requires { W::id; }) {
		return fixed_equal<W::id, Key>();
	} else {
		return false;
	}
}

template<FixedString Key, class... Cs>
consteval std::size_t childIndexById() {
	constexpr std::size_t n = sizeof...(Cs);
	constexpr auto idx = [] {
		constexpr std::array<std::size_t, n> matches = { childIdMatches<Cs, Key>() ? 1u : 0u ... };
		std::size_t found = n;
		std::size_t count = 0;
		for (std::size_t i = 0; i < n; ++i) {
			if (matches[i]) {
				if (found == n) found = i;
				++count;
			}
		}
		return std::pair{found, count};
	}();
	static_assert(idx.second == 1, "wg2: static child id not found, or duplicate id");
	return idx.first;
}

} // namespace detail

// ============ 垂直盒 ============

template<FixedString Id, class... Children>
struct VBox {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float innerW = std::max(0.0f, avail.w - mx * 2.0f);

		auto items = detail::collectItems(children, false, innerW);
		const float sp = detail::spacingY(spacing);
		float h = 0.0f, w = 0.0f;
		for (const auto& it : items) {
			h += it.hint;
			w = std::max(w, it.cross);
		}
		if (items.size() > 1) h += sp * static_cast<float>(items.size() - 1);
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float sp = detail::spacingY(spacing);
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);
		const float innerH = std::max(0.0f, r.h - my * 2.0f);

		auto items = detail::collectItems(children, false, innerW);
		// 子项之间还要插入 sp 间距：先扣掉再求解，否则最后一子项整体溢出 (n-1)*sp
		const float gaps = items.size() > 1 ? sp * static_cast<float>(items.size() - 1) : 0.0f;
		auto tracks = detail::solveRow(items, std::max(0.0f, innerH - gaps));

		std::size_t vi = 0;
		float y = r.y + my;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Rect cr{r.x + mx, y, innerW, tracks[vi]};
				drawWidget(child, cr);
				y += tracks[vi] + sp;
				++vi;
			};
			(one(c), ...);
		}, children);

		ImGui::SetCursorPos(saved);
	}

	template<std::size_t I>
	constexpr decltype(auto) child() & { return std::get<I>(children); }

	template<std::size_t I>
	constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() {
		return detail::childIndexById<Key, Children...>();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) {
			return c;
		} else {
			return c.template at<Rest...>();
		}
	}
};

// ============ 水平盒 ============

template<FixedString Id, class... Children>
struct HBox {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float innerH = std::max(0.0f, avail.h - my * 2.0f);

		auto items = detail::collectItems(children, true, innerH);
		const float sp = detail::spacingX(spacing);
		float w = 0.0f, h = 0.0f;
		for (const auto& it : items) {
			w += it.hint;
			h = std::max(h, it.cross);
		}
		if (items.size() > 1) w += sp * static_cast<float>(items.size() - 1);
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float sp = detail::spacingX(spacing);
		const float innerH = std::max(0.0f, r.h - my * 2.0f);
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);

		auto items = detail::collectItems(children, true, innerH);
		// 同上：列间距先扣除，避免末列被挤出容器右缘
		const float gaps = items.size() > 1 ? sp * static_cast<float>(items.size() - 1) : 0.0f;
		auto tracks = detail::solveRow(items, std::max(0.0f, innerW - gaps));

		std::size_t vi = 0;
		float x = r.x + mx;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Rect cr{x, r.y + my, tracks[vi], innerH};
				drawWidget(child, cr);
				x += tracks[vi] + sp;
				++vi;
			};
			(one(c), ...);
		}, children);

		ImGui::SetCursorPos(saved);
	}

	template<std::size_t I>
	constexpr decltype(auto) child() & { return std::get<I>(children); }

	template<std::size_t I>
	constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() {
		return detail::childIndexById<Key, Children...>();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) {
			return c;
		} else {
			return c.template at<Rest...>();
		}
	}
};

// SplitH / SplitV：当前求解器已支持 stretch，语义与 HBox / VBox 相同。
template<FixedString Id, class... Children>
using SplitH = HBox<Id, Children...>;

template<FixedString Id, class... Children>
using SplitV = VBox<Id, Children...>;

// ============ 网格 ============

template<FixedString Id, class... Children>
struct Grid {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	int cols = 3;
	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float spx = detail::spacingX(spacing), spy = detail::spacingY(spacing);
		const int c = std::max(1, cols);

		auto items = detail::collectItems(children, true, 0.0f);
		const std::size_t rows = (items.size() + static_cast<std::size_t>(c) - 1) / static_cast<std::size_t>(c);

		float w = 0.0f, h = 0.0f;
		for (std::size_t i = 0; i < items.size() && i < static_cast<std::size_t>(c); ++i) w += items[i].hint;
		if (!items.empty() && items.size() > 1) w += spx * static_cast<float>(std::min<std::size_t>(items.size(), static_cast<std::size_t>(c)) - 1);

		std::vector<float> rowH(rows, 0.0f);
		for (std::size_t i = 0; i < items.size(); ++i) {
			rowH[i / static_cast<std::size_t>(c)] = std::max(rowH[i / static_cast<std::size_t>(c)], items[i].cross);
		}
		for (float v : rowH) h += v;
		if (rows > 1) h += spy * static_cast<float>(rows - 1);

		(void)avail;
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float spx = detail::spacingX(spacing), spy = detail::spacingY(spacing);
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);
		const int c = std::max(1, cols);

		auto items = detail::collectItems(children, true, 0.0f);
		const std::size_t rows = (items.size() + static_cast<std::size_t>(c) - 1) / static_cast<std::size_t>(c);
		const float colW = innerW > 0.0f
			? (innerW - spx * static_cast<float>(c - 1)) / static_cast<float>(c)
			: 0.0f;

		std::vector<float> rowH(rows, 0.0f);
		for (std::size_t i = 0; i < items.size(); ++i) {
			rowH[i / static_cast<std::size_t>(c)] = std::max(rowH[i / static_cast<std::size_t>(c)], items[i].cross);
		}

		std::size_t vi = 0;
		std::apply([&](auto&... child) {
			auto one = [&](auto& w) {
				if (!w.state.visible) return;
				const std::size_t ri = vi / static_cast<std::size_t>(c);
				const std::size_t ci = vi % static_cast<std::size_t>(c);
				const float x = r.x + mx + static_cast<float>(ci) * (colW + spx);
				const float y = r.y + my;
				float yy = y;
				for (std::size_t i = 0; i < ri; ++i) yy += rowH[i] + spy;
				drawWidget(w, Rect{x, yy, colW, rowH[ri]});
				++vi;
			};
			(one(child), ...);
		}, children);

		ImGui::SetCursorPos(saved);
	}

	template<std::size_t I>
	constexpr decltype(auto) child() & { return std::get<I>(children); }

	template<std::size_t I>
	constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() {
		return detail::childIndexById<Key, Children...>();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) {
			return c;
		} else {
			return c.template at<Rest...>();
		}
	}
};

// ============ 自动换行 ============

template<FixedString Id, class... Children>
struct Wrap {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float spx = detail::spacingX(spacing), spy = detail::spacingY(spacing);
		const float maxW = avail.w > 0.0f ? std::max(0.0f, avail.w - mx * 2.0f) : 1.0e9f;

		auto items = detail::collectItems(children, true, 0.0f);
		float x = 0.0f, y = 0.0f, rowH = 0.0f, usedW = 0.0f;
		for (auto& it : items) {
			if (x > 0.0f && x + it.hint > maxW) {
				y += rowH + spy;
				x = 0.0f;
				rowH = 0.0f;
			}
			x += it.hint;
			if (x > 0.0f) x += spx;   // 暂存；收尾时扣除最后一个
			rowH = std::max(rowH, it.cross);
			usedW = std::max(usedW, x);
		}
		if (rowH > 0.0f || !items.empty()) y += rowH;
		const float w = usedW > spx ? usedW - spx : 0.0f;
		return Size{w + mx * 2.0f, y + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float spx = detail::spacingX(spacing), spy = detail::spacingY(spacing);
		const float innerX = r.x + mx, innerY = r.y + my;
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);

		auto items = detail::collectItems(children, true, 0.0f);
		std::size_t vi = 0;
		float x = innerX, y = innerY, rowH = 0.0f;

		std::apply([&](auto&... child) {
			auto one = [&](auto& w) {
				if (!w.state.visible) return;
				const LayoutItem& it = items[vi];
				if (x > innerX && x + it.hint > innerX + innerW) {
					y += rowH + spy;
					x = innerX;
					rowH = 0.0f;
				}
				drawWidget(w, Rect{x, y, it.hint, it.cross});
				x += it.hint + spx;
				rowH = std::max(rowH, it.cross);
				++vi;
			};
			(one(child), ...);
		}, children);

		ImGui::SetCursorPos(saved);
	}

	template<std::size_t I>
	constexpr decltype(auto) child() & { return std::get<I>(children); }

	template<std::size_t I>
	constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() {
		return detail::childIndexById<Key, Children...>();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) {
			return c;
		} else {
			return c.template at<Rest...>();
		}
	}
};

// ============ 覆盖盒 ============

template<FixedString Id, class... Children>
struct Overlay {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	float marginX = -1.0f, marginY = -1.0f;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(Size{std::max(0.0f, avail.w - mx * 2.0f), 0.0f});
				w = std::max(w, s.w);
				h = std::max(h, s.h);
			};
			(one(c), ...);
		}, children);
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const Rect inner{r.x + mx, r.y + my, std::max(0.0f, r.w - mx * 2.0f), std::max(0.0f, r.h - my * 2.0f)};
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible) drawWidget(child, inner);
			};
			(one(c), ...);
		}, children);
		ImGui::SetCursorPos(saved);
	}

	template<std::size_t I>
	constexpr decltype(auto) child() & { return std::get<I>(children); }

	template<std::size_t I>
	constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() {
		return detail::childIndexById<Key, Children...>();
	}

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) {
			return c;
		} else {
			return c.template at<Rest...>();
		}
	}
};

} // namespace Mk::wg2
