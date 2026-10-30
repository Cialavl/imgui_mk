#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <string>
#include <vector>

// 第一层 · 表格与文件对话框

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;

TEST(Wg2Component, TextTableAndFnTableDrawRows) {
	Harness h;
	const char* headers[] = {"Name", "Value"};
	std::vector<std::vector<std::string>> rows = {{"A", "1"}, {"B", "2"}};

	TextTable<Id("tt")> tt;
	tt.headers = headers;
	tt.headerCount = 2;
	tt.rows = &rows;
	tt.idname = "wg2-test-table";

	FnTable<Id("ft")> ft;
	ft.headers = headers;
	ft.headerCount = 2;
	ft.rowCount = 3;
	ft.idname = "wg2-test-fntable";
	ft.cell = [](int r, int c) { return std::to_string(r) + "," + std::to_string(c); };

	h.inFrame([&] {
		EXPECT_GT(tt.hint(Size{300.0f, 0.0f}).h, 0.0f);
		EXPECT_GT(ft.hint(Size{300.0f, 0.0f}).h, 0.0f);
		layout(tt, Rect{0.0f, 0.0f, 300.0f, 100.0f});
		layout(ft, Rect{0.0f, 120.0f, 300.0f, 100.0f});
	});
}

TEST(Wg2Component, FileDialogIdleStateIsInert) {
	FileDialog dlg;
	EXPECT_FALSE(dlg.busy());
	std::string path;
	EXPECT_FALSE(dlg.poll(path));   // 未打开时 poll 恒为 false
}
