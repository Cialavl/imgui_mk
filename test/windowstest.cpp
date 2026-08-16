#include <imgui_mk/windowRendereren.h>

#include <iostream>
#include <vector>
#include <string_mk/mk_string.h>
#include <imgui_mk/window.h>
#include<imgui_mk/mkui.h>

int main() {

    // 获取全局引擎，创建窗口会自动注册到引擎
    Mk::mkui<Mk::window>::RegistrationWindow();
    auto w1 = std::make_unique<Mk::window>("Window 1", 800, 600);
    auto w2 = std::make_unique<Mk::window>("Window 2", 800, 600);

    // 初始化之后源对象仍可使用（引擎与调用方共享窗口状态）
    std::cout << "handles: " << w1->getHandle() << ", " << w2->getHandle() << std::endl;

    // 程序退出时清理
    Mk::mkui<Mk::window>::run();


    return 0;

}