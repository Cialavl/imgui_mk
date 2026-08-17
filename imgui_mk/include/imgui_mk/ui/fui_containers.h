#pragma once
#include <imgui_mk/ui/fui_core.h>
#include <functional>
#include <span>
#include <vector>

// 函数式控件库·结构容器模块：标签页（TabBar）、表格（数据驱动/泛型单元格）、
// 菜单栏（可嵌套子菜单）、可在宿主内拖动的浮动面板。

namespace Mk::fui {

	// —— 结构体定义 ——

	// 标签页项：tabBar 的子项（名称 + 任意子视图）
	template<typename T>
	struct Tab {
		std::string name;
		T child;
	};

	// 标签栏：仅激活页的子视图每帧渲染
	template<typename... Ts>
	struct TabBar {
		std::string id;
		ImGuiTabBarFlags flags;
		std::tuple<Ts...> children;
	};

	// 文本表格：headers 由调用方持有，rows 引用外部数据（逻辑侧可维护）
	struct TextTable {
		std::string id;
		std::span<const char* const> headers;
		const std::vector<std::vector<std::string>>& rows;
		ImGuiTableFlags flags;
	};

	// 泛型表格：cell(row, col) 返回单元格文本（函数式：数据来源任意）
	struct FnTable {
		std::string id;
		std::span<const char* const> headers;
		int rowCount;
		std::function<std::string(int, int)> cell;
		ImGuiTableFlags flags;
	};

	// 菜单项：点击触发回调
	struct MenuItem {
		std::string label;
		std::string shortcut;
		std::function<void()> onClick;
	};
	inline void draw(const MenuItem& w) {
		if(ImGui::MenuItem(w.label.c_str(),
			   w.shortcut.empty() ? nullptr : w.shortcut.c_str())
			&& w.onClick) {
			w.onClick();
		}
	}

	// 菜单：可嵌套（子菜单 = Menu 里再放 Menu）
	template<typename... Ts>
	struct Menu {
		std::string label;
		bool enabled;
		std::tuple<Ts...> children;
	};

	// 顶部主菜单栏
	template<typename... Ts>
	struct MenuBar {
		std::tuple<Ts...> children;
	};

	// 浮动面板：可在宿主窗口内容区内拖动/缩放（ImGui::Begin 实现）
	template<typename... Ts>
	struct Panel {
		std::string name;
		ImGuiWindowFlags flags;
		std::tuple<Ts...> children;
	};

	// draw 重载：先声明、再定义（见 fui_core.h 的说明）
	template<typename... Ts> void draw(const TabBar<Ts...>& w);
	template<typename... Ts> void draw(const Menu<Ts...>& w);
	template<typename... Ts> void draw(const MenuBar<Ts...>& w);
	template<typename... Ts> void draw(const Panel<Ts...>& w);

	inline void draw(const TextTable& w) {
		if(!ImGui::BeginTable(w.id.c_str(), (int)w.headers.size(), w.flags)) return;
		for(const char* h : w.headers) ImGui::TableSetupColumn(h);
		ImGui::TableHeadersRow();
		for(const auto& row : w.rows) {
			ImGui::TableNextRow();
			for(const auto& cell : row) {
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(cell.c_str());
			}
		}
		ImGui::EndTable();
	}

	inline void draw(const FnTable& w) {
		if(!ImGui::BeginTable(w.id.c_str(), (int)w.headers.size(), w.flags)) return;
		for(const char* h : w.headers) ImGui::TableSetupColumn(h);
		ImGui::TableHeadersRow();
		for(int r = 0; r < w.rowCount; ++r) {
			ImGui::TableNextRow();
			for(int col = 0; col < (int)w.headers.size(); ++col) {
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(w.cell(r, col).c_str());
			}
		}
		ImGui::EndTable();
	}

	template<typename... Ts>
	void draw(const TabBar<Ts...>& w) {
		if(!ImGui::BeginTabBar(w.id.c_str(), w.flags)) return;
		std::apply([&](const auto&... cs) {
			const auto one = [](const auto& c) {
				if(ImGui::BeginTabItem(c.name.c_str())) {
					draw(c.child);
					ImGui::EndTabItem();
				}
			};
			(one(cs), ...);
		}, w.children);
		ImGui::EndTabBar();
	}

	template<typename... Ts>
	void draw(const Menu<Ts...>& w) {
		if(ImGui::BeginMenu(w.label.c_str(), w.enabled)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::EndMenu();
		}
	}

	template<typename... Ts>
	void draw(const MenuBar<Ts...>& w) {
		if(ImGui::BeginMainMenuBar()) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::EndMainMenuBar();
		}
	}

	template<typename... Ts>
	void draw(const Panel<Ts...>& w) {
		ImGui::Begin(w.name.c_str(), nullptr, w.flags);
		std::apply([&](const auto&... cs) {
			const auto one = [](const auto& c) { draw(c); };
			(one(cs), ...);
		}, w.children);
		ImGui::End();
	}

	// —— 工厂 ——

	template<typename T>
	auto tab(std::string name, T view) {
		return Tab<std::decay_t<T>>{std::move(name), std::move(view)};
	}
	template<typename... Ts>
	auto tabBar(std::string id, ImGuiTabBarFlags flags, Ts&&... tabs) {
		return TabBar<std::decay_t<Ts>...>{
			std::move(id), flags, std::make_tuple(std::forward<Ts>(tabs)...)};
	}
	inline auto textTable(std::string id, std::span<const char* const> headers,
		const std::vector<std::vector<std::string>>& rows,
		ImGuiTableFlags flags = ImGuiTableFlags_Borders) {
		return TextTable{std::move(id), headers, rows, flags};
	}
	inline auto fnTable(std::string id, std::span<const char* const> headers,
		int rowCount, std::function<std::string(int, int)> cell,
		ImGuiTableFlags flags = ImGuiTableFlags_Borders) {
		return FnTable{std::move(id), headers, rowCount, std::move(cell), flags};
	}
	inline auto menuItem(std::string label, std::function<void()> onClick = nullptr,
		std::string shortcut = "") {
		return MenuItem{std::move(label), std::move(shortcut), std::move(onClick)};
	}
	template<typename... Ts>
	auto menu(std::string label, Ts&&... items) {
		return Menu<std::decay_t<Ts>...>{std::move(label), true,
			std::make_tuple(std::forward<Ts>(items)...)};
	}
	template<typename... Ts>
	auto menuBar(Ts&&... menus) {
		return MenuBar<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(menus)...)};
	}
	template<typename... Ts>
	auto panel(std::string name, ImGuiWindowFlags flags, Ts&&... views) {
		return Panel<std::decay_t<Ts>...>{
			std::move(name), flags, std::make_tuple(std::forward<Ts>(views)...)};
	}

}