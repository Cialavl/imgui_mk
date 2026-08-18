# 第 06 课：完整控件集巡礼

配套程序：`src/tutorial06_catalog.cpp`——一个程序装下全部常用控件，
按"输入 / 显示 / 容器 / 弹窗"四区组织，本文按区逐项讲契约。

## §6.1 输入控件：一切皆引用绑定

输入类控件的统一哲学：**绑定外部变量，交互直接写回**。
items 类（radioGroup/combo/listBox）的字符串数组由调用方持有
（`static constexpr` 数组或长期存活容器），存活期覆盖 run()。

| 控件 | 绑定 | 备注 |
|---|---|---|
| `inputText("标签", buf)` | `std::array<char,N>&` | 容量编译期可知；配合 `withMinWidth/widthFrac` 控宽 |
| `inputInt/inputFloat/inputDouble` | `int&/float&/double&` | |
| `inputInt2/inputFloat2` | `std::array<T,2>&` | vec2 场景 |
| `dragF/dragI` | 数值引用 + speed/min/max | 拖拽改值 |
| `slider("标签", v, mn, mx)` | `float&` | 还有 sliderInt/sliderAngle |
| `radioGroup(items, current)` | `int&` | 单选组 |
| `combo/listBox("标签", current, items)` | `int&` + items span | 选中下标 |

配套程序 §6.1 区每个控件都接了实时回显（labelText 显示绑定变量当前值）——
改任何输入，回显同帧变化，这是"绑定"最直观的证明。

## §6.2 显示控件：状态外置的只读视图

| 控件 | 数据来源 | 备注 |
|---|---|---|
| `label(text)` / `labelEllipsis(text)` | 持字符串副本 | ellipsis 版超宽截断（§5.1） |
| `textColored(text, color)` / `textWrapped(text)` / `bulletText(text)` | 副本 | 变体样式 |
| `labelText("名", "值")` | 两段副本 | 属性表风格 |
| `progressBar(fraction, size, overlay)` | `float&` | 绑定引用可被逻辑侧驱动 |
| `selectable("行", selected)` | `bool&` | 选中态外置 |
| `plotLines("名", values, size)` | `const std::vector<float>&` | 历史曲线；逻辑侧 push 数据 |
| `image(tex, size)` / `imageButton(id, tex, size, cb)` | `ImTextureID` | 纹理经 `loadTextureRGBA` 上传 |
| `separator()` / `separatorText("标题")` | — | 分隔 |

纹理辅助（`fui_display.h`）：

```cpp
ImTextureID tex = Mk::fui::loadTextureRGBA(rgbaPixels, w, h);  // 上传
Mk::fui::deleteTexture(tex);                                    // 释放（GL 上下文存活期间）
```

## §6.3 容器：结构即布局

第 03 课讲过 OOP 容器；函数式同款（子视图变参收入 tuple）：
`vbox/hbox/group/collapsing/subwindow`，以及进阶容器
`tabBar/tab`（仅激活页渲染）、`textTable`（数据驱动表格）、
`fnTable`（cell 函数泛型表格）、`menuBar/menu/menuItem`（菜单栏，
注意放 UI 树开头——它画完后把宿主光标下移，避免与内容重叠）、
`panel`（宿主内可拖动浮动面板）。

```cpp
// fnTable：单元格是函数，数据来源任意（本课程序的表格区）
fnTable("表", kHeaders, 3, [](int r, int c) {
    return std::format("r{}c{}", r, c);
});
```

`textTable` 绑定 `const std::vector<std::vector<std::string>>&`——
逻辑侧改 vector，表格内容每帧跟随（配套程序演示实时刷新的表）。

## §6.4 弹窗与颜色：标志触发式

popup 的设计：**bool 标志触发**——置位当帧 `OpenPopup` 后自动回落；
弹窗内容每帧渲染，直到某个回调里 `CloseCurrentPopup()`（gallery 的
`ClosePopupButton` 控件就是干这个的）。

```cpp
bool aboutOpen = false;
button("关于", [&]{ aboutOpen = true; });      // 触发
popup("about", aboutOpen,                      // 内容（每帧）
    label("..."),
    CloseButton{});                            // 自定义控件：点了就关
```

模态版 `popupModal(id, open, flags, 子视图...)` 阻塞宿主交互。
**注意**：模态打开会强制关闭已打开的普通弹窗（ImGui 模态独占语义）——
不要同时常驻两者。

颜色控件绑定 `std::array<float,3|4>`：`colorEdit3/4`（紧凑编辑）、
`colorPicker3/4`（完整拾取器）。`checkboxFlags("名", flags, mask)`
绑定 `int&` 按位切换——位标志面板的标配。

## §6.5 一个真实面板的组装节奏

配套程序最后一块"综合区"按真实开发节奏组装：

1. 先摆骨架（vbox + 分区 separatorText）；
2. 填输入（滑块/勾选/下拉，全部绑定状态）；
3. 接显示（进度/曲线回显状态）；
4. 包修饰（when 控制高级区、disabled 控制关联项）；
5. 挂弹窗（about/settings 触发按钮 + popup 内容）。

## §6.6 常见错误

1. **items 数组是局部变量**：`combo` 收到的 span 指向已析构的栈数组。
   items 必须 static/全局/长期存活。
2. **plotLines 的 vector 在 setUI 里每帧重建再填**：能跑但浪费；
   正确姿势是外置 vector + 逻辑侧 push/裁剪（滚动窗口）。
3. **menuBar 放树中间**：光标下移发生在它画完的位置——放最开头。
4. **弹窗想"打开时做一次性初始化"**：popup 内容每帧执行！一次性逻辑
   放触发按钮回调里（置位标志旁边），别放弹窗内。
5. **模态+普通弹窗同时常驻**：见 §6.4，模态会关掉普通弹窗。

## §6.7 练习

1. 给综合区加一个 ListBox 选择"主题预设"（假数据即可），选中项显示在
   labelText。
2. plotLines 区：把 sin 换成两个 sin 叠加，观察曲线形状。
3. 挑战：点表格某行弹出该行详情 popup（提示：fnTable 的 cell 函数里
   无法直接放 Selectable——改用 textTable + 行前 button，或自定义 cell 控件）。

## §6.8 进一步阅读

- `apps/gallery/src/gallery_pages.h`：38 个演示页 = 本课的完全体。
- 附录 A：全部 API 的速查表。
- 下一课：布局系统——让控件在窗口大小/字号变化时依然优雅。