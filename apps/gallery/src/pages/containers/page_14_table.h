#pragma once
// p14 表格：TextTable/FnTable（wg2）
#include "../../gallery_chrome.h"

#include <string>

namespace gallery {

struct PageTable {
using Root = VBox<
Id("root"),
TextTable<Id("data")>,
FnTable<Id("fn")>
>;

Page<Root> page;

	static constexpr const char* kHeaders[] = {"Name", "Value", "Note"};

	// 本页自己的数据（不共享、不依赖外部状态容器）
	std::vector<std::vector<std::string>> rows{
		{"A", "1", "data driven"},
		{"B", "2", "external vector"},
		{"C", "3", "session-local"}};

PageTable() {
page.setup("表格",
"wg2 TextTable（数据驱动：外部 vector 只读绑定）；FnTable（单元回调 cell(r,c)）。",
snips::kTable);

auto& w_data = page.root.at<Id("data")>();
w_data.idname = "tbl";
w_data.headers = kHeaders;
w_data.headerCount = 3;
w_data.rows = &rows;
auto& w_fn = page.root.at<Id("fn")>();
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
