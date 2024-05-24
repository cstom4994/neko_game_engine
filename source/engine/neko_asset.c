
#include "neko_asset.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/neko.h"

// builtin
#include "engine/neko_aseprite.h"
#include "engine/neko_png.h"

#if defined(NEKO_PLATFORM_LINUX)
#include <errno.h>
#define fopen_s fopen
#endif

neko_asset_t __neko_asset_handle_create_impl(u64 type_id, u32 asset_id, u32 importer_id) {
    neko_asset_t asset = neko_default_val();
    asset.type_id = type_id;
    asset.asset_id = asset_id;
    asset.importer_id = importer_id;
    return asset;
}

neko_asset_manager_t neko_asset_manager_new() {
    neko_asset_manager_t assets = neko_default_val();

    // Register default asset importers
    neko_asset_importer_desc_t tex_desc = neko_default_val();
    neko_asset_importer_desc_t font_desc = neko_default_val();
    // neko_asset_importer_desc_t audio_desc = neko_default_val();
    neko_asset_importer_desc_t mesh_desc = neko_default_val();
    neko_asset_importer_desc_t asset_desc = neko_default_val();

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
        neko_assert(false);
        return NULL;
    }

    // Need to grab the appropriate importer based on type
    if (!neko_hash_table_key_exists(am->importers, type_id)) {
        neko_println("Warning: Importer type %zu does not exist.", type_id);
        neko_assert(false);
        return NULL;
    }

    neko_asset_importer_t *imp = neko_hash_table_getp(am->importers, type_id);

    // Vertify that importer id and handle importer id align
    if (imp->importer_id != hndl.importer_id) {
        neko_println("Warning: Importer id: %zu does not match handle importer id: %zu.", imp->importer_id, hndl.importer_id);
        neko_assert(false);
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
    neko_asset_t a = neko_default_val();
    return a;
}

void neko_asset_default_load_from_file(const_str path, void *out) {
    // Nothing...
}

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// STB
#include "deps/imgui/imstb_rectpack.h"
#include "deps/imgui/imstb_truetype.h"

NEKO_API_DECL bool32_t neko_util_load_texture_data_from_memory(const void *memory, size_t sz, s32 *width, s32 *height, u32 *num_comps, void **data, bool32_t flip_vertically_on_load) {
    // Load texture data

    neko_png_image_t img = neko_png_load_mem(memory, sz);

    if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = img.pix;
    *width = img.w;
    *height = img.h;

    if (!*data) {
        neko_png_free(&img);
        return false;
    }
    return true;
}

NEKO_API_DECL bool neko_asset_texture_load_from_file(const_str path, void *out, neko_graphics_texture_desc_t *desc, bool32_t flip_on_load, bool32_t keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(neko_graphics_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    neko_png_image_t img = neko_png_load(path);

    if (t->desc.flip_y) neko_png_flip_image_horizontal(&img);

    t->desc.data[0] = img.pix;
    t->desc.width = img.w;
    t->desc.height = img.h;

    if (!t->desc.data) {
        neko_log_warning("failed to load texture data %s", path);
        return false;
    }

    t->hndl = neko_graphics_texture_create(t->desc);

    if (!keep_data) {
        neko_png_free(&img);
        *t->desc.data = NULL;
    }

    return true;
}

/*
bool neko_asset_texture_load_from_file(const char* path, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data)
{
    size_t len = 0;
    char* file_data = neko_platform_read_file_contents(path, "rb", &len);
    neko_assert(file_data);
    bool32_t ret = neko_asset_texture_load_from_memory(file_data, len, out, desc, flip_on_load, keep_data);
    neko_free(file_data);
    return ret;
}
 */

bool neko_asset_texture_load_from_memory(const void *memory, size_t sz, void *out, neko_graphics_texture_desc_t *desc, bool32_t flip_on_load, bool32_t keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(neko_graphics_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    s32 num_comps = 0;
    bool32_t loaded = neko_util_load_texture_data_from_memory(memory, sz, (s32 *)&t->desc.width, (s32 *)&t->desc.height, (u32 *)&num_comps, (void **)&t->desc.data, t->desc.flip_y);

    if (!loaded) {
        return false;
    }

    t->hndl = neko_graphics_texture_create(t->desc);

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
        neko_log_warning("font: %s: point size not declared. setting to default 16.", neko_fs_get_filename(path));
        point_size = 16;
    }
    bool ret = neko_asset_ascii_font_load_from_memory(ttf, len, out, point_size);
    if (!ret) {
        neko_log_warning("font failed to load: %s", neko_fs_get_filename(path));
    } else {
        neko_log_trace("font successfully loaded: %s", neko_fs_get_filename(path));
    }
    neko_safe_free(ttf);
    return ret;
}

bool neko_asset_ascii_font_load_from_memory(const void *memory, size_t sz, void *out, u32 point_size) {
    neko_asset_ascii_font_t *f = (neko_asset_ascii_font_t *)out;

    if (!point_size) {
        neko_log_warning("font: point size not declared. setting to default 16.");
        point_size = 16;
    }

    // Poor attempt at an auto resized texture
    const u32 point_wh = neko_max(point_size, 32);
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

    neko_graphics_texture_desc_t desc = neko_default_val();
    desc.width = w;
    desc.height = h;
    *desc.data = flipmap;
    desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;

    // 使用位图数据生成位图的图集纹理
    f->texture.hndl = neko_graphics_texture_create(desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    bool success = false;
    if (v <= 0) {
        neko_log_warning("font failed to load, baked texture was too small: %d", v);
    } else {
        neko_log_trace("font baked size: %d", v);
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
            stbtt_aligned_quad q = neko_default_val();
            stbtt_GetBakedQuad((stbtt_bakedchar *)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            h = neko_max(neko_max(h, fabsf(q.y0)), fabsf(q.y1));
        }
        txt++;
    };
    return h;
}

NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions(const neko_asset_ascii_font_t *fp, const_str text, s32 len) { return neko_asset_ascii_font_text_dimensions_ex(fp, text, len, 0); }

NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions_ex(const neko_asset_ascii_font_t *fp, const_str text, s32 len, bool32_t include_past_baseline) {
    neko_vec2 dimensions = neko_v2s(0.f);

    if (!fp || !text) return dimensions;
    f32 x = 0.f;
    f32 y = 0.f;
    f32 y_under = 0;

    while (text[0] != '\0' && len--) {
        char c = text[0];
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q = neko_default_val();
            stbtt_GetBakedQuad((stbtt_bakedchar *)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);
            dimensions.x = neko_max(dimensions.x, x);
            dimensions.y = neko_max(dimensions.y, fabsf(q.y0));
            if (include_past_baseline) y_under = neko_max(y_under, fabsf(q.y1));
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
        neko_assert(false);
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
            neko_asset_mesh_primitive_t prim = neko_default_val();
            prim.count = m->index_sizes[p] / sizeof(u16);

            // Vertex buffer decl
            neko_graphics_vertex_buffer_desc_t vdesc = neko_default_val();
            vdesc.data = m->vertices[p];
            vdesc.size = m->vertex_sizes[p];

            // Construct vertex buffer for primitive
            prim.vbo = neko_graphics_vertex_buffer_create(vdesc);

            // Index buffer decl
            neko_graphics_index_buffer_desc_t idesc = neko_default_val();
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = neko_graphics_index_buffer_create(idesc);

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

neko_static_inline u16 UnalignedLoad16(const void *p) { return *(const u16 *)(p); }
neko_static_inline u32 UnalignedLoad32(const void *p) { return *(const u32 *)(p); }
neko_static_inline void UnalignedStore16(void *p, u16 x) { *(u16 *)(p) = x; }
neko_static_inline void UnalignedCopy64(void *d, const void *s) { *(u64 *)(d) = *(const u64 *)(s); }

neko_static_inline void __neko_ulz_wild_copy(u8 *d, const u8 *s, int n) {
    UnalignedCopy64(d, s);

    for (int i = 8; i < n; i += 8) UnalignedCopy64(d + i, s + i);
}

neko_static_inline u32 __neko_ulz_hash32(const void *p) { return (UnalignedLoad32(p) * 0x9E3779B9) >> (32 - NEKO_LZ_HASH_BITS); }

neko_static_inline void __neko_ulz_encode_mod(u8 **p, u32 x) {
    while (x >= 128) {
        x -= 128;
        *(*p)++ = 128 + (x & 127);
        x >>= 7;
    }
    *(*p)++ = x;
}

neko_static_inline u32 __neko_ulz_decode_mod(const u8 **p) {
    u32 x = 0;
    for (int i = 0; i <= 21; i += 7) {
        const u32 c = *(*p)++;
        x += c << i;
        if (c < 128) break;
    }
    return x;
}

// LZ77

neko_global int __neko_ulz_compress_fast(const u8 *in, int inlen, u8 *out, int outlen) {
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

neko_global int __neko_ulz_compress(const u8 *in, int inlen, u8 *out, int outlen, int level) {
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

neko_global int __neko_ulz_decompress(const u8 *in, int inlen, u8 *out, int outlen) {
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
inline static FILE *openFile(const char *filePath, const char *mode) {
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

neko_private(void) destroy_pack_items(u64 item_count, pack_item *items) {
    neko_assert(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) neko_safe_free(items[i].path);
    neko_safe_free(items);
}

neko_private(neko_pack_result) create_pack_items(FILE *packFile, u64 item_count, pack_item **_items) {
    neko_assert(packFile);
    neko_assert(item_count > 0);
    neko_assert(_items);

    pack_item *items = (pack_item *)neko_safe_malloc(item_count * sizeof(pack_item));

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

        char *path = (char *)neko_safe_malloc((info.path_size + 1) * sizeof(char));

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

neko_pack_result neko_pack_read(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory, neko_packreader_t *pack_reader) {
    neko_assert(file_path);
    neko_assert(pack_reader);

    // neko_packreader_t *pack = (neko_packreader_t *)neko_safe_calloc(1, sizeof(neko_packreader_t));

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

    neko_pack_result pack_result = create_pack_items(file, item_count, &items);

    if (pack_result != 0) {
        neko_pack_destroy(pack_reader);
        ;
        return pack_result;
    }

    pack_reader->item_count = item_count;
    pack_reader->items = items;

    u8 *data_buffer;

    if (data_buffer_capacity > 0) {
        data_buffer = (u8 *)neko_safe_malloc(data_buffer_capacity * sizeof(u8));

        if (!data_buffer) {
            neko_pack_destroy(pack_reader);
            return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
        }
    } else {
        data_buffer = NULL;
    }

    pack_reader->data_buffer = data_buffer;
    pack_reader->data_size = data_buffer_capacity;

    neko_log_trace("load pack %s buildnum: %d (engine %d)", neko_fs_get_filename(file_path), buildnum, neko_buildnum());

    //*pack_reader = pack;
    return 0;
}
void neko_pack_destroy(neko_packreader_t *pack_reader) {
    if (!pack_reader) return;

    if (pack_reader->file_ref_count != 0) {
        neko_log_warning("assets loader leaks detected %d refs", pack_reader->file_ref_count);
    }

    neko_safe_free(pack_reader->data_buffer);
    neko_safe_free(pack_reader->zip_buffer);
    destroy_pack_items(pack_reader->item_count, pack_reader->items);
    if (pack_reader->file) closeFile(pack_reader->file);
    // neko_safe_free(pack_reader);
}

u64 neko_pack_item_count(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    return pack_reader->item_count;
}

neko_private(int) neko_compare_pack_items(const void *_a, const void *_b) {
    // NOTE: a and b should not be NULL!
    // Skipping here neko_assertions for debug build speed.

    const pack_item *a = (pack_item *)_a;
    const pack_item *b = (pack_item *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

b8 neko_pack_item_index(neko_packreader_t *pack_reader, const_str path, u64 *index) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(index);
    neko_assert(strlen(path) <= UINT8_MAX);

    pack_item *search_item = &pack_reader->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    pack_item *item = (pack_item *)bsearch(search_item, pack_reader->items, pack_reader->item_count, sizeof(pack_item), neko_compare_pack_items);

    if (!item) return false;

    *index = item - pack_reader->items;
    return true;
}

u32 neko_pack_item_size(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->item_count);
    return pack_reader->items[index].info.data_size;
}

const_str neko_pack_item_path(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->item_count);
    return pack_reader->items[index].path;
}

neko_pack_result neko_pack_item_data_with_index(neko_packreader_t *pack_reader, u64 index, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->item_count);
    neko_assert(data);
    neko_assert(size);

    pack_iteminfo info = pack_reader->items[index].info;
    u8 *data_buffer = pack_reader->data_buffer;

    if (data_buffer) {
        if (info.data_size > pack_reader->data_size) {
            data_buffer = (u8 *)neko_safe_realloc(data_buffer, info.data_size * sizeof(u8));

            if (!data_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            pack_reader->data_buffer = data_buffer;
            pack_reader->data_size = info.data_size;
        }
    } else {
        data_buffer = (u8 *)neko_safe_malloc(info.data_size * sizeof(u8));

        if (!data_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

        pack_reader->data_buffer = data_buffer;
        pack_reader->data_size = info.data_size;
    }

    u8 *zip_buffer = pack_reader->zip_buffer;

    if (zip_buffer) {
        if (info.zip_size > pack_reader->zip_size) {
            zip_buffer = (u8 *)neko_safe_realloc(zip_buffer, info.zip_size * sizeof(u8));

            if (!zip_buffer) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

            pack_reader->zip_buffer = zip_buffer;
            pack_reader->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            zip_buffer = (u8 *)neko_safe_malloc(info.zip_size * sizeof(u8));

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

        neko_log_trace("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

        if (result < 0 || result != info.data_size) {
            return -1; /*FAILED_TO_DECOMPRESS_PACK_RESULT*/
        }
    } else {
        size_t result = fread(data_buffer, sizeof(u8), info.data_size, file);

        if (result != info.data_size) return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/
    }

    //(*data) = data_buffer;
    (*size) = info.data_size;

    (*data) = neko_safe_malloc(info.data_size);

    memcpy((void *)(*data), data_buffer, info.data_size);

    pack_reader->file_ref_count++;

    return 0;
}

neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, const_str path, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(data);
    neko_assert(size);
    neko_assert(strlen(path) <= UINT8_MAX);

    u64 index;

    if (!neko_pack_item_index(pack_reader, path, &index)) {
        return -1; /*FAILED_TO_GET_ITEM_PACK_RESULT*/
    }

    return neko_pack_item_data_with_index(pack_reader, index, data, size);
}

void neko_pack_item_free(neko_packreader_t *pack_reader, void *data) {
    neko_safe_free(data);
    pack_reader->file_ref_count--;
}

void neko_pack_free_buffers(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    neko_safe_free(pack_reader->data_buffer);
    neko_safe_free(pack_reader->zip_buffer);
    pack_reader->data_buffer = NULL;
    pack_reader->zip_buffer = NULL;
}

neko_private(void) neko_pack_remove_item(u64 item_count, pack_item *pack_items) {
    neko_assert(item_count == 0 || (item_count > 0 && pack_items));

    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

neko_pack_result neko_pack_unzip(const_str file_path, b8 print_progress) {
    neko_assert(file_path);

    neko_packreader_t pack_reader;

    neko_pack_result pack_result = neko_pack_read(file_path, 128, false, &pack_reader);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pack_reader.item_count;
    pack_item *items = pack_reader.items;

    for (u64 i = 0; i < item_count; i++) {
        pack_item *item = &items[i];

        if (print_progress) {
            neko_log_info("Unpacking %s", item->path);
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

neko_private(neko_pack_result) neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, b8 print_progress) {
    neko_assert(pack_file);
    neko_assert(item_count > 0);
    neko_assert(item_paths);

    u32 buffer_size = 128;  // 提高初始缓冲大小 修复 neko_safe_realloc 异常释放

    u8 *item_data = (u8 *)neko_safe_malloc(sizeof(u8) * buffer_size);
    if (!item_data) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u8 *zip_data = (u8 *)neko_safe_malloc(sizeof(u8) * buffer_size);
    if (!zip_data) {
        neko_safe_free(item_data);
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
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        FILE *item_file = openFile(item_path, "rb");

        if (!item_file) {
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*FAILED_TO_OPEN_FILE_PACK_RESULT*/
        }

        int seek_result = seekFile(item_file, 0, SEEK_END);

        if (seek_result != 0) {
            closeFile(item_file);
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        u64 item_size = (u64)tellFile(item_file);

        if (item_size == 0 || item_size > UINT32_MAX) {
            closeFile(item_file);
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*BAD_DATA_SIZE_PACK_RESULT*/
        }

        seek_result = seekFile(item_file, 0, SEEK_SET);

        if (seek_result != 0) {
            closeFile(item_file);
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/
        }

        if (item_size > buffer_size) {
            u8 *new_buffer = (u8 *)neko_safe_realloc(item_data, item_size * sizeof(u8));

            if (!new_buffer) {
                closeFile(item_file);
                neko_safe_free(zip_data);
                neko_safe_free(item_data);
                return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            item_data = new_buffer;

            new_buffer = (u8 *)neko_safe_realloc(zip_data, item_size * sizeof(u8));

            if (!new_buffer) {
                closeFile(item_file);
                neko_safe_free(zip_data);
                neko_safe_free(item_data);
                return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/
            }

            zip_data = new_buffer;
        }

        size_t result = fread(item_data, sizeof(u8), item_size, item_file);

        closeFile(item_file);

        if (result != item_size) {
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
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
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        result = fwrite(item_path, sizeof(char), info.path_size, pack_file);

        if (result != info.path_size) {
            neko_safe_free(zip_data);
            neko_safe_free(item_data);
            return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, pack_file);

            if (result != zip_size) {
                neko_safe_free(zip_data);
                neko_safe_free(item_data);
                return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
            }
        } else {
            result = fwrite(item_data, sizeof(u8), item_size, pack_file);

            if (result != item_size) {
                neko_safe_free(zip_data);
                neko_safe_free(item_data);
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

    neko_safe_free(zip_data);
    neko_safe_free(item_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size, compression);
    }

    return 0;
}

neko_private(int) neko_pack_compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

neko_pack_result neko_pack_build(const_str file_path, u64 file_count, const_str *file_paths, b8 print_progress) {
    neko_assert(file_path);
    neko_assert(file_count > 0);
    neko_assert(file_paths);

    char **item_paths = (char **)neko_safe_malloc(file_count * sizeof(char *));

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
        neko_safe_free(item_paths);
        return -1; /*FAILED_TO_CREATE_FILE_PACK_RESULT*/
    }

    char header[neko_pack_head_size] = {
            'P', 'A', 'C', 'K', 0, 0, 0, !neko_little_endian,
    };

    s32 buildnum = neko_buildnum();

    size_t write_result = fwrite(header, sizeof(u8), neko_pack_head_size, pack_file);
    write_result += fwrite(&buildnum, sizeof(s32), 1, pack_file);

    if (write_result != neko_pack_head_size + 1) {
        neko_safe_free(item_paths);
        closeFile(pack_file);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    write_result = fwrite(&item_count, sizeof(u64), 1, pack_file);

    if (write_result != 1) {
        neko_safe_free(item_paths);
        closeFile(pack_file);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    neko_pack_result pack_result = neko_write_pack_items(pack_file, item_count, item_paths, print_progress);

    neko_safe_free(item_paths);
    closeFile(pack_file);

    if (pack_result != 0) {
        remove(file_path);
        return pack_result;
    }

    return 0;
}

neko_pack_result neko_pack_info(const_str file_path, u8 *pack_version, b8 *isLittleEndian, u64 *_item_count) {
    neko_assert(file_path);
    neko_assert(pack_version);
    neko_assert(isLittleEndian);
    neko_assert(_item_count);

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

#pragma region text_draw

void neko_fontbatch_init(neko_fontbatch_t *font_batch, const neko_vec2_t fbs, const_str img_path, char *content, int content_size) {

    font_batch->font_scale = 3.0f;

    font_batch->font_render = neko_graphics_batch_make_ctx(32);

    const char *font_vs =
            "#version 330\n"
            "uniform mat4 u_mvp;\n"
            "in vec2 in_pos; in vec2 in_uv;\n"
            "out vec2 v_uv;\n"
            "void main() {\n"
            "    v_uv = in_uv;\n"
            "    gl_Position = u_mvp * vec4(in_pos, 0, 1);\n"
            "}\n";
    const char *font_ps =
            "#version 330\n"
            "precision mediump float;\n"
            "uniform sampler2D u_sprite_texture;\n"
            "in vec2 v_uv; out vec4 out_col;\n"
            "void main() { out_col = texture(u_sprite_texture, v_uv); }\n";

    neko_graphics_batch_vertex_data_t font_vd;
    neko_graphics_batch_make_vertex_data(&font_vd, 1024 * 1024, GL_TRIANGLES, sizeof(neko_font_vert_t), GL_DYNAMIC_DRAW);
    neko_graphics_batch_add_attribute(&font_vd, "in_pos", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, x));
    neko_graphics_batch_add_attribute(&font_vd, "in_uv", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, u));

    neko_graphics_batch_make_renderable(&font_batch->font_renderable, &font_vd);
    neko_graphics_batch_load_shader(&font_batch->font_shader, font_vs, font_ps);
    neko_graphics_batch_set_shader(&font_batch->font_renderable, &font_batch->font_shader);

    neko_graphics_batch_ortho_2d(fbs.x / font_batch->font_scale, fbs.y / font_batch->font_scale, 0, 0, font_batch->font_projection);

    neko_graphics_batch_send_matrix(&font_batch->font_shader, "u_mvp", font_batch->font_projection);

    neko_png_image_t img = neko_png_load(img_path);
    font_batch->font_tex_id = generate_texture_handle(img.pix, img.w, img.h, NULL);
    font_batch->font = neko_font_load_bmfont(font_batch->font_tex_id, content, content_size, 0);
    if (font_batch->font->atlas_w != img.w || font_batch->font->atlas_h != img.h) {
        neko_log_warning("failed to load font");
    }
    neko_png_free(&img);

    font_batch->font_verts = (neko_font_vert_t *)neko_safe_malloc(sizeof(neko_font_vert_t) * 1024 * 2);
}

void neko_fontbatch_draw(neko_fontbatch_t *font_batch, const neko_vec2_t fbs, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font_batch->font, text);
    f32 text_h = (f32)neko_font_text_height(font_batch->font, text);

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    if (scale == 0.f) scale = font_batch->font_scale;

    neko_font_rect_t clip_rect;
    clip_rect.left = -fbs.x / scale * clip_region;
    clip_rect.right = fbs.x / scale * clip_region + 0.5f;
    clip_rect.top = fbs.y / scale * clip_region + 0.5f;
    clip_rect.bottom = -fbs.y / scale * clip_region;

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale + text_h / 2.f;
    f32 wrap_width = wrap_x - x0;

    neko_font_fill_vertex_buffer(font_batch->font, text, x0, y0, wrap_width, line_height, &clip_rect, font_batch->font_verts, 1024 * 2, &font_batch->font_vert_count);

    if (font_batch->font_vert_count) {
        neko_graphics_batch_draw_call_t call;
        call.textures[0] = (u32)font_batch->font->atlas_id;
        call.texture_count = 1;
        call.r = &font_batch->font_renderable;
        call.verts = font_batch->font_verts;
        call.vert_count = font_batch->font_vert_count;

        neko_graphics_batch_push_draw_call(font_batch->font_render, call);
    }
}

#pragma endregion text_draw

#pragma region aseprite

bool neko_aseprite_load(neko_aseprite *spr, const_str filepath) {

    ase_t *ase = neko_aseprite_load_from_file(filepath);

    if (NULL == ase) {
        neko_log_error("unable to load ase %s", filepath);
        return false;
    }

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;

        frame->pixels[0] = (neko_color_t *)neko_safe_malloc((int)(sizeof(neko_color_t)) * ase->w * ase->h);

        spr->mem_used += (sizeof(neko_color_t)) * ase->w * ase->h;

        memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t *dst = frame->pixels[0];

        // neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            // neko_println_debug(" - %s", cel->layer->name);

            // 确定图块所在层与父层可视
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
                neko_assert(found);
            }

            void *src = cel->cel_pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -neko_min(cx, 0);
            int ct = -neko_min(cy, 0);
            int dl = neko_max(cx, 0);
            int dt = neko_max(cy, 0);
            int dr = neko_min(ase->w, cw + cx);
            int db = neko_min(ase->h, ch + cy);
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

    s32 rect = ase->w * ase->h * 4;

    neko_aseprite s = neko_default_val();

    // neko_array<neko_sprite_frame> frames = {};
    // neko_array_reserve(&frames, ase->frame_count);

    // neko_array<u8> pixels = {};
    // neko_array_reserve(&pixels, ase->frame_count * rect);
    //  neko_defer([&] { neko_array_dctor(&pixels); });

    u8 *pixels = neko_safe_malloc(ase->frame_count * rect);

    for (s32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t *frame = &ase->frames[i];

        neko_aseprite_frame sf = neko_default_val();
        sf.duration = frame->duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (f32)(i + 1) / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (f32)i / ase->frame_count;

        neko_dyn_array_push(s.frames, sf);

        neko_color_t *data = frame->pixels[0];

        // 不知道是直接读取aseprite解析的数据还是像这样拷贝为一个贴图 在渲染时改UV来得快
        memcpy(pixels + (i * rect), &data[0].r, rect);
    }

    neko_graphics_t *gfx = neko_instance()->ctx.graphics;

    neko_graphics_texture_desc_t t_desc = neko_default_val();

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h * ase->frame_count;
    // t_desc.num_comps = 4;

    // 大小为 ase->frame_count * rect
    // neko_tex_flip_vertically(ase->w, ase->h * ase->frame_count, (u8 *)pixels.data);
    t_desc.data[0] = pixels;

    neko_texture_t tex = neko_graphics_texture_create(t_desc);

    neko_safe_free(pixels);

    // img.width = desc.width;
    // img.height = desc.height;

    // neko_hashmap<neko_sprite_loop> by_tag;
    // neko_hash_table(u64, neko_sprite_loop) by_tag;
    // neko_hashmap_reserve(&by_tag, neko_hashmap_reserve_size((u64)ase->tag_count));

    for (s32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t *tag = &ase->tags[i];

        neko_aseprite_loop loop = neko_default_val();

        for (s32 j = tag->from_frame; j <= tag->to_frame; j++) {
            neko_dyn_array_push(loop.indices, j);
        }

        u64 key = neko_hash_str64(tag->name /*, strlen(tag.name)*/);
        neko_hash_table_insert(s.by_tag, key, loop);
    }

    // for (s32 i = 0; i < ase->layer_count; i++) {
    //     ase_layer_t* layer = &ase->layers[i];
    //     neko_println_debug("%s", layer->name);
    // }

    // neko_log_trace(format("created sprite size({3}) with image id: {0} and {1} frames with {2} layers", tex.id, neko_dyn_array_size(s.frames), ase->layer_count,(spr->mem_used + pixels.capacity *
    // sizeof(u8)) / 1e6).c_str());

    s.img = tex;
    // s.frames = frames;
    //  s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *spr = s;

    neko_aseprite_free(ase);

    return true;
}

void neko_aseprite_end(neko_aseprite *spr) {
    neko_dyn_array_free(spr->frames);

    for (neko_hash_table_iter it = neko_hash_table_iter_new(spr->by_tag); neko_hash_table_iter_valid(spr->by_tag, it); neko_hash_table_iter_advance(spr->by_tag, it)) {
        u64 key = neko_hash_table_iter_getk(spr->by_tag, it);
        neko_aseprite_loop *v = neko_hash_table_getp(spr->by_tag, key);
        neko_dyn_array_free(v->indices);
    }

    neko_hash_table_free(spr->by_tag);
}

void neko_aseprite_renderer_play(neko_aseprite_renderer *sr, const_str tag) {
    neko_aseprite_loop *loop = neko_hash_table_getp(sr->sprite->by_tag, neko_hash_str64(tag));
    if (loop != NULL) sr->loop = loop;
    sr->current_frame = 0;
    sr->elapsed = 0;
}

void neko_aseprite_renderer_update(neko_aseprite_renderer *sr, f32 dt) {
    s32 index;
    u64 len;
    if (sr->loop) {
        index = sr->loop->indices[sr->current_frame];
        len = neko_dyn_array_size(sr->loop->indices);
    } else {
        index = sr->current_frame;
        len = neko_dyn_array_size(sr->sprite->frames);
    }

    neko_aseprite_frame frame = sr->sprite->frames[index];

    sr->elapsed += dt * 1000;
    if (sr->elapsed > frame.duration) {
        if (sr->current_frame == len - 1) {
            sr->current_frame = 0;
        } else {
            sr->current_frame++;
        }

        sr->elapsed -= frame.duration;
    }
}

void neko_aseprite_renderer_set_frame(neko_aseprite_renderer *sr, s32 frame) {
    s32 len;
    if (sr->loop) {
        len = neko_dyn_array_size(sr->loop->indices);
    } else {
        len = neko_dyn_array_size(sr->sprite->frames);
    }

    if (0 <= frame && frame < len) {
        sr->current_frame = frame;
        sr->elapsed = 0;
    }
}

#pragma endregion aseprite

#pragma region tiled

void neko_tiled_load(map_t *map, const_str tmx_path, const_str res_path) {

    map->doc = neko_xml_parse_file(tmx_path);
    if (!map->doc) {
        neko_log_error("Failed to parse XML: %s", neko_xml_get_error());
        return;
    }

    char tmx_root_path[256];
    if (NULL == res_path) {
        neko_util_get_dir_from_file(tmx_root_path, 256, tmx_path);
    } else {
        strcpy(tmx_root_path, res_path);
    }

    neko_xml_node_t *map_node = neko_xml_find_node(map->doc, "map");
    neko_assert(map_node);  // Must have a map node!

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "tileset"); neko_xml_node_iter_next(&it);) {
        tileset_t tileset = {0};

        tileset.first_gid = (u32)neko_xml_find_attribute(it.current, "firstgid")->value.number;

        char tileset_path[256];
        neko_snprintf(tileset_path, 256, "%s/%s", tmx_root_path, neko_xml_find_attribute(it.current, "source")->value.string);
        neko_xml_document_t *tileset_doc = neko_xml_parse_file(tileset_path);
        if (!tileset_doc) {
            neko_log_error("Failed to parse XML from %s: %s", tileset_path, neko_xml_get_error());
            return;
        }

        neko_xml_node_t *tileset_node = neko_xml_find_node(tileset_doc, "tileset");
        tileset.tile_width = (u32)neko_xml_find_attribute(tileset_node, "tilewidth")->value.number;
        tileset.tile_height = (u32)neko_xml_find_attribute(tileset_node, "tileheight")->value.number;
        tileset.tile_count = (u32)neko_xml_find_attribute(tileset_node, "tilecount")->value.number;

        neko_xml_node_t *image_node = neko_xml_find_node_child(tileset_node, "image");
        const char *image_path = neko_xml_find_attribute(image_node, "source")->value.string;

        char full_image_path[256];
        neko_snprintf(full_image_path, 256, "%s/%s", tmx_root_path, image_path);

        FILE *checker = fopen(full_image_path, "rb"); /* Check that the file exists. */
        if (!checker) {
            neko_log_error("Failed to fopen texture file: %s", full_image_path);
            return;
        }
        fclose(checker);

        void *tex_data = NULL;
        s32 w, h;
        u32 cc;
        neko_util_load_texture_data_from_file(full_image_path, &w, &h, &cc, &tex_data, false);

        neko_graphics_texture_desc_t tileset_tex_decl = {.width = (u32)w,
                                                         .height = (u32)h,
                                                         .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                         .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                         .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                         .num_mips = 0};

        tileset_tex_decl.data[0] = tex_data;

        tileset.texture = neko_graphics_texture_create(tileset_tex_decl);

        tileset.width = w;
        tileset.height = h;

        neko_free(tex_data);

        neko_dyn_array_push(map->tilesets, tileset);
    }

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "layer"); neko_xml_node_iter_next(&it);) {
        neko_xml_node_t *layer_node = it.current;

        layer_t layer = {0};
        layer.tint = (neko_color_t){255, 255, 255, 255};

        layer.width = (u32)neko_xml_find_attribute(layer_node, "width")->value.number;
        layer.height = (u32)neko_xml_find_attribute(layer_node, "height")->value.number;

        neko_xml_attribute_t *tint_attrib = neko_xml_find_attribute(layer_node, "tintcolor");
        if (tint_attrib) {
            const char *hexstring = tint_attrib->value.string;
            u32 *cols = (u32 *)layer.tint.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            layer.tint.a = 255;
        }

        neko_xml_node_t *data_node = neko_xml_find_node_child(layer_node, "data");

        const char *encoding = neko_xml_find_attribute(data_node, "encoding")->value.string;

        if (strcmp(encoding, "csv") != 0) {
            neko_log_error("%s", "Only CSV data encoding is supported.");
            return;
        }

        const char *data_text = data_node->text;

        const char *cd_ptr = data_text;

        layer.tiles = (tile_t *)neko_safe_malloc(layer.width * layer.height * sizeof(tile_t));

        for (u32 y = 0; y < layer.height; y++) {
            for (u32 x = 0; x < layer.width; x++) {
                u32 gid = (u32)strtod(cd_ptr, NULL);
                u32 tls_id = 0;

                u32 closest = 0;
                for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
                    if (map->tilesets[i].first_gid <= gid) {
                        if (map->tilesets[i].first_gid > closest) {
                            closest = map->tilesets[i].first_gid;
                            tls_id = i;
                        }
                    }
                }

                layer.tiles[x + y * layer.width].id = gid;
                layer.tiles[x + y * layer.width].tileset_id = tls_id;

                while (*cd_ptr && *cd_ptr != ',') {
                    cd_ptr++;
                }

                cd_ptr++; /* Skip the comma. */
            }
        }

        neko_dyn_array_push(map->layers, layer);
    }

    for (neko_xml_node_iter_t it = neko_xml_new_node_child_iter(map_node, "objectgroup"); neko_xml_node_iter_next(&it);) {
        neko_xml_node_t *object_group_node = it.current;

        object_group_t object_group = {0};
        object_group.color = (neko_color_t){255, 255, 255, 255};

        // 对象组名字
        neko_xml_attribute_t *name_attrib = neko_xml_find_attribute(object_group_node, "name");
        if (name_attrib) {
            const char *namestring = name_attrib->value.string;

            object_group.name = name_attrib->value.string;
            // u32 *cols = (u32 *)object_group.color.rgba;
            //*cols = (u32)strtol(hexstring + 1, NULL, 16);
            // object_group.color.a = 128;
            neko_println("objectgroup: %s", namestring);
        } else {
        }

        // 对象组默认颜色
        neko_xml_attribute_t *color_attrib = neko_xml_find_attribute(object_group_node, "color");
        if (color_attrib) {
            const char *hexstring = color_attrib->value.string;
            u32 *cols = (u32 *)object_group.color.rgba;
            *cols = (u32)strtol(hexstring + 1, NULL, 16);
            object_group.color.a = 128;
        }

        for (neko_xml_node_iter_t iit = neko_xml_new_node_child_iter(object_group_node, "object"); neko_xml_node_iter_next(&iit);) {
            neko_xml_node_t *object_node = iit.current;

            object_t object = {0};
            object.id = (s32)neko_xml_find_attribute(object_node, "id")->value.number;
            object.x = (s32)neko_xml_find_attribute(object_node, "x")->value.number;
            object.y = (s32)neko_xml_find_attribute(object_node, "y")->value.number;

            object.phy_type = C2_TYPE_POLY;

            neko_xml_attribute_t *attrib;
            if (attrib = neko_xml_find_attribute(object_node, "width")) {
                object.width = attrib->value.number;
            } else {
                object.width = 1;
            }

            if (attrib = neko_xml_find_attribute(object_node, "height")) {
                object.height = attrib->value.number;
            } else {
                object.height = 1;
            }

            object.aabb = (c2AABB){c2V(object.x, object.y), c2V(object.width, object.height)};

            if (object.phy_type == C2_TYPE_POLY) {
                object.phy.poly.verts[0] = (top_left(object.aabb));
                object.phy.poly.verts[1] = (bottom_left(object.aabb));
                object.phy.poly.verts[2] = (bottom_right(object.aabb));
                object.phy.poly.verts[3] = c2Add(top_right(object.aabb), c2Mulvs(bottom_right(object.aabb), 0.5f));
                object.phy.poly.count = 4;
                c2Norms(object.phy.poly.verts, object.phy.poly.norms, object.phy.poly.count);
            }

            neko_dyn_array_push(object_group.objects, object);
        }

        neko_dyn_array_push(map->object_groups, object_group);
    }
}

void neko_tiled_unload(map_t *map) {
    for (u32 i = 0; i < neko_dyn_array_size(map->tilesets); i++) {
        neko_graphics_texture_destroy(map->tilesets[i].texture);
    }

    for (u32 i = 0; i < neko_dyn_array_size(map->layers); i++) {
        neko_safe_free(map->layers[i].tiles);
    }

    neko_dyn_array_free(map->layers);
    neko_dyn_array_free(map->tilesets);

    neko_dyn_array_free(map->object_groups);

    neko_xml_free(map->doc);
}

void neko_tiled_render_init(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, const_str vert_src, const_str frag_src) {

    neko_graphics_vertex_buffer_desc_t vb_decl = {
            .data = NULL,
            .size = BATCH_SIZE * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
            .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
    };

    renderer->vb = neko_graphics_vertex_buffer_create(vb_decl);

    neko_graphics_index_buffer_desc_t ib_decl = {
            .data = NULL,
            .size = BATCH_SIZE * IND_PER_QUAD * sizeof(u32),
            .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
    };

    renderer->ib = neko_graphics_index_buffer_create(ib_decl);

    if (!vert_src || !frag_src) {
        neko_log_error("%s", "Failed to load tiled renderer shaders.");
    }

    neko_graphics_uniform_desc_t u_desc = (neko_graphics_uniform_desc_t){
            .stage = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "batch_texture",
            .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_SAMPLER2D},
    };

    renderer->u_batch_tex = neko_graphics_uniform_create(u_desc);

    renderer->shader = neko_graphics_shader_create((neko_graphics_shader_desc_t){.sources =
                                                                                         (neko_graphics_shader_source_desc_t[]){
                                                                                                 {.type = NEKO_GRAPHICS_SHADER_STAGE_VERTEX, .source = vert_src},
                                                                                                 {.type = NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = frag_src},
                                                                                         },
                                                                                 .size = 2 * sizeof(neko_graphics_shader_source_desc_t),
                                                                                 .name = "tiled_sprite_shader"});

    renderer->u_camera =
            neko_graphics_uniform_create((neko_graphics_uniform_desc_t){.name = "tiled_sprite_camera", .layout = &(neko_graphics_uniform_layout_desc_t){.type = NEKO_GRAPHICS_UNIFORM_MAT4}});

    renderer->pip = neko_graphics_pipeline_create(
            (neko_graphics_pipeline_desc_t){.raster = {.shader = renderer->shader, .index_buffer_element_size = sizeof(uint32_t)},
                                            .layout = {.attrs =
                                                               (neko_graphics_vertex_attribute_desc_t[]){
                                                                       {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "position"},
                                                                       {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "uv"},
                                                                       {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4, .name = "color"},
                                                                       {.format = NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT, .name = "use_texture"},
                                                               },
                                                       .size = 4 * sizeof(neko_graphics_vertex_attribute_desc_t)},
                                            .blend = {.func = NEKO_GRAPHICS_BLEND_EQUATION_ADD, .src = NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA, .dst = NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA}});
}

void neko_tiled_render_deinit(neko_tiled_renderer *renderer) {

    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        neko_dyn_array_free(v);
    }

    neko_hash_table_free(renderer->quad_table);

    neko_graphics_shader_destroy(renderer->shader);
    // neko_command_buffer_free(cb);
}

void neko_tiled_render_begin(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // neko_graphics_clear_desc_t clear = {.actions = &(neko_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.0f}}};
    // neko_graphics_clear(cb, &clear);

    renderer->quad_count = 0;
}

void neko_tiled_render_flush(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // const neko_vec2 ws = neko_platform_window_sizev(neko_platform_main_window());
    // neko_graphics_set_viewport(cb, 0, 0, ws.x, ws.y);

    // renderer->camera_mat = neko_mat4_ortho(0.0f, ws.x, ws.y, 0.0f, -1.0f, 1.0f);

    // clang-format off
    neko_graphics_bind_desc_t binds = {
    .vertex_buffers = {&(neko_graphics_bind_vertex_buffer_desc_t){.buffer = renderer->vb}},
    .index_buffers = {.desc = &(neko_graphics_bind_index_buffer_desc_t){.buffer = renderer->ib}},
    .uniforms = {
        .desc = (neko_graphics_bind_uniform_desc_t[2]){
            {.uniform = renderer->u_camera, .data = &renderer->camera_mat},
            {.uniform = renderer->u_batch_tex, .data = &renderer->batch_texture}
        },
        .size = 2 * sizeof(neko_graphics_bind_uniform_desc_t)
    },
    .image_buffers = {
        .desc = &(neko_graphics_bind_image_buffer_desc_t){renderer->batch_texture, 0, NEKO_GRAPHICS_ACCESS_READ_ONLY},  
        .size = sizeof(neko_graphics_bind_image_buffer_desc_t)
    }};
    // clang-format on

    neko_graphics_pipeline_bind(cb, renderer->pip);
    neko_graphics_apply_bindings(cb, &binds);

    neko_graphics_draw(cb, &(neko_graphics_draw_desc_t){.start = 0, .count = renderer->quad_count * IND_PER_QUAD});

    // neko_check_gl_error();

    renderer->quad_count = 0;
}

void neko_tiled_render_push(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, neko_tiled_quad_t quad) {

    // 如果这个quad的tileset还不存在于quad_table中则插入一个
    // tileset_id为quad_table的键值
    if (!neko_hash_table_exists(renderer->quad_table, quad.tileset_id))                              //
        neko_hash_table_insert(renderer->quad_table, quad.tileset_id, (neko_tiled_quad_list_t){0});  //

    //
    neko_tiled_quad_list_t *quad_list = neko_hash_table_getp(renderer->quad_table, quad.tileset_id);
    neko_dyn_array_push(quad_list->quad_list, quad);
}

void neko_tiled_render_draw(neko_command_buffer_t *cb, neko_tiled_renderer *renderer) {

    // TODO: 23/10/16 检测quad是否在屏幕视角范围外 进行剔除性优化

    // iterate quads hash table
    for (neko_hash_table_iter it = neko_hash_table_iter_new(renderer->quad_table); neko_hash_table_iter_valid(renderer->quad_table, it); neko_hash_table_iter_advance(renderer->quad_table, it)) {
        u32 k = neko_hash_table_iter_getk(renderer->quad_table, it);
        neko_tiled_quad_list_t quad_list = neko_hash_table_iter_get(renderer->quad_table, it);

        neko_dyn_array(neko_tiled_quad_t) v = quad_list.quad_list;

        u32 quad_size = neko_dyn_array_size(v);

        for (u32 i = 0; i < quad_size; i++) {

            neko_tiled_quad_t *quad = &v[i];

            f32 tx = 0.f, ty = 0.f, tw = 0.f, th = 0.f;

            if (quad->use_texture) {
                tx = (f32)quad->rectangle.x / quad->texture_size.x;
                ty = (f32)quad->rectangle.y / quad->texture_size.y;
                tw = (f32)quad->rectangle.z / quad->texture_size.x;
                th = (f32)quad->rectangle.w / quad->texture_size.y;

                // for (u32 i = 0; i < renderer->texture_count; i++) {
                //     if (renderer->textures[i].id == quad->texture.id) {
                //         tex_id = i;
                //         break;
                //     }
                // }

                //// 添加新Tiled贴图
                // if (tex_id == -1) {
                //     renderer->textures[renderer->texture_count] = quad->texture;
                //     tex_id = renderer->texture_count++;
                //     if (renderer->texture_count >= MAX_TEXTURES) {
                //         neko_tiled_render_flush(cb);
                //         tex_id = 0;
                //         renderer->textures[0] = quad->texture;
                //     }
                // }

                renderer->batch_texture = quad->texture;
            }

            const f32 x = quad->position.x;
            const f32 y = quad->position.y;
            const f32 w = quad->dimentions.x;
            const f32 h = quad->dimentions.y;

            const f32 r = (f32)quad->color.r / 255.0f;
            const f32 g = (f32)quad->color.g / 255.0f;
            const f32 b = (f32)quad->color.b / 255.0f;
            const f32 a = (f32)quad->color.a / 255.0f;

            f32 verts[] = {
                    x,     y,     tx,      ty,      r, g, b, a, (f32)quad->use_texture,  //
                    x + w, y,     tx + tw, ty,      r, g, b, a, (f32)quad->use_texture,  //
                    x + w, y + h, tx + tw, ty + th, r, g, b, a, (f32)quad->use_texture,  //
                    x,     y + h, tx,      ty + th, r, g, b, a, (f32)quad->use_texture   //
            };

            const u32 idx_off = renderer->quad_count * VERTS_PER_QUAD;

            u32 indices[] = {idx_off + 3, idx_off + 2, idx_off + 1,   //
                             idx_off + 3, idx_off + 1, idx_off + 0};  //

            neko_graphics_vertex_buffer_request_update(
                    cb, renderer->vb,
                    &(neko_graphics_vertex_buffer_desc_t){.data = verts,
                                                          .size = VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32),
                                                          .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
                                                          .update = {.type = NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * VERTS_PER_QUAD * FLOATS_PER_VERT * sizeof(f32)}});

            neko_graphics_index_buffer_request_update(
                    cb, renderer->ib,
                    &(neko_graphics_index_buffer_desc_t){.data = indices,
                                                         .size = IND_PER_QUAD * sizeof(u32),
                                                         .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC,
                                                         .update = {.type = NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA, .offset = renderer->quad_count * IND_PER_QUAD * sizeof(u32)}});

            renderer->quad_count++;

            if (renderer->quad_count >= BATCH_SIZE) {
                neko_tiled_render_flush(cb, renderer);
            }
        }

        neko_dyn_array_clear(v);

        neko_tiled_render_flush(cb, renderer);
    }
}

#pragma endregion tiled