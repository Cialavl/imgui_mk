#pragma once
#include <imgui.h>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>

// Mk::wg2 —— 纯编译期静态控件体系（Node 能力按需 opt-in 到 dynamic.h）。
//
// 本文件只放与 ImGui 无关的基础类型：几何、尺寸策略、编译期字符串 ID、
// 控件状态登记表。渲染相关包装见 widget.h。

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

// —— 控件样式覆盖：只设置需要的字段，绘制时由框架压栈/弹栈 ——
//   所有控件通用（含组合控件）：颜色、圆角、边框、内边距、间距、字体。
struct WidgetStyle {
	std::optional<ImVec4> text;         // 文字色
	std::optional<ImVec4> background;   // 控件底色（按钮/输入框/表头/页签）
	std::optional<ImVec4> hovered;      // 悬停底色
	std::optional<ImVec4> active;       // 按下底色
	std::optional<ImVec4> border;       // 边框色
	std::optional<float> rounding;      // 圆角
	std::optional<float> borderSize;    // 边框粗细
	std::optional<ImVec2> padding;      // FramePadding
	std::optional<ImVec2> spacing;      // ItemSpacing
	std::optional<ImFont*> font;        // 字体（FontScope）

	bool empty() const {
		return !(text || background || hovered || active || border
			|| rounding || borderSize || padding || spacing || font);
	}
};

// —— 控件状态：由引擎统一持有 ——
// 控件只负责 hint/policy/draw；可见性、可用性、布局权重、尺寸覆盖、样式作用域
// 都在引擎里按控件地址登记，布局与绘制统一经 widgetChrome() 取值。
struct Chrome {
	bool visible = true;
	// 拉取式可见性：设置后每帧求值，优先于 visible（条件显隐）
	std::function<bool()> visibleFn;
	bool shown() const { return visibleFn ? visibleFn() : visible; }
	bool enabled = true;
	float stretch = 0.0f;      // 布局权重（Qt addWidget(w, stretch) 语义）
	SizeOverrides size;        // setFixedSize / setMinSize / setPrefSize
	WidgetStyle style;         // 每控件样式覆盖（drawWidget 压栈应用）
	const void* parent = nullptr;   // 父节点（挂载它的窗口/组件）
	bool focusRequest = false;      // requestFocus 置位；drawWidget 消费（下个 item 取焦点）
	bool focused = false;           // 最近一帧是否拥有键盘焦点（hasFocus 查询）
};

namespace detail {

// 控件身份 = 类型 + 地址：容器与其首个子控件地址相同（标准布局首成员规则），必须靠类型区分
struct ChromeKey {
	const void* addr;
	const std::type_info* type;
	bool operator==(const ChromeKey&) const = default;
};
struct ChromeKeyHash {
	std::size_t operator()(const ChromeKey& k) const noexcept {
		return std::hash<const void*>{}(k.addr) ^ (std::hash<const std::type_info*>{}(k.type) << 1);
	}
};

// 控件状态登记表：按（类型, 地址）索引——静态树对象地址稳定，
// 容器与其首个子控件地址相同，因此必须靠类型区分。控件应是长生命周期对象；
// 重建界面树/测试夹具用 clearChrome() 复位。
inline std::unordered_map<ChromeKey, Chrome, ChromeKeyHash>& chromeStore() {
	static std::unordered_map<ChromeKey, Chrome, ChromeKeyHash> store;
	return store;
}
inline Chrome& chromeOf(const void* addr, const std::type_info* type) {
	return chromeStore()[ChromeKey{addr, type}];
}

// 对象搬移：状态跟着走，旧地址不留残余
inline void moveChrome(const void* fromAddr, const std::type_info* fromType,
	const void* toAddr, const std::type_info* toType) {
	auto& store = chromeStore();
	auto it = store.find(ChromeKey{fromAddr, fromType});
	if (it == store.end()) return;
	store[ChromeKey{toAddr, toType}] = std::move(it->second);
	store.erase(it);
}
} // namespace detail

// 控件状态入口：所有控件统一问引擎要
template<class W>
inline Chrome& widgetChrome(W&& w) {
	return detail::chromeOf(static_cast<const void*>(&w), &typeid(std::remove_cvref_t<W>));
}

// 清空登记表：重建界面树/测试夹具时显式重置
inline void clearChrome() { detail::chromeStore().clear(); }

// 定向回收单个控件的登记：控件销毁时用
template<class W>
inline void clearChromeFor(W& w) {
	detail::chromeStore().erase(detail::ChromeKey{static_cast<const void*>(&w), &typeid(std::remove_cvref_t<W>)});
}

// 登记条目数（诊断/测试用）
inline std::size_t chromeCount() { return detail::chromeStore().size(); }

// 父节点（Qt 式控件树）：挂载时登记
template<class W>
inline void setParent(W& w, const void* parent) { widgetChrome(w).parent = parent; }

template<class W>
inline const void* parentOf(W& w) { return widgetChrome(w).parent; }

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

// —— 矩形坐标约定 ——
// 容器传给子控件的 Rect 一律是**屏幕坐标**；控件自己开子窗口时，需换算成窗口局部坐标再定位。
inline ImVec2 toWindowLocal(const ImVec2& screenPos) {
	const ImVec2 origin = ImGui::GetWindowPos();
	return ImVec2(screenPos.x - origin.x, screenPos.y - origin.y);
}
inline void placeCursorWindow(const Rect& r) { ImGui::SetCursorPos(toWindowLocal(ImVec2(r.x, r.y))); }
inline void placeCursorScreen(const Rect& r) { ImGui::SetCursorScreenPos(ImVec2(r.x, r.y)); }
// 控件内子区域（BeginChild）的当前屏幕矩形：x/y 取光标屏幕位置，宽高取可用区
inline Rect contentRect (void) {
	const ImVec2 sp = ImGui::GetCursorScreenPos();
	const ImVec2 av = ImGui::GetContentRegionAvail();
	return Rect{sp.x, sp.y, av.x > 0.0f ? av.x : 0.0f, av.y > 0.0f ? av.y : 0.0f};
}

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

// ============ 鼠标指针形状 ============
enum class Cursor { Arrow, TextInput, Hand, ResizeEW, ResizeNS, ResizeAll, NotAllowed };

constexpr ImGuiMouseCursor toImGuiCursor(Cursor c) {
	switch (c) {
	case Cursor::TextInput: return ImGuiMouseCursor_TextInput;
	case Cursor::Hand: return ImGuiMouseCursor_Hand;
	case Cursor::ResizeEW: return ImGuiMouseCursor_ResizeEW;
	case Cursor::ResizeNS: return ImGuiMouseCursor_ResizeNS;
	case Cursor::ResizeAll: return ImGuiMouseCursor_ResizeAll;
	case Cursor::NotAllowed: return ImGuiMouseCursor_NotAllowed;
	case Cursor::Arrow:
	default: return ImGuiMouseCursor_Arrow;
	}
}

inline void setCursor(Cursor c) { ImGui::SetMouseCursor(toImGuiCursor(c)); }
inline void cursorOnHover(Cursor c) { if (ImGui::IsItemHovered()) setCursor(c); }

} // namespace Mk::wg2
