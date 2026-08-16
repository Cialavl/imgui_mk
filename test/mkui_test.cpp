#include <gtest/gtest.h>
#include <imgui_mk/mkui.h>

// 测试用假窗口：不创建真实 GLFW 窗口，只计数 render 次数
// Impl 契约：render() / shouldClose() / close()（引擎在窗口关闭时调用 close() 清理资源）
struct FakeWindow {
	struct Impl {
		int renderCount = 0;
		int maxRenders = 0; // render 次数达到后关闭；0 表示立即关闭
		bool closed = false;
		void render() { ++renderCount; }
		bool shouldClose() const { return closed || maxRenders == 0 || renderCount >= maxRenders; }
		void close() { closed = true; }
	};
};

struct FakeWindow2 {
	struct Impl {
		void render() {}
		bool shouldClose() const { return true; }
		void close() {}
	};
};

class MkuiEngineTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() {
		ASSERT_TRUE(glfwInit()); // run() 内部调用 glfwPollEvents()
	}
};

TEST_F(MkuiEngineTest, RegistrationWindowIsSingleton) {
	auto& a = Mk::mkui<FakeWindow>::RegistrationWindow();
	auto& b = Mk::mkui<FakeWindow>::RegistrationWindow();
	EXPECT_EQ(&a, &b);
}

TEST_F(MkuiEngineTest, DifferentWindowTypesHaveSeparateEngines) {
	EXPECT_NE(static_cast<const void*>(&Mk::mkui<FakeWindow>::RegistrationWindow()),
		static_cast<const void*>(&Mk::mkui<FakeWindow2>::RegistrationWindow()));
}

// 注册的窗口立即关闭（shouldClose 恒真）→ run() 立刻退出
TEST_F(MkuiEngineTest, RunExitsWhenAllWindowsClosed) {
	auto impl = std::make_shared<FakeWindow::Impl>();
	Mk::mkui<FakeWindow>::RegistrationWindow().Registration(impl);
	Mk::mkui<FakeWindow>::run();
	EXPECT_TRUE(impl->shouldClose());
}

// 窗口在第 3 次 render 后关闭 → run() 恰好渲染 3 帧后退出并清空注册表
TEST_F(MkuiEngineTest, RunRendersUntilClosed) {
	auto impl = std::make_shared<FakeWindow::Impl>();
	impl->maxRenders = 3;
	Mk::mkui<FakeWindow>::RegistrationWindow().Registration(impl);
	Mk::mkui<FakeWindow>::run();
	EXPECT_EQ(impl->renderCount, 3);
}