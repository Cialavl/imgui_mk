#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/ui/widget.h>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// 函数式控件库·核心：原子控件、布局容器、基础修饰器、与 Mk::ui 的双向桥。
// 其余模块（fui_inputs / fui_display / fui_containers / fui_popups）都依赖本文件；
// 日常使用直接包含伞头 <imgui_mk/ui/fui.h>。
//
// 设计：控件是纯值（聚合类型，无基类、无虚函数），渲染是自由函数重载
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

	// ============ 布局容器 ============

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

	// ============ 基础修饰器：装饰靠组合不靠继承 ============

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

	// OOP 子树桥：把 Mk::ui::Widget（含墓碑机制的动态增删树）嵌入函数式树。
	// OOP 对象存活期仍须覆盖引擎 run() 循环（引用不接管所有权）
	struct Dyn {
		ui::Widget& widget;
	};

	// ============ 自适应布局原语 ============

	// 弹性分格（hsplit/vsplit 的格子）：fixed 固定像素；stretch 按权重瓜分剩余。
	// 格子内容以 BeginChild 承载（独立 id，继承主题样式）
	template<typename T>
	struct SplitCell {
		bool stretch;     // false=固定，true=弹性
		float size;       // fixed: 像素；stretch: 权重（相对其它 stretch 格）
		std::string id;   // 子窗口 id（需在分格树内唯一）
		T child;
	};

	// 水平弹性分格：avail 依次扣除 fixed 格与项间 spacing 后，按权重分配给 stretch 格
	template<typename... Ts>
	struct HSplit {
		std::tuple<Ts...> cells;
	};

	// 垂直弹性分格：同 HSplit，方向为上到下
	template<typename... Ts>
	struct VSplit {
		std::tuple<Ts...> cells;
	};

	// 宽度比例修饰：子视图内下一个 item 宽度 = 可用宽 × frac（1.0 占满）
	template<typename T>
	struct WidthFrac {
		float frac;
		T child;
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
	inline void draw(const Dyn& w);
	template<typename... Ts> void draw(const HSplit<Ts...>& w);
	template<typename... Ts> void draw(const VSplit<Ts...>& w);
	template<typename T> void draw(const WidthFrac<T>& w);

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

	inline void draw(const Dyn& w) { w.widget.draw(); }

	namespace detail {
		// 渲染一个分格：占 track 像素（水平格传宽，垂直格传高），另一维填满
		template<typename T>
		void drawCell(const SplitCell<T>& cell, bool horizontal, float track) {
			const ImVec2 sz = horizontal
				? ImVec2(track, 0.0f)
				: ImVec2(0.0f, track);
			if(ImGui::BeginChild(cell.id.c_str(), sz, ImGuiChildFlags_Border)) {
				draw(cell.child);
			}
			ImGui::EndChild();
		}
	}

	template<typename... Ts>
	void draw(const HSplit<Ts...>& w) {
		const float avail = ImGui::GetContentRegionAvail().x;
		const float gap = ImGui::GetStyle().ItemSpacing.x;

		// 两遍：先求 fixed 总量与权重和，再逐格分配
		float fixedSum = 0.0f, weightSum = 0.0f;
		std::size_t nCells = 0;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				if(c.stretch) weightSum += c.size;
				else fixedSum += c.size;
				++nCells;
			};
			(one(cs), ...);
		}, w.cells);

		const float stretchTotal = avail - fixedSum - (float)(nCells > 1 ? nCells - 1 : 0) * gap;
		std::size_t idx = 0;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				if(idx > 0) ImGui::SameLine(0.0f, gap);
				const float track = c.stretch
					? (weightSum > 0.0f ? stretchTotal * (c.size / weightSum) : 0.0f)
					: c.size;
				detail::drawCell(c, true, track);
				++idx;
			};
			(one(cs), ...);
		}, w.cells);
	}

	template<typename... Ts>
	void draw(const VSplit<Ts...>& w) {
		const float avail = ImGui::GetContentRegionAvail().y;
		const float gap = ImGui::GetStyle().ItemSpacing.y;

		float fixedSum = 0.0f, weightSum = 0.0f;
		std::size_t nCells = 0;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				if(c.stretch) weightSum += c.size;
				else fixedSum += c.size;
				++nCells;
			};
			(one(cs), ...);
		}, w.cells);

		const float stretchTotal = avail - fixedSum - (float)(nCells > 1 ? nCells - 1 : 0) * gap;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				const float track = c.stretch
					? (weightSum > 0.0f ? stretchTotal * (c.size / weightSum) : 0.0f)
					: c.size;
				detail::drawCell(c, false, track);
			};
			(one(cs), ...);
		}, w.cells);
	}

	template<typename T>
	void draw(const WidthFrac<T>& w) {
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * w.frac);
		draw(w.child);
		ImGui::PopItemWidth();
	}

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
	template<typename T>
	auto itemWidth(float width, T child) { return ItemWidth<std::decay_t<T>>{width, std::move(child)}; }

	// —— 自适应布局工厂 ——
	// 固定格：size 像素；id 需在同一分格树内唯一
	template<typename T>
	auto fixed(std::string id, float size, T child) {
		return SplitCell<std::decay_t<T>>{false, size, std::move(id), std::move(child)};
	}
	// 弹性格：weight 相对权重（同树内比较），瓜分 fixed 与间距之外的剩余空间
	template<typename T>
	auto stretch(std::string id, float weight, T child) {
		return SplitCell<std::decay_t<T>>{true, weight, std::move(id), std::move(child)};
	}
	template<typename... Ts>
	auto hsplit(Ts&&... cells) {
		return HSplit<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(cells)...)};
	}
	template<typename... Ts>
	auto vsplit(Ts&&... cells) {
		return VSplit<std::decay_t<Ts>...>{std::make_tuple(std::forward<Ts>(cells)...)};
	}
	template<typename T>
	auto widthFrac(float frac, T child) {
		return WidthFrac<std::decay_t<T>>{frac, std::move(child)};
	}

	// 子窗口尺寸语义化别名：fill()=两维填满；fillW(h)=填宽定高；fillH(w)=填高定宽
	inline ImVec2 fill() { return ImVec2(0.0f, 0.0f); }
	inline ImVec2 fillW(float h) { return ImVec2(0.0f, h); }
	inline ImVec2 fillH(float w) { return ImVec2(w, 0.0f); }

	inline auto dyn(ui::Widget& w) { return Dyn{w}; }

	template<typename F>
	auto asWidget(F maker) { return WidgetAdapter<std::decay_t<F>>(std::move(maker)); }

}