#include <gtest/gtest.h>

#include <Mainwindow.h>
#include <imgui_mk/window.h>

// 窗口层 · 视图与逻辑都走 setCentralWidget 的两种形态：
//   setCentralWidget(root)                    静态树直挂（窗口每帧自动布局）
//   setCentralWidget([](const Rect& r){...})  回调式（回调里先跑逻辑再 layout）
// 本文件只做编译期实例化，不创建真实 GLFW 窗口（传 nullptr 提前返回）。

using namespace Mk::wg2;

namespace {

struct Probe {
	static constexpr auto id = Id("probe");
	WidgetState<> state;
	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {}
};

void staticTreeCheck(Mk::window* w, Probe* root) {
	if (!w || !root) return;
	w->setCentralWidget(*root);
}

void callbackCheck(Mk::window* w) {
	if (!w) return;
	w->setCentralWidget([](const Rect&) {});
}

void mainWindowCheck(Mk::MainWindow<Mk::window>* m) {
	if (!m) return;
	m->setCentralWidget([](const Rect&) {});
}

} // namespace

TEST(Wg2Window, CentralWidgetAcceptsTreeOrLayoutCallback) {
	staticTreeCheck(nullptr, nullptr);
	callbackCheck(nullptr);
	mainWindowCheck(nullptr);
	SUCCEED();
}
