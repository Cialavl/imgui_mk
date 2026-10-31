#pragma once
// p4 文本显示：Label/TextColored/TextWrapped/BulletText/LabelText/EllipsisLabel/SeparatorText
#include "../gallery_chrome.h"

namespace gallery {

struct PageText {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Label<Mk::wg2::Id("label")>,
Mk::wg2::TextColored<Mk::wg2::Id("colored")>,
Mk::wg2::TextWrapped<Mk::wg2::Id("wrapped")>,
Mk::wg2::BulletText<Mk::wg2::Id("bullet1")>,
Mk::wg2::BulletText<Mk::wg2::Id("bullet2")>,
Mk::wg2::LabelText<Mk::wg2::Id("pair")>,
Mk::wg2::EllipsisLabel<Mk::wg2::Id("ellipsis")>,
Mk::wg2::SeparatorText<Mk::wg2::Id("section")>
>;

Page page;
Root root;

PageText() {
page.setup("文本显示",
"wg2 显示控件直接使用 Label/TextColored/TextWrapped/BulletText/"
"LabelText/EllipsisLabel/SeparatorText。",
snips::kTexts);

root.at<Mk::wg2::Id("label")>().text = "文本原子（wg2 显示文本统一 text 字段）";
root.at<Mk::wg2::Id("colored")>().text = "彩色文本（TextColored）";
root.at<Mk::wg2::Id("colored")>().color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
root.at<Mk::wg2::Id("wrapped")>().text = "自动换行的长文本：wg2 原子控件语义与裸 ImGui 一致，数据+函数在控件内部。";
root.at<Mk::wg2::Id("bullet1")>().text = "圆点项一";
root.at<Mk::wg2::Id("bullet2")>().text = "圆点项二";
root.at<Mk::wg2::Id("pair")>().name = "属性";
root.at<Mk::wg2::Id("pair")>().value = "值";
root.at<Mk::wg2::Id("ellipsis")>().text = "EllipsisLabel：路径/长文本自动截断加省略号（侧栏/路径等窄空间专用）";
root.at<Mk::wg2::Id("section")>().name = "分隔标题";

page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
}

};

} // namespace gallery
