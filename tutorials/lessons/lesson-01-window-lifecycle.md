# 第 01 课：窗口生命周期

配套程序：`src/tutorial01_lifecycle.cpp`。本课回答四个问题：

1. `Mk::window` 对象到底是什么？
2. 点 X 之后发生了什么？
3. 为什么"关闭"会立即让所有句柄失效？
4. 怎么拦截 X（点 X 只隐藏，程序继续跑）？

## §1.1 window 是共享句柄，不是窗口本身

先建立一个准确的心智模型：

```text
Mk::window  ──(shared_ptr)──▶  window::Impl
                                 ├─ GLFWwindow*   真正的 OS 窗口
                                 ├─ ImGuiContext* 这个窗口专属的 ImGui 上下文
                                 └─ m_UserUI      每帧 UI 回调
```

- **你手里的 `Mk::window` 对象只是把手**（handle）。拷贝它不拷贝窗口，
  两个把手指向同一个 `Impl`。
- **引擎也持有同一个 `Impl` 的 shared_ptr**（第 00 课的"构造即注册"）。
  所以窗口对象既不归你独有，也不归引擎独有——双方共享。
- `Impl` 才是窗口的本体；它活到**最后一个 shared_ptr 释放**为止。

这个设计带来一个重要推论：**同一窗口可以有任意多个把手，且都有效**。

## §1.2 关闭 = 立即销毁真实资源

mkui 里"关闭窗口"是**真关闭**，不是标记待关闭：

```cpp
mainWindow.close();   // 或用户点了 X，引擎循环发现后同样调用它
```

`close()` 执行后（同步完成）：

- GLFW 窗口被销毁 → `getHandle()` 立刻变 `nullptr`；
- ImGui 上下文被销毁 → 所有指向它的把手只剩空壳；
- 引擎在下一次循环检查时把它移出注册表（`shouldClose()` 恒真）。

**注意：`Impl` 对象本身还活着**（shared_ptr 引用计数未归零），
所以你还能安全地调用把手上的查询方法——它们只是查不到东西了：

```cpp
mainWindow.close();
mainWindow.getHandle();      // nullptr，不再崩溃也不再指向已销毁资源
mainWindow.shouldClose();    // true
```

为什么这样设计？因为"窗口死了但对象还在被引用"是 GUI 程序的常态
（用户随时可能关掉任何窗口，而你的代码可能还持有它的引用）。
共享把手 + 空壳化让这类场景天然安全。

## §1.3 实验观察（配套程序 §1.3 区）

程序窗口左侧有一个演示区，点"手动关闭窗口"按钮前后观察状态表：

| 查询 | 关闭前 | 关闭后 |
|---|---|---|
| `getHandle()` | 非空地址 | `nullptr` |
| `shouldClose()` | false | true |
| 把手能否继续调用方法 | ✓ | ✓（查询类安全，空操作） |

（按钮实际触发的是 `glfwSetWindowShouldClose(handle, true)`——
**在 UI 回调里永远不要直接调 `window.close()`**，原因见 §1.5。）

## §1.4 X 拦截否决：检查器模式

很多时候你不想让用户真的关掉某个窗口（比如第二顶层窗口"检查器"，
用户点 X 只是暂时不想看）。标准做法是拦截关闭请求：

```cpp
GLFWwindow* h = inspector.getHandle();
glfwSetWindowCloseCallback(h, [](GLFWwindow* w) {
    glfwSetWindowShouldClose(w, GLFW_FALSE);   // 否决！不进入引擎销毁流程
    glfwHideWindow(w);                          // 只是隐藏
});
```

配套程序按 "V" 键或点按钮切换否决模式，开启后点 X 窗口只是隐藏，
再按 "S"（或按钮）重新显示。这是 gallery 应用 Inspector 窗口的真实模式。

**配套的隐藏窗口如何退出程序**：主窗口关闭后，检查器窗口的 UI 回调
里发现主窗口 `getHandle() == nullptr`，主动请求自身关闭（第 09 课细讲）。

## §1.5 在 UI 回调里关闭窗口的正确姿势

**错误**（会崩溃）：

```cpp
mainWindow.setUI([&] {
    if(ImGui::Button("退出"))
        mainWindow.close();     // 在渲染中途销毁 ImGui 上下文 → 本帧后续代码 UB
});
```

**正确**——只设置"想关"的标志，让引擎在帧间安全时机销毁：

```cpp
mainWindow.setUI([&] {
    if(ImGui::Button("退出"))
        glfwSetWindowShouldClose(mainWindow.getHandle(), true);
});
```

引擎循环在**下一轮迭代开始时**发现该标志，调用 `close()` 真正销毁——
此时本帧渲染早已完成。记住口诀：**回调里说"我想关"，引擎来"真关"**。

这条规则同样适用于从其他线程关窗口：工作线程只能
`glfwSetWindowShouldClose`（第 11 课文件对话框会用到）。

## §1.6 生命周期契约小结

| 角色 | 存活期 |
|---|---|
| `Mk::window` 把手 | 随你的代码（栈/成员/容器均可） |
| `Impl` | 最后一个 shared_ptr 释放（引擎 + 你的把手共同决定） |
| GLFW/ImGui 真实资源 | 从构造到 `close()`，关闭后把手空壳化 |
| `setUI` 的回调对象 | **必须活过 `run()` 全程**（lambda 捕获的变量不能是会在 run 期间析构的临时） |

## §1.7 常见错误

1. **回调里直接 `close()`** → 崩溃。见 §1.5。
2. **以为析构把手会关窗口** → 不会。引擎还持着 shared_ptr；
   想关必须显式触发关闭请求。
3. **否决回调里做重活** → 关闭回调运行在事件处理线程时机，
   只做"标志 + 轻量动作"（hide/show）。
4. **检查 `getHandle()` 一次就认为永远有效** → 窗口随时可能被用户关掉，
   每帧重新检查。

## §1.8 练习

1. 在配套程序里开启否决模式，隐藏窗口后按 S 复活；再关闭否决模式，
   点 X 让程序退出。
2. 改造程序：点"手动关闭"按钮后 3 秒（用帧数估算）自动请求退出整个程序。
   （提示：主窗口关了 `run()` 自然结束……但如果你把唯一窗口关了呢？）
3. 思考：如果两个 `Mk::window` 把手指向同一窗口，其中一个调 `close()`，
   另一个的 `getHandle()` 是多少？（答案：`nullptr`——共享的就是同一个 Impl。）

## §1.9 进一步阅读

- `imgui_mk/include/imgui_mk/window.h` 的 `Impl::close()`：注释详细说明了
  ImGui 后端清理顺序为什么是"load-bearing"（顺序错了会在多窗口下崩溃，
  这是真实修过的 bug，附录 B 有完整故事）。
- `apps/gallery/src/main.cpp`：Inspector 的否决 + 跟随退出全模式。
- 下一课：立即模式——理解了窗口的骨架，该往里面填内容了。