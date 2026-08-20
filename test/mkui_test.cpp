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

// ============ 引擎共享状态注册表（类型键多槽）============

namespace {
struct StateA { int value = 0; };
struct StateB { double value = 0.0; };
struct StateC {};   // 未注册类型（抛异常用）
}

TEST_F(MkuiEngineTest, RegisterAndGetStateSharedInstance) {
	auto& eng = Mk::mkui<FakeWindow>::RegistrationWindow();
	auto s = std::make_shared<StateA>();
	eng.registerState(s);
	EXPECT_EQ(eng.state<StateA>().value, 0);
	eng.state<StateA>().value = 7;   // 经引擎写
	EXPECT_EQ(s->value, 7);          // 源实例联动（同一对象）
}

TEST_F(MkuiEngineTest, StateIsolatedByType) {
	auto& eng = Mk::mkui<FakeWindow>::RegistrationWindow();
	eng.registerState(std::make_shared<StateB>());
	eng.state<StateB>().value = 2.5;
	EXPECT_EQ(eng.state<StateA>().value, 7);         // 与前例 StateA 互不干扰
	EXPECT_DOUBLE_EQ(eng.state<StateB>().value, 2.5);
}

TEST_F(MkuiEngineTest, MissingStateThrows) {
	auto& eng = Mk::mkui<FakeWindow>::RegistrationWindow();
	EXPECT_THROW(eng.state<StateC>(), std::runtime_error);
}

TEST_F(MkuiEngineTest, BroadcastGuardedForMinimalImpl) {
	// 最小契约 Impl（无 requestTheme/requestRescale/m_Handle）：广播经 requires 守卫
	// 为空操作、fontSize() 恒 0——编译期验证不实例化错误路径
	auto& eng = Mk::mkui<FakeWindow>::RegistrationWindow();
	auto impl = std::make_shared<FakeWindow::Impl>();
	impl->maxRenders = 1;
	eng.Registration(impl);
	eng.applyThemeAll(12345);
	eng.rescaleAll(24.0f);
	EXPECT_FLOAT_EQ(eng.fontSize(), 0.0f);
	Mk::mkui<FakeWindow>::run();   // 1 帧后关闭并清空窗口表
}