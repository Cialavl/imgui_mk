#pragma once
// p1 基础：生命周期（RuntimeList 动态叶子 + When 隐藏）
#include "../../gallery_chrome.h"

#include <memory>

namespace gallery {

struct PageLifecycle {
using Root = VBox<
Id("root"),
HBox<Id("buttons"),
Button<Id("add")>,
Button<Id("remove")>
>,
CheckBox<Id("hide")>,
LabelText<Id("state")>,
LabelText<Id("note")>,
Label<Id("when_text")>,
RuntimeList<Id("dynamic")>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	bool mute = false;

PageLifecycle() {
page.setup("基础：生命周期",
"wg2 静态树没有全局 Node 状态机；动态叶子通过 RuntimeList opt-in，"
"When 装饰件负责显示/隐藏。",
snips::kLife);

auto& w_add = root.at<Id("buttons"), Id("add")>();
w_add.label = "添加动态叶子";
w_add.onClick = [&] {
auto label = std::make_unique<Label<Id("leaf")>>(
"动态叶子（wg2 RuntimeList 项）");
AnyWidget any = AnyWidget::own(std::move(label));
any.idname = "lifeLeaf";
root.at<Id("dynamic")>().add(std::move(any));
};
auto& w_remove = root.at<Id("buttons"), Id("remove")>();
w_remove.label = "移除动态叶子";
w_remove.onClick = [&] {
root.at<Id("dynamic")>().removeLast();
};
auto& w_hide = root.at<Id("hide")>();
w_hide.name = "隐藏叶子（When show=false）";
w_hide.checked = &mute;
auto& w_state = root.at<Id("state")>();
w_state.name = "叶子状态";
w_state.valueFn = [this] {
return root.at<Id("dynamic")>().size()
? std::string("有叶子")
: std::string("无叶子");
};
auto& w_note = root.at<Id("note")>();
w_note.name = "说明";
w_note.value = "勾选：When 包裹控件隐藏/恢复。";
root.at<Id("when_text")>().text = "（When 包裹的文本，mute 时隐藏）";

bindPage(page, root);
	root.at<Id("when_text")>().state.visibleFn = [&] { return !mute; };
}


};

} // namespace gallery
