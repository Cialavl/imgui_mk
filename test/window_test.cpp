#include <gtest/gtest.h>
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <chrono>
#include <thread>

// 真实窗口（GLFW + OpenGL）：窗口会在测试期间短暂出现，每个测试结束前必须显式关闭，
// 否则窗口注册进全局引擎后无人清理（引擎持有 shared_ptr，单靠析构不会销毁窗口）

TEST(MkWindow, CreatesHandle) {
	Mk::window w("test", 320, 240);
	EXPECT_NE(w.getHandle(), nullptr);
	w.close(); // 真正关闭：销毁 GLFW 窗口与 ImGui 上下文
	EXPECT_EQ(w.getHandle(), nullptr);
}

// 句柄拷贝共享同一窗口状态（shared_ptr<Impl>），关闭后所有共享句柄同步失效
TEST(MkWindow, CopySharesImpl) {
	Mk::window w1("test1", 320, 240);
	Mk::window w2 = w1;
	EXPECT_EQ(w1.getHandle(), w2.getHandle());
	w1.close();
	EXPECT_EQ(w1.getHandle(), nullptr);
	EXPECT_EQ(w2.getHandle(), nullptr);
}

// 构造即注册进引擎：裸临时对象与命名局部对象行为一致（引擎与调用方共享所有权），
// 析构不影响引擎持有的窗口，因此测试必须显式 close() 清理
TEST(MkWindow, BareTemporaryRegisters) {
	Mk::window w("test", 320, 240);
	EXPECT_NE(w.getHandle(), nullptr);
	w.close();
}

// 注入 UI 回调不崩溃，关闭时回调随 ImGui 上下文一并清理
TEST(MkWindow, SetUserUI) {
	Mk::window w("test", 320, 240);
	bool called = false;
	w.setUserUI([&called]() { called = true; });
	SUCCEED();
	w.close();
}

// 引擎逐个管理窗口：关闭一个窗口不影响另一个，全部关闭后 run() 退出。
// 计时线程分别在第 400ms / 800ms 设置两个窗口的关闭请求。
TEST(MkWindow, RunClosesWindowsIndividually) {
	Mk::window w1("test1", 320, 240);
	Mk::window w2("test2", 320, 240);

	std::thread closer([&w1, &w2] {
		std::this_thread::sleep_for(std::chrono::milliseconds(400));
		glfwSetWindowShouldClose(w1.getHandle(), true); // 只请求关闭 w1
		std::this_thread::sleep_for(std::chrono::milliseconds(400));
		glfwSetWindowShouldClose(w2.getHandle(), true);
	});

	auto start = std::chrono::steady_clock::now();
	Mk::mkui<Mk::window>::run();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - start).count();
	closer.join();

	// w1 关闭后 w2 继续运行（run() 没有一起退出），w2 关闭后才退出
	EXPECT_GE(elapsed, 700);
	EXPECT_LT(elapsed, 5000);
	EXPECT_EQ(w1.getHandle(), nullptr); // 关闭即销毁
	EXPECT_EQ(w2.getHandle(), nullptr);
}
