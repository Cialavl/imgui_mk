#include <imgui_mk/windowRendereren.h>
#include <iostream>
#include <vector>
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
int main() {
	// 获取全局引擎，创建窗口会自动注册到引擎
	Mk::mkui<Mk::window>::RegistrationWindow();
	// 主窗口组件：创建窗口并注入 UI，组件持有源对象
	Mk::MainWindow<Mk::window> mainWindow("Pw", 1600, 900);
	Mk::mkui<Mk::window>::run();
	return 0;
}