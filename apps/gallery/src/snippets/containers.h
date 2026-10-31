#pragma once
// 示例代码（containers）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kContainers = R"(Mk::wg2::Overlay<Id("overlay"),
    Button<Id("b1")>,
    Mk::wg2::Offset<Id("off"), Label<Id("l1")>>   // 叠放错开，避免文字重叠
> overlay;
overlay.at<Id("off")>().dxEm = 7.5f;
Mk::wg2::Collapsing<Id("coll"), Label<Id("cl")>> collapsing;
Mk::wg2::Tree<Id("tree"), Label<Id("leaf1")>, Label<Id("leaf2")>> tree;
Mk::wg2::SubWindow<Id("sub"), Label<Id("sl")>> sub;
sub.height = 80.0f;)";

inline constexpr std::string_view kTabBar = R"(Mk::wg2::TabBar<Id("tabs"),
    Mk::wg2::Tab<Id("p1"), Label<Id("t1")>>,
    Mk::wg2::Tab<Id("p2"), Label<Id("t2")>>
> tabs;
tabs.at<Id("p1")>().title = "页一";)";

inline constexpr std::string_view kTable = R"(Mk::wg2::TextTable<Id("tt")> tt;   // 数据驱动
tt.headers = kHeaders; tt.headerCount = 3; tt.rows = &rows;
Mk::wg2::FnTable<Id("ft")> ft;          // 单元格函数
ft.cell = [](int r, int c){ return std::to_string(r) + "," + std::to_string(c); };)";

} // namespace gallery::snips
