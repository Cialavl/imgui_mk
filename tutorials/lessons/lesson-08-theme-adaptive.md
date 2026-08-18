# 第 08 课：主题与全动态自适应（演进式）

配套程序：`src/tutorial08_theme.cpp`。延续第 07 课的方法：先看到
"能跑但不优雅"，再逐层修。本课回答三个问题：

1. 颜色/间距/圆角怎么统一管理？（Theme 值类型）
2. 换肤能作用到哪一层？（窗口级 / 子树级）
3. 字号与几何如何随 DPI 和窗口尺寸自适应？（全动态链路）

## §8.1 演进第一步：把观感参数从硬编码中解放

没有主题系统时，颜色/间距散落在各处。mkui 用 **Theme 纯值类型**
统一管理（`fui_style.h`）：

```cpp
struct Theme {
    ImVec4 windowBg, frameBg, text, button, ...;   // 约 20 项颜色
    float  rounding, frameRounding, ...;            // 几何
    float  fontSize;                                // 基准字号（第 8.4 节主角）
    static Theme dark();   static Theme light();    // 预设
    Theme withAccent(ImVec4) const;                 // 链式派生（值语义副本）
    Theme withRounding(float) const;
    Theme withItemSpacing(x, y) const;
    Theme withFontSize(float) const;
    // ...
};
```

用法（窗口级，替换窗口默认观感）：

```cpp
mainWindow.setTheme(Theme::dark().withAccent(ImVec4(0.2f, 0.9f, 0.4f, 1)));
```

配套程序 §8.1 区：深/浅主题切换 + 主色拾取器实时换肤
（`colorEdit3` 改 `col3` → 用 `withAccent` 派生新主题 → `setTheme`）。
每窗口有独立 ImGui 上下文，**不同窗口可以不同皮肤**（第 09 课验证）。

## §8.2 作用域换肤：styled 修饰器

窗口级 setTheme 是全局的；局部区域要"另起一套观感"用 `styled`：

```cpp
draw(styled(Theme::dark().withItemSpacing(2, 2).withRounding(0),
    vbox(2.0f,
        button("紧凑区按钮", ...),      // 这一区间距 2、直角
        ...)));
```

`styled` 内部 Push 全部主题色 + 几何变量，渲染完配对 Pop——
**作用域外不受影响**（配套程序 §8.2 区上下对照：紧凑区 vs 正常区）。
`styleVar(idx, value, view)` 是单变量粒度（如只改 ItemSpacing）。

## §8.3 字号等比几何：Theme 的隐藏主线

主题几何不是孤立的像素值，而是 **fontSize × 比值** 推导：

| 参数 | 比值 | 18px 实值 |
|---|---|---|
| WindowPadding | 0.60F | 10.8 |
| ItemSpacing | (0.58F, 0.34F) | (10.4, 6.1) |
| FramePadding | (0.45F, 0.22F) | (8.1, 4.0) |
| Indent | 1.10F | 20 |

含义：**改一个 fontSize，文字、间距、内边距整体等比联动**——
控件高 = 字号 + 2×内边距 = 26px（18px 时）恒成立。这就是"控件比例
相对文字恒定"的源头（第 07 课 em 的同一思想在主题层的体现）。

配套程序 §8.3 区一个 `slider("主题字号", fontSize, 14, 26)` 试试：
拖动它 → `setTheme(theme.withFontSize(newSize))`——**文字/间距/内边距
同步变化**。（注意：运行时改字号需要重建字体位图——mkui 的 rescale
机制负责，见 §8.5；本区演示的是同一机制）

## §8.4 全动态自适应链路（rescale / 档位 / 跨屏）

字号并非恒定：mkui 提供完整链路让它跟随环境。

```text
最终字号 = 基准字号(18) × 显示器缩放(contentScale) × 宽度档位(bucket)

宽度档位：<900 → 0.75× | <1400 → 1.0× | <2000 → 1.25× | ≥2000 → 1.5×
```

```cpp
// 1) 启动时按显示器 DPI 派生默认主题（opt-in，测试保持确定性）
Mk::fui::setDefaultTheme(Mk::fui::scaledForMonitor(Mk::fui::Theme::dark()));

// 2) 开启运行时自适应（跨屏拖动 + 宽度档位）
mainWindow.getWindow().setAutoScaleFont(true);

// 3) 手动请求重烘焙（任意时机，下一帧生效）
mainWindow.getWindow().rescale(24.0f);
```

配套程序 §8.4 区：自动模式开关 + 手动档位按钮。**拖窗口跨过
900/1400/2000 宽度**，观察字号/间距/控件整体变化；或手动点
"14px/18px/27px" 看 rescale 即时重烘焙（有一瞬卡顿是 CJK 全量图集
烘焙，属预期）。

窗口默认尺寸也可以随显示器工作区自适应（不再超小屏）：

```cpp
const auto [w, h] = Mk::workAreaSize(0.75f, 0.8f, 1440, 880);   // 工作区比例，封顶
```

## §8.5 rescale 的正确用法与代价

- `rescale(newSize)`：**请求**制（任意线程/回调可调），下一帧渲染前
  执行：`Fonts->Clear()`（全量重置，含配置）→ 按新字号重挂字体链 →
  `ImGui_ImplOpenGL3_DestroyFontsTexture()`（GL 后端下一帧自动重建）→
  `applyTheme(theme.withFontSize(newSize))`（几何等比跟随）。
- **代价**：全量 CJK 图集（约 21000 字形）重烘焙约 0.3~0.5 秒卡顿。
  这就是档位设计成"量化阈值"的原因——把重烘焙频率压到最低。
- **字体链可定制**：`setFontLoader([](float size){ ... })`——gallery
  用它把中文链 + 等宽代码字体一起重挂（第 12 课再展开）。

## §8.6 常见错误

1. **setTheme 不重建字体位图**：换肤只改几何/颜色；字号必须走 rescale
   （`setTheme(theme.withFontSize(24))` 后还要 `rescale(24)`）。
   配套程序的 §8.3 滑块两者都调了。
2. **setDefaultTheme 时机**：必须在**首个窗口创建前**调用（窗口构造时
   读取烘焙）；之后调用只影响新窗口。
3. **测试断言踩 DPI**：测试代码不要开自适应/DPI（test4..15 全程不调用
   scaledForMonitor——这就是"opt-in"的设计原因）。
4. **styled 里写死颜色**：作用域换肤要读主题派生的颜色（或用默认值），
   深/浅主题下才都协调。

## §8.7 练习

1. §8.1 区：给主题加一个"边框圆角"滑块（withRounding），验证派生副本。
2. §8.3 区：把滑块改造成"只在 14/18/22/26 四个离散档间跳"（
   `setTheme + rescale` 各调一次）。
3. 挑战：做一个"高对比度"主题预设函数，一键切换后对比观感。

## §8.8 进一步阅读

- `imgui_mk/include/imgui_mk/ui/fui_style.h`：Theme 全字段 + 比值表注释。
- `imgui_mk/include/imgui_mk/window.h`：rescale/processAdaptiveScaling
  实现（40 行）。
- `test/windowstest13.cpp`：rescale 往返 + 档位升降的确定性断言。
- 下一课：多窗口——每窗口一套皮肤、一份状态，共享同一引擎循环。