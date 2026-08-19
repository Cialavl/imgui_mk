# 第 06 课：控件目录巡礼

配套程序：`src/tutorial06_catalog.cpp`。一站式过一遍 wg 的常用控件，
以及"树外常驻"的菜单栏与弹窗。

## §6.1 输入类

```cpp
Mk::wg::InputText<64>  text;   text.self.label = "文本";
Mk::wg::InputInt       i;      i.self.value = &iVal;
Mk::wg::InputFloat     f;      f.self.value = &fVal;
Mk::wg::InputFloat2    v2;     v2.self.value = &fv;      // std::array<float,2>*
Mk::wg::DragF          drag;   drag.self.value = &drag;  drag.self.speed = 0.01f;
Mk::wg::SliderF        slide;  slide.self.value = &slideV;
Mk::wg::RadioGroup     rg;     rg.self.items = kItems; rg.self.count = 3; rg.self.current = &radio;
Mk::wg::Combo          combo;  combo.self.current = &comboSel; combo.self.items = kItems; combo.self.count = 3;
Mk::wg::ListBox        list;   list.self.current = &listSel;   list.self.items = kItems; list.self.count = 3;
```

要点：`items` 必须长期存活（配套程序用 `static constexpr` 数组）；
`current` 是选中下标（`int*`）。

## §6.2 显示类

```cpp
Mk::wg::ProgressBar pb;   pb.self.fraction = &progress;
Mk::wg::Selectable  sl;   sl.self.selected = &rowSel;
Mk::wg::PlotLines   pl;   pl.self.values = &history;   // const vector<float>*
```

`history` 由逻辑侧每帧 push 并裁剪（配套程序帧循环里做），PlotLines
只读绑定——显示控件不持有数据。

## §6.3 容器：TabBar 与表格

```cpp
Mk::wg::TabBar<Mk::wg::Tab<Mk::wg::TextTable>, Mk::wg::Tab<Mk::wg::FnTable>> tabs;
tabs.self.id = "tabs6";
tabs.child<0>().node.id = "数据表";          // 页名存 node.id
tabs.child<0>().child<0>().self.rows = &rows; // TextTable：数据驱动
tabs.child<1>().child<0>().self.cell = [](int r, int c) {
    return std::to_string(r) + "," + std::to_string(c);
};                                            // FnTable：单元格函数
```

TabBar 只渲染激活页；Tab 的页名就是其 `node.id`。

## §6.4 弹窗：标志触发式

```cpp
Mk::wg::Popup<Mk::wg::Label, CloseButton> pop;
pop.self.id = "about6";
pop.self.open = &aboutOpen;   // 置 true 当帧 OpenPopup，随后自动回落
pop.child<1>().self.label = "关闭";
```

`PopupModal` 是模态变体（阻挡宿主交互）。关闭按钮必须调用
`ImGui::CloseCurrentPopup()`（配套程序的 `CloseButton` 控件）。

## §6.5 菜单栏：树外常驻

菜单栏不属于主体树——它在窗口顶部独立布局：

```cpp
Mk::wg::MenuBar<Mk::wg::Menu<Mk::wg::MenuItem>> menuTree;
menuTree.child<0>().self.label = "文件";
menuTree.child<0>().child<0>().self.label = "关于";
menuTree.child<0>().child<0>().self.onClick = [&] { aboutOpen = true; };
// 每帧：menuTree.layout({0, 0, w, 24});
```

## 练习

1. 给目录加一个 `CheckboxFlags` 三连（位 A/B/C 绑定同一个 `int`）。
2. 把 FnTable 的 cell 改成返回"第 r 行第 c 列"的中文。
3. 观察 TabBar：切页后另一页的控件状态是否保留？（控件是常驻对象，
   状态在绑定变量里，天然保留。）
