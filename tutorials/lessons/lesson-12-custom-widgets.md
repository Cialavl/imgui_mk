# 第 12 课：扩展你自己的控件

配套程序：`src/tutorial12_custom.cpp`。wg 的扩展点就一个：**写模板
struct，经 `Control` 组合**。本课演示四种自定义形态。

## §12.1 纯显示 / 交互控件

```cpp
struct StatusBarImpl {
    int* frames = nullptr;
    void renderImpl(Mk::wg::Node&) {
        if(frames) ImGui::Text("帧 %d（自定义 wg 控件 StatusBar）", *frames);
    }
    Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size& avail) {
        return Mk::wg::Size{avail.w, 20.0f};   // 占满宽、固定高
    }
    void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using StatusBar = Mk::wg::Control<StatusBarImpl, Mk::wg::DefaultStyle>;
```

三个行为函数是契约：`renderImpl` 画、`measureImpl` 求期望尺寸、
`arrangeImpl` 记 bounds（写 `n.bounds = r`）。`Control` 组合壳自动完成
`node.bind<Self>()` 行为表填充与子节点挂树。

## §12.2 自定义装饰件：样式作用域

装饰件包裹子树做 push/pop 作用域（与内置 Tooltip/Colored/Indent 同构）：

```cpp
struct EmphasisImpl {
    float border = 2.0f;
    void renderImpl(Mk::wg::Node& n) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, border);
        for(auto* c : n.children) if(c) c->draw();   // 画子树
        ImGui::PopStyleVar();                        // 严格配对
    }
    // measure/arrange：透传子节点（取最后一个子/最大尺寸）
};
using Emphasis = Mk::wg::Control<EmphasisImpl, Mk::wg::DefaultStyle>;

Emphasis emph;  emph.self.border = 2.0f;
emph.node.add(emphBtn.node);        // 子节点运行时挂入
emph.node.add(emphSlider.node);
```

## §12.3 自定义复合控件

完全脱离公共控件集的复合体：持有引用、自带折叠区与交互：

```cpp
struct TogglePanelImpl {
    int* clicks = nullptr;
    float* vol = nullptr;
    void renderImpl(Mk::wg::Node&) {
        if(ImGui::CollapsingHeader("自定义复合面板（点击展开）")) {
            ImGui::Text("内部计数 %d", clicks ? *clicks : 0);
            if(vol) ImGui::SliderFloat("音量", vol, 0.0f, 1.0f);
            if(ImGui::Button("+1")) { if(clicks) ++(*clicks); }
        }
    }
    ...
};
using TogglePanel = Mk::wg::Control<TogglePanelImpl, Mk::wg::DefaultStyle>;
```

## §12.4 自定义字体链与重烘焙

```cpp
mainWindow.getWindow().setFontLoader([](float size) {
    if(!Mk::window::Impl::defaultFontLoader(size)) return false;  // 中文链
    ImFontConfig cfg;
    cfg.SizePixels = size * 0.9f;               // 等宽字 ×0.9 联动
    ImGui::GetIO().Fonts->AddFontDefault(&cfg);
    return true;
});
// 之后所有 rescale/档位/跨屏重烘焙都走这条链
```

## §12.5 扩展方式小结

| 想做什么 | 怎么做 |
|---|---|
| 新原子控件 | struct + render/measure/arrange，`Control` 组合 |
| 装饰件 | struct + Push/Pop 包裹子树（子经 `node.add`） |
| 复合控件 | struct 自带渲染，可完全脱离公共控件集 |
| 桥接裸 ImGui | `FnRender<F>`（第 10 课） |
| 改皮肤/字号 | `Theme` + `Styled` + `rescale`（第 08 课） |

## 练习

1. 写一个 `ProgressWithLabel` 控件：进度条 + 百分比文本。
2. 写一个 `VSpacer`（占位控件，measure 返回固定高、不渲染）。
3. 给 `Emphasis` 加 `color` 字段：PushStyleColor 边框色。
