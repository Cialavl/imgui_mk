#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/ui/widget.h>
#include <array>
#include <functional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

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

	inline auto dyn(ui::Widget& w) { return Dyn{w}; }

	template<typename F>
	auto asWidget(F maker) { return WidgetAdapter<std::decay_t<F>>(std::move(maker)); }

}