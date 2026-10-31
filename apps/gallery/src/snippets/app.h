#pragma once
// 示例代码（app）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kLife = R"(Mk::wg2::RuntimeList<Id("list")> list;
auto leaf = std::make_unique<Mk::wg2::Label<Id("leaf")>>("动态叶子");
Mk::wg2::AnyWidget any = Mk::wg2::AnyWidget::own(std::move(leaf));
list.add(std::move(any));   // 渲染期安全增删
list.removeLast();)";

inline constexpr std::string_view kPanel = R"(Mk::wg2::SubWindow<Id("panel"),
    SliderF<Id("slider")>,
    LabelText<Id("frame")>
> panel;
panel.height = 110.0f;)";

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
