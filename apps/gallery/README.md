# mkui Widget Gallery

wg2 控件库的演示外壳：一个主窗口 + 一个检查器子窗口，29 个演示页按分组挂在左侧导航。

## 目录职责

| 文件 | 职责 |
|---|---|
| `src/main.cpp` | 程序入口：建窗口、建页面对象、注册到导航 |
| `src/gallery_app.h` | 应用壳 `GalleryWindow`：菜单栏 / 侧栏 / 页面宿主 / 状态栏 / 弹窗 |
| `src/gallery_chrome.h` | 页面骨架：`SectionHead` / `PageDemo` / `CodeBlock` / `Page`，以及 `bindPage()` |
| `src/gallery_wg2_bridge.h` | 导航列表 `PageNavWg2` 与页面宿主 `PageHostWg2` |
| `src/gallery_inspector.h` | 检查器窗口（独立 ImGui 上下文）及其界面树 |
| `src/snippets.h` | 每页右下角 "Code" 区展示的示例代码 |
| `src/pages/page_NN_*.h` | 单个演示页，一页一文件 |

## 页面契约

每个页面是一个自包含的结构体，**自己持有演示数据**：

```cpp
struct PageText {
    using Root = Mk::wg2::VBox<Mk::wg2::Id("root"), /* ... */>;

    Page page;      // 页架：标题 / 描述 / 代码块
    Root root;      // wg2 控件树

    PageText() {                       // 无参；需要窗口就传 Mk::window*
        page.setup("文本显示", "描述", snips::kText);
        root.at<Mk::wg2::Id("label")>().text = "hi";
        bindPage(page, root);          // 唯一装配入口：layout + hint
    }
};
```

约定：

- **数据归页面**：不在页面之间共享状态；需要别的组件的数据就拿到显式引用或订阅事件。
- **构造参数是引用或指针时**，必须存成成员再用 `[this]` 捕获——`[&]` 捕获形参会悬垂（历史上两次崩溃都源于此）。
- 布局/绘制一律走 `bindPage(page, root)`，页面里不出现 `layout()` / `hint()`。

## 加一个页面

1. 新建 `src/pages/page_NN_<name>.h`，按上面的契约写结构体；
2. 在 `src/gallery_pages.h` include 它；
3. 在 `src/main.cpp` 声明对象并 `window.addPage(pageX, gallery::kGroupXxx);`；
4. 需要示例代码就在 `src/snippets.h` 加一段 `std::string_view`。

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
