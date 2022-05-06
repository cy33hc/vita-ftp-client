#include <vitaGL.h>
#include <imgui_vita2d/imgui_vita.h>
#include <vita2d.h>
#include "textures.h"

Tex connect_icon;
Tex disconnect_icon;
Tex search_icon;
Tex refresh_icon;

Tex folder_icon;
Tex file_icon;
Tex update_icon;

namespace Textures {
	
	bool LoadImageFile(const std::string filename, Tex *texture)
	{
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
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/connect.png", &connect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/disconnect.png", &disconnect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/search.png", &search_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/refresh.png", &refresh_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/folder.png", &folder_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/file.png", &file_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/update.png", &update_icon);
	}

	void Exit(void) {
	}

	void Free(Tex *texture) {
		vita2d_free_texture(texture->id);
	}
	
}
