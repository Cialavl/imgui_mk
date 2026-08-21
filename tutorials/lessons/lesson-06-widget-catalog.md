# 第 06 课：wg2 控件目录

所有控件都按 `Widget<Id("...")>` 形式使用，子控件路径用 `at<Id(...)>()`。

## 输入类

```cpp
InputText<Id("text"), 128> text;
text.name = "文本";
text.setName("初始值");

SliderF<Id("vol")> vol;   vol.name = "音量";  vol.value = &v;
DragF<Id("drag")> drag;   drag.name = "拖拽"; drag.value = &v;
CheckBox<Id("chk")> chk;  chk.name = "静音"; chk.checked = &mute;
Combo<Id("combo")> combo; combo.name = "下拉"; combo.current = &sel;
```

## 显示类

```cpp
Label<Id("lbl")> lbl;        lbl.text = "文本";
TextColored<Id("tc")> tc;    tc.text = "彩色"; tc.color = ImVec4(...);
ProgressBar<Id("bar")> bar;  bar.fraction = &v;
PlotLines<Id("plot")> plot;  plot.values = &history;
```

## 容器类

```cpp
SubWindow<Id("sub"), Label<Id("sl")>> sub;
TabBar<Id("tabs"), Tab<Id("t1"), Label<Id("tl")>>, Tab<Id("t2"), Label<Id("t2l")>>> tabs;
Tree<Id("tree"), Label<Id("leaf1")>, Label<Id("leaf2")>> tree;
```

## 弹窗类

```cpp
bool open = false;
Popup<Id("pop"), Label<Id("pl")>> pop;
pop.open = &open;
PopupModal<Id("modal"), Label<Id("ml")>> modal;
modal.open = &open;
```

## 表格类

```cpp
TextTable<Id("tt")> tt; tt.headers = kHeaders; tt.rows = &rows;
FnTable<Id("ft")> ft;   ft.cell = [](int r, int c) { return "..."; };
```

## 练习

用 `VBox` 把输入、显示、容器、弹窗各选一个控件组成页面，并全部用路径访问配置。
