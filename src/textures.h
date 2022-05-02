#ifndef LAUNCHER_TEXTURES_H
#define LAUNCHER_TEXTURES_H

#include <string>
#include <vita2d.h>

typedef struct {
    vita2d_texture *id=0;
    int width;
    int height;
} Tex;

extern GLuint connect_icon;
extern GLuint disconnect_icon;
extern GLuint search_icon;
extern GLuint refresh_icon;

extern GLuint square_icon;
extern GLuint triangle_icon;
extern GLuint circle_icon;
extern GLuint cross_icon;

extern GLuint folder_icon;
extern GLuint file_icon;
extern GLuint update_icon;
extern GLuint catalog_icon;

namespace Textures {
    void LoadFonts(Tex *font_texture);
    bool LoadImageFile(const std::string filename, GLuint *texture);
    void Init(void);
    void Exit(void);
    void Free(Tex *texture);
}

#endif
