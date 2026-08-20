#pragma once
// Mk::wg 扩展控件与布局协议（应用层可选）：Sidebar / PageHost / SectionHead / CodeBlock /
// 保持精简，本头是"把 wg 树挂进全屏宿主 + 常用应用骨架"的扩展层。
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <algorithm>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Mk::wg {

	// ============ FrameLabel：帧数标签 ============
	struct FrameLabelImpl {
		int* frames = nullptr;
		void renderImpl(Node&) { if(frames) ImGui::Text("Frame %d", *frames); }
		Size measureImpl(Node&, const Size&) {
			ImVec2 s = ImGui::CalcTextSize("Frame 999999");
			return Size{s.x, s.y};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using FrameLabel = Control<FrameLabelImpl, DefaultStyle>;

	// ============ ClosePopupButton：点击关闭所在弹窗 ============
	struct ClosePopupButtonImpl {
		const char* name = "Close";
		void renderImpl(Node&) {
			if(ImGui::Button(name)) ImGui::CloseCurrentPopup();
		}
		Size measureImpl(Node&, const Size&) {
			ImVec2 s = ImGui::CalcTextSize(name);
			return Size{s.x + 20.0f, s.y + 12.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using ClosePopupButton = Control<ClosePopupButtonImpl, DefaultStyle>;

	// ============ SectionHead：节标题（粗标题 + 自动换行说明 + 分隔线）============
	struct SectionHeadImpl {
		const char* title = "";
		const char* desc = "";
		void renderImpl(Node& n) {
			ImGui::TextUnformatted(title);
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + n.bounds.w);   // wrap 宽 = 格子宽（与 measure 一致）
			ImGui::TextDisabled("%s", desc);
			ImGui::PopTextWrapPos();
			ImGui::Separator();
		}
		Size measureImpl(Node&, const Size& avail) {
			ImVec2 d = ImGui::CalcTextSize(desc, nullptr, true, avail.w);   // 换行多行高
			ImVec2 t = ImGui::CalcTextSize(title);
			return Size{avail.w, t.y + d.y + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f};   // 补 Separator 实高
		}
		// Qt 式协议：自然高（描述按单行计）；真实高由 heightForWidthImpl 按格子宽换算（V 布局 arrange 应用 HFW）
		Size sizeHintImpl(Node&) {
			ImVec2 t = ImGui::CalcTextSize(title);
			return Size{0.0f, t.y + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f};
		}
		float heightForWidthImpl(Node&, float w) {
			const float wrap = w > 0.0f ? w : -1.0f;   // -1 = 不换行（CalcTextSize 约定）
			ImVec2 d = ImGui::CalcTextSize(desc, nullptr, true, wrap);
			ImVec2 t = ImGui::CalcTextSize(title);
			return t.y + d.y + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f;
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using SectionHead = Control<SectionHeadImpl, DefaultStyle>;

	// ============ Sidebar：导航目录（分组 header + 条目列表 + 当前选中）============
	struct SidebarEntry { const char* label; bool header; };

	struct SidebarImpl {
		std::span<const SidebarEntry> entries;
		int* current = nullptr;
		float itemH = 22.0f;
		void renderImpl(Node& n) {
			// 用自己格子的尺寸开滚动子窗口（不横跨宿主）
			if(ImGui::BeginChild("sidebar", ImVec2(n.bounds.w, n.bounds.h), 0)) {
				int idx = 0;
				for(const auto& e : entries) {
					if(e.header) {
						ImGui::SeparatorText(e.label);
						continue;
					}
					ImGui::PushID(idx);
					const bool sel = (idx == *current);
					const float avail = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
					std::string clipped;   // 超宽才构造截断缓冲（省每帧分配）
					const char* label = e.label;
					if(ImGui::CalcTextSize(e.label).x > avail) { clipped = detail::ellipsize(e.label, avail); label = clipped.c_str(); }
					if(ImGui::Selectable(label, sel)) *current = idx;
					ImGui::PopID();
					++idx;
				}
			}
			ImGui::EndChild();
		}
		Size measureImpl(Node&, const Size& avail) {
			float h = 0.0f;
			for(const auto& e : entries) h += e.header ? 26.0f : itemH;
			return Size{avail.w, h};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using Sidebar = Control<SidebarImpl, DefaultStyle>;

	// ============ 代码语法着色扫描（CodeBlock 内部用）============
	namespace codehl {
		struct Seg { const char* begin; const char* end; int kind; };
		// kind: 0 默认 1 关键字 2 字符串 3 数字 4 注释

		inline bool identChar(char ch) {
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
		}
		inline bool digitChar(char ch) { return ch >= '0' && ch <= '9'; }

		inline bool keyword(std::string_view w) {
			static constexpr std::string_view kW[] = {
				"Mk", "wg", "std", "auto", "const", "constexpr", "return", "if", "else",
				"for", "while", "float", "double", "bool", "int", "void", "struct", "class",
				"static", "namespace", "true", "false", "nullptr", "ImVec2", "ImVec4",
				"template", "typename",
			};
			for(auto k : kW) {
				if(k == w) return true;
			}
			return false;
		}

		// 扫描一行 [p,end)，块注释状态 inBlock 跨行传递；结果覆盖写入 out（每行复用）
		inline void scanLine(const char* p, const char* end, bool& inBlock, std::vector<Seg>& out) {
			out.clear();
			const auto push = [&](const char* b, const char* e, int kind) {
				if(b < e) out.push_back({b, e, kind});
			};
			while(p < end) {
				if(inBlock) {   // 块注释内：找 */
					const char* q = p;
					while(q + 1 < end && !(q[0] == '*' && q[1] == '/')) ++q;
					if(q + 1 < end) {
						push(p, q + 2, 4);
						inBlock = false;
						p = q + 2;
					}
					else {
						push(p, end, 4);
						return;
					}
					continue;
				}
				if(p + 1 < end && p[0] == '/' && p[1] == '/') {   // 行注释
					push(p, end, 4);
					return;
				}
				if(p + 1 < end && p[0] == '/' && p[1] == '*') {   // 块注释开始
					const char* q = p + 2;
					while(q + 1 < end && !(q[0] == '*' && q[1] == '/')) ++q;
					if(q + 1 < end) {
						push(p, q + 2, 4);
						p = q + 2;
					}
					else {
						push(p, end, 4);
						inBlock = true;
						return;
					}
					continue;
				}
				if(*p == '"') {   // 字符串（处理 \ 转义，未闭合记到行尾）
					const char* q = p + 1;
					while(q < end) {
						if(*q == '\\') {
							q += 2;
							continue;
						}
						if(*q == '"') {
							++q;
							break;
						}
						++q;
					}
					if(q > end) q = end;
					push(p, q, 2);
					p = q;
					continue;
				}
				if(identChar(*p)) {   // 标识符/关键字
					const char* q = p;
					while(q < end && (identChar(*q) || digitChar(*q))) ++q;
					push(p, q, keyword(std::string_view(p, (size_t)(q - p))) ? 1 : 0);
					p = q;
					continue;
				}
				if(digitChar(*p)) {   // 数字（简化处理 . 和 f 后缀）
					const char* q = p;
					while(q < end && (digitChar(*q) || *q == '.' || *q == 'f')) ++q;
					push(p, q, 3);
					p = q;
					continue;
				}
				// 其他字符（括号/运算符/空白）：合并为默认段直到下一个特殊字符
				const char* q = p;
				while(q < end && !identChar(*q) && *q != '"' && *q != '/' && !digitChar(*q)) ++q;
				push(p, q, 0);
				p = q;
			}
		}
	} // namespace codehl
	// ============ CodeBlock：代码显示（等宽 + 行号 + 语法着色 + Copy）============
	// 等宽字体由使用方装载并注入（框架不持有字体资源）。
	struct CodeBlockImpl {
		std::string_view code;
		ImFont* mono = nullptr;
		std::function<ImFont*()> monoFn;   // 拉取式字体（渲染时调用；优先于 mono——重烘焙后首帧即生效）
		ImU32 copiedFrame = 0;             // Copy 反馈：最近一次复制帧号（60 帧内显示"已复制"）
		ImFont* activeMono() const { return monoFn ? monoFn() : mono; }
		void renderImpl(Node& n) {
			const ImGuiStyle& st = ImGui::GetStyle();
			const ImVec4 baseText = st.Colors[ImGuiCol_Text];
			const ImVec4 accent = st.Colors[ImGuiCol_TabActive];
			const ImVec4 gray = st.Colors[ImGuiCol_TextDisabled];
			// 主题亮度感知：浅色主题下字符串/数字换深色系，保证对比度
			const ImVec4 winBg = st.Colors[ImGuiCol_WindowBg];
			const bool light = (winBg.x + winBg.y + winBg.z) > 1.5f;
			const auto segColor = [&](int kind) -> ImVec4 {
				switch(kind) {
				case 1: return accent;
				case 2: return light ? ImVec4(0.13f, 0.55f, 0.30f, 1.0f) : ImVec4(0.55f, 0.85f, 0.58f, baseText.w);
				case 3: return light ? ImVec4(0.75f, 0.42f, 0.10f, 1.0f) : ImVec4(0.95f, 0.70f, 0.45f, baseText.w);
				case 4: return gray;
				default: return baseText;
				}
			};

			const ImVec2 avail0 = ImGui::GetContentRegionAvail();
			{
				const bool justCopied = ImGui::GetFrameCount() - copiedFrame < 60;
				const char* copyLabel = justCopied ? "已复制" : "Copy";
				const float w = ImGui::CalcTextSize(copyLabel).x + st.FramePadding.x * 2.0f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail0.x - w);
				if(ImGui::SmallButton(copyLabel)) {
					ImGui::SetClipboardText(std::string(code).c_str());
					copiedFrame = ImGui::GetFrameCount();
				}
			}

			ImVec4 bg = st.Colors[ImGuiCol_ChildBg];
			bg.x += (baseText.x - bg.x) * 0.06f;
			bg.y += (baseText.y - bg.y) * 0.06f;
			bg.z += (baseText.z - bg.z) * 0.06f;
			ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

			ImFont* cm = activeMono();
			if(cm) ImGui::PushFont(cm);
			const float copyRow = ImGui::GetFrameHeight() + st.ItemSpacing.y;   // Copy 行已占光标高
			const float childH = n.bounds.h > copyRow ? n.bounds.h - copyRow : 0.0f;   // 子窗口底对齐块底，不溢出
			if(ImGui::BeginChild("code", ImVec2(n.bounds.w, childH), ImGuiChildFlags_Border,
				   ImGuiWindowFlags_HorizontalScrollbar)) {
				const size_t totalLines = 1 + (size_t)std::count(code.begin(), code.end(), '\n');
				char wbuf[16];
				std::snprintf(wbuf, sizeof wbuf, "%zu", totalLines);
				const std::string probe((size_t)std::strlen(wbuf), '9');
				const float numW = ImGui::CalcTextSize(probe.c_str()).x + ImGui::GetStyle().FramePadding.x;

				char numBuf[16];
				int lineNo = 1;
				bool inBlock = false;
				std::vector<codehl::Seg> segs;
				const char* p = code.data();
				const char* end = p + code.size();
				while(p < end) {
					const char* eol = (const char*)std::memchr(p, '\n', (size_t)(end - p));
					if(!eol) eol = end;
					codehl::scanLine(p, eol, inBlock, segs);
					p = eol + 1;

					std::snprintf(numBuf, sizeof numBuf, "%d", lineNo++);
					const float tw = ImGui::CalcTextSize(numBuf).x;
					ImGui::SetCursorPosX(st.WindowPadding.x + numW - tw);
					ImGui::TextColored(gray, "%s", numBuf);
					ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x);

					for(size_t i = 0; i < segs.size(); ++i) {
						if(i > 0) ImGui::SameLine(0.0f, 0.0f);
						ImGui::TextColored(segColor(segs[i].kind), "%.*s",
							(int)(segs[i].end - segs[i].begin), segs[i].begin);
					}
					if(segs.empty()) {
						ImGui::TextUnformatted("");
					}
				}
			}
			ImGui::EndChild();
			if(cm) ImGui::PopFont();
			ImGui::PopStyleColor();
		}
		// 块总高：代码行（含行距）+ 子窗口壳 + Copy 行——measure 与 render 同源，防底部截断
		float blockHeight() const {
			const ImGuiStyle& st = ImGui::GetStyle();
			const size_t lines = 1 + (size_t)std::count(code.begin(), code.end(), '\n');
			ImFont* m = activeMono();
			const float lh = (m ? m->FontSize : ImGui::GetFontSize()) + st.ItemSpacing.y;   // 行高含行距
			const float content = (float)lines * lh - st.ItemSpacing.y;           // 末行无尾随行距
			const float chrome = 2.0f * (st.WindowPadding.y + st.ChildBorderSize); // 子窗口壳
			const float copyRow = ImGui::GetFrameHeight() + st.ItemSpacing.y;      // Copy 行
			return content + chrome + copyRow;
		}
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w, blockHeight()}; }
		Size sizeHintImpl(Node&) { return Size{0.0f, blockHeight()}; }   // Qt 式协议直连
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using CodeBlock = Control<CodeBlockImpl, DefaultStyle>;

	// ============ PageHost：多页宿主（滚动预览 child + 选中页布局）============
	// pages 为各页根 Node（visible 由本控件按选中页切换）；预览区不滚（画布），页内部各自滚动。
	struct PageHostImpl2 {
		int* current = nullptr;
		std::span<Node* const> pages;
		void renderImpl(Node& n) {
			const ImVec2 sz(n.bounds.w, n.bounds.h);
			if(ImGui::BeginChild("preview", sz, ImGuiChildFlags_Border,
				   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
				const int cnt = (int)pages.size();
				for(int i = 0; i < cnt; ++i) {
					if(pages[i]) pages[i]->visible = (i == *current);   // 非选中页隐藏（不布局不渲染）
				}
				if(*current >= 0 && *current < cnt && pages[*current]) {
					const ImVec2 cp = ImGui::GetCursorPos();
					const ImVec2 avail = ImGui::GetContentRegionAvail();
					pages[*current]->layout(
						Rect{cp.x, cp.y, avail.x > 0.0f ? avail.x : 0.0f, avail.y > 0.0f ? avail.y : 0.0f});
				}
			}
			ImGui::EndChild();
		}
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w, avail.h}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using PageHost2 = Control<PageHostImpl2, DefaultStyle>;


} // namespace Mk::wg
