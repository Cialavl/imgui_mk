#pragma once
// p12 标签页：TabBar/Tab
#include "../gallery_chrome.h"

namespace gallery {

struct PageTab {
	Label tab1Text, tab2Text;
	Tab<> tab1, tab2;    // Tab 页运行时加入（页签 = idname）
	TabBar<> tabs;

	Page page;
	PageTab(State& s) {
		(void)s;
		page.setup("标签页",
			"TabBar 容器渲染子页签；页签 = Tab 节点 idname。Tab 运行时挂入 TabBar。",
			snips::kTabBar);
		tab1Text.self.setName("TabBar 容器：页签渲染，页签 = idname");
		tab2Text.self.setName("第二页内容");
		tab1.node.idname = "页一";
		tab1.node.add(tab1Text.node);
		tab2.node.idname = "页二";
		tab2.node.add(tab2Text.node);
		tabs.node.idname = "galtabs";
		tabs.node.add(tab1.node);
		tabs.node.add(tab2.node);
		page.addWidget(tabs.node);
	}
};

} // namespace gallery
