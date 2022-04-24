#include <imgui_vita2d/imgui_vita.h>
#include <stdio.h>
#include <vita2d.h>
#include "windows.h"
#include "gui.h"
#include "textures.h"

bool done = false;
int gui_mode = GUI_MODE_BROWSER;

namespace GUI {
	int RenderLoop(void) {
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		Windows::Init();
		while (!done) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			if (gui_mode < GUI_MODE_IME)
			{
				ImGui_ImplVita2D_NewFrame();
			}
			
			if (gui_mode == GUI_MODE_BROWSER)
			{
				Windows::MainWindow();
			} else if (gui_mode == GUI_MODE_IME)
			{
				Windows::HandleImeInput();
			}
			
			if (gui_mode < GUI_MODE_IME)
			{
				ImGui::Render();
				ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
			}

			vita2d_end_drawing();
			vita2d_common_dialog_update();
			vita2d_swap_buffers();
			sceDisplayWaitVblankStart();
		}
		
		return 0;
	}
}
