#pragma once
// p6 文本输入：InputText<N>（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageInput {
using Root = VBox<
Id("root"),
InputText<Id("big"), 128>,
LabelText<Id("echo")>,
InputText<Id("small"), 64>
>;

Page page;
Root root;

PageInput() {
page.setup("文本输入",
"wg2 InputText<N>（N = 缓冲容量）编辑内部文本；name 作标签；setName 写入内容。",
snips::kInputText);

auto& _w1 = root.at<Id("big")>();
_w1.name = "文本（128 字符）";
_w1.setName("hello");
auto& _w2 = root.at<Id("echo")>();
_w2.name = "内容回显";
_w2.valueFn = [this] {
return root.at<Id("big")>().c_str();
};
root.at<Id("small")>().name = "短文本输入（64 字符）";

bindPage(page, root);
}

};

} // namespace gallery
