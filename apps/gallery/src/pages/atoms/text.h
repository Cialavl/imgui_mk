#pragma once
// p4 文本显示：ui::Label/ui::TextColored/ui::TextWrapped/ui::BulletText/ui::LabelText/ui::EllipsisLabel/ui::SeparatorText
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageText {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "文本显示";
	static constexpr const char* desc  = "wg2 显示控件直接使用 ui::Label/ui::TextColored/ui::TextWrapped/ui::BulletText/"
"ui::LabelText/ui::EllipsisLabel/ui::SeparatorText。";
	static constexpr std::string_view code = snips::kTexts;

using Root = ui::VBox<
ui::Id("root"),
ui::Label<ui::Id("label")>,
ui::TextColored<ui::Id("colored")>,
ui::TextWrapped<ui::Id("wrapped")>,
ui::BulletText<ui::Id("bullet1")>,
ui::BulletText<ui::Id("bullet2")>,
ui::LabelText<ui::Id("pair")>,
ui::EllipsisLabel<ui::Id("ellipsis")>,
ui::SeparatorText<ui::Id("section")>
>;

	Root root;

PageText() {


root.at<ui::Id("label")>().text = "文本原子（wg2 显示文本统一 text 字段）";
auto& w_colored = root.at<ui::Id("colored")>();
w_colored.text = "彩色文本（ui::TextColored）";
w_colored.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
root.at<ui::Id("wrapped")>().text = "自动换行的长文本：wg2 原子控件语义与裸 ImGui 一致，数据+函数在控件内部。";
root.at<ui::Id("bullet1")>().text = "圆点项一";
root.at<ui::Id("bullet2")>().text = "圆点项二";
auto& w_pair = root.at<ui::Id("pair")>();
w_pair.name = "属性";
w_pair.value = "值";
root.at<ui::Id("ellipsis")>().text = "ui::EllipsisLabel：路径/长文本自动截断加省略号（侧栏/路径等窄空间专用）";
root.at<ui::Id("section")>().name = "分隔标题";

}

};

} // namespace gallery
