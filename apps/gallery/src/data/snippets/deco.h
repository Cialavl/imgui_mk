#pragma once
// 示例代码（deco）：页面 Code 区展示用
#include <string_view>

namespace gallery::snips {

inline constexpr std::string_view kDeco = R"(Mk::wg2::Tooltip<Id("tip"), Label<Id("tl")>> tip;
tip.text = "提示";
Mk::wg2::Colored<Id("col"), Label<Id("cl")>> col;
col.color = ImVec4(0.4f,0.9f,0.6f,1);
Mk::wg2::Indent<Id("ind"), Label<Id("il")>> ind; ind.amount = 24.0f;
Mk::wg2::ItemWidth<Id("iw"), DragF<Id("df")>> iw; iw.width = 180.0f;
Mk::wg2::When<Id("w"), Label<Id("wl")>> w; w.show = cond;
btn.state.enabled = false;)";

} // namespace gallery::snips
