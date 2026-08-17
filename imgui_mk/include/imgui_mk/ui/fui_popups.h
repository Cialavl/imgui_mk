#pragma once
#include <imgui_mk/ui/fui_core.h>
#include <array>

// 函数式控件库·弹窗与高级控件模块：Popup/PopupModal（标志触发式）、
// 颜色编辑/拾取器、位标志复选框。

namespace Mk::fui {

	// 弹窗：open 置位当帧打开（一次性触发器，打开后自动回落）；
	// 弹窗内容每帧渲染，直到子视图回调里调 ImGui::CloseCurrentPopup()
	template<typename... Ts>
	struct Popup {
		std::string id;
		bool& open;
		std::tuple<Ts...> children;
	};

	// 模态弹窗：同 Popup，但阻塞宿主窗口交互
	//（注意：模态打开会强制关闭已打开的普通弹窗——ImGui 模态独占语义）
	template<typename... Ts>
	struct PopupModal {
		std::string id;
		bool& open;
		ImGuiWindowFlags flags;
		std::tuple<Ts...> children;
	};

	// 颜色编辑/拾取器：绑定 std::array<float,3|4>&（RGB/RGBA）
	struct ColorEdit3 { std::string label; std::array<float, 3>& color; };
	inline void draw(const ColorEdit3& w) { ImGui::ColorEdit3(w.label.c_str(), w.color.data()); }

	struct ColorEdit4 { std::string label; std::array<float, 4>& color; };
	inline void draw(const ColorEdit4& w) { ImGui::ColorEdit4(w.label.c_str(), w.color.data()); }

	struct ColorPicker3 { std::string label; std::array<float, 3>& color; };
	inline void draw(const ColorPicker3& w) { ImGui::ColorPicker3(w.label.c_str(), w.color.data()); }

	struct ColorPicker4 { std::string label; std::array<float, 4>& color; };
	inline void draw(const ColorPicker4& w) { ImGui::ColorPicker4(w.label.c_str(), w.color.data()); }

	// 位标志复选框：绑定 int&，按 mask 切换一个位
	struct CheckboxFlags { std::string label; int& flags; int mask; };
	inline void draw(const CheckboxFlags& w) { ImGui::CheckboxFlags(w.label.c_str(), &w.flags, w.mask); }

	// draw 重载：先声明、再定义（见 fui_core.h 的说明）
	template<typename... Ts> void draw(const Popup<Ts...>& w);
	template<typename... Ts> void draw(const PopupModal<Ts...>& w);

	template<typename... Ts>
	void draw(const Popup<Ts...>& w) {
		if(w.open) {
			ImGui::OpenPopup(w.id.c_str());
			w.open = false;
		}
		if(ImGui::BeginPopup(w.id.c_str())) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::EndPopup();
		}
	}

	template<typename... Ts>
	void draw(const PopupModal<Ts...>& w) {
		if(w.open) {
			ImGui::OpenPopup(w.id.c_str());
			w.open = false;
		}
		if(ImGui::BeginPopupModal(w.id.c_str(), nullptr, w.flags)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::EndPopup();
		}
	}

	// —— 工厂 ——

	template<typename... Ts>
	auto popup(std::string id, bool& open, Ts&&... views) {
		return Popup<std::decay_t<Ts>...>{
			std::move(id), open, std::make_tuple(std::forward<Ts>(views)...)};
	}
	template<typename... Ts>
	auto popupModal(std::string id, bool& open, ImGuiWindowFlags flags, Ts&&... views) {
		return PopupModal<std::decay_t<Ts>...>{
			std::move(id), open, flags, std::make_tuple(std::forward<Ts>(views)...)};
	}
	inline auto colorEdit3(std::string lbl, std::array<float, 3>& c) {
		return ColorEdit3{std::move(lbl), c};
	}
	inline auto colorEdit4(std::string lbl, std::array<float, 4>& c) {
		return ColorEdit4{std::move(lbl), c};
	}
	inline auto colorPicker3(std::string lbl, std::array<float, 3>& c) {
		return ColorPicker3{std::move(lbl), c};
	}
	inline auto colorPicker4(std::string lbl, std::array<float, 4>& c) {
		return ColorPicker4{std::move(lbl), c};
	}
	inline auto checkboxFlags(std::string lbl, int& flags, int mask) {
		return CheckboxFlags{std::move(lbl), flags, mask};
	}

}