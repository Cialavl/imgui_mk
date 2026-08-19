#pragma once
// p5 按钮图片：Button/ImageButton/Image
#include "../gallery_chrome.h"

namespace gallery {

struct PageButton {
	Button btnPlain, btnCount;
	ImageButton btnImage;
	LabelText btnClicks;
	Image imageA, imageB;
	HLayout imageRow;

	Page page;
	PageButton(State& s) {
		page.setup("按钮与图片",
			"Button（onClick）/ ImageButton（idname 即 ImGui ID）/ Image（loadTextureRGBA 生成纹理）。",
			snips::kButton);
		btnPlain.self.setName("普通按钮");
		btnPlain.self.onClick = [&] { ++s.clicks; };
		btnCount.self.setName("计数按钮");
		btnCount.self.onClick = [&] { s.clicks += 10; };
		btnImage.node.idname = "galimg";
		btnImage.self.tex = s.tex;
		btnImage.self.size = ImVec2(96, 96);
		btnImage.self.onClick = [&] { ++s.imgBtnClicks; };
		btnClicks.self.name = "图片按钮点击";
		imageA.self.tex = s.tex;
		imageA.self.size = ImVec2(64, 64);
		imageB.self.tex = s.tex;
		imageB.self.size = ImVec2(64, 64);

		imageRow.addWidget(imageA.node);
		imageRow.addWidget(imageB.node);
		page.addWidget(btnPlain);
		page.addWidget(btnCount);
		page.addWidget(btnImage);
		page.addWidget(btnClicks);
		page.addWidget(imageRow);
	}

	void refresh(State& s) {
		btnClicks.self.value = std::to_string(s.imgBtnClicks);
	}
};

} // namespace gallery
