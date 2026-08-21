#pragma once
// p6 文本输入：InputText<N>（wg2）
#include "../gallery_chrome.h"

namespace gallery {

struct PageInput {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::InputText<Mk::wg2::Id("big"), 128>,
Mk::wg2::LabelText<Mk::wg2::Id("echo")>,
Mk::wg2::InputText<Mk::wg2::Id("small"), 64>
>;

Page page;
Root root;

PageInput(State& s) {
(void)s;
page.setup("文本输入",
"wg2 InputText<N>（N = 缓冲容量）编辑内部文本；name 作标签；setName 写入内容。",
snips::kInputText);

root.at<Mk::wg2::Id("big")>().name = "文本（128 字符）";
root.at<Mk::wg2::Id("big")>().setName("hello");
root.at<Mk::wg2::Id("echo")>().name = "内容回显";
root.at<Mk::wg2::Id("echo")>().valueFn = [this] {
return root.at<Mk::wg2::Id("big")>().c_str();
};
root.at<Mk::wg2::Id("small")>().name = "短文本输入（64 字符）";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
