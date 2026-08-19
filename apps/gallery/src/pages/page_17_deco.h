#pragma once
// p17 装饰件：Tooltip/Colored/Indent/ItemWidth/When/Styled + node.enabled
#include "../gallery_chrome.h"

namespace gallery {

struct PageDeco {
	Label decoTipTarget, decoColoredText, decoIndentText, decoWhenText;
	Tooltip decoTip;
	Colored decoColored;
	Indent decoIndent;
	ItemWidth<DragF> decoWidth;
	When<Label> decoWhen;
	Button decoStyledBtn, decoDisabled;
	Styled<Button> decoStyled;

	Page page;
	PageDeco(State& s) {
		page.setup("装饰件",
			"Tooltip/Colored/Indent（子节点 node.add）；ItemWidth/When/Styled（模板子控件）。"
			"禁用用 node.enabled = false。",
			snips::kDeco);
		decoTipTarget.self.setName("悬停看我（tooltip 演示）");
		decoTip.self.name = "提示文本";
		decoTip.node.add(decoTipTarget.node);
		decoColoredText.self.setName("彩色文字（colored 包裹）");
		decoColored.self.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
		decoColored.node.add(decoColoredText.node);
		decoIndentText.self.setName("缩进的文本");
		decoIndent.self.amount = 24.0f;
		decoIndent.node.add(decoIndentText.node);
		decoWidth.self.width = 180.0f;
		decoWidth.child<0>().self.name = "限宽拖拽";
		decoWidth.child<0>().self.value = &s.drag;
		decoWhenText.self.setName("When 包裹的文本，show=false 时不渲染不占位。");
		decoWhen.node.add(decoWhenText.node);
		decoStyled.self.theme = Theme::light();
		decoStyledBtn.self.setName("Styled 主题按钮");
		decoStyled.node.add(decoStyledBtn.node);
		decoDisabled.self.setName("禁用按钮（node.enabled=false）");
		decoDisabled.node.enabled = false;
		page.addWidget(decoTip);
		page.addWidget(decoColored);
		page.addWidget(decoIndent);
		page.addWidget(decoWidth);
		page.addWidget(decoWhen);
		page.addWidget(decoStyled);
		page.addWidget(decoDisabled);
	}

	void refresh(State& s) {
		decoWhen.self.show = !s.mute;
	}
};

} // namespace gallery
