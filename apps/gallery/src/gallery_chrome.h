#pragma once
// gallery 页铬适配层：Page 页架（组装 SectionHead/SubWindow/CodeBlock）。
// 通用控件（Sidebar/PageHost/SectionHead/CodeBlock/FrameLabel/ClosePopupButton）已下沉到 wg_extras.h；
// Inspector 子窗口见 gallery_inspector.h（独立文件分层）。
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
		code.self.monoFn = [] { return gMono; };   // 拉取式等宽字体：首帧重烘焙后就绪（不拷构造期快照）
		root.setContentsMargins(0.0f, 0.0f);   // 页内不叠加外边距：preview 子窗口自带 WindowPadding
		root.addWidget(head.node);
		root.addWidget(demo.node, 1.0f);   // 演示区弹性吃剩余（SubWindow 内部滚）
		root.addWidget(codeTitle.node);
		root.addWidget(code.node);
	}
};


} // namespace gallery
