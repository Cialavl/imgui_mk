# mkui Widget Gallery

wg2 控件库的演示外壳：一个主窗口 + 一个检查器子窗口，30 个演示页按分组挂在左侧导航。

## 目录职责

| 文件 | 职责 |
|---|---|
| `src/main.cpp` | 程序入口：设全局主题、构造 gallery 主窗口（构造即建窗并挂载界面）、`window.run()` 跑主循环 |
| `src/gallery_app.h` | `GalleryWindow : Mk::MainWindow`：主窗口，构造里 `setContent(layout)` 一行挂载；`GalleryLayout`：组合界面（三行 + 检查器节点，只装配、不做窗口级设置） |
| `src/content_area/` | 内容区层：内容区组件（页簿 + 30 页数据）+ 演示界面（见下表） |
| `src/menu_bar.h` / `status_bar.h` | 菜单栏（About 弹窗随菜单持有）/ 状态栏；两者都由自己的 `setup(ctx)` 完成装配 |
| `src/tools/inspector_window.h` | 检查器窗口组件：树上的窗口节点（独立 ImGui 上下文），自己持有并挂载窗口 |
| `src/data/snippets.h` | 每页 `Code` 区展示的示例代码 |

### content_area 层

| 文件 | 职责 |
|---|---|
| `content_area/content_area.h` | 界面：`ContentArea` 组件——持有框架页簿（`HBox<NavTree, PageHost>`）与 30 页数据；`setup(ctx)` 注册翻页快捷键、`ready(ctx)` 注入注册 + 页标题服务 + 启动页 |
| `content_area/demo_page.h` | 演示界面：页面骨架（`SectionHead` / `PageDemo` / `CodeBlock` / `DemoPage`）+ 分组常量 + 29 页注册表 |
| `content_area/pages/` | 演示页内容：一页一文件，子目录 = 侧栏分组 |

## 页面契约

每个页面是一个自包含的结构体，**自己持有演示数据**：

```cpp
struct PageText {
    static constexpr const char* title = "文本显示";
    static constexpr const char* desc  = "..一句话说明..";
    static constexpr std::string_view code = snips::kTexts;

    using Root = Mk::wg2::VBox<Mk::wg2::Id("root"), /* ... */>;
    Root root;

    PageText() { root.at<...>().text = "..."; }   // 初始化自己的控件树
};
```

约定：

- **数据归页面**：不在页面之间共享状态；需要别的组件的数据就拿到显式引用或订阅事件。
- **构造参数是引用或指针时**，必须存成成员再用 `[this]` 捕获——`[&]` 捕获形参会悬垂（历史上两次崩溃都源于此）。
- 页面不自绘外壳：标题/描述/代码由 `DemoPage<Content>` 统一装配（`setup()`），演示槽由 `renderTo()` 渲染。

## 加一个页面

1. 新建 `src/content_area/pages/<group>/<name>.h`，按上面的契约写结构体（`<group>` 与侧栏分组同目录）；
2. 在 `src/content_area/demo_page.h` include 它，并加一个 `DemoPage<PageXxx>` 成员；
3. 在同文件的 `registerAll()` 里 `add(pageXxx, kGroupXxx)`——声明顺序即侧栏顺序；
4. 需要示例代码就在 `src/data/snippets.h` 加一段 `std::string_view`。

## 组件间通信

引擎提供类型化事件通道（`on<E>` / `emit<E>`），gallery 用它做三件事：

| 事件 | 发送方 | 接收方 |
|---|---|---|
| `ThemeChanged` | 主题菜单 / 主题页 | 各窗口（自己 `setTheme`） |
| `ToggleInspector` | Window 菜单 / 多窗口页 | 检查器窗口（自己 show/hide） |
| `OpenFile` | File 菜单 / Ctrl+O | 文件页（自持 FileDialog） |

## 调试

- `GALLERY_START_PAGE=<0..28>`：启动即打开指定页，便于按页复现；
- 崩溃自诊断写入 `%TEMP%\opencode\gallery_crash.txt`（字段：异常码、地址、窗口帧号）。
