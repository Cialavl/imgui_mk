#pragma once
#include <string_view>

// 画廊代码片段：每个演示页下方代码区显示的代表性片段（raw string 静态存储），
// 与页面上方的真实控件人工保持同步。当前 API：独立布局类型 + addWidget 装配 +
// 设计像素尺寸（em 缩放）；对外示例统一用 Mk::wg:: 全限定。

namespace gallery::snips {

inline constexpr std::string_view kBase = R"(Mk::wg::VLayout box;              // 独立布局类型（恒填满宿主）
Mk::wg::Label title;  title.self.setName("标题");
Mk::wg::Button btn;   btn.self.setName("点我");   // 显示名
btn.node.idname = "okBtn";                        // 身份 idname
btn.self.onClick = [&]{ ++clicks; };
box.addWidget(title.node);
box.addWidget(btn.node);
box.layout({0, 0, 400, 200});)";

inline constexpr std::string_view kLife = R"(Mk::wg::Label leaf;
leaf.self.setName("动态叶子");
box.addWidget(leaf.node);      // 运行时挂入（墓碑增删，回调内安全）
box.node.remove(leaf.node);    // 移除触发 onDestroy)";

inline constexpr std::string_view kLayout = R"(Mk::wg::HLayout row;   // 行
Mk::wg::VLayout col;   // 列
col.addWidget(a.node);
col.addWidget(b.node);
row.addWidget(col.node);
row.addWidget(c.node);  // 布局嵌布局自然递归
// 两遍布局：measure（求期望）→ arrange（父分配 bounds）→ draw)";

inline constexpr std::string_view kSplit = R"(Mk::wg::SplitHLayout split;
split.addWidget(nav.node);            // 固定 160（设计像素，em 缩放）
split.addWidget(main.node, 2.0f);     // 权重 2
split.addWidget(side.node, 1.0f);     // 权重 1
nav.setFixedWidth(160.0f);
Mk::wg::WrapLayout flow;   // 流式换行
Mk::wg::GridLayout grid;   // 固定列网格
grid.setCols(3);)";

inline constexpr std::string_view kTexts = R"(Mk::wg::Label lbl;          lbl.self.setName("Hello");
Mk::wg::TextColored tc;   tc.self.name = "彩色";
Mk::wg::TextWrapped tw;   tw.self.name = "自动换行...";
Mk::wg::BulletText bt;    bt.self.name = "圆点项";
Mk::wg::LabelText lt;     lt.self.name = "属性";  lt.self.value = "值";
Mk::wg::EllipsisLabel el; el.self.name = "超宽截断...";)";

inline constexpr std::string_view kButton = R"(Mk::wg::Button btn;  btn.self.setName("点我");
btn.node.idname = "okBtn";
btn.self.onClick = [&]{ ++clicks; };
Mk::wg::ImageButton img;  img.node.idname = "img1";
img.self.tex = tex;  img.self.size = ImVec2(96, 96);)";

inline constexpr std::string_view kInputText = R"(Mk::wg::InputText<128> it;   // N = 缓冲容量
it.self.name = "文本";         // 显示名（左侧标签）
it.self.setName("初始值");     // 缓冲内容
// std::array<char, N> buf 内置于控件)";

inline constexpr std::string_view kInputNum = R"(Mk::wg::InputInt ii;   ii.self.name = "整数";   ii.self.value = &i;
Mk::wg::InputFloat fi;  fi.self.name = "浮点";   fi.self.value = &f;
Mk::wg::InputDouble di; di.self.name = "双精度"; di.self.value = &d;
Mk::wg::InputInt2 i2;   i2.self.name = "ivec2";  i2.self.value = &iv;)";

inline constexpr std::string_view kDrag = R"(Mk::wg::DragF df;  df.self.name = "拖拽f"; df.self.value = &v;
df.self.speed = 0.01f;
Mk::wg::SliderF sf;  sf.self.name = "滑块";  sf.self.value = &v;
Mk::wg::SliderAngle sa; sa.self.name = "角度"; sa.self.valueRad = &rad;)";

inline constexpr std::string_view kSelect = R"(Mk::wg::CheckBox cb;  cb.self.name = "静音";  cb.self.checked = &mute;
Mk::wg::RadioGroup rg; rg.self.items = kItems; rg.self.count = 3;
rg.self.current = &sel;
Mk::wg::Combo co;      co.self.name = "下拉";  co.self.current = &sel;
co.self.items = kItems;  co.self.count = 3;
Mk::wg::CheckboxFlags cf; cf.self.name = "位A";
cf.self.flags = &bits;  cf.self.mask = 1;)";

inline constexpr std::string_view kFeedback = R"(Mk::wg::ProgressBar pb; pb.self.fraction = &volume;
Mk::wg::PlotLines pl;  pl.self.name = "曲线";  pl.self.values = &history;
Mk::wg::SeparatorText st; st.self.name = "分组";)";

inline constexpr std::string_view kContainers = R"(Mk::wg::OverlayLayout overlay;   // 叠放
Mk::wg::Collapsing col; col.self.name = "折叠";
col.self.flags = ImGuiTreeNodeFlags_DefaultOpen;
Mk::wg::Tree tree;     tree.self.name = "树";    tree.self.defaultOpen = true;
Mk::wg::SubWindow<> sub; sub.node.idname = "sub";  sub.self.height = 80.0f;
sub.node.add(lbl.node);)";

inline constexpr std::string_view kTabBar = R"(Mk::wg::TabBar<> t;   // 页名 = 页节点 idname
t.node.idname = "tabs";
Mk::wg::Tab<> p1;  p1.node.idname = "页一";
t.node.add(p1.node);   // Tab 运行时挂入)";

inline constexpr std::string_view kPanel = R"(Mk::wg::Panel<> p;
p.node.idname = "面板";   // 窗口标题读 idname（可拖动）
p.node.add(slider.node);
p.node.add(lbl.node);
// MenuBar/Menu/MenuItem 是常驻层控件（见主窗口顶部）)";

inline constexpr std::string_view kTable = R"(Mk::wg::TextTable tt;   // 数据驱动
tt.node.idname = "tbl";       // 表格 ImGui ID
tt.self.headers = kHeaders;  tt.self.headerCount = 3;
tt.self.rows = &rows;        // const vector<vector<string>>*
Mk::wg::FnTable ft;          // 单元格函数
ft.self.cell = [](int r, int c){ return std::to_string(r) + "," + std::to_string(c); };)";

inline constexpr std::string_view kPopup = R"(bool open = false;
Mk::wg::Popup<> pop;        // 标志触发式
pop.node.idname = "dlg";    // 弹窗 ImGui ID
pop.self.open = &open;
pop.node.add(lbl.node);
Mk::wg::PopupModal<> modal; // 模态变体
modal.node.idname = "modal"; modal.self.open = &open;)";

inline constexpr std::string_view kColor = R"(std::array<float,3> rgb{...};
Mk::wg::ColorEdit3 ce3;   ce3.self.name = "RGB";  ce3.self.color = &rgb;
Mk::wg::ColorEdit4 ce4;   ce4.self.name = "RGBA"; ce4.self.color = &rgba;
Mk::wg::ColorPicker3 cp3; cp3.self.name = "拾取"; cp3.self.color = &rgb;)";

inline constexpr std::string_view kDeco = R"(Mk::wg::Tooltip tip;    tip.self.name = "提示";  tip.node.add(btn.node);
Mk::wg::Colored col;    col.self.color = ImVec4(0.4f,0.9f,0.6f,1);
col.node.add(lbl.node);
Mk::wg::Indent ind;     ind.self.amount = 24.0f;  ind.node.add(lbl.node);
Mk::wg::ItemWidth<Mk::wg::DragF> iw;  iw.self.width = 180.0f;
Mk::wg::When<Mk::wg::Label> w;        w.self.show = cond;
Mk::wg::Styled<Mk::wg::Button> st;    st.self.theme = Mk::wg::Theme::light();
btn.node.enabled = false;             // 禁用子树)";

inline constexpr std::string_view kTheme = R"(auto t = Mk::wg::Theme::dark().withAccent(accent);
win.setTheme(t);                       // 每窗口独立 ImGui 上下文
win.getWindow().rescale(24.0f);        // 重烘焙字号（几何等比联动）
// 子树级作用域换肤：Styled 控件包裹
Mk::wg::Styled<Mk::wg::Button> st;
st.self.theme = t.withItemSpacing(2.0f, 2.0f);)";

inline constexpr std::string_view kMultiWindow = R"(Mk::MainWindow<> gallery("Gallery", 1440, 880);
Mk::window inspector("Inspector", 420, 640);
inspector.setCentralWidget(insTree.node);   // 中央挂载：引擎每帧自动根布局
Mk::mkui<Mk::window>::run();                // 引擎驱动双窗口)";

inline constexpr std::string_view kFile = R"(Mk::wg::FileDialog dlg;   // IFileOpenDialog worker 线程
dlg.open("Open File", {{"Text", "*.txt"}, {"All", "*.*"}});
std::string path;
if(dlg.poll(path) && !path.empty()) { /* load */ }
// 对话框打开期间主循环不冻结)";

inline constexpr std::string_view kFnRender = R"(auto drawer = []{ ImGui::ProgressBar(v, ImVec2(-FLT_MIN,0)); };
Mk::wg::FnRender<decltype(drawer)> fn(drawer);   // 裸 ImGui 桥接节点
fn.layout({0, 0, 300, 30});)";

} // namespace gallery::snips