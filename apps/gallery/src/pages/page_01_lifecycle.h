#pragma once
// p1 基础：生命周期（动态 add/remove 叶子 + When 隐藏）
#include "../gallery_chrome.h"

namespace gallery {

struct PageLifecycle {
	Button lifeAdd, lifeRemove;
	CheckBox lifeHide;
	LabelText lifeState, lifeNote;
	Label lifeWhenText;
	When<Label> lifeWhen;
	std::unique_ptr<Label> lifeLeaf;   // 动态叶子：裸构造删除

	Page page;
	PageLifecycle(State& s) {
		page.setup("基础：生命周期",
			"draw() 首帧挂载触发 onCreate；可见性变化 onShow/onHide；移除 onDestroy。"
			"动态叶子演示运行时 add/remove（墓碑机制，渲染回调中安全）。",
			snips::kLife);
		lifeAdd.self.setName("添加动态叶子");
		lifeAdd.self.onClick = [&] {
			if(!lifeLeaf) {
				lifeLeaf = std::make_unique<Label>();
				lifeLeaf->self.setName("动态叶子（onCreate/onDestroy）");
		page.addWidget(lifeLeaf->node);
			}
		};
		lifeRemove.self.setName("移除动态叶子");
		lifeRemove.self.onClick = [&] {
			if(lifeLeaf) {
				page.demo.node.remove(lifeLeaf->node);
				lifeLeaf.reset();
			}
		};
		lifeHide.self.name = "隐藏叶子（onHide/onShow）";
		lifeHide.self.checked = &s.mute;
		lifeState.self.name = "叶子状态";
		lifeState.self.valueFn = [this] { return lifeLeaf ? std::string("有叶子（onCreate 已触发）") : std::string("无叶子"); };
		lifeNote.self.name = "说明";
		lifeNote.self.value = "勾选：演示观察 When 包裹控件的隐藏/恢复（onHide/onShow 日志）。";
		lifeWhenText.self.setName("（When 包裹的文本，mute 时隐藏显示）");
		lifeWhen.node.add(lifeWhenText.node);
		page.addWidget(lifeAdd);
		page.addWidget(lifeRemove);
		page.addWidget(lifeHide);
		page.addWidget(lifeState);
		page.addWidget(lifeNote);
		page.addWidget(lifeWhen);
	}

	void refresh(State& s) {
		lifeWhen.self.show = !s.mute;
	}
};

} // namespace gallery
