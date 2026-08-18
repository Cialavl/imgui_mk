// mkui 控件画廊应用入口：双顶层窗口（Gallery + Inspector）+ 常驻层（菜单栏/
// 浮动面板/三个弹窗）+ 文件对话框轮询。演示页本体见 gallery_pages.h，
// 代码片段见 snippets.h。交互式应用：菜单 File/Exit 或直接关窗退出。
#include "gallery_pages.h"
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_bridge.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

// 程序生成 64x64 棋盘格 RGBA 纹理（无外部文件依赖）
void makeChecker(unsigned char* px) {
	for(int y = 0; y < 64; ++y) {
		for(int x = 0; x < 64; ++x) {
			const bool odd = ((x / 8) + (y / 8)) % 2 == 1;
			unsigned char* p = &px[(y * 64 + x) * 4];
			p[0] = odd ? 220 : 60;
			p[1] = odd ? 160 : 120;
			p[2] = odd ? 60 : 220;
			p[3] = 255;
		}
	}
}

} // namespace

int main() {
	// DPI + 全动态自适应（应用显式 opt-in，测试代码不启用以保持像素断言确定性）：
	//   字号 = 基准 18 × 显示器缩放（启动时）× 宽度档位/跨屏变化（运行时）
	//   窗口尺寸 = 主显示器工作区百分比（小屏不超界、大屏饱满）
	Mk::fui::setDefaultTheme(Mk::fui::scaledForMonitor(Mk::fui::Theme::dark()));
	const auto [galW, galH] = Mk::workAreaSize(0.75f, 0.8f, 1440, 880);

	Mk::MainWindow<> gallery("mkui Widget Gallery", galW, galH);

	// 自定义字体链：中文（默认链）+ 等宽代码字体。ProggyClean 是内嵌矢量 TTF，
	// SizePixels 任意尺寸光栅化都清晰——代码字随 UI 字号联动（×0.85 略小一级，
	// 层级分明），rescale/档位/跨屏重烘焙时同步重挂。
	// 设置后立即按当前字号重烘焙一次，此后所有 rescale 都走这条链
	gallery.getWindow().setFontLoader([](float fontSize) {
		if(!Mk::window::Impl::defaultFontLoader(fontSize)) return false;
		ImFontConfig cfg;
		cfg.SizePixels = fontSize * 0.85f;
		gallery::gMono = ImGui::GetIO().Fonts->AddFontDefault(&cfg);
		return true;
	});

	// 运行时自适应：跨屏拖动/系统缩放变化 + 窗口宽度档位（900/1400/2000）
	gallery.getWindow().setAutoScaleFont(true);

	gallery::State s;
	s.win = &gallery;

	// —— 第二顶层窗口：Inspector（多窗口联动）。与画廊共享全部外置状态；
	// 点 X 仅隐藏（关闭回调拦截否决），画廊关闭（引擎销毁）后下一帧跟随退出
	const auto [insW, insH] = Mk::workAreaSize(0.3f, 0.7f, 420, 640);
	Mk::window inspector("Inspector", insW, insH);
	inspector.setAutoScaleFont(true);
	s.inspectorHandle = inspector.getHandle();
	if(s.inspectorHandle) {
		glfwSetWindowCloseCallback(s.inspectorHandle, [](GLFWwindow* w) {
			glfwSetWindowShouldClose(w, GLFW_FALSE);
			glfwHideWindow(w);
		});
	}
	inspector.setUserUI([&] {
		if(gallery.getHandle() == nullptr) {
			glfwSetWindowShouldClose(inspector.getHandle(), true);
			return;
		}
		++s.insFrames;
		using namespace Mk::fui;
		draw(vbox(8.0f,
			separatorText("Shared state (live link with Gallery)"),
			slider("Volume", s.volume, 0.0f, 1.0f),
			checkbox("Mute", s.mute),
			colorEdit3("Theme color", s.col3),
			hbox(8.0f,
				checkboxFlags("Bit A", s.bitFlags, 1),
				checkboxFlags("Bit B", s.bitFlags, 2),
				checkboxFlags("Bit C", s.bitFlags, 4)),
			separatorText("Independent frame loops"),
			labelText("Gallery frame", std::to_string(s.frames)),
			labelText("Inspector frame", std::to_string(s.insFrames)),
			textWrapped("Two top-level windows each own an ImGui context and frame loop; binding the same external variables keeps them live-linked: drag any slider or color in either window and the other follows the same frame. Clicking X only hides the Inspector (menu Window or the Gallery sidebar restores it); it follows the Gallery out when the Gallery closes.")));
	});

	// 棋盘纹理 + OOP 反向桥适配器
	static unsigned char pixels[64 * 64 * 4];
	makeChecker(pixels);
	s.tex = Mk::fui::loadTextureRGBA(pixels, 64, 64);
	s.adapter = std::make_unique<Mk::fui::WidgetAdapter<gallery::State::RevMaker>>(
		gallery::State::RevMaker{s});
	s.oopBox.add(*s.adapter);

	// —— 组装 wg 演示树（main 局部长期存活；State.wgTreeX 指向其 node，WgHost 渲染）——
	// 38 对象树与生命周期：VBox 组合 + 运行时 add 叶子 + 按钮更新计数
	Mk::wg::VBox<Mk::wg::Button, Mk::wg::Label> lifeBox;
	Mk::wg::Button addLeafBtn;   addLeafBtn.self.setLabel("添加 wg 叶子");
	Mk::wg::Label leafCount;     leafCount.self.setText("叶子 0");
	lifeBox.add(addLeafBtn.node);
	lifeBox.add(leafCount.node);
	int leafNo = 0;
	addLeafBtn.self.onClick = [&] { ++leafNo; leafCount.self.setText(("叶子 " + std::to_string(leafNo)).c_str()); };
	s.wgTreeLife = &lifeBox.node;

	// 39 布局：VBox 内嵌 HBox（模板组合）+ 滑块/复选
	Mk::wg::VBox<Mk::wg::HBox<Mk::wg::Label, Mk::wg::Label>, Mk::wg::SliderF, Mk::wg::CheckBox> layoutBox;
	layoutBox.child<0>().child<0>().self.setText("左");
	layoutBox.child<0>().child<1>().self.setText("右");
	layoutBox.child<1>().self.label = "音量";  layoutBox.child<1>().self.value = &s.volume;
	layoutBox.child<2>().self.label = "静音";  layoutBox.child<2>().self.checked = &s.mute;
	s.wgTreeLayout = &layoutBox.node;

	// 40 原子：文本/输入/滑块/复选/进度/下拉/标签值
	Mk::wg::VBox<Mk::wg::Label, Mk::wg::InputText<>, Mk::wg::SliderF, Mk::wg::CheckBox,
		Mk::wg::ProgressBar, Mk::wg::Combo, Mk::wg::LabelText> atomsBox;
	atomsBox.child<0>().self.setText("wg 原子控件演示");
	atomsBox.child<1>().self.label = "文本";
	atomsBox.child<2>().self.label = "滑块";  atomsBox.child<2>().self.value = &s.volume;
	atomsBox.child<3>().self.label = "开关";  atomsBox.child<3>().self.checked = &s.mute;
	atomsBox.child<4>().self.fraction = &s.volume;
	atomsBox.child<5>().self.label = "下拉";  atomsBox.child<5>().self.current = &s.comboSel;
	atomsBox.child<5>().self.items = s.kItems;  atomsBox.child<5>().self.count = 3;
	atomsBox.child<6>().self.label = "帧";    atomsBox.child<6>().self.text = std::to_string(s.frames);
	s.wgTreeAtoms = &atomsBox.node;

	// 41 容器：折叠 + 数据表 + 函数表
	Mk::wg::VBox<Mk::wg::Collapsing, Mk::wg::TextTable, Mk::wg::FnTable> containersBox;
	containersBox.child<0>().self.label = "折叠区";
	containersBox.child<0>().self.flags = ImGuiTreeNodeFlags_DefaultOpen;
	Mk::wg::Label folded; folded.self.setText("折叠内容");
	containersBox.child<0>().node.add(folded.node);
	containersBox.child<1>().self.id = "wgtable";
	containersBox.child<1>().self.headers = s.kItems; containersBox.child<1>().self.headerCount = 3;
	containersBox.child<1>().self.rows = &s.rows;
	containersBox.child<2>().self.id = "wgfn";
	containersBox.child<2>().self.headers = s.kItems; containersBox.child<2>().self.headerCount = 3;
	containersBox.child<2>().self.rowCount = 2;
	containersBox.child<2>().self.cell = [](int r, int c) {
		return std::to_string(r) + "," + std::to_string(c);
	};
	s.wgTreeContainers = &containersBox.node;

	// 42 桥接：wg 树内嵌 fui 视图（每帧重建）
	auto wgBridgeMaker = [&] {
		return Mk::fui::hbox(8.0f,
			Mk::fui::label("fui 视图（每帧重建）"),
			Mk::fui::labelText("音量", std::to_string(s.volume).substr(0, 4)));
	};
	Mk::wg::FuiView<decltype(wgBridgeMaker)> bridgeNode(wgBridgeMaker);
	s.wgTreeBridge = &bridgeNode.node;

	GLFWwindow* handle = gallery.getHandle();   // 菜单 Exit 回调用

	gallery.setUI([&] {
		// 动画驱动
		++s.frames;
		s.volume = 0.5f + 0.5f * std::sin(s.frames * 0.02f);
		s.history.push_back(0.5f + 0.5f * std::sin(s.frames * 0.05f));
		if(s.history.size() > 300) s.history.erase(s.history.begin());

		// 文件对话框轮询：完成即读取（文本截前 64KB，二进制只记大小）
		std::string picked;
		if(s.fileDlg.poll(picked)) {
			s.filePath = picked;
			s.filePreview.clear();
			s.fileSize = 0;
			s.fileBinary = false;
			s.fileMissing = false;
			if(!s.filePath.empty()) {
				std::error_code ec;
				s.fileSize = std::filesystem::file_size(s.filePath, ec);
				if(ec) {
					s.fileMissing = true;
				}
				else {
					std::ifstream in(s.filePath, std::ios::binary);
					constexpr std::size_t kCap = 64 * 1024;
					std::string buf(kCap, '\0');
					in.read(buf.data(), (std::streamsize)kCap);
					buf.resize((size_t)in.gcount());
					s.fileBinary = buf.find('\0') != std::string::npos;
					if(!s.fileBinary) s.filePreview = std::move(buf);
				}
			}
		}

		using namespace Mk::fui;

		const bool insVisible = s.inspectorHandle != nullptr
			&& glfwGetWindowAttrib(s.inspectorHandle, GLFW_VISIBLE) != 0;

		// —— 常驻层：菜单栏 + 浮动面板 ——
		draw(menuBar(
			menu("File",
				menuItem("Open...", [&] {
					s.fileDlg.open("Open File", {
						{"Text/Code", "*.txt;*.md;*.cpp;*.h;*.hpp;*.json;*.xml;*.ini;*.log"},
						{"All files", "*.*"},
					});
				}, "Ctrl+O"),
				menuItem("Exit", [handle] { glfwSetWindowShouldClose(handle, true); }, "Alt+F4")),
			menu("Theme",
				menuItem("Dark", [&] {
					s.theme = Mk::fui::Theme::dark();
					gallery.setTheme(s.theme);
				}),
				menuItem("Light", [&] {
					s.theme = Mk::fui::Theme::light();
					gallery.setTheme(s.theme);
				})),
			menu("Window",
				menuItem(insVisible ? "Hide Inspector" : "Show Inspector", [&] {
					if(s.inspectorHandle) {
						if(glfwGetWindowAttrib(s.inspectorHandle, GLFW_VISIBLE))
							glfwHideWindow(s.inspectorHandle);
						else
							glfwShowWindow(s.inspectorHandle);
					}
				})),
			menu("Help",
				menuItem("About Gallery", [&] { s.aboutOpen = true; }))));

		draw(panel("Floating Panel", 0,
			slider("Panel slider", s.volume, 0.0f, 1.0f),
			gallery::FrameLabel{s.frames}));

		// —— 主体：左侧目录 + 右侧演示（上控件下代码）——
		draw(hsplit(
			fixed("sidebar", 250.0f, gallery::Sidebar{gallery::kSidebar, s.sel}).min(150.0f).atMost(0.35f),
			stretch("preview", 1.0f, gallery::Preview{s})));

		// —— 弹窗（树外常驻）——
		draw(popup("Gallery Popup", s.popupOpen,
			label("A regular popup."),
			gallery::ClosePopupButton{"Close"}));
		draw(popupModal("Gallery Modal", s.modalOpen, 0,
			label("A modal popup blocks interaction with the host."),
			gallery::ClosePopupButton{"Close"}));
		draw(popupModal("About", s.aboutOpen, 0,
			label("mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nOOP widget tree + functional widget library"),
			gallery::ClosePopupButton{"OK"}));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}