#pragma once
#include <imgui.h>

#include <cstdlib>
#include <functional>
#include <utility>
#include <vector>

// Mk::wg2 字体（引擎托管，带默认值）：
//   defaultFont() —— 当前上下文的默认界面字体（io.FontDefault；为空 = ImGui 图集默认字体）
//   monoFont()    —— 等宽字体：provider 给出句柄就用它，否则回退 defaultFont()，调用方不必判空
//   setMonoFontProvider() —— 应用启动时注册一次（窗口烘焙完字体后按需返回句柄）
// FontScope(nullptr) 是空操作，所以“回退到空”同样安全。

namespace Mk::wg2 {

namespace detail {
inline std::function<ImFont*()>& monoProvider() {
	static std::function<ImFont*()> fn;
	return fn;
}
} // namespace detail

// 默认界面字体：当前上下文（每个窗口一份图集）
inline ImFont* defaultFont() {
	if (!ImGui::GetCurrentContext()) return nullptr;
	return ImGui::GetIO().FontDefault;
}

// 注册等宽字体提供者（全局一份：应用只有一套代码字体）
inline void setMonoFontProvider(std::function<ImFont*()> fn) {
	detail::monoProvider() = std::move(fn);
}

// 等宽字体：provider 未注册或还没烘焙出句柄时回退默认界面字体
inline ImFont* monoFont() {
	auto& fn = detail::monoProvider();
	if (fn) {
		if (ImFont* f = fn()) return f;
	}
	return defaultFont();
}

// 清空 provider（重建应用 / 测试夹具）
inline void clearFonts() { detail::monoProvider() = nullptr; }

namespace detail {
inline std::vector<ImWchar>& extraGlyphPairs() {
	static std::vector<ImWchar> v;
	return v;
}
} // namespace detail

// 应用登记用字：把界面实际用到的码点（成对 [first,last]，0 结尾）加进图集。
// 在窗口创建 / 换字号前调用一次即可；字形 = 常用汉字 + 这里登记的字符。
inline void setExtraGlyphs(const ImWchar* ranges) {
	auto& v = detail::extraGlyphPairs();
	v.clear();
	if (!ranges) return;
	for (const ImWchar* p = ranges; *p; ) {
		v.push_back(*p++);
		v.push_back(*p++);
	}
}

// 字体图集字形范围：默认「常用汉字 + 应用用字」；MKUI_FONT_FULL_CJK=1 用全量 CJK（重建慢）
inline const ImWchar* fontGlyphRanges() {
	static const bool full = [] {
		const char* v = std::getenv("MKUI_FONT_FULL_CJK");
		return v && *v && *v != '0';
	}();
	if (full) return ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();

	static std::vector<ImWchar> combined;
	const ImWchar* base = ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon();
	combined.clear();
	for (const ImWchar* p = base; *p; ) {
		combined.push_back(*p++);
		combined.push_back(*p++);
	}
	for (const ImWchar c : detail::extraGlyphPairs()) combined.push_back(c);
	combined.push_back(0);
	return combined.data();
}

} // namespace Mk::wg2
