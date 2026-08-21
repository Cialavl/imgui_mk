# 第 08 课：主题与自适应

wg2 主题工具位于 `wg2/style.h`，当前实现与旧 `Theme` 兼容。

## §8.1 设置全局主题

```cpp
#include <imgui_mk/ui/wg2/style.h>
using namespace Mk::wg2;

setDefaultTheme(scaledForMonitor(Theme::dark()));
```

## §8.2 运行时换肤

```cpp
applyThemeWindow(Theme::light());   // 引擎广播：各窗口自己帧内应用
applyThemeWindow(Theme::dark().withAccent(ImVec4(1,0,0,1)));
```

## §8.3 字号重烘焙

```cpp
engineRescale(24.0f);   // 文字、控件、间距、边距等比缩放
```

## §8.4 局部颜色作用域

```cpp
Colored<Id("col"), Button<Id("btn")>> col;
col.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
```

## 练习

1. 在页面里放 Dark/Light/Accent 三个按钮切换全局主题。
2. 用 `Colored` 做一个不受全局主题影响的局部按钮。
