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
		auto& w_add = root.at<Id("bar"), Id("add")>();
		w_add.label = "添加一项";
		w_add.onClick = [this] {
			auto item = std::make_unique<Label<Id("item")>>(
				"列表项 #" + std::to_string(++added));
			AnyWidget any = AnyWidget::own(std::move(item));
			any.idname = "item" + std::to_string(added);
			root.at<Id("zone")>().child<0>().add(std::move(any));
		};
		auto& w_del = root.at<Id("bar"), Id("del")>();
		w_del.label = "删除末项";
		w_del.onClick = [this] {
			root.at<Id("zone")>().child<0>().removeLast();
		};
		auto& w_count = root.at<Id("bar"), Id("count")>();
		w_count.name = "当前项数";
		w_count.valueFn = [this] {
			return std::to_string(root.at<Id("zone")>().child<0>().size());
		};

		auto& w_zone = root.at<Id("zone")>();
		w_zone.idname = "layoutRuntimeList";
		w_zone.height = 180.0f;
		w_zone.border = true;
		root.at<Id("note")>().text = "项是 Label<Id(\"item\")> 的实例（同一类型、不同 idname），由 AnyWidget 统一持有。";

		bindPage(page, root);
	}
};

} // namespace gallery
