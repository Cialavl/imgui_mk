#pragma once
#include <array>
#include <cstdio>
#include <string>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 输入类原子控件：直接 template struct，绑定指针与旧 wg 相同。

namespace Mk::wg2 {

template<FixedString Id>
struct CheckBox {
	static constexpr auto id = Id;

	std::string name;
	bool* checked = nullptr;
	// 勾选状态变化时回调（事件式，不必每帧轮询）
	std::function<void(bool)> onChange;

	Size hint(const Size&) const {
		const float w = ImGui::CalcTextSize(name.c_str()).x
			+ ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
		return Size{w, detail::frameH()};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (!checked) return;
		if (ImGui::Checkbox(name.c_str(), checked) && onChange) onChange(*checked);
	}
};

template<FixedString Id, std::size_t N = 64>
struct InputText {
	static constexpr auto id = Id;

	std::string name;
	std::array<char, N> buf{};

	void setName(const char* t) { std::snprintf(buf.data(), N, "%s", t); }
	void setName(const std::string& t) { setName(t.c_str()); }
	const char* c_str() const { return buf.data(); }

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 220.0f, detail::frameH()};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputText(name.c_str(), buf.data(), N);
	}
};

// 多行文本输入（IME 候选框由窗口层 Platform_SetImeDataFn 定位）
template<FixedString Id, std::size_t N = 4096>
struct InputTextMultiline {
	static constexpr auto id = Id;

	std::string name;
	std::array<char, N> buf{};
	int lines = 5;           // <=0：用 height
	float height = 0.0f;     // >0 优先
	bool allowTab = false;

	void setText(const std::string& t) { std::snprintf(buf.data(), N, "%s", t.c_str()); }
	const char* c_str() const { return buf.data(); }

	Size hint(const Size& avail) const {
		const float h = height > 0.0f ? height
			: (lines > 0 ? static_cast<float>(lines) * ImGui::GetTextLineHeightWithSpacing()
				: ImGui::GetTextLineHeightWithSpacing() * 5.0f);
		return Size{avail.w > 0.0f ? avail.w : 320.0f, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		ImGuiInputTextFlags flags = allowTab ? ImGuiInputTextFlags_AllowTabInput : 0;
		ImGui::InputTextMultiline(name.c_str(), buf.data(), N, ImVec2(r.w, r.h), flags);
	}
};

template<FixedString Id>
struct InputInt {
	static constexpr auto id = Id;
	std::string name;
	int* value = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputInt(name.c_str(), value);
	}
};

template<FixedString Id>
struct InputFloat {
	static constexpr auto id = Id;
	std::string name;
	float* value = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputFloat(name.c_str(), value);
	}
};

template<FixedString Id>
struct InputDouble {
	static constexpr auto id = Id;
	std::string name;
	double* value = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputDouble(name.c_str(), value);
	}
};

template<FixedString Id>
struct InputInt2 {
	static constexpr auto id = Id;
	std::string name;
	std::array<int, 2>* value = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputInt2(name.c_str(), value->data());
	}
};

template<FixedString Id>
struct InputFloat2 {
	static constexpr auto id = Id;
	std::string name;
	std::array<float, 2>* value = nullptr;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::InputFloat2(name.c_str(), value->data());
	}
};

template<FixedString Id>
struct SliderF {
	static constexpr auto id = Id;
	std::string name;
	float* value = nullptr;
	float mn = 0.0f, mx = 1.0f;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 200.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::SliderFloat(name.c_str(), value, mn, mx);
	}
};

template<FixedString Id>
struct SliderInt {
	static constexpr auto id = Id;
	std::string name;
	int* value = nullptr;
	int mn = 0, mx = 100;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::SliderInt(name.c_str(), value, mn, mx);
	}
};

template<FixedString Id>
struct SliderAngle {
	static constexpr auto id = Id;
	std::string name;
	float* valueRad = nullptr;
	float degMin = -360.0f, degMax = 360.0f;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!valueRad) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::SliderAngle(name.c_str(), valueRad, degMin, degMax);
	}
};

template<FixedString Id>
struct DragF {
	static constexpr auto id = Id;
	std::string name;
	float* value = nullptr;
	float speed = 0.01f, mn = 0.0f, mx = 10.0f;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::DragFloat(name.c_str(), value, speed, mn, mx);
	}
};

template<FixedString Id>
struct DragI {
	static constexpr auto id = Id;
	std::string name;
	int* value = nullptr;
	float speed = 1.0f;
	int mn = 0, mx = 1000;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 160.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!value) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::DragInt(name.c_str(), value, speed, mn, mx);
	}
};

template<FixedString Id>
struct Combo {
	static constexpr auto id = Id;
	std::string name;
	int* current = nullptr;
	const char* const* items = nullptr;
	int count = 0;

	Size hint(const Size& avail) const { return Size{avail.w > 0.0f ? avail.w : 220.0f, detail::frameH()}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!current || !items) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::Combo(name.c_str(), current, items, count);
	}
};

template<FixedString Id>
struct RadioButton {
	static constexpr auto id = Id;
	std::string name;
	bool* active = nullptr;

	Size hint(const Size&) const {
		const float w = ImGui::CalcTextSize(name.c_str()).x
			+ ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
		return Size{w, detail::frameH()};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {
		if (active) ImGui::RadioButton(name.c_str(), active);
	}
};

template<FixedString Id>
struct RadioGroup {
	static constexpr auto id = Id;
	const char* const* items = nullptr;
	int count = 0;
	int* current = nullptr;

	Size hint(const Size&) const {
		const float h = static_cast<float>(count) * detail::frameH()
			+ (count > 1 ? static_cast<float>(count - 1) * ImGui::GetStyle().ItemSpacing.y : 0.0f);
		return Size{160.0f, h};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {
		if (!items || !current) return;
		for (int i = 0; i < count; ++i) {
			bool sel = (i == *current);
			if (ImGui::RadioButton(items[i], &sel)) *current = i;
		}
	}
};

template<FixedString Id>
struct ListBox {
	static constexpr auto id = Id;
	std::string name;
	const char* const* items = nullptr;
	int count = 0;
	int* current = nullptr;

	Size hint(const Size&) const {
		const float h = static_cast<float>(count) * detail::frameH()
			+ (count > 1 ? static_cast<float>(count - 1) * ImGui::GetStyle().ItemSpacing.y : 0.0f);
		return Size{160.0f, h};
	}
	Policy policy() const { return Policy{}; }
	void draw(const Rect& r) {
		if (!items || !current) return;
		ImGui::SetNextItemWidth(detail::frameWidthFor(r.w, name.c_str()));
		ImGui::ListBox(name.c_str(), current, items, count);
	}
};

} // namespace Mk::wg2
