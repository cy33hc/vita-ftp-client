#include <imgui_vita2d/imgui_vita.h>
#include <vita2d.h>

#include "textures.h"

Tex connect_icon;
Tex disconnect_icon;
Tex search_icon;
Tex refresh_icon;

Tex square_icon;
Tex triangle_icon;
Tex circle_icon;
Tex cross_icon;

Tex folder_icon;
Tex file_icon;
Tex update_icon;

namespace Textures {
	
	void LoadFonts()
	{
		// Build and load the texture atlas into a texture
		uint32_t* pixels = NULL;
		int width, height;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF(
					"ux0:app/SMLA00001/Ubuntu-R.ttf",
					16.0f,
					0,
					io.Fonts->GetGlyphRangesDefault());
	}

	bool LoadImageFile(const std::string filename, Tex *texture)
	{
		// Load from file
		vita2d_texture *image = vita2d_load_PNG_file(filename.c_str());
		if (image == NULL) {
			return false;
		}
		int image_width = vita2d_texture_get_width(image);
		int image_height = vita2d_texture_get_height(image);
		vita2d_texture_set_filters(image, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);

		texture->id = image;
		texture->width = image_width;
		texture->height = image_height;

		return true;
	}
	
	void Init(void) {
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/connect.png", &connect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/disconnect.png", &disconnect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/search.png", &search_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/refresh.png", &refresh_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/square.png", &square_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/circle.png", &circle_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/triangle.png", &triangle_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/cross.png", &cross_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/folder.png", &folder_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/file.png", &file_icon);
		Textures::LoadImageFile("ux0:app/FTPCLIENT/icons/update.png", &update_icon);
	}

	void Exit(void) {
		vita2d_free_texture(connect_icon.id);
		vita2d_free_texture(disconnect_icon.id);
		vita2d_free_texture(search_icon.id);
		vita2d_free_texture(refresh_icon.id);
		vita2d_free_texture(square_icon.id);
		vita2d_free_texture(circle_icon.id);
		vita2d_free_texture(triangle_icon.id);
		vita2d_free_texture(cross_icon.id);
		vita2d_free_texture(folder_icon.id);
		vita2d_free_texture(file_icon.id);
		vita2d_free_texture(update_icon.id);
	}

	void Free(Tex *texture) {
		vita2d_free_texture(texture->id);
	}
	
}
