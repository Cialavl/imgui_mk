#pragma once
// gallery 应用层：GalleryUI —— struct 创建控件设好属性 + 布局组装 + 一次根 layout
// 布局层次：
//   root (Overlay 叠放，全幅)
//   ├── mainCol (VBox)
//   │   ├── menuBar       无 role → 按内容高
//   │   ├── contentRow (SplitH, stretch)
//   │   │   ├── pageNav   pref 250（min 150，设计像素 em 缩放）
//   │   │   ├── pageHost  stretch（预览区）
//   │   │   └── sidePanel pref 260（SubWindow 嵌入右格）
//   │   └── statusRow     状态栏（FPS / 帧数 / 当前页）
//   └── popups（popupTree/modalTree/aboutTree：流式 BeginPopup/Modal，不占布局）
#include "gallery_pagehost.h"
#include "gallery_pages.h"
#include <functional>
#include <vector>

namespace gallery {

struct GalleryWindow {   // Qt 风格主窗口：构造组装 + addPage 注册页面
	// ---- 菜单栏 ----
	MenuItem miOpen, miExit, miDark, miLight, miInspector, miAbout;
	Menu<> menuFile, menuTheme, menuWindow, menuHelp;
	MenuBar<> menuBar;
	// ---- 主体 ----
	PageNav pageNav;             // 页面导航（侧栏目录）
	PageHost pageHost;           // 页面宿主（addPage 注册页面）
	// ---- 嵌入侧面板（SubWindow，固定右格）----
	SliderF panelSlider;
	FrameLabel panelFrame;
	SubWindow<> sidePanel;
	// ---- 状态栏 ----
	LabelText stFps, stFrame, stPage;
	HLayout statusRow;
	// ---- 弹窗（流式，不参与布局）----
	Label popupText, modalText, aboutText;
	ClosePopupButton popupClose, modalClose, aboutOk;
	Popup<> popupTree;
	PopupModal<> modalTree, aboutTree;
	// ---- 布局 ----
	SplitHLayout contentRow;   // SplitH: pageNav | pageHost | sidePanel
	VLayout mainCol;           // VBox: menuBar / contentRow / statusRow
	OverlayLayout root;        // Overlay: mainCol + popups

	explicit GalleryWindow(State& s) : sRef(&s) {   // 构造函数组装窗口（Qt 风格）
		initMenus(s);
		initPanels(s);
		initPopups(s);
		initStatus(s);
		initLayout();
	}

	// Qt 风格：窗口添加页面（组名仅在组首传入；refresh(State&) 存在则自动注册每帧刷新）
	template<typename P>
	void addPage(P& page, const char* group = nullptr) {
		if(group && *group) pageNav.self.addEntry(group, true);
		pageNav.self.addEntry(page.page.title, false);
		pageHost.self.addPage(page.page.rootNode());
		if constexpr (requires { page.refresh(*sRef); }) refreshers.push_back([&page](State& s) { page.refresh(s); });
	}

	// 每帧刷新所有已注册页面（替代 refreshGallery）+ 状态栏/菜单勾选态
	void refreshAll(State& s) {
		for(auto& fn : refreshers) fn(s);
		updateStatus(s);
	}

	// 弹窗常驻浮层：触发后自己显示（OpenPopup + BeginPopup/Modal），不参与布局
	void drawPopups() {
		popupTree.node.draw();
		modalTree.node.draw();
		aboutTree.node.draw();
	}

private:
	void initMenus(State& s) {
		miOpen.self.name = "Open...";
		miOpen.self.shortcut = "Ctrl+O";
		miOpen.self.onClick = [&] { s.openFileDlg(); };
		miExit.self.name = "Exit";
		miExit.self.shortcut = "Alt+F4";   // 系统级关闭（GLFW/Windows 处理）
		miExit.self.onClick = [&] { glfwSetWindowShouldClose(s.win->getHandle(), true); };
		miDark.self.name = "Dark";
		miDark.self.onClick = [&] { s.applyTheme(Theme::dark()); };
		miLight.self.name = "Light";
		miLight.self.onClick = [&] { s.applyTheme(Theme::light()); };
		miInspector.self.name = "Show/Hide Inspector";
		miInspector.self.onClick = [&] { s.toggleInspector(); };
		miAbout.self.name = "About Gallery";
		miAbout.self.onClick = [&] { s.aboutOpen = true; };

		menuFile.self.name = "File";
		menuFile.node.add(miOpen.node);
		menuFile.node.add(miExit.node);
		menuTheme.self.name = "Theme";
		menuTheme.node.add(miDark.node);
		menuTheme.node.add(miLight.node);
		menuWindow.self.name = "Window";
		menuWindow.node.add(miInspector.node);
		menuHelp.self.name = "Help";
		menuHelp.node.add(miAbout.node);
		menuBar.node.add(menuFile.node);
		menuBar.node.add(menuTheme.node);
		menuBar.node.add(menuWindow.node);
		menuBar.node.add(menuHelp.node);
	}

	void initPanels(State& s) {
		pageNav.self.current = &s.sel;
		pageHost.self.current = &s.sel;

		panelSlider.self.name = "Panel slider";
		panelSlider.self.value = &s.volume;
		panelFrame.self.frames = &s.frames;
		sidePanel.node.idname = "sidepanel";
		sidePanel.self.border = true;
		sidePanel.node.add(panelSlider.node);
		sidePanel.node.add(panelFrame.node);
	}

	void initPopups(State& s) {
		popupText.self.setName("A regular popup.");
		popupClose.self.name = "关闭";
		modalText.self.setName("A modal popup blocks interaction with the host.");
		modalClose.self.name = "关闭";
		aboutText.self.setName("mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg template struct widget library");
		aboutOk.self.name = "OK";
		popupTree.node.idname = "gpop";
		popupTree.self.open = &s.popupOpen;
		popupTree.node.add(popupText.node);
		popupTree.node.add(popupClose.node);
		modalTree.node.idname = "gmodal";
		modalTree.self.open = &s.modalOpen;
		modalTree.node.add(modalText.node);
		modalTree.node.add(modalClose.node);
		aboutTree.node.idname = "about";
		aboutTree.self.open = &s.aboutOpen;
		aboutTree.node.add(aboutText.node);
		aboutTree.node.add(aboutOk.node);
	}

	void initStatus(State& s) {
		(void)s;
		stFps.self.name = "FPS";
		stFrame.self.name = "帧";
		stPage.self.name = "页";
		statusRow.addWidget(stFps.node);
		statusRow.addWidget(stFrame.node);
		statusRow.addWidget(stPage.node);
	}

	void initLayout() {
		pageNav.setPrefSize(250.0f, 0.0f);    pageNav.setMinSize(150.0f, 0.0f);    // 导航 250（最小 150，设计像素 em 缩放）
		sidePanel.setPrefSize(260.0f, 0.0f);  sidePanel.setMinSize(180.0f, 0.0f);  // 侧面板 260（最小 180）
		contentRow.addWidget(pageNav.node);
		contentRow.addWidget(pageHost.node, 1.0f);   // 预览区弹性
		contentRow.addWidget(sidePanel.node);
		mainCol.addWidget(menuBar.node);
		mainCol.addWidget(contentRow.node, 1.0f);    // 内容行弹性（menuBar/statusRow 按内容高）
		mainCol.addWidget(statusRow.node);
		// 主布局外边距：窗口层默认（WindowPadding）已由引擎提供，主布局各层置 0 防叠加
		contentRow.setContentsMargins(0.0f, 0.0f);
		mainCol.setContentsMargins(0.0f, 0.0f);
		root.setContentsMargins(0.0f, 0.0f);
		root.addWidget(mainCol.node);
	}

	void updateStatus(State& s) {
		stFps.self.value = std::to_string((int)detail::fps()) + " fps";
		stFrame.self.value = std::to_string(s.frames);
		stPage.self.value = currentPageTitle();
		miDark.self.selected = s.darkActive;
		miLight.self.selected = !s.darkActive;
	}

	const char* currentPageTitle() const {
		const int target = pageNav.self.current ? *pageNav.self.current : 0;
		int i = 0;
		for(const auto& e : pageNav.self.entries) {
			if(!e.header) {
				if(i++ == target) return e.label;
			}
		}
		return "(none)";
	}

	State* sRef = nullptr;
	std::vector<std::function<void(State&)>> refreshers;
};

} // namespace gallery