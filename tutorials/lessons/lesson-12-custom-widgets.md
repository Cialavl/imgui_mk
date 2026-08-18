# 第 12 课：扩展你自己的控件

配套程序：`src/tutorial12_custom.cpp`。完结课：把本系列教的模式
（ADL、修饰器、生命周期契约、字体链）串起来，扩展两样东西：
**自定义控件**与**自定义字体链**。

## §12.1 ADL 三步法（函数式路线）

函数式库扩展控件只需三步，零继承、零改库：

```cpp
// 第一步：定义纯数据结构（存什么显示什么）
struct StatusBar {
	int& frames;          // 状态外置（第 02 课规矩）
};

// 第二步：在自己的命名空间写 draw 重载
void draw(const StatusBar& s) {
	ImGui::Text("帧 %d", s.frames);
}

// 第三步：直接用——容器/修饰器经 ADL 自动发现
Mk::fui::draw(Mk::fui::vbox(8.0f,
	StatusBar{frames},
	Mk::fui::label("...")));
```

原理：函数式容器渲染子视图时调 `draw(child)`，这是依赖调用——实例化
时经**依赖实参查找（ADL）**在子视图类型的关联命名空间里找重载。
你的类型在哪个命名空间，`draw` 就放哪个命名空间。

**数据型控件**（上面这种：渲染既有数据）与**交互型控件**（渲染 + 检测
点击/悬停）是同一模式——交互型在 draw 里用 ImGui 返回值驱动回调即可。

配套程序 §12.1 区：StatusBar（数据型）+ ColorSwatch（交互型，点击
循环切换颜色）两个自定义控件直接进 vbox/修饰器。

## §12.2 自定义修饰器

修饰器 = "持子视图 + 包裹行为"的结构 + 自己的 draw：

```cpp
template<typename T>
struct Emphasis { T child; };
template<typename T>
void draw(const Emphasis<T>& e) {
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	draw(e.child);                       // 递归交给库的容器/控件
	ImGui::PopStyleVar();
}
auto emphasis = [](auto child) { return Emphasis<std::decay_t<decltype(child)>>{child}; };
```

配套程序 §12.2 区：`Emphasis` 修饰器（加粗边框）包任意按钮——验证
"修饰器可包一切"的组合性。

## §12.3 自定义 OOP 控件（对象路线）

需要长期存活 + 自带状态/子控件的复合控件用 OOP（第 03 课）：

```cpp
class TogglePanel : public Mk::ui::Widget {
	// 长期成员 + 子控件
protected:
	void render() override { ... }
};
```

配套程序 §12.3 区：一个"折叠 + 计数"面板——继承 Widget、组合两个
子控件、render 里控制子树。OOP 与函数式两条路线在本课首次同台。

## §12.4 自定义字体链（FontLoader）

rescale（第 08 课）重烘焙字体时，默认链只挂中文字体；应用可以替换
整条链——gallery 用它把"中文 + 等宽代码字体"一起重挂：

```cpp
mainWindow.getWindow().setFontLoader([](float size) {
	if(!Mk::window::Impl::defaultFontLoader(size)) return false;  // 中文链
	gallery::gMono = ImGui::GetIO().Fonts->AddFontDefault();     // 等宽链
	return true;
});
```

配套程序 §12.4 区：自定义链 = 中文 + 大号等宽（改 SizePixels），
配合 §8.4 的手动 rescale 按钮，验证重烘焙后两字体同步生效。

## §12.5 组合的边界与最佳实践

- **首选组合**：库控件 + 库修饰器，能不写控件就不写（第 05 课精神）；
- **数据型自定义**：ADL 三步法（§12.1）；
- **行为包装**：自定义修饰器（§12.2）；
- **长期复合体**：OOP 子类（§12.3）；
- **资源型**：自定义字体链（§12.4）与纹理辅助（第 06 课 §6.2）。

写完自定义控件后自问：能否用库内置组合 + 修饰器达到同样效果？
能——就用库的；不能——按上表选路线。

## §12.6 常见错误

1. **draw 重载放进 Mk::fui 命名空间**：能编译但不是你的地盘，ADL
   反而可能被别的重载遮蔽；放自己命名空间。
2. **修饰器忘了 Pop**：任何 Push 都要配对 Pop（异常路径除外）；
   参考库内修饰器写法（每个都严格配对）。
3. **OOP 控件析构顺序**：子控件引用外部状态（第 03 课契约）——成员
   析构顺序与声明相反，绑定的外部变量必须活得比控件久。
4. **FontLoader 里假设旧字体还在**：rescale 先 Clear 了图集；loader
   必须完整重建（不能只"追加"）。

## §12.7 练习

1. §12.1：把 StatusBar 升级成"悬浮提示版"（tooltip 修饰器包裹即可——
   不需要改控件）。
2. §12.2：写一个 `Rounded` 修饰器（改 FrameRounding），包 InputText。
3. 挑战：做一个 `Switch` 开关控件（数据型：bool&；交互型：点击切换），
   并在 vbox 里与 CheckBox 并排使用——对比"自定义 vs 库内置"的成本。

## §12.8 完结与进一步阅读

到这里 mkui 系列 12 课完结。把两套库、窗口生命周期、布局、主题、
多窗口、文件、扩展全部串起来的最好教材是**读 gallery 源码**：

- `apps/gallery/src/gallery_pages.h`：38 页全部用本课模式写成
  （Sidebar/DemoHead/CodeBlock/AccentEdit 都是 ADL 自定义控件）；
- 附录 A：全部 API 速查表；
- 附录 B：本系列开发中踩过的真实陷阱（读它 = 少走我们的弯路）。