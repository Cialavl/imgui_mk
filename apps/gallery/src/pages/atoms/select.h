#pragma once
// p9 选择控件：ui::CheckBox/ui::RadioButton/ui::RadioGroup/ui::Combo/ui::ListBox/ui::Selectable/ui::CheckboxFlags（wg2）
#include "../../demo_page.h"
#include <bitset>

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageSelect {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "选择控件";
	static constexpr const char* desc  = "wg2 CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags。"
"items 数组需长寿命（static constexpr）。";
	static constexpr std::string_view code = snips::kSelect;

using Root = ui::VBox<
ui::Id("root"),
ui::CheckBox<ui::Id("check")>,
ui::RadioButton<ui::Id("radio")>,
ui::RadioGroup<ui::Id("group")>,
ui::Combo<ui::Id("combo")>,
ui::ListBox<ui::Id("list")>,
ui::Selectable<ui::Id("row")>,
ui::CheckboxFlags<ui::Id("bit1")>,
ui::CheckboxFlags<ui::Id("bit2")>,
ui::CheckboxFlags<ui::Id("bit3")>,
ui::LabelText<ui::Id("bits")>
>;

	Root root;

	static constexpr const char* kItems[] = {"Option A", "Option B", "Option C"};

	// 本页自己的数据（不共享、不依赖外部状态容器）
	bool mute = false;
	bool radioBool = false;
	int radio = 1, comboSel = 0, listSel = 0;
	bool rowSel = false;
	int bitFlags = 0b101;

PageSelect() {


auto& w_check = root.at<ui::Id("check")>();
w_check.name = "勾选";
w_check.checked = &mute;
auto& w_radio = root.at<ui::Id("radio")>();
w_radio.name = "单选（布尔）";
w_radio.active = &radioBool;
auto& w_group = root.at<ui::Id("group")>();
w_group.items = kItems;
w_group.count = 3;
w_group.current = &radio;
auto& w_combo = root.at<ui::Id("combo")>();
w_combo.name = "下拉";
w_combo.current = &comboSel;
w_combo.items = kItems;
w_combo.count = 3;
auto& w_list = root.at<ui::Id("list")>();
w_list.name = "列表";
w_list.current = &listSel;
w_list.items = kItems;
w_list.count = 3;
auto& w_row = root.at<ui::Id("row")>();
w_row.name = "可选中行";
w_row.selected = &rowSel;
root.at<ui::Id("bit1")>().name = "位A"; root.at<ui::Id("bit1")>().flags = &bitFlags; root.at<ui::Id("bit1")>().mask = 1;
root.at<ui::Id("bit2")>().name = "位B"; root.at<ui::Id("bit2")>().flags = &bitFlags; root.at<ui::Id("bit2")>().mask = 2;
root.at<ui::Id("bit3")>().name = "位C"; root.at<ui::Id("bit3")>().flags = &bitFlags; root.at<ui::Id("bit3")>().mask = 4;
root.at<ui::Id("bits")>().name = "位标志";

	root.at<ui::Id("bits")>().valueFn = [&] { return "0b" + std::bitset<3>(bitFlags).to_string(); };
}


};

} // namespace gallery
