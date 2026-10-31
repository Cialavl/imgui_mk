#pragma once
// p4 文本显示：Label/TextColored/TextWrapped/BulletText/LabelText/EllipsisLabel/SeparatorText
#include "../../gallery_chrome.h"

namespace gallery {

struct PageText {
using Root = VBox<
Id("root"),
Label<Id("label")>,
TextColored<Id("colored")>,
TextWrapped<Id("wrapped")>,
BulletText<Id("bullet1")>,
BulletText<Id("bullet2")>,
LabelText<Id("pair")>,
EllipsisLabel<Id("ellipsis")>,
SeparatorText<Id("section")>
>;

Page page;
Root root;

PageText() {
page.setup("文本显示",
"wg2 显示控件直接使用 Label/TextColored/TextWrapped/BulletText/"
"LabelText/EllipsisLabel/SeparatorText。",
snips::kTexts);

root.at<Id("label")>().text = "文本原子（wg2 显示文本统一 text 字段）";
auto& _w1 = root.at<Id("colored")>();
_w1.text = "彩色文本（TextColored）";
_w1.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
root.at<Id("wrapped")>().text = "自动换行的长文本：wg2 原子控件语义与裸 ImGui 一致，数据+函数在控件内部。";
root.at<Id("bullet1")>().text = "圆点项一";
root.at<Id("bullet2")>().text = "圆点项二";
auto& _w2 = root.at<Id("pair")>();
_w2.name = "属性";
_w2.value = "值";
root.at<Id("ellipsis")>().text = "EllipsisLabel：路径/长文本自动截断加省略号（侧栏/路径等窄空间专用）";
root.at<Id("section")>().name = "分隔标题";

bindPage(page, root);
}

};

} // namespace gallery
