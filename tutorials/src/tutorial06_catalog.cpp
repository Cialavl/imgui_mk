// 教程 06：控件目录巡礼。文档：lessons/lesson-06-widget-catalog.md
// 输入/显示/容器/弹窗 + 综合展示；全部状态外置，每帧刷新动态值。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace {

// 关弹窗按钮（wg 控件）：必须在弹窗作用域内调用 CloseCurrentPopup
struct CloseButtonImpl {
	const char* name = "关闭";
	void renderImpl(Mk::wg::Node&) {
		if(ImGui::Button(name)) ImGui::CloseCurrentPopup();
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size&) {
		ImVec2 s = ImGui::CalcTextSize(name);
		return Mk::wg::Size{s.x + 20.0f, s.y + 12.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using CloseButton = Mk::wg::Control<CloseButtonImpl, Mk::wg::DefaultStyle>;

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程06：控件目录巡礼", 1280, 900);

	int frames = 0;
	// 输入类状态
	std::array<char, 64> text{};
	int iVal = 42; float fVal = 3.14f; std::array<float, 2> fv{1, 2};
	float drag = 0.5f, slideV = 0.4f;
	int radio = 1, comboSel = 0, listSel = 0;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};
	static constexpr const char* kCols[] = {"项", "值"};
	// 显示类状态
	float progress = 0.3f; bool rowSel = false;
	std::vector<float> history;
	// 表格数据（长期存活，textTable 只读绑定）
	std::vector<std::vector<std::string>> rows{{"甲", "0"}, {"乙", "0"}};
	// 弹窗与颜色
	bool aboutOpen = false, modalOpen = false;
	std::array<float, 3> rgb{0.9f, 0.3f, 0.2f};
	int flags = 0b101;

	// —— 菜单栏（窗口顶部，树外常驻）——
	Mk::wg::MenuItem miAbout;
	miAbout.self.name = "关于";
	miAbout.self.onClick = [&] { aboutOpen = true; };
	Mk::wg::Menu<> menuFile;
	menuFile.self.name = "文件";
	menuFile.node.add(miAbout.node);
	Mk::wg::MenuBar<> menuTree;
	menuTree.node.add(menuFile.node);

	// —— 主体树（Layout V + 命名控件）——
	Mk::wg::SeparatorText sec1, sec2, sec3, sec4;
	sec1.self.name = "输入：一切结果绑定外部状态";
	sec2.self.name = "显示：只读视图（逻辑全在外部）";
	sec3.self.name = "容器：TabBar / 表格";
	sec4.self.name = "弹窗与颜色（标志触发式）";

	Mk::wg::InputText<64> inputText;
	inputText.self.name = "文本";
	inputText.self.setName("可编辑");
	Mk::wg::InputInt inI;
	inI.self.name = "整数";  inI.self.value = &iVal;
	Mk::wg::InputFloat inF;
	inF.self.name = "浮点";  inF.self.value = &fVal;
	Mk::wg::Layout numRow;
	numRow.self.mode = Mk::wg::LayoutMode::H;
	numRow.node.add(inI.node);
	numRow.node.add(inF.node);
	Mk::wg::InputFloat2 inF2;
	inF2.self.name = "vec2";  inF2.self.value = &fv;
	Mk::wg::DragF dragF;
	dragF.self.name = "拖拽";  dragF.self.value = &drag;  dragF.self.speed = 0.01f;
	Mk::wg::SliderF slideF;
	slideF.self.name = "滑块";  slideF.self.value = &slideV;
	Mk::wg::RadioGroup radioG;
	radioG.self.items = kItems;  radioG.self.count = 3;  radioG.self.current = &radio;
	Mk::wg::Combo combo;
	combo.self.name = "下拉";  combo.self.current = &comboSel;
	combo.self.items = kItems;  combo.self.count = 3;
	Mk::wg::ListBox listB;
	listB.self.name = "列表";  listB.self.current = &listSel;
	listB.self.items = kItems;  listB.self.count = 3;
	Mk::wg::LabelText sumLt;
	sumLt.self.name = "综合";

	Mk::wg::ProgressBar pb;
	pb.self.fraction = &progress;
	Mk::wg::Selectable sel;
	sel.self.name = "可选中行";
	sel.self.selected = &rowSel;
	Mk::wg::Layout dispRow;
	dispRow.self.mode = Mk::wg::LayoutMode::H;
	dispRow.node.add(pb.node);
	dispRow.node.add(sel.node);
	Mk::wg::PlotLines plot;
	plot.self.name = "正弦历史（外部 vector）";
	plot.self.values = &history;

	Mk::wg::TextTable tt;
	tt.node.idname = "t6";
	tt.self.headers = kCols;  tt.self.headerCount = 2;  tt.self.rows = &rows;
	Mk::wg::FnTable ft;
	ft.node.idname = "f6";
	ft.self.headers = kCols;  ft.self.headerCount = 2;  ft.self.rowCount = 3;
	ft.self.cell = [](int r, int c) { return std::to_string(r) + "," + std::to_string(c); };
	Mk::wg::Tab<> tab1, tab2;
	tab1.node.idname = "数据表";
	tab1.node.add(tt.node);
	tab2.node.idname = "函数表";
	tab2.node.add(ft.node);
	Mk::wg::TabBar<> tabs;
	tabs.node.idname = "tabs6";
	tabs.node.add(tab1.node);
	tabs.node.add(tab2.node);

	Mk::wg::Button btnPop, btnModal;
	btnPop.self.setName("弹窗");
	btnPop.self.onClick = [&] { aboutOpen = true; };
	btnModal.self.setName("模态");
	btnModal.self.onClick = [&] { modalOpen = true; };
	Mk::wg::Layout popRow;
	popRow.self.mode = Mk::wg::LayoutMode::H;
	popRow.node.add(btnPop.node);
	popRow.node.add(btnModal.node);
	Mk::wg::ColorEdit3 ce3;
	ce3.self.name = "RGB";  ce3.self.color = &rgb;
	Mk::wg::CheckboxFlags cf1, cf2, cf3;
	cf1.self.name = "位A";  cf1.self.flags = &flags;  cf1.self.mask = 1;
	cf2.self.name = "位B";  cf2.self.flags = &flags;  cf2.self.mask = 2;
	cf3.self.name = "位C";  cf3.self.flags = &flags;  cf3.self.mask = 4;
	Mk::wg::Layout flagRow;
	flagRow.self.mode = Mk::wg::LayoutMode::H;
	flagRow.node.add(cf1.node);
	flagRow.node.add(cf2.node);
	flagRow.node.add(cf3.node);

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(sec1.node);
	root.node.add(inputText.node);
	root.node.add(numRow.node);
	root.node.add(inF2.node);
	root.node.add(dragF.node);
	root.node.add(slideF.node);
	root.node.add(radioG.node);
	root.node.add(combo.node);
	root.node.add(listB.node);
	root.node.add(sumLt.node);
	root.node.add(sec2.node);
	root.node.add(dispRow.node);
	root.node.add(plot.node);
	root.node.add(sec3.node);
	root.node.add(tabs.node);
	root.node.add(sec4.node);
	root.node.add(popRow.node);
	root.node.add(ce3.node);
	root.node.add(flagRow.node);

	// 弹窗（树外常驻）
	Mk::wg::Label aboutLbl, modalLbl;
	aboutLbl.self.setName("教程06 的关于弹窗\n（标志触发式：open 置位当帧 OpenPopup）");
	modalLbl.self.setName("模态弹窗：阻挡宿主交互，必须显式关闭");
	CloseButton cb1, cb2;
	Mk::wg::Popup<> aboutPop;
	aboutPop.node.idname = "about6";
	aboutPop.self.open = &aboutOpen;
	aboutPop.node.add(aboutLbl.node);
	aboutPop.node.add(cb1.node);
	Mk::wg::PopupModal<> modalPop;
	modalPop.node.idname = "modal6";
	modalPop.self.open = &modalOpen;
	modalPop.node.add(modalLbl.node);
	modalPop.node.add(cb2.node);

	mainWindow.setUI([&] {
		++frames;
		// 每帧动态数据
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 200) history.erase(history.begin());
		rows[0][1] = std::to_string(frames);
		progress = 0.5f + 0.5f * std::sin(frames * 0.02f);
		sumLt.self.value = std::to_string(iVal) + " | radio=" + std::to_string(radio)
			+ " | combo=" + kItems[comboSel];

		menuTree.layout(Mk::wg::Rect{0, 0, 1280, 24});
		root.layout(Mk::wg::Rect{20, 30, 1240, 860});
		aboutPop.layout(Mk::wg::Rect{0, 0, 300, 120});
		modalPop.layout(Mk::wg::Rect{0, 0, 300, 120});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
