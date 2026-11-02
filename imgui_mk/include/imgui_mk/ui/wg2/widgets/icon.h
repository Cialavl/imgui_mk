#pragma once
// Mk::wg2 图标：图标字体注册 + 码点转 UTF-8 + 图标控件
//   用法：setIconFont(io.Fonts->AddFontFromFileTTF("fa-solid-900.ttf", 16));
//         IconText<Id("icon")>{ .glyph = 0xF001 };
#include <string>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

namespace Mk::wg2 {

namespace detail {
inline ImFont*& iconFontSlot() {
	static ImFont* font = nullptr;
	return font;
}
} // namespace detail

inline void setIconFont(ImFont* font) { detail::iconFontSlot() = font; }
inline ImFont* iconFont() { return detail::iconFontSlot(); }

// Unicode 码点 → UTF-8（图标字体通常用私有区码点）
inline std::string iconGlyph(unsigned int cp) {
	std::string s;
	if (cp < 0x80u) {
		s += static_cast<char>(cp);
	}
	else if (cp < 0x800u) {
		s += static_cast<char>(0xC0u | (cp >> 6));
		s += static_cast<char>(0x80u | (cp & 0x3Fu));
	}
	else if (cp < 0x10000u) {
		s += static_cast<char>(0xE0u | (cp >> 12));
		s += static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
		s += static_cast<char>(0x80u | (cp & 0x3Fu));
	}
	else {
		s += static_cast<char>(0xF0u | (cp >> 18));
		s += static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu));
		s += static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
		s += static_cast<char>(0x80u | (cp & 0x3Fu));
	}
	return s;
}

template<FixedString Id>
struct IconText {
	static constexpr auto id = Id;

	unsigned int glyph = 0;
	ImU32 color = 0;   // 0 = 主题文字色

	Size hint(const Size&) const { return Size{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		FontScope scope(iconFont());
		if (color) ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextUnformatted(iconGlyph(glyph).c_str());
		if (color) ImGui::PopStyleColor();
	}
};

} // namespace Mk::wg2
