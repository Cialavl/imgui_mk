# 第 09 课：多窗口编程

配套程序：`src/tutorial09_multiwindow.cpp`——两个顶层窗口，共享状态联动。
本课内容与 gallery 应用的"主窗口 + 检查器"模式完全同源。

## §9.1 引擎如何驱动多个窗口

`Mk::mkui<WindowT>::run()` 的循环驱动**同一类型的所有已注册窗口**：

```text
while (注册表非空) {
    glfwPollEvents();
    for (每个窗口) {
        应关闭 ? 销毁并移出注册表 : render() 一帧
    }
}
```

配套程序构造两个 `Mk::window` 后直接 `run()`——两个窗口各有一份
GLFW 句柄 + ImGui 上下文，引擎逐帧轮流渲染。**run() 在所有窗口
都关闭后才返回**（任一窗口关闭，其余继续运行）。

```cpp
Mk::MainWindow<> main("主窗口", 1200, 800);
Mk::window inspector("检查器", 420, 560);   // 构造即注册（第 00 课）
Mk::mkui<Mk::window>::run();                // 两个窗口一起跑
```

## §9.2 独立上下文：每窗口一套皮肤

每个 `Mk::window` 创建自己的 ImGui 上下文——`setTheme` 只作用于本窗口：

```cpp
inspector.setTheme(Theme::light());   // 检查器浅色
main.setTheme(Theme::dark());         // 主窗口深色（互不影响）
```

配套程序 §9.2 区让两个窗口异肤并排，直观验证"上下文隔离"。
（第 08 课"每窗口独立上下文"的推论在此落地。）

## §9.3 共享状态联动：绑定同一批变量

两个窗口的 UI 回调都按引用捕获同一组状态：

```cpp
float volume = 0.6f;
bool  mute = false;

main.setUI([&] { draw(slider("音量", volume, 0, 1)); });      // 主窗
inspector.setUserUI([&] { draw(slider("音量", volume, 0, 1)); });  // 检查器
```

拖任何一个窗口的滑块 → 另一窗口同帧跟随（读的是同一个 float）。
**这是多窗口联动的全部秘密**：状态在共享作用域，视图各自渲染。

配套程序 §9.3 区：主窗滑块 ↔ 检查器滑块/进度条 ↔ 勾选框，全部绑定
同一组变量。每个窗口还显示"对窗帧号"（`labelText` 读对方计数）——
证明两个窗口是**独立帧循环**（帧号不同步）。

## §9.4 检查器模式：X 只隐藏，跟随退出

真实应用（gallery）里第二窗口的生命周期模式：

```cpp
// 1) X 拦截否决：点 X 只隐藏，不销毁（第 01 课）
glfwSetWindowCloseCallback(inspector.getHandle(), [](GLFWwindow* w) {
    glfwSetWindowShouldClose(w, GLFW_FALSE);
    glfwHideWindow(w);
});

// 2) 菜单/按钮可重新显示
glfwShowWindow(inspector.getHandle());

// 3) 跟随退出：主窗被销毁（引擎 close）后，检查器下一帧自请关闭
inspector.setUserUI([&] {
    if(main.getHandle() == nullptr) {          // 主窗已死
        glfwSetWindowShouldClose(inspector.getHandle(), true);
        return;
    }
    // ...正常 UI
});
```

这样主窗关闭 → run() 干净退出（检查器自请关闭后被引擎销毁），
没有"孤儿窗口"悬挂。配套程序 §9.4 区完整实现了这套模式：
勾选"拦截 X"体验隐藏/复活；不勾选时点 X 即真关。

## §9.5 关闭顺序与清理

- **主窗先关**：检查器自请关闭 → run() 退出 ✓
- **检查器先关**（不拦截时）：主窗继续，run() 不退出 ✓
- **全关**：run() 返回，main 结束，各自 Impl 随 shared_ptr 析构 ✓

清理顺序（第 01 课 §1.2）在多窗口下尤其关键——窗口销毁期间系统向
存活窗口同步发激活消息，ImGui 的 WndProc 钩子读当前上下文无空指针
保护。mkui 引擎已处理（关闭后切换到存活窗口的上下文），你**不需要**
自己处理，但知道为什么多窗口关闭是"load-bearing"即可（附录 B 详述）。

## §9.6 常见错误

1. **共享状态被两个回调同时改**：单线程引擎下不会并发（所有回调都在
   run() 循环内），放心。
2. **把窗口对象建在临时作用域**：run() 前析构 → 注册表空了直接退出。
3. **检查器用 setWidget 后还想 setUserUI**：同一通道，后调用覆盖
   （第 03 课 §3.6）。多窗口同理——每窗口各自一个通道，互不干扰。
4. **X 拦截后 run() 不退出**：正常！还有一个窗口被否决了。要退出就得
   让它自请关闭（跟随退出模式）或显式关闭。

## §9.7 练习

1. 给检查器加一个"主窗主题色"编辑框（第 08 课 accent 控件），
   验证跨窗口 setTheme 只影响主窗。
2. 做第三个窗口"日志"，显示另两个窗口的帧号差。
3. 挑战：主窗关闭时不是让检查器退出，而是把它转正为新主窗
   （提示：需要另一把 shared_ptr 把手 + 状态迁移——想清楚 run() 何时退出）。

## §9.8 进一步阅读

- `apps/gallery/src/main.cpp`：检查器模式的完整实现（否决 + 跟随 + 菜单复活）。
- `test/windowstest6.cpp`：双窗口 + 三层嵌套子窗口的确定性版本。
- 下一课：让 OOP 树与函数式树在同一窗口里互操作。