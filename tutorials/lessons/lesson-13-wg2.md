# 第 13 课：wg2 静态树 + 动态列表 opt-in

配套程序：`src/tutorial13_wg2.cpp`。

`Mk::wg2` 是 wg 的下一步控件层：普通控件是直接模板 struct，
静态树经 `VBox<Id("..."), ...>` 编译期组合，字符串路径访问；
只有真正需要运行时增删的列表才使用 `RuntimeList`。

## 静态树

```cpp
using Root = VBox<
    Id("root"),
    Label<Id("status")>,
    HBox<Id("toolbar"), Button<Id("add")>, Button<Id("del")>>,
    SubWindow<Id("listwin"), RuntimeList<Id("list")>>
>;

Root root;
root.at<Id("toolbar"), Id("add")>().label = "添加";
```

## 动态列表 opt-in

```cpp
root.at<Id("toolbar"), Id("add")>().onClick = [&] {
    auto label = std::make_unique<Label<Id("item")>>("新项");
    AnyWidget any = AnyWidget::own(std::move(label));
    any.idname = "item";
    root.at<Id("listwin"), Id("list")>().add(std::move(any));
};
```

`RuntimeList` 内部保留渲染期安全增删（pending + 墓碑），普通静态控件
不携带这些能力。

## 布局入口

```cpp
mainWindow.setUI([&] {
    layout(root, Rect{20, 20, 1240, 760});
});
```
