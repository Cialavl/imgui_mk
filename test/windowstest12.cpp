#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <bitset>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <cstdio>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace {

// —— 画廊本地扩展控件（ADL：无需继承，提供 draw 重载即可被容器接受）——

// 关弹窗按钮：draw 内直接调 ImGui::CloseCurrentPopup（必须在弹窗作用域内）
struct ClosePopupButton { std::string label; };
inline void draw(const ClosePopupButton& b) {
	if(ImGui::Button(b.label.c_str())) ImGui::CloseCurrentPopup();
}

// 侧边栏条目：header 为分组标题（不可选）
struct SidebarEntry { const char* label; bool header; };

// 侧边栏：条目列表 + 当前选中下标（Selectable 高亮，点击切换右侧演示）
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

// 帧计数标签：每帧刷新（画廊动画源之一）
struct FrameLabel { int& frames; };
inline void draw(const FrameLabel& f) {
	ImGui::Text("帧 %d", f.frames);
}

// 演示面板骨架：标题 + 一句说明 + 分隔线（右侧内容统一开头）
struct DemoHead { const char* title; const char* desc; };
inline void draw(const DemoHead& h) {
	ImGui::TextUnformatted(h.title);
	ImGui::TextDisabled("%s", h.desc);
	ImGui::Separator();
}

// 主从列表（本地 ADL 控件）：Selectable 单选风格，点击写回选中下标
struct MasterList { std::span<const char* const> items; int& current; };
inline void draw(const MasterList& m) {
	for(int i = 0; i < (int)m.items.size(); ++i) {
		const bool sel = (i == m.current);
		if(ImGui::Selectable(m.items[i], sel)) m.current = i;
	}
}

// 文本预览（本地 ADL 控件）：整段 TextUnformatted（含换行），外部 string 引用
struct FilePreview { const std::string& text; };
inline void draw(const FilePreview& p) {
	ImGui::TextUnformatted(p.text.data(), p.text.data() + p.text.size());
}

// —— OOP 动态列表（dyn 桥演示）：自持子控件所有权池，运行时增删 ——

class DynList : public Mk::ui::Widget {
public:
	void addItem() {
		m_Pool.push_back(std::make_unique<Mk::ui::Label>(
			("OOP 列表项 #" + std::to_string(++m_Count)).c_str()));
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
			ImGui::TextUnformatted("（空列表：点“添加 OOP 项”）");
			return;
		}
		renderChildren();
	}

private:
	std::vector<std::unique_ptr<Mk::ui::Label>> m_Pool;
	int m_Count = 0;
};

// 程序生成 64x64 棋盘格 RGBA 纹理（无外部文件依赖）
static unsigned char g_Pixels[64 * 64 * 4];
static void MakeChecker() {
	for(int y = 0; y < 64; ++y) {
		for(int x = 0; x < 64; ++x) {
			const bool odd = ((x / 8) + (y / 8)) % 2 == 1;
			unsigned char* px = &g_Pixels[(y * 64 + x) * 4];
			px[0] = odd ? 220 : 60;
			px[1] = odd ? 160 : 120;
			px[2] = odd ? 60 : 220;
			px[3] = 255;
		}
	}
}

// 侧边栏目录（顺序 = 右侧 when 链的下标，条目见 main 内注释）
static constexpr SidebarEntry kSidebar[] = {
	{"基础", true},
	{"Label 文本", false},            // 0
	{"Button 按钮", false},           // 1
	{"SliderF 滑块", false},          // 2
	{"CheckBox 复选框", false},       // 3
	{"disabled 禁用修饰", false},     // 4
	{"when 条件修饰", false},         // 5
	{"withId 同 ID 隔离", false},     // 6
	{"itemWidth 定宽修饰", false},    // 7
	{"输入", true},
	{"InputText 文本输入", false},    // 8
	{"数值输入 Int/Float/Double", false}, // 9
	{"vec2 二分量输入", false},       // 10
	{"Drag 拖拽输入", false},         // 11
	{"SliderInt/Angle", false},       // 12
	{"RadioGroup 单选组", false},     // 13
	{"Combo 下拉选择", false},        // 14
	{"ListBox 列表选择", false},      // 15
	{"显示", true},
	{"彩色/换行/圆点文本", false},    // 16
	{"ProgressBar 进度条", false},    // 17
	{"Selectable 可选行", false},     // 18
	{"Image 图片", false},            // 19
	{"PlotLines 折线图", false},      // 20
	{"TreeNode 树节点", false},       // 21
	{"tooltip/colored/indent", false}, // 22
	{"容器", true},
	{"TabBar 标签页", false},         // 23
	{"Table 表格", false},            // 24
	{"Collapsing/SubWindow", false},  // 25
	{"弹窗与颜色", true},
	{"Popup 弹窗", false},            // 26
	{"ColorEdit/Picker", false},      // 27
	{"CheckboxFlags 位标志", false},  // 28
	{"双向桥", true},
	{"dyn OOP 子树", false},          // 29
	{"asWidget 反向桥", false},       // 30
	{"布局样例", true},
	{"三栏布局", false},              // 31
	{"上下结构", false},              // 32
	{"仪表盘", false},                // 33
	{"主从布局", false},              // 34
	{"多窗口联动", true},
	{"检查器联动", false},            // 35
	{"文件", true},
	{"打开本地文件", false},          // 36
};

} // namespace

// mkui 控件画廊（侧边栏版）：左侧目录列出全部控件（点击切换），右侧实时演示。
// 常驻层：菜单栏 + 可拖动浮动面板 + 三个弹窗（Popup 演示页触发）。
// 本地 ADL 扩展：Sidebar / DemoHead / ClosePopupButton / FrameLabel。
int main() {
	Mk::MainWindow<> mainWindow("mkui 控件画廊", 1440, 880);

	// ============ 外置状态 ============
	int frames = 0;
	int sel = 0;                       // 侧边栏选中下标
	int clicks = 0, sameClicks = 0, imgBtnClicks = 0;
	float volume = 0.6f;
	bool mute = false;
	std::vector<float> history;
	history.reserve(512);

	std::array<char, 128> textBuf{};
	std::snprintf(textBuf.data(), textBuf.size(), "mkui");
	int iVal = 42; float fVal = 3.14f; double dVal = 2.718;
	std::array<int, 2> iv{10, 20};
	std::array<float, 2> fv{1.5f, 2.5f};
	float drag = 0.5f; int dragInt = 100;
	int slideI = 50; float angle = 0.785f;
	bool radioBool = false;
	int radio = 1, comboSel = 0, listSel = 0;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};

	bool rowSel = false;
	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};
	int bitFlags = 0b101;
	int navSel = 0, detailSel = 0, insFrames = 0;
	static constexpr const char* kNav[] = {"概览", "数据", "设置"};
	Mk::fui::FileDialog fileDlg;        // 异步文件对话框（worker 线程跑 IFileOpenDialog）
	std::string filePath, filePreview;  // 选中路径与文本预览（前 64KB）
	std::uintmax_t fileSize = 0;
	bool fileBinary = false, fileMissing = false;
	static constexpr const char* kDetail[] = {"项目甲", "项目乙", "项目丙", "项目丁"};

	static constexpr const char* kHeaders[] = {"名称", "数值", "备注"};
	std::vector<std::vector<std::string>> rows{
		{"甲", "1", "数据驱动"},
		{"乙", "2", "外部 vector"},
		{"丙", "3", "每帧刷新"},
	};

	MakeChecker();
	const ImTextureID tex = Mk::fui::loadTextureRGBA(g_Pixels, 64, 64);

	// OOP 侧对象（存活期覆盖 run()）
	DynList dynList;
	Mk::ui::VBox oopBox(4.0f);
	auto revAdapter = Mk::fui::asWidget([&] {
		return Mk::fui::hbox(8.0f,
			Mk::fui::label("OOP 容器内的函数式视图（asWidget 反向桥）"),
			Mk::fui::button("点我", [&] { ++clicks; }));
	});
	oopBox.add(revAdapter);

	// —— 第二顶层窗口：检查器（多窗口联动）。与画廊共享全部外置状态；
	// 点 X 仅隐藏（关闭回调拦截，不进引擎销毁流程），
	// 画廊关闭（引擎销毁）后检查器下一帧请求自身关闭，run() 随之退出
	Mk::window inspector("检查器（多窗口联动）", 420, 640);
	if(auto* h = inspector.getHandle()) {
		glfwSetWindowCloseCallback(h, [](GLFWwindow* w) {
			glfwSetWindowShouldClose(w, GLFW_FALSE);
			glfwHideWindow(w);
		});
	}
	inspector.setUserUI([&] {
		if(mainWindow.getHandle() == nullptr) {
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		++insFrames;
		// 文件对话框轮询：完成即读取（文本截取前 64KB，二进制只记大小）
		std::string picked;
		if(fileDlg.poll(picked)) {
			filePath = picked;
			filePreview.clear();
			fileSize = 0;
			fileBinary = false;
			fileMissing = false;
			if(!filePath.empty()) {
				std::error_code ec;
				fileSize = std::filesystem::file_size(filePath, ec);
				if(ec) { fileMissing = true; }
				else {
					std::ifstream in(filePath, std::ios::binary);
					constexpr std::size_t kCap = 64 * 1024;
					std::string buf(kCap, '\0');
					in.read(buf.data(), (std::streamsize)kCap);
					buf.resize((size_t)in.gcount());
					fileBinary = buf.find('\0') != std::string::npos;
					if(!fileBinary) filePreview = std::move(buf);
				}
			}
		}

		using namespace Mk::fui;
		draw(vbox(8.0f,
			separatorText("共享状态（与画廊实时联动）"),
			slider("音量", volume, 0.0f, 1.0f),
			checkbox("静音", mute),
			colorEdit3("主题色", col3),
			hbox(8.0f,
				checkboxFlags("位 A", bitFlags, 1),
				checkboxFlags("位 B", bitFlags, 2),
				checkboxFlags("位 C", bitFlags, 4)),
			separatorText("独立帧循环"),
			labelText("画廊帧", std::to_string(frames)),
			labelText("检查器帧", std::to_string(insFrames)),
			textWrapped("两个顶层窗口各有独立 ImGui 上下文与帧循环，绑定同一批外部变量即可实时联动：拖动任意窗口的滑块或颜色，另一窗口同帧跟随。点 X 仅隐藏本窗口，菜单 窗口 或画廊侧边栏可再次显示；画廊关闭时检查器自动跟随退出。")));
	});

	// 打开本地文件：启动异步对话框，完成后由每帧 poll 装载
	auto openFileDialog = [&] {
		fileDlg.open("打开文件（mkui 画廊）", {
			{"文本/代码", "*.txt;*.md;*.cpp;*.h;*.hpp;*.json;*.xml;*.ini;*.log"},
			{"所有文件", "*.*"},
		});
	};

	GLFWwindow* handle = mainWindow.getHandle();

	mainWindow.setUI([&] {
		// 动画驱动
		++frames;
		volume = 0.5f + 0.5f * std::sin(frames * 0.02f);
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 300) history.erase(history.begin());

		// 文件对话框轮询：完成即读取（文本截取前 64KB，二进制只记大小）
		std::string picked;
		if(fileDlg.poll(picked)) {
			filePath = picked;
			filePreview.clear();
			fileSize = 0;
			fileBinary = false;
			fileMissing = false;
			if(!filePath.empty()) {
				std::error_code ec;
				fileSize = std::filesystem::file_size(filePath, ec);
				if(ec) { fileMissing = true; }
				else {
					std::ifstream in(filePath, std::ios::binary);
					constexpr std::size_t kCap = 64 * 1024;
					std::string buf(kCap, '\0');
					in.read(buf.data(), (std::streamsize)kCap);
					buf.resize((size_t)in.gcount());
					fileBinary = buf.find('\0') != std::string::npos;
					if(!fileBinary) filePreview = std::move(buf);
				}
			}
		}

		using namespace Mk::fui;

		// —— 常驻层：菜单栏 + 浮动面板 ——
		const bool insVisible = inspector.getHandle() != nullptr
			&& glfwGetWindowAttrib(inspector.getHandle(), GLFW_VISIBLE) != 0;

		draw(menuBar(
			menu("文件",
				menuItem("打开…", openFileDialog, "Ctrl+O"),
				menuItem("退出", [handle] { glfwSetWindowShouldClose(handle, true); }, "Alt+F4")),
			menu("窗口",
				menuItem(insVisible ? "隐藏检查器" : "显示检查器", [&] {
					if(auto* h = inspector.getHandle()) {
						if(glfwGetWindowAttrib(h, GLFW_VISIBLE)) glfwHideWindow(h);
						else glfwShowWindow(h);
					}
				})),
			menu("帮助",
				menuItem("关于画廊", [&] { aboutOpen = true; }))));

		draw(panel("浮动面板（Panel，可拖动）", 0,
			slider("面板滑块", volume, 0.0f, 1.0f),
			FrameLabel{frames}));

		// —— 主体：左目录 + 右演示 ——
		draw(hbox(0.0f,
			subwindow("sidebar", ImVec2(250, 0), true,
				Sidebar{kSidebar, sel}),
			subwindow("preview", ImVec2(0, 0), true,
				vbox(8.0f,

				// ==== 基础（0..7）====
				when(sel == 0, vbox(6.0f,
					DemoHead{"Label 文本", "最简单的显示控件：持字符串副本的纯值"},
					label("你好，mkui！"),
					textColored("彩色变体 textColored", ImVec4(0.4f, 0.9f, 0.6f, 1.0f)))),

				when(sel == 1, vbox(6.0f,
					DemoHead{"Button 按钮", "点击回调：Button 工厂 + std::function"},
					hbox(8.0f,
						button("点我计数", [&] { ++clicks; }),
						button("再来一个", [&] { clicks += 10; })),
					labelText("点击次数", std::to_string(clicks)))),

				when(sel == 2, vbox(6.0f,
					DemoHead{"SliderF 滑块", "绑定 float 引用；下面滑块被逻辑侧正弦驱动"},
					slider("音量（正弦驱动）", volume, 0.0f, 1.0f),
					slider("可拖动的滑块", history.back(), 0.0f, 1.0f))),

				when(sel == 3, vbox(6.0f,
					DemoHead{"CheckBox 复选框", "绑定 bool 引用，交互直接写回"},
					checkbox("静音", mute),
					labelText("当前值", mute ? "true" : "false"))),

				when(sel == 4, vbox(6.0f,
					DemoHead{"disabled 禁用修饰", "视觉禁用（灰化不可交互），子视图仍每帧渲染"},
					hbox(8.0f,
						checkbox("勾选以禁用右侧", mute),
						disabled(slider("被禁用的滑块", volume, 0.0f, 1.0f))))),

				when(sel == 5, vbox(6.0f,
					DemoHead{"when 条件修饰", "cond 为 false 时整棵子树本帧跳过"},
					checkbox("显示下方文本", mute),
					when(mute, textColored("现在你能看见我", ImVec4(1.0f, 0.8f, 0.2f, 1.0f))))),

				when(sel == 6, vbox(6.0f,
					DemoHead{"withId 同 ID 隔离", "函数式视图无稳定地址，同 label 冲突时显式指定 ID"},
					hbox(8.0f,
						withId("a", button("同 ID 按钮", [&] { ++sameClicks; })),
						withId("b", button("同 ID 按钮", [&] { ++sameClicks; }))),
					labelText("两边独立可点", std::to_string(sameClicks)))),

				when(sel == 7, vbox(6.0f,
					DemoHead{"itemWidth 定宽修饰", "PushItemWidth 包裹子视图内下一个 item"},
					itemWidth(220.0f, dragF("定宽的拖拽条", drag, 0.01f, 0.0f, 10.0f)),
					dragF("默认宽度的拖拽条", drag, 0.01f, 0.0f, 10.0f))),

				// ==== 输入（8..15）====
				when(sel == 8, vbox(6.0f,
					DemoHead{"InputText 文本输入", "绑定 std::array<char,N>（容量编译期可知）"},
					inputText("文本", textBuf),
					labelText("缓冲内容", textBuf.data()))),

				when(sel == 9, vbox(6.0f,
					DemoHead{"数值输入", "Int/Float/Double：交互直接写回外部变量"},
					inputInt("整数", iVal),
					inputFloat("浮点", fVal),
					inputDouble("双精度", dVal))),

				when(sel == 10, vbox(6.0f,
					DemoHead{"vec2 二分量输入", "绑定 std::array<T,2>"},
					inputInt2("ivec2", iv),
					inputFloat2("vec2", fv))),

				when(sel == 11, vbox(6.0f,
					DemoHead{"Drag 拖拽输入", "按 speed 拖动改变，mn/mx 限范围"},
					dragF("拖拽浮点", drag, 0.01f, 0.0f, 10.0f),
					dragI("拖拽整数", dragInt, 1.0f, 0, 1000))),

				when(sel == 12, vbox(6.0f,
					DemoHead{"SliderInt / SliderAngle", "整数滑块与角度滑块（弧度绑定，角度显示）"},
					sliderInt("整数滑块", slideI, 0, 100),
					sliderAngle("角度", angle))),

				when(sel == 13, vbox(6.0f,
					DemoHead{"RadioGroup 单选组", "items 为调用方持有的 span，current 为选中下标"},
					radioGroup(kItems, radio),
					radioButton("独立单选（bool 绑定）", radioBool),
					labelText("选中下标", std::to_string(radio)))),

				when(sel == 14, vbox(6.0f,
					DemoHead{"Combo 下拉选择", "current 为选中下标"},
					combo("下拉", comboSel, kItems),
					labelText("选中", kItems[comboSel]))),

				when(sel == 15, vbox(6.0f,
					DemoHead{"ListBox 列表选择", "同 Combo，列表形态"},
					listBox("列表", listSel, kItems),
					labelText("选中", kItems[listSel]))),

				// ==== 显示（16..22）====
				when(sel == 16, vbox(6.0f,
					DemoHead{"彩色/换行/圆点文本", "TextColored / TextWrapped / BulletText / LabelText"},
					textColored("彩色文本（金色）", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)),
					textWrapped("自动换行长文本：函数式控件库中视图是每帧构造的临时值，状态全部外置于被绑定的变量，因此不存在悬垂与生命周期负担。"),
					bulletText("圆点列表项一"),
					bulletText("圆点列表项二"),
					labelText("外置标签", "外置值"))),

				when(sel == 17, vbox(6.0f,
					DemoHead{"ProgressBar 进度条", "绑定 fraction（此处被正弦驱动）"},
					progressBar(volume, ImVec2(-FLT_MIN, 0), "加载中"),
					progressBar(volume, ImVec2(200, 20)))),

				when(sel == 18, vbox(6.0f,
					DemoHead{"Selectable 可选行", "绑定 bool（选中态外置）"},
					selectable("点我切换选中态", rowSel),
					labelText("选中", rowSel ? "true" : "false"))),

				when(sel == 19, vbox(6.0f,
					DemoHead{"Image 图片", "ImTextureID + 纹理上传辅助 loadTextureRGBA（棋盘程序生成）"},
					hbox(8.0f,
						image(tex, ImVec2(96, 96)),
						imageButton("画廊图钮", tex, ImVec2(96, 96), [&] { ++imgBtnClicks; })),
					labelText("图钮点击", std::to_string(imgBtnClicks)))),

				when(sel == 20, vbox(6.0f,
					DemoHead{"PlotLines 折线图", "绑定外部 vector（逻辑侧维护，滚动窗口）"},
					plotLines("正弦历史", history, ImVec2(0, 120)))),

				when(sel == 21, vbox(6.0f,
					DemoHead{"TreeNode 树节点", "展开时才渲染子视图，可任意嵌套"},
					treeNode("一级节点（默认展开）", ImGuiTreeNodeFlags_DefaultOpen,
						treeNode("二级节点", 0,
							label("最深层的子视图"))))),

				when(sel == 22, vbox(6.0f,
					DemoHead{"tooltip / colored / indent", "三个修饰器的叠加演示"},
					tooltip(button("悬停看提示", [] {}), "tooltip 修饰器"),
					colored(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.6f, 1.0f),
						label("colored 修饰的文本")),
					indent(30.0f, label("indent 修饰的文本")))),

				// ==== 容器（23..25）====
				when(sel == 23, vbox(6.0f,
					DemoHead{"TabBar 标签页", "仅激活页的子视图每帧渲染；可任意嵌套"},
					tabBar("演示标签栏", 0,
						tab("页一", label("第一页内容")),
						tab("页二", label("第二页内容"))))),

				when(sel == 24, vbox(6.0f,
					DemoHead{"Table 表格", "左：TextTable 数据驱动；右：FnTable 泛型单元格"},
					hbox(8.0f,
						textTable("文本表", kHeaders, rows),
						fnTable("函数表", kHeaders, 4, [](int r, int c) {
							return "r" + std::to_string(r) + "c" + std::to_string(c);
						})))),

				when(sel == 25, vbox(6.0f,
					DemoHead{"Collapsing / SubWindow", "折叠容器内嵌子窗口"},
					collapsing("折叠容器（默认展开）", ImGuiTreeNodeFlags_DefaultOpen,
						subwindow("折叠内子窗口", ImVec2(0, 100), true,
							label("子窗口（BeginChild）可独立滚动"))),
					subwindow("独立子窗口", ImVec2(0, 80), true,
						label("另一个子窗口")))),

				// ==== 弹窗与颜色（26..28）====
				when(sel == 26, vbox(6.0f,
					DemoHead{"Popup / PopupModal", "标志触发式：置位当帧打开；模态会强制关闭普通弹窗"},
					hbox(8.0f,
						button("打开普通弹窗", [&] { popupOpen = true; }),
						button("打开模态弹窗", [&] { modalOpen = true; })))),

				when(sel == 27, vbox(6.0f,
					DemoHead{"ColorEdit / ColorPicker", "绑定 std::array<float,3|4>"},
					colorEdit3("RGB 编辑", col3),
					colorEdit4("RGBA 编辑", col4),
					collapsing("颜色拾取器（点击展开）", 0,
						hbox(8.0f,
							colorPicker3("RGB 拾取", pick3),
							colorPicker4("RGBA 拾取", pick4))))),

				when(sel == 28, vbox(6.0f,
					DemoHead{"CheckboxFlags 位标志", "绑定 int& 按 mask 切换位"},
					hbox(8.0f,
						checkboxFlags("位 A(1)", bitFlags, 1),
						checkboxFlags("位 B(2)", bitFlags, 2),
						checkboxFlags("位 C(4)", bitFlags, 4)),
					labelText("位标志值", "0b" + std::bitset<3>(bitFlags).to_string()))),

				// ==== 双向桥（29..30）====
				when(sel == 29, vbox(6.0f,
					DemoHead{"dyn：函数式树内嵌 OOP 子树", "DynList 是 Mk::ui::Widget（墓碑增删安全）"},
					hbox(8.0f,
						button("添加 OOP 项", [&] { dynList.addItem(); }),
						button("删除末项", [&] { dynList.removeLast(); }),
						labelText("项数", std::to_string(dynList.count()))),
					subwindow("OOP 动态子树", ImVec2(0, 150), true,
						dyn(dynList)))),

				when(sel == 30, vbox(6.0f,
					DemoHead{"asWidget：OOP 树内嵌函数式视图", "适配器每帧调用 maker，保持每帧重建语义"},
					subwindow("OOP 容器", ImVec2(0, 100), true,
						dyn(oopBox)))),

				// ==== 布局样例（31..34）====
				when(sel == 31, vbox(6.0f,
					DemoHead{"三栏布局", "导航 + 内容 + 检查器：左中右三个子窗口，左右定宽、中间自适应"},
					hbox(0.0f,
						subwindow("布局导航", ImVec2(150, 0), true,
							radioGroup(kNav, navSel)),
						subwindow("布局内容", ImVec2(0, 0), true,
							when(navSel == 0, plotLines("概览折线", history, ImVec2(0, 120))),
							when(navSel == 1, textTable("数据表", kHeaders, rows)),
							when(navSel == 2, vbox(4.0f,
								slider("设置项", volume, 0.0f, 1.0f),
								checkbox("开关", mute)))),
						subwindow("布局检查器", ImVec2(220, 0), true,
							vbox(4.0f,
								colorEdit3("主题色", col3),
								progressBar(volume, ImVec2(-FLT_MIN, 0))))))),

				when(sel == 32, vbox(0.0f,
					DemoHead{"上下结构", "工具栏 + 主内容 + 状态栏：顶部底部定高、中间自适应"},
					subwindow("工具栏", ImVec2(0, 44), true,
						hbox(4.0f,
							button("新建", [] {}),
							button("打开", [] {}),
							button("保存", [] {}))),
					subwindow("主内容", ImVec2(0, 0), true,
						plotLines("主内容区折线", history, ImVec2(0, 100))),
					subwindow("状态栏", ImVec2(0, 30), true,
						hbox(16.0f,
							labelText("帧", std::to_string(frames)),
							labelText("音量", std::to_string(volume).substr(0, 4)),
							labelText("静音", mute ? "是" : "否"))))),

				when(sel == 33, vbox(8.0f,
					DemoHead{"仪表盘", "Group 打包指标卡，hbox/vbox 排成网格"},
					hbox(8.0f,
						group(vbox(4.0f,
							separatorText("吞吐"),
							textColored("+12.4%", ImVec4(0.4f, 0.9f, 0.6f, 1.0f)),
							progressBar(volume, ImVec2(160, 0)))),
						group(vbox(4.0f,
							separatorText("延迟"),
							textColored("-3.1%", ImVec4(0.4f, 0.9f, 0.6f, 1.0f)),
							progressBar(history.back(), ImVec2(160, 0))))),
					hbox(8.0f,
						group(vbox(4.0f,
							separatorText("错误率"),
							textColored("+0.2%", ImVec4(0.9f, 0.4f, 0.4f, 1.0f)),
							plotLines("err", history, ImVec2(160, 48)))),
						group(vbox(4.0f,
							separatorText("连接数"),
							textColored("128", ImVec4(0.6f, 0.8f, 1.0f, 1.0f)),
							labelText("上限", "256")))))),

				when(sel == 34, vbox(6.0f,
					DemoHead{"主从布局", "左列表右详情：MasterList（本地 ADL 单选列表）选中项驱动右侧详情"},
					hbox(0.0f,
						subwindow("master", ImVec2(200, 0), true,
							MasterList{kDetail, detailSel}),
						subwindow("detail", ImVec2(0, 0), true,
							when(detailSel == 0, vbox(4.0f,
								separatorText("项目甲"),
								labelText("状态", "进行中"),
								slider("进度", volume, 0.0f, 1.0f))),
							when(detailSel == 1, vbox(4.0f,
								separatorText("项目乙"),
								labelText("状态", "已挂起"),
								checkbox("启用", mute))),
							when(detailSel >= 2, vbox(4.0f,
								separatorText("其它项目"),
								labelText("选中下标", std::to_string(detailSel)),
								colorEdit3("标记色", col3))))))),

				// ==== 多窗口联动（35）====
				when(sel == 35, vbox(6.0f,
					DemoHead{"多窗口联动", "第二个顶层窗口（检查器）与画廊共享全部外置状态"},
					labelText("画廊帧", std::to_string(frames)),
					labelText("检查器帧", std::to_string(insFrames)),
					button(insVisible ? "隐藏检查器窗口" : "显示检查器窗口", [&] {
						if(auto* h = inspector.getHandle()) {
							if(glfwGetWindowAttrib(h, GLFW_VISIBLE)) glfwHideWindow(h);
							else glfwShowWindow(h);
						}
					}),
					textWrapped("两个顶层窗口各有独立 ImGui 上下文与帧循环，绑定同一批外部变量即可实时联动：拖动任意窗口的滑块或颜色，另一窗口同帧跟随。				检查器点 X 仅隐藏（关闭回调拦截），画廊关闭时自动跟随退出。"))),

				// ==== 文件（36）====
				when(sel == 36, vbox(6.0f,
					DemoHead{"打开本地文件", "FileDialog：Win32 IFileOpenDialog 跑在 worker 线程，渲染循环不冻结；主循环每帧 poll 取结果"},
					hbox(8.0f,
						button("打开文件…", openFileDialog),
						when(fileDlg.busy(), textColored("对话框打开中…（主窗口仍在渲染）", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)))),
					labelText("路径", filePath.empty() ? "（未选择）" : filePath.c_str()),
					labelText("大小", std::to_string(fileSize) + " 字节"),
					when(fileMissing, textColored("文件不可读", ImVec4(0.9f, 0.4f, 0.4f, 1.0f))),
					when(!filePath.empty() && fileBinary,
						textColored("二进制文件：仅显示大小", ImVec4(0.6f, 0.8f, 1.0f, 1.0f))),
					when(!filePath.empty() && !fileBinary,
						subwindow("文本预览（前 64KB）", ImVec2(0, 0), true,
							FilePreview{filePreview}))))
				))));

		// —— 弹窗（树外常驻）——
		draw(popup("画廊弹窗", popupOpen,
			label("普通弹窗内容"),
			ClosePopupButton{"关闭"}));
		draw(popupModal("画廊模态", modalOpen, 0,
			label("模态弹窗：阻塞宿主交互"),
			ClosePopupButton{"关闭"}));
		draw(popupModal("关于", aboutOpen, 0,
			label("mkui 控件画廊\nImGui + GLFW + OpenGL 封装\nOOP 控件树 + 函数式控件库"),
			ClosePopupButton{"确定"}));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}