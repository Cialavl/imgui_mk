# 第 10 课：桥接与动态树

wg2 静态树覆盖大多数界面；需要运行时增删时，按需使用动态容器。

## §10.1 RuntimeList

```cpp
RuntimeList<Id("list")> list;

auto item = std::make_unique<Label<Id("item")>>("新项");
AnyWidget any = AnyWidget::own(std::move(item));
any.idname = "item";
list.add(std::move(any));
list.removeLast();
```

`add/remove` 使用 pending + tombstone，渲染回调内安全。

## §10.2 RuntimeTabs

```cpp
RuntimeTabs<Id("tabs")> tabs;
tabs.add("页 A", AnyWidget::own(std::make_unique<Label<Id("a")>>("A")));
tabs.add("页 B", AnyWidget::own(std::make_unique<Label<Id("b")>>("B")));
```

## §10.3 RuntimeGrid

```cpp
RuntimeGrid<Id("grid")> grid;
grid.cols = 2;
grid.add(AnyWidget::own(std::make_unique<Button<Id("b")>>()));
```

## §10.4 裸绘制桥

```cpp
auto drawer = []{ ImGui::Text("hello"); };
FnWidget<Id("fn"), decltype(drawer)> fn(drawer);
```

## 练习

用 `RuntimeList` 做一个按钮添加/删除列表，并把列表嵌在静态 `VBox` 中。
