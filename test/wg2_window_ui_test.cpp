#include <gtest/gtest.h>

#include <Mainwindow.h>
#include <imgui_mk/window.h>

// 窗口层 · 视图挂载只有一种形态：setCentralWidget(root)，窗口每帧自动布局。
// 每帧逻辑不通过窗口回调暴露，而是用 wg2::Update<Id> 控件放进树里。
// 本文件只做编译期实例化，不创建真实 GLFW 窗口（传 nullptr 提前返回）。

using namespace Mk::wg2;

namespace {

struct Probe {
	static constexpr auto id = Id("probe");
	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {}
};

void windowCheck(Mk::window* w, Probe* root) {
	if (!w || !root) return;
	w->setCentralWidget(*root);
}

void mainWindowCheck(Mk::MainWindow<Mk::window>* m, Probe* root) {
	if (!m || !root) return;
	m->setCentralWidget(*root);
}

} // namespace

TEST(Wg2Window, CentralWidgetTakesStaticTreeOnly) {
	windowCheck(nullptr, nullptr);
	mainWindowCheck(nullptr, nullptr);
	SUCCEED();
}
