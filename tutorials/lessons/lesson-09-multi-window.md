# 第 09 课：多窗口编程

配套程序：`src/tutorial09_multiwindow.cpp`。多窗口的要点：**每个窗口
独立的 ImGui 上下文与帧循环**，但**共享同一组外部变量**——这就是
跨窗口实时联动的全部秘密。

## §9.1 构造即注册

```cpp
Mk::MainWindow<> main("主窗口", 1200, 800);
Mk::window inspector("检查器", 420, 560);
```

两个窗口构造完即注册进全局引擎；`Mk::mkui<Mk::window>::run()` 逐帧驱动
两者。引擎的注册表按窗口类型分组（`mkui<WindowT>` 模板）。

## §9.2 每窗口独立主题

每个窗口一个 ImGui 上下文 → 各持一套皮肤：

```cpp
main.setTheme(Mk::wg::Theme::dark());
inspector.setTheme(Mk::wg::Theme::light());
```

## §9.3 共享状态：两棵 wg 树绑同一组变量

```cpp
float volume = 0.6f;
bool mute = false;

// 主窗口树
mainTree.child<1>().self.value = &volume;
// 检查器树
insTree.child<1>().self.value = &volume;
```

拖动任一个窗口的滑块，另一个同帧跟随——两个控件读写同一个 `float`。
帧计数（`mainFrames`/`insFrames`）各自独立：**状态共享、帧循环独立**。

## §9.4 否决模式：X 只隐藏

```cpp
glfwSetWindowCloseCallback(insp, [](GLFWwindow* w) {
    glfwSetWindowShouldClose(w, GLFW_FALSE);   // 否决关闭请求
    glfwHideWindow(w);                         // 只隐藏
});
```

配套程序用复选框切换：启用时点 X 只隐藏；停用时恢复默认（`nullptr`
回调）。setter 每帧同步（幂等）。

## §9.5 跟随退出

主窗口关闭后（引擎销毁其状态），检查器下一帧检测到
`main.getHandle() == nullptr` → 自己 `shouldClose` → `run()` 返回：

```cpp
inspector.setUserUI([&] {
    if(main.getHandle() == nullptr) {
        glfwSetWindowShouldClose(inspector.getHandle(), true);
        return;
    }
    ...
});
```

## 练习

1. 加第三个窗口，主题换成 accent 红，绑定同一 `volume`。
2. 把否决模式也用到主窗口（点 X 隐藏主窗口，检查器菜单唤回）。
3. 观察：主窗口关闭瞬间检查器的"主窗帧"停在哪一帧（帧循环独立）。
