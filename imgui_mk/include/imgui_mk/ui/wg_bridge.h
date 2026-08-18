#pragma once
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/fui.h>
#include <functional>

// Mk::wg 与 Mk::fui 互通桥。
// wg 是 retained 对象树控件（模板 struct 组合 + 类型分发）；
// fui 是每帧值语义视图。桥支持两个方向：
//   1) wgToFuiView(maker)：wg 树里嵌入一段每帧构造的 fui 视图
//   2) fuiToWgNode(fui 绘制函数)：wg 节点渲染时调用自由绘制函数
// 两者都经 Node 的类型擦除行为表接入——无需继承、无需改库。

namespace Mk::wg {

	// —— 方向 1：wg 节点 → fui 视图（每帧重建语义）——
	// 用法：Node n; n.bind<FuiViewImpl<F>>(); n.owner = &view;
	// 或直接经 FuiView 控件组合。
	template<typename F>
	struct FuiViewImpl {
		F make;   // 每帧调用，返回 fui 视图值

		void renderImpl(Node&) {
			Mk::fui::draw(make());
		}
		Size measureImpl(Node&, const Size& avail) {
			return Size{avail.w, 0.0f};   // 高度由 fui 流式布局决定
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	template<typename F>
	using FuiView = Control<FuiViewImpl<F>, DefaultStyle>;

	// —— 方向 2：fui 绘制函数 → wg 节点 ——
	// F: void() 每帧执行（直接调 fui::draw 或裸 ImGui 调用）
	template<typename F>
	struct FnRenderImpl {
		F fn;
		void renderImpl(Node&) { fn(); }
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w, 0.0f}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	template<typename F>
	using FnRender = Control<FnRenderImpl<F>, DefaultStyle>;

}