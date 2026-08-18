# 第 00 课：环境与第一个窗口

配套程序：`src/tutorial00_first_window.cpp`（先构建运行它，再回来读本文）

```bash
cmake --build out/build/x64-Debug --target tutorial00_first_window
out\build\x64-Debug\tutorials\tutorial00_first_window.exe
```

## §0.1 mkui 是什么

mkui 是 C++23 的 GUI 库，封装 ImGui（ Dear ImGui 1.91）+ GLFW + OpenGL：

- **ImGui** 负责控件绘制与交互（立即模式，见第 02 课）；
- **GLFW** 负责操作系统窗口与输入事件；
- **OpenGL 3** 负责实际渲染。

mkui 在这三者之上做了三件事：窗口生命周期管理（`Mk::window`）、
多窗口引擎循环（`Mk::mkui`）、两套控件库（OOP 树 `Mk::ui` 与函数式 `Mk::fui`，
第 03、04 课起分别讲授）。

## §0.2 环境要求

| 依赖 | 要求 | 说明 |
|---|---|---|
| CMake | 4.1.1+ | 根 CMakeLists 用到实验性 C++23 module 支持 |
| 编译器 | MSVC（/std:c++latest） | 实践上仅支持 MSVC |
| vcpkg 包 | glfw3、assimp | `find_package` 由环境提供 |
| 字体 | 系统微软雅黑/黑体/宋体 | 中文显示（库自动回退加载，缺字体时英文不受影响） |

配置与构建（与主仓库相同，教程目标一起构建）：

```bash
cmake --build out/build/x64-Debug --target tutorial00_first_window
```

## §0.3 最小窗口程序逐行走读

配套程序全文只有 20 行左右，我们逐段读：

```cpp
#include <imgui_mk/window.h>     // Mk::window：窗口封装
#include <imgui_mk/mkui.h>       // Mk::mkui：引擎循环
#include <Mainwindow.h>          // Mk::MainWindow：主窗口便捷组件
```

三个头文件的分工：`window.h` 是核心（也间接包含控件树头）；
`mkui.h` 只有一个模板结构体 `mkui<WindowT>`，提供 `run()`；
`Mainwindow.h` 是可选的糖——把"创建窗口 + 注入 UI"合成一个组件。

```cpp
int main() {
    Mk::MainWindow<> mainWindow("我的第一个 mkui 窗口", 1280, 800);
```

这行做了远比看上去多的事（§0.4 详述）：创建 GLFW 窗口、创建独立的
ImGui 上下文、加载中文字体、应用默认主题，并且**自动注册进全局引擎**。

模板参数留空 `<>` 表示默认窗口类型 `Mk::window`。

```cpp
    mainWindow.setUI([&] {
        // 这里的代码每帧执行一次（第 02 课的主题）
        ImGui::Text(...);
    });

    Mk::mkui<Mk::window>::run();   // 引擎循环：直到所有窗口关闭才返回
    return 0;
}
```

`run()` 内部是一个简单循环（伪代码）：

```text
while (注册表里还有窗口) {
    glfwPollEvents();               // 处理操作系统输入事件
    for (每个窗口) {
        窗口收到关闭请求 ? 真正销毁并移出注册表 : 渲染一帧
    }
}
```

所有窗口都关闭后 `run()` 返回，`main` 结束。这就是 mkui 程序的骨架。

## §0.4 构造即注册：引擎与窗口的协作

`Mk::window` 的构造函数最后一行是：

```cpp
Mk::mkui<window>::RegistrationWindow().Registration(m_Impl);
```

含义拆解：

- `mkui<window>::RegistrationWindow()` 返回**按窗口类型区分的单例引擎**。
  同类型窗口共用一个引擎；不同类型（将来你自定义的窗口类）各有一份注册表。
- `Registration(shared_ptr)` 把窗口的内部状态（`Impl`）**共享**给引擎——
  注意是共享不是转移：你手里的 `mainWindow` 在 `run()` 期间、甚至 `run()`
  返回之后都仍然可用（能查 `getHandle()` 判断是否已被销毁）。

因此你**不需要**手动把窗口交给谁，构造完成的那一刻起引擎已经认识它。
这也是为什么教程程序里 `mainWindow` 是普通局部变量而不是 `new` 出来的。

## §0.5 程序里那两行动态文本

配套程序在窗口里显示了两行"活"的信息：

```cpp
ImGui::Text("第 %d 帧", ++frames);                       // 每帧自增 → 数字在跳
ImGui::Text("鼠标：%.0f, %.0f", io.MousePos.x, ...);      // 鼠标坐标实时跟随
```

它们在提醒你一件第 02 课要展开的事：`setUI` 里的代码**每帧都从头执行**，
所以 `++frames` 会持续累加（`frames` 定义在回调外面的栈上）。
如果显示的是静止的"Hello World"，你反而无法区分"跑起来了"和"卡死了"。

## §0.6 常见错误

1. **构建报 `stddef.h` 找不到**：cl.exe 不在 PATH。用 VS 开发者命令行，
   或参照 AGENTS.md 用 VsDevCmd 包装。
2. **窗口一闪而过**：`run()` 前没有任何存活窗口（比如窗口对象在临时
   作用域里构造后立刻析构，且你没有持有 shared_ptr）——注册表为空，
   `run()` 立即返回。教程程序中 `mainWindow` 活到 `main` 结束，不会发生。
3. **中文显示成问号**：老版本库的问题，现由构造函数自动加载系统字体，
   若你在精简版 Windows 上运行需确认 `C:\Windows\Fonts\msyh.ttc` 存在。

## §0.7 练习

1. 把窗口标题改成自己的名字，重新构建运行（验证你的工具链顺手）。
2. 在 `setUI` 里再加一行 `ImGui::Text`，显示你自己定义的变量。
3. 观察：把 `++frames` 改成 `frames + 1`（不自增），显示会怎样？为什么？
   （答案在第 02 课 §2.2。）

## §0.8 进一步阅读

- `imgui_mk/include/imgui_mk/window.h` 构造函数：看"自动注册"那行注释。
- `imgui_mk/include/imgui_mk/mkui.h`：`run()` 全文不足 40 行，值得通读。
- 下一课：窗口的生与死——为什么"关闭"在 mkui 里是个需要认真对待的话题。