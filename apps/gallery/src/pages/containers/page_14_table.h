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

Page page;
Root root;

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

auto& _w1 = root.at<Id("data")>();
_w1.idname = "tbl";
_w1.headers = kHeaders;
_w1.headerCount = 3;
_w1.rows = &rows;
auto& _w2 = root.at<Id("fn")>();
_w2.idname = "fn";
_w2.headers = kHeaders;
_w2.headerCount = 3;
_w2.rowCount = 4;
_w2.cell = [](int r, int c) {
return "r" + std::to_string(r) + "c" + std::to_string(c);
};

bindPage(page, root);
}

};

} // namespace gallery
