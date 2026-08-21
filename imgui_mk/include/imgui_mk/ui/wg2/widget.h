#pragma once
#include <imgui.h>
#include <concepts>
#include <type_traits>
#include <imgui_mk/ui/wg2/core.h>

// Mk::wg2 静态控件协议与统一渲染包装。
//
// 静态控件 = 普通 struct，满足 StaticWidget：
//   static constexpr auto id;
//   WidgetState<> state;
//   Size hint(const Size& avail) const;
//   Policy policy() const;
//   void draw(const Rect& bounds);
//
// 容器用 drawWidget() 包装子节点，统一处理：
//   visible 跳过、enabled 禁用、ImGui ID、样式作用域、item 宽度。

namespace Mk::wg2 {

template<class W>
concept StaticWidget = requires(const W& w, W& mw, const Size& avail, const Rect& r) {
	{ w.hint(avail) } -> std::convertible_to<Size>;
	{ w.policy() } -> std::convertible_to<Policy>;
	mw.draw(r);
};

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
} // namespace detail

// ============ 统一渲染包装 ============

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void drawWidget(W& widget, const Rect& r) {
	if (!widget.state.visible) return;

	ImGui::SetCursorPos(ImVec2(r.x, r.y));
	// 静态树对象通常长期存活，地址即稳定 ID；临时对象应显式传 ID。
	ImGui::PushID(static_cast<const void*>(&widget));
	if (!widget.state.enabled) ImGui::BeginDisabled();
	if (r.w > 0.0f) ImGui::SetNextItemWidth(r.w);

	widget.state.style.push();
	widget.draw(r);
	widget.state.style.pop();

	if (!widget.state.enabled) ImGui::EndDisabled();
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
	w.state.size.fixedW = width;
	w.state.size.fixedH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setPrefSize(W&& w, float width, float height) {
	w.state.size.prefW = width;
	w.state.size.prefH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setMinSize(W&& w, float width, float height) {
	w.state.size.minW = width;
	w.state.size.minH = height;
}

template<class W>
requires StaticWidget<std::remove_cvref_t<W>>
inline void setMaxSize(W&& w, float width, float height) {
	w.state.size.maxW = width;
	w.state.size.maxH = height;
}

} // namespace Mk::wg2
