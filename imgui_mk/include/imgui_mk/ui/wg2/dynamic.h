#pragma once
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 动态层：只有真正需要运行时增删/查找的控件才使用。
//
// AnyWidget = 拥有具体静态控件的类型擦除包装。
// RuntimeList = 动态垂直列表，具备：
//   · 渲染期 add 延迟（pending）
//   · remove 墓碑（不搬移正在遍历的 vector）
//   · findById 运行时查找
// 普通静态控件不携带这些能力。

namespace Mk::wg2 {

class AnyWidget {
public:
	using HintFn = Size(*)(void*, const Size&);
	using PolicyFn = Policy(*)(void*);
	using DrawFn = void(*)(void*, const Rect&);
	using DestroyFn = void(*)(void*);

	AnyWidget() = default;
	AnyWidget(const AnyWidget&) = delete;
	AnyWidget& operator=(const AnyWidget&) = delete;

	AnyWidget(AnyWidget&& other) noexcept
		: obj_(std::exchange(other.obj_, nullptr)),
		  innerType_(std::exchange(other.innerType_, nullptr)),
		  hintFn_(std::exchange(other.hintFn_, nullptr)),
		  policyFn_(std::exchange(other.policyFn_, nullptr)),
		  drawFn_(std::exchange(other.drawFn_, nullptr)),
		  destroyFn_(std::exchange(other.destroyFn_, nullptr)),
		  idname(std::move(other.idname)) {
		// 状态随对象迁移：旧地址不留残余
		detail::moveChrome(&other, &typeid(AnyWidget), this, &typeid(AnyWidget));
	}

	AnyWidget& operator=(AnyWidget&& other) noexcept {
		if (this != &other) {
			reset();
			obj_ = std::exchange(other.obj_, nullptr);
			innerType_ = std::exchange(other.innerType_, nullptr);
			hintFn_ = std::exchange(other.hintFn_, nullptr);
			policyFn_ = std::exchange(other.policyFn_, nullptr);
			drawFn_ = std::exchange(other.drawFn_, nullptr);
			destroyFn_ = std::exchange(other.destroyFn_, nullptr);
			idname = std::move(other.idname);
			detail::moveChrome(&other, &typeid(AnyWidget), this, &typeid(AnyWidget));
		}
		return *this;
	}

	~AnyWidget() { reset(); }

	template<StaticWidget W>
	static AnyWidget own(std::unique_ptr<W> w) {
		AnyWidget a;
		if (!w) return a;
		a.obj_ = w.release();
		a.innerType_ = &typeid(W);   // 内部控件的状态回收用
		a.hintFn_ = [](void* p, const Size& s) -> Size {
			return hintOf(*static_cast<const W*>(p), s);
		};
		a.policyFn_ = [](void* p) -> Policy {
			return policyOf(*static_cast<const W*>(p));
		};
		a.drawFn_ = [](void* p, const Rect& r) {
			drawWidget(*static_cast<W*>(p), r);
		};
		a.destroyFn_ = [](void* p) {
			delete static_cast<W*>(p);
		};
		return a;
	}

	bool valid() const { return obj_ != nullptr; }

	Size hint(const Size& avail) const {
		return hintFn_ ? hintFn_(obj_, avail) : Size{0.0f, 0.0f};
	}

	Policy policy() const {
		return policyFn_ ? policyFn_(obj_) : Policy{};
	}

	void draw(const Rect& r) const {
		if (drawFn_) drawFn_(obj_, r);
	}

	std::string idname;

private:
	void reset() {
		if (obj_) {
			if (innerType_) detail::chromeStore().erase(detail::ChromeKey{obj_, innerType_});   // 内部控件状态回收
			if (destroyFn_) destroyFn_(obj_);
		}
		clearChromeFor(*this);   // 自身状态回收
		obj_ = nullptr;
		innerType_ = nullptr;
		hintFn_ = nullptr;
		policyFn_ = nullptr;
		drawFn_ = nullptr;
		destroyFn_ = nullptr;
	}

	void* obj_ = nullptr;
	const std::type_info* innerType_ = nullptr;
	HintFn hintFn_ = nullptr;
	PolicyFn policyFn_ = nullptr;
	DrawFn drawFn_ = nullptr;
	DestroyFn destroyFn_ = nullptr;
};

template<FixedString Id = FixedString<1>{}>
class RuntimeList {
public:
	static constexpr auto id = Id;

	std::string idname;

	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	// —— 运行时增删（渲染回调内安全）——
	void add(AnyWidget w) {
		if (!w.valid()) return;
		if (g_RenderDepth > 0) {
			pending_.push_back(std::move(w));
			return;
		}
		items_.push_back(std::make_unique<AnyWidget>(std::move(w)));
	}

	void removeAt(std::size_t index) {
		if (index < items_.size()) {
			items_[index].reset();   // 墓碑：不搬移后续元素
		}
	}

	void removeLast() {
		if (!items_.empty()) removeAt(items_.size() - 1);
	}

	void clear() {
		for (auto& slot : items_) slot.reset();
		items_.clear();
		pending_.clear();
	}

	void compact() {
		items_.erase(
			std::remove(items_.begin(), items_.end(), nullptr),
			items_.end());
	}

	AnyWidget* findById(std::string_view want) {
		if (g_RenderDepth == 0) compact();
		for (auto& slot : items_) {
			if (slot && slot->idname == want) return slot.get();
		}
		return nullptr;
	}

	const AnyWidget* findById(std::string_view want) const {
		for (auto& slot : items_) {
			if (slot && slot->idname == want) return slot.get();
		}
		return nullptr;
	}

	std::size_t size() const {
		std::size_t n = 0;
		for (auto& slot : items_) if (slot) ++n;
		return n + pending_.size();
	}

	Size hint(const Size& avail) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		flushPending();
		const float mx = detailMarginX(), my = detailMarginY();
		const float innerW = std::max(0.0f, avail.w - mx * 2.0f);

		float h = 0.0f, w = 0.0f;
		std::size_t n = 0;
		for (auto& slot : items_) {
			if (!slot) continue;
			const Size s = slot->hint(Size{innerW, 0.0f});
			h += s.h;
			w = std::max(w, s.w);
			++n;
		}
		for (auto& p : pending_) {
			const Size s = p.hint(Size{innerW, 0.0f});
			h += s.h;
			w = std::max(w, s.w);
			++n;
		}
		const float sp = detailSpacingY();
		if (n > 1) h += sp * static_cast<float>(n - 1);
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		++g_RenderDepth;
		flushPending();

		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detailMarginX(), my = detailMarginY();
		const float sp = detailSpacingY();
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);

		float y = r.y + my;
		for (auto& slot : items_) {
			if (!slot) continue;
			const Size hs = slot->hint(Size{innerW, 0.0f});
			const Rect cr{r.x + mx, y, innerW, hs.h};
			slot->draw(cr);
			y += hs.h + sp;
		}

		ImGui::SetCursorPos(saved);
		--g_RenderDepth;
	}

private:
	void flushPending() const {
		for (auto& p : pending_) {
			if (p.valid()) items_.push_back(std::make_unique<AnyWidget>(std::move(p)));
		}
		pending_.clear();
	}

	float detailSpacingY() const { return spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.y; }
	float detailMarginX() const { return marginX >= 0.0f ? marginX : ImGui::GetStyle().WindowPadding.x; }
	float detailMarginY() const { return marginY >= 0.0f ? marginY : ImGui::GetStyle().WindowPadding.y; }

	mutable std::vector<std::unique_ptr<AnyWidget>> items_;
	mutable std::vector<AnyWidget> pending_;
};

// ============ RuntimeTabs：动态页签容器 ============

template<FixedString Id = FixedString<1>{}>
class RuntimeTabs {
public:
	static constexpr auto id = Id;

	std::string idname;

	void add(std::string title, AnyWidget w) {
		if (!w.valid()) return;
		Entry e;
		e.title = std::move(title);
		e.widget = std::move(w);
		if (g_RenderDepth > 0) {
			pending_.push_back(std::move(e));
			return;
		}
		items_.push_back(std::make_unique<Entry>(std::move(e)));
	}

	void removeAt(std::size_t index) {
		if (index < items_.size()) items_[index].reset();
	}

	void removeLast() {
		if (!items_.empty()) removeAt(items_.size() - 1);
	}

	void clear() {
		items_.clear();
		pending_.clear();
	}

	void compact() {
		items_.erase(std::remove(items_.begin(), items_.end(), nullptr), items_.end());
	}

	AnyWidget* findById(std::string_view want) {
		if (g_RenderDepth == 0) compact();
		for (auto& slot : items_) {
			if (slot && (slot->widget.idname == want || slot->title == want)) return &slot->widget;
		}
		return nullptr;
	}

	std::size_t size() const {
		std::size_t n = 0;
		for (auto& slot : items_) if (slot) ++n;
		return n + pending_.size();
	}

	Size hint(const Size& avail) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		flushPending();
		float w = 0.0f, h = 0.0f;
		for (auto& slot : items_) {
			if (!slot) continue;
			const Size s = slot->widget.hint(avail);
			w = std::max(w, s.w);
			h = std::max(h, s.h);
		}
		for (auto& e : pending_) {
			const Size s = e.widget.hint(avail);
			w = std::max(w, s.w);
			h = std::max(h, s.h);
		}
		return Size{w, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		++g_RenderDepth;
		flushPending();
		const char* tid = idname.empty() ? id.data : idname.c_str();
		if (ImGui::BeginTabBar(tid)) {
			for (auto& slot : items_) {
				if (!slot) continue;
				// 页签按标题自然宽（默认会拉满整条）
				ImGui::SetNextItemWidth(ImGui::CalcTextSize(slot->title.c_str()).x
					+ ImGui::GetStyle().FramePadding.x * 2.0f + 12.0f);
				if (ImGui::BeginTabItem(slot->title.c_str())) {
					// 用页签条之后的当前光标作为内容起点（直接传页签栏矩形会画重叠）
					const ImVec2 p = ImGui::GetCursorPos();
					const ImVec2 av = ImGui::GetContentRegionAvail();
					slot->widget.draw(Rect{p.x, p.y,
						std::max(0.0f, av.x), std::max(0.0f, av.y)});
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
		--g_RenderDepth;
	}

private:
	struct Entry {
		std::string title;
		AnyWidget widget;
	};

	void flushPending() const {
		for (auto& e : pending_) {
			if (e.widget.valid()) items_.push_back(std::make_unique<Entry>(std::move(e)));
		}
		pending_.clear();
	}

	mutable std::vector<std::unique_ptr<Entry>> items_;
	mutable std::vector<Entry> pending_;
};

// ============ RuntimeGrid：动态网格容器 ============

template<FixedString Id = FixedString<1>{}>
class RuntimeGrid {
public:
	static constexpr auto id = Id;

	std::string idname;

	int cols = 3;
	float spacing = -1.0f;
	float marginX = -1.0f, marginY = -1.0f;

	void add(AnyWidget w) {
		if (!w.valid()) return;
		if (g_RenderDepth > 0) {
			pending_.push_back(std::move(w));
			return;
		}
		items_.push_back(std::make_unique<AnyWidget>(std::move(w)));
	}

	void removeAt(std::size_t index) {
		if (index < items_.size()) items_[index].reset();
	}

	void removeLast() {
		if (!items_.empty()) removeAt(items_.size() - 1);
	}

	void clear() {
		items_.clear();
		pending_.clear();
	}

	void compact() {
		items_.erase(std::remove(items_.begin(), items_.end(), nullptr), items_.end());
	}

	AnyWidget* findById(std::string_view want) {
		if (g_RenderDepth == 0) compact();
		for (auto& slot : items_) {
			if (slot && slot->idname == want) return slot.get();
		}
		return nullptr;
	}

	std::size_t size() const {
		std::size_t n = 0;
		for (auto& slot : items_) if (slot) ++n;
		return n + pending_.size();
	}

	Size hint(const Size& avail) const {
		if (!widgetChrome(*this).visible) return Size{0.0f, 0.0f};
		flushPending();
		const float mx = detailMarginX(), my = detailMarginY();
		const float spx = detailSpacingX(), spy = detailSpacingY();
		const int c = std::max(1, cols);

		std::size_t n = 0;
		float maxW = 0.0f;
		std::vector<float> rowH;
		float curRowH = 0.0f;

		auto feed = [&](const AnyWidget& w) {
			const Size s = w.hint(Size{avail.w > 0.0f ? avail.w : 0.0f, 0.0f});
			maxW = std::max(maxW, s.w);
			curRowH = std::max(curRowH, s.h);
			if (++n % static_cast<std::size_t>(c) == 0) {
				rowH.push_back(curRowH);
				curRowH = 0.0f;
			}
		};

		for (auto& slot : items_) if (slot) feed(*slot);
		for (auto& p : pending_) feed(p);

		if (n % static_cast<std::size_t>(c) != 0) rowH.push_back(curRowH);

		float h = 0.0f;
		for (float v : rowH) h += v;
		if (rowH.size() > 1) h += spy * static_cast<float>(rowH.size() - 1);

		const std::size_t firstRow = std::min<std::size_t>(n, static_cast<std::size_t>(c));
		const float w = firstRow == 0
			? 0.0f
			: maxW * static_cast<float>(firstRow) + spx * static_cast<float>(firstRow - 1);
		return Size{w + mx * 2.0f, h + my * 2.0f};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		++g_RenderDepth;
		flushPending();

		const ImVec2 saved = ImGui::GetCursorPos();
		const float mx = detailMarginX(), my = detailMarginY();
		const float spx = detailSpacingX(), spy = detailSpacingY();
		const float innerW = std::max(0.0f, r.w - mx * 2.0f);
		const int c = std::max(1, cols);
		const float colW = innerW > 0.0f
			? (innerW - spx * static_cast<float>(c - 1)) / static_cast<float>(c)
			: 0.0f;

		std::size_t ri = 0, ci = 0, seen = 0;
		std::vector<float> rowHeights;
		float rowH = 0.0f;
		for (auto& slot : items_) {
			if (!slot) continue;
			const Size s = slot->hint(Size{colW, 0.0f});
			rowH = std::max(rowH, s.h);
			++seen;
			if (++ci >= static_cast<std::size_t>(c)) {
				rowHeights.push_back(rowH);
				rowH = 0.0f;
				ci = 0;
			}
		}
		if (ci != 0 && seen != 0) rowHeights.push_back(rowH);

		ci = 0;
		ri = 0;
		for (auto& slot : items_) {
			if (!slot) continue;
			const Size s = slot->hint(Size{colW, 0.0f});
			const float h = rowHeights[ri];
			const float x = r.x + mx + static_cast<float>(ci) * (colW + spx);
			const float y = r.y + my;
			float yy = y;
			for (std::size_t i = 0; i < ri; ++i) yy += rowHeights[i] + spy;
			slot->draw(Rect{x, yy, colW, h});
			if (++ci >= static_cast<std::size_t>(c)) {
				ci = 0;
				++ri;
			}
		}

		ImGui::SetCursorPos(saved);
		--g_RenderDepth;
	}

private:
	void flushPending() const {
		for (auto& p : pending_) {
			if (p.valid()) items_.push_back(std::make_unique<AnyWidget>(std::move(p)));
		}
		pending_.clear();
	}

	float detailSpacingX() const { return spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.x; }
	float detailSpacingY() const { return spacing >= 0.0f ? spacing : ImGui::GetStyle().ItemSpacing.y; }
	float detailMarginX() const { return marginX >= 0.0f ? marginX : ImGui::GetStyle().WindowPadding.x; }
	float detailMarginY() const { return marginY >= 0.0f ? marginY : ImGui::GetStyle().WindowPadding.y; }

	mutable std::vector<std::unique_ptr<AnyWidget>> items_;
	mutable std::vector<AnyWidget> pending_;
};

} // namespace Mk::wg2
