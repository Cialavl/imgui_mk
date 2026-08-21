# 第 05 课：组合与装饰件

wg2 装饰件是编译期包装器，子控件经模板参数传入。

## §5.1 enabled / visible

```cpp
widget.state.enabled = false;   // 灰色但可见
widget.state.visible = false;   // 不渲染且不参与布局
```

## §5.2 When

```cpp
When<Id("danger"), TextColored<Id("dangerText")>> danger;
danger.show = isAdmin;
danger.at<Id("dangerText")>().text = "危险操作";
```

## §5.3 ItemWidth

```cpp
ItemWidth<Id("width"), DragF<Id("drag")>> iw;
iw.width = 220.0f;
iw.at<Id("drag")>().value = &v;
```

## §5.4 Tooltip / Colored / Indent

```cpp
Tooltip<Id("tip"), Button<Id("btn")>> tip;
tip.text = "悬停提示";

Colored<Id("col"), Label<Id("lbl")>> col;
col.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);

Indent<Id("ind"), Label<Id("il")>> ind;
ind.amount = 24.0f;
```

## 练习

1. 把 `When` 包在 `ItemWidth` 外面，观察嵌套路径访问。
2. 把同一个按钮先包 `Colored` 再包 `Tooltip`。
