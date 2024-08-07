#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/base.h"
#include "engine/glew_glfw.h"

typedef struct Texture {
    char *filename;
    GLuint id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;

    u64 last_modified;

    bool flip_image_vertical;
} Texture;

bool texture_load(const char *filename, bool flip_image_vertical = true);
void texture_bind(const char *filename);
CVec2 texture_get_size(const char *filename);  // (width, height)
Texture *texture_get_ptr(const char *filename);

void texture_init();
void texture_fini();
void texture_update();

#endif
