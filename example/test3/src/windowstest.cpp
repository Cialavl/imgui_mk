#include <imgui_mk/windowRendereren.h>
#include <iostream>
#include <vector>
#include <string_mk/mk_string.h>
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <test3/windowtest.h>
int main() {
	mk::string<8> str = "Pw";
	AppMainWindow<int>::init(str);
	Mk::mkui::run();
	return 0;
}