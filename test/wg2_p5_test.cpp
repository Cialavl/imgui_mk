#include <gtest/gtest.h>

#include "wg2_test_harness.h"

#include <imgui_mk/ui/wg2/wg2.h>

using namespace Mk::wg2;

namespace {

// 绘制时把当前 ImGui 样式记下来，验证框架的样式作用域
struct StyleProbe {
	static constexpr auto id = Id("styleProbe");
	float rounding = -1.0f;
	float textR = -1.0f;
	float spacingY = -1.0f;

	Size hint(const Size&) const { return Size{10.0f, 10.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {
		rounding = ImGui::GetStyle().FrameRounding;
		textR = ImGui::GetStyleColorVec4(ImGuiCol_Text).x;
		spacingY = ImGui::GetStyle().ItemSpacing.y;
	}
};

// 组合控件：只声明 root（无协议函数）
struct StyleComposite {
	static constexpr auto id = Id("styleComp");
	StyleProbe root;
};

} // namespace

TEST(Wg2P5, StyleOverridesAppliedDuringDrawAndRestored) {
	wg2test::Harness h;
	StyleProbe probe;
	const float baseRounding = ImGui::GetStyle().FrameRounding;
	const float baseSpacing = ImGui::GetStyle().ItemSpacing.y;

	styleOf(probe).rounding = 12.0f;
	styleOf(probe).text = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	styleOf(probe).spacing = ImVec2(3.0f, 4.0f);

	h.layoutFrame(probe, Rect{0.0f, 0.0f, 20.0f, 20.0f});

	EXPECT_FLOAT_EQ(probe.rounding, 12.0f);
	EXPECT_FLOAT_EQ(probe.textR, 1.0f);
	EXPECT_FLOAT_EQ(probe.spacingY, 4.0f);
	EXPECT_FLOAT_EQ(ImGui::GetStyle().FrameRounding, baseRounding);   // 画完恢复
	EXPECT_FLOAT_EQ(ImGui::GetStyle().ItemSpacing.y, baseSpacing);
}

TEST(Wg2P5, StyleOfReturnsSameSlotAndClearWorks) {
	StyleProbe probe;
	styleOf(probe).rounding = 3.0f;
	ASSERT_TRUE(styleOf(probe).rounding.has_value());
	EXPECT_FLOAT_EQ(*styleOf(probe).rounding, 3.0f);

	clearStyle(probe);
	EXPECT_FALSE(styleOf(probe).rounding.has_value());
}

TEST(Wg2P5, CompositeStyleReachesRoot) {
	wg2test::Harness h;
	StyleComposite comp;
	const float baseRounding = ImGui::GetStyle().FrameRounding;

	styleOf(comp).rounding = 9.0f;
	h.layoutFrame(comp, Rect{0.0f, 0.0f, 50.0f, 50.0f});

	EXPECT_FLOAT_EQ(comp.root.rounding, 9.0f);   // 组合控件：样式作用到 root
	EXPECT_FLOAT_EQ(ImGui::GetStyle().FrameRounding, baseRounding);
}
