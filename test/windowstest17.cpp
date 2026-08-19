#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_style.h>
#include <cmath>
#include <iostream>
#include <array>
#include <memory>
#include <vector>

// test17：wg 全控件 soak——输入/显示/容器/分格/弹窗/颜色/装饰件逐帧渲染 120 帧，
// 期间触发运行时增删、菜单、弹窗、主题作用域。退出码 0 = 全程无崩溃通过。
int main() {
	Mk::MainWindow<> win("wg 全控件 soak", 1280, 900);

	// —— 共享状态 ——
	int frames = 0, clicks = 0, menuClicks = 0;
	float volume = 0.5f;
	bool mute = false, radioB = false, selRow = false;
	int iVal = 42, comboSel = 0, radioSel = 1, listSel = 0, slideI = 50, dragIv = 100;
	float fVal = 3.14f, dragF = 0.5f, angle = 0.8f;
	double dVal = 2.718;
	std::array<int, 2> iv{1, 2};
	std::array<float, 2> fv{1.5f, 2.5f};
	int bitFlags = 0b101;
	bool popupOpen = false, modalOpen = false;
	std::array<float, 3> col3{0.9f, 0.3f, 0.2f};
	std::array<float, 4> col4{0.2f, 0.6f, 0.9f, 1.0f};
	std::vector<float> history;
	static constexpr const char* kItems[] = {"选项甲", "选项乙", "选项丙"};
	static constexpr const char* kCols[] = {"名称", "数值"};
	std::vector<std::vector<std::string>> rows{{"甲", "1"}, {"乙", "2"}};
	std::unique_ptr<Mk::wg::Label> dynLeaf;

	// —— 输入类面板 ——
	Mk::wg::InputText<> it1;
	it1.self.name = "文本";
	Mk::wg::InputInt ii;
	ii.self.name = "整数";  ii.self.value = &iVal;
	Mk::wg::InputFloat fi;
	fi.self.name = "浮点";  fi.self.value = &fVal;
	Mk::wg::InputDouble di;
	di.self.name = "双精度";  di.self.value = &dVal;
	Mk::wg::InputInt2 i2v;
	i2v.self.name = "ivec2";  i2v.self.value = &iv;
	Mk::wg::InputFloat2 f2v;
	f2v.self.name = "vec2";  f2v.self.value = &fv;
	Mk::wg::DragF df;
	df.self.name = "拖拽f";  df.self.value = &dragF;
	Mk::wg::DragI dint;
	dint.self.name = "拖拽i";  dint.self.value = &dragIv;
	Mk::wg::SliderF sf;
	sf.self.name = "滑块";  sf.self.value = &volume;
	Mk::wg::SliderInt si;
	si.self.name = "滑块i";  si.self.value = &slideI;
	Mk::wg::SliderAngle sa;
	sa.self.name = "角度";  sa.self.valueRad = &angle;
	Mk::wg::RadioButton rb;
	rb.self.name = "单选";  rb.self.active = &radioB;
	Mk::wg::RadioGroup rg;
	rg.self.items = kItems;  rg.self.count = 3;  rg.self.current = &radioSel;
	Mk::wg::Combo co;
	co.self.name = "下拉";  co.self.current = &comboSel;
	co.self.items = kItems;  co.self.count = 3;
	Mk::wg::ListBox lb;
	lb.self.name = "列表";  lb.self.current = &listSel;
	lb.self.items = kItems;  lb.self.count = 3;
	Mk::wg::CheckBox cb;
	cb.self.name = "开关";  cb.self.checked = &mute;
	Mk::wg::VLayout inputs;
	
	for(Mk::wg::Node* nd : {&it1.node, &ii.node, &fi.node, &di.node, &i2v.node,
			&f2v.node, &df.node, &dint.node, &sf.node, &si.node, &sa.node,
			&rb.node, &rg.node, &co.node, &lb.node, &cb.node}) {
		inputs.addWidget(*nd);
	}

	// —— 显示类面板 ——
	Mk::wg::Label l1;
	l1.self.setName("显示类原子");
	Mk::wg::TextColored tc;
	tc.self.name = "彩色";
	tc.self.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
	Mk::wg::TextWrapped tw;
	tw.self.name = "自动换行的长文本，展示 TextWrapped 行为。";
	Mk::wg::BulletText bt1;
	bt1.self.name = "圆点";
	Mk::wg::LabelText lt1;
	lt1.self.name = "属性";  lt1.self.value = "值";
	Mk::wg::EllipsisLabel el;
	el.self.name = "超宽省略号标签会在这个面板宽度下截断显示…";
	Mk::wg::Separator sep;
	Mk::wg::SeparatorText st1;
	st1.self.name = "分隔标题";
	Mk::wg::Selectable sl;
	sl.self.name = "可选中行";  sl.self.selected = &selRow;
	Mk::wg::ProgressBar pb1;
	pb1.self.fraction = &volume;
	Mk::wg::PlotLines pl1;
	pl1.self.name = "曲线";  pl1.self.values = &history;
	Mk::wg::CheckboxFlags cfa, cfb, cfc;
	cfa.self.name = "位A";  cfa.self.flags = &bitFlags;  cfa.self.mask = 1;
	cfb.self.name = "位B";  cfb.self.flags = &bitFlags;  cfb.self.mask = 2;
	cfc.self.name = "位C";  cfc.self.flags = &bitFlags;  cfc.self.mask = 4;
	Mk::wg::VLayout display;
	
	for(Mk::wg::Node* nd : {&l1.node, &tc.node, &tw.node, &bt1.node, &lt1.node,
			&el.node, &sep.node, &st1.node, &sl.node, &pb1.node, &pl1.node,
			&cfa.node, &cfb.node, &cfc.node}) {
		display.addWidget(*nd);
	}

	// —— 容器类面板 ——
	Mk::wg::Collapsing collapsing;
	collapsing.self.name = "折叠容器（默认展开）";
	collapsing.self.flags = ImGuiTreeNodeFlags_DefaultOpen;
	Mk::wg::Label collLabel;
	collLabel.self.setName("折叠内容");
	collapsing.node.add(collLabel.node);

	Mk::wg::Tree tree;
	tree.self.name = "树节点（默认展开）";
	tree.self.defaultOpen = true;
	Mk::wg::Label leaf1, leaf2;
	leaf1.self.setName("叶子一");
	leaf2.self.setName("叶子二");
	tree.node.add(leaf1.node);
	tree.node.add(leaf2.node);

	Mk::wg::SubWindow<> subwin;
	subwin.node.idname = "sub";
	subwin.self.height = 60.0f;
	Mk::wg::Label subLabel;
	subLabel.self.setName("子窗口内容");
	subwin.node.add(subLabel.node);

	Mk::wg::Label tab1Lbl, tab2Lbl;
	tab1Lbl.self.setName("第一页");
	tab2Lbl.self.setName("第二页");
	Mk::wg::Tab<> tab1, tab2;
	tab1.node.idname = "页一";
	tab1.node.add(tab1Lbl.node);
	tab2.node.idname = "页二";
	tab2.node.add(tab2Lbl.node);
	Mk::wg::TabBar<> tabs;
	tabs.node.idname = "tabs";
	tabs.node.add(tab1.node);
	tabs.node.add(tab2.node);

	Mk::wg::Label grpLbl;
	grpLbl.self.setName("Group 打包");
	Mk::wg::Button grpBtn;
	grpBtn.self.setName("组内按钮");
	grpBtn.self.onClick = [&] { ++clicks; };
	Mk::wg::OverlayLayout group;
	
	group.addWidget(grpLbl.node);
	group.addWidget(grpBtn.node);

	// —— 分格布局 ——
	Mk::wg::Label h1, h2, h3;
	h1.self.setName("fixed 160");
	h2.self.setName("stretch 1");
	h3.self.setName("stretch 2");
	h1.setFixedWidth(160.0f);


	Mk::wg::SplitHLayout hsplit;
	
	hsplit.addWidget(h1.node);
	hsplit.addWidget(h2.node, 1.0f);
	hsplit.addWidget(h3.node, 2.0f);

	Mk::wg::Label v1, v2;
	v1.self.setName("fixed 40");
	v2.self.setName("stretch");
	v1.setFixedHeight(40.0f);

	Mk::wg::SplitVLayout vsplit;
	
	vsplit.addWidget(v1.node);
	vsplit.addWidget(v2.node, 1.0f);

	// —— 弹窗与颜色 ——
	Mk::wg::Button bPop, bModal;
	bPop.self.setName("打开弹窗");
	bPop.self.onClick = [&] { popupOpen = true; };
	bModal.self.setName("打开模态");
	bModal.self.onClick = [&] { modalOpen = true; };
	Mk::wg::ColorEdit3 ce3;
	ce3.self.name = "RGB";  ce3.self.color = &col3;
	Mk::wg::ColorEdit4 ce4;
	ce4.self.name = "RGBA";  ce4.self.color = &col4;
	Mk::wg::ColorPicker3 cp3;
	cp3.self.name = "拾取";  cp3.self.color = &col3;
	Mk::wg::ColorPicker4 cp4;
	cp4.self.name = "拾取4";  cp4.self.color = &col4;
	Mk::wg::VLayout popCol;
	
	for(Mk::wg::Node* nd : {&bPop.node, &bModal.node, &ce3.node, &ce4.node, &cp3.node, &cp4.node}) {
		popCol.addWidget(*nd);
	}

	Mk::wg::Popup<> popup;
	popup.node.idname = "p17";
	popup.self.open = &popupOpen;
	Mk::wg::Label popLabel;
	popLabel.self.setName("wg 弹窗内容");
	popup.node.add(popLabel.node);
	Mk::wg::PopupModal<> modal;
	modal.node.idname = "m17";
	modal.self.open = &modalOpen;
	Mk::wg::Label modLabel;
	modLabel.self.setName("wg 模态弹窗");
	modal.node.add(modLabel.node);

	// —— 装饰件 ——
	Mk::wg::Label tipTarget, colTarget, indTarget, whenLabel;
	tipTarget.self.setName("悬停看提示");
	colTarget.self.setName("colored 包裹");
	indTarget.self.setName("缩进文本");
	whenLabel.self.setName("when 隐藏时不可见");
	Mk::wg::Tooltip tip;
	tip.self.name = "tooltip";
	tip.node.add(tipTarget.node);
	Mk::wg::Colored colored;
	colored.self.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
	colored.node.add(colTarget.node);
	Mk::wg::Indent indent;
	indent.self.amount = 24.0f;
	indent.node.add(indTarget.node);
	Mk::wg::DragF iwDrag;
	iwDrag.self.name = "定宽拖拽";
	iwDrag.self.value = &dragF;
	Mk::wg::ItemWidth<Mk::wg::DragF> iw;
	iw.self.width = 180.0f;
	Mk::wg::When<Mk::wg::Label> when;
	when.node.add(whenLabel.node);
	Mk::wg::Button styledBtn;
	styledBtn.self.setName("light 作用域按钮");
	Mk::wg::Styled<Mk::wg::Button> styled;
	styled.self.theme = Mk::wg::Theme::light();
	styled.node.add(styledBtn.node);
	Mk::wg::Button disabledBtn;
	disabledBtn.self.setName("disabled 按钮");
	disabledBtn.node.enabled = false;
	Mk::wg::VLayout deco;
	
	for(Mk::wg::Node* nd : {&tip.node, &colored.node, &indent.node, &iw.node,
			&when.node, &styled.node, &disabledBtn.node}) {
		deco.addWidget(*nd);
	}

	// —— 菜单栏 ——
	Mk::wg::MenuItem mi1, mi2;
	mi1.self.name = "打开";
	mi1.self.onClick = [&] { ++menuClicks; };
	mi2.self.name = "退出";
	mi2.self.onClick = [&] { ++menuClicks; };
	Mk::wg::Menu<> fileMenu;
	fileMenu.self.name = "文件";
	fileMenu.node.add(mi1.node);
	fileMenu.node.add(mi2.node);
	Mk::wg::MenuBar<> menuBar;
	menuBar.node.add(fileMenu.node);

	// —— 纹理 ——
	static unsigned char px[8 * 8 * 4];
	for(int y = 0; y < 8; ++y) for(int x = 0; x < 8; ++x) {
		unsigned char v = ((x + y) % 2) ? 200 : 60;
		px[(y * 8 + x) * 4 + 0] = v; px[(y * 8 + x) * 4 + 1] = v;
		px[(y * 8 + x) * 4 + 2] = v; px[(y * 8 + x) * 4 + 3] = 255;
	}
	ImTextureID tex = Mk::wg::loadTextureRGBA(px, 8, 8);
	Mk::wg::ImageButton imgBtn;
	imgBtn.node.idname = "img17";
	imgBtn.self.tex = tex;
	imgBtn.self.size = ImVec2(48, 48);
	imgBtn.self.onClick = [&] { ++clicks; };

	// —— 布局批次F：Wrap / Grid / 窗口嵌窗口 ——
	Mk::wg::Label w1, w2, w3, w4;
	w1.self.setName("流式甲");
	w2.self.setName("流式乙（较长）");
	w3.self.setName("流式丙");
	w4.self.setName("流式丁");
	Mk::wg::WrapLayout wrapRow;
	
	for(Mk::wg::Node* nd : {&w1.node, &w2.node, &w3.node, &w4.node}) {
		wrapRow.addWidget(*nd);
	}
	Mk::wg::Label g1, g2, g3, g4, g5, g6;
	g1.self.setName("格 0-0"); g2.self.setName("格 0-1"); g3.self.setName("格 0-2");
	g4.self.setName("格 1-0"); g5.self.setName("格 1-1"); g6.self.setName("格 1-2");
	Mk::wg::GridLayout grid;
	
	grid.setCols(3);
	for(Mk::wg::Node* nd : {&g1.node, &g2.node, &g3.node, &g4.node, &g5.node, &g6.node}) {
		grid.addWidget(*nd);
	}
	Mk::wg::Label o1, i1, v1a, v1b;
	o1.self.setName("外层子窗口");
	i1.self.setName("内层子窗口");
	v1a.self.setName("内层 VBox 一");
	v1b.self.setName("内层 VBox 二");
	Mk::wg::VLayout innerV;
	
	innerV.addWidget(v1a.node);
	innerV.addWidget(v1b.node);
	Mk::wg::SubWindow<> innerWin;
	innerWin.node.idname = "nestInner";
	innerWin.self.height = 70.0f;
	innerWin.node.add(i1.node);
	innerWin.node.add(innerV.node);
	Mk::wg::SubWindow<> nestWin;
	nestWin.node.idname = "nestOuter";
	nestWin.self.height = 120.0f;
	nestWin.node.add(o1.node);
	nestWin.node.add(innerWin.node);

	// —— 动态叶子（帧 40 add / 帧 80 remove）——
	dynLeaf = std::make_unique<Mk::wg::Label>();
	dynLeaf->self.setName("动态叶子（墓碑增删）");

	win.setUI([&] {
		++frames;
		history.push_back(0.5f + 0.5f * std::sin(frames * 0.05f));
		if(history.size() > 120) history.erase(history.begin());

		// 布局全部面板
		inputs.layout(Mk::wg::Rect{10, 10, 300, 760});
		display.layout(Mk::wg::Rect{330, 10, 300, 760});
		popCol.layout(Mk::wg::Rect{650, 10, 280, 300});
		collapsing.layout(Mk::wg::Rect{650, 320, 300, 90});
		tree.layout(Mk::wg::Rect{650, 420, 300, 90});
		subwin.layout(Mk::wg::Rect{650, 520, 300, 80});
		tabs.layout(Mk::wg::Rect{650, 610, 300, 80});
		group.layout(Mk::wg::Rect{960, 10, 300, 70});
		hsplit.layout(Mk::wg::Rect{960, 90, 300, 60});
		vsplit.layout(Mk::wg::Rect{960, 170, 300, 120});
		deco.layout(Mk::wg::Rect{960, 310, 300, 260});
		menuBar.layout(Mk::wg::Rect{0, 0, 1280, 24});
		popup.layout(Mk::wg::Rect{0, 0, 200, 80});
		modal.layout(Mk::wg::Rect{0, 0, 200, 80});
		imgBtn.layout(Mk::wg::Rect{960, 600, 60, 50});
		wrapRow.layout(Mk::wg::Rect{650, 700, 300, 60});
		grid.layout(Mk::wg::Rect{960, 660, 300, 90});
		nestWin.layout(Mk::wg::Rect{960, 770, 300, 120});

		// 动态增删
		if(frames == 40) inputs.addWidget(dynLeaf->node);
		if(frames == 80) inputs.node.remove(dynLeaf->node);

		// 弹窗触发 + When 翻转 + 菜单
		if(frames == 50) popupOpen = true;
		if(frames == 60) modalOpen = true;
		if(frames == 70) when.self.show = true;

		ImGui::Text("frames=%d clicks=%d menu=%d", frames, clicks, menuClicks);

		if(frames >= 120) {
			glfwSetWindowShouldClose(win.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	std::cout << "test17 wg 全控件 soak PASS (frames=" << frames
		<< " clicks=" << clicks << " menu=" << menuClicks << ")" << std::endl;
	return 0;
}
