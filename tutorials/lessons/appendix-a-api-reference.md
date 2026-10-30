# 附录 A：API 速查表

按模块汇总 mkui 全部常用 API。斜体为"已在前文讲解、此处仅索引"。

## 窗口与引擎（window.h / mkui.h / Mainwindow.h / wg_style.h）

| API | 说明 | 课 |
|---|---|---|
| `Mk::window(title, w, h)` | 创建窗口并自动注册；每窗独立 ImGui 上下文 | 00 |
| `window::setUserUI(f)` / `MainWindow::setUI(f)` | 每帧 UI 回调（后调用覆盖） | 02 |
| `window::setTheme(Theme)` | 本窗口换肤（独立上下文互不影响） | 08 |
| `window::rescale(fontSize)` | 请求重烘焙字号（下一帧生效） | 08 |
| `window::setAutoScaleFont(bool)` | 运行时自适应：跨屏 DPI + 窗口尺寸比例（无级；防抖 + 5% 滞回） | 08 |
| `window::setFontLoader(f)` | 自定义字体链（重烘焙时调用） | 12 |
| `window::show()/hide()/close()` | 显示/隐藏/真销毁 | 01 |
| `window::getHandle()/shouldClose()` | 查询：OS 句柄（关闭后 nullptr）/关闭态 | 01 |
| `Mk::mkui<WindowT>::run()` | 引擎循环：驱动所有同类型窗口，全关返回 | 00 |
| `Mk::MainWindow<WindowT>` | 便捷组件：创建 + 注入 UI + 转发全部 API | 00 |
| `Mk::workAreaSize(fW, fH, maxW, maxH)` | 工作区百分比窗口尺寸（封顶） | 08 |
| `Mk::wg::setDefaultTheme(t)` | 全局默认主题（**首窗创建前**调用） | 08 |
| `Mk::wg::Theme::dark()/light()` + `with*()` | 主题预设与链式派生 | 08 |
| `Mk::wg::scaledForMonitor(t)` / `dpiScale()` | DPI 派生 / 主显示器缩放 | 08 |

## wg 体系（ui/wg.h，`Mk::wg`）

### 基座

| API | 说明 | 课 |
|---|---|---|
| `Control<Self, Style, Ts...>` | 组合壳：self（数据）+ node（节点）+ style + children（tuple） | 03 |
| `control.child<I>()` | 编译期子控件访问 | 03 |
| `Node::add/remove/clear` | 运行时增删（墓碑机制，渲染回调内安全） | 03 |
| `Node::findById(id)` | 递归查找（顺带 compact） | 03 |
| `Node::visible / enabled` | 隐藏（不渲染）/禁用（灰色仍渲染） | 03/05 |
| `Node::layout(host)` | 应用入口：measure → arrange → draw（根 origin = host 左上角） | 03 |
| `Node::origin` | 绘制原点（当前窗口坐标空间）：父容器传播，子窗口内局部重起算——任意嵌套坐标链不断 | 07 |
| `Node::role`（CellRole） | 分格角色（Fixed/Stretch measure 时填入） | 07 |
| `Node::fill` | 填充型容器：layout 保留 host 尺寸 | 07 |
| `detail::renderChildrenOnly(n)` | 只画子节点（生命周期探针用） | 03 |

### 原子控件（ui/wg_controls.h）

| 控件 | 绑定/参数 | 课 |
|---|---|---|
| `Label` | 自持字符串（setText/c_str） | 04 |
| `TextColored/TextWrapped/BulletText/LabelText` | 文本变体 | 06 |
| `EllipsisLabel` | 超宽省略号标签 | 05 |
| `Button` | `onClick` 回调（setLabel） | 04 |
| `ImageButton/Image` | `tex` + `size` | 06 |
| `InputText<N>` | 内置 `std::array<char,N>`（容量编译期） | 06 |
| `InputInt/InputFloat/InputDouble` | 数值指针 | 06 |
| `InputInt2/InputFloat2` | `std::array<T,2>*` | 06 |
| `DragF/DragI` | 数值指针 + speed/mn/mx | 06 |
| `SliderF/SliderInt/SliderAngle` | 数值指针（角度绑弧度） | 06 |
| `CheckBox/RadioButton` | `bool*` | 06 |
| `RadioGroup/Combo/ListBox` | `int*` + items（长期存活） | 06 |
| `Selectable` | `bool*` | 06 |
| `ProgressBar` | `float*` fraction | 06 |
| `PlotLines` | `const vector<float>*` | 06 |
| `Separator/SeparatorText` | 分隔 | 06 |
| `CheckboxFlags` | `int*` + mask | 06 |
| `ColorEdit3/4`、`ColorPicker3/4` | `std::array<float,3|4>*` | 06 |
| `loadTextureRGBA(px, w, h)` / `deleteTexture(tex)` | 纹理上传/释放 | 06 |

### 容器与布局

| 容器 | 说明 | 课 |
|---|---|---|
| `VBox/HBox<spacing>` | 垂直/水平布局（模板子组合） | 03 |
| `Group<...>` | 打包整体（重叠布局） | 03 |
| `SubWindow<...>` | 嵌套子窗口（独立滚动） | 03 |
| `Collapsing` | 可折叠容器（子经运行时 add） | 03 |
| `Tree` | 树节点（展开才渲染子） | 03 |
| `TabBar<Tab<T>...>` + `Tab` | 标签页（页名存 node.id，仅激活页渲染） | 06 |
| `Panel<...>` | 浮动面板（ImGui::Begin/End） | 06 |
| `MenuBar<Menu<MenuItem>...>` | 菜单栏（放树首） | 06 |
| `TextTable` / `FnTable` | 数据驱动 / 单元格函数表格 | 06 |
| `HSplit/VSplit` + `Fixed<T>`/`Stretch<T>` | 弹性分格：fixed（设计像素+minPx/maxFrac）/stretch（权重） | 07 |
| `Wrap<...>` | 流式换行布局（超宽自动换行，行高=行内最高子） | 07 |
| `Grid<C, ...>` | 固定列网格（C 列均分宽，行数按子数自增） | 07 |
| `em(px)` / `em2(w,h)` | 设计像素 → 实际像素（×字号/18） | 07 |

### 装饰件与工具

| 装饰件 | 语义 | 课 |
|---|---|---|
| `Tooltip` / `Colored` / `Indent` | 悬停提示 / 上色 / 缩进（子经运行时 add） | 05 |
| `ItemWidth<T>` | 项宽作用域（PushItemWidth 包裹） | 05 |
| `When<T>` | 条件渲染（false 不渲染不占布局） | 05 |
| `Styled<T>` | 主题作用域换肤（push/pop 严格配对） | 08 |
| `FnRender<F>` | 裸 ImGui 绘制桥（F 须可默认构造） | 10 |

### 文件 / 生命周期

| API | 说明 | 课 |
|---|---|---|
| `Mk::wg::FileDialog::open/busy/poll` | 异步文件对话框状态机 | 11 |
| `onCreate/onDestroy/onShow/onHide` | draw 驱动的生命周期回调（成员或 ADL） | 03 |

## 全局设计契约（贯穿全书）

1. **状态外置**：控件绑指针/引用，存活期覆盖 run()（第 02 课）。
2. **items 长期存活**：RadioGroup/Combo/ListBox 的字符串数组 static/全局
   （第 06 课）。
3. **回调里只说"想关"**，引擎来真关（第 01 课）。
4. **setDefaultTheme 在首窗创建前**（第 08 课）。
5. **树在 main 组装一次、长期存活**：每帧只 layout（第 03 课）。
6. **控件树用引用语义**：Control 禁拷贝，对象位置即身份（ID 隔离来源）。
7. **测试代码不开 DPI/自适应**（第 08 课）。

