#pragma once
// p9 选择控件：CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags
#include "../gallery_chrome.h"
#include <bitset>

namespace gallery {

struct PageSelect {
	CheckBox selCheck;
	RadioButton selRadio;
	RadioGroup selGroup;
	Combo selCombo;
	ListBox selList;
	Selectable selRow;
	CheckboxFlags selBit1, selBit2, selBit3;
	LabelText selBits;

	Page page;
	PageSelect(State& s) {
		page.setup("选择控件",
			"CheckBox/RadioButton/RadioGroup/Combo/ListBox/Selectable/CheckboxFlags。"
			"items 数组需长寿命（static constexpr）。",
			snips::kSelect);
		selCheck.self.name = "勾选";
		selCheck.self.checked = &s.mute;
		selRadio.self.name = "单选（布尔）";
		selRadio.self.active = &s.radioBool;
		selGroup.self.items = State::kItems;  selGroup.self.count = 3;
		selGroup.self.current = &s.radio;
		selCombo.self.name = "下拉";
		selCombo.self.current = &s.comboSel;
		selCombo.self.items = State::kItems;  selCombo.self.count = 3;
		selList.self.name = "列表";
		selList.self.current = &s.listSel;
		selList.self.items = State::kItems;  selList.self.count = 3;
		selRow.self.name = "可选中行";
		selRow.self.selected = &s.rowSel;
		selBit1.self.name = "位A"; selBit1.self.flags = &s.bitFlags; selBit1.self.mask = 1;
		selBit2.self.name = "位B"; selBit2.self.flags = &s.bitFlags; selBit2.self.mask = 2;
		selBit3.self.name = "位C"; selBit3.self.flags = &s.bitFlags; selBit3.self.mask = 4;
		selBits.self.name = "位标志";
		page.addWidget(selCheck);
		page.addWidget(selRadio);
		page.addWidget(selGroup);
		page.addWidget(selCombo);
		page.addWidget(selList);
		page.addWidget(selRow);
		page.addWidget(selBit1);
		page.addWidget(selBit2);
		page.addWidget(selBit3);
		page.addWidget(selBits);
	}

	void refresh(State& s) {
		selBits.self.value = "0b" + std::bitset<3>(s.bitFlags).to_string();
	}
};

} // namespace gallery
