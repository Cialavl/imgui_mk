# 第 04 课：wg2 原子控件与状态外置

状态全部外置，控件只保存绑定指针；同一变量可绑定多个控件，实时联动。

## §4.1 指针绑定

```cpp
float volume = 0.6f;
bool mute = false;

SliderF<Id("volume")> vol;
vol.name = "音量";
vol.value = &volume;

CheckBox<Id("mute")> muteBox;
muteBox.name = "静音";
muteBox.checked = &mute;
```

## §4.2 同变量多控件联动

```cpp
root.at<Id("panel"), Id("volume")>().value = &volume;
root.at<Id("main"), Id("volume")>().value = &volume;
```

拖动任意一个滑块，另一个同帧跟随。

## §4.3 自定义逻辑控件

```cpp
struct SineWave {
    int* frames = nullptr;
    float* value = nullptr;

    Size hint(const Size&) const { return {0.0f, 20.0f}; }
    Policy policy() const { return {}; }

    void draw(const Rect&) {
        *value = 0.5f + 0.5f * std::sin(*frames * 0.05f);
        ImGui::Text("正弦值 %.3f", *value);
    }
};

using Sine = FnWidget<Id("sine"), SineWave>; // 或直接作为 VBox 子类型
```

## §4.4 常用原子

`Label / Button / SliderF / CheckBox / InputText<N> / InputInt / InputFloat / Combo / ProgressBar`。

## 练习

1. 给 `SineWave` 增加频率字段，并绑一个 `SliderF` 调它。
2. 观察两个滑块绑定同一变量、另一个绑定不同变量的行为。
