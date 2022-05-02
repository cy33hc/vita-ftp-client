#include <imgui_vita.h>
#include <vita2d.h>
#include "textures.h"

GLuint connect_icon;
GLuint disconnect_icon;
GLuint search_icon;
GLuint refresh_icon;

GLuint square_icon;
GLuint triangle_icon;
GLuint circle_icon;
GLuint cross_icon;

GLuint folder_icon;
GLuint file_icon;
GLuint update_icon;
GLuint catalog_icon;

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

	bool LoadImageFile(const std::string filename, GLuint *texture)
	{
		// Load from file
		vita2d_texture *image = vita2d_load_PNG_file(filename.c_str());
		if (image == NULL) {
				return false;
		}
		int width = vita2d_texture_get_width(image);
		int height = vita2d_texture_get_height(image);

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, vita2d_texture_get_datap(image));
		vita2d_free_texture(image);
		return true;
	}
	
	void Init(void) {
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/connect.png", &connect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/disconnect.png", &disconnect_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/search.png", &search_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/refresh.png", &refresh_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/square.png", &square_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/circle.png", &circle_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/triangle.png", &triangle_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/cross.png", &cross_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/folder.png", &folder_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/file.png", &file_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/update.png", &update_icon);
		Textures::LoadImageFile("ux0:app/FTPCLI001/icons/catalog.png", &catalog_icon);
	}

	void Exit(void) {
		/*
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
		vita2d_free_texture(catalog_icon.id);
		*/
	}

	void Free(Tex *texture) {
		vita2d_free_texture(texture->id);
	}
	
}
