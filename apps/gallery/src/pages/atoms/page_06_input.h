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

Page<Root> page;

PageInput() {
page.setup("文本输入",
"wg2 InputText<N>（N = 缓冲容量）编辑内部文本；name 作标签；setName 写入内容。",
snips::kInputText);

auto& w_big = page.root.at<Id("big")>();
w_big.name = "文本（128 字符）";
w_big.setName("hello");
auto& w_echo = page.root.at<Id("echo")>();
w_echo.name = "内容回显";
w_echo.valueFn = [this] {
return page.root.at<Id("big")>().c_str();
};
page.root.at<Id("small")>().name = "短文本输入（64 字符）";

}

};

} // namespace gallery
