#pragma once
// p27 布局·简单动态：RuntimeList 运行时增删（墓碑机制）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名

// —— 工具条 + 列表区 ——
using ListToolbar = ui::HBox<ui::Id("bar"),
	ui::Button<ui::Id("add")>,
	ui::Button<ui::Id("del")>,
	ui::LabelText<ui::Id("count")>>;

using ListZone = ui::SubWindow<ui::Id("zone"), ui::RuntimeList<ui::Id("list")>>;

struct PageLayoutRuntime {
	static constexpr const char* title = "布局·简单动态：RuntimeList 增删";
	static constexpr const char* desc  = "静态树里嵌一个 RuntimeList：运行时 add/remove 项不破坏兄弟节点，"
		"渲染回调内增删也安全（墓碑 + pending）。";
	static constexpr std::string_view code = snips::kLayoutRuntime;

	// 根只列结构：说明 / 工具条 / 列表区 / 注解
	using Root = ui::VBox<ui::Id("root"),
		ui::Label<ui::Id("intro")>,
		ListToolbar,
		ListZone,
		ui::Label<ui::Id("note")>>;

	Root root;
	int added = 0;

	PageLayoutRuntime() {
		root.at<ui::Id("intro")>().text = "点击添加：新项进入下方滚动区；删除总是移除末项";

		auto& w_add = root.at<ui::Id("bar"), ui::Id("add")>();
		w_add.label = "添加一项";
		w_add.onClick = [this] {
			auto item = std::make_unique<ui::Label<ui::Id("item")>>(
				"列表项 #" + std::to_string(++added));
			ui::AnyWidget any = ui::AnyWidget::own(std::move(item));
			any.idname = "item" + std::to_string(added);
			root.at<ui::Id("zone")>().child<0>().add(std::move(any));
		};

		auto& w_del = root.at<ui::Id("bar"), ui::Id("del")>();
		w_del.label = "删除末项";
		w_del.onClick = [this] { root.at<ui::Id("zone")>().child<0>().removeLast(); };

		auto& w_count = root.at<ui::Id("bar"), ui::Id("count")>();
		w_count.name = "当前项数";
		w_count.valueFn = [this] { return std::to_string(root.at<ui::Id("zone")>().child<0>().size()); };

		auto& w_zone = root.at<ui::Id("zone")>();
		w_zone.idname = "layoutRuntimeList";
		w_zone.height = 180.0f;
		w_zone.border = true;

		root.at<ui::Id("note")>().text = "项是 Label<Id(\"item\")> 的实例（同一类型、不同 idname），由 AnyWidget 统一持有。";
	}
};

} // namespace gallery
