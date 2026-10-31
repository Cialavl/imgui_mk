#include <gtest/gtest.h>

#include <Mainwindow.h>
#include <imgui_mk/window.h>

// 窗口层 · 职责划分：
//   setUI   = 每帧逻辑回调（() / (window&) / (ImGuiIO&)）
//   setCentralWidget = 视图挂载（静态树 或 (const Rect&) 回调）
// 本文件只做编译期实例化，不创建真实 GLFW 窗口（传 nullptr 提前返回）。

namespace {

void setUICheck(Mk::window* w) {
	if (!w) return;
	w->setUI([] {});                                        // 普通每帧 UI
	w->setUI([](Mk::window&) {});                           // 窗口回调
	w->setUI([](ImGuiIO& io) { (void)io.DisplaySize; });    // 框架注入 io
}

void centralWidgetCheck(Mk::window* w) {
	if (!w) return;
	w->setCentralWidget([](const Mk::wg2::Rect&) {});       // 回调式布局
}

void mainWindowCheck(Mk::MainWindow<Mk::window>* m) {
	if (!m) return;
	m->setUI([](ImGuiIO& io) { (void)io.DeltaTime; });      // MainWindow 转发同样支持
	m->setUI([](Mk::window&) {});
}

} // namespace

TEST(Wg2Window, SetUIAcceptsPerFrameCallbacksOnly) {
	setUICheck(nullptr);        // 只实例化模板，不构造窗口
	centralWidgetCheck(nullptr);
	mainWindowCheck(nullptr);
	SUCCEED();
}
