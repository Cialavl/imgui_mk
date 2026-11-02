#pragma once
// Mk::wg2 轻量富文本：**加粗**、代码（反引号包裹）、[链接](href)
//   没有粗体字库，加粗用强调色呈现；代码用等宽字体；链接可点击。
#include <functional>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/font.h>
#include <imgui_mk/ui/wg2/widget.h>

namespace Mk::wg2 {

enum class RichKind { Text, Bold, Code, Link };

struct RichSpan {
	RichKind kind = RichKind::Text;
	std::string text;
	std::string href;
};

// 解析轻量标记；未闭合的标记按普通文本处理
inline std::vector<RichSpan> parseRichText(const std::string& src) {
	constexpr char kTick = 96;   // 反引号
	std::vector<RichSpan> out;
	std::string plain;
	auto flush = [&] {
		if (!plain.empty()) {
			out.push_back(RichSpan{RichKind::Text, plain, {}});
			plain.clear();
		}
	};

	for (std::size_t i = 0; i < src.size();) {
		if (i + 1 < src.size() && src[i] == '*' && src[i + 1] == '*') {
			const std::size_t end = src.find("**", i + 2);
			if (end != std::string::npos) {
				flush();
				out.push_back(RichSpan{RichKind::Bold, src.substr(i + 2, end - i - 2), {}});
				i = end + 2;
				continue;
			}
		}
		if (src[i] == kTick) {
			const std::size_t end = src.find(kTick, i + 1);
			if (end != std::string::npos) {
				flush();
				out.push_back(RichSpan{RichKind::Code, src.substr(i + 1, end - i - 1), {}});
				i = end + 1;
				continue;
			}
		}
		if (src[i] == '[') {
			const std::size_t mid = src.find("](", i + 1);
			const std::size_t end = (mid == std::string::npos) ? std::string::npos : src.find(')', mid + 2);
			if (mid != std::string::npos && end != std::string::npos) {
				flush();
				out.push_back(RichSpan{RichKind::Link, src.substr(i + 1, mid - i - 1), src.substr(mid + 2, end - mid - 2)});
				i = end + 1;
				continue;
			}
		}
		plain += src[i++];
	}
	flush();
	return out;
}

template<FixedString Id>
struct RichText {
	static constexpr auto id = Id;

	std::string text;
	std::function<void(const std::string&)> onLink;

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 320.0f, ImGui::GetTextLineHeightWithSpacing()};
	}
	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		const std::vector<RichSpan> spans = parseRichText(text);
		bool first = true;
		for (const auto& s : spans) {
			if (!first) ImGui::SameLine(0.0f, 0.0f);
			first = false;
			switch (s.kind) {
			case RichKind::Bold:
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
				ImGui::TextUnformatted(s.text.c_str());
				ImGui::PopStyleColor();
				break;
			case RichKind::Code: {
				FontScope scope(monoFont());
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				ImGui::TextUnformatted(s.text.c_str());
				ImGui::PopStyleColor();
				break;
			}
			case RichKind::Link: {
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
				ImGui::TextUnformatted(s.text.c_str());
				ImGui::PopStyleColor();
				const ImVec2 p0 = ImGui::GetItemRectMin();
				const ImVec2 p1 = ImGui::GetItemRectMax();
				ImGui::GetWindowDrawList()->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p1.y),
					ImGui::GetColorU32(ImGuiCol_CheckMark));
				if (ImGui::IsItemClicked() && onLink) onLink(s.href);
				break;
			}
			case RichKind::Text:
			default:
				ImGui::TextUnformatted(s.text.c_str());
				break;
			}
		}
	}
};

} // namespace Mk::wg2
