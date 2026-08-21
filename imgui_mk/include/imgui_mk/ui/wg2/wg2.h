#pragma once
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/static_layout.h>
#include <imgui_mk/ui/wg2/dynamic.h>
#include <imgui_mk/ui/wg2/basic_widgets.h>
#include <imgui_mk/ui/wg2/widgets/input.h>
#include <imgui_mk/ui/wg2/widgets/display.h>
#include <imgui_mk/ui/wg2/widgets/decorator.h>
#include <imgui_mk/ui/wg2/widgets/container.h>
#include <imgui_mk/ui/wg2/widgets/composite.h>
#include <imgui_mk/ui/wg2/widgets/color.h>
#include <imgui_mk/ui/wg2/widgets/file.h>

// Mk::wg2 —— 纯编译期静态控件体系。
//
// 使用：
//   using namespace Mk::wg2;
//   using Root = VBox<
//       Id("root"),
//       HBox<Id("bar"), Button<Id("ok")>, Button<Id("cancel")>>,
//       RuntimeList<Id("list")>                            // 动态能力按需引入
//   >;
//
//   Root root;
//   root.at<Id("bar"), Id("ok")>().label = "OK";
//   layout(root, Rect{20, 20, 800, 600});
