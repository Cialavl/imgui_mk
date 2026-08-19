# 第 05 课：组合与装饰件

配套程序：`src/tutorial05_combinators.cpp`。本课讲 wg 的组合能力：
禁用/条件渲染/ID 隔离/定宽/嵌套装饰件/省略号。

## §5.1 disabled：Node.enabled

`Node::enabled = false` 用 `BeginDisabled/EndDisabled` 包裹子树——
灰色、不可交互，但**仍然渲染**（与"隐藏"相反）。配套程序里"静音"勾选
时音量滑块禁用：

```cpp
root.child<1>().child<2>().node.enabled = !mute;   // 每帧同步
```

## §5.2 when：条件渲染（When 控件）

`When<T>` 是模板装饰件：`self.show` 为 false 时子树不渲染、不占布局
（measure 返回 0），等价于把整段子树从布局中摘除：

```cpp
Mk::wg::When<Mk::wg::TextColored> danger;
danger.child<0>().self.text = "危险操作";
// ...
danger.self.show = isAdmin;   // 每帧同步
```

配套程序用它对"管理员可见"和"高级参数"两个区域做条件渲染。
注意：`when`（隐藏）与 `enabled`（禁用）语义相反——隐藏连布局都不占。

## §5.3 ID 隔离：无需 withId

wg 的 `Node::draw()` 自动 `ImGui::PushID(this)`——每个节点对象地址即
唯一 ID。两个同样文字的按钮不再需要手动 `withId`：

```cpp
root.child<3>().child<0>().self.setLabel("确认 A");
root.child<3>().child<1>().self.setLabel("确认 B");
```

## §5.4 定宽：ItemWidth 控件

`ItemWidth<T>` 包裹一个输入类控件，`PushItemWidth` 覆盖其宽度：

```cpp
Mk::wg::ItemWidth<Mk::wg::InputText<64>> iw;
iw.self.width = 220.0f;
iw.child<0>().self.label = "定宽 220";
```

## §5.5 嵌套装饰件：tooltip / colored / indent

`Tooltip`、`Colored`、`Indent` 是**非模板**装饰件——子节点经运行时
`node.add` 挂入，从内向外逐层包裹：

```cpp
Mk::wg::Label nested;   nested.self.setText("悬停看提示");
Mk::wg::Indent indent;  indent.self.amount = 24.0f;
Mk::wg::Colored colored; colored.self.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
indent.node.add(nested.node);      // 内 → 外
colored.node.add(indent.node);
root.child<7>().node.add(colored.node);
```

装饰件的渲染约定：push 样式 → 画子节点 → pop 样式（严格配对）。

## §5.6 EllipsisLabel：超宽省略号

`EllipsisLabel` 按自己的 `bounds.w` 截断文本加 `...`（二分查找可行长度），
适合路径/侧栏等窄栏场景：

```cpp
Mk::wg::EllipsisLabel path;
path.self.text = longPath;   // 超宽自动截断
```

## 练习

1. 把"管理员模式"改成三档（`RadioGroup`），危险提示在档位 2 才显示。
2. 给 `ItemWidth` 里的输入框外面再套一层 `Colored`（观察嵌套顺序）。
3. 用 `When<Styled<Button>>` 组合：条件成立时按钮变主题色。
