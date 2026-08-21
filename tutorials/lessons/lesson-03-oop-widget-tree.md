# 第 03 课：wg2 静态对象树

本课从旧 `Mk::wg` 迁移到 `Mk::wg2`：普通控件是直接模板 struct，
静态树由 `VBox<Id("..."), ...>` 编译期组合，字符串路径访问。

## §3.1 控件 = 直接模板 struct

```cpp
#include <imgui_mk/ui/wg2/wg2.h>
using namespace Mk::wg2;

Label<Id("title")> title;
title.text = "Hello wg2";

Button<Id("ok")> ok;
ok.label = "OK";
ok.onClick = [] { /* ... */ };
```

没有 `Control<Self>` 外壳，没有 `self`，没有 `node`。

## §3.2 静态树组合

```cpp
using Toolbar = HBox<Id("toolbar"),
    Button<Id("save")>,
    Button<Id("cancel")>
>;

using Root = VBox<Id("root"),
    Label<Id("title")>,
    Toolbar
>;

Root root;
```

子控件按类型编译期确定；布局递归自然发生。

## §3.3 字符串路径访问

```cpp
root.at<Id("title")>().text = "设置";
root.at<Id("toolbar"), Id("save")>().label = "保存";
root.at<Id("toolbar"), Id("cancel")>().label = "取消";
```

路径拼错或重复 ID 会在编译期报错。

## §3.4 渲染入口

```cpp
mainWindow.setUI([&] {
    layout(root, Rect{20, 20, 1240, 760});
});
```

## 练习

1. 给 Root 增加一个 `HBox`，并通过 `at<...>()` 配置子按钮。
2. 故意把 `at<Id("toolbar")>()` 写成不存在的 ID，观察编译错误。
