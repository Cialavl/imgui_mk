#pragma once
// gallery 页面聚合：按分组分子目录（core/atoms/containers/popup_color/deco/app/layout） + kGroupXxx 分组常量（页面对象在 main 组装注册）
#include "gallery_chrome.h"
#include "pages/core/page_00_base.h"
#include "pages/core/page_01_lifecycle.h"
#include "pages/core/page_02_layout.h"
#include "pages/core/page_03_split.h"
#include "pages/atoms/page_04_text.h"
#include "pages/atoms/page_05_button.h"
#include "pages/atoms/page_06_input.h"
#include "pages/atoms/page_07_number.h"
#include "pages/atoms/page_08_drag.h"
#include "pages/atoms/page_09_select.h"
#include "pages/atoms/page_10_feedback.h"
#include "pages/containers/page_11_container.h"
#include "pages/containers/page_12_tab.h"
#include "pages/containers/page_13_menu.h"
#include "pages/containers/page_14_table.h"
#include "pages/popup_color/page_15_popup.h"
#include "pages/popup_color/page_16_color.h"
#include "pages/deco/page_17_deco.h"
#include "pages/app/page_18_theme.h"
#include "pages/app/page_19_window.h"
#include "pages/app/page_20_file.h"
#include "pages/app/page_21_fnrender.h"
#include "pages/core/page_22_wg2.h"
#include "pages/core/page_23_wg2_catalog.h"
#include "pages/layout/page_24_layout_basic.h"
#include "pages/layout/page_25_layout_size.h"
#include "pages/layout/page_26_layout_grid.h"
#include "pages/layout/page_27_layout_runtime.h"
#include "pages/layout/page_28_layout_dynamic.h"

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
