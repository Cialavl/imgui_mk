#include <gtest/gtest.h>

#include <imgui_mk/ui/wg2/mount.h>
#include "wg2_test_harness.h"

// 第 0 层 · 基础类型：编译期字符串 ID、字符串路径访问、尺寸策略

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Core, FixedStringLiteralCreatesComparableId) {
	constexpr auto a = Id("save");
	constexpr auto b = Id("save");
	constexpr auto c = Id("cancel");
	EXPECT_EQ(a, b);
	EXPECT_NE(a, c);
	EXPECT_EQ(a.size, 4u);
	EXPECT_EQ(c.size, 6u);
	// 括号包住模板实参：宏按逗号切参数，模板实参里的逗号必须躲进括号
	EXPECT_TRUE((fixed_equal<Id("save"), Id("save")>()));
	EXPECT_FALSE((fixed_equal<Id("save"), Id("cancel")>()));
}

TEST(Wg2Core, StringPathReplacesNumericChildIndex) {
	VBox<Id("root"),
		HBox<Id("bar"), wg2test::RectProbe<Id("save")>, wg2test::RectProbe<Id("cancel")>>,
		wg2test::RectProbe<Id("status")>>
		root;

	root.at<Id("bar"), Id("save")>().w = 11.0f;
	root.at<Id("bar"), Id("cancel")>().w = 22.0f;
	root.at<Id("status")>().w = 33.0f;

	// 字符串路径与数字下标等价
	EXPECT_FLOAT_EQ(root.child<0>().child<0>().w, 11.0f);
	EXPECT_FLOAT_EQ(root.child<0>().child<1>().w, 22.0f);
	EXPECT_FLOAT_EQ(root.child<1>().w, 33.0f);
}

TEST(Wg2Core, PolicyKindsGateGrowShrinkAndExpand) {
	const Policy preferred{};
	EXPECT_TRUE(preferred.canGrow(Policy::Preferred));
	EXPECT_TRUE(preferred.canShrink(Policy::Preferred));
	EXPECT_FALSE(preferred.wantsExpand(Policy::Preferred));

	const Policy fixed{Policy::Fixed, Policy::Fixed};
	EXPECT_FALSE(fixed.canGrow(Policy::Fixed));
	EXPECT_FALSE(fixed.canShrink(Policy::Fixed));

	const Policy expanding{Policy::Preferred, Policy::Expanding};
	EXPECT_TRUE(expanding.wantsExpand(Policy::Expanding));
}

namespace {

// 不带 state 成员的控件：状态由引擎按控件地址登记（widgetChrome）
struct StatelessProbe {
	static constexpr auto id = Id("stateless");
	Size hint(const Size&) const { return Size{10.0f, 10.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {}
};

} // namespace

TEST(Wg2Core, StatelessWidgetGetsEngineChrome) {
	StatelessProbe probe;
	EXPECT_TRUE(widgetChrome(probe).visible);   // 引擎给默认状态

	setStretch(probe, 2.0f);
	setFixedSize(probe, 40.0f, 24.0f);
	setEnabled(probe, false);
	setVisible(probe, false);

	EXPECT_FALSE(widgetChrome(probe).shown());
	EXPECT_FALSE(widgetChrome(probe).enabled);
	EXPECT_FLOAT_EQ(widgetChrome(probe).stretch, 2.0f);
	EXPECT_FLOAT_EQ(widgetChrome(probe).size.fixedW, 40.0f);
	EXPECT_FLOAT_EQ(widgetChrome(probe).size.fixedH, 24.0f);

	StatelessProbe other;   // 按控件地址登记：实例之间互不影响
	EXPECT_TRUE(widgetChrome(other).visible);
	EXPECT_FLOAT_EQ(widgetChrome(other).stretch, 0.0f);
}

TEST(Wg2Core, LayoutReadsEngineStateForStatelessWidget) {
	Harness h;
	HBox<Id("row"), StatelessProbe> row;
	auto& probe = row.at<Id("stateless")>();

	float shownW = 0.0f, hiddenW = 0.0f;
	h.inFrame([&] { shownW = row.hint(Size{200.0f, 100.0f}).w; });
	setVisible(probe, false);
	h.inFrame([&] { hiddenW = row.hint(Size{200.0f, 100.0f}).w; });

	EXPECT_FLOAT_EQ(shownW - hiddenW, 10.0f);   // 隐藏后不再占位（探测控件的宽度）
}

TEST(Wg2Core, AttachRecordsParent) {
	HBox<Id("row"), StatelessProbe> row;
	int host = 0;   // 扮演主窗口/父组件

	Mk::wg2::attach(row, host);

	EXPECT_EQ(parentOf(row), static_cast<const void*>(&host));   // root 的父 = 宿主
}

TEST(Wg2Core, FontsFallBackToContextDefault) {
	Harness h;
	EXPECT_EQ(defaultFont(), ImGui::GetIO().FontDefault);   // 未注册 → 上下文默认（可能为空）
	EXPECT_EQ(monoFont(), defaultFont());                    // 等宽字体回退默认界面字体
}

TEST(Wg2Core, MonoFontProviderOverridesDefault) {
	Harness h;
	ImFont* base = ImGui::GetIO().Fonts->Fonts[0];

	setMonoFontProvider([] { return static_cast<ImFont*>(nullptr); });
	EXPECT_EQ(monoFont(), defaultFont());   // provider 还没烘焙出句柄 → 回退

	setMonoFontProvider([base] { return base; });
	EXPECT_EQ(monoFont(), base);            // provider 给出字体就用它

	clearFonts();
	EXPECT_EQ(monoFont(), defaultFont());
}

TEST(Wg2Core, DefaultTextureIsOfflineSafeAndOverridable) {
	Harness h;   // 无渲染后端
	EXPECT_EQ(defaultTexture(), ImTextureID{});   // 不触碰 GL，返回空
	EXPECT_EQ(textureOr(0), ImTextureID{});

	const ImTextureID mine = (ImTextureID)(intptr_t)7;
	setDefaultTexture(mine);                       // 应用可覆盖引擎默认值
	EXPECT_EQ(textureOr(0), mine);
	EXPECT_EQ(textureOr((ImTextureID)(intptr_t)3), (ImTextureID)(intptr_t)3);   // 自己设了用自己的

	clearTextures();
	EXPECT_EQ(defaultTexture(), ImTextureID{});
}
