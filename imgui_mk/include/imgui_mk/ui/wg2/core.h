#pragma once
#include <imgui.h>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <utility>

// Mk::wg2 —— 纯编译期静态控件体系（Node 能力按需 opt-in 到 dynamic.h）。
//
// 本文件只放与 ImGui 无关的基础类型：几何、尺寸策略、编译期字符串 ID、
// 控件公共状态。渲染相关包装见 widget.h。

namespace Mk::wg2 {

// ============ 基础布局类型 ============

struct Size {
	float w = 0.0f, h = 0.0f;
};

struct Rect {
	float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
	Rect() = default;
	Rect(float xx, float yy, float ww, float hh) : x(xx), y(yy), w(ww), h(hh) {}
};

// ============ Qt 式尺寸协议 ============

struct Policy {
	enum Kind : unsigned char { Fixed, Minimum, Maximum, Preferred, Expanding, Ignored };
	Kind h = Preferred, v = Preferred;
	constexpr bool canGrow(Kind k) const { return k == Minimum || k == Preferred || k == Expanding; }
	constexpr bool canShrink(Kind k) const { return k == Maximum || k == Preferred || k == Expanding || k == Ignored; }
	constexpr bool wantsExpand(Kind k) const { return k == Expanding; }
};

// 显式尺寸覆盖（优先于控件 hint/policy）：负值 = 未设置
struct SizeOverrides {
	float minW = -1.0f, minH = -1.0f;
	float maxW = -1.0f, maxH = -1.0f;
	float prefW = -1.0f, prefH = -1.0f;
	float fixedW = -1.0f, fixedH = -1.0f;

	// em 版：> 0 时按当前字号换算（字体缩放后宽度自动跟随，不需要每帧同步数值）
	float minWEm = -1.0f, minHEm = -1.0f;
	float maxWEm = -1.0f, maxHEm = -1.0f;
	float prefWEm = -1.0f, prefHEm = -1.0f;
	float fixedWEm = -1.0f, fixedHEm = -1.0f;
};

// ============ 编译期字符串 ID ============
// 用法：
//   using namespace Mk::wg2;
//   Button<Id("save")> b;
//   panel.at<Id("toolbar"), Id("save")>();
//
// 支持 UDL 的编译器也可写成 "save"_id；MSVC 请使用 Id("save")。

template<std::size_t N>
struct FixedString {
	char data[N]{};   // 含结尾 '\0'

	constexpr FixedString() = default;
	constexpr FixedString(const char (&s)[N]) {
		for (std::size_t i = 0; i < N; ++i) data[i] = s[i];
	}

	static constexpr std::size_t size = N == 0 ? 0 : N - 1;

	constexpr bool operator==(const FixedString&) const = default;

	template<std::size_t M>
	constexpr bool operator==(const FixedString<M>& other) const {
		if constexpr (N != M) return false;
		else {
			for (std::size_t i = 0; i < N; ++i) {
				if (data[i] != other.data[i]) return false;
			}
			return true;
		}
	}

	constexpr char operator[](std::size_t i) const { return data[i]; }
};

template<std::size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

template<char... Cs>
consteval auto operator""_id() {
	constexpr char arr[] = {Cs..., '\0'};
	return FixedString<sizeof...(Cs) + 1>{arr};
}

// MSVC 目前不支持字符串字面量模板 UDL（"x"_id），因此提供等价函数写法：
//   Button<Id("save")>
template<std::size_t N>
constexpr FixedString<N> Id(const char (&s)[N]) {
	return FixedString<N>{s};
}

inline constexpr FixedString<1> kNoId{""};

// 不同长度 FixedString 之间的编译期比较（模板实参比较用）
template<FixedString A, FixedString B>
consteval bool fixed_equal() {
	if constexpr (A.size != B.size) return false;
	else {
		for (std::size_t i = 0; i < A.size; ++i) {
			if (A.data[i] != B.data[i]) return false;
		}
		return true;
	}
}

// ============ 控件公共状态（组合成员，非基类） ============

struct DefaultStyle {
	static void push() {}
	static void pop() {}
};

template<class Style = DefaultStyle>
struct WidgetState {
	bool visible = true;
	// 拉取式可见性：设置后每帧求值，优先于 visible（控件自带的条件显隐）
	std::function<bool()> visibleFn;
	bool shown() const { return visibleFn ? visibleFn() : visible; }
	bool enabled = true;
	float stretch = 0.0f;      // 布局权重（Qt addWidget(w, stretch) 语义）
	SizeOverrides size;        // setFixedSize / setMinSize / setPrefSize
	[[no_unique_address]] Style style;
};

// 布局条目（静态求解器内部使用）
struct LayoutItem {
	float hint = 0.0f;
	float mn = 0.0f;
	float mx = 1.0e9f;
	bool canGrow = false;
	bool canShrink = false;
	bool wantsExpand = false;
	float stretch = 0.0f;
	float cross = 0.0f;
};

// ============ 渲染深度：动态容器渲染期增删保护 ============

inline thread_local int g_RenderDepth = 0;

// 度量入口：把"量字/取尺寸"的 ImGui 调用收成一处，控件与应用都不直接碰 ImGui 度量
namespace metrics {
inline float fontSize() { return ImGui::GetFontSize(); }
inline float lineHeight() { return ImGui::GetTextLineHeightWithSpacing(); }
inline float frameHeight() { return ImGui::GetFrameHeight(); }
inline ImVec2 itemSpacing() { return ImGui::GetStyle().ItemSpacing; }
inline ImVec2 windowPadding() { return ImGui::GetStyle().WindowPadding; }
inline ImVec2 textSize(const char* t, float wrapWidth = -1.0f) {
	return ImGui::CalcTextSize(t, nullptr, true, wrapWidth);
}
inline unsigned int frameCount() { return ImGui::GetFrameCount(); }
inline ImVec2 cursorPos() { return ImGui::GetCursorPos(); }
inline ImVec2 availRegion() { return ImGui::GetContentRegionAvail(); }

} // namespace metrics

// 字体作用域：构造 PushFont、析构 PopFont（命令式绘制区切字体的唯一入口）
struct FontScope {
	bool pushed = false;
	explicit FontScope(ImFont* f) { if (f) { ImGui::PushFont(f); pushed = true; } }
	~FontScope() { if (pushed) ImGui::PopFont(); }
	FontScope(const FontScope&) = delete;
	FontScope& operator=(const FontScope&) = delete;
};

// 剪贴板
inline void setClipboard(const std::string& text) { ImGui::SetClipboardText(text.c_str()); }

} // namespace Mk::wg2
