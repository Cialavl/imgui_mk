#pragma once
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include "snippets.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdio>
#include <cstring>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// 画廊演示页：本地 ADL 控件（无需继承，提供 draw 重载即可）+ 38 个英文演示页。
// 页面统一结构：DemoHead（标题/说明）+ vsplit（上：控件本体，下：对应代码片段）。

namespace gallery {

using namespace Mk::fui;

// 等宽字体句柄：main 在画廊窗口构造后、首帧前 AddFontDefault 装入图集
inline ImFont* gMono = nullptr;

// ============ 本地 ADL 控件 ============

// 关弹窗按钮：必须在弹窗作用域内调用 CloseCurrentPopup
struct ClosePopupButton { std::string label; };
inline void draw(const ClosePopupButton& b) {
	if(ImGui::Button(b.label.c_str())) ImGui::CloseCurrentPopup();
}

// 侧边栏条目：header 为分组标题（不可选）
struct SidebarEntry { const char* label; bool header; };

// 侧边栏：条目列表 + 当前选中下标（Selectable 高亮单选）
struct Sidebar { std::span<const SidebarEntry> entries; int& current; };
inline void draw(const Sidebar& sb) {
	int idx = 0;
	for(const auto& e : sb.entries) {
		if(e.header) {
			ImGui::SeparatorText(e.label);
			continue;
		}
		ImGui::PushID(idx);
		const bool sel = (idx == sb.current);
		if(ImGui::Selectable(e.label, sel)) sb.current = idx;
		ImGui::PopID();
		++idx;
	}
}

// 帧计数标签
struct FrameLabel { int& frames; };
inline void draw(const FrameLabel& f) { ImGui::Text("Frame %d", f.frames); }

// 演示页骨架：标题 + 一句说明 + 分隔线
struct DemoHead { const char* title; const char* desc; };
inline void draw(const DemoHead& h) {
	ImGui::TextUnformatted(h.title);
	ImGui::TextDisabled("%s", h.desc);
	ImGui::Separator();
}

// —— 代码着色扫描：五类段，行内状态机 + 跨行块注释状态 ——

struct CodeSeg { const char* begin; const char* end; int kind; };
// kind: 0 默认 1 关键字 2 字符串 3 数字 4 注释

inline bool codeIdentChar(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}
inline bool codeDigitChar(char ch) { return ch >= '0' && ch <= '9'; }

inline bool codeKeyword(std::string_view w) {
	static constexpr std::string_view kW[] = {
		"Mk", "fui", "std", "auto", "const", "constexpr", "return", "if", "else",
		"for", "while", "float", "double", "bool", "int", "void", "struct", "class",
		"static", "namespace", "true", "false", "nullptr", "ImVec2", "ImVec4",
		"template", "typename",
	};
	for(auto k : kW) {
		if(k == w) return true;
	}
	return false;
}

// 扫描一行 [p,end)：块注释状态经 inBlock 跨行传递；段追加进 out（每行复用）
inline void scanCodeLine(const char* p, const char* end, bool& inBlock,
	std::vector<CodeSeg>& out) {
	out.clear();
	const auto push = [&](const char* b, const char* e, int kind) {
		if(b < e) out.push_back({b, e, kind});
	};
	while(p < end) {
		if(inBlock) {   // 块注释内：找 */
			const char* q = p;
			while(q + 1 < end && !(q[0] == '*' && q[1] == '/')) ++q;
			if(q + 1 < end) {
				push(p, q + 2, 4);
				inBlock = false;
				p = q + 2;
			}
			else {
				push(p, end, 4);
				return;
			}
			continue;
		}
		if(p + 1 < end && p[0] == '/' && p[1] == '/') {   // 行注释
			push(p, end, 4);
			return;
		}
		if(p + 1 < end && p[0] == '/' && p[1] == '*') {   // 块注释开始
			const char* q = p + 2;
			while(q + 1 < end && !(q[0] == '*' && q[1] == '/')) ++q;
			if(q + 1 < end) {
				push(p, q + 2, 4);
				p = q + 2;
			}
			else {
				push(p, end, 4);
				inBlock = true;
				return;
			}
			continue;
		}
		if(*p == '"') {   // 字符串字面量（处理 \ 转义，未闭合容忍到行尾）
			const char* q = p + 1;
			while(q < end) {
				if(*q == '\\') {
					q += 2;
					continue;
				}
				if(*q == '"') {
					++q;
					break;
				}
				++q;
			}
			if(q > end) q = end;
			push(p, q, 2);
			p = q;
			continue;
		}
		if(codeIdentChar(*p)) {   // 标识符/关键字
			const char* q = p;
			while(q < end && (codeIdentChar(*q) || codeDigitChar(*q))) ++q;
			push(p, q, codeKeyword(std::string_view(p, (size_t)(q - p))) ? 1 : 0);
			p = q;
			continue;
		}
		if(codeDigitChar(*p)) {   // 数字字面量（含 . 与 f 后缀）
			const char* q = p;
			while(q < end && (codeDigitChar(*q) || *q == '.' || *q == 'f')) ++q;
			push(p, q, 3);
			p = q;
			continue;
		}
		// 其它字符（括号/运算符/空白）：合并为默认段直到下一个特殊起点
		const char* q = p;
		while(q < end && !codeIdentChar(*q) && *q != '"' && *q != '/' && !codeDigitChar(*q)) ++q;
		push(p, q, 0);
		p = q;
	}
}

// 代码块：等宽字体 + 五色语法着色 + 行号列 + 深一档底色 + Copy 按钮。
// 着色颜色不写死：关键字/注释取自当前主题（accent/灰），字符串/数字固定色调、
// alpha 跟主题文本——深浅主题切换后自动协调
struct CodeBlock { std::string_view code; };
inline void draw(const CodeBlock& b) {
	const ImGuiStyle& st = ImGui::GetStyle();
	const ImVec4 baseText = st.Colors[ImGuiCol_Text];
	const ImVec4 accent = st.Colors[ImGuiCol_TabActive];   // applyTheme 映射为主题 accent
	const ImVec4 gray = st.Colors[ImGuiCol_TextDisabled];
	const auto segColor = [&](int kind) -> ImVec4 {
		switch(kind) {
		case 1: return accent;
		case 2: return ImVec4(0.55f, 0.85f, 0.58f, baseText.w);   // 字符串：绿
		case 3: return ImVec4(0.95f, 0.70f, 0.45f, baseText.w);   // 数字：橙
		case 4: return gray;
		default: return baseText;
		}
	};

	// Copy 按钮（右对齐，UI 字体）：整段（含换行）进剪贴板
	const ImVec2 avail0 = ImGui::GetContentRegionAvail();
	{
		const float w = ImGui::CalcTextSize("Copy").x + st.FramePadding.x * 2.0f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail0.x - w);
		if(ImGui::SmallButton("Copy")) {
			ImGui::SetClipboardText(std::string(b.code).c_str());
		}
	}

	// 代码子窗口：底色向文本色混合 6%（深色主题变亮一档、浅色变暗一档，两主题通用）
	ImVec4 bg = st.Colors[ImGuiCol_ChildBg];
	bg.x += (baseText.x - bg.x) * 0.06f;
	bg.y += (baseText.y - bg.y) * 0.06f;
	bg.z += (baseText.z - bg.z) * 0.06f;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

	if(gMono) ImGui::PushFont(gMono);
	if(ImGui::BeginChild("code", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border)) {
		// 行号列宽：总行数位数个 '9' 的宽（等宽字体下稳定）
		const size_t totalLines = 1 + (size_t)std::count(b.code.begin(), b.code.end(), '\n');
		char wbuf[16];
		std::snprintf(wbuf, sizeof wbuf, "%zu", totalLines);
		const std::string probe((size_t)std::strlen(wbuf), '9');
		const float numW = ImGui::CalcTextSize(probe.c_str()).x + ImGui::GetStyle().FramePadding.x;

		char numBuf[16];
		int lineNo = 1;
		bool inBlock = false;
		std::vector<CodeSeg> segs;
		const char* p = b.code.data();
		const char* end = p + b.code.size();
		while(p < end) {
			const char* eol = (const char*)std::memchr(p, '\n', (size_t)(end - p));
			if(!eol) eol = end;
			scanCodeLine(p, eol, inBlock, segs);
			p = eol + 1;

			// 行号（右对齐灰字）
			std::snprintf(numBuf, sizeof numBuf, "%d", lineNo++);
			const float tw = ImGui::CalcTextSize(numBuf).x;
			ImGui::SetCursorPosX(st.WindowPadding.x + numW - tw);
			ImGui::TextColored(gray, "%s", numBuf);
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x);

			// 着色段无缝拼接
			for(size_t i = 0; i < segs.size(); ++i) {
				if(i > 0) ImGui::SameLine(0.0f, 0.0f);
				ImGui::TextColored(segColor(segs[i].kind), "%.*s",
					(int)(segs[i].end - segs[i].begin), segs[i].begin);
			}
			if(segs.empty()) {
				ImGui::TextUnformatted("");   // 空行占位
			}
		}
	}
	ImGui::EndChild();
	if(gMono) ImGui::PopFont();
	ImGui::PopStyleColor();
}

// 主从列表：Selectable 单选，点击写回选中下标
struct MasterList { std::span<const char* const> items; int& current; };
inline void draw(const MasterList& m) {
	for(int i = 0; i < (int)m.items.size(); ++i) {
		const bool sel = (i == m.current);
		if(ImGui::Selectable(m.items[i], sel)) m.current = i;
	}
}

// 文本预览：整段 TextUnformatted（含换行）
struct FilePreview { const std::string& text; };
inline void draw(const FilePreview& p) {
	ImGui::TextUnformatted(p.text.data(), p.text.data() + p.text.size());
}

// 主色编辑器：编辑即整窗换肤（ColorEdit3 返回 true 的副作用驱动）
struct AccentEdit { Mk::fui::Theme& theme; std::array<float, 3>& col; Mk::MainWindow<>& win; };
inline void draw(const AccentEdit& e) {
	if(ImGui::ColorEdit3("Accent color", e.col.data())) {
		e.theme = e.theme.withAccent(ImVec4(e.col[0], e.col[1], e.col[2], 1.0f));
		e.win.setTheme(e.theme);
	}
}

// OOP 动态列表：自持子控件所有权池，运行时增删（墓碑安全）
class DynList : public Mk::ui::Widget {
public:
	void addItem() {
		m_Pool.push_back(std::make_unique<Mk::ui::Label>(
			("OOP item #" + std::to_string(++m_Count)).c_str()));
		add(*m_Pool.back());
	}
	void removeLast() {
		if(m_Pool.empty()) return;
		remove(*m_Pool.back());
		m_Pool.pop_back();
	}
	int count() const { return (int)m_Pool.size(); }

protected:
	void render() override {
		if(childCount() == 0) {
			ImGui::TextUnformatted("(empty - press Add OOP item)");
			return;
		}
		renderChildren();
	}

private:
	std::vector<std::unique_ptr<Mk::ui::Label>> m_Pool;
	int m_Count = 0;
};

// ============ 共享状态（函数式：视图每帧重建，状态全部外置于此）============

struct State {
	int frames = 0;
	int sel = 0;
	int clicks = 0, sameClicks = 0, imgBtnClicks = 0;
	float volume = 0.6f;
	bool mute = false;
	std::vector<float> history;

	std::array<char, 128> textBuf{};
	int iVal = 42;
	float fVal = 3.14f;
	double dVal = 2.718;
	std::array<int, 2> iv{10, 20};
	std::array<float, 2> fv{1.5f, 2.5f};
	float drag = 0.5f;
	int dragInt = 100;
	int slideI = 50;
	float angle = 0.785f;
	bool radioBool = false;
	int radio = 1, comboSel = 0, listSel = 0;

	bool rowSel = false;
	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};
	int bitFlags = 0b101;

	int navSel = 0, detailSel = 0, insFrames = 0;

	Mk::fui::Theme theme = Mk::fui::Theme::dark();
	ImTextureID tex{};

	Mk::fui::FileDialog fileDlg;
	std::string filePath, filePreview;
	std::uintmax_t fileSize = 0;
	bool fileBinary = false, fileMissing = false;

	Mk::MainWindow<>* win = nullptr;
	GLFWwindow* inspectorHandle = nullptr;

	// OOP 侧对象（存活期覆盖 run()）
	DynList dynList;
	Mk::ui::VBox oopBox{4.0f};
	struct RevMaker {
		State& s;
		auto operator()() const {
			return Mk::fui::hbox(8.0f,
				Mk::fui::label("Functional view inside an OOP container (asWidget)"),
				Mk::fui::button("Click me", [this] { ++s.clicks; }));
		}
	};
	std::unique_ptr<Mk::fui::WidgetAdapter<RevMaker>> adapter;

	static constexpr const char* kItems[] = {"Option A", "Option B", "Option C"};
	static constexpr const char* kNav[] = {"Overview", "Data", "Settings"};
	static constexpr const char* kDetail[] = {"Item 1", "Item 2", "Item 3", "Item 4"};
	static constexpr const char* kHeaders[] = {"Name", "Value", "Note"};
	std::vector<std::vector<std::string>> rows{
		{"A", "1", "data driven"},
		{"B", "2", "external vector"},
		{"C", "3", "refreshed per frame"},
	};
};

// ============ 侧边栏目录（顺序 = when 链下标）============

inline constexpr SidebarEntry kSidebar[] = {
	{"Basics", true},
	{"Label", false},                        // 0
	{"Button", false},                       // 1
	{"SliderF", false},                      // 2
	{"CheckBox", false},                     // 3
	{"disabled", false},                     // 4
	{"when", false},                         // 5
	{"withId", false},                       // 6
	{"itemWidth", false},                    // 7
	{"Inputs", true},
	{"InputText", false},                    // 8
	{"Input Int/Float/Double", false},       // 9
	{"InputInt2 / InputFloat2", false},      // 10
	{"DragF / DragI", false},                // 11
	{"SliderInt / SliderAngle", false},      // 12
	{"RadioButton / RadioGroup", false},     // 13
	{"Combo", false},                        // 14
	{"ListBox", false},                      // 15
	{"Display", true},
	{"Text variants", false},                // 16
	{"ProgressBar", false},                  // 17
	{"Selectable", false},                   // 18
	{"Image / ImageButton", false},          // 19
	{"PlotLines", false},                    // 20
	{"TreeNode", false},                     // 21
	{"tooltip / colored / indent", false},   // 22
	{"Containers", true},
	{"TabBar", false},                       // 23
	{"Table", false},                        // 24
	{"Collapsing / SubWindow", false},       // 25
	{"Popups & Colors", true},
	{"Popup / PopupModal", false},           // 26
	{"ColorEdit / ColorPicker", false},      // 27
	{"CheckboxFlags", false},                // 28
	{"Bridges", true},
	{"dyn (OOP subtree)", false},            // 29
	{"asWidget (reverse)", false},           // 30
	{"Layouts", true},
	{"Three-pane", false},                   // 31
	{"Top-down", false},                     // 32
	{"Dashboard", false},                    // 33
	{"Master-detail", false},                // 34
	{"Windows", true},
	{"Inspector link", false},               // 35
	{"Files", true},
	{"Open local file", false},              // 36
	{"Theme", true},
	{"Theme & adaptive", false},             // 37
};

// ============ 页面工厂：标题/说明 + 上演示下代码 ============

template<typename Demo>
auto page(const char* title, const char* desc, Demo demo, std::string_view code) {
	return vbox(6.0f,
		DemoHead{title, desc},
		vsplit(
			stretch("demo", 1.2f, std::move(demo)),
			stretch("code", 1.0f, vbox(4.0f,
				separatorText("Code"),
				CodeBlock{code}))));
}

// 预览区入口（hsplit 的 stretch 格子内容）
struct Preview { State& s; };
inline void draw(const Preview& p);

// ============ 38 个演示页 ============

inline void drawPreview(State& s) {
	draw(vbox(8.0f,

		// ==== Basics 0..7 ====
		when(s.sel == 0, page("Label", "Plain text display; the view holds its own string copy",
			vbox(6.0f,
				label("Hello, mkui!"),
				textColored("Colored variant (textColored)", ImVec4(0.4f, 0.9f, 0.6f, 1.0f))),
			snips::kLabel)),

		when(s.sel == 1, page("Button", "Action callback bound via std::function",
			hbox(8.0f,
				button("Click me", [&] { ++s.clicks; }),
				button("Add 10", [&] { s.clicks += 10; }),
				labelText("Clicks", std::to_string(s.clicks))),
			snips::kButton)),

		when(s.sel == 2, page("SliderF", "Binds a float reference; driven by logic-side sine wave",
			vbox(6.0f,
				slider("Volume (sine driven)", s.volume, 0.0f, 1.0f),
				slider("Draggable", s.history.back(), 0.0f, 1.0f)),
			snips::kSlider)),

		when(s.sel == 3, page("CheckBox", "Binds a bool reference; edits write straight back",
			vbox(6.0f,
				checkbox("Mute", s.mute),
				labelText("Value", s.mute ? "true" : "false")),
			snips::kCheckBox)),

		when(s.sel == 4, page("disabled", "Visual disable (greyed, non-interactive); subtree still renders",
			hbox(8.0f,
				checkbox("Check to disable right", s.mute),
				disabled(slider("Disabled slider", s.volume, 0.0f, 1.0f))),
			snips::kDisabled)),

		when(s.sel == 5, page("when", "Conditional rendering: subtree skipped while cond is false",
			vbox(6.0f,
				checkbox("Show text below", s.mute),
				when(s.mute, textColored("Now you see me", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)))),
			snips::kWhen)),

		when(s.sel == 6, page("withId", "Scope IDs explicitly - functional views have no stable address",
			vbox(6.0f,
				hbox(8.0f,
					withId("a", button("Same ID", [&] { ++s.sameClicks; })),
					withId("b", button("Same ID", [&] { ++s.sameClicks; }))),
				labelText("Independent clicks", std::to_string(s.sameClicks))),
			snips::kWithId)),

		when(s.sel == 7, page("itemWidth", "PushItemWidth around the next item in the subtree",
			vbox(6.0f,
				itemWidth(220.0f, dragF("Fixed width", s.drag, 0.01f, 0.0f, 10.0f)),
				dragF("Default width", s.drag, 0.01f, 0.0f, 10.0f)),
			snips::kItemWidth)),

		// ==== Inputs 8..15 ====
		when(s.sel == 8, page("InputText", "Binds std::array<char, N> - capacity known at compile time",
			vbox(6.0f,
				inputText("Text", s.textBuf),
				labelText("Buffer", s.textBuf.data())),
			snips::kInputText)),

		when(s.sel == 9, page("Numeric inputs", "Int / Float / Double write straight back to bound variables",
			vbox(6.0f,
				inputInt("Int", s.iVal),
				inputFloat("Float", s.fVal),
				inputDouble("Double", s.dVal)),
			snips::kInputNum)),

		when(s.sel == 10, page("vec2 inputs", "Bind std::array<T, 2>",
			vbox(6.0f,
				inputInt2("ivec2", s.iv),
				inputFloat2("vec2", s.fv)),
			snips::kInputVec2)),

		when(s.sel == 11, page("Drag inputs", "Drag to change at the given speed, clamped to min/max",
			vbox(6.0f,
				dragF("Float", s.drag, 0.01f, 0.0f, 10.0f),
				dragI("Int", s.dragInt, 1.0f, 0, 1000)),
			snips::kDrag)),

		when(s.sel == 12, page("SliderInt / SliderAngle", "Integer slider and angle slider (radians bound, degrees shown)",
			vbox(6.0f,
				sliderInt("Count", s.slideI, 0, 100),
				sliderAngle("Angle", s.angle)),
			snips::kSliderIntAngle)),

		when(s.sel == 13, page("RadioButton / RadioGroup", "Group takes a caller-owned span plus selected index",
			vbox(6.0f,
				radioGroup(State::kItems, s.radio),
				radioButton("Standalone (bool bound)", s.radioBool),
				labelText("Selected index", std::to_string(s.radio))),
			snips::kRadio)),

		when(s.sel == 14, page("Combo", "current is the selected index",
			vbox(6.0f,
				combo("Select", s.comboSel, State::kItems),
				labelText("Selected", State::kItems[s.comboSel])),
			snips::kCombo)),

		when(s.sel == 15, page("ListBox", "Same as Combo, list form",
			vbox(6.0f,
				listBox("Select", s.listSel, State::kItems),
				labelText("Selected", State::kItems[s.listSel])),
			snips::kListBox)),

		// ==== Display 16..22 ====
		when(s.sel == 16, page("Text variants", "textColored / textWrapped / bulletText / labelText",
			vbox(6.0f,
				textColored("Colored text (gold)", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)),
				textWrapped("Wrapped long text: functional views are per-frame temporaries, all state lives in bound variables - no lifetime burden."),
				bulletText("Bulleted item one"),
				bulletText("Bulleted item two"),
				labelText("Label", "Value")),
			snips::kTexts)),

		when(s.sel == 17, page("ProgressBar", "Binds a fraction (sine driven here)",
			vbox(6.0f,
				progressBar(s.volume, ImVec2(-FLT_MIN, 0), "Loading"),
				progressBar(s.volume, ImVec2(200, 20))),
			snips::kProgress)),

		when(s.sel == 18, page("Selectable", "Binds a bool - selection state lives outside the view",
			vbox(6.0f,
				selectable("Pick me (toggles)", s.rowSel),
				labelText("Selected", s.rowSel ? "true" : "false")),
			snips::kSelectable)),

		when(s.sel == 19, page("Image / ImageButton", "Texture from loadTextureRGBA (procedural checker, no files)",
			hbox(8.0f,
				image(s.tex, ImVec2(96, 96)),
				imageButton("gal", s.tex, ImVec2(96, 96), [&] { ++s.imgBtnClicks; }),
				labelText("Clicks", std::to_string(s.imgBtnClicks))),
			snips::kImage)),

		when(s.sel == 20, page("PlotLines", "Binds an external vector maintained by the logic side",
			plotLines("Sine history", s.history, ImVec2(0, 120)),
			snips::kPlot)),

		when(s.sel == 21, page("TreeNode", "Renders children only while expanded; nests freely",
			treeNode("Node (default open)", ImGuiTreeNodeFlags_DefaultOpen,
				treeNode("Child", 0, label("Deepest leaf"))),
			snips::kTree)),

		when(s.sel == 22, page("tooltip / colored / indent", "Three decorators stacked",
			vbox(6.0f,
				tooltip(button("Hover me", [] {}), "tooltip decorator"),
				colored(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.6f, 1.0f), label("colored text")),
				indent(30.0f, label("indented text"))),
			snips::kModifiers)),

		// ==== Containers 23..25 ====
		when(s.sel == 23, page("TabBar", "Only the active tab renders each frame",
			tabBar("Demo tabs", 0,
				tab("One", label("Page one")),
				tab("Two", label("Page two"))),
			snips::kTabBar)),

		when(s.sel == 24, page("Table", "Left: textTable (data driven); right: fnTable (cell function)",
			hbox(8.0f,
				textTable("Table", State::kHeaders, s.rows),
				fnTable("Fn", State::kHeaders, 4, [](int r, int c) {
					return "r" + std::to_string(r) + "c" + std::to_string(c);
				})),
			snips::kTable)),

		when(s.sel == 25, page("Collapsing / SubWindow", "Collapsing container nesting a scrollable child window",
			vbox(6.0f,
				collapsing("Section (default open)", ImGuiTreeNodeFlags_DefaultOpen,
					subwindow("Child", ImVec2(0, 100), true,
						label("Scrollable child window"))),
				subwindow("Standalone child", ImVec2(0, 80), true,
					label("Another child window"))),
			snips::kCollapsing)),

		// ==== Popups & Colors 26..28 ====
		when(s.sel == 26, page("Popup / PopupModal", "Flag-triggered: setting the flag opens it next frame",
			hbox(8.0f,
				button("Open popup", [&] { s.popupOpen = true; }),
				button("Open modal", [&] { s.modalOpen = true; })),
			snips::kPopup)),

		when(s.sel == 27, page("ColorEdit / ColorPicker", "Bind std::array<float, 3|4>",
			vbox(6.0f,
				colorEdit3("RGB", s.col3),
				colorEdit4("RGBA", s.col4),
				collapsing("Pickers (click to expand)", 0,
					hbox(8.0f,
						colorPicker3("Pick RGB", s.pick3),
						colorPicker4("Pick RGBA", s.pick4)))),
			snips::kColor)),

		when(s.sel == 28, page("CheckboxFlags", "Binds int& and toggles one mask bit",
			vbox(6.0f,
				hbox(8.0f,
					checkboxFlags("Bit A (1)", s.bitFlags, 1),
					checkboxFlags("Bit B (2)", s.bitFlags, 2),
					checkboxFlags("Bit C (4)", s.bitFlags, 4)),
				labelText("Value", "0b" + std::bitset<3>(s.bitFlags).to_string())),
			snips::kCheckboxFlags)),

		// ==== Bridges 29..30 ====
		when(s.sel == 29, page("dyn", "Embed an OOP subtree (tombstone add/remove) in a functional tree",
			vbox(6.0f,
				hbox(8.0f,
					button("Add OOP item", [&] { s.dynList.addItem(); }),
					button("Remove last", [&] { s.dynList.removeLast(); }),
					labelText("Items", std::to_string(s.dynList.count()))),
				subwindow("OOP dynamic subtree", ImVec2(0, 150), true,
					dyn(s.dynList))),
			snips::kDyn)),

		when(s.sel == 30, page("asWidget", "Functional view inside an OOP tree; maker runs every frame",
			subwindow("OOP container", ImVec2(0, 100), true,
				dyn(s.oopBox)),
			snips::kAsWidget)),

		// ==== Layouts 31..34 ====
		when(s.sel == 31, page("Three-pane", "hsplit: nav + content + inspector (fixed / stretch / fixed)",
			hsplit(
				fixed("Nav", 150.0f, radioGroup(State::kNav, s.navSel)),
				stretch("Content", 1.0f, vbox(0.0f,
					when(s.navSel == 0, plotLines("Overview", s.history, ImVec2(0, 120))),
					when(s.navSel == 1, textTable("Data", State::kHeaders, s.rows)),
					when(s.navSel == 2, vbox(4.0f,
						slider("Setting", s.volume, 0.0f, 1.0f),
						checkbox("Toggle", s.mute))))),
				fixed("Side", 220.0f, vbox(4.0f,
					colorEdit3("Theme color", s.col3),
					progressBar(s.volume, ImVec2(-FLT_MIN, 0))))),
			snips::kHsplit)),

		when(s.sel == 32, page("Top-down", "vsplit: toolbar + main + status bar (fixed / stretch / fixed)",
			vsplit(
				fixed("Toolbar", 44.0f, hbox(4.0f,
					button("New", [] {}),
					button("Open", [] {}),
					button("Save", [] {}))),
				stretch("Main", 1.0f, plotLines("Main plot", s.history, ImVec2(0, 100))),
				fixed("Status", 30.0f, hbox(16.0f,
					labelText("Frame", std::to_string(s.frames)),
					labelText("Volume", std::to_string(s.volume).substr(0, 4)),
					labelText("Mute", s.mute ? "yes" : "no")))),
			snips::kVsplit)),

		when(s.sel == 33, page("Dashboard", "Group packs metric cards into a grid",
			vbox(8.0f,
				hbox(8.0f,
					group(vbox(4.0f,
						separatorText("Throughput"),
						textColored("+12.4%", ImVec4(0.4f, 0.9f, 0.6f, 1.0f)),
						progressBar(s.volume, ImVec2(160, 0)))),
					group(vbox(4.0f,
						separatorText("Latency"),
						textColored("-3.1%", ImVec4(0.4f, 0.9f, 0.6f, 1.0f)),
						progressBar(s.history.back(), ImVec2(160, 0))))),
				hbox(8.0f,
					group(vbox(4.0f,
						separatorText("Error rate"),
						textColored("+0.2%", ImVec4(0.9f, 0.4f, 0.4f, 1.0f)),
						plotLines("err", s.history, ImVec2(160, 48)))),
					group(vbox(4.0f,
						separatorText("Connections"),
						textColored("128", ImVec4(0.6f, 0.8f, 1.0f, 1.0f)),
						labelText("Limit", "256"))))),
			snips::kDashboard)),

		when(s.sel == 34, page("Master-detail", "Local ADL MasterList drives the right-hand detail views",
			hsplit(
				fixed("List", 200.0f, MasterList{State::kDetail, s.detailSel}),
				stretch("Detail", 1.0f, vbox(0.0f,
					when(s.detailSel == 0, vbox(4.0f,
						separatorText("Item 1"),
						labelText("Status", "in progress"),
						slider("Progress", s.volume, 0.0f, 1.0f))),
					when(s.detailSel == 1, vbox(4.0f,
						separatorText("Item 2"),
						labelText("Status", "suspended"),
						checkbox("Enabled", s.mute))),
					when(s.detailSel >= 2, vbox(4.0f,
						separatorText("Other item"),
						labelText("Index", std::to_string(s.detailSel)),
						colorEdit3("Mark color", s.col3)))))),
			snips::kMasterDetail)),

		// ==== Windows 35 ====
		when(s.sel == 35, page("Inspector link", "Second top-level window shares all external state",
			vbox(6.0f,
				labelText("Gallery frame", std::to_string(s.frames)),
				labelText("Inspector frame", std::to_string(s.insFrames)),
				button(s.inspectorHandle && glfwGetWindowAttrib(s.inspectorHandle, GLFW_VISIBLE)
					? "Hide Inspector window" : "Show Inspector window", [&] {
					if(s.inspectorHandle) {
						if(glfwGetWindowAttrib(s.inspectorHandle, GLFW_VISIBLE))
							glfwHideWindow(s.inspectorHandle);
						else
							glfwShowWindow(s.inspectorHandle);
					}
				}),
				textWrapped("Two top-level windows each own an ImGui context and frame loop; binding the same variables keeps them live-linked. Closing the Inspector only hides it; it follows the Gallery out when the Gallery closes.")),
			snips::kMultiWindow)),

		// ==== Files 36 ====
		when(s.sel == 36, page("Open local file", "FileDialog runs IFileOpenDialog on a worker thread",
			vbox(6.0f,
				hbox(8.0f,
					button("Open file...", [&] {
						s.fileDlg.open("Open File", {
							{"Text/Code", "*.txt;*.md;*.cpp;*.h;*.hpp;*.json;*.xml;*.ini;*.log"},
							{"All files", "*.*"},
						});
					}),
					when(s.fileDlg.busy(), textColored("Dialog open... (main window still rendering)",
						ImVec4(1.0f, 0.8f, 0.2f, 1.0f)))),
				labelText("Path", s.filePath.empty() ? "(none)" : s.filePath.c_str()),
				labelText("Size", std::to_string(s.fileSize) + " bytes"),
				when(s.fileMissing, textColored("File unreadable", ImVec4(0.9f, 0.4f, 0.4f, 1.0f))),
				when(!s.filePath.empty() && s.fileBinary,
					textColored("Binary file: size only", ImVec4(0.6f, 0.8f, 1.0f, 1.0f))),
				when(!s.filePath.empty() && !s.fileBinary,
					subwindow("Text preview (first 64KB)", ImVec2(0, 0), true,
						FilePreview{s.filePreview}))),
			snips::kFile)),

		// ==== Theme 37 ====
		when(s.sel == 37, page("Theme & adaptive", "Theme value type: presets, chained derivation, scoped overrides",
			vbox(6.0f,
				hbox(8.0f,
					button("Dark", [&] {
						s.theme = Mk::fui::Theme::dark();
						s.win->setTheme(s.theme);
					}),
					button("Light", [&] {
						s.theme = Mk::fui::Theme::light();
						s.win->setTheme(s.theme);
					}),
					AccentEdit{s.theme, s.col3, *s.win}),
				separatorText("styled scope: local compact spacing (global untouched)"),
				styled(s.theme.withItemSpacing(2.0f, 2.0f).withRounding(0.0f),
					vbox(2.0f,
						button("Compact button", [] {}),
						button("Another compact button", [] {}))),
				separatorText("hsplit/vsplit mixed weights"),
				hsplit(
					fixed("sa", 100.0f, labelText("A", "fixed 100")),
					stretch("sb", 2.0f, labelText("B", "stretch 2")),
					stretch("sc", 1.0f, labelText("C", "stretch 1")),
					fixed("sd", 90.0f, labelText("D", "fixed 90"))),
				vsplit(
					fixed("se", 40.0f, labelText("E", "fixed 40")),
					stretch("sf", 1.0f, labelText("F", "stretch 1")))),
			snips::kTheme))
	));
}

inline void draw(const Preview& p) { drawPreview(p.s); }

} // namespace gallery