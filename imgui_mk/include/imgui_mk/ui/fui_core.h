#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/ui/widget.h>
#include <algorithm>
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

	// 设计基准字号：控件尺寸写「按 18px 调好的设计像素」，运行时随当前字号等比缩放——
	// 文字与控件比例恒定，字号升档/DPI 变化不再溢出或比例失衡
	inline constexpr float kDesignFontBase = 18.0f;

	// 设计像素 → 实际像素（× 当前字号/18）
	inline float em(float designPx) {
		return designPx * ImGui::GetFontSize() / kDesignFontBase;
	}
	// ImVec2 版
	inline ImVec2 em2(float designW, float designH) {
		const float k = ImGui::GetFontSize() / kDesignFontBase;
		return ImVec2(designW * k, designH * k);
	}
	// 弹性分格（hsplit/vsplit 的格子）：fixed 固定宽；stretch 按权重瓜分剩余。
	// 格子内容以 BeginChild 承载（独立 id，继承主题样式）。
	// fixed 格三层约束（flexbox 语义）：min=文字可读下限 / preferred=设计像素 /
	// maxFrac=占容器上限——空间够按 preferred（比例美观），不足向 min 收缩
	//（文字永远可读），极窄由格内 ellipsis 兜底
	template<typename T>
	struct SplitCell {
		bool stretch;     // false=固定，true=弹性
		float size;       // fixed: 设计像素（em 缩放）；stretch: 权重（相对其它 stretch 格）
		bool absolute;    // fixed 的尺寸语义：false=设计像素（随字号缩放），true=真像素
		std::string id;   // 子窗口 id（需在分格树内唯一）
		T child;
		float minPx = 0.0f;    // 下限（设计像素，em 缩放）；默认 0=可完全收缩
		float maxFrac = 1.0f;  // 占容器可用宽的上限

		// 链式约束（工厂返回的临时对象上链式调用）：fixed("sb", 250, x).min(150).atMost(0.35f)
		SplitCell&& min(float designPx) && { minPx = designPx; return std::move(*this); }
		SplitCell&& atMost(float frac) && { maxFrac = frac; return std::move(*this); }
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

	// 单行省略文本：超出可用宽时二分截断加“…”（配 tooltip 可看全文）
	struct LabelEllipsis {
		std::string text;
	};

	// 最小宽度修饰：子视图内下一个 item 撑到设计宽（em 缩放）；内容更宽时不变
	template<typename T>
	struct WithMinWidth {
		float minDesignPx;
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
inline void draw(const LabelEllipsis& w);
template<typename T> void draw(const WithMinWidth<T>& w);

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
		// 渲染一个分格：track 为外层算好的实际像素（约束/收缩/em 都在外层完成，
		// 账目同源），水平格传宽、垂直格传高，另一维填满
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

		// fixed 格理想值：preferred（em 缩放）先按 maxFrac 封顶、再由 min 托底——
		// min 是硬下限（CSS flexbox 同语义：min-width 优先于 max-width），
		// 容器再窄也不低于文字可读底线
		template<typename T>
		float clampCell(const SplitCell<T>& cell, float avail) {
			const float pref = cell.absolute ? cell.size : em(cell.size);
			const float mn = em(cell.minPx);
			const float mx = cell.maxFrac * avail;
			const float v = pref > mx ? mx : pref;
			return v < mn ? mn : v;
		}

		// 截断文本至指定显示宽（含 "..."）；不超宽则原样返回（LabelEllipsis / Sidebar 共用）
		inline std::string ellipsize(const std::string& text, float avail) {
			const ImVec2 full = ImGui::CalcTextSize(text.c_str());
			if(full.x <= avail) return text;
			const float ell = ImGui::CalcTextSize("...").x;
			const float budget = avail - ell;
			size_t lo = 0, hi = text.size();   // lo 可行、hi 不可行
			while(lo < hi) {
				const size_t mid = (lo + hi + 1) / 2;
				if(ImGui::CalcTextSize(text.c_str(), text.c_str() + mid).x <= budget) lo = mid;
				else hi = mid - 1;
			}
			return text.substr(0, lo) + "...";
		}
	}

	// 水平弹性分格：fixed 格先按 clamp(em(pref), em(min), maxFrac×avail) 得理想值，
	// 超出容器时按 flexbox shrink 语义向 min 均匀收缩（k=保留比例）；
	// stretch 格按权重瓜分剩余（floor 0）。全程 em 后的实际像素记账——修复
	// 旧版 fixedSum 用设计值、drawCell 再 em 的双重缩放账目错位（stretch 超发越界）
	template<typename... Ts>
	void draw(const HSplit<Ts...>& w) {
		const float avail = ImGui::GetContentRegionAvail().x;
		const float gap = ImGui::GetStyle().ItemSpacing.x;
		std::size_t nCells = 0;
		float weightSum = 0.0f;

		float desiredSum = 0.0f, minSum = 0.0f;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				++nCells;
				if(c.stretch) { weightSum += c.size; return; }
				desiredSum += detail::clampCell(c, avail);
				minSum += em(c.minPx);
			};
			(one(cs), ...);
		}, w.cells);

		const float gaps = (float)(nCells > 1 ? nCells - 1 : 0) * gap;
		const float need = desiredSum + gaps - avail;            // 超出量
		const float shrinkable = desiredSum - minSum;            // 可收缩总量
		const bool shrink = need > 0.0f && shrinkable > 0.0f;
		const float k = shrink ? std::max(0.0f, 1.0f - need / shrinkable) : 1.0f;
		const float finalFixed = shrink ? minSum + shrinkable * k : desiredSum;
		const float stretchTotal = avail - finalFixed - gaps;

		std::size_t idx = 0;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				if(idx > 0) ImGui::SameLine(0.0f, gap);
				float track;
				if(c.stretch) {
					track = (weightSum > 0.0f && stretchTotal > 0.0f)
						? stretchTotal * (c.size / weightSum) : 0.0f;
				}
				else {
					const float des = detail::clampCell(c, avail);
					const float mn = em(c.minPx);
					track = shrink ? std::max(mn, mn + (des - mn) * k) : des;
				}
				detail::drawCell(c, true, track);
				++idx;
			};
			(one(cs), ...);
		}, w.cells);
	}

	// 垂直弹性分格：与 HSplit 同构（min/preferred/maxFrac + shrink），方向为上到下
	template<typename... Ts>
	void draw(const VSplit<Ts...>& w) {
		const float avail = ImGui::GetContentRegionAvail().y;
		const float gap = ImGui::GetStyle().ItemSpacing.y;
		std::size_t nCells = 0;
		float weightSum = 0.0f;

		float desiredSum = 0.0f, minSum = 0.0f;
		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				++nCells;
				if(c.stretch) { weightSum += c.size; return; }
				desiredSum += detail::clampCell(c, avail);
				minSum += em(c.minPx);
			};
			(one(cs), ...);
		}, w.cells);

		const float gaps = (float)(nCells > 1 ? nCells - 1 : 0) * gap;
		const float need = desiredSum + gaps - avail;
		const float shrinkable = desiredSum - minSum;
		const bool shrink = need > 0.0f && shrinkable > 0.0f;
		const float k = shrink ? std::max(0.0f, 1.0f - need / shrinkable) : 1.0f;
		const float finalFixed = shrink ? minSum + shrinkable * k : desiredSum;
		const float stretchTotal = avail - finalFixed - gaps;

		std::apply([&](const auto&... cs) {
			const auto one = [&](const auto& c) {
				float track;
				if(c.stretch) {
					track = (weightSum > 0.0f && stretchTotal > 0.0f)
						? stretchTotal * (c.size / weightSum) : 0.0f;
				}
				else {
					const float des = detail::clampCell(c, avail);
					const float mn = em(c.minPx);
					track = shrink ? std::max(mn, mn + (des - mn) * k) : des;
				}
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

	inline void draw(const LabelEllipsis& w) {
		ImGui::TextUnformatted(detail::ellipsize(w.text,
			ImGui::GetContentRegionAvail().x).c_str());
	}

	template<typename T>
	void draw(const WithMinWidth<T>& w) {
		// 真最小宽：与当前默认计算宽取 max（避免把本来更宽的 item 压窄）
		ImGui::PushItemWidth(std::max(ImGui::CalcItemWidth(), em(w.minDesignPx)));
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
	// 固定格：size 为 18px 基准的设计像素（随字号等比缩放，见 em()）；id 需在同一分格树内唯一
	template<typename T>
	auto fixed(std::string id, float size, T child) {
		return SplitCell<std::decay_t<T>>{false, size, false, std::move(id), std::move(child)};
	}
	// 固定格（真像素）：不随字号缩放，罕用逃生舱（如对齐外部资源尺寸）
	template<typename T>
	auto fixedPx(std::string id, float size, T child) {
		return SplitCell<std::decay_t<T>>{false, size, true, std::move(id), std::move(child)};
	}
	// 弹性格：weight 相对权重（同树内比较），瓜分 fixed 与间距之外的剩余空间
	template<typename T>
	auto stretch(std::string id, float weight, T child) {
		return SplitCell<std::decay_t<T>>{true, weight, false, std::move(id), std::move(child)};
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
	inline auto labelEllipsis(std::string text) { return LabelEllipsis{std::move(text)}; }
	template<typename T>
	auto withMinWidth(float minDesignPx, T child) {
		return WithMinWidth<std::decay_t<T>>{minDesignPx, std::move(child)};
	}

	// 子窗口尺寸语义化别名：fill()=两维填满；fillW(h)=填宽定高；fillH(w)=填高定宽
	inline ImVec2 fill() { return ImVec2(0.0f, 0.0f); }
	inline ImVec2 fillW(float h) { return ImVec2(0.0f, h); }
	inline ImVec2 fillH(float w) { return ImVec2(w, 0.0f); }

	inline auto dyn(ui::Widget& w) { return Dyn{w}; }

	template<typename F>
	auto asWidget(F maker) { return WidgetAdapter<std::decay_t<F>>(std::move(maker)); }

}