// 教程 13：Mk::wg2 纯编译期静态树 + 动态列表 opt-in。
//
// 与教程 03 对比：
//   - 旧 wg：普通控件也携带 Node，动态列表用 node.add/remove；
//   - 新 wg2：普通控件是直接 struct，树用 VBox<...> 编译期组合；
//     只有 RuntimeList 内部使用 AnyWidget 类型擦除。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg2/wg2.h>

#include <memory>
#include <string>

using namespace Mk::wg2;

using Root = VBox<
	Id("root"),
	Label<Id("status")>,
	HBox<Id("toolbar"),
		Button<Id("add")>,
		Button<Id("del")>,
		Button<Id("toggle")>
	>,
	VBox<Id("settings"),
		Label<Id("hint")>,
		SliderF<Id("volume")>,
		CheckBox<Id("mute")>
	>,
	SubWindow<Id("listwin"),
		RuntimeList<Id("list")>
	>
>;

int main() {
	Mk::MainWindow<> mainWindow("教程13：wg2 静态树 + 动态列表", 1280, 800);

	// 状态外置
	int frames = 0, added = 0;
	float volume = 0.6f;
	bool mute = false, showSettings = true;

	Root root;

	// 静态路径配置：不再使用 child<数字下标>
	root.at<Id("status")>().text = "状态文本";
	root.at<Id("toolbar"), Id("add")>().label = "添加";
	root.at<Id("toolbar"), Id("del")>().label = "删除末项";
	root.at<Id("toolbar"), Id("toggle")>().label = "显示/隐藏设置区";
	root.at<Id("settings"), Id("hint")>().text = "设置组（VBox 编译期子控件）";
	root.at<Id("settings"), Id("volume")>().name = "音量";
	root.at<Id("settings"), Id("volume")>().value = &volume;
	root.at<Id("settings"), Id("mute")>().name = "静音";
	root.at<Id("settings"), Id("mute")>().checked = &mute;

	root.at<Id("listwin")>().height = 200.0f;
	root.at<Id("listwin")>().idname = "listwin";

	// 动态能力只在 RuntimeList 中 opt-in
	root.at<Id("toolbar"), Id("add")>().onClick = [&] {
		++added;
		auto label = std::make_unique<Label<Id("item")>>(
			"列表项 #" + std::to_string(added));
		AnyWidget any = AnyWidget::own(std::move(label));
		any.idname = "item" + std::to_string(added);
		root.at<Id("listwin"), Id("list")>().add(std::move(any));
	};

	root.at<Id("toolbar"), Id("del")>().onClick = [&] {
		root.at<Id("listwin"), Id("list")>().removeLast();
	};

	root.at<Id("toolbar"), Id("toggle")>().onClick = [&] {
		showSettings = !showSettings;
	};

	mainWindow.setUI([&] {
		++frames;
		root.at<Id("status")>().text =
			"第 " + std::to_string(frames) + " 帧 | 列表 "
			+ std::to_string(root.at<Id("listwin"), Id("list")>().size()) + " 项";
		root.at<Id("settings")>().state.visible = showSettings;

		layout(root, Rect{20, 20, 1240, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
