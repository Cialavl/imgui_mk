#pragma once
// 控件树遍历（Qt 式两段式初始化）：
//   setupTree：自顶向下 setup(ctx) —— 各组件按上下文把自己配好（含登记子窗口/服务）
//   readyTree：全部 setup 完成后 ready(ctx) —— 处理跨组件依赖（如状态栏取页标题）
// 容器带 children 元组时自动递归；组合组件在 setup/ready 里调用本函数继续往下即可。
#include <tuple>

namespace Mk::wg2 {

template <class W, class Ctx>
void setupTree(W& w, Ctx& ctx) {
	if constexpr (requires { w.setup(ctx); }) w.setup(ctx);
	if constexpr (requires { w.children; }) {
		std::apply([&](auto&... child) { (setupTree(child, ctx), ...); }, w.children);
	}
}

template <class W, class Ctx>
void readyTree(W& w, Ctx& ctx) {
	if constexpr (requires { w.ready(ctx); }) w.ready(ctx);
	if constexpr (requires { w.children; }) {
		std::apply([&](auto&... child) { (readyTree(child, ctx), ...); }, w.children);
	}
}

} // namespace Mk::wg2
