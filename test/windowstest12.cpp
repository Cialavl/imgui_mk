#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <bitset>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

// —— 画廊本地扩展控件（ADL：无需继承，提供 draw 重载即可被容器接受）——

// 关弹窗按钮：draw 内直接调 ImGui::CloseCurrentPopup（必须在弹窗作用域内）
struct ClosePopupButton { std::string label; };
inline void draw(const ClosePopupButton& b) {
	if(ImGui::Button(b.label.c_str())) ImGui::CloseCurrentPopup();
}

// 帧计数标签：每帧刷新（画廊动画源之一）
struct FrameLabel { int& frames; };
inline void draw(const FrameLabel& f) {
	ImGui::Text("帧 %d", f.frames);
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

} // namespace

// mkui 控件画廊：单窗口六分区展示双库全量控件——
//   基础    Label/Button/Slider/CheckBox + disabled/when/withId/itemWidth 修饰器
//   输入    文本/数值/vec2 输入、拖拽、滑块、单选组、下拉、列表
//   显示    彩色/换行/圆点/标签值文本、进度条、分隔、可选行、图片、折线、树节点、tooltip/colored/indent
//   容器    嵌套标签栏、双形态表格、折叠、子窗口（菜单栏与浮动面板常驻展示）
//   弹窗    普通弹窗/模态（含菜单“关于”触发）、颜色编辑/拾取、位标志
//   桥接    dyn()：函数式树内嵌 OOP 动态子树；asWidget()：OOP 树内嵌每帧函数式视图
// 交互式：随便把玩，点菜单“文件/退出”或直接关窗结束。
int main() {
	Mk::MainWindow<> mainWindow("mkui 控件画廊", 1440, 880);

	// ============ 外置状态（函数式：视图每帧重建，状态全在这里）============
	int frames = 0;
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

	bool sel = false;
	bool popupOpen = false, modalOpen = false, aboutOpen = false;
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::array<float, 3> pick3{0.4f, 0.6f, 0.8f};
	std::array<float, 4> pick4{0.8f, 0.6f, 0.4f, 1.0f};
	int bitFlags = 0b101;

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

	GLFWwindow* handle = mainWindow.getHandle();

	mainWindow.setUI([&] {
		// 动画驱动
		++frames;
		volume = 0.5f + 0.5f * std::sin(frames * 0.02f);
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 300) history.erase(history.begin());

		using namespace Mk::fui;

		// —— 常驻：菜单栏 + 浮动面板（不受标签页切换影响）——
		draw(menuBar(
			menu("文件",
				menuItem("退出", [handle] { glfwSetWindowShouldClose(handle, true); }, "Alt+F4")),
			menu("帮助",
				menuItem("关于画廊", [&] { aboutOpen = true; }))));

		draw(panel("浮动面板（可拖动）", 0,
			label("独立于标签页的常驻面板"),
			slider("面板滑块", volume, 0.0f, 1.0f)));

		// —— 六分区标签页 ——
		draw(tabBar("画廊主栏", 0,

			tab("基础", vbox(6.0f,
				label("原子控件 + 修饰器（每帧重建的纯值视图）"),
				hbox(8.0f,
					button("点我计数", [&] { ++clicks; }),
					labelText("点击次数", std::to_string(clicks))),
				hbox(8.0f,
					slider("音量（正弦驱动）", volume, 0.0f, 1.0f),
					checkbox("静音", mute)),
				hbox(8.0f,
					withId("a", button("同 ID 按钮", [&] { ++sameClicks; })),
					withId("b", button("同 ID 按钮", [&] { ++sameClicks; })),
					labelText("两边独立可点", std::to_string(sameClicks))),
				hbox(8.0f,
					disabled(checkbox("静音时禁用", mute)),
					when(!mute, label("未静音时才显示（when 条件渲染）"))),
				itemWidth(220.0f, dragF("定宽拖拽（itemWidth 修饰）", drag, 0.01f, 0.0f, 10.0f)),
				FrameLabel{frames})),

			tab("输入", vbox(6.0f,
				label("数据绑定输入：交互直接写回外部变量"),
				inputText("文本", textBuf),
				labelText("缓冲内容", textBuf.data()),
				hbox(8.0f,
					inputInt("整数", iVal),
					inputFloat("浮点", fVal),
					inputDouble("双精度", dVal)),
				hbox(8.0f,
					inputInt2("ivec2", iv),
					inputFloat2("vec2", fv)),
				hbox(8.0f,
					dragF("拖拽浮点", drag, 0.01f, 0.0f, 10.0f),
					dragI("拖拽整数", dragInt, 1.0f, 0, 1000)),
				hbox(8.0f,
					sliderInt("整数滑块", slideI, 0, 100),
					sliderAngle("角度", angle)),
				radioButton("独立单选", radioBool),
				radioGroup(kItems, radio),
				combo("下拉选择", comboSel, kItems),
				listBox("列表选择", listSel, kItems))),

			tab("显示", vbox(6.0f,
				textColored("彩色文本（金色）", ImVec4(1.0f, 0.8f, 0.2f, 1.0f)),
				textWrapped("自动换行长文本：函数式控件库中视图是每帧构造的临时值，状态全部外置于被绑定的变量，因此不存在悬垂与生命周期负担。"),
				bulletText("圆点列表项一"),
				bulletText("圆点列表项二"),
				labelText("外置标签", "外置值"),
				progressBar(volume, ImVec2(-FLT_MIN, 0), "跟随音量"),
				separator(),
				separatorText("分隔标题"),
				selectable("可选中行（选中态外置）", sel),
				hbox(8.0f,
					image(tex, ImVec2(64, 64)),
					imageButton("画廊图钮", tex, ImVec2(64, 64), [&] { ++imgBtnClicks; }),
					labelText("图钮点击", std::to_string(imgBtnClicks))),
				plotLines("正弦历史（外部 vector）", history, ImVec2(0, 90)),
				tooltip(button("悬停看提示", [] {}), "tooltip 修饰器"),
				colored(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.6f, 1.0f),
					label("colored 修饰的文本")),
				indent(30.0f, label("indent 修饰的文本")),
				treeNode("树节点（默认展开）", ImGuiTreeNodeFlags_DefaultOpen,
					treeNode("嵌套树节点", 0, label("最深层的子视图"))))),

			tab("容器", vbox(6.0f,
				label("结构容器：标签页 / 表格 / 折叠 / 子窗口"),
				tabBar("嵌套标签栏", 0,
					tab("嵌套页一", label("嵌套标签栏的内容一")),
					tab("嵌套页二", label("嵌套标签栏的内容二"))),
				hbox(8.0f,
					textTable("文本表", kHeaders, rows),
					fnTable("函数表", kHeaders, 4, [](int r, int c) {
						return "r" + std::to_string(r) + "c" + std::to_string(c);
					})),
				collapsing("折叠容器（默认展开）", ImGuiTreeNodeFlags_DefaultOpen,
					label("展开时才渲染的内容"),
					subwindow("折叠内子窗口", ImVec2(0, 80), true,
						label("子窗口（BeginChild）可独立滚动"))),
				subwindow("独立子窗口", ImVec2(0, 100), true,
					hbox(8.0f, label("子窗口内的水平行"), button("按钮", [] {}))))),

			tab("弹窗与颜色", vbox(6.0f,
				hbox(8.0f,
					button("打开普通弹窗", [&] { popupOpen = true; }),
					button("打开模态弹窗", [&] { modalOpen = true; })),
				colorEdit3("RGB 编辑", col3),
				colorEdit4("RGBA 编辑", col4),
				collapsing("颜色拾取器（点击展开）", 0,
					hbox(8.0f,
						colorPicker3("RGB 拾取", pick3),
						colorPicker4("RGBA 拾取", pick4))),
				hbox(8.0f,
					checkboxFlags("位 A(1)", bitFlags, 1),
					checkboxFlags("位 B(2)", bitFlags, 2),
					checkboxFlags("位 C(4)", bitFlags, 4)),
				labelText("位标志值", "0b" + std::bitset<3>(bitFlags).to_string()))),

			tab("双向桥", vbox(6.0f,
				label("dyn()：函数式树内嵌 OOP 动态子树（墓碑增删安全）"),
				hbox(8.0f,
					button("添加 OOP 项", [&] { dynList.addItem(); }),
					button("删除末项", [&] { dynList.removeLast(); }),
					labelText("OOP 项数", std::to_string(dynList.count()))),
				subwindow("OOP 动态子树", ImVec2(0, 120), true,
					dyn(dynList)),
				separatorText("反向桥"),
				subwindow("OOP 容器（内嵌函数式视图）", ImVec2(0, 80), true,
					dyn(oopBox))))));

		// —— 弹窗（树外常驻：任意标签页下都可达）——
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