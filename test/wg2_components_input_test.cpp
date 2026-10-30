#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <array>

// 第一层 · 输入类控件：勾选 / 文本 / 数值 / 滑块 / 拖拽 / 下拉 / 单选 / 列表

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;
using wg2test::kItems;

TEST(Wg2Component, InputWidgetsDrawAndBind) {
	Harness h;

	bool checked = false;
	int intVal = 3;
	float floatVal = 0.5f;
	double doubleVal = 1.5;
	std::array<int, 2> int2{1, 2};
	std::array<float, 2> float2{1.0f, 2.0f};
	int combo = 0, radioIdx = 1, listSel = 0;
	bool radioBool = false;

	h.inFrame([&] {
		InputText<Id("text"), 32> text;
		text.name = "文本";
		text.setName("hello");
		EXPECT_STREQ(text.c_str(), "hello");
		EXPECT_GT(text.hint(Size{200.0f, 0.0f}).h, 0.0f);
		layout(text, Rect{10.0f, 10.0f, 260.0f, 28.0f});

		CheckBox<Id("check")> check;
		check.name = "勾选";
		check.checked = &checked;
		EXPECT_GT(check.hint(Size{0.0f, 0.0f}).w, 0.0f);
		layout(check, Rect{10.0f, 45.0f, 200.0f, 24.0f});

		InputInt<Id("ii")> ii;      ii.name = "int";    ii.value = &intVal;
		InputFloat<Id("fi")> fi;    fi.name = "float";  fi.value = &floatVal;
		InputDouble<Id("di")> di;   di.name = "double"; di.value = &doubleVal;
		InputInt2<Id("i2")> i2;     i2.name = "int2";   i2.value = &int2;
		InputFloat2<Id("f2")> f2;   f2.name = "float2"; f2.value = &float2;
		layout(ii, Rect{10.0f, 75.0f, 260.0f, 24.0f});
		layout(fi, Rect{10.0f, 105.0f, 260.0f, 24.0f});
		layout(di, Rect{10.0f, 135.0f, 260.0f, 24.0f});
		layout(i2, Rect{10.0f, 165.0f, 260.0f, 24.0f});
		layout(f2, Rect{10.0f, 195.0f, 260.0f, 24.0f});

		SliderF<Id("sf")> sf;       sf.name = "滑块"; sf.value = &floatVal;
		SliderInt<Id("si")> si;     si.name = "整数"; si.value = &intVal;
		float angle = 0.3f;
		SliderAngle<Id("sa")> sa;   sa.name = "角度"; sa.valueRad = &angle;
		DragF<Id("df")> df;         df.name = "拖拽"; df.value = &floatVal;
		DragI<Id("di2")> di2;       di2.name = "拖拽i"; di2.value = &intVal;
		layout(sf, Rect{10.0f, 225.0f, 260.0f, 24.0f});
		layout(si, Rect{10.0f, 255.0f, 260.0f, 24.0f});
		layout(sa, Rect{10.0f, 285.0f, 260.0f, 24.0f});
		layout(df, Rect{10.0f, 315.0f, 260.0f, 24.0f});
		layout(di2, Rect{10.0f, 345.0f, 260.0f, 24.0f});

		Combo<Id("combo")> comboW;  comboW.name = "下拉"; comboW.current = &combo;
		comboW.items = kItems; comboW.count = 3;
		layout(comboW, Rect{10.0f, 375.0f, 260.0f, 24.0f});

		RadioButton<Id("rb")> rb;   rb.name = "单选"; rb.active = &radioBool;
		layout(rb, Rect{10.0f, 405.0f, 260.0f, 24.0f});

		RadioGroup<Id("rg")> rg;    rg.items = kItems; rg.count = 3; rg.current = &radioIdx;
		layout(rg, Rect{10.0f, 435.0f, 260.0f, 80.0f});

		ListBox<Id("lb")> lb;       lb.name = "列表"; lb.items = kItems; lb.count = 3; lb.current = &listSel;
		layout(lb, Rect{10.0f, 520.0f, 260.0f, 70.0f});
	});
}
