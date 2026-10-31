#pragma once
// 示例代码（popup_color）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kPopup = R"(bool open = false;
Mk::wg2::Popup<Id("pop"), Label<Id("pl")>> pop;
pop.open = &open;
Mk::wg2::PopupModal<Id("modal"), Label<Id("ml")>> modal;
modal.open = &open;)";

} // namespace gallery::snips
