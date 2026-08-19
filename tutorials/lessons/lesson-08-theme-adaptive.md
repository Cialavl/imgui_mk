# 第 08 课：主题与全局动态自适应

配套程序：`src/tutorial08_theme.cpp`。主题是纯值类型（`Mk::wg::Theme`），
应用粒度两层：窗口级（`setTheme`）与子树级（`Styled` 控件）。

## §8.1 Theme：预设与链式派生

```cpp
Mk::wg::Theme::dark();                                  // 深色预设
Mk::wg::Theme::light();                                 // 浅色预设
theme.withAccent(ImVec4(1, 0, 0, 1));                   // 链式派生
theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f);   // 几何微调
```

`Theme` 含 17 组颜色 + 几何（圆角/边距/间距/字号）。几何字段默认 -1 =
按字号比值推导；显式写入绝对值则优先。

## §8.2 窗口级换肤

```cpp
Mk::wg::setDefaultTheme(Mk::wg::scaledForMonitor(Mk::wg::Theme::dark()));
// 须在首个窗口创建前：窗口构造时读取它烘焙字体位图
mainWindow.setTheme(theme);   // 运行时换肤：每窗口独立 ImGui 上下文
```

`setTheme` 只换皮肤不换字号（字体位图不重建，几何按当前烘焙字号推导）。
配套程序的 accent 编辑控件编辑即整窗换肤（`ColorEdit3` 返回 true 时
`withAccent` + `setTheme`）。

## §8.3 字号等比联动：rescale

```cpp
theme = theme.withFontSize(fontSize);
mainWindow.setTheme(theme);
mainWindow.getWindow().rescale(fontSize);   // 重烘焙字体位图
```

`rescale` 重烘焙字体图集并按新字号重应用主题——文字/控件/间距/边距全部
等比。全量 CJK 图集重烘焙约 0.3~0.5s 一次性卡顿，属预期（应用用
量化档位控制频率）。

## §8.4 Styled：子树级换肤

```cpp
Mk::wg::Styled<Mk::wg::VBox<Mk::wg::Button, Mk::wg::Button>> st;
st.self.theme = theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f);
```

渲染时 push 主题全部颜色与几何变量，子树画完配对 pop——局部紧凑区、
局部 accent 覆盖都靠它，全局不受影响。

## §8.5 全动态自适应

```cpp
mainWindow.getWindow().setAutoScaleFont(true);   // 总开关（opt-in）
```

开启后两路联动：跨屏拖动/系统缩放变化时字号与窗口尺寸同步重标定；
窗口宽度跨档位（900/1400/2000）时字号量化升降。测试代码不开启以保持
像素断言确定性；配套程序开启以演示。

## 练习

1. 自定一套主题（改 windowBg/accent/border），`setDefaultTheme` 应用。
2. 用 `Styled` 在页面里做一块"危险区"（红色 accent + 大圆角）。
3. 观察：rescale 后 `em()` 尺寸如何变化（提示：与字号同源）。
