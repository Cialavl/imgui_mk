#pragma once
#include <functional>
#include <string>
#include <utility>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 基础原子控件：直接 template struct，无 Node、无函数指针表。

namespace Mk::wg2 {

template<FixedString Id>
struct Label {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::string text;

	Label() = default;
	Label(std::string t) : text(std::move(t)) {}

	Size hint(const Size&) const {
		const ImVec2 s = ImGui::CalcTextSize(text.c_str());
		return Size{s.x, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		// 容器给了宽度时按宽度省略，避免长文本被右缘硬切（r.w == 0 为流式上下文，原样输出）
		if (r.w > 0.0f && ImGui::CalcTextSize(text.c_str()).x > r.w) {
			ImGui::TextUnformatted(detail::ellipsize(text, r.w).c_str());
			return;
		}
		ImGui::TextUnformatted(text.c_str());
	}
};

template<FixedString Id>
struct Button {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::string label;
	std::function<void()> onClick;

	Button() = default;
	Button(std::string l, std::function<void()> cb = {})
		: label(std::move(l)), onClick(std::move(cb)) {}

	template<class F>
	void setOnClick(F&& cb) { onClick = std::forward<F>(cb); }

	Size hint(const Size&) const {
		const ImVec2 s = ImGui::CalcTextSize(label.c_str());
		const ImGuiStyle& st = ImGui::GetStyle();
		return Size{
			s.x + st.FramePadding.x * 2.0f + st.ItemInnerSpacing.x * 2.0f,
			ImGui::GetFrameHeight()
		};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (ImGui::Button(label.c_str())) {
			if (onClick) onClick();
		}
	}
};

template<FixedString Id>
struct TextColored {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::string text;
	ImVec4 color{0.92f, 0.92f, 0.94f, 1.0f};

	TextColored() = default;
	TextColored(std::string t, ImVec4 c = {0.92f, 0.92f, 0.94f, 1.0f})
		: text(std::move(t)), color(c) {}

	Size hint(const Size&) const {
		const ImVec2 s = ImGui::CalcTextSize(text.c_str());
		return Size{s.x, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		ImGui::TextColored(color, "%s", text.c_str());
	}
};

template<FixedString Id>
struct Separator {
	static constexpr auto id = Id;

	WidgetState<> state;

	Size hint(const Size&) const {
		return Size{0.0f, ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		ImGui::Separator();
	}
};

// 裸绘制桥：把普通 callable 包装成静态控件（可再被 AnyWidget::own 包装）
template<FixedString Id, class F>
struct FnWidget {
	static constexpr auto id = Id;

	WidgetState<> state;
	F fn;

	FnWidget() = default;
	FnWidget(F f) : fn(std::move(f)) {}

	// 裸绘制默认占一行高：hint 为 0 时容器会给 0 高度，绘制内容会盖到下一个控件上
	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 220.0f, ImGui::GetFrameHeight()};
	}
	Policy policy() const { return Policy{Policy::Expanding, Policy::Preferred}; }

	void draw(const Rect& r) {
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		fn();
	}
};

} // namespace Mk::wg2
