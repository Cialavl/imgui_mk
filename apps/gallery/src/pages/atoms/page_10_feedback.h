#pragma once
// p10 反馈指示：ui::ProgressBar/ui::PlotLines/ui::Separator/ui::SeparatorText（wg2）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageFeedback {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "反馈与指示";
	static constexpr const char* desc  = "wg2 ui::ProgressBar（fraction 绑定）/ ui::PlotLines（外部 vector 只读绑定）/ ui::Separator(Text)。";
	static constexpr std::string_view code = snips::kFeedback;

using Root = ui::VBox<
ui::Id("root"),
ui::ProgressBar<ui::Id("bar1")>,
ui::ProgressBar<ui::Id("bar2")>,
ui::PlotLines<ui::Id("plot")>,
ui::Separator<ui::Id("sep")>,
ui::SeparatorText<ui::Id("section")>
>;

	Root root;
	Mk::window* win = nullptr;   // 形参存成员，避免 [&] 捕获形参后悬垂

	// 本页自己的数据（不共享、不依赖外部状态容器）
	float volume = 0.6f;

PageFeedback(Mk::window* w) : win(w) {


auto& w_bar1 = root.at<ui::Id("bar1")>();
w_bar1.fraction = &volume;
w_bar1.overlay = "Loading";
root.at<ui::Id("bar2")>().fraction = &volume;
root.at<ui::Id("plot")>().name = "波形历史";
// 曲线自己算：不依赖外部缓冲，也就不需要每帧填数据
	root.at<ui::Id("plot")>().valuesFn = [this]() -> const std::vector<float>& {
		static std::vector<float> buf(240);
		const auto f = win->frameIndex();
		for (std::size_t i = 0; i < buf.size(); ++i)
			buf[i] = 0.5f + 0.4f * std::sin(static_cast<float>(i + f) * 0.08f);
		return buf;
	};
root.at<ui::Id("section")>().name = "分隔标题";

}

};

} // namespace gallery
