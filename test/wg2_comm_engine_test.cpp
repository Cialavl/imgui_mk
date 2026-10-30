#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <imgui_mk/mkui.h>
#include <memory>
#include <stdexcept>

// 第四层 · 引擎层：单例与类型隔离、run() 调度、共享状态注册表、跨窗口广播、主题工具

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;
using wg2test::LoopWindow;
using wg2test::BroadcastWindow;
using wg2test::LoopImpl;
using wg2test::SharedState;
using wg2test::OtherState;
using wg2test::BroadcastImpl;
using wg2test::MinimalWindow;

TEST(Wg2Comm, RegistrationWindowIsSingleton) {
	EXPECT_EQ(&Mk::mkui<LoopWindow>::RegistrationWindow(),
		&Mk::mkui<LoopWindow>::RegistrationWindow());
}

TEST(Wg2Comm, DifferentWindowTypesHaveSeparateEngines) {
	EXPECT_NE(static_cast<const void*>(&Mk::mkui<LoopWindow>::RegistrationWindow()),
		static_cast<const void*>(&Mk::mkui<BroadcastWindow>::RegistrationWindow()));
}

TEST(Wg2Comm, RunExitsWhenAllWindowsClosed) {
	ASSERT_TRUE(glfwInit());   // run() 内部调用 glfwPollEvents()

	auto impl = std::make_shared<LoopImpl>();   // maxRenders = 0 → 立刻要求关闭
	Mk::mkui<LoopWindow>::RegistrationWindow().Registration(impl);
	Mk::mkui<LoopWindow>::run();
	EXPECT_TRUE(impl->closed);
	EXPECT_EQ(impl->renderCount, 0);
}

TEST(Wg2Comm, RunRendersUntilClosed) {
	ASSERT_TRUE(glfwInit());

	auto impl = std::make_shared<LoopImpl>();
	impl->maxRenders = 3;
	Mk::mkui<LoopWindow>::RegistrationWindow().Registration(impl);
	Mk::mkui<LoopWindow>::run();
	EXPECT_EQ(impl->renderCount, 3);
}

TEST(Wg2Comm, StateRegistrySharesTypedState) {
	auto& engine = Mk::mkui<LoopWindow>::RegistrationWindow();

	EXPECT_THROW(engine.state<SharedState>(), std::runtime_error);   // 未注册即取用 → fail-fast

	auto shared = std::make_shared<SharedState>();
	engine.registerState(shared);
	engine.state<SharedState>().counter = 9;
	EXPECT_EQ(shared->counter, 9);        // 引擎与调用方是同一实例
}

namespace {

// 本文件私有的状态类型：保证「按类型隔离」用例不受其它用例注册过的类型影响
struct IsolatedState {
	int value = 0;
};

} // namespace

TEST(Wg2Comm, StateIsolatedByType) {
	auto& engine = Mk::mkui<LoopWindow>::RegistrationWindow();
	engine.registerState(std::make_shared<OtherState>());
	engine.state<OtherState>().value = 2.5;

	engine.registerState(std::make_shared<IsolatedState>());
	engine.state<IsolatedState>().value = 3;

	EXPECT_DOUBLE_EQ(engine.state<OtherState>().value, 2.5);
	EXPECT_EQ(engine.state<IsolatedState>().value, 3);   // 两种类型互不覆盖
}

TEST(Wg2Comm, BroadcastReachesEveryRegisteredWindow) {
	auto& engine = Mk::mkui<BroadcastWindow>::RegistrationWindow();
	auto w1 = std::make_shared<BroadcastImpl>();
	auto w2 = std::make_shared<BroadcastImpl>();
	engine.Registration(w1);
	engine.Registration(w2);

	engine.applyThemeAll(Theme::light());
	EXPECT_EQ(w1->themes, 1);
	EXPECT_EQ(w2->themes, 1);

	engine.rescaleAll(27.0f);
	EXPECT_FLOAT_EQ(w1->lastScale, 27.0f);
	EXPECT_FLOAT_EQ(w2->lastScale, 27.0f);

	// 假 Impl 没有 m_Handle/m_LastBakedSize → 编译期走 0 分支
	EXPECT_FLOAT_EQ(engine.fontSize(), 0.0f);
}

TEST(Wg2Comm, BroadcastIsCompileTimeNoOpForMinimalImpl) {
	auto& engine = Mk::mkui<MinimalWindow>::RegistrationWindow();
	engine.applyThemeAll(Theme::dark());     // requires 不满足 → 编译期 no-op
	engine.rescaleAll(18.0f);
	EXPECT_FLOAT_EQ(engine.fontSize(), 0.0f);
}

TEST(Wg2Comm, ThemeDefaultsAndMonitorScaling) {
	const Theme saved = defaultTheme();

	Theme custom = Theme::dark().withAccent(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)).withFontSize(20.0f);
	setDefaultTheme(custom);
	EXPECT_FLOAT_EQ(defaultTheme().fontSize, 20.0f);
	EXPECT_FLOAT_EQ(defaultTheme().accent.x, 1.0f);

	const Theme scaled = scaledForMonitor(custom);
	EXPECT_NEAR(scaled.fontSize, 20.0f * dpiScale(), 1e-3f);   // 只放大字号，其余几何随字号联动

	setDefaultTheme(saved);   // 还原，避免影响其它用例
}
