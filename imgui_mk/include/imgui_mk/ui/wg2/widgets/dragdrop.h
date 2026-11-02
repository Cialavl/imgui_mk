#pragma once
// Mk::wg2 拖放（ImGui DnD 封装）：类型安全的拖拽源 / 接收目标。
//   负载类型用 typeid hash 生成签名（ImGui 负载类型名 ≤32 字符）。
#include <string>
#include <typeinfo>

#include <imgui.h>

namespace Mk::wg2 {

// 负载签名：同一 T 一致，不同 T 不同
template <class T>
inline const char* payloadType() {
	static const std::string token = [] {
		std::string t = "wg2:";
		t += std::to_string(typeid(T).hash_code());
		return t;
	}();
	return token.c_str();
}

// 拖拽源：在控件绘制之后调用（依赖当前 item 的 hover 状态）
template <class T>
bool dragSource(const char* label, const T& payload) {
	if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) return false;
	ImGui::SetDragDropPayload(payloadType<T>(), &payload, sizeof(T));
	if (label && *label) ImGui::TextUnformatted(label);
	ImGui::EndDragDropSource();
	return true;
}

// 接收目标：本帧放下成功时调用 onDrop(payload) 并返回 true
template <class T, class F>
bool dropTarget(F&& onDrop) {
	if (!ImGui::BeginDragDropTarget()) return false;
	bool got = false;
	if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(payloadType<T>())) {
		if (p->Data && p->DataSize == static_cast<int>(sizeof(T))) {
			onDrop(*static_cast<const T*>(p->Data));
			got = true;
		}
	}
	ImGui::EndDragDropTarget();
	return got;
}

} // namespace Mk::wg2
