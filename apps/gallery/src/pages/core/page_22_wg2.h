#pragma once
// p22 wg2：新控件层演示页。PageHostWg2 保存渲染回调；wg2 layout 回调 在旧 demo
// SubWindow 的 bounds 内执行 layout()。
#include "../../gallery_chrome.h"

#include <memory>
#include <string>

namespace gallery {


struct PageWg2 {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "wg2：静态树 + 动态列表 opt-in";
	static constexpr const char* desc  = "普通控件是直接 struct：VBox<Id(\"...\"), ...> 编译期组合，"
			"字符串路径访问；只有 RuntimeList 内部才有 AnyWidget 类型擦除。";
	static constexpr std::string_view code = R"(using Root = VBox<
    Id("root"),
    Label<Id("title")>,
    SubWindow<Id("listwin"),
        RuntimeList<Id("list")>>
>;)";

	using Wg2Root = VBox<
		Id("root"),
		Label<Id("title")>,
		HBox<Id("row"),
			Button<Id("add")>,
			LabelText<Id("count")>
		>,
		SubWindow<Id("listwin"),
			RuntimeList<Id("list")>
		>
	>;

	Wg2Root root;

	// 本页自己的数据
	int clicks = 0;

	PageWg2() {
		

		root.at<Id("title")>().text = "这是 Mk::wg2 静态树（PageDemo 演示槽内渲染）";
		root.at<Id("row"), Id("add")>().label = "添加 wg2 动态项";
		root.at<Id("row"), Id("count")>().name = "clicks";

		auto& w_listwin = root.at<Id("listwin")>();
		w_listwin.height = 180.0f;
		w_listwin.idname = "wg2:list";

		root.at<Id("row"), Id("add")>().onClick = [&] {
			++clicks;
			auto label = std::make_unique<Label<Id("item")>>(
				"wg2 动态项 #" + std::to_string(clicks));
			AnyWidget any = AnyWidget::own(std::move(label));
			any.idname = "wg2item" + std::to_string(clicks);
			root.at<Id("listwin"), Id("list")>().add(std::move(any));
		};

	}

};

} // namespace gallery
