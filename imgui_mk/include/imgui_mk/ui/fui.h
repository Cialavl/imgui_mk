#pragma once
#include <imgui_mk/ui/fui_core.h>
#include <imgui_mk/ui/fui_inputs.h>
#include <imgui_mk/ui/fui_display.h>
#include <imgui_mk/ui/fui_containers.h>
#include <imgui_mk/ui/fui_popups.h>
#include <imgui_mk/ui/fui_file.h>
#include <imgui_mk/ui/fui_style.h>

// 函数式控件库·伞头：包含全部模块（Mk::fui 命名空间）。
//
//   fui_core.h        原子控件（Label/Button/SliderF/CheckBox）、布局容器
//                     （VBox/HBox/Group/Collapsing/SubWindow）、基础修饰器
//                     （disabled/when/withId/itemWidth）、与 Mk::ui 的双向桥
//                     （dyn/asWidget）
//   fui_inputs.h      文本/数值输入、拖拽、滑块、单选组、下拉/列表选择
//   fui_display.h     文本展示、进度条、分隔、可选中行、图片、折线图、
//                     树节点、tooltip/colored/indent、纹理上传辅助
//   fui_containers.h  标签页、表格（数据驱动/泛型单元格）、菜单栏、浮动面板
//   fui_popups.h      Popup/PopupModal、颜色编辑/拾取器、位标志复选框
//   fui_file.h         本地文件打开对话框（异步，Windows IFileOpenDialog）
//   fui_style.h        Theme 主题（预设/链式派生/窗口级应用/styled 作用域换肤）
//
// 设计：控件是纯值（聚合类型，无基类、无虚函数），渲染是自由函数重载
// draw(const T&)——编译期决议、可内联；容器持子控件值的 tuple，fold 表达式编译期展开。
//
// 使用范式（每帧重建）：setUI 回调内每帧构造整棵视图值并绘制，状态全部外置于被绑定
// 的变量（引用绑定每帧重建仍实时联动），视图本身无生命周期负担：
//
//   mainWindow.setUI([&]{
//       Mk::fui::draw(Mk::fui::vbox(8.0f,
//           Mk::fui::label("标题"),
//           Mk::fui::button("确定", [&]{ ++clicks; }),
//           Mk::fui::disabled(Mk::fui::slider("音量", volume, 0.0f, 1.0f))
//       ));
//   });
//
// 扩展：任何类型只要在自己的命名空间提供 draw(const T&) 重载，即可被容器/修饰器/
// 适配器接受（依赖实参查找 ADL）——不需要继承任何基类。
//
// 与 Mk::ui（OOP 控件树）双向互通：
//   dyn(oopWidget)     把 OOP 子树（含墓碑动态增删）嵌入函数式树
//   asWidget(maker)    把每帧新鲜构造的函数式视图嵌入 OOP 树