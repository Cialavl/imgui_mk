# 第 10 课：桥接与动态树

配套程序：`src/tutorial10_bridges.cpp`。单一体系下仍需两种"桥"：
**动态列表**（运行时增删）与 **FnRender**（裸 ImGui 绘制桥接成节点）。

## §10.1 动态列表：拥有权池 + 墓碑增删

模板组合是编译期的，动态列表必须走运行时通道：

```cpp
std::vector<std::unique_ptr<Mk::wg::Label>> pool;   // 拥有权池
Mk::wg::SubWindow<> listZone;                        // 列表容器（无模板子）

// 添加：渲染回调内 add 安全
pool.push_back(std::make_unique<Mk::wg::Label>(("wg 项 #" + ...).c_str()));
root.child<2>().node.add(pool.back()->node);

// 删除：置空槽位（墓碑），不搬移 vector
root.child<2>().node.remove(pool.back()->node);
pool.pop_back();
```

`remove` 触发已绘制节点的 `onDestroy` 生命周期回调；槽位留 `nullptr`
直到下一次 `compact`——**在渲染回调里增删全程安全**。

## §10.2 FnRender：裸 ImGui 绘制桥

有些绘制（进度条、自定义绘制序列）直接写裸 ImGui 更自然。FnRender
把它桥接为 wg 节点：

```cpp
struct CounterDrawerImpl {
    int* clicks = nullptr;
    void operator()() {
        if(clicks) ImGui::Text("FnRender 内裸绘制：点击 %d 次", *clicks);
    }
};
Mk::wg::FnRender<CounterDrawerImpl> fnView;
fnView.self.fn = CounterDrawerImpl{&clicks};
root.child<4>().self.fn = CounterDrawerImpl{&clicks};   // 或直接进树
```

`FnRender<F>` 的 measure 占满可用宽、高度 0——渲染内容由 `fn()` 决定。
可默认构造的 callable 才能作为 tuple 子控件（配套程序的
`CounterDrawerImpl` 用指针成员满足）。

## §10.3 数据镜像：跨位置联动

配套程序的"镜像"练习：点击后把列表长度写进 `mirrorCount`，下方
`When` 条件渲染提示——数据（数量）经普通变量流动，视图（When）只读：

```cpp
root.child<6>().self.text = std::to_string(mirrorCount);
root.child<7>().self.show = mirrorCount > 0;
```

## 练习

1. 给列表项加"点击高亮"（`Selectable` + `selected` 指针池）。
2. 用 FnRender 画一段自定义进度条（叠加两个矩形）。
3. 把"镜像"改成反向：从 FnRender 里写回 `mirrorCount`。
