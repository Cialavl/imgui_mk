#pragma once
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_style.h>
#include <array>
#include <cstdio>
#include <cfloat>
#include <cmath>
#include <string>
#include <vector>

// Mk::wg 核心控件集：容器（本文件）与原子控件（见下）。
// 每个控件 = 自己的模板 struct（数据+功能函数）经 Control 组合，零继承。
// 布局契约：measureImpl 求期望尺寸（两遍布局第一遍），arrangeImpl 给
// 子节点分配 bounds（第二遍），renderImpl 按 bounds 驱动 ImGui 渲染。

namespace Mk::wg {
	namespace detail {
		// —— 真实几何度量（measure 与 arrange 与渲染三者同源，杜绝高度漂移）——
		inline float frameH() {   // 输入类控件行高（滑块/输入/下拉/进度条…）
			return ImGui::GetFrameHeight();
		}
		inline float headerH() {  // 树/折叠标题行高（头 + 行距）
			return ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
		}
		inline float lineH() {    // 文本行高（含行距）
			return ImGui::GetTextLineHeightWithSpacing();
		}
		// item 总宽（frame + 右侧标签）换算：设 frame 宽使渲染后总宽恰为 totalW。
		inline float frameWidthFor(float totalW, const char* name) {
			if(totalW <= 0.0f) return totalW;
			const float lw = (name && name[0])
				? ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemInnerSpacing.x
				: 0.0f;
			const float w = totalW - lw;
			return w > 0.0f ? w : totalW;
		}

		// —— em 设计像素换算与分格收敛（前置：Layout 分格账目需要）——
		inline constexpr float kDesignFontBase = 18.0f;
		// 截断文本至指定显示宽（含 "..."）；不超宽则原样返回（EllipsisLabel 用）
		inline std::string ellipsize(const std::string& text, float avail) {
			const ImVec2 full = ImGui::CalcTextSize(text.c_str());
			if(full.x <= avail) return text;
			const float ell = ImGui::CalcTextSize("...").x;
			const float budget = avail - ell;
			size_t lo = 0, hi = text.size();   // lo 可行、hi 不可行
			while(lo < hi) {
				const size_t mid = (lo + hi + 1) / 2;
				if(ImGui::CalcTextSize(text.c_str(), text.c_str() + mid).x <= budget) lo = mid;
				else hi = mid - 1;
			}
			return text.substr(0, lo) + "...";
		}
		// 分格尺寸收敛：先压 maxFrac 上限、再保 minPx 下限（mx<mn 时取下限，无断言）
		inline float clampCellValue(float des, float mn, float mx) {
			const float v = des > mx ? mx : des;
			return v < mn ? mn : v;
		}
		// 布局缺省哨兵解析（Qt 语义：spacing()/contentsMargins() 负值 = 样式默认；
		// 回落 ImGui style → 主题/字号联动；setSpacing/setContentsMargins 显式覆盖、-1 还原）
		inline float spacingX(float s) { return s >= 0.0f ? s : ImGui::GetStyle().ItemSpacing.x; }
		inline float spacingY(float s) { return s >= 0.0f ? s : ImGui::GetStyle().ItemSpacing.y; }
		inline float marginX(float m) { return m >= 0.0f ? m : ImGui::GetStyle().WindowPadding.x; }
		inline float marginY(float m) { return m >= 0.0f ? m : ImGui::GetStyle().WindowPadding.y; }
	}

	// 设计像素 → 实际像素（× 当前字号/18）
	inline float em(float designPx) {
		return designPx * ImGui::GetFontSize() / detail::kDesignFontBase;
	}

	// ============ 唯一布局控件：Layout（六模式，子运行时挂入，天然嵌套）============
	// mode 切换堆叠语义：V（垂直）/ H（水平）/ Wrap（流式换行）/ Grid（固定列网格）/
	//   SplitH / SplitV（分格：子经 setFixed/setStretch 预设角色）/ Overlay（重叠）。
	// 子控件一律 node.add 运行时挂入 → 嵌套 = Layout 套 Layout；
	// 声明契约：子控件必须先于父 Layout 声明（析构反序保证父 clear() 时子存活）。

	// ============================================================
	// ============ 工具：em2 / 纹理 ============
// em2：设计像素 → 当前字体缩放（基准 18）
inline ImVec2 em2(float designW, float designH) {
	const float k = ImGui::GetFontSize() / detail::kDesignFontBase;
	return ImVec2(designW * k, designH * k);
}

// RGBA8 纹理上传为 OpenGL 纹理（返回 ImTextureID；配合 deleteTexture 释放）
inline ImTextureID loadTextureRGBA(const unsigned char* pixels, int width, int height) {
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return (ImTextureID)(intptr_t)tex;
}
inline void deleteTexture(ImTextureID tex) {
	GLuint t = (GLuint)(intptr_t)tex;
	glDeleteTextures(1, &t);
}

// Qt 式布局体系（独立布局类型 + 共享盒式求解器）
	//   VLayout / HLayout / GridLayout / WrapLayout / SplitHLayout / SplitVLayout / OverlayLayout
	//   addWidget(w[,stretch]) / addStretch(w) / setSpacing / setContentsMargins
	// 求解：hint 起步 -> 不足按 (hint-min) 比例压缩 -> 富余按 stretch/Expanding 分配
	// ============================================================

	namespace detail {
		struct SolveItem {
			Node* node;
			float hint, mn, mx;      // 主轴 hint / 下限 / 上限
			bool canGrow, canShrink, wantsExpand;
			float stretch;
			float cross;             // 副轴自然尺寸
		};

		inline std::vector<SolveItem> collectItems(Node& n, bool axis, float availCross) {
			std::vector<SolveItem> items;
			for(auto* c : n.children) {
				if(!c) continue;
				const auto& ov = c->overrides;
				Size h = c->hint();
				Size mn = c->minHint();
				Policy p = c->getPolicy();
				if(!axis && c->hasHFW() && availCross > 0.0f) {   // 纵轴格子宽已定（innerW）：换行类按宽求高（横轴求解前宽度未知，跳过）
					const float h2 = c->hfw(availCross);
					if(h2 >= 0.0f) h.h = h2;
				}
				SolveItem it{};
				it.node = c;
				it.hint = axis ? h.w : h.h;
				it.mn = axis ? mn.w : mn.h;
				it.mx = 1e9f;
				// 尺寸覆盖均为设计像素（em 缩放）：随字号/DPI 等比，字号 18 时恒等
				if(axis) {
					if(ov.prefW >= 0) it.hint = em(ov.prefW);   // hint 覆盖（min/max 独立）
					if(ov.fixedW >= 0) { it.hint = em(ov.fixedW); it.mn = em(ov.fixedW); it.mx = em(ov.fixedW); }
					if(ov.minW >= 0) it.mn = std::max(it.mn, em(ov.minW));
					if(ov.maxW >= 0) it.mx = std::min(it.mx, em(ov.maxW));
				} else {
					if(ov.prefH >= 0) it.hint = em(ov.prefH);
					if(ov.fixedH >= 0) { it.hint = em(ov.fixedH); it.mn = em(ov.fixedH); it.mx = em(ov.fixedH); }
					if(ov.minH >= 0) it.mn = std::max(it.mn, em(ov.minH));
					if(ov.maxH >= 0) it.mx = std::min(it.mx, em(ov.maxH));
				}
				it.mn = std::min(it.mn, it.hint);
				// mx 不随 hint 抬升（max 覆盖允许压低于 hint，由最终 clamp 生效）
				it.mn = std::min(it.mn, it.mx);   // max 覆盖可能压低于 mn（如 maxFrac），保持区间有效
				const auto kind = axis ? p.h : p.v;
				it.canGrow = p.canGrow(kind) || c->role.stretch > 0.0f;
				it.canShrink = p.canShrink(kind);
				it.wantsExpand = p.wantsExpand(kind) || c->role.stretch > 0.0f;
				it.stretch = c->role.stretch;
				it.cross = axis ? h.h : h.w;
				items.push_back(it);
			}
			return items;
		}

		// 盒式求解（QBoxLayout 简化）：返回每项主轴尺寸
		inline std::vector<float> solveRow(std::vector<SolveItem>& items, float room) {
			const int cnt = (int)items.size();
			std::vector<float> out(cnt, 0.0f);
			if(cnt == 0) return out;
			float sumHint = 0, sumMin = 0, sumShrink = 0, sumStretch = 0, sumExpand = 0;
			for(int i = 0; i < cnt; ++i) {
				out[i] = items[i].hint;
				sumHint += items[i].hint;
				sumMin += items[i].mn;
				sumShrink += items[i].canShrink ? (items[i].hint - items[i].mn) : 0.0f;
				if(items[i].canGrow && items[i].stretch > 0.0f) sumStretch += items[i].stretch;
				if(items[i].canGrow && items[i].wantsExpand) sumExpand += 1.0f;
			}
			if(sumHint > room && sumShrink > 0.0f) {
				const float k = std::min(1.0f, (sumHint - room) / sumShrink);
				for(int i = 0; i < cnt; ++i) {
					if(!items[i].canShrink) continue;
					out[i] = items[i].hint - (items[i].hint - items[i].mn) * k;
				}
			}
			float used = 0;
			for(float v : out) used += v;
			float extra = room - used;
			if(extra > 0.01f) {
				if(sumStretch > 0.0f) {
					for(int i = 0; i < cnt; ++i) if(items[i].canGrow && items[i].stretch > 0.0f)
						out[i] += extra * (items[i].stretch / sumStretch);
				} else if(sumExpand > 0.0f) {
					for(int i = 0; i < cnt; ++i) if(items[i].canGrow && items[i].wantsExpand)
						out[i] += extra / sumExpand;
				} else {
					int g = 0;
					for(int i = 0; i < cnt; ++i) if(items[i].canGrow) ++g;
					if(g > 0) for(int i = 0; i < cnt; ++i) if(items[i].canGrow) out[i] += extra / (float)g;
				}
			}
			for(int i = 0; i < cnt; ++i) out[i] = std::clamp(out[i], items[i].mn, items[i].mx);
			return out;
		}
	} // namespace detail

	// 布局内核（六模式共用；K 为编译期模式标记）
	enum class LayoutKind : unsigned char { V, H, Grid, Wrap, SplitH, SplitV, Overlay };

	template<LayoutKind K>
	struct LayoutCore {
		static constexpr bool kLayoutFill = true;   // Qt 语义：布局恒填满宿主容器
		float spacing = -1.0f;                   // 子间距哨兵：<0 = 回落 ImGui style ItemSpacing（主题/字号联动）
		float marginX = -1.0f, marginY = -1.0f;  // 内容边距哨兵：<0 = 回落 WindowPadding（Qt contentsMargins 语义）
		int cols = 3;                            // Grid 列数

		// Qt 风格子项管理（转发 node.add + 写角色）
		void addWidget(Node& host, Node& w, float stretch = 0.0f) { w.role = ItemRole{stretch, true}; host.add(w); }
		void addStretch(Node& host, float weight = 1.0f) { sink.role = ItemRole{weight, true}; host.add(sink); }
		void setSpacing(float s) { spacing = s; }
		void setContentsMargins(float x, float y) { marginX = x; marginY = y; }

		Node sink;   // addStretch 占位节点（吸附剩余空间；无渲染行为）

		Size measureImpl(Node& n, const Size& avail) { const Size h = sizeHintImpl(n); return Size{avail.w > 0.0f ? avail.w : h.w, h.h}; }   // 旧协议桥：宽取容器可用，高取自然
		Size sizeHintImpl(Node& n);
		Size minimumSizeHintImpl(Node& n);
		Policy policyImpl(Node&) { return Policy{}; }
		void renderImpl(Node& n) {   // 布局渲染：逐子按 bounds 定位（origin 传播），结束后恢复光标
			const ImVec2 cursorSave = ImGui::GetCursorPos();
			for(auto* c : n.children) { if(!c) continue; detail::drawChildAt(n, *c); }
			ImGui::SetCursorPos(cursorSave);
		}
		void arrangeImpl(Node& n, const Rect& r);
	};

	template<LayoutKind K>
	Size LayoutCore<K>::sizeHintImpl(Node& n) {
		if constexpr(K == LayoutKind::V || K == LayoutKind::SplitV) {
			auto items = detail::collectItems(n, false, 0.0f);
			const float sp = detail::spacingY(spacing), mx = detail::marginX(marginX), my = detail::marginY(marginY);
			float h = 0, w = 0;
			for(auto& it : items) { h += it.hint; w = std::max(w, it.cross); }
			if(items.size() > 1) h += sp * (float)(items.size() - 1);
			return Size{w + mx * 2.0f, h + my * 2.0f};
		} else if constexpr(K == LayoutKind::H || K == LayoutKind::SplitH) {
			auto items = detail::collectItems(n, true, 0.0f);
			const float sp = detail::spacingX(spacing), mx = detail::marginX(marginX), my = detail::marginY(marginY);
			float w = 0, h = 0;
			for(auto& it : items) { w += it.hint; h = std::max(h, it.cross); }
			if(items.size() > 1) w += sp * (float)(items.size() - 1);
			return Size{w + mx * 2.0f, h + my * 2.0f};
		} else if constexpr(K == LayoutKind::Overlay) {
			auto items = detail::collectItems(n, false, 0.0f);
			const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
			float w = 0, h = 0;
			for(auto& it : items) { w = std::max(w, it.cross); h = std::max(h, it.hint); }
			return Size{w + mx * 2.0f, h + my * 2.0f};
		} else if constexpr(K == LayoutKind::Wrap) {
			// 两遍：按自然宽打包成行，行高=max
			auto items = detail::collectItems(n, true, 0.0f);
			const float sp = detail::spacingX(spacing), mx = detail::marginX(marginX), my = detail::marginY(marginY);
			float x = 0, y = 0, rowH = 0, maxW = 0;
			// availW 未知时用 hint 总和宽（无界，单行）——Wrap measure 需外部 avail，见 Node::measure 桥
			for(auto& it : items) {
				(void)0;   // sizeHint 无界单行；换行打包在 arrangeImpl
				x += it.hint + sp;
				rowH = std::max(rowH, it.cross);
			}
			if(x > 0) { maxW = std::max(maxW, x - sp); y += rowH; }
			return Size{maxW + mx * 2.0f, y + my * 2.0f};
		} else {
			// Grid：cols 列均分行
			auto items = detail::collectItems(n, true, 0.0f);
			const int c = std::max(1, cols);
			const float spx = detail::spacingX(spacing), spy = detail::spacingY(spacing);
			const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
			float rowH = 0, maxCellH = 0, w = 0;
			for(std::size_t i = 0; i < items.size(); ++i) {
				rowH = std::max(rowH, items[i].cross);
				if((i % c) == (std::size_t)(c - 1) || i + 1 == items.size()) { maxCellH += rowH; rowH = 0; }
			}
			for(std::size_t i = 0; i < items.size() && i < (std::size_t)c; ++i) w += items[i].hint;
			return Size{w + spx * (float)(c - 1) + mx * 2.0f, maxCellH + spy * (float)((items.size() + c - 1) / c > 1 ? (items.size() + c - 1) / c - 1 : 0) + my * 2.0f};
		}
	}

	template<LayoutKind K>
	Size LayoutCore<K>::minimumSizeHintImpl(Node& n) {
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		float w = 0, h = 0; int cnt = 0;
		for(auto* c : n.children) {
			if(!c) continue; ++cnt;
			Size mn = c->minHint();
			if constexpr(K == LayoutKind::V || K == LayoutKind::SplitV) { w = std::max(w, mn.w); h += mn.h; }
			else if constexpr(K == LayoutKind::H || K == LayoutKind::SplitH) { h = std::max(h, mn.h); w += mn.w; }
			else { w = std::max(w, mn.w); h = std::max(h, mn.h); }
		}
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	template<LayoutKind K>
	void LayoutCore<K>::arrangeImpl(Node& n, const Rect& r) {
		const float mx = detail::marginX(marginX), my = detail::marginY(marginY);
		const float innerX = r.x + mx, innerY = r.y + my;
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);
		const float innerH = std::max(0.0f, r.h - my * 2.0f);
		if constexpr(K == LayoutKind::V) {
			auto items = detail::collectItems(n, false, innerW);
			const float spY = detail::spacingY(spacing);
			const float room = innerH - spY * (float)(items.size() > 1 ? items.size() - 1 : 0);
			auto tracks = detail::solveRow(items, room);
			float y = innerY;
			for(std::size_t i = 0; i < items.size(); ++i) {
				items[i].node->arrange(Rect{innerX, y, innerW, tracks[i]});
				y += tracks[i] + spY;
			}
		} else if constexpr(K == LayoutKind::H) {
			auto items = detail::collectItems(n, true, innerH);
			const float spX = detail::spacingX(spacing);
			const float room = innerW - spX * (float)(items.size() > 1 ? items.size() - 1 : 0);
			auto tracks = detail::solveRow(items, room);
			float x = innerX;
			for(std::size_t i = 0; i < items.size(); ++i) {
				items[i].node->arrange(Rect{x, innerY, tracks[i], innerH});
				x += tracks[i] + spX;
			}
		} else if constexpr(K == LayoutKind::SplitH) {
			auto items = detail::collectItems(n, true, innerH);
			const float spX = detail::spacingX(spacing);
			const float room = innerW - spX * (float)(items.size() > 1 ? items.size() - 1 : 0);
			auto tracks = detail::solveRow(items, room);
			float x = innerX;
			for(std::size_t i = 0; i < items.size(); ++i) {
				items[i].node->arrange(Rect{x, innerY, tracks[i], innerH});
				x += tracks[i] + spX;
			}
		} else if constexpr(K == LayoutKind::SplitV) {
			auto items = detail::collectItems(n, false, innerW);
			const float spY = detail::spacingY(spacing);
			const float room = innerH - spY * (float)(items.size() > 1 ? items.size() - 1 : 0);
			auto tracks = detail::solveRow(items, room);
			float y = innerY;
			for(std::size_t i = 0; i < items.size(); ++i) {
				items[i].node->arrange(Rect{innerX, y, innerW, tracks[i]});
				y += tracks[i] + spY;
			}
		} else if constexpr(K == LayoutKind::Overlay) {
			for(auto* c : n.children) { if(c) c->arrange(Rect{innerX, innerY, innerW, innerH}); }
		} else if constexpr(K == LayoutKind::Wrap) {
			auto items = detail::collectItems(n, true, 0.0f);
			const float spX = detail::spacingX(spacing), spY = detail::spacingY(spacing);
			float x = innerX, y = innerY, rowH = 0;
			for(auto& it : items) {
				if(x > innerX && x + it.hint > innerX + innerW) { y += rowH + spY; x = innerX; rowH = 0; }
				it.node->arrange(Rect{x, y, it.hint, it.cross});
				x += it.hint + spX;
				rowH = std::max(rowH, it.cross);
			}
		} else {
			// Grid
			auto items = detail::collectItems(n, true, 0.0f);
			const int c = std::max(1, cols);
			// Grid：列均分（Qt QGridLayout 语义：无 stretch 列均分可用宽），行高 = 行内最大 hint 高
			const std::size_t rows = (items.size() + c - 1) / c;
			const float spX = detail::spacingX(spacing), spY = detail::spacingY(spacing);
			const float colW = innerW > 0.0f ? (innerW - spX * (float)(c - 1)) / (float)c : 0.0f;
			std::vector<float> rowH(rows, 0.0f);
			for(std::size_t i = 0; i < items.size(); ++i) rowH[i / c] = std::max(rowH[i / c], items[i].cross);
			float yy = innerY;
			for(std::size_t ri = 0; ri < rows; ++ri) {
				float xx = innerX;
				for(int ci = 0; ci < c; ++ci) {
					const std::size_t idx = ri * c + ci;
					if(idx < items.size()) items[idx].node->arrange(Rect{xx, yy, colW, rowH[ri]});
					xx += colW + spX;
				}
				yy += rowH[ri] + spY;
			}
		}
	}

	// Wrap 布局 measure 需要 avail 宽（打包依据）——通过 Node::measure 桥传入：
	// 布局类型自带 wrapAvailW 缓存，由 measure 桥刷新（见下 WrapLayout 特化钩子）。
	// 简化实现：WrapLayoutCore 在 sizeHintImpl 中读 n.lastAvailW（Node 通用字段）。
	template<LayoutKind K> struct LayoutDataQ : LayoutCore<K> {
		// Wrap 换行宽度由 arrangeImpl 按 r.w 打包（sizeHint 返回无界单行）
	};

	template<LayoutKind K>
	using LayoutQ = Control<LayoutDataQ<K>, DefaultStyle>;

	using VLayout      = LayoutQ<LayoutKind::V>;
	using HLayout      = LayoutQ<LayoutKind::H>;
	using GridLayout   = LayoutQ<LayoutKind::Grid>;
	using WrapLayout   = LayoutQ<LayoutKind::Wrap>;
	using SplitHLayout = LayoutQ<LayoutKind::SplitH>;
	using SplitVLayout = LayoutQ<LayoutKind::SplitV>;
	using OverlayLayout= LayoutQ<LayoutKind::Overlay>;

	// FnRender：裸绘制函数节点（F 可默认构造；尺寸由 arrange 给全）
	template<typename F>
	struct FnRenderImpl {
		F fn;
		void renderImpl(Node&) { fn(); }
		Size sizeHintImpl(Node&) { return Size{0.0f, 0.0f}; }
		Policy policyImpl(Node&) { return Policy{Policy::Expanding, Policy::Expanding}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	template<typename F>
	using FnRender = Control<FnRenderImpl<F>, DefaultStyle>;

	// ============ SubWindow（嵌套子窗口，独立滚动，id/尺寸/边框）============

	template<typename... Ts>
	struct SubWindowData {
		float width = 0.0f;    // 0 = 填充可用
		float height = 0.0f;   // 0 = 填充可用
		bool border = true;
		float spacing = -1.0f;  // 子控件纵向间距哨兵：<0 = 回落 ItemSpacing.y（主题/字号联动），显式值覆盖
		float lastContent = 0.0f;   // 上一帧真实内容高（renderImpl 反馈，自愈 measure 误差）

		// Qt 式装配转发：Control::addWidget 探测本函数后把子节点挂入 host（SubWindow 按 measure 纵向堆叠）
		void addWidget(Node& host, Node& w, float stretch = 0.0f) { (void)stretch; host.add(w); }
	};
	template<typename... Ts>
	using SubWindow = Control<SubWindowData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(SubWindowData<Ts...>& self, Node& n, const Size& avail) {
		float w = self.width > 0 ? self.width : avail.w;
		if(self.height > 0) return Size{w, self.height};   // 显式高度优先
		Size inner{0, 0};
		int cnt = 0;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{w, avail.h});
			inner.h += s.h;
			++cnt;
		}
		if(cnt == 0) return Size{w, avail.h};   // 无子：填满可用高度
		const float padY = ImGui::GetStyle().WindowPadding.y * 2.0f;   // child 上下内边距：不算则内容恒溢出 ~2*pad（永远可滚）
		const float spY = detail::spacingY(self.spacing);
		const float computed = inner.h + spY * (float)(cnt - 1) + padY;
		if(self.lastContent > 0.0f) return Size{w, std::min(self.lastContent, avail.h)};   // 上限帽：期望高不超过可用高（track 内滚动即可），固定格不被挤死
		return Size{w, computed};   // 首帧估计（内容计算高）
	}

	template<typename... Ts>
	void arrangeImpl(SubWindowData<Ts...>& self, Node& n, const Rect& r) {
		const float spY = detail::spacingY(self.spacing);
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h + spY;
		}
	}

	template<typename... Ts>
	void renderImpl(SubWindowData<Ts...>& self, Node& n) {
		const float hGuard = n.bounds.h > 1.0f ? n.bounds.h : ImGui::GetFrameHeight() * 2.0f;   // 0 高保底：防 BeginChild(h=0) 自动展开
		const ImVec2 sz(n.bounds.w, hGuard);
		if(ImGui::BeginChild(n.idname.empty() ? "child" : n.idname.c_str(), sz,
			   self.border ? ImGuiChildFlags_Border : 0)) {
			const float winY = ImGui::GetWindowPos().y;   // child 本地坐标基準（屏幕→本地）
			float realBottom = 0.0f;
			for(auto* c : n.children) {
				if(!c) continue;
				detail::drawChildLocal(n, *c);
				// 真实底边：最后一个提交 item 的屏幕底 - child 窗口顶（嵌套 Layout 恢复光标也不丢：取的是其内部最深 item）
				realBottom = std::max(realBottom, ImGui::GetItemRectMax().y - winY);
			}
			// 反馈：真实内容高 = 最深 item 底 + 底部内边距（增减双向跟踪，下一帧 measure 采用）
			// 复合控件（TabBar/Table 等）光标深于内部末 item：取两者较深者
			const float cursorBottom = ImGui::GetCursorPos().y - ImGui::GetStyle().ItemSpacing.y;   // 扣除尾随间距
			realBottom = std::max(realBottom, cursorBottom);
			self.lastContent = realBottom + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().WindowPadding.y;
		}
		ImGui::EndChild();
	}

	// ============ TabBar（标签页，仅激活页渲染）============

	template<typename... Ts>
	struct TabData {
		std::string name;
	};
	template<typename... Ts>
	using Tab = Control<TabData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(TabData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h = std::max(total.h, s.h);
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(TabData<Ts...>&, Node& n, const Rect& r) {
		for(auto* c : n.children) {
			if(!c) continue;
			c->arrange(r);
		}
	}
	template<typename... Ts>
	void renderImpl(TabData<Ts...>& self, Node& n) {
		(void)self;
		for(auto* c : n.children) {
			if(!c) continue;
			detail::drawChildFlow(*c);
		}
	}

	// 标签页：name 存于 node.id（Tab 构造后设置 tab.node.id = "页名"），
	// 便于 TabBar 渲染时经类型擦除直接读取
	template<typename... Ts>
	struct TabBarData {
	};
	template<typename... Ts>
	using TabBar = Control<TabBarData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(TabBarData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h = std::max(total.h, s.h);
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(TabBarData<Ts...>&, Node& n, const Rect& r) {
		for(auto* c : n.children) {
			if(!c) continue;
			c->arrange(r);
		}
	}
	template<typename... Ts>
	void renderImpl(TabBarData<Ts...>& self, Node& n) {
		if(!ImGui::BeginTabBar(n.idname.empty() ? "tabs" : n.idname.c_str())) return;
		for(auto* c : n.children) {
			if(!c) continue;
			if(ImGui::BeginTabItem(c->idname.empty() ? "tab" : c->idname.c_str())) {
				detail::drawChildFlow(*c);
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	// ============ 原子控件 ============
	// 每个原子 = 模板 struct（数据 + 功能函数），经 Control 组合。
	// 数据件含自持状态（字符串/值），功能函数含行为（render/measure/…）。
	// 布局：measureImpl 返回内容尺寸；arrangeImpl 记录 bounds。

	// —— Label（文本标签，持字符串副本）——
	template<std::size_t MaxLen = 128>
	struct LabelImpl {
		std::string name;

		void setName(const char* t) { name = t; }
		void setName(const std::string& t) { name = t; }
		const char* c_str() const { return name.c_str(); }

		void renderImpl(Node& n) { ImGui::TextUnformatted(name.c_str()); }
		Size measureImpl(Node& n, const Size& avail) {
			ImVec2 s = ImGui::CalcTextSize(name.c_str());
			return Size{s.x, s.y};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using Label = Control<LabelImpl<>, DefaultStyle>;

	// —— Button（按钮，点击回调）——
	struct ButtonImpl {
		std::string name;
		std::function<void()> onClick;

		void setName(const char* l) { name = l; }
		const char* c_str() const { return name.c_str(); }

		void renderImpl(Node& n) {
			if(ImGui::Button(name.c_str()) && onClick) onClick();
		}
		Size measureImpl(Node& n, const Size&) {
			ImVec2 s = ImGui::CalcTextSize(name.c_str());
			const ImVec2 fp = ImGui::GetStyle().FramePadding;
			return Size{s.x + fp.x * 2.0f, s.y + fp.y * 2.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using Button = Control<ButtonImpl, DefaultStyle>;

	// —— SliderF（滑块，绑定 float&——引用成员：控件经转发构造初始化，非 tuple 子）——
	struct SliderFImpl {
		std::string name;
		float* value = nullptr;   // 指针绑定（可默认构造，兼容 tuple 子控件）
		float mn = 0, mx = 1;

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 200.0f, detail::frameH()};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::SliderFloat(name.c_str(), value, mn, mx); }
		}
	};
	using SliderF = Control<SliderFImpl, DefaultStyle>;

	// —— CheckBox（复选框，绑定 bool*）——
	struct CheckBoxImpl {
		std::string name;
		bool* checked = nullptr;

		Size measureImpl(Node& n, const Size&) {
			return Size{ImGui::CalcTextSize(name.c_str()).x + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x, detail::frameH()};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(checked) ImGui::Checkbox(name.c_str(), checked);
		}
	};
	using CheckBox = Control<CheckBoxImpl, DefaultStyle>;

	// —— InputText（文本输入，模板参数 N = 缓冲容量）——
	template<std::size_t N = 64>
	struct InputTextImpl {
		std::string name;
		std::array<char, N> buf{};

		void setName(const char* t) { std::snprintf(buf.data(), N, "%s", t); }
		const char* c_str() const { return buf.data(); }

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 220.0f, detail::frameH()};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str()));
			ImGui::InputText(name.c_str(), buf.data(), N);
		}
	};
	template<std::size_t N = 64>
	using InputText = Control<InputTextImpl<N>, DefaultStyle>;

	// —— Combo（下拉，items 长期存活）——
	struct ComboImpl {
		std::string name;
		int* current = nullptr;
		const char* const* items = nullptr;
		int count = 0;

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 220.0f, detail::frameH()};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(current && items) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::Combo(name.c_str(), current, items, count); }
		}
	};
	using Combo = Control<ComboImpl, DefaultStyle>;

	// —— ProgressBar（进度条，绑定 float* fraction）——
	struct ProgressBarImpl {
		float* fraction = nullptr;
		std::string overlay;

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 200.0f, detail::frameH()};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(fraction) {
				ImGui::ProgressBar(*fraction, ImVec2(n.bounds.w, n.bounds.h),
					overlay.empty() ? nullptr : overlay.c_str());
			}
		}
	};
	using ProgressBar = Control<ProgressBarImpl, DefaultStyle>;

	// —— Tree（可折叠节点，展开才渲染子）——
	struct TreeImpl {
		std::string name;
		bool defaultOpen = false;

		void renderImpl(Node& n) {
			ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
			if(ImGui::TreeNodeEx(name.c_str(), flags)) {
				for(auto* c : n.children) {
					if(!c) continue;
					detail::drawChildAt(n, *c);
				}
				ImGui::TreePop();
			}
		}
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(avail);
				total.h += s.h;
			}
			return Size{avail.w, total.h + detail::headerH()};
		}
		void arrangeImpl(Node& n, const Rect& r) {
			float y = r.y + 20.0f;   // 标题行下
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(Size{r.w, r.h - (y - r.y)});
				c->arrange(Rect{r.x, y, s.w, s.h});
				y += s.h;
			}
		}
	};
	using Tree = Control<TreeImpl, DefaultStyle>;


	// ============ 输入类原子（批次A：fui 输入控件对象化）============
	// 统一约定：绑定用指针成员（可默认构造，兼容 tuple 子控件）；
	// 交互直接写回绑定的外部变量（与 fui 同语义）。

	// —— InputInt（整数输入，绑定 int*）——
	struct InputIntImpl {
		std::string name;
		int* value = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::InputInt(name.c_str(), value); } }
	};
	using InputInt = Control<InputIntImpl, DefaultStyle>;

	// —— InputFloat / InputDouble ——
	struct InputFloatImpl {
		std::string name;
		float* value = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::InputFloat(name.c_str(), value); } }
	};
	using InputFloat = Control<InputFloatImpl, DefaultStyle>;

	struct InputDoubleImpl {
		std::string name;
		double* value = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::InputDouble(name.c_str(), value); } }
	};
	using InputDouble = Control<InputDoubleImpl, DefaultStyle>;

	// —— vec2 输入（std::array<T,2>* 绑定）——
	struct InputInt2Impl {
		std::string name;
		std::array<int, 2>* value = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::InputInt2(name.c_str(), value->data()); } }
	};
	using InputInt2 = Control<InputInt2Impl, DefaultStyle>;

	struct InputFloat2Impl {
		std::string name;
		std::array<float, 2>* value = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::InputFloat2(name.c_str(), value->data()); } }
	};
	using InputFloat2 = Control<InputFloat2Impl, DefaultStyle>;

	// —— DragF / DragI（拖拽输入）——
	struct DragFImpl {
		std::string name;
		float* value = nullptr;
		float speed = 0.01f, mn = 0.0f, mx = 10.0f;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::DragFloat(name.c_str(), value, speed, mn, mx); } }
	};
	using DragF = Control<DragFImpl, DefaultStyle>;

	struct DragIImpl {
		std::string name;
		int* value = nullptr;
		float speed = 1.0f;
		int mn = 0, mx = 1000;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::DragInt(name.c_str(), value, speed, mn, mx); } }
	};
	using DragI = Control<DragIImpl, DefaultStyle>;

	// —— SliderInt / SliderAngle ——
	struct SliderIntImpl {
		std::string name;
		int* value = nullptr;
		int mn = 0, mx = 100;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(value) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::SliderInt(name.c_str(), value, mn, mx); } }
	};
	using SliderInt = Control<SliderIntImpl, DefaultStyle>;

	struct SliderAngleImpl {
		std::string name;
		float* valueRad = nullptr;
		float degMin = -360.0f, degMax = 360.0f;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(valueRad) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::SliderAngle(name.c_str(), valueRad, degMin, degMax); } }
	};
	using SliderAngle = Control<SliderAngleImpl, DefaultStyle>;

	// —— RadioButton / RadioGroup ——
	struct RadioButtonImpl {
		std::string name;
		bool* active = nullptr;
		Size measureImpl(Node&, const Size&) { return Size{ImGui::CalcTextSize(name.c_str()).x + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { if(active) ImGui::RadioButton(name.c_str(), active); }
	};
	using RadioButton = Control<RadioButtonImpl, DefaultStyle>;

	struct RadioGroupImpl {
		const char* const* items = nullptr;
		int count = 0;
		int* current = nullptr;
		Size measureImpl(Node&, const Size&) { return Size{160.0f, (float)count * detail::frameH() + (float)(count > 0 ? count - 1 : 0) * ImGui::GetStyle().ItemSpacing.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) {
			if(!items || !current) return;
			for(int i = 0; i < count; ++i) {
				bool sel = (i == *current);
				if(ImGui::RadioButton(items[i], &sel)) *current = i;
			}
		}
	};
	using RadioGroup = Control<RadioGroupImpl, DefaultStyle>;

	// —— ListBox（列表选择，items 长期存活契约）——
	struct ListBoxImpl {
		std::string name;
		const char* const* items = nullptr;
		int count = 0;
		int* current = nullptr;
		Size measureImpl(Node&, const Size&) { return Size{160.0f, (float)count * detail::frameH() + (float)(count > 0 ? count - 1 : 0) * ImGui::GetStyle().ItemSpacing.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) {
			if(items && current) ImGui::ListBox(name.c_str(), current, items, count);
		}
	};
	using ListBox = Control<ListBoxImpl, DefaultStyle>;


	// ============ 显示类原子（批次B：fui 显示控件对象化）============

	// —— TextColored / TextWrapped / BulletText / LabelText ——
	struct TextColoredImpl {
		std::string name;
		ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
		Size measureImpl(Node&, const Size&) { ImVec2 s = ImGui::CalcTextSize(name.c_str()); return Size{s.x, s.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { ImGui::TextColored(color, "%s", name.c_str()); }
	};
	using TextColored = Control<TextColoredImpl, DefaultStyle>;

	struct TextWrappedImpl {
		std::string name;
		Size measureImpl(Node&, const Size& avail) {
			ImVec2 s = ImGui::CalcTextSize(name.c_str(), nullptr, true, avail.w);   // 换行多行高（PushTextWrapPos 不影响 CalcTextSize）
			return Size{avail.w, s.y};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + n.bounds.w);   // wrap 宽 = 格子宽，与 measure 一致
			ImGui::Text("%s", name.c_str());
			ImGui::PopTextWrapPos();
		}
	};
	using TextWrapped = Control<TextWrappedImpl, DefaultStyle>;

	struct BulletTextImpl {
		std::string name;
		Size measureImpl(Node&, const Size&) { ImVec2 s = ImGui::CalcTextSize(name.c_str()); return Size{s.x + 16.0f, s.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { ImGui::BulletText("%s", name.c_str()); }
	};
	using BulletText = Control<BulletTextImpl, DefaultStyle>;

	struct LabelTextImpl {
		std::string name;    // 左侧项名
		std::string value;   // 右侧值文本
		Size measureImpl(Node&, const Size& avail) {
			(void)avail;
			const ImVec2 v = ImGui::CalcTextSize(value.c_str());
			const float nameW = ImGui::CalcTextSize(name.c_str()).x;
			return Size{v.x + nameW + 8.0f, v.y};   // value + 8 间距 + name；高 = 文本行高
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			ImGui::TextUnformatted(value.c_str());
			ImGui::SameLine(0.0f, 8.0f);
			ImGui::TextDisabled("%s", name.c_str());
		}
	};
	using LabelText = Control<LabelTextImpl, DefaultStyle>;

	// —— Selectable（可选行，bool* 绑定）——
	struct SelectableImpl {
		std::string name;
		bool* selected = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 160.0f, ImGui::CalcTextSize(name.c_str()).y + ImGui::GetStyle().FramePadding.y * 2.0f}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(selected) {
				const ImVec2 sp = ImGui::GetStyle().ItemSpacing;
				const float sL = std::trunc(sp.x * 0.5f);
				const float sU = std::trunc(sp.y * 0.5f);
				ImGui::SetCursorPos(ImVec2(n.origin.x + sL, n.origin.y + sU));
				ImGui::Selectable(name.c_str(), selected, 0, ImVec2(n.bounds.w - sp.x, n.bounds.h - sp.y));
			}
		}
	};
	using Selectable = Control<SelectableImpl, DefaultStyle>;

	// —— Separator / SeparatorText ——
	struct SeparatorImpl {
		Size measureImpl(Node&, const Size&) { return Size{0.0f, 12.0f}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { ImGui::Separator(); }
	};
	using Separator = Control<SeparatorImpl, DefaultStyle>;

	struct SeparatorTextImpl {
		std::string name;
		Size measureImpl(Node&, const Size&) { return Size{0.0f, detail::headerH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { ImGui::SeparatorText(name.c_str()); }
	};
	using SeparatorText = Control<SeparatorTextImpl, DefaultStyle>;

	// —— Image / ImageButton（ImTextureID + size）——
	struct ImageImpl {
		ImTextureID tex{};
		ImVec2 size{64, 64};
		Size measureImpl(Node&, const Size&) { return Size{size.x, size.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			ImGui::SetCursorPos(ImVec2(n.bounds.x, n.bounds.y));
			ImGui::Image(tex, size);
		}
	};
	using Image = Control<ImageImpl, DefaultStyle>;

	struct ImageButtonImpl {
		ImTextureID tex{};
		ImVec2 size{64, 64};
		std::function<void()> onClick;
		Size measureImpl(Node&, const Size&) { return Size{size.x, size.y}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(ImGui::ImageButton(n.idname.empty() ? "imgbtn" : n.idname.c_str(), tex, size) && onClick) onClick();
		}
	};
	using ImageButton = Control<ImageButtonImpl, DefaultStyle>;

	// —— PlotLines（折线，绑定 const vector<float>*）——
	struct PlotLinesImpl {
		std::string name;
		const std::vector<float>* values = nullptr;
		ImVec2 size{0.0f, 60.0f};
		Size measureImpl(Node&, const Size& avail) {
			return Size{size.x > 0 ? size.x : avail.w, size.y};
		}
		void renderImpl(Node& n) {
			if(values && !values->empty()) {
				// 折线 frame 宽扣除右侧标签：总宽（frame+label）恰为格子宽
				ImGui::PlotLines(name.c_str(), values->data(), (int)values->size(),
					0, nullptr, FLT_MAX, FLT_MAX,
					ImVec2(detail::frameWidthFor(n.bounds.w, name.c_str()), n.bounds.h));
			}
		}

	};
	using PlotLines = Control<PlotLinesImpl, DefaultStyle>;


	// ============ 容器类（批次C：fui 容器控件对象化）============

	// —— Collapsing（可折叠容器，展开才渲染子；flags 传 DefaultOpen）——
	struct CollapsingImpl {
		std::string name;
		ImGuiTreeNodeFlags flags = 0;
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(avail);
				total.h += s.h;
			}
			return Size{avail.w, total.h + detail::headerH()};
		}
		void arrangeImpl(Node& n, const Rect& r) {
			float y = r.y + detail::headerH();
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(Size{r.w, r.h - (y - r.y)});
				c->arrange(Rect{r.x, y, s.w, s.h});
				y += s.h;
			}
		}
		void renderImpl(Node& n) {
			if(ImGui::CollapsingHeader(name.c_str(), flags)) {
				for(auto* c : n.children) {
					if(!c) continue;
					detail::drawChildAt(n, *c);
				}
			}
		}
	};
	using Collapsing = Control<CollapsingImpl, DefaultStyle>;

	// —— Panel（浮动面板：宿主内可拖动，ImGui::Begin/End）——
	template<typename... Ts>
	struct PanelImpl;
	template<typename... Ts>
	struct PanelData {
		std::string title;
	};
	template<typename... Ts>
	using Panel = Control<PanelData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(PanelData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h += s.h;
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(PanelData<Ts...>&, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h;
		}
	}
	template<typename... Ts>
	void renderImpl(PanelData<Ts...>& self, Node& n) {
		ImGui::Begin(self.title.empty() ? (n.idname.empty() ? "panel" : n.idname.c_str()) : self.title.c_str(), nullptr, 0);
		for(auto* c : n.children) {
			if(!c) continue;
			ImGui::SetCursorPos(ImVec2(c->bounds.x, c->bounds.y));
			c->draw();
		}
		ImGui::End();
	}

	// —— MenuBar / Menu / MenuItem（菜单栏，放树首；标签名存 node.id）——
	struct MenuItemImpl {
		std::string shortcut;   // 快捷键显示（如 "Ctrl+O"；空 = 不显示）
		std::string name;
		bool selected = false;  // 勾选态（如当前主题项；应用层每帧刷新）
		std::function<void()> onClick;
		void renderImpl(Node&) {
			if(ImGui::MenuItem(name.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), selected) && onClick) onClick();
		}
		Size measureImpl(Node&, const Size&) { return Size{ImGui::CalcTextSize(name.c_str()).x + 40.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using MenuItem = Control<MenuItemImpl, DefaultStyle>;

	template<typename... Ts>
	struct MenuImpl;
	template<typename... Ts>
	struct MenuData {
		std::string name;
	};
	template<typename... Ts>
	using Menu = Control<MenuData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(MenuData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.h += s.h;
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(MenuData<Ts...>&, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h;
		}
	}
	template<typename... Ts>
	void renderImpl(MenuData<Ts...>& self, Node& n) {
		if(ImGui::BeginMenu(self.name.c_str())) {
			for(auto* c : n.children) {
	if(!c) continue;
	detail::drawChildFlow(*c);
	}
			ImGui::EndMenu();
		}
	}

	template<typename... Ts>
	struct MenuBarImpl;
	template<typename... Ts>
	struct MenuBarData {};
	template<typename... Ts>
	using MenuBar = Control<MenuBarData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(MenuBarData<Ts...>&, Node&, const Size&) { return Size{0.0f, ImGui::GetFrameHeight() + 2.0f}; }
	template<typename... Ts>
	void arrangeImpl(MenuBarData<Ts...>&, Node&, const Rect&) {}
	template<typename... Ts>
	void renderImpl(MenuBarData<Ts...>&, Node& n) {
		if(ImGui::BeginMainMenuBar()) {
			for(auto* c : n.children) {
	if(!c) continue;
	detail::drawChildFlow(*c);
	}
			ImGui::EndMainMenuBar();
		}
		// 菜单栏是浮顶独立窗口：宿主光标下移，防重叠
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetFrameHeight());
	}

	// —— TextTable（数据驱动表格，rows 绑定外部 vector）——
	struct TextTableImpl {
		const char* const* headers = nullptr;
		int headerCount = 0;
		const std::vector<std::vector<std::string>>* rows = nullptr;
		Size measureImpl(Node&, const Size& avail) {
			const int n = rows ? (int)rows->size() : 0;
			return Size{avail.w, (float)(n + 1) * (detail::lineH() + 4.0f) + 8.0f};   // 表头+数据行
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(!headers || !rows) return;
			if(!ImGui::BeginTable(n.idname.empty() ? "table" : n.idname.c_str(), headerCount, ImGuiTableFlags_Borders)) return;
			for(int i = 0; i < headerCount; ++i) ImGui::TableSetupColumn(headers[i]);
			ImGui::TableHeadersRow();
			for(const auto& row : *rows) {
				ImGui::TableNextRow();
				for(int i = 0; i < headerCount; ++i) {
					ImGui::TableNextColumn();
					if((int)row.size() > i) ImGui::TextUnformatted(row[i].c_str());
				}
			}
			ImGui::EndTable();
		}
	};
	using TextTable = Control<TextTableImpl, DefaultStyle>;

	// —— FnTable（泛型表格，cellFn(row,col) 返回字符串）——
	struct FnTableImpl {
		const char* const* headers = nullptr;
		int headerCount = 0;
		int rowCount = 0;
		std::function<std::string(int, int)> cell;
		Size measureImpl(Node&, const Size& avail) {
			return Size{avail.w, (float)(rowCount + 1) * (detail::lineH() + 4.0f) + 8.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(!headers || !cell) return;
			if(!ImGui::BeginTable(n.idname.empty() ? "table" : n.idname.c_str(), headerCount, ImGuiTableFlags_Borders)) return;
			for(int i = 0; i < headerCount; ++i) ImGui::TableSetupColumn(headers[i]);
			ImGui::TableHeadersRow();
			for(int r = 0; r < rowCount; ++r) {
				ImGui::TableNextRow();
				for(int c = 0; c < headerCount; ++c) {
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(cell(r, c).c_str());
				}
			}
			ImGui::EndTable();
		}
	};
	using FnTable = Control<FnTableImpl, DefaultStyle>;


	// ============ 弹窗与颜色（批次D：fui popups 对象化）============
	// 标志触发式：open 置位当帧 OpenPopup 后自动回落；内容每帧渲染
	// 直到子回调 CloseCurrentPopup()

	template<typename... Ts>
	struct PopupImpl;
	template<typename... Ts>
	struct PopupData {
		bool* open = nullptr;
	};
	template<typename... Ts>
	using Popup = Control<PopupData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(PopupData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h += s.h;
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(PopupData<Ts...>&, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h;
		}
	}
	template<typename... Ts>
	void renderImpl(PopupData<Ts...>& self, Node& n) {
		if(self.open && *self.open) {
			ImGui::OpenPopup(n.idname.empty() ? "popup" : n.idname.c_str());
			*self.open = false;
		}
		if(ImGui::BeginPopup(n.idname.empty() ? "popup" : n.idname.c_str())) {
			for(auto* c : n.children) {
	if(!c) continue;
	detail::drawChildFlow(*c);
	}
			ImGui::EndPopup();
		}
	}

	template<typename... Ts>
	struct PopupModalImpl;
	template<typename... Ts>
	struct PopupModalData {
		bool* open = nullptr;
		ImGuiWindowFlags flags = 0;
	};
	template<typename... Ts>
	using PopupModal = Control<PopupModalData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(PopupModalData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.h += s.h;
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(PopupModalData<Ts...>&, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h;
		}
	}
	template<typename... Ts>
	void renderImpl(PopupModalData<Ts...>& self, Node& n) {
		if(self.open && *self.open) {
			ImGui::OpenPopup(n.idname.empty() ? "popup" : n.idname.c_str());
			*self.open = false;
		}
		if(ImGui::BeginPopupModal(n.idname.empty() ? "modal" : n.idname.c_str(), nullptr, self.flags)) {
			for(auto* c : n.children) {
	if(!c) continue;
	detail::drawChildFlow(*c);
	}
			ImGui::EndPopup();
		}
	}

	// —— 颜色编辑/拾取器（std::array<float,N>* 绑定）——
	struct ColorEdit3Impl {
		std::string name;
		std::array<float, 3>* color = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 260.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(color) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::ColorEdit3(name.c_str(), color->data()); } }
	};
	using ColorEdit3 = Control<ColorEdit3Impl, DefaultStyle>;

	struct ColorEdit4Impl {
		std::string name;
		std::array<float, 4>* color = nullptr;
		Size measureImpl(Node&, const Size& avail) { return Size{avail.w > 0 ? avail.w : 260.0f, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(color) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::ColorEdit4(name.c_str(), color->data()); } }
	};
	using ColorEdit4 = Control<ColorEdit4Impl, DefaultStyle>;

	struct ColorPicker3Impl {
		std::string name;
		std::array<float, 3>* color = nullptr;
		Size measureImpl(Node&, const Size&) { return Size{260.0f, 320.0f}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(color) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::ColorPicker3(name.c_str(), color->data()); } }
	};
	using ColorPicker3 = Control<ColorPicker3Impl, DefaultStyle>;

	struct ColorPicker4Impl {
		std::string name;
		std::array<float, 4>* color = nullptr;
		Size measureImpl(Node&, const Size&) { return Size{260.0f, 320.0f}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) { if(color) { ImGui::SetNextItemWidth(detail::frameWidthFor(n.bounds.w, name.c_str())); ImGui::ColorPicker4(name.c_str(), color->data()); } }
	};
	using ColorPicker4 = Control<ColorPicker4Impl, DefaultStyle>;

	// —— CheckboxFlags（位标志，int* + mask）——
	struct CheckboxFlagsImpl {
		std::string name;
		int* flags = nullptr;
		int mask = 1;
		Size measureImpl(Node&, const Size&) { return Size{ImGui::CalcTextSize(name.c_str()).x + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x, detail::frameH()}; }
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node&) { if(flags) ImGui::CheckboxFlags(name.c_str(), flags, mask); }
	};
	using CheckboxFlags = Control<CheckboxFlagsImpl, DefaultStyle>;

	// ============ 装饰件（批次D：fui 修饰器对象化）============
	// 装饰件 = 包装子节点的控件（子经运行时 add 挂入）

	// —— Tooltip（悬停提示）——
	struct TooltipImpl {
		std::string name;
		void renderImpl(Node& n) {
			for(auto* c : n.children) { if(c) detail::drawChildAt(n, *c); }
			if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s", name.c_str());
		}
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) { if(c) { Size s = c->measure(avail); total = s; } }
			return total;
		}
		void arrangeImpl(Node& n, const Rect& r) {
			for(auto* c : n.children) { if(c) c->arrange(r); }
		}
	};
	using Tooltip = Control<TooltipImpl, DefaultStyle>;

	// —— Colored（颜色作用域包裹）——
	struct ColoredImpl {
		ImGuiCol idx = ImGuiCol_Text;
		ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
		void renderImpl(Node& n) {
			ImGui::PushStyleColor(idx, color);
			for(auto* c : n.children) { if(c) detail::drawChildAt(n, *c); }
			ImGui::PopStyleColor();
		}
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) { if(c) { Size s = c->measure(avail); total = s; } }
			return total;
		}
		void arrangeImpl(Node& n, const Rect& r) {
			for(auto* c : n.children) { if(c) c->arrange(r); }
		}
	};
	using Colored = Control<ColoredImpl, DefaultStyle>;

	// —— Indent（缩进包裹）——
	struct IndentImpl {
		float amount = 20.0f;
		void renderImpl(Node& n) {
			for(auto* c : n.children) { if(c) detail::drawChildAt(n, *c); }
		}
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) { if(c) { Size s = c->measure(avail); total = s; } }
			return total;
		}
		void arrangeImpl(Node& n, const Rect& r) {
			for(auto* c : n.children) { if(c) c->arrange(Rect{r.x + amount, r.y, r.w - amount, r.h}); }   // 缩进
		}
	};
	using Indent = Control<IndentImpl, DefaultStyle>;

	// ============ 装饰件（EllipsisLabel / ItemWidth / When / Styled）============

	// —— EllipsisLabel：超宽省略号标签（超出 bounds 截断加 "..."）——
	struct EllipsisLabelImpl {
		std::string name;
		void renderImpl(Node& n) {
			ImGui::TextUnformatted(detail::ellipsize(name,
				std::max(n.bounds.w, 0.0f)).c_str());
		}
		Size measureImpl(Node&, const Size& avail) {
			ImVec2 s = ImGui::CalcTextSize(name.c_str());
			return Size{avail.w > 0 ? avail.w : s.x, s.y};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using EllipsisLabel = Control<EllipsisLabelImpl, DefaultStyle>;

	// —— ItemWidth：项宽作用域包裹（PushItemWidth 覆盖子树）——
	template<typename T>
	struct ItemWidthData {
		float width = 0.0f;
	};
	template<typename T>
	using ItemWidth = Control<ItemWidthData<T>, DefaultStyle, T>;

	template<typename T>
	Size measureImpl(ItemWidthData<T>&, Node& n, const Size& avail) {
		Size m{0, 0};
		for(auto* c : n.children) if(c) m = c->measure(avail);
		return m;
	}
	template<typename T>
	void arrangeImpl(ItemWidthData<T>& self, Node& n, const Rect& r) {
	for(auto* c : n.children) if(c) c->arrange(Rect{r.x, r.y, self.width > 0.0f ? std::min(r.w, self.width) : r.w, r.h});   // 限宽
	}
	template<typename T>
	void renderImpl(ItemWidthData<T>& self, Node& n) {
		for(auto* c : n.children) if(c) detail::drawChildAt(n, *c);
	}

	// —— When：条件包裹（条件假时子树不渲染、不占布局）——
	template<typename T>
	struct WhenData {
		bool show = true;
	};
	template<typename T>
	using When = Control<WhenData<T>, DefaultStyle, T>;

	template<typename T>
	Size measureImpl(WhenData<T>& self, Node& n, const Size& avail) {
		if(!self.show) return Size{0.0f, 0.0f};
		Size m{0, 0};
		for(auto* c : n.children) if(c) m = c->measure(avail);
		return m;
	}
	template<typename T>
	void arrangeImpl(WhenData<T>& self, Node& n, const Rect& r) {
		if(!self.show) return;   // 隐藏时不布局子节点（避免 0 尺寸布局传给子容器）
		for(auto* c : n.children) if(c) c->arrange(r);
	}
	template<typename T>
	void renderImpl(WhenData<T>& self, Node& n) {
		if(!self.show) return;
	for(auto* c : n.children) if(c) detail::drawChildAt(n, *c);
	}

	// —— Styled：主题作用域换肤（Push 全部主题色 + 几何，子树渲染后配对 Pop）——
	template<typename T>
	struct StyledData {
		Theme theme;
	};
	template<typename T>
	using Styled = Control<StyledData<T>, DefaultStyle, T>;

	template<typename T>
	Size measureImpl(StyledData<T>&, Node& n, const Size& avail) {
		Size m{0, 0};
		for(auto* c : n.children) if(c) m = c->measure(avail);
		return m;
	}
	template<typename T>
	void arrangeImpl(StyledData<T>&, Node& n, const Rect& r) {
		for(auto* c : n.children) if(c) c->arrange(r);
	}
	template<typename T>
	void renderImpl(StyledData<T>& self, Node& n) {
		detail::pushTheme(self.theme);
	for(auto* c : n.children) if(c) detail::drawChildAt(n, *c);
		ImGui::PopStyleColor(26);
		ImGui::PopStyleVar(3);
	}
}





