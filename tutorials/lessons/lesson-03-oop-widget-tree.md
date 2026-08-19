# 第 03 课：wg 模板对象树

配套程序：`src/tutorial03_oop_tree.cpp`。从本课起讲授 mkui 唯一的控件体系
`Mk::wg`：**无继承的模板 struct 控件**，经组合壳 `Control` 组装成树。

## §3.1 控件 = 模板 struct（数据 + 功能函数）

经典 OOP 用"继承 + 虚函数"做多态；wg 反过来：**控件就是自己的模板
struct**——数据成员（字符串、绑定指针、回调）与功能函数（render/measure/
arrange 等）都在同一个 struct 里，零继承、零虚表：

```cpp
struct ButtonImpl {
    std::string label;
    std::function<void()> onClick;

    void renderImpl(Node& n) { ... }        // 画自己
    Size measureImpl(Node&, const Size&);   // 求期望尺寸
    void arrangeImpl(Node&, const Rect&);   // 记录 bounds
};
using Button = Mk::wg::Control<ButtonImpl, Mk::wg::DefaultStyle>;
```

配套程序里的自定义 `StatusDriver` 就是这种形态：render 时驱动整棵树的
逻辑（刷新状态文本、同步 `settings->visible`）。

## §3.2 组合壳 Control：编译期子节点

`Control<Self, Style, Ts...>` 持有四样东西：

- `self`：控件数据（上面的 struct 实例）；
- `node`：类型擦除的树节点（`Mk::wg::Node`）；
- `style`：样式件（push/pop 作用域）；
- `children`：`std::tuple<Ts...>`——**子控件经模板参数传入**，编译期组合。

```cpp
Mk::wg::VBox<StatusDriver, Mk::wg::Label,
    Mk::wg::HBox<Mk::wg::Button, Mk::wg::Button, Mk::wg::Button>,
    Mk::wg::VBox<Mk::wg::Label, Mk::wg::SliderF, Mk::wg::CheckBox>,
    Mk::wg::SubWindow<>> root;
```

`child<I>()` 按编译期下标访问子控件（`.self` 拿数据、`.node` 拿节点）。
配合索引读写即可在 `main` 里集中配置整棵树（见配套程序的配置段）。

## §3.3 Node：类型擦除行为表

`Node` 不是基类，而是一个**持有函数指针的普通结构体**：

```cpp
void (*m_Render)(Node&);
Size (*m_Measure)(Node&, const Size&);
void (*m_Arrange)(Node&, const Rect&);
// 生命周期：m_OnCreate / m_OnDestroy / m_OnShow / m_OnHide
```

`node.bind<Self>()` 在构造时按 `Self` 类型填充行为表：
`ControlTraits<Self>` 用 `if constexpr` 探测成员函数（成员优先 →
ADL 自由函数 → 缺省兜底）。渲染、布局、生命周期都走这张表——
**添加新控件只需写 struct，库不需要任何改动**（第 12 课详述）。

## §3.4 运行时增删：墓碑机制

模板组合是编译期的；动态列表用运行时 `add/remove`：

```cpp
pool.push_back(std::make_unique<Mk::wg::Label>(...));  // 拥有权池
root.child<4>().node.add(pool.back()->node);           // 渲染回调内 add 安全
root.child<4>().node.remove(pool.back()->node);        // 置空槽位（墓碑），不搬移
```

`remove` 只把槽位置 `nullptr`，不在回调中搬移 vector——这就是"渲染回调内
增删全程安全"的来源；`compact()`（或下一次 `findById`）才真正清理空槽。

## §3.5 布局与渲染入口

```cpp
mainWindow.setUI([&] {
    root.layout(Mk::wg::Rect{20, 20, 1240, 760});   // measure → arrange → draw
});
```

`layout(host)` 是应用侧唯一入口：先 measure 求期望尺寸，再 arrange 由父
容器分配 bounds，最后 draw 渲染。控件树在 `main` 组装一次、长期存活，
每帧只做布局与渲染。

## §3.6 可见性控制

`Node::visible = false` 整段子树跳过渲染（但仍参与布局测量为 0）；
`Node::enabled = false` 用 `ImGui::BeginDisabled` 包裹子树（灰色但可见）。
配套程序的"显示/隐藏设置区"按钮只改布尔，Driver 每帧同步到节点。

## 练习

1. 在工具栏 HBox 里加第四个按钮，`child<I>()` 下标顺延后重新配置。
2. 把"删除末项"改成"删除选中项"（给列表项加点击回调）。
3. 观察：把 `remove` 换成 `clear()`，再在回调里添加新项，会发生什么？
   （墓碑槽被清空后 add 正常追加。）
