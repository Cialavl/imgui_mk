# Mk::wg2

按“纯编译期静态树 + 动态按需 opt-in”方案新增的控件层。旧的 `Mk::wg` 保持兼容。

## 已完成的第一阶段

- `core.h`
  - `FixedString` 编译期 ID：`Button<Id("save")>`
  - `WidgetState`：`visible / enabled / stretch / size overrides / style`
  - 布局基础类型与渲染深度保护
- `widget.h`
  - `StaticWidget` concept
  - `drawWidget / layout`
- `static_layout.h`
  - `VBox / HBox / Grid / Wrap / SplitH / SplitV / Overlay`
  - `at<Id("a"), Id("b")>()` 编译期字符串路径访问
  - `child<I>()`
- `dynamic.h`
  - `AnyWidget`：类型擦除包装，只有动态场景才使用
  - `RuntimeList`：渲染期安全 `add / removeAt / findById / pending / tombstone`
  - `RuntimeTabs`：动态页签容器，同样具备渲染期安全增删
  - `RuntimeGrid`：动态网格容器
- `basic_widgets.h`
  - `Label / Button / TextColored / Separator / FnWidget`
- `widgets/input.h`
  - `CheckBox / InputText / InputInt / InputFloat / InputDouble / InputInt2 / InputFloat2`
  - `SliderF / SliderInt / SliderAngle / DragF / DragI`
  - `Combo / RadioButton / RadioGroup / ListBox`
- `widgets/display.h`
  - `ProgressBar / TextWrapped / BulletText / LabelText / Selectable`
  - `SeparatorText / Image / ImageButton / PlotLines`
- `widgets/decorator.h`
  - `When / ItemWidth / Indent / Colored / Tooltip`
- `widgets/container.h`
  - `SubWindow / Panel / Tab / TabBar / Collapsing`
- `widgets/composite.h`
  - `Tree / Menu / MenuBar / MenuItem / Popup / PopupModal`
  - `TextTable / FnTable`
- `widgets/color.h`
  - `ColorEdit3 / ColorEdit4 / ColorPicker3 / ColorPicker4`
  - `CheckboxFlags`
- `widgets/file.h`
  - `FileDialog`（复用旧 `Mk::wg::FileDialog` 实现）

## 使用

```cpp
#include <imgui_mk/ui/wg2/wg2.h>
using namespace Mk::wg2;

using Root = VBox<
    Id("root"),
    HBox<Id("toolbar"), Button<Id("ok")>, Button<Id("cancel")>>,
    RuntimeList<Id("list")>
>;

Root root;
root.at<Id("toolbar"), Id("ok")>().label = "OK";
layout(root, Rect{20, 20, 800, 600});
```

## 验证

- `test/`：gtest 单测按层拆分（见 `test/README.md`）：`wg2_core` / `wg2_components_*` / `wg2_layout_*` / `wg2_loop_*` / `wg2_comm_*`，
  随 `BUILD_TESTING=ON` 编入 `unit_tests`；`ctest -R Wg2` 可按层跑（原 `wg2_smoke`、`wg2_test`、`mkui_test` 已并入该体系）
- 教程程序已移出本仓库（原 `tutorials/`，现位于 `C:\Users\26495\source\repos\tutorials`）
- `apps/gallery/src/pages/page_00..page_21`：旧演示页已逐页迁移到 wg2
- `apps/gallery/src/pages/page_22..page_23`：wg2 新增演示页
- `apps/gallery/src/gallery_wg2_bridge.h`：旧 PageHost ↔ wg2 的桥接节点
- `apps/gallery/src/gallery_app.h`：菜单 / 状态栏 / 侧面板 / 弹窗已迁移到 wg2
- `apps/gallery/src/gallery_inspector.h`：Inspector 窗口 UI 已迁移到 wg2
- `wg2/style.h`：`Theme / setDefaultTheme / scaledForMonitor` 等主题工具（当前转发旧实现）
- `wg2/texture.h`：`loadTextureRGBA / deleteTexture`
- `apps/gallery/src/gallery_state.h`：状态/主题/文件对话框改用 `Mk::wg2`

- `apps/gallery/src/snippets.h`：代码片段已改为 `Mk::wg2`


## 已删除旧兼容层

- `imgui_mk/ui/wg.h`
- `imgui_mk/ui/wg_controls.h`
- `imgui_mk/ui/wg_style.h`
- `imgui_mk/ui/wg_extras.h`
- `imgui_mk/ui/wg_file.h`
- `apps/gallery/src/gallery_pagehost.h`
- 旧 wg 测试源码（`wg_test.cpp` / `windowstest16..22.cpp`）
- 旧离线冒烟与散装单测（`test/wg2_smoke.cpp`、`test/wg2_test.cpp`、`test/mkui_test.cpp`、`test/window_test.cpp`），
  内容已并入 `test/wg2_*_test.cpp` 分层单测
- 旧窗口演示程序（`test/windowstest.cpp`、`windowstest2.cpp`、`windowstest3.cpp`）

## 尚未迁移

- 旧教程配套源码 `tutorial03..12`：教程已整体移出本仓库（`C:\Users\26495\source\repos\tutorials`），
  其中仍使用已移除的 `Mk::wg` 兼容层，在那边以 `find_package(mkui)` 独立构建时自动跳过
