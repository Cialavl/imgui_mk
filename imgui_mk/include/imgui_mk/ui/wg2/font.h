#pragma once
#include <imgui.h>

#include <functional>
#include <utility>

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

} // namespace Mk::wg2
