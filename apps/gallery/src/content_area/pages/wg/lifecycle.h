#pragma once
// p1 基础：生命周期（ui::RuntimeList 动态叶子 + When 隐藏）
#include "../../demo_page.h"

#include <memory>

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageLifecycle {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "基础：生命周期";
	static constexpr const char* desc  = "wg2 静态树没有全局 Node 状态机；动态叶子通过 RuntimeList opt-in，"
"When 装饰件负责显示/隐藏。";
	static constexpr std::string_view code = snips::kLife;

using Root = ui::VBox<
ui::Id("root"),
ui::HBox<ui::Id("buttons"),
ui::Button<ui::Id("add")>,
ui::Button<ui::Id("remove")>
>,
ui::CheckBox<ui::Id("hide")>,
ui::LabelText<ui::Id("state")>,
ui::LabelText<ui::Id("note")>,
ui::Label<ui::Id("when_text")>,
ui::RuntimeList<ui::Id("dynamic")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	bool mute = false;

PageLifecycle() {


auto& w_add = root.at<ui::Id("buttons"), ui::Id("add")>();
w_add.label = "添加动态叶子";
w_add.onClick = [&] {
auto label = std::make_unique<ui::Label<ui::Id("leaf")>>(
"动态叶子（wg2 RuntimeList 项）");
ui::AnyWidget any = ui::AnyWidget::own(std::move(label));
any.idname = "lifeLeaf";
root.at<ui::Id("dynamic")>().add(std::move(any));
};
auto& w_remove = root.at<ui::Id("buttons"), ui::Id("remove")>();
w_remove.label = "移除动态叶子";
w_remove.onClick = [&] {
root.at<ui::Id("dynamic")>().removeLast();
};
auto& w_hide = root.at<ui::Id("hide")>();
w_hide.name = "隐藏叶子（When show=false）";
w_hide.checked = &mute;
auto& w_state = root.at<ui::Id("state")>();
w_state.name = "叶子状态";
w_state.valueFn = [this] {
return root.at<ui::Id("dynamic")>().size()
? std::string("有叶子")
: std::string("无叶子");
};
auto& w_note = root.at<ui::Id("note")>();
w_note.name = "说明";
w_note.value = "勾选：When 包裹控件隐藏/恢复。";
root.at<ui::Id("when_text")>().text = "（When 包裹的文本，mute 时隐藏）";

	Mk::wg2::setVisibleFn(root.at<ui::Id("when_text")>(), [&] { return !mute; });
}


};

} // namespace gallery
