# 第 04 课：函数式核心思想

配套程序：`src/tutorial04_functional.cpp`。mkui 的第二套控件库 `Mk::fui`
用另一种方式工程化立即模式：**控件是纯值，渲染是自由函数重载**。

## §4.1 对照：同一颗按钮的两种写法

OOP（第 03 课）——对象长期存活，树构造一次：

```cpp
Mk::ui::Button btn("点我", [&]{ ++clicks; });   // 构造一次
root.add(btn);                                   // 挂进长期存活的树
mainWindow.setWidget(root);
```

函数式——每帧从零构造一棵"视图值"：

```cpp
mainWindow.setUI([&] {
    Mk::fui::draw(Mk::fui::vbox(8.0f,
        Mk::fui::label("标题"),
        Mk::fui::button("点我", [&]{ ++clicks; })   // 每帧构造的临时值
    ));
});
```

没有基类、没有虚函数、没有 `new`。`button(...)` 返回一个聚合结构体
（`std::string` + `std::function`），帧结束整体销毁。

## §4.2 分派机制：自由函数重载 + ADL

OOP 用虚表分派；函数式用**重载决议**：

```cpp
struct Label  { std::string text; };
struct Button { std::string label; std::function<void()> onClick; };

void draw(const Label&);    // 编译期决议、可内联
void draw(const Button&);
```

容器持子视图的 `std::tuple`，渲染用折叠表达式编译期展开——
从窗口到你那个按钮的调用链上**一个虚调用都没有**。

**扩展自己的类型零成本**（ADL，第 12 课的主角）：

```cpp
// 你的命名空间里
struct StatusBar { int& frames; };
void draw(const StatusBar& s) { ImGui::Text("帧 %d", s.frames); }

// 立刻能进任何容器/修饰器：
Mk::fui::draw(Mk::fui::vbox(8.0f, StatusBar{frames}, Mk::fui::label("...")));
```

不需要继承任何基类、不需要改库——容器收到的子视图只要"自己的命名空间
里有 `draw(const T&)` 重载"就自动被找到（依赖实参查找）。

## §4.3 使用范式：每帧重建，状态全外置

函数式的总纲（`fui_core.h` 文件头注释的全文翻译）：

> setUI 回调内每帧构造整棵视图值并绘制，状态全部外置于被绑定的变量。
> 视图本身无生命周期负担——引用绑定每帧重建仍实时联动。

配套程序 §4.3 演示同一变量绑定两个控件（窗口上方滑块 + 子窗口内滑块）：
拖任何一个，另一个同帧跟随——因为它们每帧读的都是同一个 `float&`。

## §4.4 容器与工厂组合子

工厂函数推导模板参数，让书写接近声明式：

```cpp
Mk::fui::vbox(spacing, 子视图...)     // 垂直；子视图按值收入 tuple
Mk::fui::hbox(spacing, 子视图...)     // 水平
Mk::fui::group(子视图...)             // 打包整体
Mk::fui::collapsing(label, flags, 子视图...)   // 折叠容器
Mk::fui::subwindow(id, size, border, 子视图...) // 嵌套子窗口
```

容器可任意嵌套（tuple 套 tuple），每层的 `draw(子)` 是依赖调用，
实例化时经 ADL 找到对应重载——**你的自定义控件和库控件在容器里
完全平权**。

## §4.5 什么时候选函数式

| 场景 | 推荐 |
|---|---|
| 固定骨架 + 声明式描述 | 函数式 |
| 运行时频繁增删同构项（动态列表） | OOP（墓碑）或 函数式+容器外置（第 10 课 dyn） |
| 局部样式作用域/条件渲染密集 | 函数式（第 05 课修饰器） |
| 大量长期存活的自定义复合控件 | 两者皆可；OOP 直觉些 |

真实项目常混用（gallery 就是）。

## §4.6 常见错误

1. **把视图值存到 setUI 外面长期复用**：能编译（普通结构体），但违背
   设计意图——视图捕获的是构造时的引用与文本快照；"每帧重建"才是
   保证状态实时联动的契约。想长期存活请用 OOP 或 `asWidget`（第 10 课）。
2. **自定义 draw 重载放进 Mk::fui 命名空间**：可以但不推荐；放自己的
   命名空间靠 ADL 更干净（第 12 课的三步法）。
3. **控件工厂的字符串参数传 `char*` 指向临时 buffer**：工厂参数是
   `std::string`（按值拷贝），安全；但如果你绕过工厂直接聚合初始化并
   存 `const char*`，就要自己保证存活期。

## §4.7 练习

1. 配套程序：把 §4.3 的第二个滑块改成进度条（绑定同一变量）。
2. 写一个 `FrameCounter` 自定义控件（ADL），显示帧号——§4.2 的例子成真。
3. 三层嵌套：`vbox > collapsing > subwindow > hbox(两个按钮)`。

## §4.8 进一步阅读

- `imgui_mk/include/imgui_mk/ui/fui_core.h`：文件头注释 + 工厂区。
- `test/windowstest7.cpp`：函数式最小完整验证（120 帧断言版）。
- 下一课：修饰器——函数式的"装饰靠组合不靠继承"。