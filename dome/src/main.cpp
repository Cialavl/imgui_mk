// dome：个人验证 demo（wg2 静态树 + 窗口直挂）
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg2/wg2.h>

static Mk::wg2::Label<Mk::wg2::Id("label1")> label1{"Hello, Mk::wg2!"};   // 静态树：声明一次（地址 = 稳定 ImGui ID）

static void ui(Mk::window& win) {
	label1.state.visible = true;    // 状态常驻，帧内可调
	label1.state.enabled = true;
	label1.state.stretch = 0.0f;
	win.setCentralWidget(label1);   // 窗口自己添加控件、管理布局（幂等，帧内重复调用安全）
}

int main() {
	Mk::MainWindow<> mainWindow("dome: wg2 demo", 900, 600);
	mainWindow.setUI<&ui>();        // 每帧窗口回调（先于中央布局执行）
	Mk::mkui<Mk::window>::run();
	return 0;
}
