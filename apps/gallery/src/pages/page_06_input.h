#pragma once
// p6 文本输入：InputText<N>
#include "../gallery_chrome.h"

namespace gallery {

struct PageInput {
	InputText<128> inBig;
	LabelText inEcho;
	InputText<64> inSmall;

	Page page;
	PageInput(State& s) {
		(void)s;
		page.setup("文本输入",
			"InputText<N>（N = 缓冲容量）编辑内部文本；self.name 作标签；setName 写入内容。",
			snips::kInputText);
		inBig.self.name = "文本（128 字符）";
		inBig.self.setName("hello");
		inEcho.self.name = "内容回显";
		inSmall.self.name = "短文本输入（64 字符）";
		page.addWidget(inBig);
		page.addWidget(inEcho);
		page.addWidget(inSmall);
	}

	void refresh(State& s) {
		inEcho.self.value = inBig.self.c_str();   // 回显 inBig 内部缓冲（textBuf 已移除）
	}
};

} // namespace gallery
