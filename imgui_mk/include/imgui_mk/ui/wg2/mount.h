#pragma once
// 界面接入（Qt 式父子）：把复合界面的 root 接到宿主（主窗口/父组件）上。
//   挂到窗口上通常不用手写这一行——MainWindow::setContent(ui) 会代劳。
#include <imgui_mk/ui/wg2/core.h>

namespace Mk::wg2 {

// 接入：登记 root 的父节点（宿主窗口/父组件）
template <class Root, class Host>
void attach(Root& root, Host& host) { setParent(root, &host); }

} // namespace Mk::wg2
