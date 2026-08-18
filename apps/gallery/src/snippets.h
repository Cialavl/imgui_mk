#pragma once
#include <string_view>

// 画廊代码片段：每个演示页下方代码区显示的代表性片段（raw string 静态存储），
// 与页面上方的真实控件人工保持同步。

namespace gallery::snips {

inline constexpr std::string_view kLabel = R"(Mk::fui::label("Hello, mkui!");

// Colored variant
Mk::fui::textColored("Colored", ImVec4(0.4f, 0.9f, 0.6f, 1.0f));)";

inline constexpr std::string_view kButton = R"(Mk::fui::button("Click me", [&]{ ++clicks; });
Mk::fui::button("Add 10",    [&]{ clicks += 10; });)";

inline constexpr std::string_view kSlider = R"(Mk::fui::slider("Volume", volume, 0.0f, 1.0f);
// binds float& - logic side may drive it every frame)";

inline constexpr std::string_view kCheckBox = R"(Mk::fui::checkbox("Mute", mute);
// binds bool& - edits write straight back)";

inline constexpr std::string_view kDisabled = R"(Mk::fui::disabled(
    Mk::fui::slider("Volume", volume, 0.0f, 1.0f));
// visual disable; subtree still renders each frame)";

inline constexpr std::string_view kWhen = R"(Mk::fui::when(mute,
    Mk::fui::label("Now you see me"));
// subtree skipped entirely while cond is false)";

inline constexpr std::string_view kWithId = R"(Mk::fui::withId("a", button("Same ID", cbA));
Mk::fui::withId("b", button("Same ID", cbB));
// functional views have no stable address - scope IDs explicitly)";

inline constexpr std::string_view kItemWidth = R"(Mk::fui::itemWidth(220.0f,
    Mk::fui::dragF("Fixed width", drag, 0.01f, 0.0f, 10.0f));)";

inline constexpr std::string_view kInputText = R"(std::array<char, 128> buf{};
Mk::fui::inputText("Text", buf);
// capacity known at compile time, type-safe)";

inline constexpr std::string_view kInputNum = R"(Mk::fui::inputInt("Int",    i);
Mk::fui::inputFloat("Float", f);
Mk::fui::inputDouble("Double", d);)";

inline constexpr std::string_view kInputVec2 = R"(std::array<int, 2>   iv{10, 20};
std::array<float, 2> fv{1.5f, 2.5f};
Mk::fui::inputInt2("ivec2", iv);
Mk::fui::inputFloat2("vec2", fv);)";

inline constexpr std::string_view kDrag = R"(Mk::fui::dragF("Float", v, 0.01f, 0.0f, 10.0f);
Mk::fui::dragI("Int",   n, 1.0f,  0, 1000);)";

inline constexpr std::string_view kSliderIntAngle = R"(Mk::fui::sliderInt("Count", n, 0, 100);
Mk::fui::sliderAngle("Angle", radians); // deg shown, radians bound)";

inline constexpr std::string_view kRadio = R"(static constexpr const char* kItems[] = {"A", "B", "C"};
Mk::fui::radioGroup(kItems, current); // span + int&
Mk::fui::radioButton("Standalone", flag);)";

inline constexpr std::string_view kCombo = R"(Mk::fui::combo("Select", current, kItems);
// current is the selected index)";

inline constexpr std::string_view kListBox = R"(Mk::fui::listBox("Select", current, kItems);)";

inline constexpr std::string_view kTexts = R"(Mk::fui::textColored("Colored", color);
Mk::fui::textWrapped("Long text wraps automatically ...");
Mk::fui::bulletText("Bulleted item");
Mk::fui::labelText("Label", "Value");)";

inline constexpr std::string_view kProgress = R"(Mk::fui::progressBar(fraction,
    ImVec2(-FLT_MIN, 0), "Loading");)";

inline constexpr std::string_view kSelectable = R"(Mk::fui::selectable("Pick me", selected);
// selection state lives outside the view)";

inline constexpr std::string_view kImage = R"(ImTextureID tex = Mk::fui::loadTextureRGBA(px, 64, 64);
Mk::fui::image(tex, ImVec2(96, 96));
Mk::fui::imageButton("btn", tex, ImVec2(96, 96), onClick);)";

inline constexpr std::string_view kPlot = R"(std::vector<float> history; // logic side maintains
Mk::fui::plotLines("History", history, ImVec2(0, 120));)";

inline constexpr std::string_view kTree = R"(Mk::fui::treeNode("Node", ImGuiTreeNodeFlags_DefaultOpen,
    Mk::fui::treeNode("Child", 0,
        Mk::fui::label("Leaf")));)";

inline constexpr std::string_view kModifiers = R"(Mk::fui::tooltip(button("Hover me", []{}), "hint");
Mk::fui::colored(ImGuiCol_Text, green, Mk::fui::label("tinted"));
Mk::fui::indent(30.0f, Mk::fui::label("indented"));)";

inline constexpr std::string_view kTabBar = R"(Mk::fui::tabBar("Tabs", 0,
    Mk::fui::tab("One", Mk::fui::label("Page one")),
    Mk::fui::tab("Two", Mk::fui::label("Page two")));
// only the active tab renders)";

inline constexpr std::string_view kTable = R"(Mk::fui::textTable("Table", kHeaders, rows);   // data driven
Mk::fui::fnTable("Fn", kHeaders, 4,                          // cell function
    [](int r, int c){ return std::format("r{}c{}", r, c); });)";

inline constexpr std::string_view kCollapsing = R"(Mk::fui::collapsing("Section", ImGuiTreeNodeFlags_DefaultOpen,
    Mk::fui::subwindow("Child", ImVec2(0, 100), true,
        Mk::fui::label("Scrollable child window")));)";

inline constexpr std::string_view kPopup = R"(bool open = false;                    // set true to trigger
Mk::fui::button("Open", [&]{ open = true; });
Mk::fui::popup("Dialog", open, Mk::fui::label("Content"));
// modal variant: popupModal - closes regular popups)";

inline constexpr std::string_view kColor = R"(std::array<float, 3> rgb{...};
std::array<float, 4> rgba{...};
Mk::fui::colorEdit3("RGB", rgb);
Mk::fui::colorEdit4("RGBA", rgba);
Mk::fui::colorPicker3("Pick", rgb);)";

inline constexpr std::string_view kCheckboxFlags = R"(int flags = 0b101;
Mk::fui::checkboxFlags("Bit A", flags, 1); // toggles one mask bit)";

inline constexpr std::string_view kDyn = R"(Mk::ui::VBox list;                 // OOP widget
list.add(item);  list.remove(item);  // tombstone-safe in callbacks
Mk::fui::subwindow("Sub", ImVec2(0, 150), true,
    Mk::fui::dyn(list));              // embed OOP subtree)";

inline constexpr std::string_view kAsWidget = R"(auto adapter = Mk::fui::asWidget([&]{
    return Mk::fui::hbox(8.0f, Mk::fui::label("..."));
});
oopBox.add(adapter);   // functional view inside an OOP tree)";

inline constexpr std::string_view kHsplit = R"(Mk::fui::hsplit(
    Mk::fui::fixed("Nav",   150.0f, radioGroup(kNav, sel)),
    Mk::fui::stretch("Main",  1.0f, content),
    Mk::fui::fixed("Side",  220.0f, inspector));
// fixed pixels + stretch weights share the remaining space)";

inline constexpr std::string_view kVsplit = R"(Mk::fui::vsplit(
    Mk::fui::fixed("Toolbar", 44.0f, toolbar),
    Mk::fui::stretch("Main",   1.0f, plotLines("...", history)),
    Mk::fui::fixed("Status",  30.0f, statusBar));)";

inline constexpr std::string_view kDashboard = R"(Mk::fui::hbox(8.0f,
    Mk::fui::group(Mk::fui::vbox(4.0f,
        Mk::fui::separatorText("Throughput"),
        Mk::fui::textColored("+12.4%", green),
        Mk::fui::progressBar(v, ImVec2(160, 0)))), ...);)";

inline constexpr std::string_view kMasterDetail = R"(Mk::fui::hsplit(
    Mk::fui::fixed("List", 200.0f, MasterList{kDetail, sel}),
    Mk::fui::stretch("Detail", 1.0f,
        Mk::fui::when(sel == 0, detailA),
        Mk::fui::when(sel >= 1, detailB)));)";

inline constexpr std::string_view kMultiWindow = R"(Mk::MainWindow<> gallery("Gallery", 1440, 880);
Mk::window inspector("Inspector", 420, 640);
inspector.setUserUI([&]{
    // same external variables -> live cross-window link
    Mk::fui::draw(Mk::fui::slider("Volume", volume, 0.0f, 1.0f));
});
Mk::mkui<Mk::window>::run();  // drives both)";

inline constexpr std::string_view kFile = R"(Mk::fui::FileDialog dlg;   // IFileOpenDialog on a worker thread
dlg.open("Open File", {{"Text", "*.txt"}, {"All", "*.*"}});
std::string path;
if(dlg.poll(path) && !path.empty()) { /* load */ }
// main loop keeps rendering while the dialog is up)";

inline constexpr std::string_view kTheme = R"(auto t = Mk::fui::Theme::dark().withAccent(accent);
mainWindow.setTheme(t);            // per-window context

Mk::fui::styled(t.withItemSpacing(2.0f, 2.0f),   // scoped override
    Mk::fui::vbox(2.0f, Mk::fui::button("Compact", []{})));)";

} // namespace gallery::snips