#pragma once
// p0 基础：模型 struct 控件
#include "../gallery_chrome.h"

namespace gallery {

struct PageBase {
	Label baseL1, baseL2, baseL3, baseL4;
	Button baseBtn;
	LabelText baseClicks;

	Page page;
	PageBase(State& s) {
		page.setup("基础：模型 struct 控件",
			"wg 不支持虚控件 = 模板 struct（数据 + 功能和函数） Control 组合壳 + Node 类型擦除行为表。"
			"控件数据在 self.name = 显示文本；node.idname = 持久标识；findByIdname 查找 / 隐藏 ID。",
			snips::kBase);
		baseL1.self.setName("wg：无继承的模板 struct 控件体系");
		baseL2.self.setName("组合壳 Control<Self, Style>：self + node，组合成一个运行时树。");
		baseL3.self.setName("Node 类型擦除行为表：成员函数 → ADL 自由函数 → 缺省实现。");
		baseL4.self.setName("继承外：运行时 add/remove 墓碑机制；Layout 嵌 Layout 自然递归。");
		baseBtn.self.setName("点我看看行为");
		baseBtn.node.idname = "probeBtn";
		baseBtn.self.onClick = [&] { ++s.clicks; };
		baseClicks.self.name = "点击次数";
		baseClicks.self.valueFn = [&s] { return std::to_string(s.clicks); };   // 拉取式绑定，免每帧赋值
		page.addWidget(baseL1);
		page.addWidget(baseL2);
		page.addWidget(baseL3);
		page.addWidget(baseL4);
		page.addWidget(baseBtn);
		page.addWidget(baseClicks);
	}
};

} // namespace gallery
