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

root.at<Id("data")>().idname = "tbl";
root.at<Id("data")>().headers = kHeaders;
root.at<Id("data")>().headerCount = 3;
root.at<Id("data")>().rows = &rows;
root.at<Id("fn")>().idname = "fn";
root.at<Id("fn")>().headers = kHeaders;
root.at<Id("fn")>().headerCount = 3;
root.at<Id("fn")>().rowCount = 4;
root.at<Id("fn")>().cell = [](int r, int c) {
return "r" + std::to_string(r) + "c" + std::to_string(c);
};

bindPage(page, root);
}

};

} // namespace gallery
