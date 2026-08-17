#pragma once
#include <imgui_mk/ui/fui_core.h>
#include <array>
#include <span>

// 函数式控件库·输入模块：文本/数值输入、拖拽、滑块、单选组、下拉与列表选择。
// 全部为数据绑定风格：持外部变量引用，每帧读引用绘制、交互时 ImGui 直接写回。
// items 类参数（RadioGroup/Combo/ListBox）由调用方持有，存活期覆盖引擎 run()。

namespace Mk::fui {

	// 单行文本输入：绑定外部定长缓冲（std::array<char,N>，容量编译期可知、类型安全）
	template<std::size_t N>
	struct InputText {
		std::string label;
		std::array<char, N>& buf;
	};

	template<std::size_t N>
	inline void draw(const InputText<N>& w) {
		ImGui::InputText(w.label.c_str(), w.buf.data(), N);
	}

	struct InputInt { std::string label; int& value; };
	inline void draw(const InputInt& w) { ImGui::InputInt(w.label.c_str(), &w.value); }

	struct InputFloat { std::string label; float& value; };
	inline void draw(const InputFloat& w) { ImGui::InputFloat(w.label.c_str(), &w.value); }

	struct InputDouble { std::string label; double& value; };
	inline void draw(const InputDouble& w) { ImGui::InputDouble(w.label.c_str(), &w.value); }

	// 二分量输入（vec2 场景）：绑定 std::array<T,2>&
	struct InputInt2 { std::string label; std::array<int, 2>& value; };
	inline void draw(const InputInt2& w) { ImGui::InputInt2(w.label.c_str(), w.value.data()); }

	struct InputFloat2 { std::string label; std::array<float, 2>& value; };
	inline void draw(const InputFloat2& w) { ImGui::InputFloat2(w.label.c_str(), w.value.data()); }

	// 拖拽数值输入：按 speed 拖动改变（mn/mx 为范围）
	struct DragF { std::string label; float& value; float speed, mn, mx; };
	inline void draw(const DragF& w) { ImGui::DragFloat(w.label.c_str(), &w.value, w.speed, w.mn, w.mx); }

	struct DragI { std::string label; int& value; float speed; int mn, mx; };
	inline void draw(const DragI& w) { ImGui::DragInt(w.label.c_str(), &w.value, w.speed, w.mn, w.mx); }

	struct SliderInt { std::string label; int& value; int mn, mx; };
	inline void draw(const SliderInt& w) { ImGui::SliderInt(w.label.c_str(), &w.value, w.mn, w.mx); }

	// 角度滑块：绑定弧度值，按角度显示/交互
	struct SliderAngle { std::string label; float& valueRad; float degMin, degMax; };
	inline void draw(const SliderAngle& w) { ImGui::SliderAngle(w.label.c_str(), &w.valueRad, w.degMin, w.degMax); }

	struct RadioButton { std::string label; bool& active; };
	inline void draw(const RadioButton& w) { ImGui::RadioButton(w.label.c_str(), &w.active); }

	// 单选组：items 由调用方持有（存活期覆盖 run()），current 为选中下标
	struct RadioGroup { std::span<const char* const> items; int& current; };
	inline void draw(const RadioGroup& w) {
		for(int i = 0; i < (int)w.items.size(); ++i) {
			bool sel = (i == w.current);
			if(ImGui::RadioButton(w.items[i], &sel)) w.current = i;
		}
	}

	// 下拉选择 / 列表选择：current 为选中下标，items 由调用方持有
	struct Combo { std::string label; int& current; std::span<const char* const> items; };
	inline void draw(const Combo& w) {
		ImGui::Combo(w.label.c_str(), &w.current, w.items.data(), (int)w.items.size());
	}

	struct ListBox { std::string label; int& current; std::span<const char* const> items; };
	inline void draw(const ListBox& w) {
		ImGui::ListBox(w.label.c_str(), &w.current, w.items.data(), (int)w.items.size());
	}

	// —— 工厂 ——
	template<std::size_t N>
	auto inputText(std::string lbl, std::array<char, N>& buf) {
		return InputText<N>{std::move(lbl), buf};
	}
	inline auto inputInt(std::string lbl, int& v) { return InputInt{std::move(lbl), v}; }
	inline auto inputFloat(std::string lbl, float& v) { return InputFloat{std::move(lbl), v}; }
	inline auto inputDouble(std::string lbl, double& v) { return InputDouble{std::move(lbl), v}; }
	inline auto inputInt2(std::string lbl, std::array<int, 2>& v) { return InputInt2{std::move(lbl), v}; }
	inline auto inputFloat2(std::string lbl, std::array<float, 2>& v) { return InputFloat2{std::move(lbl), v}; }
	inline auto dragF(std::string lbl, float& v, float speed, float mn, float mx) {
		return DragF{std::move(lbl), v, speed, mn, mx};
	}
	inline auto dragI(std::string lbl, int& v, float speed, int mn, int mx) {
		return DragI{std::move(lbl), v, speed, mn, mx};
	}
	inline auto sliderInt(std::string lbl, int& v, int mn, int mx) {
		return SliderInt{std::move(lbl), v, mn, mx};
	}
	inline auto sliderAngle(std::string lbl, float& vRad, float degMin = -360.0f, float degMax = 360.0f) {
		return SliderAngle{std::move(lbl), vRad, degMin, degMax};
	}
	inline auto radioButton(std::string lbl, bool& active) { return RadioButton{std::move(lbl), active}; }
	inline auto radioGroup(std::span<const char* const> items, int& current) {
		return RadioGroup{items, current};
	}
	inline auto combo(std::string lbl, int& current, std::span<const char* const> items) {
		return Combo{std::move(lbl), current, items};
	}
	inline auto listBox(std::string lbl, int& current, std::span<const char* const> items) {
		return ListBox{std::move(lbl), current, items};
	}

}