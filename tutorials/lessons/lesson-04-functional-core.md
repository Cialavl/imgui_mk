# 第 04 课：wg 原子控件与状态外置

配套程序：`src/tutorial04_functional.cpp`。上一课讲了"控件是什么"，
本课讲"控件怎么写进页面"：**状态全部外置，控件绑定指针；同一变量被
多个控件绑定 = 实时联动**。

## §4.1 状态外置：指针绑定

wg 原子控件不持有"业务状态"，只持**绑定指针**（可默认构造，兼容
tuple 子控件）：

```cpp
Mk::wg::SliderF  s;   s.self.label = "音量";   s.self.value = &volume;
Mk::wg::CheckBox c;   c.self.label = "静音";   c.self.checked = &mute;
```

`volume`、`mute` 定义在 `main` 栈上（覆盖 `run()`）。控件每帧渲染时
直接读写绑定变量——交互写回、逻辑侧也可随时改写，**两边天然同帧同步**。

## §4.2 同一变量绑多个控件：实时联动

配套程序把 `volume` 同时绑到树上的滑块和子窗口里的滑块：

```cpp
root.child<3>().self.value = &volume;              // 音量 A
root.child<4>().child<0>().self.value = &volume;   // 音量 B（子窗口内）
```

拖动任意一个，另一个同帧跟随——因为两个控件读写的是同一个 `float`。
多窗口联动（第 09 课）是同一原理的跨窗口版本。

## §4.3 自定义"逻辑驱动"控件

wg 控件不限于画 UI：render 里可以做任何事，包括**写回绑定变量**。
配套程序的 `SineWaveImpl` 每帧把 `*value = 0.5 + 0.5*sin(frame*0.05)`，
"逻辑"就长在控件里，与视图同帧执行。

```cpp
struct SineWaveImpl {
    int* frames = nullptr;
    float* value = nullptr;
    void renderImpl(Mk::wg::Node&) {
        *value = 0.5f + 0.5f * std::sin(*frames * 0.05f);   // 逻辑侧驱动
        ImGui::Text("正弦值 %.3f", *value);
    }
    // measureImpl / arrangeImpl 同上课
};
```

三个函数（render/measure/arrange）是 wg 控件的"行为契约"：渲染、求尺寸、
记录 bounds。缺省可省略（traits 兜底），但布局容器需要 measure/arrange。

## §4.4 常用原子速览（详见第 06 课目录）

| 原子 | 绑定 | 说明 |
|---|---|---|
| `Label` | 自持字符串 | `setText()/c_str()` |
| `Button` | `onClick` 回调 | `setLabel()` |
| `SliderF` | `float*` | `mn/mx` |
| `CheckBox` | `bool*` | — |
| `InputText<N>` | 内置 `std::array<char,N>` | N = 缓冲容量 |
| `InputInt/Float/Double` | 数值指针 | 含 vec2 变体 |
| `DragF/DragI` | 数值指针 | `speed/mn/mx` |
| `SliderInt/SliderAngle` | 数值指针 | 角度绑弧度 |
| `RadioGroup/Combo/ListBox` | `int*` + items | items 长期存活 |
| `ProgressBar/PlotLines` | 指针绑定 | 只读视图 |

## 练习

1. 给 `SineWaveImpl` 加一个"频率"字段，再绑一个 `SliderF` 去调它。
2. 把 FrameCounter 改成 `LabelText`（label/value 两段式）。
3. 观察：两个滑块绑定同一个 `float`，但 `InputFloat` 绑定另一个——
   验证它们互不影响（不同变量）。
