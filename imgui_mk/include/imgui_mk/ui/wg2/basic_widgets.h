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

	std::string text;
	// 拉取式文本：每帧取（标题/状态这类随外部状态变化的文本）
	std::function<std::string()> textFn;
	int textColor = -1;   // -1 = 默认；否则用该 ImGuiCol（随主题变化）

	Label() = default;
	Label(std::string t) : text(std::move(t)) {}

	Size hint(const Size&) const {
		const std::string shown = textFn ? textFn() : text;
		const ImVec2 s = ImGui::CalcTextSize(shown.c_str());
		return Size{s.x, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (textFn) text = textFn();
		if (textColor >= 0)
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(static_cast<ImGuiCol>(textColor)));
		// 容器给了宽度时按宽度省略，避免长文本被右缘硬切（r.w == 0 为流式上下文，原样输出）
		const std::string shown = (r.w > 0.0f && ImGui::CalcTextSize(text.c_str()).x > r.w)
			? detail::ellipsize(text, r.w) : text;
		ImGui::TextUnformatted(shown.c_str());
		if (textColor >= 0) ImGui::PopStyleColor();
	}
};

template<FixedString Id>
struct Button {
	static constexpr auto id = Id;

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

// 每帧驱动控件：不占布局空间，draw 时执行 fn。
// 放在树的最前面即可"每帧先更新状态、再由后面的控件拉取"——取代窗口级的每帧回调。
template<FixedString Id>
struct Update {
	static constexpr auto id = Id;

	std::function<void()> fn;

	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{Policy::Fixed, Policy::Fixed}; }
	void draw(const Rect&) { if (fn) fn(); }
};

// 弹性空白：HBox 里推挤后续控件到右端；width>0 时固定宽
template<FixedString Id>
struct Spacer {
	static constexpr auto id = Id;
	float width = 0.0f;

	Size hint(const Size& avail) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		return Size{width > 0.0f ? width : avail.w, 0.0f};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) { ImGui::Dummy(ImVec2(r.w > 0.0f ? r.w : 0.0f, r.h > 0.0f ? r.h : 0.0f)); }
};

// 小按钮：与 ImGui::SmallButton 等价，带点击回调
template<FixedString Id>
struct SmallButton {
	static constexpr auto id = Id;
	std::string label;
	std::function<void()> onClick;

	Size hint(const Size&) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		return Size{ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f,
		            ImGui::GetFrameHeight()};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {
		if (ImGui::SmallButton(label.c_str()) && onClick) onClick();
	}
};

// 水平分隔线（对应 ImGui::Separator）：高度 = 行距
template<FixedString Id>
struct Rule {
	static constexpr auto id = Id;

	Size hint(const Size& avail) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		return Size{avail.w, ImGui::GetStyle().ItemSpacing.y};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) { ImGui::Separator(); }
};

} // namespace Mk::wg2
