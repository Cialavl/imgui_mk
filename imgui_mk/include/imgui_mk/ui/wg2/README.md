# Mk::wg2

按“纯编译期静态树 + 动态按需 opt-in”方案新增的控件层。旧的 `Mk::wg` 保持兼容。

## 已完成的第一阶段

- `core.h`
  - `FixedString` 编译期 ID：`Button<Id("save")>`
  - 控件状态 `Chrome`（`visible / enabled / stretch / size overrides / style`）：
    由引擎按「类型 + 地址」登记，统一经 `widgetChrome()` 读取；控件本身不携带状态成员
  - `WidgetStyle`：每控件样式覆盖（文字/底色/悬停/按下/边框/圆角/边框粗细/内边距/间距/字体）
  - 布局基础类型与渲染深度保护
- `widget.h`
  - `StaticWidget` concept（完整协议，或只声明 `root` 的组合控件）
  - `drawWidget / layout`
  - `hintOf / policyOf`：尺寸/策略统一入口（无 hint/policy 时自动转发到 `root`）
  - `renderable(w)`：渲染开关（引擎 `setVisible/setVisibleFn` + 控件可选 `shouldRender()`）
  - `styleOf / setStyle / clearStyle`：所有控件通用（原子/容器/组合）的样式设置接口
  - 组合控件：有 `root` 成员或 `root()` 访问器时，hint/policy/绘制全部由框架转发；`overlays()` 返回的浮层只每帧提交（不进布局）
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
  - `CheckBox / InputText / InputTextMultiline / InputInt / InputFloat / InputDouble / InputInt2 / InputFloat2`
  - `SliderF / SliderInt / SliderAngle / DragF / DragI`
  - `Combo / RadioButton / RadioGroup / ListBox`
- `widgets/display.h`
  - `ProgressBar / TextWrapped / BulletText / LabelText / Selectable`
  - `SeparatorText / Image / ImageButton / PlotLines`（Image/ImageButton 未设 `tex` 时用引擎默认纹理）
  - `Image`：圆角/边框/UV/着色/等比缩放（`fitRect` 居中）
- `mount.h`
  - `attach(root, host)`：登记 root 的父节点；窗口挂载由 `MainWindow::setContent(ui)` 一行代劳
- `tree.h`
  - `setupTree / readyTree`：Qt 式两段式控件树初始化（沿 `children` 递归，组件各自 `setup(ctx)/ready(ctx)`）
- `font.h`
  - `defaultFont()` / `monoFont()`：等宽字体 provider + 回退默认界面字体
- `texture.h`
  - `loadTextureRGBA / deleteTexture`：RGBA8 上传与删除
  - `defaultTexture()` / `textureOr(tex)`：引擎默认棋盘纹理（每窗口一份，懒创建），离线测试安全返回 0
- `widgets/decorator.h`
  - `When / ItemWidth / Indent / Colored / Tooltip`
- `widgets/container.h`
  - `SubWindow / Panel / Tab / TabBar / Collapsing`（`Tab::open` 非空时可关闭）
- `widgets/nav.h`
  - `NavTree`：WinUI NavigationView 风格导航树（分组可展开/收起、叶子选中左侧强调条）
  - `PageHost / PageBook`：页宿主与页簿；页簿用 `HBox<NavTree, PageHost>` 组合，宽度切分交给布局容器（选中页归页簿所有）
- `widgets/composite.h`
  - `Tree / Menu / MenuBar / MenuItem / Popup / PopupModal`
  - `TextTable / FnTable`
- `widgets/dataview.h`
  - `VirtualList`：虚拟列表（ImGuiListClipper 只画可见行、单选/多选、双击/滚动定位）
  - `DataTable`：数据表格（排序表头、可拖列宽、行选中/多选、双击、虚拟滚动、数字感知排序）
- `widgets/selection.h`
  - `SelectionModel`：单选 / Ctrl 多选 / Shift 连选
- `widgets/search.h`
  - `SearchCombo`：可搜索下拉（大小写不敏感子串过滤）
- `widgets/icon.h`
  - `setIconFont / iconFont`：图标字体注册；`iconGlyph(cp)` 码点转 UTF-8；`IconText` 控件
- `widgets/contextmenu.h`
  - `contextMenu / contextMenuWindow`：item 上 / 空白处右键菜单
- `widgets/chart.h`
  - `LineChart / BarChart`：draw list 自绘图表（自动值域、网格、轴标签）
- `widgets/dock.h`
  - `DockSpace`：停靠布局（分割树 + 页签组；拖页签合并、拖叶子四边停靠、拖分隔条调比例；serialize/deserialize）
- `widgets/richtext.h`
  - `parseRichText / RichText`：轻量富文本（**加粗**、代码、可点击链接）
- `style.h`
  - `Theme::withDensity(d)`：密度系数（内边距/间距/滚动条等比缩放）
  - `Theme::tokens()`：语义色 token（bg/surface/border/text/accent/danger/warning/success）
- `i18n.h`
  - `setTranslation / tr`：key → 文本表，未命中回退 key
- `motion.h`
  - `easeOutCubic / easeInOutCubic / Motion / Transition`：缓动、帧率无关趋近、定时过渡
- `widgets/dragdrop.h`
  - `dragSource / dropTarget`：类型安全的拖拽源与接收目标（`payloadType<T>()` 生成负载签名）
- `widgets/dialogs.h`
  - `ConfirmDialog`：模态确认框（确定/取消回调）
  - `ToastHost`：右下角 Toast 堆叠（自动过期/淡出）
  - `Spinner`：不确定进度指示器
- `core.h` / `widget.h`
  - 焦点：`requestFocus(w)` / `hasFocus(w)`（drawWidget 消费，键盘导航已启用）
  - 指针：`Cursor` / `setCursor` / `cursorOnHover`
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
