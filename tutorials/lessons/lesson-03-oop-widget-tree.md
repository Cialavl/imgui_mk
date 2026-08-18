# 第 03 课：OOP 控件树

配套程序：`src/tutorial03_oop_tree.cpp`。前两课我们手写 ImGui 调用，
当代码量增长，需要结构化手段——第一套是 OOP 控件树（`Mk::ui`）。

## §3.1 Widget：统一 draw() 入口

```cpp
class Widget {
public:
    void draw();                       // 唯一公共渲染入口（非虚！）
    virtual void render() = 0;         // 子类实现这里（protected）
    Widget& add(Widget& child);        // 挂子控件（裸引用，不接管所有权）
    bool remove(Widget& child);        // 摘除（墓碑，见 §3.4）
    void setVisible(bool);             // 子树整体隐藏
    void setEnabled(bool);             // 子树整体禁用（灰化）
};
```

关键设计：`draw()` 是非虚的公共入口，它统一做三件事后委托给虚函数
`render()`：

```text
draw()
 ├─ 不可见 → 整棵子树直接跳过
 ├─ PushID(this) → 以对象地址隔离 ID（同名按钮不再冲突）
 ├─ 禁用 → BeginDisabled() 包裹
 └─ render()   ← 你的实现
```

这保证了**任何控件都免费获得**可见性/禁用/ID 隔离，子类不必重复实现。

## §3.2 组装一棵树

配套程序组装的树（声明顺序 = 组装顺序，全部是 main 栈上的长期对象）：

```text
VBox root
├─ Label status            （每帧 setText 更新文本）
├─ HBox toolRow
│   ├─ Button add("添加")
│   └─ Button del("删除")
├─ CheckBox visibleBox     （控制下面整组）
├─ Group settings
│   ├─ SliderF vol("音量", volume)     ← 引用绑定外部 float
│   └─ CheckBox muteBox("静音", mute)  ← 引用绑定外部 bool
└─ SubWindow panel("列表子窗口")
    └─ VBox listZone      （动态增删区，§3.4）
```

挂载只需一行（`window.h` 的便捷 API）：

```cpp
mainWindow.setWidget(root);   // 等价 setUI([&]{ root.draw(); })
```

## §3.3 数据绑定：控件持外部引用

OOP 控件的"记忆"不在控件里，在你的变量里：

```cpp
float volume = 0.6f;
Mk::ui::SliderF vol("音量", volume, 0.0f, 1.0f);
//                        ^^^^^^ 引用绑定：拖滑块直接写回 volume
```

逻辑侧（比如一个每帧回调）改 `volume`，下一帧滑块自动跟随——
两个方向（UI→状态、状态→UI）由同一条引用打通，无需任何同步代码。

**契约**：被绑定的变量存活期必须覆盖 `run()`（第 02 课规矩的特例）。
配套程序全部状态都在 `main` 栈上，天然满足。

## §3.4 动态树：墓碑机制

列表项随时增删是刚需。难点：增删经常发生在**渲染过程中**（按钮回调
就是渲染的一部分），直接改容器会踩坏正在进行的遍历。

mkui 的解法是**墓碑**：

- `remove(child)`：槽位置空（nullptr），正在渲染的迭代跳过空槽——
  **回调里调用安全**；
- `add(child)`：渲染中追加的孩子本帧不画（数量快照），下一帧生效；
- 每次遍历开始前压实（清掉墓碑槽位）。

配套程序"添加/删除"按钮的回调里直接调 `listZone.add/remove`，
配一个 `std::unique_ptr` 池持有列表项所有权（控件树本身只存引用）。

```cpp
pool.push_back(std::make_unique<Mk::ui::Label>(...));
listZone.add(*pool.back());      // 渲染中调用，安全
listZone.remove(*pool.back());   // 同上
pool.pop_back();                 // 所有权池收缩，控件随之析构
```

## §3.5 容器速览

| 容器 | 布局语义 |
|---|---|
| `VBox(spacing)` | 子控件从上到下；spacing<0 用主题默认 |
| `HBox(spacing)` | 从左到右同一行 |
| `Group` | 打包成整体（不产生边框）参与外部布局 |
| `CollapsibleHeader(label, flags)` | 可折叠区，收起时子树不渲染 |
| `SubWindow(id, size, border)` | 嵌套子窗口（BeginChild），独立滚动 |

`SubWindow` 的 id/size/border 都有运行时 setter；布局尺寸语义
（em、约束）在第 07 课统一深化。

## §3.6 setWidget vs 手写 setUI

`setWidget(root)` 就是 `setUI([&]{ root.draw(); })` 的语法糖——
树在 run 期间长期存活（栈上/成员里），每帧被遍历重画。
所以 OOP 树 = "构造一次的结构 + 每帧的立即模式渲染"，两者并不矛盾：
树提供的是**代码组织结构**，不是保留模式的控件驻留。

## §3.7 常见错误

1. **控件是局部临时对象**：`root.add(Mk::ui::Label("x"));` 挂了个
   析构掉的临时——树里是悬垂引用。控件必须长期存活（main 栈/成员/池）。
2. **绑定临时变量**：`Mk::ui::SliderF("v", float{0.5f}, ...)` 绑了个
   字面量临时。绑定目标必须长期存活。
3. **忘调 setWidget/setUI**：树组装好了但没挂到窗口——什么都看不见。
4. **同容器两个同名控件**：有 PushID(this) 兜底其实**不冲突**了，
   但同名仍影响可读性；改个有意义的名字。

## §3.8 练习

1. 给配套程序加一个"清空列表"按钮（提示：`clearChildren()` + 清池）。
2. 把 `settings.setVisible(false)` 接到一个新按钮上，观察整组消失。
3. 挑战：`SubWindow` 里再嵌一个 `CollapsibleHeader`——树可以任意深。

## §3.9 进一步阅读

- `imgui_mk/include/imgui_mk/ui/widget.h`：本课全部内容的权威源（300 行）。
- `test/windowstest4.cpp`：本课程序的断言版（120 帧自动验证）。
- 下一课：另一种工程化——控件不是对象而是每帧的值，函数式库登场。