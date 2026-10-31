#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <tuple>
#include <vector>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/static_layout.h>

// Mk::wg2 复合控件补充：Tree / Menu / Popup / Table。

namespace Mk::wg2 {

// ============ Tree（可折叠节点） ============

template<FixedString Id, class... Children>
struct Tree {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	std::string title;
	bool defaultOpen = false;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float h = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		float w = ImGui::CalcTextSize(title.c_str()).x;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				h += s.h;
				w = std::max(w, s.w);
			};
			(one(c), ...);
		}, children);
		return Size{std::max(avail.w, w), h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		if (!ImGui::TreeNodeEx(title.empty() ? id.data : title.c_str(), flags)) return;

		float y = r.y + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(Size{r.w, r.h - (y - r.y)});
				drawWidget(child, Rect{r.x, y, s.w, s.h});
				y += s.h;
			};
			(one(c), ...);
		}, children);
		ImGui::TreePop();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

// ============ MenuItem / Menu ============

template<FixedString Id>
struct MenuItem {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::string name;
	std::string shortcut;
	bool selected = false;
	std::function<bool()> selectedFn;   // 拉取式勾选态（如"当前主题"菜单项）

	bool selectedNow() const { return selectedFn ? selectedFn() : selected; }
	std::function<void()> onClick;

	template<class F>
	void setOnClick(F&& cb) { onClick = std::forward<F>(cb); }

	Size hint(const Size&) const {
		return Size{ImGui::CalcTextSize(name.c_str()).x + 40.0f, ImGui::GetFrameHeight()};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (ImGui::MenuItem(name.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), selectedNow()) && onClick) {
			onClick();
		}
	}
};

template<FixedString Id, class... Children>
struct Menu {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	std::string name;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				w = std::max(w, s.w);
				h += s.h;
			};
			(one(c), ...);
		}, children);
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (!ImGui::BeginMenu(name.c_str())) return;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible) drawFlow(child);   // 弹菜单内流式排布（含内边距）
			};
			(one(c), ...);
		}, children);
		ImGui::EndMenu();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

template<FixedString Id, class... Children>
struct MenuBar {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;

	Size hint(const Size&) const {
		return Size{0.0f, ImGui::GetFrameHeight() + 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (!ImGui::BeginMainMenuBar()) return;   // 浮顶独立菜单栏（宿主窗口无需 MenuBar 标志）
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible) drawFlow(child);   // 流式排布：菜单从左到右跟随光标
			};
			(one(c), ...);
		}, children);
		ImGui::EndMainMenuBar();
		// 主菜单栏是浮顶独立窗口：宿主光标下移，防重叠
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetFrameHeight());
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

// ============ Popup / PopupModal ============

template<FixedString Id, class... Children>
struct Popup {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	bool* open = nullptr;
	std::string idname;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				w = std::max(w, s.w);
				h += s.h;
			};
			(one(c), ...);
		}, children);
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const char* pid = idname.empty() ? id.data : idname.c_str();
		if (open && *open) {
			ImGui::OpenPopup(pid);
			*open = false;
		}
		if (!ImGui::BeginPopup(pid)) return;
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible) drawFlow(child);   // 弹窗内流式排布（含内边距）
			};
			(one(c), ...);
		}, children);
		ImGui::EndPopup();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

template<FixedString Id, class... Children>
struct PopupModal {
	static constexpr auto id = Id;

	WidgetState<> state;
	std::tuple<Children...> children;
	bool* open = nullptr;
	ImGuiWindowFlags flags = 0;
	std::string idname;

	Size hint(const Size& avail) const {
		if (!state.visible) return Size{0.0f, 0.0f};
		float w = 0.0f, h = 0.0f;
		std::apply([&](const auto&... c) {
			auto one = [&](const auto& child) {
				if (!child.state.visible) return;
				const Size s = child.hint(avail);
				w = std::max(w, s.w);
				h += s.h;
			};
			(one(c), ...);
		}, children);
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const char* pid = idname.empty() ? id.data : idname.c_str();
		if (open && *open) {
			ImGui::OpenPopup(pid);
			*open = false;
		}
		if (!ImGui::BeginPopupModal(pid, nullptr, flags)) { if (open) *open = false; return; }   // 关闭后同步外部开关，否则下一帧会重新打开
		std::apply([&](auto&... c) {
			auto one = [&](auto& child) {
				if (child.state.visible) drawFlow(child);   // 模态弹窗内流式排布（含内边距）
			};
			(one(c), ...);
		}, children);
		ImGui::EndPopup();
	}

	template<std::size_t I> constexpr decltype(auto) child() & { return std::get<I>(children); }
	template<std::size_t I> constexpr decltype(auto) child() const & { return std::get<I>(children); }

	template<FixedString Key>
	static consteval std::size_t childIndex() { return detail::childIndexById<Key, Children...>(); }

	template<FixedString First, FixedString... Rest>
	constexpr decltype(auto) at() & {
		constexpr std::size_t idx = childIndex<First>();
		auto& c = std::get<idx>(children);
		if constexpr (sizeof...(Rest) == 0) return c;
		else return c.template at<Rest...>();
	}
};

// ============ 表格 ============

template<FixedString Id>
struct TextTable {
	static constexpr auto id = Id;

	WidgetState<> state;
	const char* const* headers = nullptr;
	int headerCount = 0;
	const std::vector<std::vector<std::string>>* rows = nullptr;
	std::string idname;

	Size hint(const Size& avail) const {
		const int n = rows ? static_cast<int>(rows->size()) : 0;
		const float line = ImGui::GetTextLineHeightWithSpacing();
		return Size{avail.w, static_cast<float>(n + 1) * (line + 4.0f) + 8.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (!headers || !rows || headerCount <= 0) return;
		const char* tid = idname.empty() ? id.data : idname.c_str();
		if (!ImGui::BeginTable(tid, headerCount, ImGuiTableFlags_Borders)) return;
		for (int i = 0; i < headerCount; ++i) ImGui::TableSetupColumn(headers[i]);
		ImGui::TableHeadersRow();
		for (const auto& row : *rows) {
			ImGui::TableNextRow();
			for (int i = 0; i < headerCount; ++i) {
				ImGui::TableNextColumn();
				if (static_cast<int>(row.size()) > i) ImGui::TextUnformatted(row[i].c_str());
			}
		}
		ImGui::EndTable();
	}
};

template<FixedString Id>
struct FnTable {
	static constexpr auto id = Id;

	WidgetState<> state;
	const char* const* headers = nullptr;
	int headerCount = 0;
	int rowCount = 0;
	std::function<std::string(int, int)> cell;
	std::string idname;

	template<class F>
	void setCell(F&& fn) { cell = std::forward<F>(fn); }

	Size hint(const Size& avail) const {
		const float line = ImGui::GetTextLineHeightWithSpacing();
		return Size{avail.w, static_cast<float>(rowCount + 1) * (line + 4.0f) + 8.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (!headers || !cell || headerCount <= 0) return;
		const char* tid = idname.empty() ? id.data : idname.c_str();
		if (!ImGui::BeginTable(tid, headerCount, ImGuiTableFlags_Borders)) return;
		for (int i = 0; i < headerCount; ++i) ImGui::TableSetupColumn(headers[i]);
		ImGui::TableHeadersRow();
		for (int r = 0; r < rowCount; ++r) {
			ImGui::TableNextRow();
			for (int c = 0; c < headerCount; ++c) {
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(cell(r, c).c_str());
			}
		}
		ImGui::EndTable();
	}
};

} // namespace Mk::wg2
