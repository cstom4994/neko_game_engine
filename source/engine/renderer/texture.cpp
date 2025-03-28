
#include "texture.h"

#include "engine/renderer/renderer.h"
#include "engine/graphics.h"
#include "base/common/vfs.hpp"
#include "base/common/logger.hpp"
#include "base/common/math.hpp"
#include "base/common/profiler.hpp"
#include "engine/asset.h"
#include "engine/bootstrap.h"

// deps
#include "extern/stb_image.h"

#define CUTE_ASEPRITE_ASSERT neko_assert
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "extern/cute_aseprite.h"

static void _flip_image_vertical(unsigned char* data, unsigned int width, unsigned int height) {
    unsigned int size, stride, i, j;
    unsigned char* new_data;

    size = width * height * 4;
    stride = sizeof(char) * width * 4;

    new_data = (unsigned char*)mem_alloc(sizeof(char) * size);
    for (i = 0; i < height; i++) {
        j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }

    memcpy(data, new_data, size);
    mem_free(new_data);
}

NEKO_FORCE_INLINE void neko_tex_flip_vertically(int width, int height, u8* data) {
    u8 rgb[4];
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int top = 4 * (x + y * width);
            int bottom = 4 * (x + (height - y - 1) * width);
            memcpy(rgb, data + top, sizeof(rgb));
            memcpy(data + top, data + bottom, sizeof(rgb));
            memcpy(data + bottom, rgb, sizeof(rgb));
        }
    }
}

AssetTexture neko_aseprite_simple(String filename) {
    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);

    neko_defer(mem_free(contents.data));

    ase_t* ase;

    {
        PROFILE_BLOCK("ase_image load");
        ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

        neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
        // neko_aseprite_default_blend_bind(ase);
    }

    u8* data = reinterpret_cast<u8*>(ase->frames->pixels);

    // gfx_texture_desc_t t_desc = {};

    // t_desc.format = GL_RGBA;
    // t_desc.mag_filter = GL_NEAREST;
    // t_desc.min_filter = GL_NEAREST;
    // t_desc.num_mips = 0;
    // t_desc.width = ase->w;
    // t_desc.height = ase->h;
    //// t_desc.num_comps = 4;
    // t_desc.data = data;

    neko_tex_flip_vertically(ase->w, ase->h, (u8*)(data));
    // gfx_texture_t tex = gfx_texture_create(t_desc);

    AssetTexture tex{};

    neko_init_texture_from_memory_uncompressed(&tex, data, ase->w, ase->h, 4, TEXTURE_ALIASED);

    cute_aseprite_free(ase);
    return tex;
}

static void ase_default_blend_bind(ase_t* ase) {

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t*)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t* dst = frame->pixels;

        LOG_INFO("neko_aseprite_default_blend_bind: frame: {} cel_count: {}", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t* frame = ase->frames + cel->linked_frame_index;
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
            void* src = cel->pixels;
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

u64 generate_texture_handle(void* pixels, int w, int h, void* udata) {
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

void destroy_texture_handle(u64 texture_id, void* udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

bool texture_update_data(AssetTexture* tex, u8* data) {

    LockGuard<Mutex> lock{gBase.gpu_mtx};

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

template <size_t>
struct texture_create;

template <>
struct texture_create<1> {

    bool run(AssetTexture& tex, u8* data) {
        if (!data) return false;

        {
            LockGuard<Mutex> lock(gBase.gpu_mtx);

            // 如果存在 则释放旧的 GL 纹理
            if (tex.id != 0) glDeleteTextures(1, &tex.id);

            // 生成 GL 纹理
            glGenTextures(1, &tex.id);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex.id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            if (tex.flip_image_vertical) {
                _flip_image_vertical(data, tex.width, tex.height);
            }

            // 将纹理数据复制到 GL
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        return true;
    }
};

template <>
struct texture_create<2> {
    ase_t* ase = nullptr;
    AssetTexture run(const String& contents) {
        AssetTexture tex;
        u8* data = nullptr;
        {
            PROFILE_BLOCK("ase_image load");
            ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

            neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex.width = ase->w;
            tex.height = ase->h;
        }
        data = reinterpret_cast<u8*>(ase->frames->pixels);
        texture_create<1> tc;
        tc.run(tex, data);
        cute_aseprite_free(ase);
        return tex;
    }
};

template <>
struct texture_create<3> {
    AssetTexture run(const String& contents) {
        AssetTexture tex;
        u8* data = nullptr;
        {
            PROFILE_BLOCK("stb_image load");
            stbi_set_flip_vertically_on_load(false);
            data = stbi_load_from_memory((u8*)contents.data, (i32)contents.len, &tex.width, &tex.height, &tex.components, 0);
        }
        texture_create<1> tc;
        tc.run(tex, data);
        stbi_image_free(data);
        return tex;
    }
};

static bool _texture_load_vfs(AssetTexture* tex, String filename) {
    neko_assert(tex);

    LOG_INFO("texture: loading texture '{}' ...", filename.cstr());

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    if (filename.ends_with(".ase")) {
        texture_create<2> tc;
        *tex = tc.run(contents);
    } else {
        texture_create<3> tc;
        *tex = tc.run(contents);
    }

    LOG_INFO("texture: loading texture '{}' successful", filename.cstr());
    return true;
}

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical) {
    tex->id = 0;
    tex->flip_image_vertical = flip_image_vertical;
    return _texture_load_vfs(tex, filename);
}

void texture_bind(const char* filename) {
    LockGuard<Mutex> lock{gBase.gpu_mtx};

    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);

    if (ok && a.texture.id != 0) glBindTexture(GL_TEXTURE_2D, a.texture.id);
}

vec2 texture_get_size(const char* filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    error_assert(ok);
    return luavec2(a.texture.width, a.texture.height);
}

AssetTexture texture_get_ptr(const char* filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    return a.texture;
}

bool texture_update(AssetTexture* tex, String filename) { return _texture_load_vfs(tex, filename); }

bool load_texture_data_from_memory(const void* memory, int sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load) {
    // Load texture data

    int channels;
    u8* image = (u8*)stbi_load_from_memory((unsigned char*)memory, sz, width, height, &channels, 0);

    // if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = image;

    if (!*data) {
        // neko_image_free(&img);
        LOG_INFO("could not load image {}", memory);
        return false;
    }
    return true;
}

bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load) {
    size_t len = 0;
    const_str file_data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, file_path, &len);
    neko_assert(file_data);
    bool ret = load_texture_data_from_memory(file_data, (int)len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        LOG_INFO("could not load texture: {}", file_path);
    }
    mem_free(file_data);
    return ret;
}

AssetTexture* neko_new_texture_from_memory(void* data, u32 size, TextureFlags flags) {
    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture_from_memory(texture, data, size, flags);
    return texture;
}

AssetTexture* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, TextureFlags flags) {
    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture_from_memory_uncompressed(texture, pixels, width, height, component_count, flags);
    return texture;
}

void neko_init_texture_from_memory(AssetTexture* texture, void* data, u32 size, TextureFlags flags) {
    if (data == NULL || size == 0) {
        LOG_INFO("Data insufficient for texture creation.");
        return;
    }

    i32 width, height, component_count;

    if (!(flags & TEXTURE_NO_FLIP_VERTICAL)) stbi_set_flip_vertically_on_load(true);

    u8* pixels = stbi_load_from_memory((u8*)data, size, &width, &height, &component_count, 0);

    if (pixels == NULL) {
        LOG_INFO("Failed to load texture: {}", stbi_failure_reason());
        return;
    }

    neko_init_texture_from_memory_uncompressed(texture, pixels, width, height, component_count, flags);

    mem_free(pixels);
}

void neko_init_texture_from_memory_uncompressed(AssetTexture* texture, unsigned char* pixels, i32 width, i32 height, i32 component_count, TextureFlags flags) {
    if (pixels == NULL) {
        LOG_INFO("Data insufficient for texture creation.");
        return;
    }

    texture->width = width;
    texture->height = height;
    texture->components = component_count;

    texture->flags = flags;

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    u32 alias_mode = GL_LINEAR;
    if (flags & TEXTURE_ALIASED) {
        alias_mode = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, alias_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, alias_mode);

    u32 format = GL_RGB;
    u32 internal_format = GL_RGB;
    if (texture->components == 4) {
        internal_format = GL_RGBA;
        format = GL_RGBA;
    } else if (texture->components == 1) {
        internal_format = GL_RED;
        format = GL_RED;
    }

    if (flags & TEXTURE_SUBTEX) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, internal_format, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, pixels);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void neko_deinit_texture(AssetTexture* texture) {
    assert(texture);
    glDeleteTextures(1, &texture->id);
}

void neko_free_texture(AssetTexture* texture) {
    assert(texture);

    neko_deinit_texture(texture);

    mem_free(texture);
}

void neko_bind_texture(AssetTexture* texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
}
