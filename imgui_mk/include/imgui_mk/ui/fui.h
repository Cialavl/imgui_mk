#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/ui/widget.h>
#include <array>
#include <cfloat>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// 函数式控件库：控件是纯值（聚合类型，无基类、无虚函数），渲染是自由函数重载
// draw(const T&)——编译期决议、可内联；容器持子控件值的 tuple，fold 表达式编译期展开。
//
// 使用范式（每帧重建）：setUI 回调内每帧构造整棵视图值并绘制，状态全部外置于被绑定
// 的变量（引用绑定每帧重建仍实时联动），视图本身无生命周期负担：
//
//   mainWindow.setUI([&]{
//       Mk::fui::draw(Mk::fui::vbox(8.0f,
//           Mk::fui::label("标题"),
//           Mk::fui::button("确定", [&]{ ++clicks; }),
//           Mk::fui::disabled(Mk::fui::slider("音量", volume, 0.0f, 1.0f))
//       ));
//   });
//
// 扩展：任何类型只要在自己的命名空间提供 draw(const T&) 重载，即可被容器/修饰器/
// 适配器接受（依赖实参查找 ADL）——不需要继承任何基类。
//
// 与 Mk::ui（OOP 控件树）双向互通：
//   dyn(oopWidget)     把 OOP 子树（含墓碑动态增删）嵌入函数式树
//   asWidget(maker)    把每帧新鲜构造的函数式视图嵌入 OOP 树

namespace Mk::fui {

	// ============ 原子控件：聚合值类型 ============

	struct Label {
		std::string text;
	};

	struct Button {
		std::string label;
		std::function<void()> onClick;
	};

	// 滑块（数据绑定）：持外部 float 引用；视图每帧重建，引用重新绑定同一变量
	struct SliderF {
		std::string label;
		float& value;
		float mn, mx;
	};

	// 复选框（数据绑定）：绑定外部 bool 引用
	struct CheckBox {
		std::string label;
		bool& checked;
	};

	inline void draw(const Label& w) { ImGui::TextUnformatted(w.text.c_str()); }
	inline void draw(const Button& w) {
		if(ImGui::Button(w.label.c_str()) && w.onClick) w.onClick();
	}
	inline void draw(const SliderF& w) { ImGui::SliderFloat(w.label.c_str(), &w.value, w.mn, w.mx); }
	inline void draw(const CheckBox& w) { ImGui::Checkbox(w.label.c_str(), &w.checked); }

	// —— 输入控件 ——
	// 全部为数据绑定风格：持外部变量引用，每帧读引用绘制、交互时 ImGui 直接写回

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

	// —— 显示反馈控件 ——

	struct TextColored { std::string text; ImVec4 color; };
	inline void draw(const TextColored& w) { ImGui::TextColored(w.color, "%s", w.text.c_str()); }

	struct TextWrapped { std::string text; };
	inline void draw(const TextWrapped& w) { ImGui::TextWrapped("%s", w.text.c_str()); }

	struct BulletText { std::string text; };
	inline void draw(const BulletText& w) { ImGui::BulletText("%s", w.text.c_str()); }

	// 标签+值成对显示（右对齐值风格）
	struct LabelText { std::string label; std::string text; };
	inline void draw(const LabelText& w) { ImGui::LabelText(w.label.c_str(), "%s", w.text.c_str()); }

	// 进度条：绑定外部 fraction（0~1）
	struct ProgressBar { float& fraction; ImVec2 size; std::string overlay; };
	inline void draw(const ProgressBar& w) {
		ImGui::ProgressBar(w.fraction, w.size,
			w.overlay.empty() ? nullptr : w.overlay.c_str());
	}

	struct Separator {};
	inline void draw(const Separator&) { ImGui::Separator(); }

	struct SeparatorText { std::string label; };
	inline void draw(const SeparatorText& w) { ImGui::SeparatorText(w.label.c_str()); }

	// 可选中行：绑定外部 bool（选中态）
	struct Selectable { std::string label; bool& selected; };
	inline void draw(const Selectable& w) { ImGui::Selectable(w.label.c_str(), &w.selected); }

	// 图片：tex 经 loadTextureRGBA 获得（OpenGL 纹理句柄的 ImTextureID 封装）
	struct Image { ImTextureID tex; ImVec2 size; ImVec4 tint; ImVec4 border; };
	inline void draw(const Image& w) {
		ImGui::Image(w.tex, w.size, ImVec2(0, 0), ImVec2(1, 1), w.tint, w.border);
	}

	struct ImageButton { std::string id; ImTextureID tex; ImVec2 size; std::function<void()> onClick; };
	inline void draw(const ImageButton& w) {
		if(ImGui::ImageButton(w.id.c_str(), w.tex, w.size) && w.onClick) w.onClick();
	}

	// 折线图：绑定外部历史数据（const 引用，逻辑侧维护）
	struct PlotLines { std::string label; const std::vector<float>& values; float mn, mx; ImVec2 size; };
	inline void draw(const PlotLines& w) {
		ImGui::PlotLines(w.label.c_str(), w.values.data(), (int)w.values.size(),
			0, nullptr, w.mn, w.mx, w.size);
	}

	// —— 结构容器 ——

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

	// —— 弹窗与高级控件 ——

	// 弹窗：open 置位当帧打开（一次性触发器，打开后自动回落）；
	// 弹窗内容每帧渲染，直到子视图回调里调 ImGui::CloseCurrentPopup()
	template<typename... Ts>
	struct Popup {
		std::string id;
		bool& open;
		std::tuple<Ts...> children;
	};

	// 模态弹窗：同 Popup，但阻塞宿主窗口交互
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

	// ============ 容器 / 修饰器 / 桥：结构体定义 ============

	// 垂直布局：空隙只在项间。spacing < 0 用 ImGui 默认行距；== 0 紧凑；> 0 精确像素
	template<typename... Ts>
	struct VBox {
		float spacing;
		std::tuple<Ts...> children;
	};

	// 水平布局：空隙只在项间。spacing 直接透传 SameLine（< 0 用默认间距）
	template<typename... Ts>
	struct HBox {
		float spacing;
		std::tuple<Ts...> children;
	};

	// 分组：把多个子视图打包成一个整体参与外部布局，不产生可见边框
	template<typename... Ts>
	struct Group {
		std::tuple<Ts...> children;
	};

	// 折叠容器：标题栏可点击展开/收起，展开时才渲染子视图
	template<typename... Ts>
	struct Collapsing {
		std::string label;
		ImGuiTreeNodeFlags flags;
		std::tuple<Ts...> children;
	};

	// 嵌套子窗口（BeginChild）：可独立滚动、可带边框；id 为字符串值，不依赖对象地址
	template<typename... Ts>
	struct SubWindow {
		std::string id;
		ImVec2 size;
		bool border;
		std::tuple<Ts...> children;
	};

	// 禁用包裹：视觉禁用（灰化、不可交互），子视图仍每帧渲染
	template<typename T>
	struct Disabled {
		T child;
	};

	// 条件渲染：cond 为 false 时整棵子树本帧跳过
	template<typename T>
	struct When {
		bool cond;
		T child;
	};

	// ID 作用域包裹：函数式视图是临时值、无稳定地址，不能像 OOP 版靠 this 隔离，
	// 同 label 控件冲突时用它显式指定 ID 前缀
	template<typename T>
	struct WithId {
		std::string id;
		T child;
	};

	// 宽度修饰：设置子视图内下一个 item 的宽度（PushItemWidth/PopItemWidth）
	template<typename T>
	struct ItemWidth {
		float width;
		T child;
	};

	// 树节点容器：TreeNodeEx 展开时渲染子视图，收起时跳过
	template<typename... Ts>
	struct TreeNode {
		std::string label;
		ImGuiTreeNodeFlags flags;
		std::tuple<Ts...> children;
	};

	// 悬停提示：子视图渲染后，鼠标悬停在该 item 上时显示提示文本
	template<typename T>
	struct Tooltip { T child; std::string text; };

	// 颜色修饰：PushStyleColor 包裹子视图（给一段任意视图上色）
	template<typename T>
	struct Colored { ImGuiCol idx; ImVec4 color; T child; };

	// 缩进修饰：子视图整体缩进 amount 像素
	template<typename T>
	struct Indent { float amount; T child; };

	// OOP 子树桥：把 Mk::ui::Widget（含墓碑机制的动态增删树）嵌入函数式树。
	// OOP 对象存活期仍须覆盖引擎 run() 循环（引用不接管所有权）
	struct Dyn {
		ui::Widget& widget;
	};

	// ============ draw 重载：先整体声明、再定义 ============
	// 容器/修饰器可任意相互嵌套，前置声明保证每个定义点的普通查找
	// 就能看到完整重载集（不依赖编译器对模板内 ADL 时机的实现差异）

	template<typename... Ts> void draw(const VBox<Ts...>& w);
	template<typename... Ts> void draw(const HBox<Ts...>& w);
	template<typename... Ts> void draw(const Group<Ts...>& w);
	template<typename... Ts> void draw(const Collapsing<Ts...>& w);
	template<typename... Ts> void draw(const SubWindow<Ts...>& w);
	template<typename T> void draw(const Disabled<T>& w);
	template<typename T> void draw(const When<T>& w);
	template<typename T> void draw(const WithId<T>& w);
	template<typename T> void draw(const ItemWidth<T>& w);
	template<typename... Ts> void draw(const TreeNode<Ts...>& w);
	template<typename T> void draw(const Tooltip<T>& w);
	template<typename T> void draw(const Colored<T>& w);
	template<typename T> void draw(const Indent<T>& w);
	template<typename... Ts> void draw(const TabBar<Ts...>& w);
	template<typename... Ts> void draw(const Menu<Ts...>& w);
	template<typename... Ts> void draw(const MenuBar<Ts...>& w);
	template<typename... Ts> void draw(const Panel<Ts...>& w);
	template<typename... Ts> void draw(const Popup<Ts...>& w);
	template<typename... Ts> void draw(const PopupModal<Ts...>& w);
	inline void draw(const Dyn& w);

	// 容器渲染：draw(child) 为依赖调用，实例化时经 ADL 找到用户自定义类型的重载
	template<typename... Ts>
	void draw(const VBox<Ts...>& box) {
		const auto gap = [&box] {
			if(box.spacing < 0.0f) ImGui::Spacing();
			else if(box.spacing > 0.0f) ImGui::Dummy(ImVec2(0.0f, box.spacing));
		};
		std::apply([&](const auto&... cs) {
			bool first = true;
			const auto one = [&](const auto& c) {
				if(!first) gap();
				first = false;
				draw(c);
			};
			(one(cs), ...);
		}, box.children);
	}

	template<typename... Ts>
	void draw(const HBox<Ts...>& box) {
		std::apply([&](const auto&... cs) {
			bool first = true;
			const auto one = [&](const auto& c) {
				if(!first) ImGui::SameLine(0.0f, box.spacing);
				first = false;
				draw(c);
			};
			(one(cs), ...);
		}, box.children);
	}

	template<typename... Ts>
	void draw(const Group<Ts...>& w) {
		ImGui::BeginGroup();
		std::apply([&](const auto&... cs) {
			const auto one = [](const auto& c) { draw(c); };
			(one(cs), ...);
		}, w.children);
		ImGui::EndGroup();
	}

	template<typename... Ts>
	void draw(const Collapsing<Ts...>& w) {
		if(ImGui::CollapsingHeader(w.label.c_str(), w.flags)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
		}
	}

	template<typename... Ts>
	void draw(const SubWindow<Ts...>& w) {
		if(ImGui::BeginChild(w.id.c_str(), w.size, w.border ? ImGuiChildFlags_Border : 0)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
		}
		ImGui::EndChild();
	}

	template<typename T>
	void draw(const Disabled<T>& w) {
		ImGui::BeginDisabled();
		draw(w.child);
		ImGui::EndDisabled();
	}

	template<typename T>
	void draw(const When<T>& w) {
		if(w.cond) draw(w.child);
	}

	template<typename T>
	void draw(const WithId<T>& w) {
		ImGui::PushID(w.id.c_str());
		draw(w.child);
		ImGui::PopID();
	}

	template<typename T>
	void draw(const ItemWidth<T>& w) {
		ImGui::PushItemWidth(w.width);
		draw(w.child);
		ImGui::PopItemWidth();
	}

	template<typename... Ts>
	void draw(const TreeNode<Ts...>& w) {
		if(ImGui::TreeNodeEx(w.label.c_str(), w.flags)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::TreePop();
		}
	}

	template<typename T>
	void draw(const Tooltip<T>& w) {
		draw(w.child);
		if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s", w.text.c_str());
	}

	template<typename T>
	void draw(const Colored<T>& w) {
		ImGui::PushStyleColor(w.idx, w.color);
		draw(w.child);
		ImGui::PopStyleColor();
	}

	template<typename T>
	void draw(const Indent<T>& w) {
		ImGui::Indent(w.amount);
		draw(w.child);
		ImGui::Unindent(w.amount);
	}

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

	inline void draw(const Dyn& w) { w.widget.draw(); }

	// ============ 反向桥：函数式视图嵌入 OOP 树 ============

	namespace detail {
		// 非成员上下文中转 draw 调用：适配器派生自 ui::Widget，其无参成员 draw()
		// 在类作用域遮蔽命名空间级 draw 重载，经此中转恢复正常（ADL）查找
		template<typename T>
		void dispatchDraw(const T& v) { draw(v); }
	}

	// maker 每帧调用、返回当帧新鲜构造的视图值，保持“每帧重建”语义；
	// 适配器本身是长期存活的 OOP Widget
	template<typename F>
	class WidgetAdapter final : public ui::Widget {
	public:
		explicit WidgetAdapter(F maker) : m_Maker(std::move(maker)) {}

	protected:
		void render() override {
			auto view = m_Maker();
			detail::dispatchDraw(view);
		}

	private:
		F m_Maker;
	};

	// ============ 工厂组合子（推导模板参数；子视图按值收入 tuple）============

	inline auto label(std::string text) { return Label{std::move(text)}; }
	inline auto button(std::string lbl, std::function<void()> onClick = nullptr) {
		return Button{std::move(lbl), std::move(onClick)};
	}
	inline auto slider(std::string lbl, float& value, float mn, float mx) {
		return SliderF{std::move(lbl), value, mn, mx};
	}
	inline auto checkbox(std::string lbl, bool& checked) {
		return CheckBox{std::move(lbl), checked};
	}

	template<typename... Ts>
	auto vbox(float spacing, Ts&&... views) {
		return VBox<std::decay_t<Ts>...>{spacing, std::make_tuple(std::forward<Ts>(views)...)};
	}
	template<typename... Ts>
	auto hbox(float spacing, Ts&&... views) {
		return HBox<std::decay_t<Ts>...>{spacing, std::make_tuple(std::forward<Ts>(views)...)};
	}
	template<typename... Ts>
	auto group(Ts&&... views) {
		return Group<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(views)...)};
	}

	template<typename... Ts>
	auto collapsing(std::string lbl, ImGuiTreeNodeFlags flags, Ts&&... views) {
		return Collapsing<std::decay_t<Ts>...>{
			std::move(lbl), flags, std::make_tuple(std::forward<Ts>(views)...)};
	}

	template<typename... Ts>
	auto subwindow(std::string id, ImVec2 size, bool border, Ts&&... views) {
		return SubWindow<std::decay_t<Ts>...>{
			std::move(id), size, border, std::make_tuple(std::forward<Ts>(views)...)};
	}

	template<typename T>
	auto disabled(T child) { return Disabled<std::decay_t<T>>{std::move(child)}; }
	template<typename T>
	auto when(bool cond, T child) { return When<std::decay_t<T>>{cond, std::move(child)}; }
	template<typename T>
	auto withId(std::string id, T child) {
		return WithId<std::decay_t<T>>{std::move(id), std::move(child)};
	}

	// —— 输入控件工厂 ——
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
	template<typename T>
	auto itemWidth(float width, T child) { return ItemWidth<std::decay_t<T>>{width, std::move(child)}; }

	// —— 显示反馈控件工厂 ——
	inline auto textColored(std::string text, const ImVec4& color) {
		return TextColored{std::move(text), color};
	}
	inline auto textWrapped(std::string text) { return TextWrapped{std::move(text)}; }
	inline auto bulletText(std::string text) { return BulletText{std::move(text)}; }
	inline auto labelText(std::string label, std::string text) {
		return LabelText{std::move(label), std::move(text)};
	}
	inline auto progressBar(float& fraction, ImVec2 size = ImVec2(-FLT_MIN, 0),
		std::string overlay = "") {
		return ProgressBar{fraction, size, std::move(overlay)};
	}
	inline auto separator() { return Separator{}; }
	inline auto separatorText(std::string label) { return SeparatorText{std::move(label)}; }
	inline auto selectable(std::string label, bool& selected) {
		return Selectable{std::move(label), selected};
	}
	inline auto image(ImTextureID tex, ImVec2 size,
		const ImVec4& tint = ImVec4(1, 1, 1, 1), const ImVec4& border = ImVec4(0, 0, 0, 0)) {
		return Image{tex, size, tint, border};
	}
	inline auto imageButton(std::string id, ImTextureID tex, ImVec2 size,
		std::function<void()> onClick = nullptr) {
		return ImageButton{std::move(id), tex, size, std::move(onClick)};
	}
	inline auto plotLines(std::string label, const std::vector<float>& values,
		ImVec2 size = ImVec2(0, 60), float mn = FLT_MAX, float mx = FLT_MAX) {
		return PlotLines{std::move(label), values, mn, mx, size};
	}
	template<typename... Ts>
	auto treeNode(std::string label, ImGuiTreeNodeFlags flags, Ts&&... views) {
		return TreeNode<std::decay_t<Ts>...>{
			std::move(label), flags, std::make_tuple(std::forward<Ts>(views)...)};
	}
	template<typename T>
	auto tooltip(T child, std::string text) {
		return Tooltip<std::decay_t<T>>{std::move(child), std::move(text)};
	}
	template<typename T>
	auto colored(ImGuiCol idx, const ImVec4& color, T child) {
		return Colored<std::decay_t<T>>{idx, color, std::move(child)};
	}
	template<typename T>
	auto indent(float amount, T child) { return Indent<std::decay_t<T>>{amount, std::move(child)}; }

	// —— 结构容器工厂 ——
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

	// —— 弹窗与高级控件工厂 ——
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

	// —— 纹理辅助（OpenGL）——
	// RGBA8 像素上传为纹理，返回可传给 image/imageButton 的 ImTextureID。
	// 须在窗口/GL 上下文存活期间调用；释放用 deleteTexture
	inline ImTextureID loadTextureRGBA(const unsigned char* pixels, int width, int height) {
		GLuint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		return (ImTextureID)(intptr_t)tex;
	}

	inline void deleteTexture(ImTextureID tex) {
		GLuint t = (GLuint)(intptr_t)tex;
		glDeleteTextures(1, &t);
	}

	inline auto dyn(ui::Widget& w) { return Dyn{w}; }

	template<typename F>
	auto asWidget(F maker) { return WidgetAdapter<std::decay_t<F>>(std::move(maker)); }

}