#pragma once
// p14 表格：TextTable/FnTable
#include "../gallery_chrome.h"

namespace gallery {

struct PageTable {
	TextTable tableData;
	FnTable tableFn;

	Page page;
	PageTable(State& s) {
		page.setup("表格",
			"TextTable（数据驱动：外部 vector 只读绑定）；FnTable（单元回调 cell(r,c)）。"
			"表格 ImGui ID 用 node.idname。",
			snips::kTable);
		tableData.node.idname = "tbl";
		tableData.self.headers = State::kHeaders;
		tableData.self.headerCount = 3;
		tableData.self.rows = &s.rows;
		tableFn.node.idname = "fn";
		tableFn.self.headers = State::kHeaders;
		tableFn.self.headerCount = 3;
		tableFn.self.rowCount = 4;
		tableFn.self.cell = [](int r, int c) {
			return "r" + std::to_string(r) + "c" + std::to_string(c);
		};
		page.addWidget(tableData.node);
		page.addWidget(tableFn.node);
	}
};

} // namespace gallery
