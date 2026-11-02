#pragma once
#include <imgui.h>
#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <imgui_mk/ui/wg2/core.h>

// Mk::wg2 静态控件协议与统一渲染包装。
//
// 静态控件 = 普通 struct，满足 StaticWidget：
//   static constexpr auto id;
//   Size hint(const Size& avail) const;
//   Policy policy() const;
//   void draw(const Rect& bounds);
//
// 状态（可见性 / 可用性 / 尺寸覆盖 / 布局权重 / 样式作用域）不放在控件里：
// 由引擎按控件地址统一登记，读取入口是 widgetChrome(w)，外部用 setVisible / setFixedSize / setStretch 配置。
//
// 容器用 drawWidget() 包装子节点，统一处理：
//   visible 跳过、enabled 禁用、ImGui ID、样式作用域、item 宽度。

namespace Mk::wg2 {

// 渲染开关：引擎可见性（setVisible / setVisibleFn）+ 控件可选 shouldRender()
template<class W>
inline bool renderable(const W& w) {
	if (!widgetChrome(w).shown()) return false;
	if constexpr (requires { w.shouldRender(); }) return w.shouldRender();
	else return true;
}

// 组合控件的布局根：成员 root，或 root() 访问器（外部接口保持不变）
template<class W>
concept HasRoot = requires(W& w) { w.root; } || requires(W& w) { w.root(); };

template<class W>
requires HasRoot<W>
inline decltype(auto) rootOf(W& w) {
	if constexpr (requires { w.root; } && !requires { w.root(); }) return (w.root);
	else return w.root();
}

// 尺寸/策略：控件可自带 hint/policy；组合控件由框架转发给它的 root。
template<class W>
inline Size hintOf(const W& w, const Size& avail) {
	if constexpr (requires { w.hint(avail); }) {
		return w.hint(avail);
	}
	else if constexpr (requires { rootOf(w); }) {
		const auto& r = rootOf(w);
		if (!renderable(r)) return Size{};
		return hintOf(r, avail);
	}
	else {
		return Size{};
	}
}

template<class W>
inline Policy policyOf(const W& w) {
	if constexpr (requires { w.policy(); }) return w.policy();
	else if constexpr (requires { rootOf(w); }) return policyOf(rootOf(w));
	else return Policy{};
}

// 静态控件：完整协议（hint/policy/draw），或只声明 root 的组合控件（框架驱动绘制）。
template<class W>
concept StaticWidget = requires(const W& w, const Size& avail) {
	{ hintOf(w, avail) } -> std::convertible_to<Size>;
	{ policyOf(w) } -> std::convertible_to<Policy>;
} && (requires(W& mw, const Rect& rr) { mw.draw(rr); } || requires(W& mw) { rootOf(mw); });

namespace detail {
inline float frameH() { return ImGui::GetFrameHeight(); }

// 有右侧标签时，设置 frame 宽度使 item 总宽恰为 totalW。
inline float frameWidthFor(float totalW, const char* name) {
	if (totalW <= 0.0f) return totalW;
	const float lw = (name && name[0])
		? ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemInnerSpacing.x
		: 0.0f;
	const float w = totalW - lw;
	return w > 0.0f ? w : totalW;
}

// UTF-8 首字节 -> 码点字节数（非法首字节按 1 字节前进，保证不越界）
inline std::size_t utf8CharLen(unsigned char c) {
	if (c < 0x80) return 1;
	if ((c >> 5) == 0x06) return 2;
	if ((c >> 4) == 0x0E) return 3;
	if ((c >> 3) == 0x1E) return 4;
	return 1;
}

// 截断文本至指定显示宽（含 "..."）；不超宽则原样返回。只在码点边界切除，
// 避免半个汉字被 ImGui 当非法 UTF-8 渲染成 '?'。
inline std::string ellipsize(const std::string& text, float avail) {
	if (avail <= 0.0f || text.empty()) return text;
	if (ImGui::CalcTextSize(text.c_str()).x <= avail) return text;
	constexpr const char* kDots = "...";
	const float dotsW = ImGui::CalcTextSize(kDots).x;
	std::size_t cut = 0;
	for (std::size_t i = 0; i < text.size(); ) {
		const std::size_t next = std::min(text.size(), i + utf8CharLen(static_cast<unsigned char>(text[i])));
		if (ImGui::CalcTextSize(text.substr(0, next).c_str()).x + dotsW > avail) break;
		cut = next;
		i = next;
	}
	return text.substr(0, cut) + kDots;
}
} // namespace detail

// ============ 样式作用域 ============
// 把 WidgetStyle 的覆盖项压进 ImGui 样式栈，析构时弹回（RAII）。
namespace detail {
struct StyleScope {
	int colors = 0;
	int vars = 0;
	bool fontPushed = false;

	explicit StyleScope(const WidgetStyle& s) {
		if (s.font && *s.font) { ImGui::PushFont(*s.font); fontPushed = true; }
		if (s.text) { ImGui::PushStyleColor(ImGuiCol_Text, *s.text); ++colors; }
		if (s.background) {
			ImGui::PushStyleColor(ImGuiCol_Button, *s.background);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, *s.background);
			ImGui::PushStyleColor(ImGuiCol_Header, *s.background);
			ImGui::PushStyleColor(ImGuiCol_Tab, *s.background);
			colors += 4;
		}
		if (s.hovered) {
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, *s.hovered);
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, *s.hovered);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, *s.hovered);
			colors += 3;
		}
		if (s.active) {
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, *s.active);
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, *s.active);
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, *s.active);
			colors += 3;
		}
		if (s.border) { ImGui::PushStyleColor(ImGuiCol_Border, *s.border); ++colors; }
		if (s.rounding) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, *s.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, *s.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, *s.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, *s.rounding);
			ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, *s.rounding);
			vars += 5;
		}
		if (s.borderSize) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, *s.borderSize);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, *s.borderSize);
			vars += 2;
		}
		if (s.padding) { ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, *s.padding); ++vars; }
		if (s.spacing) { ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, *s.spacing); ++vars; }
	}

	~StyleScope() {
		if (fontPushed) ImGui::PopFont();
		if (colors > 0) ImGui::PopStyleColor(colors);
		if (vars > 0) ImGui::PopStyleVar(vars);
	}

	StyleScope(const StyleScope&) = delete;
	StyleScope& operator=(const StyleScope&) = delete;
};
} // namespace detail

// ============ 统一渲染包装 ============

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void drawWidget(W& widget, const Rect& r) {
	auto& st = widgetChrome(widget);      // 控件状态统一问引擎要
	if (!st.shown()) return;              // 引擎可见性：先判，省掉后续工作

	ImGui::SetCursorPos(ImVec2(r.x, r.y));
	// 静态树对象通常长期存活，地址即稳定 ID；临时对象应显式传 ID。
	ImGui::PushID(static_cast<const void*>(&widget));
	if (!renderable(widget)) {            // 控件自身条件（如弹窗只在打开时提交）
		ImGui::PopID();
		return;
	}
	if (!st.enabled) ImGui::BeginDisabled();
	if (r.w > 0.0f) ImGui::SetNextItemWidth(r.w);

	if (st.focusRequest) {   // 焦点请求：作用于本控件绘制的第一个 item
		ImGui::SetKeyboardFocusHere();
		st.focusRequest = false;
	}

	detail::StyleScope styleScope(st.style);
	if constexpr (requires { widget.draw(r); }) {
		widget.draw(r);
	}
	else if constexpr (requires { rootOf(widget); }) {
		drawWidget(rootOf(widget), r);   // 组合控件：框架画 root
		if constexpr (requires { widget.overlays(); }) {
			std::apply([&](auto&... overlay) { (drawWidget(overlay, r), ...); }, widget.overlays());
		}
	}

	st.focused = ImGui::IsItemFocused();
	(void)styleScope;

	if (!st.enabled) ImGui::EndDisabled();
	ImGui::PopID();
}

// 流式包装：跟随光标排布（弹窗/菜单/主菜单栏等流式上下文专用）。
// 流式上下文中的子项应为流式原子（Label/Button/MenuItem…，draw() 忽略 rect）；
// 容器请用 drawWidget 显式定位。
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void drawFlow(W& widget) {
	auto& st = widgetChrome(widget);
	if (!st.shown()) return;
	ImGui::PushID(static_cast<const void*>(&widget));
	if (!renderable(widget)) {
		ImGui::PopID();
		return;
	}
	if (!st.enabled) ImGui::BeginDisabled();
	detail::StyleScope styleScope(st.style);
	if constexpr (requires { widget.draw(Rect{}); }) {
		widget.draw(Rect{0.0f, 0.0f, 0.0f, 0.0f});
	}
	else if constexpr (requires { rootOf(widget); }) {
		drawWidget(rootOf(widget), Rect{0.0f, 0.0f, 0.0f, 0.0f});
	}
	if (!st.enabled) ImGui::EndDisabled();
	ImGui::PopID();
}

// 根节点入口：静态树只需 layout(root, hostRect)
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void layout(W& w, const Rect& host) {
	drawWidget(w, host);
}

// ============ 尺寸覆盖便捷函数 ============

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setFixedSize(W&& w, float width, float height) {
	auto& st = widgetChrome(w);
	st.size.fixedW = width;
	st.size.fixedH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setPrefSize(W&& w, float width, float height) {
	auto& st = widgetChrome(w);
	st.size.prefW = width;
	st.size.prefH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setMinSize(W&& w, float width, float height) {
	auto& st = widgetChrome(w);
	st.size.minW = width;
	st.size.minH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setMaxSize(W&& w, float width, float height) {
	auto& st = widgetChrome(w);
	st.size.maxW = width;
	st.size.maxH = height;
}

// 可见性 / 可用性 / 布局权重：统一入口
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setVisible(W&& w, bool v) { widgetChrome(w).visible = v; }

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setEnabled(W&& w, bool v) { widgetChrome(w).enabled = v; }

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setStretch(W&& w, float s) { widgetChrome(w).stretch = s; }

// 拉取式可见性：每帧求值，优先于 setVisible
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setVisibleFn(W&& w, std::function<bool()> fn) { widgetChrome(w).visibleFn = std::move(fn); }

// —— 样式覆盖（所有控件通用：原子控件、容器、组合控件）——
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline WidgetStyle& styleOf(W&& w) { return widgetChrome(w).style; }

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setStyle(W&& w, WidgetStyle style) { widgetChrome(w).style = std::move(style); }

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void clearStyle(W&& w) { widgetChrome(w).style = WidgetStyle{}; }

// 焦点：请求下一次绘制把键盘焦点给该控件 / 查询最近一帧是否拥有焦点
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void requestFocus(W&& w) { widgetChrome(w).focusRequest = true; }

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline bool hasFocus(W&& w) { return widgetChrome(w).focused; }

// 单边尺寸设置：sizeOf(w).fixedW / .prefWEm / .maxH ...
template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline SizeOverrides& sizeOf(W&& w) { return widgetChrome(w).size; }

} // namespace Mk::wg2
