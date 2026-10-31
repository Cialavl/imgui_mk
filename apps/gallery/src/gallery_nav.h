#pragma once
// gallery 侧栏命名：框架层 NavList/PageHost 的别名（页面代码零改动）
#include <imgui_mk/ui/wg2/widgets/nav.h>

namespace gallery {

using Wg2PageEntry = Mk::wg2::NavEntry;

template<Mk::wg2::FixedString Id>
using PageNavWg2 = Mk::wg2::NavList<Id>;

template<Mk::wg2::FixedString Id>
using PageHostWg2 = Mk::wg2::PageHost<Id>;

} // namespace gallery
