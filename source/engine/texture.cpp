#include "engine/texture.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "console.h"
#include "engine/asset.h"
#include "engine/base.h"
#include "engine/prelude.h"

// deps
#include <stb_image.h>

#define CUTE_ASEPRITE_ASSERT NEKO_ASSERT
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

typedef struct Texture Texture;
struct Texture {
    char *filename;
    GLuint gl_name;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;

    u64 last_modified;
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

static void ase_default_blend_bind(ase_t *ase) {

    NEKO_ASSERT(ase);
    NEKO_ASSERT(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t *)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t *dst = frame->pixels;

        NEKO_DEBUG_LOG("neko_aseprite_default_blend_bind: frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t *frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                NEKO_ASSERT(found);
            }
            void *src = cel->pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -NEKO_MIN(cx, 0);
            int ct = -NEKO_MIN(cy, 0);
            int dl = NEKO_MAX(cx, 0);
            int dt = NEKO_MAX(cy, 0);
            int dr = NEKO_MIN(ase->w, cw + cx);
            int db = NEKO_MIN(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    ase_color_t dst_color = dst[dst_index];
                    ase_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

static bool texture_load(Texture *tex) {
    u8 *data = nullptr;

    // already have latest?

    u64 modtime = vfs_file_modtime(tex->filename);

    if (modtime == 0) {
        if (tex->last_modified == 0) {
            console_printf("texture: loading texture '%s' ... unsuccessful\n", tex->filename);
            tex->last_modified = modtime;
        }
        return false;
    }

    if (modtime == tex->last_modified) return tex->gl_name != 0;

    console_printf("texture: loading texture '%s' ...", tex->filename);

    String filename = {tex->filename};

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase;

    if (filename.ends_with(".ase")) {

        {
            PROFILE_BLOCK("ase_image load");
            ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

            NEKO_ASSERT(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex->width = ase->w;
            tex->height = ase->h;
        }

        data = reinterpret_cast<u8 *>(ase->frames->pixels);

    } else {

        {
            PROFILE_BLOCK("stb_image load");
            data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &tex->width, &tex->height, &tex->components, 0);
        }
    }

    // 读入纹理数据
    if (!data) {
        tex->last_modified = modtime;
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

    if (filename.ends_with(".ase")) {
        cute_aseprite_free(ase);
    } else {
        stbi_image_free(data);
    }

    tex->last_modified = modtime;
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
    return texture_load(tex);
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

void texture_fini() {
    Texture *tex;
    array_foreach(tex, textures) mem_free(tex->filename);
    array_free(textures);
}

void texture_update() {
    Texture *tex;

    array_foreach(tex, textures) texture_load(tex);
}
