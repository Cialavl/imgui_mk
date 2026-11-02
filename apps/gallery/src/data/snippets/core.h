#pragma once
// 示例代码（core）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kBase = R"(using Mk::wg2::Id;
using Mk::wg2::VBox;
using Mk::wg2::Label;
using Mk::wg2::Button;

using Root = VBox<Id("root"), Label<Id("title")>, Button<Id("ok")>>;
Root root;
root.at<Id("title")>().text = "标题";
root.at<Id("ok")>().label = "点我";
root.at<Id("ok")>().onClick = [&]{ ++clicks; };
Mk::wg2::layout(root, {0, 0, 400, 200});)";

inline constexpr std::string_view kSplit = R"(using Root = Mk::wg2::VBox<
    Id("root"),
    Mk::wg2::SplitH<Id("split"),
        Mk::wg2::Label<Id("nav")>,
        Mk::wg2::Label<Id("main")>,
        Mk::wg2::Label<Id("side")>
    >,
    Mk::wg2::Wrap<Id("wrap"), Label<Id("w1")>, Label<Id("w2")>>,
    Mk::wg2::Grid<Id("grid"), Label<Id("g00")>, Label<Id("g01")>>
>;
Root root;
root.at<Id("split"), Id("nav")>().state.size.fixedW = 160.0f;
root.at<Id("split"), Id("main")>().state.stretch = 2.0f;
root.at<Id("split"), Id("side")>().state.stretch = 1.0f;)";

} // namespace gallery::snips
