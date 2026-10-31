// 框架层导航/页面宿主：不依赖 ImGui 上下文的那部分逻辑
#include <gtest/gtest.h>

#include <imgui_mk/ui/wg2/widgets/nav.h>

using namespace Mk::wg2;

namespace {

using Nav = NavList<Id("nav")>;
using Host = PageHost<Id("host")>;

} // namespace

TEST(Wg2Nav, NavEntryDeduplicatesConsecutiveHeaders) {
	Nav nav;
	nav.addEntry("分组 A", true);
	nav.addEntry("分组 A", true);   // 连续同名组标题只保留一个
	nav.addEntry("页 1", false);
	nav.addEntry("页 2", false);
	EXPECT_EQ(nav.entries.size(), std::size_t{3});
	EXPECT_TRUE(nav.entries[0].header);
}

TEST(Wg2Nav, PageCountSkipsHeaders) {
	Nav nav;
	nav.addEntry("分组 A", true);
	nav.addEntry("页 1", false);
	nav.addEntry("页 2", false);
	nav.addEntry("分组 B", true);
	nav.addEntry("页 3", false);
	EXPECT_EQ(nav.pageCount(), 3);
}

TEST(Wg2Nav, PageHostTracksRegisteredPages) {
	Host host;
	EXPECT_EQ(host.pageCount(), 0);
	host.addPage([](const Rect&) {});
	host.addPage([](const Rect&) {});
	EXPECT_EQ(host.pageCount(), 2);
}
