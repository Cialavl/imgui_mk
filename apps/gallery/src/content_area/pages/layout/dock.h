#pragma once
#include "../../demo_page.h"

namespace gallery {

// 停靠布局：分割树 + 页签组 + 拖拽停靠（DockSpace）
struct PageDock {
	static constexpr const char* title = "停靠布局";
	static constexpr const char* desc  = "DockSpace：分割树 + 页签组；拖页签合并、拖四边停靠、拖分隔条调比例。";
	static constexpr std::string_view code = R"(
Mk::wg2::DockSpace dock;
int viewport = dock.addPanel("视口", [](const Rect& r){ ... });
int scene    = dock.addPanel("层级", [](const Rect& r){ ... });
int props    = dock.addPanel("属性", [](const Rect& r){ ... });
dock.dockPanel(scene, dock.root(), DockEdge::Left);
dock.dockPanel(props, dock.leafOf(viewport), DockEdge::Right);
)";

	// 固定尺寸宿主：演示区域里给停靠区一个确定的大小
	struct DockHost {
		static constexpr auto id = Mk::wg2::Id("dockHost");
		Mk::wg2::DockSpace dock;

		Mk::wg2::Size hint(const Mk::wg2::Size& avail) const {
			return Mk::wg2::Size{avail.w > 0.0f ? avail.w : 720.0f, 420.0f};
		}
		void draw(const Mk::wg2::Rect& r) {
			const float w = std::min(r.w > 0.0f ? r.w : 720.0f, 900.0f);
			const float h = std::min(r.h > 0.0f ? r.h : 420.0f, 420.0f);
			dock.draw(Mk::wg2::Rect{r.x, r.y, w, h});
		}
	};

	DockHost root;

	PageDock() {
		const int viewport = root.dock.addPanel("视口", [](const Mk::wg2::Rect&) {
			ImGui::TextUnformatted("Viewport");
			ImGui::TextDisabled("拖页签到别处，或拖到叶子四边停靠");
		});
		const int scene = root.dock.addPanel("层级", [](const Mk::wg2::Rect&) {
			for (int i = 0; i < 8; ++i) ImGui::Text("Node %d", i);
		});
		const int props = root.dock.addPanel("属性", [](const Mk::wg2::Rect&) {
			ImGui::TextUnformatted("Inspector");
			ImGui::TextDisabled("分隔条可拖动；页签可拖走");
		});
		root.dock.dockPanel(scene, root.dock.root(), Mk::wg2::DockEdge::Left);
		root.dock.dockPanel(props, root.dock.leafOf(viewport), Mk::wg2::DockEdge::Right);
	}
};

} // namespace gallery
