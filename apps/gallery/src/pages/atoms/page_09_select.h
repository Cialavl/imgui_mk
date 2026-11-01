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

	Page<Root> page;

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

auto& w_check = page.root.at<Id("check")>();
w_check.name = "勾选";
w_check.checked = &mute;
auto& w_radio = page.root.at<Id("radio")>();
w_radio.name = "单选（布尔）";
w_radio.active = &radioBool;
auto& w_group = page.root.at<Id("group")>();
w_group.items = kItems;
w_group.count = 3;
w_group.current = &radio;
auto& w_combo = page.root.at<Id("combo")>();
w_combo.name = "下拉";
w_combo.current = &comboSel;
w_combo.items = kItems;
w_combo.count = 3;
auto& w_list = page.root.at<Id("list")>();
w_list.name = "列表";
w_list.current = &listSel;
w_list.items = kItems;
w_list.count = 3;
auto& w_row = page.root.at<Id("row")>();
w_row.name = "可选中行";
w_row.selected = &rowSel;
page.root.at<Id("bit1")>().name = "位A"; page.root.at<Id("bit1")>().flags = &bitFlags; page.root.at<Id("bit1")>().mask = 1;
page.root.at<Id("bit2")>().name = "位B"; page.root.at<Id("bit2")>().flags = &bitFlags; page.root.at<Id("bit2")>().mask = 2;
page.root.at<Id("bit3")>().name = "位C"; page.root.at<Id("bit3")>().flags = &bitFlags; page.root.at<Id("bit3")>().mask = 4;
page.root.at<Id("bits")>().name = "位标志";

	page.root.at<Id("bits")>().valueFn = [&] { return "0b" + std::bitset<3>(bitFlags).to_string(); };
}


};

} // namespace gallery
