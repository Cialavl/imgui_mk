#pragma once
// 示例代码（atoms）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kTexts = R"(Mk::wg2::Label<Id("lbl")> lbl;          lbl.text = "Hello";
Mk::wg2::TextColored<Id("tc")> tc;  tc.text = "彩色";
Mk::wg2::TextWrapped<Id("tw")> tw;  tw.text = "自动换行...";
Mk::wg2::BulletText<Id("bt")> bt;   bt.text = "圆点项";
Mk::wg2::LabelText<Id("lt")> lt;    lt.name = "属性"; lt.value = "值";
Mk::wg2::EllipsisLabel<Id("el")> el; el.text = "超宽截断...";)";

inline constexpr std::string_view kButton = R"(Mk::wg2::Button<Id("btn")> btn;
btn.label = "点我";
btn.onClick = [&]{ ++clicks; };

Mk::wg2::ImageButton<Id("img")> img;
img.tex = tex;  img.size = ImVec2(96, 96);)";

inline constexpr std::string_view kInputText = R"(Mk::wg2::InputText<Id("text"), 128> it;   // N = 缓冲容量
it.name = "文本";           // 显示名（左侧标签）
it.setName("初始值");       // 缓冲内容
// std::array<char, N> buf 内置于控件)";

inline constexpr std::string_view kInputNum = R"(Mk::wg2::InputInt<Id("ii")> ii;    ii.name = "整数";   ii.value = &i;
Mk::wg2::InputFloat<Id("fi")> fi;  fi.name = "浮点";   fi.value = &f;
Mk::wg2::InputDouble<Id("di")> di; di.name = "双精度"; di.value = &d;
Mk::wg2::InputInt2<Id("iv")> iv;   iv.name = "ivec2";  iv.value = &iv2;)";

inline constexpr std::string_view kDrag = R"(Mk::wg2::DragF<Id("df")> df;  df.name = "拖拽f"; df.value = &v;
df.speed = 0.01f;
Mk::wg2::SliderF<Id("sf")> sf;  sf.name = "滑块";  sf.value = &v;
Mk::wg2::SliderAngle<Id("sa")> sa; sa.name = "角度"; sa.valueRad = &rad;)";

inline constexpr std::string_view kSelect = R"(Mk::wg2::CheckBox<Id("cb")> cb;  cb.name = "静音";  cb.checked = &mute;
Mk::wg2::RadioGroup<Id("rg")> rg; rg.items = kItems; rg.count = 3; rg.current = &sel;
Mk::wg2::Combo<Id("co")> co;      co.name = "下拉";  co.current = &sel;
Mk::wg2::CheckboxFlags<Id("cf")> cf; cf.name = "位A"; cf.flags = &bits; cf.mask = 1;)";

inline constexpr std::string_view kFeedback = R"(Mk::wg2::ProgressBar<Id("pb")> pb; pb.fraction = &volume;
Mk::wg2::PlotLines<Id("pl")> pl;  pl.name = "曲线";  pl.values = &history;
Mk::wg2::SeparatorText<Id("st")> st; st.name = "分组";)";

inline constexpr std::string_view kColor = R"(std::array<float,3> rgb{...};
Mk::wg2::ColorEdit3<Id("ce3")> ce3; ce3.name = "RGB"; ce3.color = &rgb;
Mk::wg2::ColorEdit4<Id("ce4")> ce4; ce4.name = "RGBA"; ce4.color = &rgba;
Mk::wg2::ColorPicker3<Id("cp3")> cp3; cp3.name = "拾取"; cp3.color = &rgb;)";

} // namespace gallery::snips
