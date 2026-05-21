#pragma once 
#include <imgui_mk/windowRendereren.h>
#include <iostream>
#include <vector>
#include <string_mk/mk_string.h>
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>

template<typename T, typename ...Args>
struct InputWindow
{

};

template<typename T, typename ...Args>
struct AppMainWindow {
	static inline std::unique_ptr<Mk::window> instance = nullptr;
	static void init(mk::string<8> str) {
		instance = std::make_unique<Mk::window>(str.data(), 1600, 900);
		Mk::mkui::RegistrationWindow(instance.get());
	}
	static std::unique_ptr<Mk::window> GetWindow() {
		if (!instance) throw std::runtime_error("Window not initialized");
		return instance;
	}
	static void setui() {
		//instance->
	}
};