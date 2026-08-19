#pragma once
// p4 文本显示：Label/TextColored/TextWrapped/BulletText/LabelText/EllipsisLabel/SeparatorText
#include "../gallery_chrome.h"

namespace gallery {

struct PageText {
	Label txtLabel;
	TextColored txtColored;
	TextWrapped txtWrapped;
	BulletText txtBullet1, txtBullet2;
	LabelText txtPair;
	EllipsisLabel txtEllipsis;
	SeparatorText txtSection;

	Page page;
	PageText(State& s) {
		(void)s;
		page.setup("文本显示",
			"Label/TextColored/TextWrapped/BulletText/LabelText(name+value)/EllipsisLabel。"
			"显示文本统一为 self.name。",
			snips::kTexts);
		txtLabel.self.setName("文本原子（组合：显示文本统一 self.name）");
		txtColored.self.name = "彩色文本（TextColored）";
		txtColored.self.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
		txtWrapped.self.name = "自动换行的长文本：wg 原子控件语义与裸 ImGui 一致，数据+函数在控件内部。";
		txtBullet1.self.name = "圆点项一";
		txtBullet2.self.name = "圆点项二";
		txtPair.self.name = "属性";  txtPair.self.value = "值";
		txtEllipsis.self.name = "EllipsisLabel：路径/长文本自动截断加省略号（侧栏/路径等窄空间专用）";
		txtSection.self.name = "分隔标题";
		page.addWidget(txtLabel);
		page.addWidget(txtColored);
		page.addWidget(txtWrapped);
		page.addWidget(txtBullet1);
		page.addWidget(txtBullet2);
		page.addWidget(txtPair);
		page.addWidget(txtEllipsis);
		page.addWidget(txtSection);
	}
};

} // namespace gallery
