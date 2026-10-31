#pragma once
// p14 表格：TextTable/FnTable（wg2）
#include "../gallery_chrome.h"

#include <string>

namespace gallery {

struct PageTable {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::TextTable<Mk::wg2::Id("data")>,
Mk::wg2::FnTable<Mk::wg2::Id("fn")>
>;

Page page;
Root root;

PageTable(std::vector<std::vector<std::string>>& rows) {
page.setup("表格",
"wg2 TextTable（数据驱动：外部 vector 只读绑定）；FnTable（单元回调 cell(r,c)）。",
snips::kTable);

root.at<Mk::wg2::Id("data")>().idname = "tbl";
root.at<Mk::wg2::Id("data")>().headers = State::kHeaders;
root.at<Mk::wg2::Id("data")>().headerCount = 3;
root.at<Mk::wg2::Id("data")>().rows = &rows;
root.at<Mk::wg2::Id("fn")>().idname = "fn";
root.at<Mk::wg2::Id("fn")>().headers = State::kHeaders;
root.at<Mk::wg2::Id("fn")>().headerCount = 3;
root.at<Mk::wg2::Id("fn")>().rowCount = 4;
root.at<Mk::wg2::Id("fn")>().cell = [](int r, int c) {
return "r" + std::to_string(r) + "c" + std::to_string(c);
};

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
