#pragma once
// 三个浮层弹窗：开关状态归本组件所有
#include "demo_page.h"

namespace gallery {

namespace ui = Mk::wg2;

struct Popups {
	static constexpr auto id = Mk::wg2::Id("popups");

	Mk::wg2::WidgetState<> state;

	using PopupTree = ui::Popup<ui::Id("popupTree"), ui::Label<ui::Id("popupText")>, ui::Button<ui::Id("popupClose")>>;
	using ModalTree = ui::PopupModal<ui::Id("modalTree"), ui::Label<ui::Id("modalText")>, ui::Button<ui::Id("modalClose")>>;
	using AboutTree = ui::PopupModal<ui::Id("aboutTree"), ui::Label<ui::Id("aboutText")>, ui::Button<ui::Id("aboutOk")>>;

	using Root = ui::Overlay<ui::Id("popupsRoot"), PopupTree, ModalTree, AboutTree>;
	Root root;

	bool popupOpen = false, modalOpen = false, aboutOpen = false;

	void showAbout() { aboutOpen = true; }

	void init() {
		auto& pop = root.at<ui::Id("popupTree")>();
		pop.idname = "gpop";
		pop.open = &popupOpen;
		root.at<ui::Id("popupTree"), ui::Id("popupText")>().text = "A regular popup.";
		auto& popClose = root.at<ui::Id("popupTree"), ui::Id("popupClose")>();
		popClose.label = "关闭";
		popClose.onClick = [this] { popupOpen = false; Mk::wg2::setClipboard(""); ImGui::CloseCurrentPopup(); };

		auto& modal = root.at<ui::Id("modalTree")>();
		modal.idname = "gmodal";
		modal.open = &modalOpen;
		root.at<ui::Id("modalTree"), ui::Id("modalText")>().text = "A modal popup blocks interaction with the host.";
		auto& modalClose = root.at<ui::Id("modalTree"), ui::Id("modalClose")>();
		modalClose.label = "关闭";
		modalClose.onClick = [this] { modalOpen = false; ImGui::CloseCurrentPopup(); };

		auto& about = root.at<ui::Id("aboutTree")>();
		about.idname = "about";
		about.open = &aboutOpen;
		root.at<ui::Id("aboutTree"), ui::Id("aboutText")>().text =
			"mkui Widget Gallery\nImGui + GLFW + OpenGL wrapper\nwg2 template struct widget library";
		auto& aboutOk = root.at<ui::Id("aboutTree"), ui::Id("aboutOk")>();
		aboutOk.label = "OK";
		aboutOk.onClick = [this] { aboutOpen = false; ImGui::CloseCurrentPopup(); };
	}

	// 控件协议转发
	Mk::wg2::Size hint(const Mk::wg2::Size& a) const { return root.hint(a); }
	Mk::wg2::Policy policy() const { return root.policy(); }
	void draw(const Mk::wg2::Rect& r) { root.draw(r); }
};

} // namespace gallery
