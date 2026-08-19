# 第 07 课：布局系统

配套程序：`src/tutorial07_layout.cpp`。wg 的布局是**两遍布局**
（measure → arrange），加上**分格容器**（HSplit/VSplit）与
**em 设计像素**，四个版本逐层演进。

## §7.1 两遍布局：measure / arrange / draw

```cpp
root.layout(Mk::wg::Rect{20, 20, 1240, 860});
```

`layout(host)` 内部三步：

1. `measure(avail)`：自底向上求期望尺寸（内容高度、文本宽度……）；
2. `arrange(rect)`：自顶向下分配 bounds（VBox 纵向叠加、HBox 横向排列）；
3. `draw()`：按 bounds 定位渲染。

容器（VBox/HBox/Group）在 arrange 时对每个子节点"measure 再给矩形"。
非填充型节点 `layout` 会用测量尺寸覆盖 host；**分格是填充型容器**：
`Node::fill` 置位后 layout 保留 host 尺寸原样交给 arrange（见 §7.2）。

## §7.2 分格：HSplit / VSplit + Fixed / Stretch

```cpp
Mk::wg::HSplit<Mk::wg::Fixed<Mk::wg::Label>,
    Mk::wg::Stretch<Mk::wg::Label>, Mk::wg::Stretch<Mk::wg::Label>> v2;
v2.child<0>().self.size = 150.0f;   // 固定：设计像素（em 缩放）
v2.child<1>().self.weight = 2.0f;   // 弹性：按权重瓜分剩余
v2.child<2>().self.weight = 1.0f;
```

分配规则（flexbox 语义）：

- Fixed 格先 `clamp(em(size), em(minPx), maxFrac × 可用)` 得理想值；
- 超出容器时按 shrink 比例向 min 均匀收缩（`minPx` 保护可读性）；
- Stretch 格按 `weight / Σweight` 瓜分剩余（可为 0）。

子格角色经 `Node::role`（CellRole）在 measure 时填入，分格 arrange 读取
——类型擦除下的零继承通道。

## §7.3 em：设计像素随字号等比

`em(designPx) = designPx × 当前字号 / 18`。控件尺寸按 18px 基准设计，
字号升档/DPI 变化时全部等比：

```cpp
v3.child<0>().self.size = 44.0f;   // 工具栏高：随字号缩放
```

对比 v1 的"固定 250 宽"：字号变大时 v1 的几何不变，v3 的几何联动。
配套程序开自动自适应（`setAutoScaleFont(true)`），拖窗口宽度即可观察
档位跳变。

## §7.4 约束：minPx / maxFrac

```cpp
v4.child<0>().self.size = 250.0f;
v4.child<0>().self.minPx = 150.0f;    // 收缩下限
v4.child<0>().self.maxFrac = 0.35f;   // 占容器上限 35%
```

空间充足时按 preferred 250；窗口变窄先向 150 收缩；再窄按 35% 封顶——
侧栏既不会无限膨胀也不会挤爆主区。

## §7.5 版本切换：When 包裹整棵版本树

四个版本各是一棵独立的树，用 `When` 包裹后挂进主树；`sel` 决定哪个
可见（不可见的 When 不布局、不渲染，零开销）：

```cpp
root.child<3>().node.add(v1.node);
// ...
root.child<3>().self.show = (sel == 0);
```

## §7.6 Wrap / Grid 与任意嵌套

除分格外还有两种布局容器：

```cpp
Mk::wg::Wrap<Lbl, Lbl, Lbl> w;        // 流式：超宽自动换行（行高 = 行内最高子）
Mk::wg::Grid<3, Lbl, Lbl, Lbl, Lbl> g; // 网格：3 列均分宽，行数按子数自增
```

**嵌套坐标系**：控件渲染在"当前窗口"坐标空间。容器嵌容器（VBox>HBox>Split…）
共享父空间；SubWindow/Panel/弹窗开启新空间，内部坐标以自身为原点重起算
（`Node::origin` 由父容器逐层传播）。因此"容器嵌容器、容器嵌子窗口、
分格嵌子窗口"任意组合都正确——把一棵独立根树嵌入某控件时，host 起点传
该控件的 `node.origin` 即可（gallery 的 PageHost 就是范例）。

## 练习

1. 给 v2 的固定格加 `minPx = 100`，拖窄窗口观察收缩行为。
2. 把 v3 的工具栏改成 `em(88)` 双倍高，验证字号联动。
3. 新开一个版本 v5：HSplit 里嵌 VSplit（先横后竖），体会任意嵌套。

