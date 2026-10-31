#pragma once
// p22 wg2：新控件层演示页。PageHostWg2 保存渲染回调；wg2 layout 回调 在旧 demo
// SubWindow 的 bounds 内执行 Mk::wg2::layout()。
#include "../../gallery_chrome.h"

#include <memory>
#include <string>

namespace gallery {


struct PageWg2 {
	using Wg2Root = Mk::wg2::VBox<
		Mk::wg2::Id("root"),
		Mk::wg2::Label<Mk::wg2::Id("title")>,
		Mk::wg2::HBox<Mk::wg2::Id("row"),
			Mk::wg2::Button<Mk::wg2::Id("add")>,
			Mk::wg2::LabelText<Mk::wg2::Id("count")>
		>,
		Mk::wg2::SubWindow<Mk::wg2::Id("listwin"),
			Mk::wg2::RuntimeList<Mk::wg2::Id("list")>
		>
	>;

	Page page;
	Wg2Root root;

	// 本页自己的数据
	int clicks = 0;

	PageWg2() {
		page.setup("wg2：静态树 + 动态列表 opt-in",
			"普通控件是直接 struct：VBox<Id(\"...\"), ...> 编译期组合，"
			"字符串路径访问；只有 RuntimeList 内部才有 AnyWidget 类型擦除。",
			R"(using Root = Mk::wg2::VBox<
    Mk::wg2::Id("root"),
    Mk::wg2::Label<Mk::wg2::Id("title")>,
    Mk::wg2::SubWindow<Mk::wg2::Id("listwin"),
        Mk::wg2::RuntimeList<Mk::wg2::Id("list")>>
>;)");

		root.at<Mk::wg2::Id("title")>().text = "这是 Mk::wg2 静态树（PageDemo 演示槽内渲染）";
		root.at<Mk::wg2::Id("row"), Mk::wg2::Id("add")>().label = "添加 wg2 动态项";
		root.at<Mk::wg2::Id("row"), Mk::wg2::Id("count")>().name = "clicks";

		root.at<Mk::wg2::Id("listwin")>().height = 180.0f;
		root.at<Mk::wg2::Id("listwin")>().idname = "wg2:list";

		root.at<Mk::wg2::Id("row"), Mk::wg2::Id("add")>().onClick = [&] {
			++clicks;
			auto label = std::make_unique<Mk::wg2::Label<Mk::wg2::Id("item")>>(
				"wg2 动态项 #" + std::to_string(clicks));
			Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(label));
			any.idname = "wg2item" + std::to_string(clicks);
			root.at<Mk::wg2::Id("listwin"), Mk::wg2::Id("list")>().add(std::move(any));
		};

		bindPage(page, root);
	}

};

} // namespace gallery
