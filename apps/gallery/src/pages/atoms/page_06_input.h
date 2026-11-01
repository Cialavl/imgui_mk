#pragma once
// p6 文本输入：InputText<N>（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageInput {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "文本输入";
	static constexpr const char* desc  = "wg2 InputText<N>（N = 缓冲容量）编辑内部文本；name 作标签；setName 写入内容。";
	static constexpr std::string_view code = snips::kInputText;

using Root = VBox<
Id("root"),
InputText<Id("big"), 128>,
LabelText<Id("echo")>,
InputText<Id("small"), 64>
>;

	Root root;

PageInput() {


auto& w_big = root.at<Id("big")>();
w_big.name = "文本（128 字符）";
w_big.setName("hello");
auto& w_echo = root.at<Id("echo")>();
w_echo.name = "内容回显";
w_echo.valueFn = [this] {
return root.at<Id("big")>().c_str();
};
root.at<Id("small")>().name = "短文本输入（64 字符）";

}

};

} // namespace gallery
