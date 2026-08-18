#pragma once
#include <imgui_mk/windowRendereren.h>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Mk::wg：无继承的模板 struct 控件体系（类型萃取 + 类型分发）。
//
// 设计三支柱：
//   1) 控件 = 自己的模板 struct（数据 + 功能函数）——如 ButtonImpl{label,onClick,renderImpl,...}
//   2) 组合壳 Control<Self, Style, Ts...>：模板参数传入组合（零继承）——
//      持有 Self self（数据函数）、Node node（树节点）、Style style、tuple<Ts...> children
//   3) Node 类型擦除行为表：ControlTraits<Self> 经 if constexpr 探测成员函数
//      （成员优先 → ADL 自由函数 → 缺省兜底），7 个函数指针构造时绑定。
//
// 对象树：编译期 tuple 子组合（模板传入）为主，Node 运行时 children（墓碑）
// 兜底动态增删。布局：显式 bounds（measure/arrange 两遍，父容器分配）。
// 生命周期：draw() 时检测（onCreate 首次绘制 / onShow/onHide 可见性变化 /
// onDestroy 从树移除时）。

namespace Mk::wg {

	// ============ 基础布局类型 ============

	struct Size {
		float w = 0.0f, h = 0.0f;
	};

	struct Rect {
		float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
		Rect() = default;
		Rect(float xx, float yy, float ww, float hh) : x(xx), y(yy), w(ww), h(hh) {}
	};

	// ============ 样式（组合件，默认值来自当前 ImGui 样式）============

	struct ControlStyle {
		ImVec4 text{0.92f, 0.92f, 0.94f, 1.0f};
		ImVec4 frameBg{0.20f, 0.20f, 0.23f, 1.0f};
		ImVec4 button{0.24f, 0.26f, 0.31f, 1.0f};
		float frameRounding = 4.0f;
		// ...可扩展（颜色/内边距/圆角），继承链：控件 → 父 → 窗口 Theme

		static ControlStyle fromCurrent() {   // 从当前 ImGui 样式快照
			const ImGuiStyle& s = ImGui::GetStyle();
			ControlStyle st;
			st.text = s.Colors[ImGuiCol_Text];
			st.frameBg = s.Colors[ImGuiCol_FrameBg];
			st.button = s.Colors[ImGuiCol_Button];
			st.frameRounding = s.FrameRounding;
			return st;
		}

		// 应用为当前样式作用域（draw 进入控件时 push，离开 pop）
		void push() const {
			ImGui::PushStyleColor(ImGuiCol_Text, text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, frameBg);
			ImGui::PushStyleColor(ImGuiCol_Button, button);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);
		}
		static void pop() {
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
		}
	};

	// 缺省样式件：无自定义（继承链向上取父/窗口）
	struct DefaultStyle {
		static void push() {}
		static void pop() {}
	};

	// ============ Node：类型擦除对象树节点（非基类）============

	struct Node;
	template<typename T> struct ControlTraits;

	struct Node {
		Node* parent = nullptr;
		std::vector<Node*> children;     // 运行时对象树（墓碑增删，动态兜底）
		std::string id;
		bool visible = true, enabled = true;
		Rect bounds;                     // 父容器 arrange 分配
		void* owner = nullptr;           // 指向拥有它的控件对象（无继承的转型通道）

		// —— 类型擦除行为表（函数指针，bind<Self>() 时由类型萃取填充）——
		void (*m_Render)(Node&) = nullptr;
		Size (*m_Measure)(Node&, const Size&) = nullptr;
		void (*m_Arrange)(Node&, const Rect&) = nullptr;
		void (*m_OnCreate)(Node&) = nullptr;
		void (*m_OnDestroy)(Node&) = nullptr;
		void (*m_OnShow)(Node&) = nullptr;
		void (*m_OnHide)(Node&) = nullptr;

		// 类型分发绑定：按 T 萃取能力填充行为表
		template<typename T>
		void bind();

		// 渲染入口：生命周期检测 + 样式作用域 + ID 隔离 + 树递归
		void draw();
		// 显式布局：measure（求期望尺寸）→ arrange（父分配 bounds）
		Size measure(const Size& avail);
		void arrange(const Rect& r);
		// 一步布局 + 渲染（应用每帧调用：root.layout(hostRect)）
		void layout(const Rect& host);

		// —— 树操作（墓碑语义：渲染回调内增删安全）——
		void add(Node& child);
		void remove(Node& child);
		void clear();

		Node* findById(const std::string& want);

	private:
		bool m_Hooked = false;       // 生命周期：首次 draw 前置位
		bool m_LastVisible = true;   // onShow/onHide 检测基准
		void compact();              // 压实墓碑槽位
	};

	// ============ 类型萃取 + 类型分发：ControlTraits<T> ============

	namespace detail {
		// 默认兜底：无 renderImpl 的节点只递归渲染孩子
		inline void renderChildrenOnly(Node& n) {
			for(auto* c : n.children) {
				if(c) c->draw();
			}
		}
	}

	template<typename T>
	struct ControlTraits {
		static void render(Node& n) {
			auto* self = static_cast<T*>(n.owner);   // 一处下沉转型
			if constexpr (requires { self->renderImpl(n); }) {
				self->renderImpl(n);
			}
			else if constexpr (requires { renderImpl(*self, n); }) {
				renderImpl(*self, n);                // ADL 自由函数兜底
			}
			else {
				detail::renderChildrenOnly(n);
			}
		}

		static Size measure(Node& n, const Size& avail) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->measureImpl(n, avail); }) {
				return self->measureImpl(n, avail);
			}
			else if constexpr (requires { measureImpl(*self, n, avail); }) {
				return measureImpl(*self, n, avail);
			}
			else {
				return Size{avail.w, 0.0f};          // 缺省：取可用宽、高度 0
			}
		}

		static void arrange(Node& n, const Rect& r) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->arrangeImpl(n, r); }) {
				self->arrangeImpl(n, r);
			}
			else if constexpr (requires { arrangeImpl(*self, n, r); }) {
				arrangeImpl(*self, n, r);
			}
			// 缺省：无操作（bounds 保持 host）
		}

		static void onCreate(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->onCreateImpl(n); }) self->onCreateImpl(n);
			else if constexpr (requires { onCreateImpl(*self, n); }) onCreateImpl(*self, n);
		}
		static void onDestroy(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->onDestroyImpl(n); }) self->onDestroyImpl(n);
			else if constexpr (requires { onDestroyImpl(*self, n); }) onDestroyImpl(*self, n);
		}
		static void onShow(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->onShowImpl(n); }) self->onShowImpl(n);
			else if constexpr (requires { onShowImpl(*self, n); }) onShowImpl(*self, n);
		}
		static void onHide(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->onHideImpl(n); }) self->onHideImpl(n);
			else if constexpr (requires { onHideImpl(*self, n); }) onHideImpl(*self, n);
		}
	};

	// ============ Node 实现 ============

	template<typename T>
	void Node::bind() {
		m_Render    = &ControlTraits<T>::render;
		m_Measure   = &ControlTraits<T>::measure;
		m_Arrange   = &ControlTraits<T>::arrange;
		m_OnCreate  = &ControlTraits<T>::onCreate;
		m_OnDestroy = &ControlTraits<T>::onDestroy;
		m_OnShow    = &ControlTraits<T>::onShow;
		m_OnHide    = &ControlTraits<T>::onHide;
	}

	inline void Node::draw() {
		// —— 生命周期检测（draw 驱动；visible=false 也要检测 onHide）——
		if(!m_Hooked) {
			m_Hooked = true;
			if(m_OnCreate) m_OnCreate(*this);
		}
		if(visible != m_LastVisible) {
			m_LastVisible = visible;
			if(!visible && m_OnHide) m_OnHide(*this);
			if(visible && m_OnShow) m_OnShow(*this);
		}
		if(!visible) return;

		ImGui::PushID(this);
		if(!enabled) ImGui::BeginDisabled();
		if(m_Render) m_Render(*this);
		else detail::renderChildrenOnly(*this);
		if(!enabled) ImGui::EndDisabled();
		ImGui::PopID();
	}

	inline Size Node::measure(const Size& avail) {
		return m_Measure ? m_Measure(*this, avail) : Size{avail.w, 0.0f};
	}

	inline void Node::arrange(const Rect& r) {
		bounds = r;
		if(m_Arrange) m_Arrange(*this, r);
	}

	inline void Node::layout(const Rect& host) {
		const Size desired = measure(Size{host.w, host.h});
		Rect r = host;
		r.w = desired.w;
		r.h = desired.h;
		arrange(r);
		draw();
	}

	inline void Node::compact() {
		children.erase(
			std::remove(children.begin(), children.end(), nullptr),
			children.end());
	}

	inline void Node::add(Node& child) {
		child.parent = this;
		children.push_back(&child);
	}

	inline void Node::remove(Node& child) {
		for(auto& slot : children) {
			if(slot == &child) {
				// onDestroy：已绘制过的节点在移除时同步触发（此后不会再被 draw）
				if(child.m_Hooked && child.m_OnDestroy) child.m_OnDestroy(child);
				child.parent = nullptr;
				slot = nullptr;    // 墓碑：渲染回调内调用安全
				return;
			}
		}
	}

	inline void Node::clear() {
		for(auto& slot : children) {
			if(slot) {
				if(slot->m_Hooked && slot->m_OnDestroy) slot->m_OnDestroy(*slot);
				slot->parent = nullptr;
			}
			slot = nullptr;
		}
	}

	inline Node* Node::findById(const std::string& want) {
		if(id == want) return this;
		compact();
		for(auto* c : children) {
			if(Node* found = c->findById(want)) return found;
		}
		return nullptr;
	}

	// ============ 组合壳：Control<Self, Style, Ts...>（零继承）============

	template<typename Self, typename Style = DefaultStyle, typename... Ts>
	struct Control {
		Self self;                    // 控件数据 + 功能函数（组合成员）
		Node node;                    // 树节点（组合成员，非基类）
		Style style;                  // 样式件（组合成员）
		std::tuple<Ts...> children;   // 编译期子控件组合（模板参数传入）

		// 无参构造：控件数据用默认值（原子控件也常用）
		// 无参构造：控件数据用默认值（原子控件也常用）；Self 不可默认构造时跳过
		Control() requires std::default_initializable<Self> : self() {
			node.owner = &self;
			node.bind<Self>();
			std::apply([&](auto&... c) { (node.add(c.node), ...); }, children);
		}

		// 转发构造：参数直通 self（控件数据/函数的聚合初始化）
		template<typename... Args>
		explicit Control(Args&&... args) : self(std::forward<Args>(args)...) {
			node.owner = &self;       // owner 指向控件对象（traits 下沉转型通道）
			node.bind<Self>();        // 类型萃取 → 填充行为表
			// 子控件挂树：tuple 成员已构造，此处 add 到本节点
			std::apply([&](auto&... c) { (node.add(c.node), ...); }, children);
		}
		Control(const Control&) = delete;             // 树用引用语义，对象位置即身份
		Control& operator=(const Control&) = delete;
		~Control() { node.clear(); }                 // 子节点随壳析构安全解挂

		Self& get() { return self; }
		const Self& get() const { return self; }

		// 编译期子控件访问（模板组合）：child<I>() 返回第 I 个子控件
		template<std::size_t I>
		auto& child() { return std::get<I>(children); }
		template<std::size_t I>
		const auto& child() const { return std::get<I>(children); }

		void draw() { node.draw(); }
		void layout(const Rect& host) { node.layout(host); }
		void add(Node& child) { node.add(child); }
		void remove(Node& child) { node.remove(child); }
		void clear() { node.clear(); }
		Node* findById(const std::string& i) { return node.findById(i); }
	};

}