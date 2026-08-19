#pragma once
// p11 容器：OverlayLayout / Collapsing / Tree / SubWindow
#include "../gallery_chrome.h"

namespace gallery {

struct PageContainer {
	Label grpLabel, collLabel, treeLeaf1, treeLeaf2, subLabel;
	Button grpButton;
	OverlayLayout overlayBox;   // Overlay 模式（原 Group）
	Collapsing boxColl;
	Tree boxTree;
	SubWindow<> boxSub;

	Page page;
	PageContainer(State& s) {
		page.setup("容器控件",
			"OverlayLayout 叠放 / Collapsing / Tree / SubWindow。容器 ID 统一为 node.idname。",
			snips::kContainers);
		grpLabel.self.setName("Overlay 叠放（重叠渲染）");
		grpButton.self.setName("叠放按钮");
		grpButton.self.onClick = [&] { ++s.clicks; };

		overlayBox.addWidget(grpLabel.node);
		overlayBox.addWidget(grpButton.node);
		boxColl.self.name = "折叠容器（默认展开）";
		boxColl.self.flags = ImGuiTreeNodeFlags_DefaultOpen;
		collLabel.self.setName("折叠内容");
		boxColl.node.add(collLabel.node);
		boxTree.self.name = "树节点（默认展开）";
		boxTree.self.defaultOpen = true;
		treeLeaf1.self.setName("叶子一");
		treeLeaf2.self.setName("叶子二");
		boxTree.node.add(treeLeaf1.node);
		boxTree.node.add(treeLeaf2.node);
		boxSub.node.idname = "subwin";
		boxSub.self.height = 70.0f;
		subLabel.self.setName("SubWindow：嵌套滚动子窗口");
		boxSub.node.add(subLabel.node);
		page.addWidget(overlayBox);
		page.addWidget(boxColl);
		page.addWidget(boxTree);
		page.addWidget(boxSub);
	}
};

} // namespace gallery
