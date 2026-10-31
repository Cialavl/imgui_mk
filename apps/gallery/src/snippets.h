#pragma once
#include <string_view>

// 画廊代码片段：每个演示页下方代码区显示的代表性片段（raw string 静态存储），
// 与页面上方的真实控件人工保持同步。当前 API：Mk::wg2 静态树 + 字符串路径。

namespace gallery::snips {

inline constexpr std::string_view kBase = R"(using Mk::wg2::Id;
using Mk::wg2::VBox;
using Mk::wg2::Label;
using Mk::wg2::Button;

using Root = VBox<Id("root"), Label<Id("title")>, Button<Id("ok")>>;
Root root;
root.at<Id("title")>().text = "标题";
root.at<Id("ok")>().label = "点我";
root.at<Id("ok")>().onClick = [&]{ ++clicks; };
Mk::wg2::layout(root, {0, 0, 400, 200});)";

inline constexpr std::string_view kLife = R"(Mk::wg2::RuntimeList<Id("list")> list;
auto leaf = std::make_unique<Mk::wg2::Label<Id("leaf")>>("动态叶子");
Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(leaf));
list.add(std::move(any));   // 渲染期安全增删
list.removeLast();)";

inline constexpr std::string_view kLayout = R"(using Root = Mk::wg2::VBox<
    Id("root"),
    Mk::wg2::Label<Id("intro")>,
    Mk::wg2::HBox<Id("row"),
        Mk::wg2::VBox<Id("col"), SliderF<Id("v")>, CheckBox<Id("m")>>,
        Mk::wg2::VBox<Id("rightCol"), Label<Id("right")>>
    >
>;
Root root;
Mk::wg2::layout(root, rect);)";

inline constexpr std::string_view kSplit = R"(using Root = Mk::wg2::VBox<
    Id("root"),
    Mk::wg2::SplitH<Id("split"),
        Mk::wg2::Label<Id("nav")>,
        Mk::wg2::Label<Id("main")>,
        Mk::wg2::Label<Id("side")>
    >,
    Mk::wg2::Wrap<Id("wrap"), Label<Id("w1")>, Label<Id("w2")>>,
    Mk::wg2::Grid<Id("grid"), Label<Id("g00")>, Label<Id("g01")>>
>;
Root root;
root.at<Id("split"), Id("nav")>().state.size.fixedW = 160.0f;
root.at<Id("split"), Id("main")>().state.stretch = 2.0f;
root.at<Id("split"), Id("side")>().state.stretch = 1.0f;)";

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

inline constexpr std::string_view kContainers = R"(Mk::wg2::Overlay<Id("overlay"),
    Button<Id("b1")>,
    Mk::wg2::Offset<Id("off"), Label<Id("l1")>>   // 叠放错开，避免文字重叠
> overlay;
overlay.at<Id("off")>().dxEm = 7.5f;
Mk::wg2::Collapsing<Id("coll"), Label<Id("cl")>> collapsing;
Mk::wg2::Tree<Id("tree"), Label<Id("leaf1")>, Label<Id("leaf2")>> tree;
Mk::wg2::SubWindow<Id("sub"), Label<Id("sl")>> sub;
sub.height = 80.0f;)";

inline constexpr std::string_view kTabBar = R"(Mk::wg2::TabBar<Id("tabs"),
    Mk::wg2::Tab<Id("p1"), Label<Id("t1")>>,
    Mk::wg2::Tab<Id("p2"), Label<Id("t2")>>
> tabs;
tabs.at<Id("p1")>().title = "页一";)";

inline constexpr std::string_view kPanel = R"(Mk::wg2::SubWindow<Id("panel"),
    SliderF<Id("slider")>,
    LabelText<Id("frame")>
> panel;
panel.height = 110.0f;)";

inline constexpr std::string_view kTable = R"(Mk::wg2::TextTable<Id("tt")> tt;   // 数据驱动
tt.headers = kHeaders; tt.headerCount = 3; tt.rows = &rows;
Mk::wg2::FnTable<Id("ft")> ft;          // 单元格函数
ft.cell = [](int r, int c){ return std::to_string(r) + "," + std::to_string(c); };)";

inline constexpr std::string_view kPopup = R"(bool open = false;
Mk::wg2::Popup<Id("pop"), Label<Id("pl")>> pop;
pop.open = &open;
Mk::wg2::PopupModal<Id("modal"), Label<Id("ml")>> modal;
modal.open = &open;)";

inline constexpr std::string_view kColor = R"(std::array<float,3> rgb{...};
Mk::wg2::ColorEdit3<Id("ce3")> ce3; ce3.name = "RGB"; ce3.color = &rgb;
Mk::wg2::ColorEdit4<Id("ce4")> ce4; ce4.name = "RGBA"; ce4.color = &rgba;
Mk::wg2::ColorPicker3<Id("cp3")> cp3; cp3.name = "拾取"; cp3.color = &rgb;)";

inline constexpr std::string_view kDeco = R"(Mk::wg2::Tooltip<Id("tip"), Label<Id("tl")>> tip;
tip.text = "提示";
Mk::wg2::Colored<Id("col"), Label<Id("cl")>> col;
col.color = ImVec4(0.4f,0.9f,0.6f,1);
Mk::wg2::Indent<Id("ind"), Label<Id("il")>> ind; ind.amount = 24.0f;
Mk::wg2::ItemWidth<Id("iw"), DragF<Id("df")>> iw; iw.width = 180.0f;
Mk::wg2::When<Id("w"), Label<Id("wl")>> w; w.show = cond;
btn.state.enabled = false;)";

inline constexpr std::string_view kTheme = R"(auto t = Mk::wg2::Theme::dark().withAccent(accent);
Mk::wg2::setDefaultTheme(t);
Mk::wg2::scaledForMonitor(t);
// 换肤由引擎广播，窗口各自帧内应用)";

inline constexpr std::string_view kMultiWindow = R"(Mk::MainWindow<> gallery("Gallery", 1440, 880);
Mk::window inspector("Inspector", 420, 640);
inspector.setCentralWidget(insRoot);   // 视图直挂，窗口每帧自动布局
Mk::mkui<Mk::window>::run();)";

inline constexpr std::string_view kFile = R"(Mk::wg2::FileDialog dlg;   // IFileOpenDialog worker 线程
dlg.open("Open File", {{"Text", "*.txt"}, {"All", "*.*"}});
std::string path;
if(dlg.poll(path) && !path.empty()) { /* load */ }
// 对话框打开期间主循环不冻结)";

inline constexpr std::string_view kFnRender = R"(auto drawer = []{ ImGui::ProgressBar(v, ImVec2(-FLT_MIN,0)); };
Mk::wg2::FnWidget<Id("fn"), decltype(drawer)> fn(drawer);
Mk::wg2::layout(fn, {0, 0, 300, 30});)";

} // namespace gallery::snips
