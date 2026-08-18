// mkui 控件画廊应用入口：双顶层窗口（Gallery + Inspector）+ 常驻层（菜单栏/
// 浮动面板/三个弹窗）+ 文件对话框轮询。演示页本体见 gallery_pages.h，
// 代码片段见 snippets.h。交互式应用：菜单 File/Exit 或直接关窗退出。
#include "gallery_pages.h"

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
	Mk::MainWindow<> gallery("mkui Widget Gallery", 1440, 880);

	// 等宽字体装入画廊图集：此刻字体纹理尚未构建（首帧才烘焙），零重建成本
	gallery::gMono = ImGui::GetIO().Fonts->AddFontDefault();

	gallery::State s;
	s.win = &gallery;

	// —— 第二顶层窗口：Inspector（多窗口联动）。与画廊共享全部外置状态；
	// 点 X 仅隐藏（关闭回调拦截否决），画廊关闭（引擎销毁）后下一帧跟随退出
	Mk::window inspector("Inspector", 420, 640);
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

	GLFWwindow* handle = gallery.getHandle();

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
			fixed("sidebar", 250.0f, gallery::Sidebar{gallery::kSidebar, s.sel}),
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