#pragma once
#include <array>
#include <string>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 颜色控件与位标志。

namespace Mk::wg2 {

template<FixedString Id>
struct ColorEdit3 {
	static constexpr auto id = Id;
	std::string name;
	std::array<float, 3>* color = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 260.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!color) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::ColorEdit3(name.c_str(), color->data());
	}
};

template<FixedString Id>
struct ColorEdit4 {
	static constexpr auto id = Id;
	std::string name;
	std::array<float, 4>* color = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 260.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!color) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::ColorEdit4(name.c_str(), color->data());
	}
};

template<FixedString Id>
struct ColorPicker3 {
	static constexpr auto id = Id;
	std::string name;
	std::array<float, 3>* color = nullptr;

	Size hint(const Size&) const { return Size{260.0f, 320.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!color) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::ColorPicker3(name.c_str(), color->data());
	}
};

template<FixedString Id>
struct ColorPicker4 {
	static constexpr auto id = Id;
	std::string name;
	std::array<float, 4>* color = nullptr;

	Size hint(const Size&) const { return Size{260.0f, 320.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!color) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::ColorPicker4(name.c_str(), color->data());
	}
};

template<FixedString Id>
struct CheckboxFlags {
	static constexpr auto id = Id;
	std::string name;
	int* flags = nullptr;
	int mask = 1;

	Size hint(const Size&) const {
		const float w = ImGui::CalcTextSize(name.c_str()).x
			+ ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
		return Size{w, detail::frameH()};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {
		if (flags) ImGui::CheckboxFlags(name.c_str(), flags, mask);
	}
};

} // namespace Mk::wg2
