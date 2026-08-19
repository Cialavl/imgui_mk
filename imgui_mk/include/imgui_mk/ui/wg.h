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

	// ===== Qt 式尺寸协议 =====
	// Policy：尺寸策略（QSizePolicy 子集）。Fixed=只用 hint；Minimum=hint 可增大；
	// Maximum=hint 可缩小；Preferred=可增可减（默认）；Expanding=可增可减且抢剩余；Ignored=可压到 min。
	struct Policy {
		enum Kind : unsigned char { Fixed, Minimum, Maximum, Preferred, Expanding, Ignored };
		Kind h = Preferred, v = Preferred;
		constexpr bool canGrow(Kind k) const { return k == Minimum || k == Preferred || k == Expanding; }
		constexpr bool canShrink(Kind k) const { return k == Maximum || k == Preferred || k == Expanding || k == Ignored; }
		constexpr bool wantsExpand(Kind k) const { return k == Expanding; }
	};

	// 显式覆盖（优先于控件 hint/policy）：负值 = 未设置
	struct SizeOverrides {
		float minW = -1, minH = -1, maxW = -1, maxH = -1;   // 尺寸钳制（像素）
		float prefW = -1, prefH = -1;                       // hint 覆盖（setPrefSize；min/max 独立生效）
		float fixedW = -1, fixedH = -1;                     // 锁定尺寸（setFixedSize/Width/Height）
	};

	// 布局条目角色（addWidget(w, stretch) 传入；仅本层布局可见）
	struct ItemRole {
		float stretch = 0.0f;   // 拉伸权重（Qt addWidget(w, stretch)）；0 = 按策略分配
		bool hasRole = false;
	};

	struct Node;
	template<typename T> struct ControlTraits;

	struct Node {
		Node* parent = nullptr;
		std::vector<Node*> children;     // 运行时对象树（墓碑增删，动态兜底）
		std::vector<Node*> m_Pending;    // deferred add queue (flushed at layout start)
		int m_LayoutDepth = 0;           // >0 while inside layout/draw
		std::string idname;             // 身份标识：findById 查找键 / 容器（Tab 页名等）ImGui ID 来源
		bool visible = true, enabled = true;
		Rect bounds;                     // 父容器 arrange 分配
		void* owner = nullptr;           // 指向拥有它的控件对象（无继承的转型通道）
		ItemRole role;                // 布局条目角色（stretch 权重；addWidget 第 2 参）
		SizeOverrides overrides;      // 显式尺寸覆盖（setFixedWidth/min/max...）
		bool fill = false;                // 填充型容器（分格）：layout 保留 host 尺寸而非内容尺寸
		ImVec2 origin{0.0f, 0.0f};        // 绘制原点（当前窗口坐标空间）：父容器 draw 子节点前设置，
		                                  // 根节点由 layout() 设为 host 左上角；子窗口类容器切换坐标空间后重起算

		// —— 类型擦除行为表（函数指针，bind<Self>() 时由类型萃取填充）——
		void (*m_Render)(Node&) = nullptr;
		Size (*m_Measure)(Node&, const Size&) = nullptr;
		void (*m_Arrange)(Node&, const Rect&) = nullptr;
		void (*m_OnCreate)(Node&) = nullptr;
		void (*m_OnDestroy)(Node&) = nullptr;
		void (*m_OnShow)(Node&) = nullptr;
		void (*m_OnHide)(Node&) = nullptr;
		// Qt 式尺寸协议函数指针（bind 时与旧协议一并绑定）
		Size (*m_SizeHint)(Node&) = nullptr;
		Size (*m_MinHint)(Node&) = nullptr;
		Policy (*m_Policy)(Node&) = nullptr;
		bool (*m_HasHFW)(Node&) = nullptr;
		float (*m_HFW)(Node&, float) = nullptr;

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
		// Qt 式尺寸查询（类外实现；求解器用）
		Size hint();
		Size minHint();
		Policy getPolicy();
		bool hasHFW();
		float hfw(float w);

		// —— 树操作（墓碑语义：渲染回调内增删安全）——
		void flushPending();   // 应用延迟 add（measure/arrange/draw/layout 开头调用）
		void add(Node& child);
		void remove(Node& child);
		void clear();

		Node* findByIdname(const std::string& want);   // 按 idname 递归查找（顺带 compact）

	private:
		bool m_Hooked = false;       // 生命周期：首次 draw 前置位
		bool m_LastVisible = true;   // onShow/onHide 检测基准
		void compact();              // 压实墓碑槽位
	};

	// ============ 类型萃取 + 类型分发：ControlTraits<T> ============

	namespace detail {
	// 全局渲染深度（thread_local）：任何 layout/draw 进行中 > 0（渲染期检测）
	inline thread_local int g_RenderDepth = 0;
		// —— 引擎读数小助手（应用层免直连 ImGui IO）——
		inline float fps() { return ImGui::GetIO().Framerate; }
		inline bool ctrlHeld() { return ImGui::GetIO().KeyCtrl; }
		inline bool keyPressed(ImGuiKey key) { return ImGui::IsKeyPressed(key, false); }
		// —— 嵌套坐标空间传播（绘制原点）——
		// 渲染发生在"当前窗口"坐标空间：SetCursorPos 取当前窗口局部坐标。
		// 子窗口类容器（SubWindow/Panel/弹窗/菜单）开启新空间，其余容器共享父空间。
		// origin 记录本节点在当前空间的绘制原点：父容器画子节点前设置，
		// 叶子/容器据此定位——任意嵌套（容器嵌容器、容器嵌子窗口）坐标链不断。

		// 同空间对位：子按 bounds 相对偏移放置（VBox/HBox/Group/分格/Tree/Collapsing）
		inline void drawChildAt(Node& n, Node& c) {
			const ImVec2 p(n.origin.x + c.bounds.x - n.bounds.x,
				n.origin.y + c.bounds.y - n.bounds.y);
			ImGui::SetCursorPos(p);
			c.origin = p;
			c.draw();
		}

		// 顺序流式：从当前光标接着排（弹窗/菜单/Tab/缩进/装饰件包裹）
		inline void drawChildFlow(Node& c) {
			c.origin = ImGui::GetCursorPos();
			c.draw();
		}

		// 子窗口局部：以本节点 bounds 为原点的新空间（SubWindow/Panel 内）
		inline void drawChildLocal(Node& n, Node& c) {
			const ImVec2 p(c.bounds.x - n.bounds.x, c.bounds.y - n.bounds.y);
			ImGui::SetCursorPos(p);
			c.origin = p;
			c.draw();
		}

		// 默认兜底：无 renderImpl 的节点只递归渲染孩子
		inline void renderChildrenOnly(Node& n) {
			for(auto* c : n.children) {
				if(c) drawChildFlow(*c);
			}
		}
	}

	template<typename T>
	struct ControlTraits {
		static void render(Node& n) {
			auto* self = static_cast<T*>(n.owner);   // 一次下转型
			if constexpr (requires { self->renderImpl(n); }) {
				self->renderImpl(n);
			}
			else if constexpr (requires { renderImpl(*self, n); }) {
				renderImpl(*self, n);                // ADL 自由函数派发
			}
			else {
				detail::renderChildrenOnly(n);
			}
		}

		// Qt 式尺寸协议：sizeHint / minimumSizeHint / policy / heightForWidth
		// member → ADL → 默认 三级回退；默认 policy = Preferred，默认 min = {0,0}。
		// 旧即时测量协议（兼容桥：旧控件仍提供 measureImpl；新布局不再调用，仅 Node::measure 使用）
		static Size measure(Node& n, const Size& avail) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->measureImpl(n, avail); }) {
				return self->measureImpl(n, avail);
			}
			else if constexpr (requires { measureImpl(*self, n, avail); }) {
				return measureImpl(*self, n, avail);
			}
			else {
				return Size{avail.w, 0.0f};
			}
		}

		static Size sizeHint(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->sizeHintImpl(n); }) {
				return self->sizeHintImpl(n);
			}
			else if constexpr (requires { sizeHintImpl(*self, n); }) {
				return sizeHintImpl(*self, n);
			}
			else if constexpr (requires { self->measureImpl(n, Size{0.0f, 0.0f}); }) {   // 旧协议 fallback：measure({0,0})
				return self->measureImpl(n, Size{0.0f, 0.0f});
			}
			else if constexpr (requires { measureImpl(*self, n, Size{0.0f, 0.0f}); }) {
				return measureImpl(*self, n, Size{0.0f, 0.0f});
			}
			else {
				return Size{0.0f, 0.0f};
			}
		}
		static Size minimumSizeHint(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->minimumSizeHintImpl(n); }) {
				return self->minimumSizeHintImpl(n);
			}
			else if constexpr (requires { minimumSizeHintImpl(*self, n); }) {
				return minimumSizeHintImpl(*self, n);
			}
			else {
				return Size{0.0f, 0.0f};
			}
		}
		static Policy policy(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->policyImpl(n); }) {
				return self->policyImpl(n);
			}
			else if constexpr (requires { policyImpl(*self, n); }) {
				return policyImpl(*self, n);
			}
			else {
				return Policy{};   // Preferred/Preferred
			}
		}
		static bool hasHFW(Node& n) {
			auto* self = static_cast<T*>(n.owner);
			return requires { self->heightForWidthImpl(n, 0.0f); } || requires { heightForWidthImpl(*self, n, 0.0f); };
		}
		static float heightForWidth(Node& n, float w) {
			auto* self = static_cast<T*>(n.owner);
			if constexpr (requires { self->heightForWidthImpl(n, w); }) {
				return self->heightForWidthImpl(n, w);
			}
			else if constexpr (requires { heightForWidthImpl(*self, n, w); }) {
				return heightForWidthImpl(*self, n, w);
			}
			else {
				return -1.0f;   // 无 HFW
			}
		}

		static void arrange(Node& n, const Rect& r) {
			if constexpr (requires { static_cast<T*>(n.owner)->arrangeImpl(n, r); }) {
				static_cast<T*>(n.owner)->arrangeImpl(n, r);
			}
			else if constexpr (requires { arrangeImpl(*static_cast<T*>(n.owner), n, r); }) {
				arrangeImpl(*static_cast<T*>(n.owner), n, r);
			}
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
		m_SizeHint = &ControlTraits<T>::sizeHint;
		m_MinHint  = &ControlTraits<T>::minimumSizeHint;
		m_Policy   = &ControlTraits<T>::policy;
		m_HasHFW   = &ControlTraits<T>::hasHFW;
		m_HFW      = &ControlTraits<T>::heightForWidth;
	}

	inline void Node::draw() {
		flushPending();
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
		// 布局-渲染几何桥：本节点若直接提交 item（原子控件/FnRender），
		// 按分配的 bounds.w 设置 item 宽——输入类控件真实填满格子而非默认宽。
		// 容器节点无害（子节点 draw 时会再次覆盖为各自宽度）。
		if(bounds.w > 0.0f) ImGui::SetNextItemWidth(bounds.w);
		if(m_Render) m_Render(*this);
		else detail::renderChildrenOnly(*this);
		if(!enabled) ImGui::EndDisabled();
		ImGui::PopID();
	}

	// ===== Qt 式尺寸查询（布局求解器用；自动应用 overrides 钳制）=====
	inline Size Node::hint() { return m_SizeHint ? m_SizeHint(*this) : Size{0.0f, 0.0f}; }
	inline Size Node::minHint() { return m_MinHint ? m_MinHint(*this) : Size{0.0f, 0.0f}; }
	inline Policy Node::getPolicy() { return m_Policy ? m_Policy(*this) : Policy{}; }
	inline bool Node::hasHFW() { return m_HasHFW && m_HasHFW(*this); }
	inline float Node::hfw(float w) { return m_HFW ? m_HFW(*this, w) : -1.0f; }

	inline Size Node::measure(const Size& avail) {
		flushPending();
		return m_Measure ? m_Measure(*this, avail) : Size{avail.w, 0.0f};
	}

	inline void Node::arrange(const Rect& r) {
		flushPending();
		bounds = r;
		if(m_Arrange) m_Arrange(*this, r);
	}

	inline void Node::layout(const Rect& host) {
		// flush deferred adds (safe: outside any children iteration)
		if(!m_Pending.empty()) {
			for(Node* pc : m_Pending) if(pc) children.push_back(pc);
			m_Pending.clear();
		}
		++detail::g_RenderDepth;
		const Size desired = hint();   // Qt 式：理想尺寸（hint 内部 fallback 到旧 measure 协议）
		Rect r = host;
		if(!fill) {   // 填充型容器（分格）直接使用 host，其余按内容尺寸
			r.w = desired.w;
			r.h = desired.h;
		}
		arrange(r);
		origin = ImVec2(r.x, r.y);   // 根节点绘制原点 = 布局矩形左上角（当前窗口空间）
		draw();
		--detail::g_RenderDepth;
	}

	inline void Node::compact() {
		children.erase(
			std::remove(children.begin(), children.end(), nullptr),
			children.end());
	}

	inline void Node::flushPending() {
		if(!m_Pending.empty()) {
			for(Node* pc : m_Pending) if(pc) children.push_back(pc);
			m_Pending.clear();
		}
	}

	inline void Node::add(Node& child) {
		child.parent = this;
		if(detail::g_RenderDepth > 0) { m_Pending.push_back(&child); return; }  // deferred during render
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
		for(auto& p : m_Pending) { if(p == &child) { p = nullptr; return; } }
		// 渲染期 add 后立即 remove：节点尚在 pending 队列，摘除（防止 flush 悬垂）
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

	inline Node* Node::findByIdname(const std::string& want) {
		if(idname == want) return this;
		if(detail::g_RenderDepth == 0) compact();   // 渲染期不压缩（避免 erase 在遍历中）
		for(auto* c : children) {
			if(Node* found = c->findByIdname(want)) return found;
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
		// 无参构造：控件数据用默认值（原子控件也常用）；Self 或任一子控件不可默认构造时跳过
		Control() requires std::default_initializable<Self> &&
			(std::default_initializable<Ts> && ...) : self() {
			node.owner = &self;
			node.bind<Self>();
			if constexpr (requires { Self::kLayoutFill; }) node.fill = Self::kLayoutFill;   // 布局类型自动 fill
			// （Qt 装配由 Control::addWidget 转发，宿主 node 显式传参，无需回填）
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
		// Qt 式尺寸覆盖（写 node.overrides；布局求解时优先于 hint/policy）
		// Qt 风格装配转发（Self 提供 addWidget/setSpacing 时可用）
		template<typename W>
		void addWidget(W& w, float stretch = 0.0f) {
			if constexpr (requires { self.addWidget(node, w.node, stretch); }) self.addWidget(node, w.node, stretch);
			else if constexpr (requires { self.addWidget(node, w, stretch); }) self.addWidget(node, w, stretch);   // W 本身就是 Node&
		}
		void setSpacing(float s) { if constexpr(requires { self.setSpacing(s); }) self.setSpacing(s); }
		void setContentsMargins(float x, float y) { if constexpr(requires { self.setContentsMargins(x, y); }) self.setContentsMargins(x, y); }
		void setCols(int c) { if constexpr(requires { self.cols = c; }) self.cols = c; }
		void addStretch(float weight = 1.0f) { if constexpr (requires { self.addStretch(node, weight); }) self.addStretch(node, weight); }
		void setFixedWidth(float w) { node.overrides.fixedW = w; }
		void setFixedHeight(float h) { node.overrides.fixedH = h; }
		void setFixedSize(float w, float h) { node.overrides.fixedW = w; node.overrides.fixedH = h; }
		void setPrefSize(float w, float h) { node.overrides.prefW = w; node.overrides.prefH = h; }
		void setMinSize(float w, float h) { node.overrides.minW = w; node.overrides.minH = h; }
		void setMaxSize(float w, float h) { node.overrides.maxW = w; node.overrides.maxH = h; }
		Node* findByIdname(const std::string& i) { return node.findByIdname(i); }
	};

}





