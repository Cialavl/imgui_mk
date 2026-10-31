#pragma once
// p9 选择控件：CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags（wg2）
#include "../../gallery_chrome.h"
#include <bitset>

namespace gallery {

struct PageSelect {
using Root = VBox<
Id("root"),
CheckBox<Id("check")>,
RadioButton<Id("radio")>,
RadioGroup<Id("group")>,
Combo<Id("combo")>,
ListBox<Id("list")>,
Selectable<Id("row")>,
CheckboxFlags<Id("bit1")>,
CheckboxFlags<Id("bit2")>,
CheckboxFlags<Id("bit3")>,
LabelText<Id("bits")>
>;

Page page;
Root root;

	static constexpr const char* kItems[] = {"Option A", "Option B", "Option C"};

	// 本页自己的数据（不共享、不依赖外部状态容器）
	bool mute = false;
	bool radioBool = false;
	int radio = 1, comboSel = 0, listSel = 0;
	bool rowSel = false;
	int bitFlags = 0b101;

PageSelect() {
page.setup("选择控件",
"wg2 CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags。"
"items 数组需长寿命（static constexpr）。",
snips::kSelect);

root.at<Id("check")>().name = "勾选";
root.at<Id("check")>().checked = &mute;
root.at<Id("radio")>().name = "单选（布尔）";
root.at<Id("radio")>().active = &radioBool;
root.at<Id("group")>().items = kItems;
root.at<Id("group")>().count = 3;
root.at<Id("group")>().current = &radio;
root.at<Id("combo")>().name = "下拉";
root.at<Id("combo")>().current = &comboSel;
root.at<Id("combo")>().items = kItems;
root.at<Id("combo")>().count = 3;
root.at<Id("list")>().name = "列表";
root.at<Id("list")>().current = &listSel;
root.at<Id("list")>().items = kItems;
root.at<Id("list")>().count = 3;
root.at<Id("row")>().name = "可选中行";
root.at<Id("row")>().selected = &rowSel;
root.at<Id("bit1")>().name = "位A"; root.at<Id("bit1")>().flags = &bitFlags; root.at<Id("bit1")>().mask = 1;
root.at<Id("bit2")>().name = "位B"; root.at<Id("bit2")>().flags = &bitFlags; root.at<Id("bit2")>().mask = 2;
root.at<Id("bit3")>().name = "位C"; root.at<Id("bit3")>().flags = &bitFlags; root.at<Id("bit3")>().mask = 4;
root.at<Id("bits")>().name = "位标志";

bindPage(page, root);
	root.at<Id("bits")>().valueFn = [&] { return "0b" + std::bitset<3>(bitFlags).to_string(); };
}


};

} // namespace gallery
