# 第 07 课：布局系统

wg2 布局容器是编译期模板，子节点经模板参数传入。

## §7.1 VBox / HBox

```cpp
VBox<Id("col"), Label<Id("a")>, Label<Id("b")>> col;
HBox<Id("row"), Label<Id("l")>, Button<Id("r")>> row;
```

`hint(avail)` 计算期望尺寸，`draw(host)` 完成求解和递归绘制。

## §7.2 尺寸覆盖与 stretch

```cpp
root.at<Id("row"), Id("nav")>().state.size.fixedW = 160.0f;
root.at<Id("row"), Id("main")>().state.stretch = 2.0f;
root.at<Id("row"), Id("side")>().state.stretch = 1.0f;
```

`WidgetState` 支持：

```text
fixedW / fixedH
prefW / prefH
minW / minH
maxW / maxH
stretch
```

## §7.3 其他布局

```cpp
Grid<Id("grid"), A, B, C, D> grid;  grid.cols = 2;
Wrap<Id("wrap"), A, B, C, D> wrap;
SplitH<Id("sh"), A, B> sh;          // HBox 别名
SplitV<Id("sv"), A, B> sv;          // VBox 别名
Overlay<Id("ov"), A, B> ov;
```

## 练习

1. 在 `VBox` 中嵌 `HBox`，再嵌 `Grid`，观察 hint 递归。
2. 给某个子节点设置 `fixedW` 和 `stretch`，观察剩余空间分配。
