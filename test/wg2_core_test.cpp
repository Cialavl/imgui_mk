#include <gtest/gtest.h>

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
