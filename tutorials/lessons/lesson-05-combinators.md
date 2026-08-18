# 第 05 课：组合子与修饰器

配套程序：`src/tutorial05_combbinators.cpp`（实际文件名 tutorial05_combinators）。
函数式的精髓一句话：**装饰靠组合，不靠继承**。

## §5.1 修饰器：包一层就多一种行为

修饰器是"持有一个子视图 + 增加一种行为"的包装类型。mkui 提供：

| 修饰器 | 语义 | 典型用途 |
|---|---|---|
| `disabled(view)` | 视觉禁用（灰化不可交互），**子视图仍每帧渲染** | 选项未满足时的控件 |
| `when(cond, view)` | cond 为 false 时**整棵子树本帧跳过** | 条件显示 |
| `withId("x", view)` | 显式 ID 作用域 | 同 label 控件并存 |
| `itemWidth(px, view)` | 子视图内下一个 item 定宽 | 输入框宽度控制 |
| `widthFrac(f, view)` | 下一个 item 宽 = 可用宽×f（1.0 占满） | 占满行 |
| `withMinWidth(px, view)` | 下一个 item 最小宽（内容更宽则不变） | 窄内容撑底 |
| `tooltip(view, text)` | 子视图渲染后，悬停显示提示 | 处处可用 |
| `colored(idx, color, view)` | PushStyleColor 包裹 | 局部上色 |
| `indent(n, view)` | 缩进包裹 | 层级视觉 |
| `styled(theme, view)` | 完整主题作用域（第 08 课） | 局部换肤 |
| `labelEllipsis(text)` | 单行超宽省略截断（配 tooltip 看全文） | 路径/长文 |

用法都是包一下：

```cpp
Mk::fui::disabled(Mk::fui::slider("音量", volume, 0, 1))
Mk::fui::when(isAdmin, Mk::fui::button("删除全部", onDelete));
Mk::fui::tooltip(Mk::fui::labelEllipsis(longPath), longPath);
```

修饰器可以**任意叠加**（先包谁后包谁是有意义的顺序）：

```cpp
tooltip( when(mute, disabled(slider(...)) ), "静音时不可用" )
// = 条件显示 > 悬停提示 > 禁用外观 > 滑块本体
```

## §5.2 disabled vs when：一字之差

- `disabled`：子视图**照常渲染**，只是灰化 + 交互被 ImGui 拦截——
  用户能看见它，知道"这里有个控件但现在不可用"。
- `when`：子视图**根本不渲染**——界面干净，但用户不知道有这个东西。

选择标准：信息是否值得占位。设置项通常用 disabled（可见性预期稳定），
危险操作/高级选项常用 when（不打扰）。

## §5.3 withId：为什么需要它

ImGui 默认用 label 文本生成控件 ID——两个 "确定" 按钮在同一窗口会
**ID 冲突**（只有一个能点）。函数式视图是每帧临时值、没有稳定地址
（OOP 版靠 `PushID(this)` 天然隔离），所以同 label 场景要显式指定：

```cpp
hbox(8.0f,
    withId("save", button("确定", onSave)),
    withId("del",  button("确定", onDelete)));   // 两个"确定"独立可点
```

配套程序 §5.3 放了两个同名按钮，分别计数验证。

## §5.4 宽度三兄弟

| 修饰器 | 语义 | 记忆点 |
|---|---|---|
| `itemWidth(220, v)` | 精确宽 | 像素级 |
| `widthFrac(0.5, v)` | 可用宽的 50% | 比例级 |
| `withMinWidth(200, v)` | 至少这么宽 | 兜底级 |

作用对象是**子视图内下一个 item**（输入框/按钮这类"frame item"；
纯文本不受 PushItemWidth 影响——文本请用容器宽度控制）。
第 07 课会把它们纳入 em 设计像素体系。

## §5.5 组合子思维小结

| 传统 OOP 思路 | 函数式思路 |
|---|---|
| 写一个 DisabledButton 子类 | `disabled(button(...))` |
| 给 Widget 加 tooltip 属性 | `tooltip(widget, "...")` |
| Visibility 布尔成员 + setter | `when(cond, view)` |

行为自由拼装、顺序即语义、零类膨胀。当你发现自己在写第 N 个
"带 XX 特性的 YY 控件"子类时，就该想到修饰器组合。

## §5.6 常见错误

1. **修饰器顺序颠倒**：`when(x, tooltip(v, t))` 与 `tooltip(when(x, v), t)`
   行为不同（后者 x=false 时 tooltip 也消失）。按"从外到内 = 从粗到细"
   推理。
2. **对纯文本用 itemWidth**：不生效（见 §5.4），改用容器/换行控制。
3. **withId 字符串复用**：同一容器里两个 withId 用了相同字符串，
   又冲突了——作用域字符串也要唯一。
4. **disabled 里放会积累状态的东西**：disabled 只拦交互不拦渲染，
   里面如果有每帧自增的探针类控件照样跑。

## §5.7 练习

1. 配套程序：给 §5.3 的两个同名按钮再加第三个（提示：再来一个 withId）。
2. 做一个"三态控件"：`when(a, when(b, disabled(v)))`，a/b 各接一个勾选框。
3. 组合 `colored + indent + tooltip + labelEllipsis` 展示一条系统路径。

## §5.8 进一步阅读

- `imgui_mk/include/imgui_mk/ui/fui_core.h` 修饰器区源码：每个不足 10 行，
  值得全部读一遍——它们是"第 12 课自定义修饰器"的最佳模板。
- `apps/gallery`：几乎每页都在用修饰器。
- 下一课：把输入/显示/容器/弹窗全控件集一次讲完。