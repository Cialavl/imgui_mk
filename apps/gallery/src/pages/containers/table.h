#pragma once
// p14 表格：ui::TextTable/ui::FnTable（wg2）
#include "../../gallery_chrome.h"

#include <string>

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageTable {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "表格";
	static constexpr const char* desc  = "wg2 TextTable（数据驱动：外部 vector 只读绑定）；FnTable（单元回调 cell(r,c)）。";
	static constexpr std::string_view code = snips::kTable;

using Root = ui::VBox<
ui::Id("root"),
ui::TextTable<ui::Id("data")>,
ui::FnTable<ui::Id("fn")>
>;

	Root root;

	static constexpr const char* kHeaders[] = {"Name", "Value", "Note"};

	// 本页自己的数据（不共享、不依赖外部状态容器）
	std::vector<std::vector<std::string>> rows{
		{"A", "1", "data driven"},
		{"B", "2", "external vector"},
		{"C", "3", "session-local"}};

PageTable() {


auto& w_data = root.at<ui::Id("data")>();
w_data.idname = "tbl";
w_data.headers = kHeaders;
w_data.headerCount = 3;
w_data.rows = &rows;
auto& w_fn = root.at<ui::Id("fn")>();
w_fn.idname = "fn";
w_fn.headers = kHeaders;
w_fn.headerCount = 3;
w_fn.rowCount = 4;
w_fn.cell = [](int r, int c) {
return "r" + std::to_string(r) + "c" + std::to_string(c);
};

}

};

} // namespace gallery
