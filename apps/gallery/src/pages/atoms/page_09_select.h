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

auto& _w1 = root.at<Id("check")>();
_w1.name = "勾选";
_w1.checked = &mute;
auto& _w2 = root.at<Id("radio")>();
_w2.name = "单选（布尔）";
_w2.active = &radioBool;
auto& _w3 = root.at<Id("group")>();
_w3.items = kItems;
_w3.count = 3;
_w3.current = &radio;
auto& _w4 = root.at<Id("combo")>();
_w4.name = "下拉";
_w4.current = &comboSel;
_w4.items = kItems;
_w4.count = 3;
auto& _w5 = root.at<Id("list")>();
_w5.name = "列表";
_w5.current = &listSel;
_w5.items = kItems;
_w5.count = 3;
auto& _w6 = root.at<Id("row")>();
_w6.name = "可选中行";
_w6.selected = &rowSel;
root.at<Id("bit1")>().name = "位A"; root.at<Id("bit1")>().flags = &bitFlags; root.at<Id("bit1")>().mask = 1;
root.at<Id("bit2")>().name = "位B"; root.at<Id("bit2")>().flags = &bitFlags; root.at<Id("bit2")>().mask = 2;
root.at<Id("bit3")>().name = "位C"; root.at<Id("bit3")>().flags = &bitFlags; root.at<Id("bit3")>().mask = 4;
root.at<Id("bits")>().name = "位标志";

bindPage(page, root);
	root.at<Id("bits")>().valueFn = [&] { return "0b" + std::bitset<3>(bitFlags).to_string(); };
}


};

} // namespace gallery
