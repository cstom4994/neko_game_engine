#include "engine/texture.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/console.h"
#include "engine/game.h"
#include "engine/prelude.h"
#include "engine/vfs.h"

// deps
#include <stb_image.h>

#define CUTE_ASEPRITE_ASSERT neko_assert
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

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

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t *)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t *dst = frame->pixels;

        console_log("neko_aseprite_default_blend_bind: frame: %d cel_count: %d", i, frame->cel_count);

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
                neko_assert(found);
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

u64 generate_texture_handle(void *pixels, int w, int h, void *udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (u64)location;
}

void destroy_texture_handle(u64 texture_id, void *udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

bool texture_update_data(Texture *tex, u8 *data) {

    LockGuard lock{&g_app->gpu_mtx};

    // 如果存在 则释放旧的 GL 纹理
    if (tex->id != 0) glDeleteTextures(1, &tex->id);

    // 生成 GL 纹理
    glGenTextures(1, &tex->id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (tex->flip_image_vertical) {
        _flip_image_vertical(data, tex->width, tex->height);
    }

    // 将纹理数据复制到 GL

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

static bool _texture_load_vfs(Texture *tex, String filename) {
    neko_assert(tex);

    u8 *data = nullptr;

    console_printf("texture: loading texture '%s' ...", filename.cstr());

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

            neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex->width = ase->w;
            tex->height = ase->h;
        }

        data = reinterpret_cast<u8 *>(ase->frames->pixels);

    } else {

        {
            PROFILE_BLOCK("stb_image load");
            stbi_set_flip_vertically_on_load(false);
            data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &tex->width, &tex->height, &tex->components, 0);
        }
    }

    // 读入纹理数据
    if (!data) {
        // tex->last_modified = modtime;
        console_printf(" unsuccessful\n");
        return false;  // 保持旧的GL纹理
    }

    {
        LockGuard lock{&g_app->gpu_mtx};

        // 如果存在 则释放旧的 GL 纹理
        if (tex->id != 0) glDeleteTextures(1, &tex->id);

        // 生成 GL 纹理
        glGenTextures(1, &tex->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (tex->flip_image_vertical) {
            _flip_image_vertical(data, tex->width, tex->height);
        }

        // 将纹理数据复制到 GL

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (filename.ends_with(".ase")) {
        cute_aseprite_free(ase);
    } else {
        stbi_image_free(data);
    }

    // tex->last_modified = modtime;
    console_printf(" successful\n");
    return true;
}

bool texture_load(Texture *tex, String filename, bool flip_image_vertical) {
    tex->id = 0;
    tex->flip_image_vertical = flip_image_vertical;
    return _texture_load_vfs(tex, filename);
}

void texture_bind(const char *filename) {
    LockGuard lock{&g_app->gpu_mtx};

    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);

    if (ok && a.texture.id != 0) glBindTexture(GL_TEXTURE_2D, a.texture.id);
}

LuaVec2 texture_get_size(const char *filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    error_assert(ok);
    return luavec2(a.texture.width, a.texture.height);
}

Texture texture_get_ptr(const char *filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    return a.texture;
}

bool texture_update(Texture *tex, String filename) { return _texture_load_vfs(tex, filename); }

bool load_texture_data_from_memory(const void *memory, size_t sz, i32 *width, i32 *height, u32 *num_comps, void **data, bool flip_vertically_on_load) {
    // Load texture data

    int channels;
    u8 *image = (u8 *)stbi_load_from_memory((unsigned char *)memory, sz, width, height, &channels, 0);

    // if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = image;

    if (!*data) {
        // neko_image_free(&img);
        console_log("could not load image %p", memory);
        return false;
    }
    return true;
}

bool load_texture_data_from_file(const char *file_path, i32 *width, i32 *height, u32 *num_comps, void **data, bool flip_vertically_on_load) {
    u64 len = 0;
    const_str file_data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, file_path, &len);
    neko_assert(file_data);
    bool ret = load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        console_log("could not load texture: %s", file_path);
    }
    mem_free(file_data);
    return ret;
}

bool neko_asset_texture_load_from_file(const_str path, void *out, gfx_texture_desc_t *desc, bool flip_on_load, bool keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(gfx_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = R_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = GL_REPEAT;
        t->desc.wrap_t = GL_REPEAT;
    }

    void *tex_data = NULL;
    i32 w, h;
    u32 cc;
    load_texture_data_from_file(path, &w, &h, &cc, &tex_data, false);
    neko_defer(mem_free(tex_data));

    t->desc.data = tex_data;
    t->desc.width = w;
    t->desc.height = h;

    if (!t->desc.data) {

        console_log("failed to load texture data %s", path);
        return false;
    }

    t->hndl = gfx_texture_create(t->desc);

    if (!keep_data) {
        // neko_image_free(&img);
        t->desc.data = NULL;
    }

    return true;
}

bool neko_asset_texture_load_from_memory(const void *memory, size_t sz, void *out, gfx_texture_desc_t *desc, bool flip_on_load, bool keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(gfx_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = R_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = GL_REPEAT;
        t->desc.wrap_t = GL_REPEAT;
    }

    // Load texture data
    i32 num_comps = 0;
    bool loaded = load_texture_data_from_memory(memory, sz, (i32 *)&t->desc.width, (i32 *)&t->desc.height, (u32 *)&num_comps, (void **)&t->desc.data, t->desc.flip_y);

    if (!loaded) {
        return false;
    }

    t->hndl = gfx_texture_create(t->desc);

    if (!keep_data) {
        mem_free(t->desc.data);
        t->desc.data = NULL;
    }

    return true;
}
