# mkui 教学课程

面向 C++ 开发者的 mkui 库系统教学：从第一个窗口到自定义控件，共 12 课 + 2 附录。
每课 = 一篇细致文档 + 一个可运行的配套程序（`src/tutorialNN_*.cpp`），
程序内注释标注了对应的文档章节号（如 `// §2.3`），边读边跑。

## 学习路径

```
基础篇                          函数式篇                 布局与自适应篇            进阶篇
00 环境与首个窗口 ─┬─ 04 函数式核心思想 ─┬─ 07 布局系统 ──────┬─ 09 多窗口编程
01 窗口生命周期 ───┤                    │                    ├─ 10 双库互操作
02 立即模式 ───────┴─ 05 组合子与修饰器 ─┤                    ├─ 11 本地文件对话框
03 OOP 控件树 ─────── 06 完整控件集巡礼 ─┴─ 08 主题与全动态自适应 ─ 12 扩展你自己的控件
```

## 课程索引

| 课 | 文档 | 配套程序 | 你将学到 |
|---|---|---|---|
| 00 | [lesson-00](lessons/lesson-00-setup-first-window.md) | `tutorial00_first_window` | 构建环境、最小窗口、引擎自动注册 |
| 01 | [lesson-01](lessons/lesson-01-window-lifecycle.md) | `tutorial01_lifecycle` | 共享句柄、关闭即销毁、X 拦截否决 |
| 02 | [lesson-02](lessons/lesson-02-immediate-mode.md) | `tutorial02_immediate` | 立即模式心智模型、每帧回调契约 |
| 03 | [lesson-03](lessons/lesson-03-oop-widget-tree.md) | `tutorial03_oop_tree` | OOP 控件树、墓碑动态增删 |
| 04 | [lesson-04](lessons/lesson-04-functional-core.md) | `tutorial04_functional` | 值语义控件、自由函数 draw、ADL 扩展 |
| 05 | [lesson-05](lessons/lesson-05-combinators.md) | `tutorial05_combinators` | 修饰器组合：disabled/when/withId… |
| 06 | [lesson-06](lessons/lesson-06-widget-catalog.md) | `tutorial06_catalog` | 输入/显示/容器/弹窗全 API 巡礼 |
| 07 | [lesson-07](lessons/lesson-07-layout.md) | `tutorial07_layout` | em 设计像素、hsplit/vsplit、min/atMost |
| 08 | [lesson-08](lessons/lesson-08-theme-adaptive.md) | `tutorial08_theme` | Theme、字号等比、DPI、运行时档位 |
| 09 | [lesson-09](lessons/lesson-09-multi-window.md) | `tutorial09_multiwindow` | 多顶层窗口、共享状态联动 |
| 10 | [lesson-10](lessons/lesson-10-bridges.md) | `tutorial10_bridges` | dyn()/asWidget() 双向桥 |
| 11 | [lesson-11](lessons/lesson-11-file-dialog.md) | `tutorial11_filedialog` | 异步文件对话框状态机 |
| 12 | [lesson-12](lessons/lesson-12-custom-widgets.md) | `tutorial12_custom` | ADL 三步法扩展自己的控件 |
| 附A | [appendix-a](lessons/appendix-a-api-reference.md) | — | API 速查表 |
| 附B | [appendix-b](lessons/appendix-b-pitfalls.md) | — | 常见陷阱 FAQ（踩坑实录） |

## 构建与运行

前置要求与主仓库一致（CMake 4.1.1+、C++23、MSVC、vcpkg 提供 glfw3/assimp），
教程目标随主工程一起构建：

```
cmake --build out/build/x64-Debug --target tutorial00_first_window
out\build\x64-Debug\tutorials\tutorial00_first_window.exe
```

也可以一次构建全部教程：

```
cmake --build out/build/x64-Debug --target tutorial00_first_window tutorial01_lifecycle ...
```

命令行构建需要 MSVC 环境（cl.exe 不在 PATH），用 VS 开发者命令行或参照
AGENTS.md 的 VsDevCmd 包装方式。

## 约定

- 教程程序均为交互式（关窗退出），无断言压力；鼓励按文档"试着改 XXX"实验点动手。
- 文档与程序注释均中文；库源码内中文注释是权威契约。
- 深入阅读：`test/windowstest4..15.cpp`（每例一个特性的确定性验证）、
  `apps/gallery`（38 页全量交互展示）——文档"进一步阅读"小节会指路。