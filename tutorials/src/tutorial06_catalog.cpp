// 教程 06：完整控件集巡礼。文档：lessons/lesson-06-widget-catalog.md
// 输入/显示/容器/弹窗四区 + 综合面板；全部状态外置，回显实时。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace {

// §6.4 关弹窗按钮（gallery 同款）：必须在弹窗作用域内调用
struct CloseButton { const char* label; };
void draw(const CloseButton& b) {
	if(ImGui::Button(b.label)) ImGui::CloseCurrentPopup();
}

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程06：控件集巡礼", 1280, 900);

	int frames = 0;
	// 输入区状态
	std::array<char, 64> text{};
	int iVal = 42; float fVal = 3.14f; std::array<float, 2> fv{1, 2};
	float drag = 0.5f, slideV = 0.4f;   // 注意：不要命名成 slider——会遮蔽工厂函数
	int radio = 1, comboSel = 0, listSel = 0;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};
	static constexpr const char* kCols[] = {"名", "值"};   // §6.6-1 items 须长期存活（不能用临时 span）
	// 显示区状态
	float progress = 0.3f; bool rowSel = false;
	std::vector<float> history;
	// 容器区数据（长期存活，textTable 绑定引用）
	std::vector<std::vector<std::string>> rows{{"甲", "0"}, {"乙", "0"}};
	// 弹窗区
	bool aboutOpen = false, modalOpen = false;
	std::array<float, 3> rgb{0.9f, 0.3f, 0.2f};
	int flags = 0b101;

	mainWindow.setUI([&] {
		++frames;
		// 逻辑侧驱动显示区
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 200) history.erase(history.begin());
		rows[0][1] = std::to_string(frames);    // §6.3 表格内容每帧跟随
		progress = 0.5f + 0.5f * std::sin(frames * 0.02f);

		using namespace Mk::fui;

		// §6.3 菜单栏放树最开头（画完把宿主光标下移，防重叠）
		draw(menuBar(menu("文件", menuItem("关于", [&] { aboutOpen = true; }))));

		draw(vbox(8.0f,
			separatorText("§6.1 输入：一切皆引用绑定（改任何输入看回显）"),
			inputText("文本", text),
			hbox(8.0f, inputInt("整数", iVal), inputFloat("浮点", fVal)),
			inputFloat2("vec2", fv),
			dragF("拖拽", drag, 0.01f, 0.0f, 10.0f),
			slider("滑块", slideV, 0.0f, 1.0f),
			radioGroup(kItems, radio),
			combo("下拉", comboSel, kItems),
			listBox("列表", listSel, kItems),
			labelText("回显", std::string(text.data()) + " | i=" + std::to_string(iVal)
				+ " | radio=" + std::to_string(radio) + " | combo=" + kItems[comboSel]),

			separatorText("§6.2 显示：只读视图（数据全在逻辑侧）"),
			hbox(8.0f,
				progressBar(progress, em2(200, 0)),
				selectable("可选中行", rowSel)),
			plotLines("正弦历史（外置 vector）", history, ImVec2(0, 80)),

			separatorText("§6.3 容器：tabBar / textTable / fnTable"),
			tabBar("标签栏", 0,
				tab("数据表", textTable("t", kCols, rows)),
				tab("函数表", fnTable("f", kCols, 3,
					[](int r, int c) { return std::to_string(r) + "," + std::to_string(c); }))),

			separatorText("§6.4 弹窗与颜色：标志触发式"),
			hbox(8.0f,
				button("关于", [&] { aboutOpen = true; }),
				button("模态", [&] { modalOpen = true; })),
			colorEdit3("RGB", rgb),
			hbox(8.0f,
				checkboxFlags("位A", flags, 1),
				checkboxFlags("位B", flags, 2),
				checkboxFlags("位C", flags, 4))));

		// 弹窗（树外常驻）
		draw(popup("about", aboutOpen,
			label("教程06 的关于弹窗\n内容每帧渲染，CloseButton 关闭"),
			CloseButton{"关闭"}));
		draw(popupModal("modal", modalOpen, 0,
			label("模态弹窗：阻塞宿主交互（会关闭普通弹窗）"),
			CloseButton{"关闭"}));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}