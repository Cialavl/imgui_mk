#pragma once
// p27 布局·简单动态：RuntimeList 运行时增删（墓碑机制）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageLayoutRuntime {
	using Root = VBox<
		Id("root"),
		Label<Id("intro")>,
		HBox<Id("bar"),
			Button<Id("add")>,
			Button<Id("del")>,
			LabelText<Id("count")>>,
		SubWindow<Id("zone"), RuntimeList<Id("list")>>,
		Label<Id("note")>>;

	Page page;
	Root root;
	int added = 0;

	PageLayoutRuntime() {
		page.setup("布局·简单动态：RuntimeList 增删",
			"静态树里嵌一个 RuntimeList：运行时 add/remove 项不破坏兄弟节点，"
			"渲染回调内增删也安全（墓碑 + pending）。",
			snips::kLayoutRuntime);

		root.at<Id("intro")>().text = "点击添加：新项进入下方滚动区；删除总是移除末项";
		root.at<Id("bar"), Id("add")>().label = "添加一项";
		root.at<Id("bar"), Id("add")>().onClick = [this] {
			auto item = std::make_unique<Label<Id("item")>>(
				"列表项 #" + std::to_string(++added));
			AnyWidget any = AnyWidget::own(std::move(item));
			any.idname = "item" + std::to_string(added);
			root.at<Id("zone")>().child<0>().add(std::move(any));
		};
		root.at<Id("bar"), Id("del")>().label = "删除末项";
		root.at<Id("bar"), Id("del")>().onClick = [this] {
			root.at<Id("zone")>().child<0>().removeLast();
		};
		root.at<Id("bar"), Id("count")>().name = "当前项数";
		root.at<Id("bar"), Id("count")>().valueFn = [this] {
			return std::to_string(root.at<Id("zone")>().child<0>().size());
		};

		root.at<Id("zone")>().idname = "layoutRuntimeList";
		root.at<Id("zone")>().height = 180.0f;
		root.at<Id("zone")>().border = true;
		root.at<Id("note")>().text = "项是 Label<Id(\"item\")> 的实例（同一类型、不同 idname），由 AnyWidget 统一持有。";

		bindPage(page, root);
	}
};

} // namespace gallery
