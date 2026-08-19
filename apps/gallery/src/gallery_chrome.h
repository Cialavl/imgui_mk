#pragma once
// gallery 页铬适配层：Page 页架（组装 SectionHead/SubWindow/CodeBlock）+ InsUi。
// 通用控件（Sidebar/PageHost/SectionHead/CodeBlock/FrameLabel/ClosePopupButton）已下沉到 wg_extras.h。
#include "gallery_state.h"
#include "snippets.h"
#include <imgui_mk/ui/wg_extras.h>
#include <algorithm>
#include <cstring>
#include <string_view>

namespace gallery {

using namespace Mk::wg;

// ============ 页面架 ============

struct Page {
	SectionHead head;
	SubWindow<> demo;        // 演示内容（V 堆叠 + 子窗口内部滚动，内容超高不再撑爆预览区）
	SeparatorText codeTitle;
	CodeBlock code;
	VLayout root;            // 页面根（VBox：head + demo(stretch) + codeTitle + code）
	const char* title = "";  // 页面标题（侧栏条目用）

	// Qt 风格：页面添加控件（转发到演示区，隐藏 demo 细节；支持 Node& 与任意控件）
	void addWidget(Node& w, float stretch = 0.0f) { (void)stretch; demo.node.add(w); }
	template<typename W>
	void addWidget(W& w, float stretch = 0.0f) { demo.addWidget(w.node, stretch); }
	Node& rootNode() { return root.node; }

	// 页铬初始化（标题 + 说明 + 代码片段 + 页根组装）
	void setup(const char* pageTitle, const char* desc, std::string_view snippet) {
		title = pageTitle;                      // 侧栏条目标题存根
		demo.node.idname = std::string("demo:") + pageTitle;   // 每页唯一：滚动位置互不干扰
		demo.self.border = false;   // spacing 默认哨兵 = ItemSpacing.y（主题/字号联动）
		head.self.title = pageTitle;
		head.self.desc = desc;
		codeTitle.self.name = "Code";
		code.self.code = snippet;
		code.self.mono = gMono;      // 等宽字体注入（gMono 由 main 装载，构建前已就绪）
		root.setContentsMargins(0.0f, 0.0f);   // 页内不叠加外边距：preview 子窗口自带 WindowPadding
		root.addWidget(head.node);
		root.addWidget(demo.node, 1.0f);   // 演示区弹性吃剩余（SubWindow 内部滚）
		root.addWidget(codeTitle.node);
		root.addWidget(code.node);
	}
};


// ============ Inspector 窗口 UI ============

struct InsUi {
	SeparatorText head;
	SliderF volume;
	CheckBox mute;
	CheckboxFlags bit1, bit2, bit3;
	SeparatorText framesHead;
	LabelText galFrames, insFrames;
	TextWrapped note;
	VLayout root;
};

inline void buildIns(InsUi& u, State& s) {
	u.head.self.name = "Shared state (live link with Gallery)";
	u.volume.self.name = "Volume";
	u.volume.self.value = &s.volume;
	u.mute.self.name = "Mute";
	u.mute.self.checked = &s.mute;
	u.bit1.self.name = "Bit A"; u.bit1.self.flags = &s.bitFlags; u.bit1.self.mask = 1;
	u.bit2.self.name = "Bit B"; u.bit2.self.flags = &s.bitFlags; u.bit2.self.mask = 2;
	u.bit3.self.name = "Bit C"; u.bit3.self.flags = &s.bitFlags; u.bit3.self.mask = 4;
	u.framesHead.self.name = "Independent frame loops";
	u.galFrames.self.name = "Gallery frame";
	u.insFrames.self.name = "Inspector frame";
	u.note.self.name = "Two top-level windows each own an ImGui context and frame loop; "
		"binding the same external variables keeps them live-linked. "
		"Clicking X only hides the Inspector; it follows the Gallery out when the Gallery closes.";
	for(Node* nd : {&u.head.node, &u.volume.node, &u.mute.node, &u.bit1.node,
		&u.bit2.node, &u.bit3.node, &u.framesHead.node, &u.galFrames.node,
		&u.insFrames.node, &u.note.node}) {
		u.root.addWidget(*nd);
	}
}

inline void refreshIns(InsUi& u, State& s) {
	u.galFrames.self.value = std::to_string(s.frames);
	u.insFrames.self.value = std::to_string(s.insFrames);
}

} // namespace gallery
