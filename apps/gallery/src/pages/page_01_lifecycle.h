#pragma once
// p1 基础：生命周期（RuntimeList 动态叶子 + When 隐藏）
#include "../gallery_chrome.h"

#include <memory>

namespace gallery {

struct PageLifecycle {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::HBox<Mk::wg2::Id("buttons"),
Mk::wg2::Button<Mk::wg2::Id("add")>,
Mk::wg2::Button<Mk::wg2::Id("remove")>
>,
Mk::wg2::CheckBox<Mk::wg2::Id("hide")>,
Mk::wg2::LabelText<Mk::wg2::Id("state")>,
Mk::wg2::LabelText<Mk::wg2::Id("note")>,
Mk::wg2::When<Mk::wg2::Id("when"),
Mk::wg2::Label<Mk::wg2::Id("when_text")>
>,
Mk::wg2::RuntimeList<Mk::wg2::Id("dynamic")>
>;

Page page;
Root root;

PageLifecycle(State& s) {
page.setup("基础：生命周期",
"wg2 静态树没有全局 Node 状态机；动态叶子通过 RuntimeList opt-in，"
"When 装饰件负责显示/隐藏。",
snips::kLife);

root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("add")>().label = "添加动态叶子";
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("add")>().onClick = [&] {
auto label = std::make_unique<Mk::wg2::Label<Mk::wg2::Id("leaf")>>(
"动态叶子（wg2 RuntimeList 项）");
Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(label));
any.idname = "lifeLeaf";
root.at<Mk::wg2::Id("dynamic")>().add(std::move(any));
};
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("remove")>().label = "移除动态叶子";
root.at<Mk::wg2::Id("buttons"), Mk::wg2::Id("remove")>().onClick = [&] {
root.at<Mk::wg2::Id("dynamic")>().removeLast();
};
root.at<Mk::wg2::Id("hide")>().name = "隐藏叶子（When show=false）";
root.at<Mk::wg2::Id("hide")>().checked = &s.mute;
root.at<Mk::wg2::Id("state")>().name = "叶子状态";
root.at<Mk::wg2::Id("state")>().valueFn = [this] {
return root.at<Mk::wg2::Id("dynamic")>().size()
? std::string("有叶子")
: std::string("无叶子");
};
root.at<Mk::wg2::Id("note")>().name = "说明";
root.at<Mk::wg2::Id("note")>().value = "勾选：When 包裹控件隐藏/恢复。";
root.at<Mk::wg2::Id("when"), Mk::wg2::Id("when_text")>().text = "（When 包裹的文本，mute 时隐藏）";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
	root.at<Mk::wg2::Id("when")>().showFn = [&s] { return !s.mute; };
}


};

} // namespace gallery
