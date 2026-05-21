#pragma once
#include <imgui_mk/windowRendereren.h>
#include <iostream>
#include <vector>
#include <string_mk/mk_string.h>
#include <imgui_mk/window.h>
#include <inputwindow.h>
template<typename T>
class MainWindow {
public:
	MainWindow() {

	}

	void initui() {
		ImGui::Text("Hello, world %d", 123);

		if(ImGui::BeginChild("left_panel", ImVec2(150, 0), true)) {
			// 左侧子窗口内容...
			if(ImGui::Button("Save")) {
				save();
				//std::cout << "save" << "\n"; 
			}
			ImGui::SameLine(100);
			if(ImGui::Button("encode")) {
				encode();
				//std::cout << "encode" << "\n"; 
			}
		}
		ImGui::EndChild();

		ImGui::SameLine(); // 放置下一个子窗口在同一行上

		if(ImGui::BeginChild("right_panel", ImVec2(0, 0), true)) {
			// 右侧子窗口内容...
			ImGui::InputText("string1", buf1.data(), IM_ARRAYSIZE(buf1.data()));
			ImGui::InputText("string2", buf2.data(), IM_ARRAYSIZE(buf2.data()));
			ImGui::InputText("string3", buf3.data(), IM_ARRAYSIZE(buf3.data()));
			ImGui::InputText("string4", buf4.data(), IM_ARRAYSIZE(buf4.data()));
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		}
		ImGui::EndChild();
	}

	void save() {
		std::cout << "save" << "\n";
	}
	void encode() {
		std::cout << "encode" << "\n";
		inputwindow.show();
	}
	void show() {
		
	}
private:

	InputWin                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               dow<int> inputwindow;
	float f = 1.0;
	mk::string<1024> buf1;
	mk::string<1024> buf2;
	mk::string<1024> buf3;
	mk::string<1024> buf4;
	
	T a;
};