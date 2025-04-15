
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
#include <stb_image.h>

#define CUTE_ASEPRITE_ASSERT neko_assert
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "deps/cute_aseprite.h"

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

void neko_tex_flip_vertically(int width, int height, u8* data) {
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

template <size_t>
struct texture_create_impl;

template <>
struct texture_create_impl<1> {

    bool run(AssetTexture* tex, u8* data) {
        if (!data) return false;

        bool EnableDSA = the<Renderer>().EnableDSA;

        {
            LockGuard<Mutex> lock(gBase.gpu_mtx);

            // 如果存在 则释放旧的 GL 纹理
            if (tex->id != 0) glDeleteTextures(1, &tex->id);

            u32 alias_mode = GL_NEAREST;
            if (tex->flags & TEXTURE_LINEAR) {
                alias_mode = GL_LINEAR;
            }

            // 生成 GL 纹理
            if (EnableDSA) [[likely]] {
                glCreateTextures(GL_TEXTURE_2D, 1, &tex->id);
                glTextureParameteri(tex->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(tex->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTextureParameteri(tex->id, GL_TEXTURE_MIN_FILTER, alias_mode);
                glTextureParameteri(tex->id, GL_TEXTURE_MAG_FILTER, alias_mode);
            } else {
                glGenTextures(1, &tex->id);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex->id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, alias_mode);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, alias_mode);
            }

            if (tex->flip_image_vertical) {
                _flip_image_vertical(data, tex->width, tex->height);
            }

            u32 format = GL_RGB;
            u32 internal_format = GL_RGB;
            if (tex->components == 4) {
                internal_format = GL_RGBA;
                format = GL_RGBA8;
            } else if (tex->components == 1) {
                internal_format = GL_RED;
                format = GL_R8;  // OpenGL 3.0+
            }

            if (EnableDSA) [[likely]] {
                if (tex->flags & TEXTURE_SUBTEX) {  // 如果已经分配了存储，仅更新纹理数据
                    glTextureSubImage2D(tex->id, 0, 0, 0, tex->width, tex->height, internal_format, GL_UNSIGNED_BYTE, data);
                } else {
                    glTextureStorage2D(tex->id, 1, format, tex->width, tex->height);
                    glTextureSubImage2D(tex->id, 0, 0, 0, tex->width, tex->height, internal_format, GL_UNSIGNED_BYTE, data);
                }
            } else {
                if (tex->flags & TEXTURE_SUBTEX) {
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->width, tex->height, internal_format, GL_UNSIGNED_BYTE, data);
                } else {
                    glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0, internal_format, GL_UNSIGNED_BYTE, data);
                }

                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        return true;
    }
};

template <>
struct texture_create_impl<2> {
    ase_t* ase = nullptr;
    bool run(AssetTexture* tex, const String& contents) {

        u8* data = nullptr;
        {
            PROFILE_BLOCK("ase_image load");
            ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

            neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex->width = ase->w;
            tex->height = ase->h;
            tex->components = 4;
            tex->flags = TEXTURE_ALIASED;
        }
        data = reinterpret_cast<u8*>(ase->frames->pixels);
        texture_create_impl<1> tc;
        tc.run(tex, data);
        cute_aseprite_free(ase);
        return true;
    }
};

template <>
struct texture_create_impl<3> {
    bool run(AssetTexture* tex, const String& contents) {
        u8* data = nullptr;
        {
            PROFILE_BLOCK("stb_image load");
            stbi_set_flip_vertically_on_load(false);
            data = stbi_load_from_memory((u8*)contents.data, (i32)contents.len, &tex->width, &tex->height, &tex->components, 0);
        }
        texture_create_impl<1> tc;
        tc.run(tex, data);
        stbi_image_free(data);
        return true;
    }
};

AssetTexture texure_aseprite_simple(String filename) {
    String contents = {};
    bool ok = the<VFS>().read_entire_file(&contents, filename);
    neko_defer(mem_free(contents.data));

    neko_assert(ok);

    ase_t* ase;

    {
        PROFILE_BLOCK("ase_image load");
        ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

        neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
        // neko_aseprite_default_blend_bind(ase);
    }

    u8* data = reinterpret_cast<u8*>(ase->frames->pixels);

    neko_tex_flip_vertically(ase->w, ase->h, (u8*)(data));

    AssetTexture tex{};

    tex.components = 4;
    tex.width = ase->w;
    tex.height = ase->h;
    tex.flags = TEXTURE_ALIASED;

    texture_create_impl<1> tc;
    tc.run(&tex, data);

    cute_aseprite_free(ase);
    return tex;
}

void ase_blend_bind(ase_t* ase) {
    PROFILE_FUNC();

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t*)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t* dst = frame->pixels;

        // LOG_INFO("neko_aseprite_default_blend_bind: frame: {} cel_count: {}", i, frame->cel_count);

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

static bool _texture_load_vfs(AssetTexture* tex, String filename) {
    neko_assert(tex);

    LOG_INFO("texture: loading texture '{}' ...", filename.cstr());

    String contents = {};
    bool ok = the<VFS>().read_entire_file(&contents, filename);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    if (filename.ends_with(".ase")) {
        texture_create_impl<2> tc;
        tc.run(tex, contents);
    } else {
        texture_create_impl<3> tc;
        tc.run(tex, contents);
    }

    LOG_INFO("texture: loading texture '{}' successful", filename.cstr());
    return true;
}

bool texture_create(AssetTexture* tex, u8* data, u32 width, u32 height, u32 num_comps, TextureFlags flags) {

    tex->components = num_comps;
    tex->width = width;
    tex->height = height;
    tex->flags = flags;

    texture_create_impl<1> tc;

    return tc.run(tex, data);
}

void texture_release(AssetTexture* texture) {
    assert(texture);
    glDeleteTextures(1, &texture->id);
}

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical) {
    tex->id = 0;
    tex->flip_image_vertical = flip_image_vertical;
    return _texture_load_vfs(tex, filename);
}

void texture_bind_byname(String filename, u32 slot) {

    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);

    if (ok && assets_get<AssetTexture>(a).id != 0) texture_bind(&assets_get<AssetTexture>(a), slot);
}

void texture_bind(u32 id, u32 slot) {
    LockGuard<Mutex> lock{gBase.gpu_mtx};

    bool EnableDSA = the<Renderer>().EnableDSA;
    if (EnableDSA) [[likely]] {
        glBindTextureUnit(slot, id);
    } else {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }
}

void texture_bind(AssetTexture* texture, u32 slot) { texture_bind(texture ? texture->id : 0, slot); }

vec2 texture_get_size(String filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    error_assert(ok);
    return luavec2(assets_get<AssetTexture>(a).width, assets_get<AssetTexture>(a).height);
}

AssetTexture texture_get_ptr(String filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    return assets_get<AssetTexture>(a);
}

bool texture_update(AssetTexture* tex, String filename) { return _texture_load_vfs(tex, filename); }

AssetTexture* neko_new_texture_from_memory(void* data, u32 size, TextureFlags flags) {
    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture_from_memory(texture, data, size, flags);
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

    texture_create(texture, pixels, width, height, component_count, flags);

    mem_free(pixels);
}
