#pragma once
// p27 布局·简单动态：RuntimeList 运行时增删（墓碑机制）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutRuntime {
	using Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("intro")>,
		Mk::wg2::HBox<Mk::wg2::Id("bar"),
			Mk::wg2::Button<Mk::wg2::Id("add")>,
			Mk::wg2::Button<Mk::wg2::Id("del")>,
			Mk::wg2::LabelText<Mk::wg2::Id("count")>>,
		Mk::wg2::SubWindow<Mk::wg2::Id("zone"), Mk::wg2::RuntimeList<Mk::wg2::Id("list")>>,
		Mk::wg2::Label<Mk::wg2::Id("note")>>;

	Page page;
	Root root;
	int added = 0;

	PageLayoutRuntime() {
		page.setup("布局·简单动态：RuntimeList 增删",
			"静态树里嵌一个 RuntimeList：运行时 add/remove 项不破坏兄弟节点，"
			"渲染回调内增删也安全（墓碑 + pending）。",
			snips::kLayoutRuntime);

		root.at<Mk::wg2::Id("intro")>().text = "点击添加：新项进入下方滚动区；删除总是移除末项";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("add")>().label = "添加一项";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("add")>().onClick = [this] {
			auto item = std::make_unique<Mk::wg2::Label<Mk::wg2::Id("item")>>(
				"列表项 #" + std::to_string(++added));
			Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(item));
			any.idname = "item" + std::to_string(added);
			root.at<Mk::wg2::Id("zone")>().child<0>().add(std::move(any));
		};
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("del")>().label = "删除末项";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("del")>().onClick = [this] {
			root.at<Mk::wg2::Id("zone")>().child<0>().removeLast();
		};
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("count")>().name = "当前项数";
		root.at<Mk::wg2::Id("bar"), Mk::wg2::Id("count")>().valueFn = [this] {
			return std::to_string(root.at<Mk::wg2::Id("zone")>().child<0>().size());
		};

		root.at<Mk::wg2::Id("zone")>().idname = "layoutRuntimeList";
		root.at<Mk::wg2::Id("zone")>().height = 180.0f;
		root.at<Mk::wg2::Id("zone")>().border = true;
		root.at<Mk::wg2::Id("note")>().text = "项是 Label<Id(\"item\")> 的实例（同一类型、不同 idname），由 AnyWidget 统一持有。";

		bindPage(page, root);
	}
};

} // namespace gallery
