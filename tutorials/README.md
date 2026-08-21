# mkui 教学课程

面向 C++ 开发者的 mkui 系统化教学：从第一个窗口到 wg2 新控件层，共 13 课 + 2 附录。
每课 = 一篇细读文档 + 一个可运行的配套程序 `src/tutorialNN_*.cpp`（源内
注释标注了对应文档章节号，如 `// §2.3`），两者对照着读。

## 学习路径

```
基础篇                          控件篇                    应用篇
00 环境与第一个窗口  ──►  03 wg 模板对象树  ──►  07 布局系统     ──►  09 多窗口编程
01 窗口生命周期       │   04 wg 原子控件       08 主题与自适应     │   10 桥接与动态树
02 立即模式           ├─► 05 组合与装饰件                        ├─► 11 异步文件对话框
                      │   06 控件目录巡礼                        └─► 12 扩展你自己的控件
```

## 课程目录

| 课 | 文档 | 配套程序 | 你将学到 |
|---|---|---|---|
| 00 | [lesson-00](lessons/lesson-00-setup-first-window.md) | `tutorial00_first_window` | 环境搭建、最小窗口、构造即注册 |
| 01 | [lesson-01](lessons/lesson-01-window-lifecycle.md) | `tutorial01_lifecycle` | 窗口关闭语义、否决回调、真销毁 |
| 02 | [lesson-02](lessons/lesson-02-immediate-mode.md) | `tutorial02_immediate` | 立即模式模型、每帧回调用途 |
| 03 | [lesson-03](lessons/lesson-03-oop-widget-tree.md) | `tutorial03_oop_tree` | wg 模板对象树、Control 组合壳、墓碑增删 |
| 04 | [lesson-04](lessons/lesson-04-functional-core.md) | `tutorial04_functional` | wg 原子控件、指针绑定、状态外置 |
| 05 | [lesson-05](lessons/lesson-05-combinators.md) | `tutorial05_combinators` | enabled/when/ItemWidth/装饰件嵌套 |
| 06 | [lesson-06](lessons/lesson-06-widget-catalog.md) | `tutorial06_catalog` | 输入/显示/容器/弹窗全 API 巡礼 |
| 07 | [lesson-07](lessons/lesson-07-layout.md) | `tutorial07_layout` | 两遍布局、HSplit/VSplit、em、minPx/maxFrac |
| 08 | [lesson-08](lessons/lesson-08-theme-adaptive.md) | `tutorial08_theme` | Theme、字号等比、DPI、运行时换肤 |
| 09 | [lesson-09](lessons/lesson-09-multi-window.md) | `tutorial09_multiwindow` | 多顶层窗口、共享状态、否决模式 |
| 10 | [lesson-10](lessons/lesson-10-bridges.md) | `tutorial10_bridges` | 动态列表、FnRender 裸绘制桥 |
| 11 | [lesson-11](lessons/lesson-11-file-dialog.md) | `tutorial11_filedialog` | 异步文件对话框状态机 |
| 12 | [lesson-12](lessons/lesson-12-custom-widgets.md) | `tutorial12_custom` | 自定义 wg 控件/装饰件/复合控件 |
| 13 | [lesson-13](lessons/lesson-13-wg2.md) | `tutorial13_wg2` | wg2 静态树、字符串路径、RuntimeList opt-in |
| 附A | [appendix-a](lessons/appendix-a-api-reference.md) | — | API 速查表 |
| 附B | [appendix-b](lessons/appendix-b-pitfalls.md) | — | 常见陷阱 FAQ（踩坑实录） |

> 说明：第 03–12 课文档与示例程序描述的是旧 `Mk::wg` 控件层，保留用于兼容学习；
> 新代码请从第 13 课开始使用 `Mk::wg2`。

## 构建与运行

前置要求与主仓库一致：CMake 4.1.1+、C++23、MSVC，vcpkg 提供 glfw3/assimp。
教程目标与主仓库一起构建：

```bash
cmake --build out/build/x64-Debug --target tutorial07_layout
out\build\x64-Debug\tutorials\tutorial07_layout.exe
```
