# 第 12 课：扩展你自己的 wg2 控件

wg2 自定义控件就是满足 StaticWidget 的普通 struct。

## 12.1 控件协议

`cpp
template<FixedString Id>
struct MyWidget {
    static constexpr auto id = Id;

    WidgetState<> state;
    float* value = nullptr;

    Size hint(const Size& avail) const { return {avail.w, 20.0f}; }
    Policy policy() const { return {}; }
    void draw(const Rect& r) {
        if (value) ImGui::Text("value = %.3f", *value);
    }
};
`

## 12.2 放入静态树

`cpp
VBox<Id("root"), MyWidget<Id("my")>, Button<Id("ok")>> root;
root.at<Id("my")>().value = &v;
layout(root, hostRect);
`

## 12.3 动态包装

`cpp
auto p = std::make_unique<MyWidget<Id("my")>>();
p->value = &v;
RuntimeList<Id("list")> list;
list.add(AnyWidget::own(std::move(p)));
`

## 12.4 自定义容器

容器保存 std::tuple<Children...>，实现 hint/draw/at。

## 练习

实现一个 StatusDot 控件，根据 float* 显示红/黄/绿三种颜色。
