# 第 09 课：多窗口编程

两个顶层窗口各自拥有 ImGui 上下文；绑定同一批外部变量即可实时联动。

## §9.1 创建子窗口

```cpp
Mk::window inspector("Inspector", 420, 640);
inspector.setAutoScaleFont(true);
```

## §9.2 wg2 中央挂载

```cpp
inspector.setCentralWidget([&](const Mk::wg2::Rect& r) {
    Mk::wg2::layout(insRoot, r);
});
```

旧 `setCentralWidget(Node&)` 仍保留，新代码建议使用回调版本。

## §9.3 共享状态

```cpp
float volume = 0.6f;

root.at<Id("volume")>().value = &volume;
insRoot.at<Id("volume")>().value = &volume;
```

任意窗口修改，另一个窗口同帧刷新。

## §9.4 挂载与隐藏

```cpp
parent.mountChild(inspector);
glfwHideWindow(inspector.getHandle());
glfwShowWindow(inspector.getHandle());
```

## 练习

在两个窗口各放一个 `SliderF` 和 `LabelText`，全部绑定 `volume`。
