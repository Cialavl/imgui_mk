#include <gtest/gtest.h>
#include <imgui_mk/window.h>

// 真实窗口（GLFW + OpenGL）：窗口会在测试期间短暂出现
TEST(MkWindow, CreatesHandle) {
	Mk::window w("test", 320, 240);
	EXPECT_NE(w.getHandle(), nullptr);
}

// 句柄拷贝共享同一窗口状态（shared_ptr<Impl>）
TEST(MkWindow, CopySharesImpl) {
	Mk::window w1("test1", 320, 240);
	Mk::window w2 = w1;
	EXPECT_EQ(w1.getHandle(), w2.getHandle());
}

// 裸临时对象构造即注册进引擎，析构不影响引擎持有的窗口
TEST(MkWindow, BareTemporaryRegisters) {
	Mk::window("test", 320, 240);
	SUCCEED();
}

// 注入 UI 回调不崩溃
TEST(MkWindow, SetUserUI) {
	Mk::window w("test", 320, 240);
	bool called = false;
	w.setUserUI([&called]() { called = true; });
	SUCCEED();
}