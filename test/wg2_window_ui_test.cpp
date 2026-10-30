#include <gtest/gtest.h>

#include <Mainwindow.h>
#include <imgui_mk/window.h>

// 窗口层 · setUI 的分派契约：()/(window&)/(const Rect&)/(ImGuiIO&)/StaticWidget 五种回调都能编译并落到正确入口。
// 本文件只做编译期实例化，不创建真实 GLFW 窗口（传 nullptr 提前返回）。

namespace {

void windowDispatchCheck(Mk::window* w) {
	if (!w) return;
	w->setUI([] {});                                  // 普通每帧 UI
	w->setUI([](Mk::window&) {});                     // 窗口回调
	w->setUI([](const Mk::wg2::Rect&) {});            // 中央布局回调
	w->setUI([](ImGuiIO& io) { (void)io.DisplaySize; });   // 框架注入 io
}

void mainWindowDispatchCheck(Mk::MainWindow<Mk::window>* m) {
	if (!m) return;
	m->setUI([](ImGuiIO& io) { (void)io.DeltaTime; });      // MainWindow 转发同样支持
	m->setUI([](const Mk::wg2::Rect&) {});
}

} // namespace

TEST(Wg2Window, SetUIDispatchCompilesForAllCallbackShapes) {
	windowDispatchCheck(nullptr);       // 只实例化模板，不构造窗口
	mainWindowDispatchCheck(nullptr);
	SUCCEED();
}
