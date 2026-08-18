# 附录 A：API 速查表

按模块汇总 mkui 全部常用 API。斜体为"已在前文讲解、此处仅索引"。

## 窗口与引擎（window.h / mkui.h / Mainwindow.h）

| API | 说明 | 课 |
|---|---|---|
| `Mk::window(title, w, h)` | 创建窗口并自动注册；每窗独立 ImGui 上下文 | 00 |
| `window::setUserUI(f)` / `setUI(f)` | 每帧 UI 回调（后调用覆盖） | 02 |
| `window::setWidget(ui::Widget&)` | 挂 OOP 控件树（= setUI 包装） | 03 |
| `window::setTheme(Theme)` | 本窗口换肤（独立上下文互不影响） | 08 |
| `window::rescale(fontSize)` | 请求重烘焙字号（下一帧生效） | 08 |
| `window::setAutoScaleFont(bool)` | 运行时自适应：跨屏 DPI + 宽度档位 | 08 |
| `window::setFontLoader(f)` | 自定义字体链（重烘焙时调用） | 12 |
| `window::show()/hide()/close()` | 显示/隐藏/真销毁 | 01 |
| `window::getHandle()/shouldClose()` | 查询：OS 句柄（关闭后 nullptr）/关闭态 | 01 |
| `Mk::mkui<WindowT>::run()` | 引擎循环：驱动所有同类型窗口，全关返回 | 00 |
| `Mk::MainWindow<WindowT>` | 便捷组件：创建 + 注入 UI + 转发全部 API | 00 |
| `Mk::workAreaSize(fW, fH, maxW, maxH)` | 工作区百分比窗口尺寸（封顶） | 08 |
| `Mk::fui::setDefaultTheme(t)` | 全局默认主题（**首窗创建前**调用） | 08 |

## OOP 控件树（ui/widget.h，`Mk::ui`）

| API | 说明 | 课 |
|---|---|---|
| `Widget::draw()` | 统一入口：可见性/禁用/PushID/委托 render | 03 |
| `Widget::add(Widget&)` → `Widget&` | 挂子控件（裸引用）；返回子引用可链式 | 03 |
| `Widget::remove(Widget&)` / `clearChildren()` | 摘除（墓碑）/清空；渲染回调内安全 | 03 |
| `Widget::setVisible/setEnabled/isVisible/isEnabled` | 子树状态 | 03 |
| `VBox/HBox(spacing)` | 垂直/水平布局（spacing<0 用主题默认） | 03 |
| `Group` / `CollapsibleHeader(label, flags)` | 打包整体 / 可折叠容器 | 03 |
| `SubWindow(id, size, border)` | 嵌套子窗口（独立滚动）；setId/setSize/setBorder | 03 |
| `Label/Button/SliderF/CheckBox` | 原子控件（引用绑定外部变量） | 03 |

## 函数式库（ui/fui_*.h，`Mk::fui`）

### 原子控件与工厂（fui_core / fui_inputs / fui_display）

| 工厂 | 绑定/参数 | 课 |
|---|---|---|
| `label(text)` / `labelEllipsis(text)` | 持字符串副本 / 超宽省略 | 04/05 |
| `button(label, cb)` | 点击回调 | 04 |
| `slider(label, float&, mn, mx)` | 滑块 | 04 |
| `checkbox(label, bool&)` | 复选框 | 04 |
| `inputText(label, array<char,N>&)` | 文本输入（容量编译期） | 06 |
| `inputInt/inputFloat/inputDouble(label, T&)` | 数值输入 | 06 |
| `inputInt2/inputFloat2(label, array<T,2>&)` | vec2 输入 | 06 |
| `dragF/dragI(label, T&, speed, mn, mx)` | 拖拽输入 | 06 |
| `sliderInt(label, int&, mn, mx)` / `sliderAngle(label, float&)` | 整数/角度滑块 | 06 |
| `radioGroup(items, int&)` / `radioButton(label, bool&)` | 单选组/单选 | 06 |
| `combo/listBox(label, int&, items)` | 下拉/列表（items 长期存活） | 06 |
| `textColored/textWrapped/bulletText/labelText` | 文本变体 | 06 |
| `progressBar(float&, size, overlay)` | 进度条 | 06 |
| `selectable(label, bool&)` | 可选行 | 06 |
| `plotLines(label, const vector<float>&, size)` | 折线（外置数据） | 06 |
| `image(imageButton)(tex, size, ...)` | 图片/图钮 | 06 |
| `separator()/separatorText(label)` | 分隔 | 06 |
| `loadTextureRGBA(px, w, h)` / `deleteTexture(tex)` | 纹理上传/释放 | 06 |

### 容器与布局

| 工厂 | 说明 | 课 |
|---|---|---|
| `vbox/hbox(spacing, views...)` | 垂直/水平布局（tuple 持子值） | 04 |
| `group(views...)` / `collapsing(label, flags, views...)` | 打包/折叠 | 04 |
| `subwindow(id, size, border, views...)` | 嵌套子窗口 | 04 |
| `hsplit/vsplit(cells...)` | 弹性分格（fixed/stretch） | 07 |
| `fixed(id, size, child).min(px).atMost(f)` | 固定格（设计像素 + 约束） | 07 |
| `stretch(id, weight, child)` / `fixedPx(id, px, child)` | 权重格 / 真像素格 | 07 |
| `em(px)` / `em2(w,h)` / `fill()/fillW(h)/fillH(w)` | 设计像素换算 / 尺寸别名 | 07 |
| `tabBar(id, flags, tabs...)` + `tab(name, view)` | 标签页（仅激活页渲染） | 06 |
| `textTable(id, headers, rows)` / `fnTable(id, headers, n, cellFn)` | 表格 | 06 |
| `menuBar(menu(...))` + `menu(label, items...)` + `menuItem(label, cb)` | 菜单栏（树首） | 06 |
| `panel(name, flags, views...)` | 浮动面板 | 06 |

### 修饰器（组合）

| 修饰器 | 语义 | 课 |
|---|---|---|
| `disabled(view)` | 视觉禁用（仍渲染） | 05 |
| `when(cond, view)` | 条件渲染（false 跳过） | 05 |
| `withId(str, view)` | 显式 ID 作用域 | 05 |
| `itemWidth(px, view)` / `widthFrac(f, view)` / `withMinWidth(px, view)` | 宽度控制 | 05/07 |
| `tooltip(view, text)` | 悬停提示 | 05 |
| `colored(idx, color, view)` / `indent(n, view)` | 上色/缩进 | 05 |
| `styled(theme, view)` / `styleVar(idx, val, view)` | 主题/单变量作用域 | 08 |

### 桥接 / 文件 / 主题

| API | 说明 | 课 |
|---|---|---|
| `dyn(ui::Widget&)` | 函数式树嵌 OOP 子树 | 10 |
| `asWidget(maker)` | OOP 树嵌函数式视图（适配器长期存活） | 10 |
| `FileDialog::open/busy/poll` | 异步文件对话框状态机 | 11 |
| `Theme::dark()/light()` + `with*()` 链式 | 主题预设与派生 | 08 |
| `scaledForMonitor(t)` / `dpiScale()` | DPI 派生 / 主显示器缩放 | 08 |

## 全局设计契约（贯穿全书）

1. **状态外置**：控件绑定引用，存活期覆盖 run()（第 02 课）。
2. **items 长期存活**：radioGroup/combo/listBox 的字符串数组 static/全局
   （第 06 课 §6.6-1）。
3. **回调里只说"想关"**，引擎来真关（第 01 课 §1.5）。
4. **setDefaultTheme 在首窗创建前**（第 08 课 §8.6-2）。
5. **setWidget/setUserUI 同通道**：后调用覆盖，驱动逻辑进树（第 03 课）。
6. **适配器/动态控件长期存活**（第 10 课 §10.6）。
7. **测试代码不开 DPI/自适应**（第 08 课 §8.6-3）。