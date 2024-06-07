
#include "neko_asset.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.h"
#include "engine/neko_common.h"

// stb_image
#include "deps/stb_image.h"

#if defined(NEKO_PLATFORM_LINUX)
#include <errno.h>
#define fopen_s fopen
#endif

neko_asset_t __neko_asset_handle_create_impl(u64 type_id, u32 asset_id, u32 importer_id) {
    neko_asset_t asset = NEKO_DEFAULT_VAL();
    asset.type_id = type_id;
    asset.asset_id = asset_id;
    asset.importer_id = importer_id;
    return asset;
}

neko_asset_manager_t neko_asset_manager_new() {
    neko_asset_manager_t assets = NEKO_DEFAULT_VAL();

    // Register default asset importers
    neko_asset_importer_desc_t tex_desc = NEKO_DEFAULT_VAL();
    neko_asset_importer_desc_t font_desc = NEKO_DEFAULT_VAL();
    // neko_asset_importer_desc_t audio_desc = NEKO_DEFAULT_VAL();
    neko_asset_importer_desc_t mesh_desc = NEKO_DEFAULT_VAL();
    neko_asset_importer_desc_t asset_desc = NEKO_DEFAULT_VAL();

    tex_desc.load_from_file = (neko_asset_load_func)&neko_asset_texture_load_from_file;
    font_desc.load_from_file = (neko_asset_load_func)&neko_asset_ascii_font_load_from_file;
    // audio_desc.load_from_file = (neko_asset_load_func)&neko_asset_audio_load_from_file;
    mesh_desc.load_from_file = (neko_asset_load_func)&neko_asset_mesh_load_from_file;

    neko_assets_register_importer(&assets, neko_asset_t, &asset_desc);
    neko_assets_register_importer(&assets, neko_asset_texture_t, &tex_desc);
    neko_assets_register_importer(&assets, neko_asset_ascii_font_t, &font_desc);
    // neko_assets_register_importer(&assets, neko_asset_audio_t, &audio_desc);
    neko_assets_register_importer(&assets, neko_asset_mesh_t, &mesh_desc);

    return assets;
}

void neko_asset_manager_free(neko_asset_manager_t *am) {
    // Free all data
}

void *__neko_assets_getp_impl(neko_asset_manager_t *am, u64 type_id, neko_asset_t hndl) {
    if (type_id != hndl.type_id) {
        neko_println("Warning: Type id: %zu doesn't match handle type id: %zu.", type_id, hndl.type_id);
        NEKO_ASSERT(false);
        return NULL;
    }

    // Need to grab the appropriate importer based on type
    if (!neko_hash_table_key_exists(am->importers, type_id)) {
        neko_println("Warning: Importer type %zu does not exist.", type_id);
        NEKO_ASSERT(false);
        return NULL;
    }

    neko_asset_importer_t *imp = neko_hash_table_getp(am->importers, type_id);

    // Vertify that importer id and handle importer id align
    if (imp->importer_id != hndl.importer_id) {
        neko_println("Warning: Importer id: %zu does not match handle importer id: %zu.", imp->importer_id, hndl.importer_id);
        NEKO_ASSERT(false);
        return NULL;
    }

    // Need to get data index from slot array using hndl asset id
    size_t offset = (((sizeof(u32) * hndl.asset_id) + 3) & (~3));
    u32 idx = *(u32 *)((char *)(imp->slot_array_indices_ptr) + offset);
    // Then need to return pointer to data at index
    size_t data_sz = imp->data_size;
    size_t s = data_sz == 8 ? 7 : 3;
    offset = (((data_sz * idx) + s) & (~s));
    return ((char *)(imp->slot_array_data_ptr) + offset);
}

void neko_asset_importer_set_desc(neko_asset_importer_t *imp, neko_asset_importer_desc_t *desc) {
    imp->desc = desc ? *desc : imp->desc;
    imp->desc.load_from_file = imp->desc.load_from_file ? (neko_asset_load_func)imp->desc.load_from_file : (neko_asset_load_func)&neko_asset_default_load_from_file;
    imp->desc.default_asset = imp->desc.default_asset ? (neko_asset_default_func)imp->desc.default_asset : (neko_asset_default_func)&neko_asset_default_asset;
}

neko_asset_t neko_asset_default_asset() {
    neko_asset_t a = NEKO_DEFAULT_VAL();
    return a;
}

void neko_asset_default_load_from_file(const_str path, void *out) {
    // Nothing...
}

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// STB
#include <imstb_rectpack.h>
#include <imstb_truetype.h>

NEKO_API_DECL neko_image_t neko_image_load(const_str path) {
    int width, height, channels;
    unsigned char *image = stbi_load(path, &width, &height, &channels, 0);
    return (neko_image_t){.w = width, .h = height, .pix = image};
}

NEKO_API_DECL void neko_image_free(neko_image_t img) { stbi_image_free(img.pix); }

NEKO_API_DECL b32 neko_util_load_texture_data_from_memory(const void *memory, size_t sz, s32 *width, s32 *height, u32 *num_comps, void **data, b32 flip_vertically_on_load) {
    // Load texture data

    int channels;
    unsigned char *image = stbi_load_from_memory(memory, sz, width, height, &channels, 0);

    // if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = image;

    if (!*data) {
        // neko_image_free(&img);
        NEKO_WARN("could not load image %p", memory);
        return false;
    }
    return true;
}

NEKO_API_DECL bool neko_asset_texture_load_from_file(const_str path, void *out, neko_render_texture_desc_t *desc, b32 flip_on_load, b32 keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(neko_render_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = R_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = R_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = R_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    neko_image_t img = neko_image_load(path);
    // if (t->desc.flip_y) neko_png_flip_image_horizontal(&img);

    t->desc.data[0] = img.pix;
    t->desc.width = img.w;
    t->desc.height = img.h;

    if (!t->desc.data) {
        neko_image_free(img);
        NEKO_WARN("failed to load texture data %s", path);
        return false;
    }

    t->hndl = neko_render_texture_create(t->desc);

    if (!keep_data) {
        // neko_image_free(&img);
        *t->desc.data = NULL;
    }

    return true;
}

/*
bool neko_asset_texture_load_from_file(const char* path, void* out, neko_render_texture_desc_t* desc, b32 flip_on_load, b32 keep_data)
{
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    NEKO_ASSERT(file_data);
    b32 ret = neko_asset_texture_load_from_memory(file_data, len, out, desc, flip_on_load, keep_data);
    neko_free(file_data);
    return ret;
}
 */

bool neko_asset_texture_load_from_memory(const void *memory, size_t sz, void *out, neko_render_texture_desc_t *desc, b32 flip_on_load, b32 keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(neko_render_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = R_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = R_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = R_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = R_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    s32 num_comps = 0;
    b32 loaded = neko_util_load_texture_data_from_memory(memory, sz, (s32 *)&t->desc.width, (s32 *)&t->desc.height, (u32 *)&num_comps, (void **)&t->desc.data, t->desc.flip_y);

    if (!loaded) {
        return false;
    }

    t->hndl = neko_render_texture_create(t->desc);

    if (!keep_data) {
        neko_free(t->desc.data);
        *t->desc.data = NULL;
    }

    return true;
}

bool neko_asset_ascii_font_load_from_file(const_str path, void *out, u32 point_size) {
    size_t len = 0;
    char *ttf = neko_platform_read_file_contents(path, "rb", &len);
    if (!point_size) {
        NEKO_WARN("font: %s: point size not declared. setting to default 16.", neko_fs_get_filename(path));
        point_size = 16;
    }
    bool ret = neko_asset_ascii_font_load_from_memory(ttf, len, out, point_size);
    if (!ret) {
        NEKO_WARN("font failed to load: %s", neko_fs_get_filename(path));
    } else {
        NEKO_TRACE("font successfully loaded: %s", neko_fs_get_filename(path));
    }
    neko_safe_free(ttf);
    return ret;
}

bool neko_asset_ascii_font_load_from_memory(const void *memory, size_t sz, void *out, u32 point_size) {
    neko_asset_ascii_font_t *f = (neko_asset_ascii_font_t *)out;

    if (!point_size) {
        NEKO_WARN("font: point size not declared. setting to default 16.");
        point_size = 16;
    }

    // Poor attempt at an auto resized texture
    const u32 point_wh = NEKO_MAX(point_size, 32);
    const u32 w = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;
    const u32 h = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;

    const u32 num_comps = 4;
    u8 *alpha_bitmap = (u8 *)neko_safe_malloc(w * h);
    u8 *flipmap = (u8 *)neko_safe_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8 *)memory, 0, (f32)point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar *)f->glyphs);

    // 翻转纹理
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i) {
        for (u32 j = 0; j < w; ++j) {
            u32 i0 = i * w + j;
            u32 i1 = i * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    neko_render_texture_desc_t desc = NEKO_DEFAULT_VAL();
    desc.width = w;
    desc.height = h;
    *desc.data = flipmap;
    desc.format = R_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = R_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = R_TEXTURE_FILTER_NEAREST;

    // 使用位图数据生成位图的图集纹理
    f->texture.hndl = neko_render_texture_create(desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    bool success = false;
    if (v <= 0) {
        NEKO_WARN("font failed to load, baked texture was too small: %d", v);
    } else {
        NEKO_TRACE("font baked size: %d", v);
        success = true;
    }

    neko_safe_free(alpha_bitmap);
    neko_safe_free(flipmap);
    return success;
}

NEKO_API_DECL f32 neko_asset_ascii_font_max_height(const neko_asset_ascii_font_t *fp) {
    if (!fp) return 0.f;
    f32 h = 0.f, x = 0.f, y = 0.f;
    const_str txt = "1l`'f()ABCDEFGHIJKLMNOjPQqSTU!";
    while (txt[0] != '\0') {
        char c = txt[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = NEKO_DEFAULT_VAL();
            stbtt_GetBakedQuad((stbtt_bakedchar *)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            h = NEKO_MAX(NEKO_MAX(h, fabsf(q.y0)), fabsf(q.y1));
        }
        txt++;
    };
    return h;
}

NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions(const neko_asset_ascii_font_t *fp, const_str text, s32 len) { return neko_asset_ascii_font_text_dimensions_ex(fp, text, len, 0); }

NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions_ex(const neko_asset_ascii_font_t *fp, const_str text, s32 len, b32 include_past_baseline) {
    neko_vec2 dimensions = neko_v2s(0.f);

    if (!fp || !text) return dimensions;
    f32 x = 0.f;
    f32 y = 0.f;
    f32 y_under = 0;

    while (text[0] != '\0' && len--) {
        char c = text[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = NEKO_DEFAULT_VAL();
            stbtt_GetBakedQuad((stbtt_bakedchar *)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            dimensions.x = NEKO_MAX(dimensions.x, x);
            dimensions.y = NEKO_MAX(dimensions.y, fabsf(q.y0));
            if (include_past_baseline) y_under = NEKO_MAX(y_under, fabsf(q.y1));
        }
        text++;
    };

    if (include_past_baseline) dimensions.y += y_under;
    return dimensions;
}

bool neko_asset_mesh_load_from_file(const_str path, void *out, neko_asset_mesh_decl_t *decl, void *data_out, size_t data_size) {
    // Cast mesh data to use
    neko_asset_mesh_t *mesh = (neko_asset_mesh_t *)out;

    if (!neko_platform_file_exists(path)) {
        neko_println("Warning:MeshLoadFromFile:File does not exist: %s", path);
        return false;
    }

    // Mesh data to fill out
    u32 mesh_count = 0;
    neko_asset_mesh_raw_data_t *meshes = NULL;

    // Get file extension from path
    neko_transient_buffer(file_ext, 32);
    neko_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (neko_string_compare_equal(file_ext, "gltf")) {
        // neko_util_load_gltf_data_from_file(path, decl, &meshes, &mesh_count);
        NEKO_ASSERT(false);
    } else {
        neko_println("Warning:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return false;
    }

    // For now, handle meshes with only single mesh count
    if (mesh_count != 1) {
        // Error
        // Free all the memory
        return false;
    }

    // Process all mesh data, add meshes
    for (u32 i = 0; i < mesh_count; ++i) {
        neko_asset_mesh_raw_data_t *m = &meshes[i];

        for (u32 p = 0; p < m->prim_count; ++p) {
            // Construct primitive
            neko_asset_mesh_primitive_t prim = NEKO_DEFAULT_VAL();
            prim.count = m->index_sizes[p] / sizeof(u16);

            // Vertex buffer decl
            neko_render_vertex_buffer_desc_t vdesc = NEKO_DEFAULT_VAL();
            vdesc.data = m->vertices[p];
            vdesc.size = m->vertex_sizes[p];

            // Construct vertex buffer for primitive
            prim.vbo = neko_render_vertex_buffer_create(vdesc);

            // Index buffer decl
            neko_render_index_buffer_desc_t idesc = NEKO_DEFAULT_VAL();
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = neko_render_index_buffer_create(idesc);

            // Add primitive to mesh
            neko_dyn_array_push(mesh->primitives, prim);
        }
    }

    // Free all mesh data
    return true;
}

/*==========================
// LZ77
==========================*/

enum {
    NEKO_LZ_EXCESS = 16,

    NEKO_LZ_WINDOW_BITS = 17,  // Hard-coded
    NEKO_LZ_WINDOW_SIZE = 1 << NEKO_LZ_WINDOW_BITS,
    NEKO_LZ_WINDOW_MASK = NEKO_LZ_WINDOW_SIZE - 1,

    NEKO_LZ_MIN_MATCH = 4,

    NEKO_LZ_HASH_BITS = 19,
    NEKO_LZ_HASH_SIZE = 1 << NEKO_LZ_HASH_BITS,
    NEKO_LZ_NIL = -1,
};

typedef struct NEKO_LZ_WORKMEM {
    int HashTable[NEKO_LZ_HASH_SIZE];
    int Prev[NEKO_LZ_WINDOW_SIZE];
} NEKO_LZ_WORKMEM;

// Utils

NEKO_STATIC_INLINE u16 UnalignedLoad16(const void *p) { return *(const u16 *)(p); }
NEKO_STATIC_INLINE u32 UnalignedLoad32(const void *p) { return *(const u32 *)(p); }
NEKO_STATIC_INLINE void UnalignedStore16(void *p, u16 x) { *(u16 *)(p) = x; }
NEKO_STATIC_INLINE void UnalignedCopy64(void *d, const void *s) { *(u64 *)(d) = *(const u64 *)(s); }

NEKO_STATIC_INLINE void __neko_ulz_wild_copy(u8 *d, const u8 *s, int n) {
    UnalignedCopy64(d, s);

    for (int i = 8; i < n; i += 8) UnalignedCopy64(d + i, s + i);
}

NEKO_STATIC_INLINE u32 __neko_ulz_hash32(const void *p) { return (UnalignedLoad32(p) * 0x9E3779B9) >> (32 - NEKO_LZ_HASH_BITS); }

NEKO_STATIC_INLINE void __neko_ulz_encode_mod(u8 **p, u32 x) {
    while (x >= 128) {
        x -= 128;
        *(*p)++ = 128 + (x & 127);
        x >>= 7;
    }
    *(*p)++ = x;
}

NEKO_STATIC_INLINE u32 __neko_ulz_decode_mod(const u8 **p) {
    u32 x = 0;
    for (int i = 0; i <= 21; i += 7) {
        const u32 c = *(*p)++;
        x += c << i;
        if (c < 128) break;
    }
    return x;
}

// LZ77

NEKO_STATIC int __neko_ulz_compress_fast(const u8 *in, int inlen, u8 *out, int outlen) {
    NEKO_LZ_WORKMEM *u = (NEKO_LZ_WORKMEM *)neko_realloc(0, sizeof(NEKO_LZ_WORKMEM));

    for (int i = 0; i < NEKO_LZ_HASH_SIZE; ++i) u->HashTable[i] = NEKO_LZ_NIL;

    u8 *op = out;
    int anchor = 0;

    int p = 0;
    while (p < inlen) {
        int best_len = 0;
        int dist = 0;

        const int max_match = inlen - p;
        if (max_match >= NEKO_LZ_MIN_MATCH) {
            const int limit = (p - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (p - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;

            const u32 h = __neko_ulz_hash32(&in[p]);
            int s = u->HashTable[h];
            u->HashTable[h] = p;

            if (s > limit && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[p])) {
                int len = NEKO_LZ_MIN_MATCH;
                while (len < max_match && in[s + len] == in[p + len]) ++len;

                best_len = len;
                dist = p - s;
            }
        }

        if (best_len == NEKO_LZ_MIN_MATCH && (p - anchor) >= (7 + 128)) best_len = 0;

        if (best_len >= NEKO_LZ_MIN_MATCH) {
            const int len = best_len - NEKO_LZ_MIN_MATCH;
            const int token = ((dist >> 12) & 16) + (len < 15 ? len : 15);

            if (anchor != p) {
                const int run = p - anchor;
                if (run >= 7) {
                    *op++ = (7 << 5) + token;
                    __neko_ulz_encode_mod(&op, run - 7);
                } else
                    *op++ = (run << 5) + token;

                __neko_ulz_wild_copy(op, &in[anchor], run);
                op += run;
            } else
                *op++ = token;

            if (len >= 15) __neko_ulz_encode_mod(&op, len - 15);

            UnalignedStore16(op, dist);
            op += 2;

            anchor = p + best_len;
            ++p;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            u->HashTable[__neko_ulz_hash32(&in[p])] = p++;
            p = anchor;
        } else
            ++p;
    }

    if (anchor != p) {
        const int run = p - anchor;
        if (run >= 7) {
            *op++ = 7 << 5;
            __neko_ulz_encode_mod(&op, run - 7);
        } else
            *op++ = run << 5;

        __neko_ulz_wild_copy(op, &in[anchor], run);
        op += run;
    }

    neko_realloc(u, 0);
    return op - out;
}

NEKO_STATIC int __neko_ulz_compress(const u8 *in, int inlen, u8 *out, int outlen, int level) {
    if (level < 1 || level > 9) return 0;
    const int max_chain = (level < 9) ? 1 << level : 1 << 13;

    NEKO_LZ_WORKMEM *u = (NEKO_LZ_WORKMEM *)neko_realloc(0, sizeof(NEKO_LZ_WORKMEM));
    for (int i = 0; i < NEKO_LZ_HASH_SIZE; ++i) u->HashTable[i] = NEKO_LZ_NIL;

    u8 *op = out;
    int anchor = 0;

    int p = 0;
    while (p < inlen) {
        int best_len = 0;
        int dist = 0;

        const int max_match = inlen - p;
        if (max_match >= NEKO_LZ_MIN_MATCH) {
            const int limit = (p - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (p - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;
            int chainlen = max_chain;

            int s = u->HashTable[__neko_ulz_hash32(&in[p])];
            while (s > limit) {
                if (in[s + best_len] == in[p + best_len] && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[p])) {
                    int len = NEKO_LZ_MIN_MATCH;
                    while (len < max_match && in[s + len] == in[p + len]) ++len;

                    if (len > best_len) {
                        best_len = len;
                        dist = p - s;

                        if (len == max_match) break;
                    }
                }

                if (--chainlen == 0) break;

                s = u->Prev[s & NEKO_LZ_WINDOW_MASK];
            }
        }

        if (best_len == NEKO_LZ_MIN_MATCH && (p - anchor) >= (7 + 128)) best_len = 0;

        if (level >= 5 && best_len >= NEKO_LZ_MIN_MATCH && best_len < max_match && (p - anchor) != 6) {
            const int x = p + 1;
            const int target_len = best_len + 1;

            const int limit = (x - NEKO_LZ_WINDOW_SIZE) > NEKO_LZ_NIL ? (x - NEKO_LZ_WINDOW_SIZE) : NEKO_LZ_NIL;
            int chainlen = max_chain;

            int s = u->HashTable[__neko_ulz_hash32(&in[x])];
            while (s > limit) {
                if (in[s + best_len] == in[x + best_len] && UnalignedLoad32(&in[s]) == UnalignedLoad32(&in[x])) {
                    int len = NEKO_LZ_MIN_MATCH;
                    while (len < target_len && in[s + len] == in[x + len]) ++len;

                    if (len == target_len) {
                        best_len = 0;
                        break;
                    }
                }

                if (--chainlen == 0) break;

                s = u->Prev[s & NEKO_LZ_WINDOW_MASK];
            }
        }

        if (best_len >= NEKO_LZ_MIN_MATCH) {
            const int len = best_len - NEKO_LZ_MIN_MATCH;
            const int token = ((dist >> 12) & 16) + (len < 15 ? len : 15);

            if (anchor != p) {
                const int run = p - anchor;
                if (run >= 7) {
                    *op++ = (7 << 5) + token;
                    __neko_ulz_encode_mod(&op, run - 7);
                } else
                    *op++ = (run << 5) + token;

                __neko_ulz_wild_copy(op, &in[anchor], run);
                op += run;
            } else
                *op++ = token;

            if (len >= 15) __neko_ulz_encode_mod(&op, len - 15);

            UnalignedStore16(op, dist);
            op += 2;

            while (best_len-- != 0) {
                const u32 h = __neko_ulz_hash32(&in[p]);
                u->Prev[p & NEKO_LZ_WINDOW_MASK] = u->HashTable[h];
                u->HashTable[h] = p++;
            }
            anchor = p;
        } else {
            const u32 h = __neko_ulz_hash32(&in[p]);
            u->Prev[p & NEKO_LZ_WINDOW_MASK] = u->HashTable[h];
            u->HashTable[h] = p++;
        }
    }

    if (anchor != p) {
        const int run = p - anchor;
        if (run >= 7) {
            *op++ = 7 << 5;
            __neko_ulz_encode_mod(&op, run - 7);
        } else
            *op++ = run << 5;

        __neko_ulz_wild_copy(op, &in[anchor], run);
        op += run;
    }

    neko_realloc(u, 0);
    return op - out;
}

NEKO_STATIC int __neko_ulz_decompress(const u8 *in, int inlen, u8 *out, int outlen) {
    u8 *op = out;
    const u8 *ip = in;
    const u8 *ip_end = ip + inlen;
    const u8 *op_end = op + outlen;

    while (ip < ip_end) {
        const int token = *ip++;

        if (token >= 32) {
            int run = token >> 5;
            if (run == 7) run += __neko_ulz_decode_mod(&ip);
            if ((op_end - op) < run || (ip_end - ip) < run)  // Overrun check
                return 0;

            __neko_ulz_wild_copy(op, ip, run);
            op += run;
            ip += run;
            if (ip >= ip_end) break;
        }

        int len = (token & 15) + NEKO_LZ_MIN_MATCH;
        if (len == (15 + NEKO_LZ_MIN_MATCH)) len += __neko_ulz_decode_mod(&ip);
        if ((op_end - op) < len)  // Overrun check
            return 0;

        const int dist = ((token & 16) << 12) + UnalignedLoad16(ip);
        ip += 2;
        u8 *cp = op - dist;
        if ((op - out) < dist)  // Range check
            return 0;

        if (dist >= 8) {
            __neko_ulz_wild_copy(op, cp, len);
            op += len;
        } else {
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            *op++ = *cp++;
            while (len-- != 4) *op++ = *cp++;
        }
    }

    return (ip == ip_end) ? op - out : 0;
}

u32 neko_lz_encode(const void *in, u32 inlen, void *out, u32 outlen, u32 flags) {
    int level = flags > 9 ? 9 : flags < 0 ? 0 : flags;  // [0..(6)..9]
    int rc = level ? __neko_ulz_compress((u8 *)in, (int)inlen, (u8 *)out, (int)outlen, level) : __neko_ulz_compress_fast((u8 *)in, (int)inlen, (u8 *)out, (int)outlen);
    return (u32)rc;
}
u32 neko_lz_decode(const void *in, u32 inlen, void *out, u32 outlen) { return (u32)__neko_ulz_decompress((u8 *)in, (int)inlen, (u8 *)out, (int)outlen); }
u32 neko_lz_bounds(u32 inlen, u32 flags) { return inlen + inlen / 255 + 16; }

/*==========================
// NEKO_PACK
==========================*/

#pragma region packer

// #include "deps/lz4/lz4.h"

#if __linux__ || __APPLE__
#define openFile(filePath, mode) fopen(filePath, mode)
#define seekFile(file, offset, whence) fseeko(file, offset, whence)
#define tellFile(file) ftello(file)
#elif _WIN32
static inline FILE *openFile(const char *filePath, const char *mode) {
    FILE *file;
    errno_t error = fopen_s(&file, filePath, mode);
    if (error != 0) return NULL;
    return file;
}

#define seekFile(file, offset, whence) _fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
#else
#error Unsupported operating system
#endif

#define closeFile(file) fclose(file)

NEKO_PRIVATE(void) destroy_pack_items(u64 item_count, pack_item *items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) neko_free(items[i].path);
    neko_free(items);
}

NEKO_PRIVATE(int) create_pack_items(FILE *packFile, u64 item_count, pack_item **_items) {
    NEKO_ASSERT(packFile);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(_items);

    pack_item *items = (pack_item *)neko_malloc(item_count * sizeof(pack_item));

    if (!items) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    for (u64 i = 0; i < item_count; i++) {
        pack_iteminfo info;

        size_t result = fread(&info, sizeof(pack_iteminfo), 1, packFile);

        if (result != 1) {
            destroy_pack_items(i, items);
            return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        if (info.data_size == 0 || info.path_size == 0) {
            destroy_pack_items(i, items);
            return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        char *path = (char *)neko_malloc((info.path_size + 1) * sizeof(char));

        if (!path) {
            destroy_pack_items(i, items);
            return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
        }

        result = fread(path, sizeof(char), info.path_size, packFile);

        path[info.path_size] = 0;

        if (result != info.path_size) {
            destroy_pack_items(i, items);
            return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        s64 fileOffset = info.zip_size > 0 ? info.zip_size : info.data_size;

        int seekResult = seekFile(packFile, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            destroy_pack_items(i, items);
            return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        pack_item *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return 0;
}

int neko_pack_read(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory, neko_packreader_t *pack_reader) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(pack_reader);

    // neko_packreader_t *pack = (neko_packreader_t *)neko_calloc(1, sizeof(neko_packreader_t));

    if (!pack_reader) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    memset(pack_reader, 0, sizeof(neko_packreader_t));

    pack_reader->zip_buffer = NULL;
    pack_reader->zip_size = 0;

    char *path;

    path = (char *)file_path;

    FILE *file = openFile(path, "rb");

    if (!file) {
        neko_pack_destroy(pack_reader);
        return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
    }

    pack_reader->file = file;

    char header[neko_pack_head_size];
    s32 buildnum;

    size_t result = fread(header, sizeof(u8), neko_pack_head_size, file);
    result += fread(&buildnum, sizeof(s32), 1, file);

    // 检查文件头大小
    if (result != neko_pack_head_size + 1) {
        neko_pack_destroy(pack_reader);
        return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    // 检查文件头
    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K' || header[4] != 0 || header[5] != 0) {
        neko_pack_destroy(pack_reader);
        return -1;
    }

    // Skipping PATCH version check
    if (header[7] != !neko_little_endian) {
        neko_pack_destroy(pack_reader);
        return -1; /*BAD_FILE_ENDIANNESS_PACK_RESULT*/
    }

    u64 item_count;

    result = fread(&item_count, sizeof(u64), 1, file);

    if (result != 1) {
        neko_pack_destroy(pack_reader);
        return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    if (item_count == 0) {
        neko_pack_destroy(pack_reader);
        return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
    }

    pack_item *items;

    int pack_result = create_pack_items(file, item_count, &items);

    if (pack_result != 0) {
        neko_pack_destroy(pack_reader);
        ;
        return pack_result;
    }

    pack_reader->item_count = item_count;
    pack_reader->items = items;

    u8 *data_buffer;

    if (data_buffer_capacity > 0) {
        data_buffer = (u8 *)neko_malloc(data_buffer_capacity * sizeof(u8));

        if (!data_buffer) {
            neko_pack_destroy(pack_reader);
            return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
        }
    } else {
        data_buffer = NULL;
    }

    pack_reader->data_buffer = data_buffer;
    pack_reader->data_size = data_buffer_capacity;

    NEKO_TRACE("load pack %s buildnum: %d (engine %d)", neko_fs_get_filename(file_path), buildnum, neko_buildnum());

    //*pack_reader = pack;
    return 0;
}
void neko_pack_destroy(neko_packreader_t *pack_reader) {
    if (!pack_reader) return;

    if (pack_reader->file_ref_count != 0) {
        NEKO_WARN("assets loader leaks detected %d refs", pack_reader->file_ref_count);
    }

    neko_free(pack_reader->data_buffer);
    neko_free(pack_reader->zip_buffer);
    destroy_pack_items(pack_reader->item_count, pack_reader->items);
    if (pack_reader->file) closeFile(pack_reader->file);
    // neko_free(pack_reader);
}

u64 neko_pack_item_count(neko_packreader_t *pack_reader) {
    NEKO_ASSERT(pack_reader);
    return pack_reader->item_count;
}

NEKO_PRIVATE(int) neko_compare_pack_items(const void *_a, const void *_b) {
    const pack_item *a = (pack_item *)_a;
    const pack_item *b = (pack_item *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

b8 neko_pack_item_index(neko_packreader_t *pack_reader, const_str path, u64 *index) {
    NEKO_ASSERT(pack_reader);
    NEKO_ASSERT(path);
    NEKO_ASSERT(index);
    NEKO_ASSERT(strlen(path) <= UINT8_MAX);

    pack_item *search_item = &pack_reader->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    pack_item *item = (pack_item *)bsearch(search_item, pack_reader->items, pack_reader->item_count, sizeof(pack_item), neko_compare_pack_items);

    if (!item) return false;

    *index = item - pack_reader->items;
    return true;
}

u32 neko_pack_item_size(neko_packreader_t *pack_reader, u64 index) {
    NEKO_ASSERT(pack_reader);
    NEKO_ASSERT(index < pack_reader->item_count);
    return pack_reader->items[index].info.data_size;
}

const_str neko_pack_item_path(neko_packreader_t *pack_reader, u64 index) {
    NEKO_ASSERT(pack_reader);
    NEKO_ASSERT(index < pack_reader->item_count);
    return pack_reader->items[index].path;
}

int neko_pack_item_data_with_index(neko_packreader_t *pack_reader, u64 index, const u8 **data, u32 *size) {
    NEKO_ASSERT(pack_reader);
    NEKO_ASSERT(index < pack_reader->item_count);
    NEKO_ASSERT(data);
    NEKO_ASSERT(size);

    pack_iteminfo info = pack_reader->items[index].info;
    u8 *data_buffer = pack_reader->data_buffer;

    if (data_buffer) {
        if (info.data_size > pack_reader->data_size) {
            data_buffer = (u8 *)neko_realloc(data_buffer, info.data_size * sizeof(u8));

            if (!data_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            pack_reader->data_buffer = data_buffer;
            pack_reader->data_size = info.data_size;
        }
    } else {
        data_buffer = (u8 *)neko_malloc(info.data_size * sizeof(u8));

        if (!data_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

        pack_reader->data_buffer = data_buffer;
        pack_reader->data_size = info.data_size;
    }

    u8 *zip_buffer = pack_reader->zip_buffer;

    if (zip_buffer) {
        if (info.zip_size > pack_reader->zip_size) {
            zip_buffer = (u8 *)neko_realloc(zip_buffer, info.zip_size * sizeof(u8));

            if (!zip_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            pack_reader->zip_buffer = zip_buffer;
            pack_reader->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            zip_buffer = (u8 *)neko_malloc(info.zip_size * sizeof(u8));

            if (!zip_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            pack_reader->zip_buffer = zip_buffer;
            pack_reader->zip_size = info.zip_size;
        }
    }

    FILE *file = pack_reader->file;

    s64 file_offset = (s64)(info.file_offset + sizeof(pack_iteminfo) + info.path_size);

    int seek_result = seekFile(file, file_offset, SEEK_SET);

    if (seek_result != 0) return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/

    if (info.zip_size > 0) {
        size_t result = fread(zip_buffer, sizeof(u8), info.zip_size, file);

        if (result != info.zip_size) return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/

        result = neko_lz_decode(zip_buffer, info.zip_size, data_buffer, info.data_size);

        NEKO_TRACE("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

        if (result < 0 || result != info.data_size) {
            return -1; /*FAILED_TO_DECOMPRESS_PACK_RESULT*/
        }
    } else {
        size_t result = fread(data_buffer, sizeof(u8), info.data_size, file);

        if (result != info.data_size) return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    //(*data) = data_buffer;
    (*size) = info.data_size;

    (*data) = neko_malloc(info.data_size);

    memcpy((void *)(*data), data_buffer, info.data_size);

    pack_reader->file_ref_count++;

    return 0;
}

int neko_pack_item_data(neko_packreader_t *pack_reader, const_str path, const u8 **data, u32 *size) {
    NEKO_ASSERT(pack_reader);
    NEKO_ASSERT(path);
    NEKO_ASSERT(data);
    NEKO_ASSERT(size);
    NEKO_ASSERT(strlen(path) <= UINT8_MAX);

    u64 index;

    if (!neko_pack_item_index(pack_reader, path, &index)) {
        return -1; /*FAILED_TO_GET_ITEM_PACK_RESULT*/
    }

    return neko_pack_item_data_with_index(pack_reader, index, data, size);
}

void neko_pack_item_free(neko_packreader_t *pack_reader, void *data) {
    neko_free(data);
    pack_reader->file_ref_count--;
}

void neko_pack_free_buffers(neko_packreader_t *pack_reader) {
    NEKO_ASSERT(pack_reader);
    neko_free(pack_reader->data_buffer);
    neko_free(pack_reader->zip_buffer);
    pack_reader->data_buffer = NULL;
    pack_reader->zip_buffer = NULL;
}

NEKO_PRIVATE(void) neko_pack_remove_item(u64 item_count, pack_item *pack_items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && pack_items));

    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

int neko_pack_unzip(const_str file_path, b8 print_progress) {
    NEKO_ASSERT(file_path);

    neko_packreader_t pack_reader;

    int pack_result = neko_pack_read(file_path, 128, false, &pack_reader);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pack_reader.item_count;
    pack_item *items = pack_reader.items;

    for (u64 i = 0; i < item_count; i++) {
        pack_item *item = &items[i];

        if (print_progress) {
            NEKO_INFO("Unpacking %s", item->path);
        }

        const u8 *data_buffer;
        u32 data_size;

        pack_result = neko_pack_item_data_with_index(&pack_reader, i, &data_buffer, &data_size);

        if (pack_result != 0) {
            neko_pack_remove_item(i, items);
            neko_pack_destroy(&pack_reader);
            return pack_result;
        }

        u8 path_size = item->info.path_size;

        char item_path[UINT8_MAX + 1];

        memcpy(item_path, item->path, path_size * sizeof(char));
        item_path[path_size] = 0;

        // 解压的时候路径节替换为 '-'
        for (u8 j = 0; j < path_size; j++)
            if (item_path[j] == '/' || item_path[j] == '\\' || (item_path[j] == '.' && j == 0)) item_path[j] = '-';

        FILE *item_file = openFile(item_path, "wb");

        if (!item_file) {
            neko_pack_remove_item(i, items);
            neko_pack_destroy(&pack_reader);
            return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        size_t result = fwrite(data_buffer, sizeof(u8), data_size, item_file);

        closeFile(item_file);

        if (result != data_size) {
            neko_pack_remove_item(i, items);
            neko_pack_destroy(&pack_reader);
            return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        if (print_progress) {
            u32 raw_file_size = item->info.data_size;
            u32 zip_file_size = item->info.zip_size > 0 ? item->info.zip_size : item->info.data_size;

            total_raw_size += raw_file_size;
            total_zip_size += zip_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", raw_file_size, zip_file_size, progress);
        }
    }

    neko_pack_destroy(&pack_reader);

    if (print_progress) {
        printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return 0;
}

NEKO_PRIVATE(int) neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, b8 print_progress) {
    NEKO_ASSERT(pack_file);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(item_paths);

    u32 buffer_size = 128;  // 提高初始缓冲大小 修复 neko_realloc 异常释放

    u8 *item_data = (u8 *)neko_malloc(sizeof(u8) * buffer_size);
    if (!item_data) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u8 *zip_data = (u8 *)neko_malloc(sizeof(u8) * buffer_size);
    if (!zip_data) {
        neko_free(item_data);
        return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
    }

    u64 total_zip_size = 0, total_raw_size = 0;

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (print_progress) {
            printf("Packing \"%s\" file. ", item_path);
            fflush(stdout);
        }

        size_t path_size = strlen(item_path);

        if (path_size > UINT8_MAX) {
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        FILE *item_file = openFile(item_path, "rb");

        if (!item_file) {
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        int seek_result = seekFile(item_file, 0, SEEK_END);

        if (seek_result != 0) {
            closeFile(item_file);
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        u64 item_size = (u64)tellFile(item_file);

        if (item_size == 0 || item_size > UINT32_MAX) {
            closeFile(item_file);
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        seek_result = seekFile(item_file, 0, SEEK_SET);

        if (seek_result != 0) {
            closeFile(item_file);
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        if (item_size > buffer_size) {
            u8 *new_buffer = (u8 *)neko_realloc(item_data, item_size * sizeof(u8));

            if (!new_buffer) {
                closeFile(item_file);
                neko_free(zip_data);
                neko_free(item_data);
                return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            item_data = new_buffer;

            new_buffer = (u8 *)neko_realloc(zip_data, item_size * sizeof(u8));

            if (!new_buffer) {
                closeFile(item_file);
                neko_free(zip_data);
                neko_free(item_data);
                return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            zip_data = new_buffer;
        }

        size_t result = fread(item_data, sizeof(u8), item_size, item_file);

        closeFile(item_file);

        if (result != item_size) {
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
        }

        size_t zip_size;

        if (item_size > 1) {

            const int max_dst_size = neko_lz_bounds(item_size, 0);
            zip_size = neko_lz_encode(item_data, item_size, zip_data, max_dst_size, 9);

            if (zip_size <= 0 || zip_size >= item_size) {
                zip_size = 0;
            }
        } else {
            zip_size = 0;
        }

        s64 file_offset = tellFile(pack_file);

        pack_iteminfo info = {
                (u32)zip_size,
                (u32)item_size,
                (u64)file_offset,
                (u8)path_size,
        };

        result = fwrite(&info, sizeof(pack_iteminfo), 1, pack_file);

        if (result != 1) {
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        result = fwrite(item_path, sizeof(char), info.path_size, pack_file);

        if (result != info.path_size) {
            neko_free(zip_data);
            neko_free(item_data);
            return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, pack_file);

            if (result != zip_size) {
                neko_free(zip_data);
                neko_free(item_data);
                return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
            }
        } else {
            result = fwrite(item_data, sizeof(u8), item_size, pack_file);

            if (result != item_size) {
                neko_free(zip_data);
                neko_free(item_data);
                return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
            }
        }

        if (print_progress) {
            u32 zip_file_size = zip_size > 0 ? (u32)zip_size : (u32)item_size;
            u32 raw_file_size = (u32)item_size;

            total_zip_size += zip_file_size;
            total_raw_size += raw_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", zip_file_size, raw_file_size, progress);
            fflush(stdout);
        }
    }

    neko_free(zip_data);
    neko_free(item_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size, compression);
    }

    return 0;
}

NEKO_PRIVATE(int) neko_pack_compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

int neko_pack_build(const_str file_path, u64 file_count, const_str *file_paths, b8 print_progress) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(file_count > 0);
    NEKO_ASSERT(file_paths);

    char **item_paths = (char **)neko_malloc(file_count * sizeof(char *));

    if (!item_paths) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u64 item_count = 0;

    for (u64 i = 0; i < file_count; i++) {
        b8 already_added = false;

        for (u64 j = 0; j < item_count; j++) {
            if (i != j && strcmp(file_paths[i], item_paths[j]) == 0) already_added = true;
        }

        if (!already_added) item_paths[item_count++] = (char *)file_paths[i];
    }

    qsort(item_paths, item_count, sizeof(char *), neko_pack_compare_item_paths);

    FILE *pack_file = openFile(file_path, "wb");

    if (!pack_file) {
        neko_free(item_paths);
        return -1; /*FAILED_TO_CREATE_FILE_PACK_RESULT*/
    }

    char header[neko_pack_head_size] = {
            'P', 'A', 'C', 'K', 0, 0, 0, !neko_little_endian,
    };

    s32 buildnum = neko_buildnum();

    size_t write_result = fwrite(header, sizeof(u8), neko_pack_head_size, pack_file);
    write_result += fwrite(&buildnum, sizeof(s32), 1, pack_file);

    if (write_result != neko_pack_head_size + 1) {
        neko_free(item_paths);
        closeFile(pack_file);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    write_result = fwrite(&item_count, sizeof(u64), 1, pack_file);

    if (write_result != 1) {
        neko_free(item_paths);
        closeFile(pack_file);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    int pack_result = neko_write_pack_items(pack_file, item_count, item_paths, print_progress);

    neko_free(item_paths);
    closeFile(pack_file);

    if (pack_result != 0) {
        remove(file_path);
        return pack_result;
    }

    return 0;
}

int neko_pack_info(const_str file_path, u8 *pack_version, b8 *isLittleEndian, u64 *_item_count) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(pack_version);
    NEKO_ASSERT(isLittleEndian);
    NEKO_ASSERT(_item_count);

    FILE *file = openFile(file_path, "rb");

    if (!file) return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/

    char header[neko_pack_head_size];
    s32 buildnum;

    size_t result = fread(header, sizeof(u8), neko_pack_head_size, file);
    result += fread(&buildnum, sizeof(s32), 1, file);

    if (result != neko_pack_head_size + 1) {
        closeFile(file);
        return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        closeFile(file);
        return -1; /*BAD_FILE_TYPE_PACK_RESULT*/
    }

    u64 item_count;

    result = fread(&item_count, sizeof(u64), 1, file);

    closeFile(file);

    if (result != 1) return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/

    *pack_version = header[4];
    /*    *pack_version = header[5];
     *pack_version = header[6];*/
    *isLittleEndian = !header[7];
    *_item_count = item_count;
    return 0;
}

#pragma endregion

#pragma region xml

#define __neko_xml_expect_not_end(c_)           \
    if (!*(c_)) {                               \
        neko_xml_emit_error("Unexpected end."); \
        return NULL;                            \
    }

typedef struct neko_xml_entity_t {
    char character;
    const_str name;
} neko_xml_entity_t;

static neko_xml_entity_t g_neko_xml_entities[] = {{'&', "&amp;"}, {'\'', "&apos;"}, {'"', "&quot;"}, {'<', "&lt;"}, {'>', "&gt;"}};
static const_str g_neko_xml_error = NULL;

static void neko_xml_emit_error(const_str error) { g_neko_xml_error = error; }

static char *neko_xml_copy_string(const_str str, u32 len) {
    char *r = (char *)neko_malloc(len + 1);
    if (!r) {
        neko_xml_emit_error("Out of memory!");
        return NULL;
    }
    r[len] = '\0';

    for (u32 i = 0; i < len; i++) {
        r[i] = str[i];
    }

    return r;
}

static bool neko_xml_string_is_decimal(const_str str, u32 len) {
    u32 i = 0;
    if (str[0] == '-') i++;

    bool used_dot = false;

    for (; i < len; i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            if (c == '.' && !used_dot) {
                used_dot = true;
                continue;
            }
            return false;
        }
    }

    return true;
}

static bool neko_xml_string_equal(const_str str_a, u32 len, const_str str_b) {
    for (u32 i = 0; i < len; i++) {
        if (str_a[i] != str_b[i]) return false;
    }

    return true;
}

static u64 neko_xml_hash_string(const_str str, u32 len) {
    u64 hash = 0, x = 0;

    for (u32 i = 0; i < len; i++) {
        hash = (hash << 4) + str[i];
        if ((x = hash & 0xF000000000LL) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash & 0x7FFFFFFFFF);
}

static void neko_xml_node_free(neko_xml_node_t *node) {
    for (neko_hash_table_iter it = neko_hash_table_iter_new(node->attributes); neko_hash_table_iter_valid(node->attributes, it); neko_hash_table_iter_advance(node->attributes, it)) {
        neko_xml_attribute_t attrib = neko_hash_table_iter_get(node->attributes, it);

        if (attrib.type == NEKO_XML_ATTRIBUTE_STRING) {
            neko_free(attrib.value.string);
        }
    }

    for (u32 i = 0; i < neko_dyn_array_size(node->children); i++) {
        neko_xml_node_free(node->children + i);
    }

    neko_free(node->name);
    neko_free(node->text);
    neko_hash_table_free(node->attributes);
    neko_dyn_array_free(node->children);
}

static char *neko_xml_process_text(const_str start, u32 length) {
    char *r = (char *)neko_malloc(length + 1);

    u32 len_sub = 0;

    for (u32 i = 0, ri = 0; i < length; i++, ri++) {
        bool changed = false;
        if (start[i] == '&') {
            for (u32 ii = 0; ii < sizeof(g_neko_xml_entities) / sizeof(*g_neko_xml_entities); ii++) {
                u32 ent_len = neko_string_length(g_neko_xml_entities[ii].name);
                if (neko_xml_string_equal(start + i, ent_len, g_neko_xml_entities[ii].name)) {
                    r[ri] = g_neko_xml_entities[ii].character;
                    i += ent_len - 1;
                    len_sub += ent_len - 1;
                    changed = true;
                    break;
                }
            }
        }

        if (!changed) r[ri] = start[i];
    }

    r[length - len_sub] = '\0';

    return r;
}

// 解析XML块 返回块中的节点数组
static neko_dyn_array(neko_xml_node_t) neko_xml_parse_block(const_str start, u32 length) {
    neko_dyn_array(neko_xml_node_t) r = neko_dyn_array_new(neko_xml_node_t);

    bool is_inside = false;

    for (const_str c = start; *c && c < start + length; c++) {
        if (*c == '<') {
            c++;
            __neko_xml_expect_not_end(c);

            if (*c == '?')  // 跳过XML头
            {
                c++;
                __neko_xml_expect_not_end(c);
                while (*c != '>') {
                    c++;
                    __neko_xml_expect_not_end(c);
                }
                continue;
            } else if (neko_xml_string_equal(c, 3, "!--"))  // 跳过注释
            {
                c++;
                __neko_xml_expect_not_end(c);
                c++;
                __neko_xml_expect_not_end(c);
                c++;
                __neko_xml_expect_not_end(c);
                while (!neko_xml_string_equal(c, 3, "-->")) {
                    c++;
                    __neko_xml_expect_not_end(c);
                }

                continue;
            }

            if (is_inside && *c == '/')
                is_inside = false;
            else
                is_inside = true;

            const_str node_name_start = c;
            u32 node_name_len = 0;

            neko_xml_node_t current_node = {0};

            current_node.attributes = neko_hash_table_new(u64, neko_xml_attribute_t);
            current_node.children = neko_dyn_array_new(neko_xml_node_t);

            if (is_inside) {
                for (; *c != '>' && *c != ' ' && *c != '/'; c++) node_name_len++;

                if (*c != '>') {
                    while (*c != '>' && *c != '/') {
                        while (neko_token_char_is_white_space(*c)) c++;

                        const_str attrib_name_start = c;
                        u32 attrib_name_len = 0;

                        while (neko_token_char_is_alpha(*c) || neko_token_char_is_numeric(*c) || *c == '_') {
                            c++;
                            attrib_name_len++;
                            __neko_xml_expect_not_end(c);
                        }

                        while (*c != '"') {
                            c++;
                            __neko_xml_expect_not_end(c);
                        }

                        c++;
                        __neko_xml_expect_not_end(c);

                        const_str attrib_text_start = c;
                        u32 attrib_text_len = 0;

                        while (*c != '"') {
                            c++;
                            attrib_text_len++;
                            __neko_xml_expect_not_end(c);
                        }

                        c++;
                        __neko_xml_expect_not_end(c);

                        neko_xml_attribute_t attrib = {0};
                        attrib.name = neko_xml_copy_string(attrib_name_start, attrib_name_len);

                        if (neko_xml_string_is_decimal(attrib_text_start, attrib_text_len)) {
                            attrib.type = NEKO_XML_ATTRIBUTE_NUMBER;
                            attrib.value.number = strtod(attrib_text_start, NULL);
                        } else if (neko_xml_string_equal(attrib_text_start, attrib_text_len, "true")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = true;
                        } else if (neko_xml_string_equal(attrib_text_start, attrib_text_len, "false")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = false;
                        } else {
                            attrib.type = NEKO_XML_ATTRIBUTE_STRING;
                            attrib.value.string = neko_xml_process_text(attrib_text_start, attrib_text_len);
                        }

                        neko_hash_table_insert(current_node.attributes, neko_xml_hash_string(attrib_name_start, attrib_name_len), attrib);
                    }
                }

                if (*c == '/')  // 对于没有任何文本的节点
                {
                    c++;
                    __neko_xml_expect_not_end(c);
                    current_node.name = neko_xml_copy_string(node_name_start, node_name_len);
                    neko_dyn_array_push(r, current_node);
                    is_inside = false;
                }
            } else {
                while (*c != '>') {
                    c++;
                    __neko_xml_expect_not_end(c);
                }
            }

            c++;
            __neko_xml_expect_not_end(c);

            if (is_inside) {
                const_str text_start = c;
                u32 text_len = 0;

                const_str end_start = c;
                u32 end_len = 0;

                current_node.name = neko_xml_copy_string(node_name_start, node_name_len);

                for (u32 i = 0; i < length; i++) {
                    if (*c == '<' && *(c + 1) == '/') {
                        c++;
                        __neko_xml_expect_not_end(c);
                        c++;
                        __neko_xml_expect_not_end(c);
                        end_start = c;
                        end_len = 0;
                        while (*c != '>') {
                            end_len++;
                            c++;
                            __neko_xml_expect_not_end(c);
                        }

                        if (neko_xml_string_equal(end_start, end_len, current_node.name)) {
                            break;
                        } else {
                            text_len += end_len + 2;
                            continue;
                        }
                    }

                    c++;
                    text_len++;

                    __neko_xml_expect_not_end(c);
                }

                current_node.children = neko_xml_parse_block(text_start, text_len);
                if (neko_dyn_array_size(current_node.children) == 0)
                    current_node.text = neko_xml_process_text(text_start, text_len);
                else
                    current_node.text = neko_xml_copy_string(text_start, text_len);

                neko_dyn_array_push(r, current_node);

                c--;
            }
        }
    }

    return r;
}

neko_xml_document_t *neko_xml_parse_file(const_str path) {
    size_t size;
    char *source = neko_read_file_contents(path, "r", &size);

    if (!source) {
        neko_xml_emit_error("Failed to load xml file!");
        return NULL;
    }

    neko_xml_document_t *doc = neko_xml_parse(source);

    neko_safe_free(source);

    return doc;
}

neko_xml_document_t *neko_xml_parse(const_str source) {
    if (!source) return NULL;

    g_neko_xml_error = NULL;
    neko_xml_document_t *doc = (neko_xml_document_t *)neko_calloc(1, sizeof(neko_xml_document_t));
    if (!doc) {
        neko_xml_emit_error("Out of memory!");
        return NULL;
    }

    doc->nodes = neko_xml_parse_block(source, neko_string_length(source));

    if (g_neko_xml_error) {
        neko_xml_free(doc);
        return NULL;
    }

    return doc;
}

void neko_xml_free(neko_xml_document_t *document) {
    for (u32 i = 0; i < neko_dyn_array_size(document->nodes); i++) {
        neko_xml_node_free(document->nodes + i);
    }

    neko_dyn_array_free(document->nodes);
    neko_free(document);
}

neko_xml_attribute_t *neko_xml_find_attribute(neko_xml_node_t *node, const_str name) {
    if (!neko_hash_table_exists(node->attributes, neko_xml_hash_string(name, neko_string_length(name)))) {
        return NULL;
    } else {
        return neko_hash_table_getp(node->attributes, neko_xml_hash_string(name, neko_string_length(name)));
    }
}

neko_xml_node_t *neko_xml_find_node(neko_xml_document_t *doc, const_str name) {
    for (u32 i = 0; i < neko_dyn_array_size(doc->nodes); i++) {
        if (neko_string_compare_equal(name, doc->nodes[i].name)) {
            return doc->nodes + i;
        }
    }

    return NULL;
}

neko_xml_node_t *neko_xml_find_node_child(neko_xml_node_t *node, const_str name) {
    for (u32 i = 0; i < neko_dyn_array_size(node->children); i++) {
        if (neko_string_compare_equal(name, node->children[i].name)) {
            return node->children + i;
        }
    }

    return NULL;
}

const_str neko_xml_get_error() { return g_neko_xml_error; }

neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t *doc, const_str name) {
    neko_xml_node_iter_t it = {.doc = doc, .name = name, .idx = 0};

    return it;
}

neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t *parent, const_str name) {
    neko_xml_node_iter_t it = {.node = parent, .name = name, .idx = 0};

    return it;
}

bool neko_xml_node_iter_next(neko_xml_node_iter_t *iter) {
    if (iter->node) {
        for (u32 i = iter->idx; i < neko_dyn_array_size(iter->node->children); i++) {
            if (neko_string_compare_equal(iter->name, iter->node->children[i].name)) {
                iter->current = iter->node->children + i;
                iter->idx = i + 1;
                return true;
            }
        }

        return false;
    } else {
        for (u32 i = iter->idx; i < neko_dyn_array_size(iter->doc->nodes); i++) {
            if (neko_string_compare_equal(iter->name, iter->doc->nodes[i].name)) {
                iter->current = iter->doc->nodes + i;
                iter->idx = i + 1;
                return true;
            }
        }
        return false;
    }
}

#pragma endregion

#pragma region font

const char *neko_font_error_reason;

// cp1252 table and decode utf8 functions by Mitton a la TIGR
// https://bitbucket.org/rmitton/tigr/src/default/

// Converts 8-bit codepage entries into unicode code points for indices of 128-256
static int neko_font_cp1252[] = {
        0x20ac, 0xfffd, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xfffd, 0x017d, 0xfffd, 0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022,
        0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xfffd, 0x017e, 0x0178, 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab,
        0x00ac, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 0x00c0, 0x00c1,
        0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
        0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed,
        0x00ee, 0x00ef, 0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
};

const char *neko_font_decode_utf8(const char *text, int *cp) {
    unsigned char c = *text++;
    int extra = 0, min = 0;
    *cp = 0;
    if (c >= 0xF0) {
        *cp = c & 0x07;
        extra = 3;
        min = 0x10000;
    } else if (c >= 0xE0) {
        *cp = c & 0x0F;
        extra = 2;
        min = 0x800;
    } else if (c >= 0xC0) {
        *cp = c & 0x1F;
        extra = 1;
        min = 0x80;
    } else if (c >= 0x80) {
        *cp = 0xFFFD;
    } else
        *cp = c;
    while (extra--) {
        c = *text++;
        if ((c & 0xC0) != 0x80) {
            *cp = 0xFFFD;
            break;
        }
        (*cp) = ((*cp) << 6) | (c & 0x3F);
    }
    if (*cp < min) *cp = 0xFFFD;
    return text;
}

typedef struct neko_font_img_t {
    void *pix;
    int w, h;
    int stride;
} neko_font_img_t;

static const char *neko_font_get_pixel(neko_font_img_t *img, int x, int y) { return ((const char *)img->pix) + y * img->w * img->stride + x * img->stride; }

static int neko_font_is_border(neko_font_img_t *img, int x, int y) {
    const char *border_color = (const char *)img->pix;
    const char *pixel = neko_font_get_pixel(img, x, y);
    for (int i = 0; i < img->stride; ++i)
        if (pixel[i] != border_color[i]) return 0;
    return 1;
}

static void neko_font_scan(neko_font_img_t *img, int *x, int *y, int *row_height) {
    while (*y < img->h) {
        if (*x >= img->w) {
            *x = 0;
            (*y) += *row_height;
            *row_height = 1;
        }
        if (!neko_font_is_border(img, *x, *y)) return;
        (*x)++;
    }
}

#define NEKO_FONT_CHECK(X, Y)           \
    do {                                \
        if (!(X)) {                     \
            neko_font_error_reason = Y; \
            goto neko_font_err;         \
        }                               \
    } while (0)
#define NEKO_FONT_FAIL_IF(X)    \
    do {                        \
        if (X) {                \
            goto neko_font_err; \
        }                       \
    } while (0)

neko_font_t *neko_font_load(neko_font_u64 atlas_id, const void *pixels, int w, int h, int stride, void *mem_ctx, int codepage) {
    int font_height = 1;
    int x = 0, y = 0;

    // Used to squeeze UVs inward by 128th of a pixel.
    float w0 = 1.0f / (float)w;
    float h0 = 1.0f / (float)h;
    float div = 1.0f / 128.0f;
    float wTol = w0 * div;
    float hTol = h0 * div;

    // algorithm by Mitton a la TIGR
    // https://bitbucket.org/rmitton/tigr/src/default/
    neko_font_t *font = (neko_font_t *)neko_safe_malloc(sizeof(neko_font_t));
    font->codes = 0;
    font->glyphs = 0;
    font->mem_ctx = mem_ctx;
    font->atlas_w = w;
    font->atlas_h = h;
    neko_font_img_t img;
    img.pix = (void *)pixels;
    img.w = w;
    img.h = h;
    img.stride = stride;

    switch (codepage) {
        case 0:
            font->glyph_count = 128 - 32;
            break;
        case 1252:
            font->glyph_count = 256 - 32;
            break;
        default:
            NEKO_FONT_CHECK(0, "Unknown codepage encountered.");
    }
    font->codes = (int *)neko_safe_malloc(sizeof(int) * font->glyph_count);
    font->glyphs = (neko_font_glyph_t *)neko_safe_malloc(sizeof(neko_font_glyph_t) * font->glyph_count);
    font->atlas_id = atlas_id;
    font->kern = 0;

    for (int i = 32; i < font->glyph_count + 32; ++i) {
        neko_font_glyph_t *glyph = NULL;
        int w = 0, h = 0;
        neko_font_scan(&img, &x, &y, &font_height);
        NEKO_FONT_CHECK(y < img.w, "Unable to properly scan glyph width. Are the text borders drawn properly?");

        while (!neko_font_is_border(&img, x + w, y)) ++w;
        while (!neko_font_is_border(&img, x, y + h)) ++h;

        glyph = font->glyphs + i - 32;
        if (i < 128)
            font->codes[i - 32] = i;
        else if (codepage == 1252)
            font->codes[i - 32] = neko_font_cp1252[i - 128];
        else
            NEKO_FONT_CHECK(0, "Unknown glyph index found.");

        glyph->xadvance = w + 1;
        glyph->w = (float)w;
        glyph->h = (float)h;
        glyph->minx = x * w0 + wTol;
        glyph->maxx = (x + w) * w0 - wTol;
        glyph->miny = y * h0 + wTol;
        glyph->maxy = (y + h) * h0 - wTol;
        glyph->xoffset = 0;
        glyph->yoffset = 0;

        if (h > font_height) font_height = h;
        x += w;
    }

    font->font_height = font_height;

    // sort by codepoint for non-ascii code pages
    if (codepage) {
        for (int i = 1; i < font->glyph_count; ++i) {
            neko_font_glyph_t glyph = font->glyphs[i];
            int code = font->codes[i];
            int j = i;
            while (j > 0 && font->codes[j - 1] > code) {
                font->glyphs[j] = font->glyphs[j - 1];
                font->codes[j] = font->codes[j - 1];
                --j;
            }
            font->glyphs[j] = glyph;
            font->codes[j] = code;
        }
    }

    return font;

neko_font_err:
    neko_safe_free(font->glyphs);
    neko_safe_free(font->codes);
    neko_safe_free(font);
    return 0;
}

neko_font_t *neko_font_load_ascii(neko_font_u64 atlas_id, const void *pixels, int w, int h, int stride, void *mem_ctx) { return neko_font_load(atlas_id, pixels, w, h, stride, mem_ctx, 0); }

neko_font_t *neko_font_load_1252(neko_font_u64 atlas_id, const void *pixels, int w, int h, int stride, void *mem_ctx) { return neko_font_load(atlas_id, pixels, w, h, stride, mem_ctx, 1252); }

#define NEKO_FONT_INTERNAL_BUFFER_MAX 1024

typedef struct neko_font_parse_t {
    const char *in;
    const char *end;
    int scratch_len;
    char scratch[NEKO_FONT_INTERNAL_BUFFER_MAX];
} neko_font_parse_t;

static int neko_font_isspace(char c) { return (c == ' ') | (c == '\t') | (c == '\n') | (c == '\v') | (c == '\f') | (c == '\r'); }

static int neko_font_next_internal(neko_font_parse_t *p, char *c) {
    NEKO_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
    while (neko_font_isspace(*c = *p->in++)) NEKO_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_next(p, c)                               \
    do {                                                   \
        NEKO_FONT_FAIL_IF(!neko_font_next_internal(p, c)); \
    } while (0)

static char neko_font_parse_char(char c) {
    switch (c) {
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '"':
            return '"';
        case 't':
            return '\t';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        default:
            return c;
    }
}

#define neko_font_expect(p, expect)                                           \
    do {                                                                      \
        char neko_font_char;                                                  \
        neko_font_next(p, &neko_font_char);                                   \
        NEKO_FONT_CHECK(neko_font_char == expect, "Found unexpected token."); \
    } while (0)

static int neko_font_read_string_internal(neko_font_parse_t *p) {
    int count = 0;
    int done = 0;
    neko_font_expect(p, '"');

    while (!done) {
        char c = 0;
        NEKO_FONT_CHECK(count < NEKO_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
        neko_font_next(p, &c);

        switch (c) {
            case '"':
                p->scratch[count] = 0;
                done = 1;
                break;

            case '\\': {
                char the_char;
                neko_font_next(p, &the_char);
                the_char = neko_font_parse_char(the_char);
                p->scratch[count++] = the_char;
            } break;

            default:
                p->scratch[count++] = c;
                break;
        }
    }

    p->scratch_len = count;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_string(p)                               \
    do {                                                       \
        NEKO_FONT_FAIL_IF(!neko_font_read_string_internal(p)); \
    } while (0)

static int neko_font_read_identifier_internal(neko_font_parse_t *p) {
    int count = 0;
    int done = 0;

    while (1) {
        char c = 0;
        NEKO_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
        NEKO_FONT_CHECK(count < NEKO_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
        c = *p->in;
        if (!neko_font_isspace(c)) break;
        p->in++;
    }

    while (!done) {
        char c = 0;
        NEKO_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
        NEKO_FONT_CHECK(count < NEKO_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
        c = *p->in++;

        if (neko_font_isspace(c)) {
            p->scratch[count] = 0;
            break;
        }

        switch (c) {
            case '=':
                p->scratch[count] = 0;
                done = 1;
                break;

            case '\\': {
                char the_char;
                neko_font_next(p, &the_char);
                the_char = neko_font_parse_char(the_char);
                p->scratch[count++] = the_char;
            } break;

            default:
                p->scratch[count++] = c;
                break;
        }
    }

    p->scratch_len = count;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_identifier(p)                               \
    do {                                                           \
        NEKO_FONT_FAIL_IF(!neko_font_read_identifier_internal(p)); \
    } while (0)

static int neko_font_read_int_internal(neko_font_parse_t *p, int *out) {
    char *end;
    int val = (int)strtoll(p->in, &end, 10);
    NEKO_FONT_CHECK(p->in != end, "Invalid integer found during parse.");
    p->in = end;
    *out = val;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_int(p, num)                               \
    do {                                                         \
        NEKO_FONT_FAIL_IF(!neko_font_read_int_internal(p, num)); \
    } while (0)

static int neko_font_read_float_internal(neko_font_parse_t *p, float *out) {
    char *end;
    float val = (float)strtod(p->in, &end);
    NEKO_FONT_CHECK(p->in != end, "Error reading float.");
    p->in = end;
    *out = val;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_float(p, num)                                                   \
    do {                                                                               \
        NEKO_FONT_FAIL_IF(neko_font_read_float_internal(p, num) != NEKO_FONT_SUCCESS); \
    } while (0)

int neko_font_expect_string_internal(neko_font_parse_t *p, const char *str) {
    neko_font_read_string(p);
    if (strncmp(p->scratch, str, p->scratch_len))
        return 0;
    else
        return 1;
neko_font_err:
    return 0;
}

#define neko_font_expect_string(p, str)                               \
    do {                                                              \
        NEKO_FONT_FAIL_IF(!neko_font_expect_string_internal(p, str)); \
    } while (0)

int neko_font_expect_identifier_internal(neko_font_parse_t *p, const char *str) {
    neko_font_read_identifier(p);
    if (strncmp(p->scratch, str, p->scratch_len))
        return 0;
    else
        return 1;
neko_font_err:
    return 0;
}

#define neko_font_expect_identifier(p, str)                               \
    do {                                                                  \
        NEKO_FONT_FAIL_IF(!neko_font_expect_identifier_internal(p, str)); \
    } while (0)

typedef struct neko_font_kern_t {
    hashtable_t table;
} neko_font_kern_t;

neko_font_t *neko_font_load_bmfont(neko_font_u64 atlas_id, const void *fnt, int size, void *mem_ctx) {
    float w0 = 0, h0 = 0, div = 0, wTol = 0, hTol = 0;
    neko_font_parse_t parse;
    neko_font_parse_t *p = &parse;
    p->in = (const char *)fnt;
    p->end = p->in + size;

    neko_font_t *font = (neko_font_t *)neko_safe_malloc(sizeof(neko_font_t));
    font->atlas_id = atlas_id;
    font->kern = 0;
    font->mem_ctx = mem_ctx;

    // Read in font information.
    neko_font_expect_identifier(p, "info");
    neko_font_expect_identifier(p, "face");
    neko_font_read_string(p);
    neko_font_expect_identifier(p, "size");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "bold");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "italic");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "charset");
    neko_font_read_string(p);
    neko_font_expect_identifier(p, "unicode");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "stretchH");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "smooth");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "aa");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "padding");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "spacing");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "outline");
    neko_font_read_identifier(p);

    neko_font_expect_identifier(p, "common");
    neko_font_expect_identifier(p, "lineHeight");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "base");
    neko_font_read_int(p, &font->font_height);
    neko_font_expect_identifier(p, "scaleW");
    neko_font_read_int(p, &font->atlas_w);
    neko_font_expect_identifier(p, "scaleH");
    neko_font_read_int(p, &font->atlas_h);
    neko_font_expect_identifier(p, "pages");
    neko_font_expect_identifier(p, "1");
    neko_font_expect_identifier(p, "packed");
    neko_font_expect_identifier(p, "0");
    neko_font_expect_identifier(p, "alphaChnl");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "redChnl");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "greenChnl");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "blueChnl");
    neko_font_read_identifier(p);

    neko_font_expect_identifier(p, "page");
    neko_font_expect_identifier(p, "id");
    neko_font_read_identifier(p);
    neko_font_expect_identifier(p, "file");
    neko_font_read_string(p);

    // Start parsing individual glyphs.
    neko_font_expect_identifier(p, "chars");
    neko_font_expect_identifier(p, "count");
    neko_font_read_int(p, &font->glyph_count);
    font->glyphs = (neko_font_glyph_t *)neko_safe_malloc(sizeof(neko_font_glyph_t) * font->glyph_count);
    font->codes = (int *)neko_safe_malloc(sizeof(int) * font->glyph_count);

    // Used to squeeze UVs inward by 128th of a pixel.
    w0 = 1.0f / (float)(font->atlas_w);
    h0 = 1.0f / (float)(font->atlas_h);
    div = 1.0f / 128.0f;
    wTol = w0 * div;
    hTol = h0 * div;

    // Read in each glyph.
    for (int i = 0; i < font->glyph_count; ++i) {
        int x = 0, y = 0;
        int width = 0, height = 0;

        neko_font_glyph_t *glyph = font->glyphs + i;
        neko_font_expect_identifier(p, "char");
        neko_font_expect_identifier(p, "id");
        neko_font_read_int(p, font->codes + i);

        neko_font_expect_identifier(p, "x");
        neko_font_read_int(p, &x);
        neko_font_expect_identifier(p, "y");
        neko_font_read_int(p, &y);
        neko_font_expect_identifier(p, "width");
        neko_font_read_int(p, &width);
        neko_font_expect_identifier(p, "height");
        neko_font_read_int(p, &height);

        glyph->w = (float)width;
        glyph->h = (float)height;
        glyph->minx = (float)x * w0 + wTol;
        glyph->miny = (float)y * h0 + hTol;
        glyph->maxx = (float)(x + width) * w0 - wTol;
        glyph->maxy = (float)(y + height) * h0 - hTol;

        neko_font_expect_identifier(p, "xoffset");
        neko_font_read_int(p, &glyph->xoffset);
        neko_font_expect_identifier(p, "yoffset");
        neko_font_read_int(p, &glyph->yoffset);
        neko_font_expect_identifier(p, "xadvance");
        neko_font_read_int(p, &glyph->xadvance);
        neko_font_expect_identifier(p, "page");
        neko_font_read_identifier(p);
        neko_font_expect_identifier(p, "chnl");
        neko_font_read_identifier(p);
    }

    if (p->end - p->in > 8) {
        int kern_count = 0;
        neko_font_kern_t *kern = NULL;
        neko_font_expect_identifier(p, "kernings");
        neko_font_expect_identifier(p, "count");
        neko_font_read_int(p, &kern_count);
        kern = (neko_font_kern_t *)neko_safe_malloc(sizeof(neko_font_kern_t));
        font->kern = kern;
        hashtable_init(&kern->table, sizeof(int), kern_count, mem_ctx);

        for (int i = 0; i < kern_count; ++i) {
            int first, second, amount;
            neko_font_expect_identifier(p, "kerning");
            neko_font_expect_identifier(p, "first");
            neko_font_read_int(p, &first);
            neko_font_expect_identifier(p, "second");
            neko_font_read_int(p, &second);
            neko_font_expect_identifier(p, "amount");
            neko_font_read_int(p, &amount);

            neko_font_u64 key = (neko_font_u64)first << 32 | (neko_font_u64)second;
            hashtable_insert(&kern->table, key, (void *)(size_t)amount);
        }
    }

    return font;

neko_font_err:
    neko_font_free(font);
    return 0;
}

void neko_font_free(neko_font_t *font) {
    void *mem_ctx = font->mem_ctx;
    neko_safe_free(font->glyphs);
    neko_safe_free(font->codes);
    if (font->kern) hashtable_term(&font->kern->table);
    neko_safe_free(font->kern);
    neko_safe_free(font);
}

int neko_font_text_width(neko_font_t *font, const char *text) {
    int x = 0;
    int w = 0;

    while (*text) {
        int c;
        text = neko_font_decode_utf8(text, &c);
        if (c == '\n' || c == '\r')
            x = 0;
        else {
            x += neko_font_get_glyph(font, neko_font_get_glyph_index(font, c))->xadvance;
            w = (x > w) ? x : w;
        }
    }

    return w;
}

int neko_font_text_height(neko_font_t *font, const char *text) {
    int font_height, h;
    h = font_height = font->font_height;

    while (*text) {
        int c;
        text = neko_font_decode_utf8(text, &c);
        if (c == '\n' && *text) h += font_height;
    }

    return h;
}

int neko_font_max_glyph_height(neko_font_t *font, const char *text) {
    int max_height = 0;

    while (*text) {
        int c;
        text = neko_font_decode_utf8(text, &c);
        int h = (int)neko_font_get_glyph(font, neko_font_get_glyph_index(font, c))->h;
        if (h > max_height) max_height = h;
    }

    return max_height;
}

int neko_font_get_glyph_index(neko_font_t *font, int code) {
    int lo = 0;
    int hi = font->glyph_count;

    while (lo < hi) {
        int guess = (lo + hi) / 2;
        if (code < font->codes[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    if (lo == 0 || font->codes[lo - 1] != code)
        return '?' - 32;
    else
        return lo - 1;
}

neko_font_glyph_t *neko_font_get_glyph(neko_font_t *font, int index) { return font->glyphs + index; }

int neko_font_kerning(neko_font_t *font, int code0, int code1) { return (int)(size_t)hashtable_find(&font->kern->table, (neko_font_u64)code0 << 32 | (neko_font_u64)code1); }

neko_font_t *neko_font_create_blank(int font_height, int glyph_count) {
    neko_font_t *font = (neko_font_t *)neko_safe_malloc(sizeof(neko_font_t));
    font->glyph_count = glyph_count;
    font->font_height = font_height;
    font->kern = 0;
    font->glyphs = (neko_font_glyph_t *)neko_safe_malloc(sizeof(neko_font_glyph_t) * font->glyph_count);
    font->codes = (int *)neko_safe_malloc(sizeof(int) * font->glyph_count);
    return font;
}

void neko_font_add_kerning_pair(neko_font_t *font, int code0, int code1, int kerning) {
    if (!font->kern) {
        neko_font_kern_t *kern = (neko_font_kern_t *)neko_safe_malloc(sizeof(neko_font_kern_t));
        font->kern = kern;
        hashtable_init(&kern->table, sizeof(int), 256, font->mem_ctx);
    }

    neko_font_u64 key = (neko_font_u64)code0 << 32 | (neko_font_u64)code1;
    hashtable_insert(&font->kern->table, key, (void *)(size_t)kerning);
}

static int s_is_space(int c) {
    switch (c) {
        case ' ':
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
            return 1;
    }
    return 0;
}

static const char *s_find_end_of_line(neko_font_t *font, const char *text, float wrap_width) {
    float x = 0;
    const char *start_of_word = 0;
    float word_w = 0;

    while (*text) {
        int cp;
        const char *text_prev = text;
        text = neko_font_decode_utf8(text, &cp);
        neko_font_glyph_t *glyph = neko_font_get_glyph(font, neko_font_get_glyph_index(font, cp));

        if (cp == '\n') {
            x = 0;
            word_w = 0;
            start_of_word = 0;
            continue;
        } else if (cp == '\r')
            continue;
        else {
            if (s_is_space(cp)) {
                x += word_w + glyph->xadvance;
                word_w = 0;
                start_of_word = 0;
            } else {
                if (!start_of_word) start_of_word = text_prev;

                if (x + word_w + glyph->xadvance < wrap_width) {
                    word_w += glyph->xadvance;
                } else {
                    // Put entire word on the next line.
                    if (word_w + glyph->xadvance < wrap_width) {
                        return start_of_word;
                    }

                    // Word itself does not fit on one line, so just cut it here.
                    else {
                        return text;
                    }
                }
            }
        }
    }

    return text;
}

static inline float s_dist_to_plane(float v, float d) { return v - d; }

static inline float s_intersect(float a, float b, float u0, float u1, float plane_d) {
    float da = s_dist_to_plane(a, plane_d);
    float db = s_dist_to_plane(b, plane_d);
    return u0 + (u1 - u0) * (da / (da - db));
}

int neko_font_fill_vertex_buffer(neko_font_t *font, const char *text, float x0, float y0, float wrap_w, float line_height, neko_font_rect_t *clip_rect, neko_font_vert_t *buffer, int buffer_max,
                                 int *count_written) {
    float x = x0;
    float y = y0;
    float font_height = (float)font->font_height;
    int i = 0;
    const char *end_of_line = 0;
    int wrap_enabled = wrap_w > 0;
    neko_font_rect_t rect;
    if (clip_rect) rect = *clip_rect;

    while (*text) {
        int cp;
        const char *prev_text = text;
        text = neko_font_decode_utf8(text, &cp);

        // Word wrapping logic.
        if (wrap_enabled) {
            if (!end_of_line) {
                end_of_line = s_find_end_of_line(font, prev_text, wrap_w);
            }

            int finished_rendering_line = !(text < end_of_line);
            if (finished_rendering_line) {
                end_of_line = 0;
                x = x0;
                y -= font_height + line_height;

                // Skip whitespace at the beginning of new lines.
                while (cp) {
                    cp = *text;
                    if (s_is_space(cp))
                        ++text;
                    else if (cp == '\n') {
                        text++;
                        break;
                    } else
                        break;
                }

                continue;
            }
        }

        if (cp == '\n') {
            x = x0;
            y -= font_height + line_height;
            continue;
        } else if (cp == '\r')
            continue;

        neko_font_glyph_t *glyph = neko_font_get_glyph(font, neko_font_get_glyph_index(font, cp));
        float x0 = (float)glyph->xoffset;
        float y0 = -(float)glyph->yoffset;

        // Bottom left (min).
        float ax = x + x0;
        float ay = y - glyph->h + y0;
        float au = glyph->minx;
        float av = glyph->maxy;

        // Top right (max).
        float bx = x + glyph->w + x0;
        float by = y + y0;
        float bu = glyph->maxx;
        float bv = glyph->miny;

        int clipped_away = 0;

        if (clip_rect) {
            int inside_clip_x_axis = ((x + x0) < rect.right) | (x + glyph->w + x0 > rect.left);
            clipped_away = !inside_clip_x_axis;

            if (inside_clip_x_axis) {
                if (ax < rect.left) {
                    au = s_intersect(ax, bx, au, bu, rect.left);
                    ax = rect.left;
                }

                if (bx > rect.right) {
                    bu = s_intersect(ax, bx, au, bu, rect.right);
                    bx = rect.right;
                }

                if (ay < rect.bottom) {
                    av = s_intersect(ay, by, av, bv, rect.bottom);
                    ay = rect.bottom;
                }

                if (by > rect.top) {
                    bv = s_intersect(ay, by, av, bv, rect.top);
                    by = rect.top;
                }

                clipped_away = (ax >= bx) | (ay >= by);
            }
        }

        if (!clipped_away) {
            // Quad is set up as:
            //
            // a----d
            // |   /|
            // |  / |
            // | /  |
            // |/   |
            // b----c

            neko_font_vert_t a;
            a.x = ax;
            a.y = by;
            a.u = au;
            a.v = bv;

            neko_font_vert_t b;
            b.x = ax;
            b.y = ay;
            b.u = au;
            b.v = av;

            neko_font_vert_t c;
            c.x = bx;
            c.y = ay;
            c.u = bu;
            c.v = av;

            neko_font_vert_t d;
            d.x = bx;
            d.y = by;
            d.u = bu;
            d.v = bv;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = a;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = b;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = d;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = d;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = b;

            NEKO_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = c;
        }

        x += glyph->xadvance;
    }

    *count_written = i;
    return 1;

neko_font_err:
    *count_written = i;
    return 0;
}

#pragma endregion font

NEKO_STATIC const_str s_error_file = NULL;  // 正在解析的文件的文件路径 如果来自内存则为 NULL
NEKO_STATIC const_str s_error_reason;       // 用于捕获 DEFLATE 解析期间的错误

#define __NEKO_ASEPRITE_CHECK(X, Y) \
    do {                            \
        if (!(X)) {                 \
            s_error_reason = Y;     \
            goto ase_err;           \
        }                           \
    } while (0)

#define __NEKO_ASEPRITE_CALL(X) \
    do {                        \
        if (!(X)) goto ase_err; \
    } while (0)

#define __NEKO_ASEPRITE_DEFLATE_MAX_BITLEN 15

// DEFLATE tables from RFC 1951
NEKO_STATIC u8 s_fixed_table[288 + 32] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};  // 3.2.6
NEKO_STATIC u8 s_permutation_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};                                                                                 // 3.2.7
NEKO_STATIC u8 s_len_extra_bits[29 + 2] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};                                                     // 3.2.5
NEKO_STATIC u32 s_len_base[29 + 2] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};                              // 3.2.5
NEKO_STATIC u8 s_dist_extra_bits[30 + 2] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0};                                         // 3.2.5
NEKO_STATIC u32 s_dist_base[30 + 2] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};  // 3.2.5

typedef struct deflate_t {
    u64 bits;
    int count;
    u32 *words;
    int word_count;
    int word_index;
    int bits_left;

    int final_word_available;
    u32 final_word;

    char *out;
    char *out_end;
    char *begin;

    u32 lit[288];
    u32 dst[32];
    u32 len[19];
    u32 nlit;
    u32 ndst;
    u32 nlen;
} deflate_t;

NEKO_STATIC int s_would_overflow(deflate_t *s, int num_bits) { return (s->bits_left + s->count) - num_bits < 0; }

NEKO_STATIC char *s_ptr(deflate_t *s) {
    NEKO_ASSERT(!(s->bits_left & 7));
    return (char *)(s->words + s->word_index) - (s->count / 8);
}

NEKO_STATIC u64 s_peak_bits(deflate_t *s, int num_bits_to_read) {
    if (s->count < num_bits_to_read) {
        if (s->word_index < s->word_count) {
            u32 word = s->words[s->word_index++];
            s->bits |= (u64)word << s->count;
            s->count += 32;
            NEKO_ASSERT(s->word_index <= s->word_count);
        }

        else if (s->final_word_available) {
            u32 word = s->final_word;
            s->bits |= (u64)word << s->count;
            s->count += s->bits_left;
            s->final_word_available = 0;
        }
    }

    return s->bits;
}

NEKO_STATIC u32 s_consume_bits(deflate_t *s, int num_bits_to_read) {
    NEKO_ASSERT(s->count >= num_bits_to_read);
    u32 bits = (u32)(s->bits & (((u64)1 << num_bits_to_read) - 1));
    s->bits >>= num_bits_to_read;
    s->count -= num_bits_to_read;
    s->bits_left -= num_bits_to_read;
    return bits;
}

NEKO_STATIC u32 s_read_bits(deflate_t *s, int num_bits_to_read) {
    NEKO_ASSERT(num_bits_to_read <= 32);
    NEKO_ASSERT(num_bits_to_read >= 0);
    NEKO_ASSERT(s->bits_left > 0);
    NEKO_ASSERT(s->count <= 64);
    NEKO_ASSERT(!s_would_overflow(s, num_bits_to_read));
    s_peak_bits(s, num_bits_to_read);
    u32 bits = s_consume_bits(s, num_bits_to_read);
    return bits;
}

NEKO_STATIC u32 s_rev16(u32 a) {
    a = ((a & 0xAAAA) >> 1) | ((a & 0x5555) << 1);
    a = ((a & 0xCCCC) >> 2) | ((a & 0x3333) << 2);
    a = ((a & 0xF0F0) >> 4) | ((a & 0x0F0F) << 4);
    a = ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
    return a;
}

// RFC 1951 section 3.2.2
NEKO_STATIC u32 s_build(deflate_t *s, u32 *tree, u8 *lens, int sym_count) {
    int n, codes[16], first[16], counts[16] = {0};

    // Frequency count
    for (n = 0; n < sym_count; n++) counts[lens[n]]++;

    // Distribute codes
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; ++n) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }

    for (int i = 0; i < sym_count; ++i) {
        u8 len = lens[i];

        if (len != 0) {
            NEKO_ASSERT(len < 16);
            u32 code = (u32)codes[len]++;
            u32 slot = (u32)first[len]++;
            tree[slot] = (code << (32 - (u32)len)) | (i << 4) | len;
        }
    }

    return (u32)first[15];
}

NEKO_STATIC int s_stored(deflate_t *s) {
    char *p;

    // 3.2.3
    // skip any remaining bits in current partially processed byte
    s_read_bits(s, s->count & 7);

    // 3.2.4
    // read LEN and NLEN, should complement each other
    u16 LEN = (u16)s_read_bits(s, 16);
    u16 NLEN = (u16)s_read_bits(s, 16);
    __NEKO_ASEPRITE_CHECK(LEN == (u16)(~NLEN), "Failed to find LEN and NLEN as complements within stored (uncompressed) stream.");
    __NEKO_ASEPRITE_CHECK(s->bits_left / 8 <= (int)LEN, "Stored block extends beyond end of input stream.");
    p = s_ptr(s);
    memcpy(s->out, p, LEN);
    s->out += LEN;
    return 1;

ase_err:
    return 0;
}

// 3.2.6
NEKO_STATIC int s_fixed(deflate_t *s) {
    s->nlit = s_build(s, s->lit, s_fixed_table, 288);
    s->ndst = s_build(0, s->dst, s_fixed_table + 288, 32);
    return 1;
}

NEKO_STATIC int s_decode(deflate_t *s, u32 *tree, int hi) {
    u64 bits = s_peak_bits(s, 16);
    u32 search = (s_rev16((u32)bits) << 16) | 0xFFFF;
    int lo = 0;
    while (lo < hi) {
        int guess = (lo + hi) >> 1;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    u32 key = tree[lo - 1];
    u32 len = (32 - (key & 0xF));
    NEKO_ASSERT((search >> len) == (key >> len));

    s_consume_bits(s, key & 0xF);
    return (key >> 4) & 0xFFF;
}

// 3.2.7
NEKO_STATIC int s_dynamic(deflate_t *s) {
    u8 lenlens[19] = {0};

    u32 nlit = 257 + s_read_bits(s, 5);
    u32 ndst = 1 + s_read_bits(s, 5);
    u32 nlen = 4 + s_read_bits(s, 4);

    for (u32 i = 0; i < nlen; ++i) lenlens[s_permutation_order[i]] = (u8)s_read_bits(s, 3);

    // Build the tree for decoding code lengths
    s->nlen = s_build(0, s->len, lenlens, 19);
    u8 lens[288 + 32];

    for (u32 n = 0; n < nlit + ndst;) {
        int sym = s_decode(s, s->len, (int)s->nlen);
        switch (sym) {
            case 16:
                for (u32 i = 3 + s_read_bits(s, 2); i; --i, ++n) lens[n] = lens[n - 1];
                break;
            case 17:
                for (u32 i = 3 + s_read_bits(s, 3); i; --i, ++n) lens[n] = 0;
                break;
            case 18:
                for (u32 i = 11 + s_read_bits(s, 7); i; --i, ++n) lens[n] = 0;
                break;
            default:
                lens[n++] = (u8)sym;
                break;
        }
    }

    s->nlit = s_build(s, s->lit, lens, (int)nlit);
    s->ndst = s_build(0, s->dst, lens + nlit, (int)ndst);
    return 1;
}

// 3.2.3
NEKO_STATIC int s_block(deflate_t *s) {
    while (1) {
        int symbol = s_decode(s, s->lit, (int)s->nlit);

        if (symbol < 256) {
            __NEKO_ASEPRITE_CHECK(s->out + 1 <= s->out_end, "Attempted to overwrite out buffer while outputting a symbol.");
            *s->out = (char)symbol;
            s->out += 1;
        }

        else if (symbol > 256) {
            symbol -= 257;
            u32 length = s_read_bits(s, (int)(s_len_extra_bits[symbol])) + s_len_base[symbol];
            int distance_symbol = s_decode(s, s->dst, (int)s->ndst);
            u32 backwards_distance = s_read_bits(s, s_dist_extra_bits[distance_symbol]) + s_dist_base[distance_symbol];
            __NEKO_ASEPRITE_CHECK(s->out - backwards_distance >= s->begin, "Attempted to write before out buffer (invalid backwards distance).");
            __NEKO_ASEPRITE_CHECK(s->out + length <= s->out_end, "Attempted to overwrite out buffer while outputting a string.");
            char *src = s->out - backwards_distance;
            char *dst = s->out;
            s->out += length;

            switch (backwards_distance) {
                case 1:  // very common in images
                    memset(dst, *src, (size_t)length);
                    break;
                default:
                    while (length--) *dst++ = *src++;
            }
        }

        else
            break;
    }

    return 1;

ase_err:
    return 0;
}

// 3.2.3
NEKO_STATIC int s_inflate(const void *in, int in_bytes, void *out, int out_bytes) {
    deflate_t *s = (deflate_t *)neko_safe_malloc(sizeof(deflate_t));
    s->bits = 0;
    s->count = 0;
    s->word_index = 0;
    s->bits_left = in_bytes * 8;

    // s->words is the in-pointer rounded up to a multiple of 4
    int first_bytes = (int)((((size_t)in + 3) & ~3) - (size_t)in);
    s->words = (u32 *)((char *)in + first_bytes);
    s->word_count = (in_bytes - first_bytes) / 4;
    int last_bytes = ((in_bytes - first_bytes) & 3);

    for (int i = 0; i < first_bytes; ++i) s->bits |= (u64)(((u8 *)in)[i]) << (i * 8);

    s->final_word_available = last_bytes ? 1 : 0;
    s->final_word = 0;
    for (int i = 0; i < last_bytes; i++) s->final_word |= ((u8 *)in)[in_bytes - last_bytes + i] << (i * 8);

    s->count = first_bytes * 8;

    s->out = (char *)out;
    s->out_end = s->out + out_bytes;
    s->begin = (char *)out;

    int count = 0;
    u32 bfinal;
    do {
        bfinal = s_read_bits(s, 1);
        u32 btype = s_read_bits(s, 2);

        switch (btype) {
            case 0:
                __NEKO_ASEPRITE_CALL(s_stored(s));
                break;
            case 1:
                s_fixed(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 2:
                s_dynamic(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 3:
                __NEKO_ASEPRITE_CHECK(0, "Detected unknown block type within input stream.");
        }

        ++count;
    } while (!bfinal);

    neko_safe_free(s);
    return 1;

ase_err:
    neko_safe_free(s);
    return 0;
}

typedef struct ase_state_t {
    u8 *in;
    u8 *end;
} ase_state_t;

NEKO_STATIC u8 s_read_uint8(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(u8));
    u8 **p = &s->in;
    u8 value = **p;
    ++(*p);
    return value;
}

NEKO_STATIC u16 s_read_uint16(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(u16));
    u8 **p = &s->in;
    u16 value;
    value = (*p)[0];
    value |= (((u16)((*p)[1])) << 8);
    *p += 2;
    return value;
}

NEKO_STATIC ase_fixed_t s_read_fixed(ase_state_t *s) {
    ase_fixed_t value;
    value.a = s_read_uint16(s);
    value.b = s_read_uint16(s);
    return value;
}

NEKO_STATIC u32 s_read_uint32(ase_state_t *s) {
    NEKO_ASSERT(s->in <= s->end + sizeof(u32));
    u8 **p = &s->in;
    u32 value;
    value = (*p)[0];
    value |= (((u32)((*p)[1])) << 8);
    value |= (((u32)((*p)[2])) << 16);
    value |= (((u32)((*p)[3])) << 24);
    *p += 4;
    return value;
}

NEKO_STATIC s16 s_read_int16(ase_state_t *s) { return (s16)s_read_uint16(s); }
NEKO_STATIC s16 s_read_int32(ase_state_t *s) { return (s32)s_read_uint32(s); }

NEKO_STATIC const char *s_read_string(ase_state_t *s) {
    int len = (int)s_read_uint16(s);
    char *bytes = (char *)neko_safe_malloc(len + 1);
    for (int i = 0; i < len; ++i) {
        bytes[i] = (char)s_read_uint8(s);
    }
    bytes[len] = 0;
    return bytes;
}

NEKO_STATIC void s_skip(ase_state_t *ase, int num_bytes) {
    NEKO_ASSERT(ase->in <= ase->end + num_bytes);
    ase->in += num_bytes;
}

NEKO_STATIC char *s_fopen(const char *path, int *size) {
    char *data = 0;
    FILE *fp = fopen(path, "rb");
    int sz = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char *)neko_safe_malloc(sz + 1);
        fread(data, sz, 1, fp);
        data[sz] = 0;
        fclose(fp);
    }

    if (size) *size = sz;
    return data;
}

ase_t *neko_aseprite_load_from_file(const char *path) {
    s_error_file = path;
    int sz;
    void *file = s_fopen(path, &sz);
    if (!file) {
        NEKO_WARN("unable to find map file %s", s_error_file ? s_error_file : "MEMORY");
        return NULL;
    }
    ase_t *aseprite = neko_aseprite_load_from_memory(file, sz);
    neko_safe_free(file);
    s_error_file = NULL;
    return aseprite;
}

ase_t *neko_aseprite_load_from_memory(const void *memory, int size) {
    ase_t *ase = (ase_t *)neko_safe_malloc(sizeof(ase_t));
    memset(ase, 0, sizeof(*ase));

    ase_state_t state = {0};
    ase_state_t *s = &state;
    s->in = (u8 *)memory;
    s->end = s->in + size;

    s_skip(s, sizeof(u32));  // File size.
    int magic = (int)s_read_uint16(s);
    NEKO_ASSERT(magic == 0xA5E0);

    ase->frame_count = (int)s_read_uint16(s);
    ase->w = s_read_uint16(s);
    ase->h = s_read_uint16(s);
    u16 bpp = s_read_uint16(s) / 8;
    if (bpp == 4)
        ase->mode = NEKO_ASE_MODE_RGBA;
    else if (bpp == 2)
        ase->mode = NEKO_ASE_MODE_GRAYSCALE;
    else {
        NEKO_ASSERT(bpp == 1);
        ase->mode = NEKO_ASE_MODE_INDEXED;
    }
    u32 valid_layer_opacity = s_read_uint32(s) & 1;
    int speed = s_read_uint16(s);
    s_skip(s, sizeof(u32) * 2);  // Spec says skip these bytes, as they're zero'd.
    ase->transparent_palette_entry_index = s_read_uint8(s);
    s_skip(s, 3);  // Spec says skip these bytes.
    ase->number_of_colors = (int)s_read_uint16(s);
    ase->pixel_w = (int)s_read_uint8(s);
    ase->pixel_h = (int)s_read_uint8(s);
    ase->grid_x = (int)s_read_int16(s);
    ase->grid_y = (int)s_read_int16(s);
    ase->grid_w = (int)s_read_uint16(s);
    ase->grid_h = (int)s_read_uint16(s);
    s_skip(s, 84);  // For future use (set to zero).

    ase->frames = (ase_frame_t *)neko_safe_malloc((int)(sizeof(ase_frame_t)) * ase->frame_count);
    memset(ase->frames, 0, sizeof(ase_frame_t) * (size_t)ase->frame_count);

    ase_udata_t *last_udata = NULL;
    int was_on_tags = 0;
    int tag_index = 0;

    ase_layer_t *layer_stack[__NEKO_ASEPRITE_MAX_LAYERS];

    // 查看 aseprite 文件标准
    // https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md

    // Parse all chunks in the .aseprite file.
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        frame->ase = ase;
        s_skip(s, sizeof(u32));  // Frame size.
        magic = (int)s_read_uint16(s);
        NEKO_ASSERT(magic == 0xF1FA);
        int chunk_count = (int)s_read_uint16(s);
        frame->duration_milliseconds = s_read_uint16(s);
        if (frame->duration_milliseconds == 0) frame->duration_milliseconds = speed;
        s_skip(s, 2);  // For future use (set to zero).
        u32 new_chunk_count = s_read_uint32(s);
        if (new_chunk_count) chunk_count = (int)new_chunk_count;

        for (int j = 0; j < chunk_count; ++j) {
            u32 chunk_size = s_read_uint32(s);
            u16 chunk_type = s_read_uint16(s);
            chunk_size -= (u32)(sizeof(u32) + sizeof(u16));
            u8 *chunk_start = s->in;

            switch (chunk_type) {
                case 0x2004:  // Layer chunk.
                {
                    NEKO_ASSERT(ase->layer_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_layer_t *layer = ase->layers + ase->layer_count++;
                    layer->flags = (ase_layer_flags_t)s_read_uint16(s);

                    // WORD Layer type
                    //  0 = Normal(image) layer
                    //  1 = Group
                    //  2 = Tilemap
                    layer->type = (ase_layer_type_t)s_read_uint16(s);

                    layer->parent = NULL;
                    int child_level = (int)s_read_uint16(s);
                    layer_stack[child_level] = layer;
                    if (child_level) {
                        layer->parent = layer_stack[child_level - 1];
                    }
                    s_skip(s, sizeof(u16));  // Default layer width in pixels (ignored).
                    s_skip(s, sizeof(u16));  // Default layer height in pixels (ignored).
                    int blend_mode = (int)s_read_uint16(s);
                    if (blend_mode) NEKO_WARN("aseprite unknown blend mode encountered.");
                    layer->opacity = s_read_uint8(s) / 255.0f;
                    if (!valid_layer_opacity) layer->opacity = 1.0f;
                    s_skip(s, 3);  // For future use (set to zero).
                    layer->name = s_read_string(s);
                    last_udata = &layer->udata;
                } break;

                case 0x2005:  // Cel chunk.
                {
                    NEKO_ASSERT(frame->cel_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_cel_t *cel = frame->cels + frame->cel_count++;
                    int layer_index = (int)s_read_uint16(s);
                    cel->layer = ase->layers + layer_index;
                    cel->x = s_read_int16(s);
                    cel->y = s_read_int16(s);
                    cel->opacity = s_read_uint8(s) / 255.0f;
                    int cel_type = (int)s_read_uint16(s);
                    s_skip(s, 7);  // For future (set to zero).
                    switch (cel_type) {
                        case 0:  // Raw cel.
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            cel->cel_pixels = neko_safe_malloc(cel->w * cel->h * bpp);
                            memcpy(cel->cel_pixels, s->in, (size_t)(cel->w * cel->h * bpp));
                            s_skip(s, cel->w * cel->h * bpp);
                            break;

                        case 1:  // Linked cel.
                            cel->is_linked = 1;
                            cel->linked_frame_index = s_read_uint16(s);
                            break;

                        case 2:  // Compressed image cel.
                        {
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            int zlib_byte0 = s_read_uint8(s);
                            int zlib_byte1 = s_read_uint8(s);
                            int deflate_bytes = (int)chunk_size - (int)(s->in - chunk_start);
                            void *pixels = s->in;
                            NEKO_ASSERT((zlib_byte0 & 0x0F) == 0x08);  // Only zlib compression method (RFC 1950) is supported.
                            NEKO_ASSERT((zlib_byte0 & 0xF0) <= 0x70);  // Innapropriate window size detected.
                            NEKO_ASSERT(!(zlib_byte1 & 0x20));         // Preset dictionary is present and not supported.
                            int pixels_sz = cel->w * cel->h * bpp;
                            void *pixels_decompressed = neko_safe_malloc(pixels_sz);
                            int ret = s_inflate(pixels, deflate_bytes, pixels_decompressed, pixels_sz);
                            if (!ret) NEKO_WARN(s_error_reason);
                            cel->cel_pixels = pixels_decompressed;
                            s_skip(s, deflate_bytes);
                        } break;
                    }
                    last_udata = &cel->udata;
                } break;

                case 0x2006:  // Cel extra chunk.
                {
                    ase_cel_t *cel = frame->cels + frame->cel_count;
                    cel->has_extra = 1;
                    cel->extra.precise_bounds_are_set = (int)s_read_uint32(s);
                    cel->extra.precise_x = s_read_fixed(s);
                    cel->extra.precise_y = s_read_fixed(s);
                    cel->extra.w = s_read_fixed(s);
                    cel->extra.h = s_read_fixed(s);
                    s_skip(s, 16);  // For future use (set to zero).
                } break;

                case 0x2007:  // Color profile chunk.
                {
                    ase->has_color_profile = 1;
                    ase->color_profile.type = (ase_color_profile_type_t)s_read_uint16(s);
                    ase->color_profile.use_fixed_gamma = (int)s_read_uint16(s) & 1;
                    ase->color_profile.gamma = s_read_fixed(s);
                    s_skip(s, 8);  // For future use (set to zero).
                    if (ase->color_profile.type == NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC) {
                        // Use the embedded ICC profile.
                        ase->color_profile.icc_profile_data_length = s_read_uint32(s);
                        ase->color_profile.icc_profile_data = neko_safe_malloc(ase->color_profile.icc_profile_data_length);
                        memcpy(ase->color_profile.icc_profile_data, s->in, ase->color_profile.icc_profile_data_length);
                        s->in += ase->color_profile.icc_profile_data_length;
                    }
                } break;

                case 0x2018:  // Tags chunk.
                {
                    ase->tag_count = (int)s_read_uint16(s);
                    s_skip(s, 8);  // For future (set to zero).
                    NEKO_ASSERT(ase->tag_count < __NEKO_ASEPRITE_MAX_TAGS);
                    for (int k = 0; k < ase->tag_count; ++k) {
                        ase_tag_t tag;
                        tag.from_frame = (int)s_read_uint16(s);
                        tag.to_frame = (int)s_read_uint16(s);
                        tag.loop_animation_direction = (ase_animation_direction_t)s_read_uint8(s);
                        tag.repeat = s_read_uint16(s);
                        s_skip(s, 6);  // For future (set to zero).
                        tag.r = s_read_uint8(s);
                        tag.g = s_read_uint8(s);
                        tag.b = s_read_uint8(s);
                        s_skip(s, 1);  // Extra byte (zero).
                        tag.name = s_read_string(s);
                        ase->tags[k] = tag;
                    }
                    was_on_tags = 1;
                } break;

                case 0x2019:  // Palette chunk.
                {
                    ase->palette.entry_count = (int)s_read_uint32(s);
                    NEKO_ASSERT(ase->palette.entry_count <= __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES);
                    int first_index = (int)s_read_uint32(s);
                    int last_index = (int)s_read_uint32(s);
                    s_skip(s, 8);  // For future (set to zero).
                    for (int k = first_index; k <= last_index; ++k) {
                        int has_name = s_read_uint16(s);
                        ase_palette_entry_t entry;
                        entry.color.r = s_read_uint8(s);
                        entry.color.g = s_read_uint8(s);
                        entry.color.b = s_read_uint8(s);
                        entry.color.a = s_read_uint8(s);
                        if (has_name) {
                            entry.color_name = s_read_string(s);
                        } else {
                            entry.color_name = NULL;
                        }
                        ase->palette.entries[k] = entry;
                    }
                } break;

                case 0x2020:  // Udata chunk.
                {
                    NEKO_ASSERT(last_udata || was_on_tags);
                    if (was_on_tags && !last_udata) {
                        NEKO_ASSERT(tag_index < ase->tag_count);
                        last_udata = &ase->tags[tag_index++].udata;
                    }
                    int flags = (int)s_read_uint32(s);
                    if (flags & 1) {
                        last_udata->has_text = 1;
                        last_udata->text = s_read_string(s);
                    }
                    if (flags & 2) {
                        last_udata->color.r = s_read_uint8(s);
                        last_udata->color.g = s_read_uint8(s);
                        last_udata->color.b = s_read_uint8(s);
                        last_udata->color.a = s_read_uint8(s);
                    }
                    last_udata = NULL;
                } break;

                case 0x2022:  // Slice chunk.
                {
                    int slice_count = (int)s_read_uint32(s);
                    int flags = (int)s_read_uint32(s);
                    s_skip(s, sizeof(u32));  // Reserved.
                    const char *name = s_read_string(s);
                    for (int k = 0; k < (int)slice_count; ++k) {
                        ase_slice_t slice = {0};
                        slice.name = name;
                        slice.frame_number = (int)s_read_uint32(s);
                        slice.origin_x = (int)s_read_int32(s);
                        slice.origin_y = (int)s_read_int32(s);
                        slice.w = (int)s_read_uint32(s);
                        slice.h = (int)s_read_uint32(s);
                        if (flags & 1) {
                            // It's a 9-patches slice.
                            slice.has_center_as_9_slice = 1;
                            slice.center_x = (int)s_read_int32(s);
                            slice.center_y = (int)s_read_int32(s);
                            slice.center_w = (int)s_read_uint32(s);
                            slice.center_h = (int)s_read_uint32(s);
                        }
                        if (flags & 2) {
                            // Has pivot information.
                            slice.has_pivot = 1;
                            slice.pivot_x = (int)s_read_int32(s);
                            slice.pivot_y = (int)s_read_int32(s);
                        }
                        NEKO_ASSERT(ase->slice_count < __NEKO_ASEPRITE_MAX_SLICES);
                        ase->slices[ase->slice_count++] = slice;
                        last_udata = &ase->slices[ase->slice_count - 1].udata;
                    }
                } break;

                default:
                    s_skip(s, (int)chunk_size);
                    break;
            }

            u32 size_read = (u32)(s->in - chunk_start);
            NEKO_ASSERT(size_read == chunk_size);
        }
    }

    return ase;
}

void neko_aseprite_default_blend_bind(ase_t *ase) {

    NEKO_ASSERT(ase);
    NEKO_ASSERT(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        frame->pixels[0] = (neko_color_t *)neko_safe_malloc((size_t)(sizeof(neko_color_t)) * ase->w * ase->h);
        memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t *dst = frame->pixels[0];

        // neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            if (!(cel->layer->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE))) {
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
            void *src = cel->cel_pixels;
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
                    neko_color_t src_color = s_color(ase, src, cw * sy + sx);
                    neko_color_t dst_color = dst[dst_index];
                    neko_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

void neko_aseprite_free(ase_t *ase) {
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        neko_safe_free(frame->pixels[0]);
        for (int j = 0; j < frame->cel_count; ++j) {
            ase_cel_t *cel = frame->cels + j;
            neko_safe_free(cel->cel_pixels);
            neko_safe_free((void *)cel->udata.text);
        }
    }
    for (int i = 0; i < ase->layer_count; ++i) {
        ase_layer_t *layer = ase->layers + i;
        neko_safe_free((void *)layer->name);
        neko_safe_free((void *)layer->udata.text);
    }
    for (int i = 0; i < ase->tag_count; ++i) {
        ase_tag_t *tag = ase->tags + i;
        neko_safe_free((void *)tag->name);
    }
    for (int i = 0; i < ase->slice_count; ++i) {
        ase_slice_t *slice = ase->slices + i;
        neko_safe_free((void *)slice->udata.text);
    }
    if (ase->slice_count) {
        neko_safe_free((void *)ase->slices[0].name);
    }
    for (int i = 0; i < ase->palette.entry_count; ++i) {
        neko_safe_free((void *)ase->palette.entries[i].color_name);
    }
    neko_safe_free(ase->color_profile.icc_profile_data);
    neko_safe_free(ase->frames);
    neko_safe_free(ase);
}