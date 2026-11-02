#pragma once
// 示例代码（popup_color）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kPopup = R"(Mk::wg2::Popup<Id("pop"), Label<Id("pl")>> pop;
pop.open = true;                         // 请求打开：开合状态归控件自己
Mk::wg2::PopupModal<Id("modal"), Label<Id("ml")>> modal;
modal.open = true;)";

} // namespace gallery::snips
