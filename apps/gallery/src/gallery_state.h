#pragma once
// gallery 共享状态：State（跨页数据）+ FnDrawerImpl（FnRender 页用）+ gMono（等宽字体）
#include <imgui_mk/mkui.h>
#include <imgui_mk/window.h>




#include <imgui_mk/ui/wg2/wg2.h>
#include <imgui_mk/ui/wg2/style.h>
#include <imgui_mk/ui/wg2/texture.h>
#include <Mainwindow.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace gallery {

using namespace Mk::wg2;

// 等宽代码字体（main 在创建窗口后、首帧前 AddFontDefault 装载填充）
inline ImFont* gMono = nullptr;

// ============ 共享状态 ============

struct State {
	int frames = 0, insFrames = 0;
	int sel = 0;
	int clicks = 0, imgBtnClicks = 0;
	float volume = 0.6f;
	bool mute = false;
	std::vector<float> history;

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

	Theme theme = Theme::dark();
	bool darkActive = true;          // 当前主题是否深色（菜单勾选态；按背景亮度推导）
	ImTextureID tex{};

	FileDialog fileDlg;
	std::string filePath, filePreview;
	std::uintmax_t fileSize = 0;
	bool fileBinary = false, fileMissing = false;

	GLFWwindow* inspectorHandle = nullptr;

	static constexpr const char* kItems[] = {"Option A", "Option B", "Option C"};
	static constexpr const char* kHeaders[] = {"Name", "Value", "Note"};
	std::vector<std::vector<std::string>> rows{
		{"A", "1", "data driven"},
		{"B", "2", "external vector"},
		{"C", "3", "refreshed per frame"},
	};

	// 统一换肤：引擎广播 applyThemeAll（各窗口自己帧内应用，跨窗口安全）；
	// darkActive 供菜单勾选态（按背景亮度推导）
	void applyTheme(const Theme& t) {
		theme = t;
		darkActive = t.windowBg.x < 0.5f;
		Mk::mkui<Mk::window>::RegistrationWindow().applyThemeAll(t);
	}

	// 显示/隐藏 Inspector（菜单项与多窗口页共用）
	void toggleInspector() {
		if(inspectorHandle) {
			if(glfwGetWindowAttrib(inspectorHandle, GLFW_VISIBLE))
				glfwHideWindow(inspectorHandle);
			else
				glfwShowWindow(inspectorHandle);
		}
	}

	// 打开文件对话框（菜单 Open / Ctrl+O / 文件页共用）
	void openFileDlg() {
		fileDlg.open("Open File", {
			{"Text/Code", "*.txt;*.md;*.cpp;*.h;*.hpp;*.json;*.xml;*.ini;*.log"},
			{"All files", "*.*"},
		});
	}
};

// ============ 引擎访问助手（页面零窗口依赖）============

// 首个存活窗口的实际烘焙字号（无窗口返回 0）
inline float engineFontSize() {
	return Mk::mkui<Mk::window>::RegistrationWindow().fontSize();
}

// 引擎级字号重烘焙广播（各窗口帧内应用）
inline void engineRescale(float fontSize) {
	Mk::mkui<Mk::window>::RegistrationWindow().rescaleAll(fontSize);
}

// FnRender 页演示用的自定义 callable（可默认构造，State 用指针绑定）
struct FnDrawerImpl {
	State* s = nullptr;
	void operator()() {
		if(s) ImGui::ProgressBar(s->volume, ImVec2(-FLT_MIN, 0), "FnRender 演示条");
	}
};

} // namespace gallery
