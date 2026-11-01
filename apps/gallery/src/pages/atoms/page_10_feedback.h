#pragma once
// p10 反馈指示：ProgressBar/PlotLines/Separator/SeparatorText（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

struct PageFeedback {
using Root = VBox<
Id("root"),
ProgressBar<Id("bar1")>,
ProgressBar<Id("bar2")>,
PlotLines<Id("plot")>,
Separator<Id("sep")>,
SeparatorText<Id("section")>
>;

Page<Root> page;
	Mk::window* win = nullptr;   // 形参存成员，避免 [&] 捕获形参后悬垂

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float volume = 0.6f;

PageFeedback(Mk::window* w) : win(w) {
page.setup("反馈与指示",
"wg2 ProgressBar（fraction 绑定）/ PlotLines（外部 vector 只读绑定）/ Separator(Text)。",
snips::kFeedback);

auto& w_bar1 = page.root.at<Id("bar1")>();
w_bar1.fraction = &volume;
w_bar1.overlay = "Loading";
page.root.at<Id("bar2")>().fraction = &volume;
page.root.at<Id("plot")>().name = "波形历史";
// 曲线自己算：不依赖外部缓冲，也就不需要每帧填数据
	page.root.at<Id("plot")>().valuesFn = [this]() -> const std::vector<float>& {
		static std::vector<float> buf(240);
		const auto f = win->frameIndex();
		for (std::size_t i = 0; i < buf.size(); ++i)
			buf[i] = 0.5f + 0.4f * std::sin(static_cast<float>(i + f) * 0.08f);
		return buf;
	};
page.root.at<Id("section")>().name = "分隔标题";

}

};

} // namespace gallery
