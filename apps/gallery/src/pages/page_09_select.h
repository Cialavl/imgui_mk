#pragma once
// p9 选择控件：CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags（wg2）
#include "../gallery_chrome.h"
#include <bitset>

namespace gallery {

struct PageSelect {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::CheckBox<Mk::wg2::Id("check")>,
Mk::wg2::RadioButton<Mk::wg2::Id("radio")>,
Mk::wg2::RadioGroup<Mk::wg2::Id("group")>,
Mk::wg2::Combo<Mk::wg2::Id("combo")>,
Mk::wg2::ListBox<Mk::wg2::Id("list")>,
Mk::wg2::Selectable<Mk::wg2::Id("row")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit1")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit2")>,
Mk::wg2::CheckboxFlags<Mk::wg2::Id("bit3")>,
Mk::wg2::LabelText<Mk::wg2::Id("bits")>
>;

Page page;
Root root;

PageSelect(bool& mute, bool& radioBool, int& radio, int& comboSel, int& listSel, bool& rowSel, int& bitFlags) {
page.setup("选择控件",
"wg2 CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags。"
"items 数组需长寿命（static constexpr）。",
snips::kSelect);

root.at<Mk::wg2::Id("check")>().name = "勾选";
root.at<Mk::wg2::Id("check")>().checked = &mute;
root.at<Mk::wg2::Id("radio")>().name = "单选（布尔）";
root.at<Mk::wg2::Id("radio")>().active = &radioBool;
root.at<Mk::wg2::Id("group")>().items = State::kItems;
root.at<Mk::wg2::Id("group")>().count = 3;
root.at<Mk::wg2::Id("group")>().current = &radio;
root.at<Mk::wg2::Id("combo")>().name = "下拉";
root.at<Mk::wg2::Id("combo")>().current = &comboSel;
root.at<Mk::wg2::Id("combo")>().items = State::kItems;
root.at<Mk::wg2::Id("combo")>().count = 3;
root.at<Mk::wg2::Id("list")>().name = "列表";
root.at<Mk::wg2::Id("list")>().current = &listSel;
root.at<Mk::wg2::Id("list")>().items = State::kItems;
root.at<Mk::wg2::Id("list")>().count = 3;
root.at<Mk::wg2::Id("row")>().name = "可选中行";
root.at<Mk::wg2::Id("row")>().selected = &rowSel;
root.at<Mk::wg2::Id("bit1")>().name = "位A"; root.at<Mk::wg2::Id("bit1")>().flags = &bitFlags; root.at<Mk::wg2::Id("bit1")>().mask = 1;
root.at<Mk::wg2::Id("bit2")>().name = "位B"; root.at<Mk::wg2::Id("bit2")>().flags = &bitFlags; root.at<Mk::wg2::Id("bit2")>().mask = 2;
root.at<Mk::wg2::Id("bit3")>().name = "位C"; root.at<Mk::wg2::Id("bit3")>().flags = &bitFlags; root.at<Mk::wg2::Id("bit3")>().mask = 4;
root.at<Mk::wg2::Id("bits")>().name = "位标志";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
	root.at<Mk::wg2::Id("bits")>().valueFn = [&] { return "0b" + std::bitset<3>(bitFlags).to_string(); };
}


};

} // namespace gallery
