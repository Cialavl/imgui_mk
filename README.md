# imgui_mk

C++23 GUI 库：封装 ImGui + GLFW + OpenGL，Windows 优先。唯一的控件体系是
`Mk::wg`——无继承的模板 struct 控件（组合壳 `Control<Self, Style, Ts...>`
+ 类型擦除 Node 树 + 两遍布局）。

## 组件

| 目录 | 说明 |
|---|---|
| `imgui_mk/` | 库（头部为主）：`window.h/mkui.h/Mainwindow.h` + `ui/`（wg 体系四头：wg.h / wg_controls.h / wg_style.h / wg_file.h）+ `string_mk/` |
| `apps/gallery/` | 控件画廊（wg 全量展示，双窗口 + 文件对话框 + 主题） |
| `test/` | GUI 示例（test/test2/test3 + test16..20 wg 验证）+ gtest 单元测试 |

## 构建

```bash
cmake -S . -B out/build/x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
# 需 MSVC 环境：VsDevCmd.bat -arch=amd64 包装
cmake --build out/build/x64-Debug --target gallery unit_tests test16
ctest --test-dir out/build/x64-Debug/test --output-on-failure
```

依赖：CMake 4.1.1+、MSVC（C++23）、glfw3（`C:\Program Files\glfw3`）、
Assimp（`C:\Program Files (x86)\Assimp`）。`imgui/`、`glad/` 树内 vendored。
