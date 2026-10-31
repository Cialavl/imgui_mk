#pragma once
// 示例代码（layout）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kLayout = R"(using Root = Mk::wg2::VBox<
    Id("root"),
    Mk::wg2::Label<Id("intro")>,
    Mk::wg2::HBox<Id("row"),
        Mk::wg2::VBox<Id("col"), SliderF<Id("v")>, CheckBox<Id("m")>>,
        Mk::wg2::VBox<Id("rightCol"), Label<Id("right")>>
    >
>;
Root root;
Mk::wg2::layout(root, rect);)";

inline constexpr std::string_view kLayoutBasic = R"(Mk::wg2::VBox<Id("root"),
    Mk::wg2::HBox<Id("row"), Button<Id("a")>, Button<Id("b")>>,
    Mk::wg2::Label<Id("note")>
> root;
root.spacing = 8.0f;          // 子项间距
root.marginX = 0.0f;          // 外边距)";

inline constexpr std::string_view kLayoutSize = R"(btn.state.size.fixedW = 120.0f;   // 钉死宽度
btn.state.size.prefW  = 200.0f;   // 期望宽度
btn.state.size.maxW   = 120.0f;   // 可被压缩的上限
btn.state.size.prefWEm = 9.0f;    // 按字号换算（随字体缩放）
other.state.stretch = 1.0f;       // 分配剩余空间)";

inline constexpr std::string_view kLayoutGrid = R"(Mk::wg2::Grid<Id("g"), Button<Id("a")>, Button<Id("b")>> g;
g.cols = 3; g.spacing = 8.0f;
Mk::wg2::Wrap<Id("w"), Button<Id("c")>, Button<Id("d")>> w;   // 放不下自动换行
Mk::wg2::Overlay<Id("o"), Label<Id("base")>, Offset<Id("off"), Button<Id("top")>>> o;
o.at<Id("off")>().dxEm = 9.0f;    // 浮层错位)";

inline constexpr std::string_view kLayoutRuntime = R"(Mk::wg2::RuntimeList<Id("list")> list;
auto item = std::make_unique<Label<Id("item")>>("项 1");
Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(item));
any.idname = "item1";
list.add(std::move(any));     // 回调内增删也安全
list.removeLast();            // 墓碑，不搬移其它项
list.size();                  // pending 也计入)";

inline constexpr std::string_view kLayoutDynamic = R"(Mk::wg2::RuntimeGrid<Id("grid")> grid; grid.cols = 2;
grid.add(Mk::wg2::AnyWidget::own(std::make_unique<Label<Id("tile")>>("卡片")));
Mk::wg2::RuntimeTabs<Id("tabs")> tabs;
tabs.add("页 1", Mk::wg2::AnyWidget::own(std::make_unique<Label<Id("body")>>("内容")));
// 两者都是 AnyWidget 类型擦除，静态树其余部分不受影响)";

} // namespace gallery::snips
