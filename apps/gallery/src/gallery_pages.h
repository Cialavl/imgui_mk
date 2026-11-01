#pragma once
// gallery 页面聚合：按 UI 层级分子目录，文件名即主题（无序号）（core/atoms/containers/popup_color/deco/app/layout） + kGroupXxx 分组常量（页面对象在 main 组装注册）
#include "gallery_chrome.h"
#include "pages/core/base.h"
#include "pages/core/lifecycle.h"
#include "pages/core/layout.h"
#include "pages/core/split.h"
#include "pages/atoms/text.h"
#include "pages/atoms/button.h"
#include "pages/atoms/input.h"
#include "pages/atoms/number.h"
#include "pages/atoms/drag.h"
#include "pages/atoms/select.h"
#include "pages/atoms/feedback.h"
#include "pages/containers/container.h"
#include "pages/containers/tab.h"
#include "pages/containers/menu.h"
#include "pages/containers/table.h"
#include "pages/popup_color/popup.h"
#include "pages/popup_color/color.h"
#include "pages/deco/deco.h"
#include "pages/app/theme.h"
#include "pages/app/window.h"
#include "pages/app/file.h"
#include "pages/app/fnrender.h"
#include "pages/core/wg2.h"
#include "pages/core/wg2_catalog.h"
#include "pages/layout/layout_basic.h"
#include "pages/layout/layout_size.h"
#include "pages/layout/layout_grid.h"
#include "pages/layout/layout_runtime.h"
#include "pages/layout/layout_dynamic.h"

namespace gallery {

// ============ 侧栏分组常量（addPage 组首传参用）============
inline constexpr const char* kGroupLayout     = "布局";
inline constexpr const char* kGroupWg          = "wg 体系";
inline constexpr const char* kGroupAtoms       = "原子控件";
inline constexpr const char* kGroupContainers  = "容器控件";
inline constexpr const char* kGroupPopColor    = "弹窗与颜色";
inline constexpr const char* kGroupDeco        = "装饰件";
inline constexpr const char* kGroupApp         = "应用";

} // namespace gallery
