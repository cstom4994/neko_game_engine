#include "engine/neko_texture.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "console.h"
#include "engine/neko_prelude.h"
#include "engine/neko_base.h"

// deps
#include <stb_image.h>

typedef struct Texture Texture;
struct Texture {
    char *filename;
    GLuint gl_name;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;

    time_t last_modified;
};

static CArray *textures;

// -------------------------------------------------------------------------

static void _flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height) {
    unsigned int size, stride, i, j;
    unsigned char *new_data;

    size = width * height * 4;
    stride = sizeof(char) * width * 4;

    new_data = (unsigned char *)mem_alloc(sizeof(char) * size);
    for (i = 0; i < height; i++) {
        j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }

    memcpy(data, new_data, size);
    mem_free(new_data);
}

static bool _load(Texture *tex) {
    struct stat st;
    unsigned char *data;

    // already have latest?
    if (stat(tex->filename, &st) != 0) {
        if (tex->last_modified == 0) {
            console_printf("texture: loading texture '%s' ... unsuccessful\n", tex->filename);
            time(&tex->last_modified);
        }
        return false;
    }
    if (st.st_mtime == tex->last_modified) return tex->gl_name != 0;

    console_printf("texture: loading texture '%s' ...", tex->filename);

    // 读入纹理数据
    data = stbi_load(tex->filename, &tex->width, &tex->height, &tex->components, 0);
    if (!data) {
        tex->last_modified = st.st_mtime;
        console_printf(" unsuccessful\n");
        return false;  // 保持旧的GL纹理
    }

    // 如果存在 则释放旧的 GL 纹理
    if (tex->gl_name != 0) glDeleteTextures(1, &tex->gl_name);

    // 生成 GL 纹理
    glGenTextures(1, &tex->gl_name);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->gl_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 将纹理数据复制到 GL
    _flip_image_vertical(data, tex->width, tex->height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    tex->last_modified = st.st_mtime;
    console_printf(" successful\n");
    return true;
}

static Texture *_find(const char *filename) {
    Texture *tex;

    array_foreach(tex, textures) if (!strcmp(tex->filename, filename)) return tex;
    return NULL;
}

bool texture_load(const char *filename) {
    Texture *tex;

    // already exists?
    if ((tex = _find(filename))) return tex->gl_name != 0;

    tex = (Texture *)array_add(textures);
    tex->gl_name = 0;
    tex->last_modified = 0;
    tex->filename = (char *)mem_alloc(strlen(filename) + 1);
    strcpy(tex->filename, filename);
    return _load(tex);
}

void texture_bind(const char *filename) {
    Texture *tex;

    tex = _find(filename);
    if (tex && tex->gl_name != 0) glBindTexture(GL_TEXTURE_2D, tex->gl_name);
}

CVec2 texture_get_size(const char *filename) {
    Texture *tex;

    tex = _find(filename);
    error_assert(tex);
    return vec2(tex->width, tex->height);
}

// -------------------------------------------------------------------------

void texture_init() { textures = array_new(Texture); }
void texture_deinit() {
    Texture *tex;
    array_foreach(tex, textures) mem_free(tex->filename);
    array_free(textures);
}

void texture_update() {
    struct stat st;
    Texture *tex;

    array_foreach(tex, textures) _load(tex);
}
