#pragma once
// Mk::wg2 右键菜单封装
#include <imgui.h>

namespace Mk::wg2 {

// 当前 item 上右键弹出（控件绘制之后调用）
template <class F>
bool contextMenu(const char* id, F&& items) {
	if (ImGui::BeginPopupContextItem(id)) {
		items();
		ImGui::EndPopup();
		return true;
	}
	return false;
}

// 窗口空白处右键弹出
template <class F>
bool contextMenuWindow(const char* id, F&& items) {
	if (ImGui::BeginPopupContextWindow(id,
			ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		items();
		ImGui::EndPopup();
		return true;
	}
	return false;
}

} // namespace Mk::wg2
