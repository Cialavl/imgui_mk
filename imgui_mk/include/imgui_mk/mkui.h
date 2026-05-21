#pragma once
#include<vector>
#include<memory>
#include<imgui_mk/window.h>

namespace Mk {

	static std::vector<window*> m_Windows;
	struct mkui {
		static void RegistrationWindow(window* hand) {

			m_Windows.push_back(hand);
		}
		static void run() {
			while (!m_Windows.empty()) {
				glfwPollEvents();
				for (auto& winp : m_Windows) {

					if (winp != nullptr) {

						if (winp->shouldClose()) {

							winp->~window();

							winp = nullptr;

						}
						else {
							winp->render();

						}
					}
				}
			}
		}
	};
}