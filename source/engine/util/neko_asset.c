
#include "neko_asset.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// CGLTF
#include "libs/cgltf/cgltf.h"

// STB
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_rect_pack.h"
#include "libs/stb/stb_truetype.h"

NEKO_API_DECL bool32_t neko_util_load_texture_data_from_memory(const void *memory, size_t sz, s32 *width, s32 *height, u32 *num_comps, void **data, bool32_t flip_vertically_on_load) {
    // Load texture data
    stbi_set_flip_vertically_on_load(flip_vertically_on_load);
    *data = stbi_load_from_memory((const stbi_uc *)memory, (s32)sz, (s32 *)width, (s32 *)height, (s32 *)num_comps, STBI_rgb_alpha);
    if (!*data) {
        neko_free(*data);
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
    FILE *f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    s32 comp = 0;
    stbi_set_flip_vertically_on_load(t->desc.flip_y);
    *t->desc.data = (u8 *)stbi_load_from_file(f, (s32 *)&t->desc.width, (s32 *)&t->desc.height, (s32 *)&comp, STBI_rgb_alpha);

    if (!t->desc.data) {
        fclose(f);
        return false;
    }

    t->hndl = neko_graphics_texture_create(&t->desc);

    if (!keep_data) {
        neko_free(*t->desc.data);
        *t->desc.data = NULL;
    }

    fclose(f);
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

    t->hndl = neko_graphics_texture_create(&t->desc);

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
        neko_log_warning("Warning: Font: %s: Point size not declared. Setting to default 16.", path);
        point_size = 16;
    }
    bool ret = neko_asset_ascii_font_load_from_memory(ttf, len, out, point_size);
    if (!ret) {
        neko_log_warning("Font Failed to Load: %s", path);
    } else {
        neko_log_trace("Font Successfully Loaded: %s", path);
    }
    neko_safe_free(ttf);
    return ret;
}

bool neko_asset_ascii_font_load_from_memory(const void *memory, size_t sz, void *out, u32 point_size) {
    neko_asset_ascii_font_t *f = (neko_asset_ascii_font_t *)out;

    if (!point_size) {
        neko_log_warning("Warning: Font: Point size not declared. Setting to default 16.");
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
    f->texture.hndl = neko_graphics_texture_create(&desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    bool success = false;
    if (v <= 0) {
        neko_log_warning("Font Failed to Load, Baked Texture Was Too Small: %d", v);
    } else {
        neko_log_trace("Font Successfully Loaded: %d", v);
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

bool neko_util_load_gltf_data_from_file(const_str path, neko_asset_mesh_decl_t *decl, neko_asset_mesh_raw_data_t **out, u32 *mesh_count) {
    // Use cgltf like a boss
    cgltf_options options = neko_default_val();
    size_t len = 0;
    char *file_data = neko_platform_read_file_contents(path, "rb", &len);
    neko_println("Loading GLTF: %s", path);

    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, file_data, (cgltf_size)len, &data);
    neko_safe_free(file_data);

    if (result != cgltf_result_success) {
        neko_println("Mesh:LoadFromFile:Failed load gltf");
        cgltf_free(data);
        return false;
    }

    // Load buffers as well
    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        neko_println("Mesh:LoadFromFile:Failed to load buffers");
        return false;
    }

    // Type of index data
    size_t index_element_size = decl ? decl->index_buffer_element_size : 0;

    // Temporary structures
    neko_dyn_array(neko_vec3) positions = NULL;
    neko_dyn_array(neko_vec3) normals = NULL;
    neko_dyn_array(neko_vec3) tangents = NULL;
    neko_dyn_array(neko_color_t) colors = NULL;
    neko_dyn_array(neko_vec2) uvs = NULL;
    neko_dyn_array(neko_asset_mesh_layout_t) layouts = NULL;
    neko_byte_buffer_t v_data = neko_byte_buffer_new();
    neko_byte_buffer_t i_data = neko_byte_buffer_new();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (neko_asset_mesh_raw_data_t *)neko_malloc(data->meshes_count * sizeof(neko_asset_mesh_raw_data_t));
    memset(*out, 0, sizeof(neko_asset_mesh_raw_data_t) * data->meshes_count);

    // Iterate through meshes in data
    for (u32 i = 0; i < data->meshes_count; ++i) {
        // Initialize mesh data
        neko_asset_mesh_raw_data_t *mesh = out[i];
        mesh->prim_count = data->meshes[i].primitives_count;
        mesh->vertex_sizes = (size_t *)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->index_sizes = (size_t *)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->vertices = (void **)neko_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->indices = (void **)neko_malloc(sizeof(size_t) * mesh->prim_count);

        // For each primitive in mesh
        for (u32 p = 0; p < data->meshes[i].primitives_count; ++p) {
            // Clear temp data from previous use
            neko_dyn_array_clear(positions);
            neko_dyn_array_clear(normals);
            neko_dyn_array_clear(tangents);
            neko_dyn_array_clear(uvs);
            neko_dyn_array_clear(colors);
            neko_dyn_array_clear(layouts);
            neko_byte_buffer_clear(&v_data);
            neko_byte_buffer_clear(&i_data);

#define __GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)                                                      \
    do {                                                                                                                              \
        s32 N = 0;                                                                                                                    \
        TYPE *BUF = (TYPE *)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset / sizeof(TYPE) + ATTR->offset / sizeof(TYPE); \
        neko_assert(BUF);                                                                                                             \
        TYPE V[COUNT] = neko_default_val();                                                                                           \
        /* For each vertex */                                                                                                         \
        for (u32 k = 0; k < ATTR->count; k++) {                                                                                       \
            /* For each element */                                                                                                    \
            for (int l = 0; l < COUNT; l++) {                                                                                         \
                V[l] = BUF[N + l];                                                                                                    \
            }                                                                                                                         \
            N += (s32)(ATTR->stride / sizeof(TYPE));                                                                                  \
            /* Add to temp data array */                                                                                              \
            ARR_TYPE ELEM = neko_default_val();                                                                                       \
            memcpy((void *)&ELEM, (void *)V, sizeof(ARR_TYPE));                                                                       \
            neko_dyn_array_push(ARR, ELEM);                                                                                           \
        }                                                                                                                             \
        /* Push into layout */                                                                                                        \
        neko_asset_mesh_layout_t LAYOUT = neko_default_val();                                                                         \
        LAYOUT.type = LAYOUT_TYPE;                                                                                                    \
        neko_dyn_array_push(LAYOUTS, LAYOUT);                                                                                         \
    } while (0)

            // For each attribute in primitive
            for (u32 a = 0; a < data->meshes[i].primitives[p].attributes_count; ++a) {
                // Accessor for attribute data
                cgltf_accessor *attr = data->meshes[i].primitives[p].attributes[a].data;

                // Switch on type for reading data
                switch (data->meshes[i].primitives[p].attributes[a].type) {
                    case cgltf_attribute_type_position: {
                        __GLTF_PUSH_ATTR(attr, f32, 3, positions, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                    } break;

                    case cgltf_attribute_type_normal: {
                        __GLTF_PUSH_ATTR(attr, f32, 3, normals, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                    } break;

                    case cgltf_attribute_type_tangent: {
                        __GLTF_PUSH_ATTR(attr, f32, 3, tangents, neko_vec3, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                    } break;

                    case cgltf_attribute_type_texcoord: {
                        __GLTF_PUSH_ATTR(attr, f32, 2, uvs, neko_vec2, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                    } break;

                    case cgltf_attribute_type_color: {
                        __GLTF_PUSH_ATTR(attr, u8, 4, colors, neko_color_t, layouts, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                    } break;

                    // Not sure what to do with these for now
                    case cgltf_attribute_type_joints: {
                        // Push into layout
                        neko_asset_mesh_layout_t layout = neko_default_val();
                        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT;
                        neko_dyn_array_push(layouts, layout);
                    } break;

                    case cgltf_attribute_type_weights: {
                        // Push into layout
                        neko_asset_mesh_layout_t layout = neko_default_val();
                        layout.type = NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT;
                        neko_dyn_array_push(layouts, layout);
                    } break;

                    // Shouldn't hit here...
                    default: {
                    } break;
                }
            }

            // Indices for primitive
            cgltf_accessor *acc = data->meshes[i].primitives[p].indices;

#define __GLTF_PUSH_IDX(BB, ACC, TYPE)                                                                                             \
    do {                                                                                                                           \
        s32 n = 0;                                                                                                                 \
        TYPE *buf = (TYPE *)acc->buffer_view->buffer->data + acc->buffer_view->offset / sizeof(TYPE) + acc->offset / sizeof(TYPE); \
        neko_assert(buf);                                                                                                          \
        TYPE v = 0;                                                                                                                \
        /* For each index */                                                                                                       \
        for (u32 k = 0; k < acc->count; k++) {                                                                                     \
            /* For each element */                                                                                                 \
            for (int l = 0; l < 1; l++) {                                                                                          \
                v = buf[n + l];                                                                                                    \
            }                                                                                                                      \
            n += (s32)(acc->stride / sizeof(TYPE));                                                                                \
            /* Add to temp positions array */                                                                                      \
            switch (index_element_size) {                                                                                          \
                case 0:                                                                                                            \
                    neko_byte_buffer_write(BB, u16, (u16)v);                                                                       \
                    break;                                                                                                         \
                case 2:                                                                                                            \
                    neko_byte_buffer_write(BB, u16, (u16)v);                                                                       \
                    break;                                                                                                         \
                case 4:                                                                                                            \
                    neko_byte_buffer_write(BB, u32, (u32)v);                                                                       \
                    break;                                                                                                         \
            }                                                                                                                      \
        }                                                                                                                          \
    } while (0)

            // If indices are available
            if (acc) {
                switch (acc->component_type) {
                    case cgltf_component_type_r_8:
                        __GLTF_PUSH_IDX(&i_data, acc, s8);
                        break;
                    case cgltf_component_type_r_8u:
                        __GLTF_PUSH_IDX(&i_data, acc, u8);
                        break;
                    case cgltf_component_type_r_16:
                        __GLTF_PUSH_IDX(&i_data, acc, s16);
                        break;
                    case cgltf_component_type_r_16u:
                        __GLTF_PUSH_IDX(&i_data, acc, u16);
                        break;
                    case cgltf_component_type_r_32u:
                        __GLTF_PUSH_IDX(&i_data, acc, u32);
                        break;
                    case cgltf_component_type_r_32f:
                        __GLTF_PUSH_IDX(&i_data, acc, f32);
                        break;

                    // Shouldn't hit here
                    default: {
                    } break;
                }
            } else {
                // Iterate over positions size, then just push back indices
                for (u32 i = 0; i < neko_dyn_array_size(positions); ++i) {
                    switch (index_element_size) {
                        default:
                        case 0:
                            neko_byte_buffer_write(&i_data, u16, (u16)i);
                            break;
                        case 2:
                            neko_byte_buffer_write(&i_data, u16, (u16)i);
                            break;
                        case 4:
                            neko_byte_buffer_write(&i_data, u32, (u32)i);
                            break;
                    }
                }
            }

            bool warnings[neko_enum_count(neko_asset_mesh_attribute_type)] = neko_default_val();

            // Grab mesh layout pointer to use
            neko_asset_mesh_layout_t *layoutp = decl ? decl->layout : layouts;
            u32 layout_ct = decl ? decl->layout_size / sizeof(neko_asset_mesh_layout_t) : neko_dyn_array_size(layouts);

            // Iterate layout to fill data buffers according to provided layout
            {
                u32 vct = 0;
                vct = neko_max(vct, neko_dyn_array_size(positions));
                vct = neko_max(vct, neko_dyn_array_size(colors));
                vct = neko_max(vct, neko_dyn_array_size(uvs));
                vct = neko_max(vct, neko_dyn_array_size(normals));
                vct = neko_max(vct, neko_dyn_array_size(tangents));

#define __NEKO_GLTF_WRITE_DATA(IT, VDATA, ARR, ARR_TYPE, ARR_DEF_VAL, LAYOUT_TYPE)              \
    do {                                                                                        \
        /* Grab data at index, if available */                                                  \
        if (IT < neko_dyn_array_size(ARR)) {                                                    \
            neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);                                \
        } else {                                                                                \
            /* Write default value and give warning.*/                                          \
            neko_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);                            \
            if (!warnings[LAYOUT_TYPE]) {                                                       \
                neko_println("Warning:Mesh:LoadFromFile:%s:Index out of range.", #LAYOUT_TYPE); \
                warnings[LAYOUT_TYPE] = true;                                                   \
            }                                                                                   \
        }                                                                                       \
    } while (0)

                for (u32 it = 0; it < vct; ++it) {
                    // For each attribute in layout
                    for (u32 l = 0; l < layout_ct; ++l) {
                        switch (layoutp[l].type) {
                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, positions, neko_vec3, neko_v3(0.f, 0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, uvs, neko_vec2, neko_v2(0.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, colors, neko_color_t, NEKO_COLOR_WHITE, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, normals, neko_vec3, neko_v3(0.f, 0.f, 1.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                            } break;

                            case NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                __NEKO_GLTF_WRITE_DATA(it, v_data, tangents, neko_vec3, neko_v3(0.f, 1.f, 0.f), NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                            } break;

                            default: {
                            } break;
                        }
                    }
                }
            }

            // Add to out data
            mesh->vertices[p] = neko_malloc(v_data.size);
            mesh->indices[p] = neko_malloc(i_data.size);
            mesh->vertex_sizes[p] = v_data.size;
            mesh->index_sizes[p] = i_data.size;

            // Copy data
            memcpy(mesh->vertices[p], v_data.data, v_data.size);
            memcpy(mesh->indices[p], i_data.data, i_data.size);
        }
    }

    // Free all data at the end
    cgltf_free(data);
    neko_dyn_array_free(positions);
    neko_dyn_array_free(normals);
    neko_dyn_array_free(tangents);
    neko_dyn_array_free(colors);
    neko_dyn_array_free(uvs);
    neko_dyn_array_free(layouts);
    neko_byte_buffer_free(&v_data);
    neko_byte_buffer_free(&i_data);
    return true;
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
        neko_util_load_gltf_data_from_file(path, decl, &meshes, &mesh_count);
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
            prim.vbo = neko_graphics_vertex_buffer_create(&vdesc);

            // Index buffer decl
            neko_graphics_index_buffer_desc_t idesc = neko_default_val();
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = neko_graphics_index_buffer_create(&idesc);

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
// NEKO_FNT
==========================*/

typedef struct {
    neko_fnt *fnt;
    void *input_data;
    neko_fnt_read_func_t input_read_func;
} neko_fnt_decoder_t;

static void neko_fnt_set_error(neko_fnt *fnt, const char *message) {
    if (fnt) {
        fnt->num_glyphs = 0;
        fnt->error_message = message;
    }
}

static bool neko_fnt_read_func(neko_fnt_decoder_t *decoder, u8 *data, size_t length) {
    if (decoder->input_read_func(decoder->input_data, data, length) == length) {
        return true;
    } else {
        neko_fnt_set_error(decoder->fnt, "Read error: error calling input function.");
        return false;
    }
}

static void neko_fnt_decode(neko_fnt *fnt, void *input_data, neko_fnt_read_func_t input_read_func);

static size_t neko_file_read_func_default(void *user_data, u8 *buffer, size_t length) { return fread(buffer, 1, length, (FILE *)user_data); }

neko_fnt *neko_fnt_read(FILE *file) {
    neko_fnt *fnt = calloc(1, sizeof(neko_fnt));
    if (file) {
        neko_fnt_decode(fnt, file, neko_file_read_func_default);
    } else {
        neko_fnt_set_error(fnt, "File not found");
    }
    return fnt;
}

neko_fnt *neko_fnt_read_from_callbacks(void *user_data, neko_fnt_read_func_t input_read_func) {
    neko_fnt *fnt = calloc(1, sizeof(neko_fnt));
    if (input_read_func) {
        neko_fnt_decode(fnt, user_data, input_read_func);
    } else {
        neko_fnt_set_error(fnt, "Invalid argument: read_func is NULL");
    }
    return fnt;
}

void neko_fnt_free(neko_fnt *fnt) {
    if (fnt) {
        free(fnt->name);
        if (fnt->page_names) {
            // 内存只分配给第一项
            // 剩余的项是第一个项中的指针 因此不应释放它们
            free(fnt->page_names[0]);
            free(fnt->page_names);
        }
        free(fnt->glyphs);
        free(fnt->kerning_pairs);
        free(fnt);
    }
}

// Decoding

typedef enum {
    NEKO_FNT_BLOCK_TYPE_INFO = 1,
    NEKO_FNT_BLOCK_TYPE_COMMON = 2,
    NEKO_FNT_BLOCK_TYPE_PAGES = 3,
    NEKO_FNT_BLOCK_TYPE_CHARS = 4,
    NEKO_FNT_BLOCK_TYPE_KERNING = 5,
} ok_fnt_block_type;

static void neko_fnt_decode2(neko_fnt_decoder_t *decoder) {
    neko_fnt *fnt = decoder->fnt;

    u8 header[4];
    if (!neko_fnt_read_func(decoder, header, sizeof(header))) {
        return;
    }
    if (memcmp("BMF", header, 3) != 0) {
        neko_fnt_set_error(fnt, "Not an AngelCode binary FNT file.");
        return;
    }
    if (header[3] != 3) {
        neko_fnt_set_error(fnt,
                           "Unsupported version of AngelCode binary FNT file "
                           "(only version 3 supported).");
        return;
    }

    u32 block_types_found = 0;
    while (true) {
        u8 block_header[5];
        if (decoder->input_read_func(decoder->input_data, block_header, sizeof(block_header)) != sizeof(block_header)) {
            // 可能是EOF 如果未找到所有必需的块则给出错误
            const int required_blocks = ((1 << NEKO_FNT_BLOCK_TYPE_COMMON) | (1 << NEKO_FNT_BLOCK_TYPE_PAGES) | (1 << NEKO_FNT_BLOCK_TYPE_CHARS));
            if ((block_types_found & required_blocks) != required_blocks) {
                neko_fnt_set_error(decoder->fnt, "Missing required blocks or unexpected EOF");
            }
            return;
        }

        ok_fnt_block_type block_type = block_header[0];
        u32 block_length = neko_read_LE32(block_header + 1);

        block_types_found |= (1 << block_type);
        switch (block_type) {
            case NEKO_FNT_BLOCK_TYPE_INFO: {
                u8 info_header[14];
                if (block_length <= sizeof(info_header)) {
                    neko_fnt_set_error(fnt, "Invalid info block");
                    return;
                }
                if (!neko_fnt_read_func(decoder, info_header, sizeof(info_header))) {
                    return;
                }
                // 获取fnt大小忽略其余
                fnt->size = neko_read_LE16(info_header);

                // 获取fnt名字
                const size_t name_buffer_length = block_length - sizeof(info_header);
                fnt->name = malloc(name_buffer_length);
                if (!fnt->name) {
                    neko_fnt_set_error(fnt, "Couldn't allocate font name");
                    return;
                }
                if (!neko_fnt_read_func(decoder, (u8 *)fnt->name, name_buffer_length)) {
                    return;
                }
                // 健全性检查 确保字符串具有空终止符
                fnt->name[name_buffer_length - 1] = 0;
                break;
            }

            case NEKO_FNT_BLOCK_TYPE_COMMON: {
                u8 common[15];
                if (block_length != sizeof(common)) {
                    neko_fnt_set_error(fnt, "Invalid common block");
                    return;
                }
                if (!neko_fnt_read_func(decoder, common, sizeof(common))) {
                    return;
                }
                // 获取height base和 page count 其余忽略
                fnt->line_height = neko_read_LE16(common);
                fnt->base = neko_read_LE16(common + 2);
                fnt->num_pages = neko_read_LE16(common + 8);
                break;
            }

            case NEKO_FNT_BLOCK_TYPE_PAGES: {
                if (fnt->num_pages <= 0 || block_length == 0) {
                    neko_fnt_set_error(fnt, "Couldn't get page names");
                    return;
                } else {
                    fnt->page_names = calloc(fnt->num_pages, sizeof(char *));
                    if (!fnt->page_names) {
                        fnt->num_pages = 0;
                        neko_fnt_set_error(fnt, "Couldn't allocate memory for page name array");
                        return;
                    }
                    // 将所有内容加载到第一个项目中
                    fnt->page_names[0] = malloc(block_length);
                    if (!fnt->page_names[0]) {
                        fnt->num_pages = 0;
                        neko_fnt_set_error(fnt, "Couldn't allocate memory for page names");
                        return;
                    }
                    if (!neko_fnt_read_func(decoder, (u8 *)fnt->page_names[0], block_length)) {
                        return;
                    }
                    char *pos = fnt->page_names[0];
                    char *const end_pos = pos + block_length;
                    // 健全性检查 确保字符串具有空终止符
                    *(end_pos - 1) = 0;

                    // 为每个页面名称设置指针
                    size_t next_index = 1;
                    while (pos + 1 < end_pos && next_index < fnt->num_pages) {
                        if (*pos == 0) {
                            fnt->page_names[next_index] = pos + 1;
                            next_index++;
                        }
                        pos++;
                    }
                    // 健全性检查 确保剩余的页面名称 如果有则指向某个地方
                    for (size_t i = next_index; i < fnt->num_pages; i++) {
                        fnt->page_names[i] = end_pos - 1;
                    }
                }
                break;
            }

            case NEKO_FNT_BLOCK_TYPE_CHARS: {
                u8 data[20];
                fnt->num_glyphs = block_length / sizeof(data);
                fnt->glyphs = malloc(fnt->num_glyphs * sizeof(neko_fnt_glyph));
                if (!fnt->glyphs) {
                    fnt->num_glyphs = 0;
                    neko_fnt_set_error(fnt, "Couldn't allocate memory for glyphs");
                    return;
                }
                // 在小端系统上 我们可以将整个块加载到内存中
                // 但是我们假设这里字节顺序未知
                for (size_t i = 0; i < fnt->num_glyphs; i++) {
                    if (!neko_fnt_read_func(decoder, data, sizeof(data))) {
                        return;
                    }
                    neko_fnt_glyph *glyph = &fnt->glyphs[i];
                    glyph->ch = neko_read_LE32(data);
                    glyph->x = neko_read_LE16(data + 4);
                    glyph->y = neko_read_LE16(data + 6);
                    glyph->width = neko_read_LE16(data + 8);
                    glyph->height = neko_read_LE16(data + 10);
                    glyph->offset_x = (s16)neko_read_LE16(data + 12);
                    glyph->offset_y = (s16)neko_read_LE16(data + 14);
                    glyph->advance_x = (s16)neko_read_LE16(data + 16);
                    glyph->page = data[18];
                    glyph->channel = data[19];
                }
                break;
            }

            case NEKO_FNT_BLOCK_TYPE_KERNING: {
                u8 data[10];
                fnt->num_kerning_pairs = block_length / sizeof(data);
                fnt->kerning_pairs = malloc(fnt->num_kerning_pairs * sizeof(neko_fnt_kerning));
                if (!fnt->kerning_pairs) {
                    fnt->num_kerning_pairs = 0;
                    neko_fnt_set_error(fnt, "Couldn't allocate memory for kerning");
                    return;
                }
                // 在小端系统上 我们可以将整个块加载到内存中
                // 但是我们假设这里字节顺序未知
                for (size_t i = 0; i < fnt->num_kerning_pairs; i++) {
                    if (!neko_fnt_read_func(decoder, data, sizeof(data))) {
                        return;
                    }
                    neko_fnt_kerning *kerning = &fnt->kerning_pairs[i];
                    kerning->first_char = neko_read_LE32(data);
                    kerning->second_char = neko_read_LE32(data + 4);
                    kerning->amount = (s16)neko_read_LE16(data + 8);
                }
                break;
            }

            default:
                neko_fnt_set_error(fnt, "Unknown block type");
                return;
        }
    }
}

static void neko_fnt_decode(neko_fnt *fnt, void *input_data, neko_fnt_read_func_t input_read_func) {
    if (fnt) {
        neko_fnt_decoder_t *decoder = calloc(1, sizeof(neko_fnt_decoder_t));
        if (!decoder) {
            neko_fnt_set_error(fnt, "Couldn't allocate decoder.");
            return;
        }
        decoder->fnt = fnt;
        decoder->input_data = input_data;
        decoder->input_read_func = input_read_func;

        neko_fnt_decode2(decoder);

        free(decoder);
    }
}

/*==========================
// NEKO_PACK
==========================*/

#pragma region packer

// #include "libs/lz4/lz4.h"

#if __linux__ || __APPLE__
#define openFile(filePath, mode) \
	fopen(filePath, mode)
#define seekFile(file, offset, whence) \
	fseeko(file, offset, whence)
#define tellFile(file) ftello(file)
#elif _WIN32
inline static FILE* openFile(const char* filePath, const char* mode)
{
    FILE* file;

    errno_t error = fopen_s(
            &file,
            filePath,
            mode);

    if (error != 0)
        return NULL;

    return file;
}

#define seekFile(file, offset, whence) \
	_fseeki64(file, offset, whence)
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
    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        neko_pack_destroy(pack_reader);
        return -1; /*BAD_FILE_TYPE_PACK_RESULT*/
    }

    if (header[4] != 0 || header[5] != 0) {
        neko_pack_destroy(pack_reader);
        return -1; /*BAD_FILE_VERSION_PACK_RESULT*/
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

    neko_log_trace("load pack %s buildnum: %d", file_path, buildnum);

    //*pack_reader = pack;
    return 0;
}
void neko_pack_destroy(neko_packreader_t *pack_reader) {
    if (!pack_reader) return;

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

    pack_item *searchItem = &pack_reader->search_item;

    searchItem->info.path_size = (u8)strlen(path);
    searchItem->path = (char *)path;

    pack_item *item = (pack_item *)bsearch(searchItem, pack_reader->items, pack_reader->item_count, sizeof(pack_item), neko_compare_pack_items);

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

    s64 fileOffset = (s64)(info.file_offset + sizeof(pack_iteminfo) + info.path_size);

    int seekResult = seekFile(file, fileOffset, SEEK_SET);

    if (seekResult != 0) return -1; /*FAILED_TO_SEEK_FILE_PACK_RESULT*/

    if (info.zip_size > 0) {
        size_t result = fread(zip_buffer, sizeof(u8), info.zip_size, file);

        if (result != info.zip_size) return -1; /*FAILED_TO_READ_FILE_PACK_RESULT*/

        // result = LZ4_decompress_safe((char *)zipBuffer, (char *)data_buffer, info.zip_size, info.data_size);

        result = neko_lz_decode(zip_buffer, info.zip_size, data_buffer, info.data_size);

        // result = info.data_size;
        // data_buffer = zip_buffer;

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

void neko_pack_free_buffers(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    neko_safe_free(pack_reader->data_buffer);
    neko_safe_free(pack_reader->zip_buffer);
    pack_reader->data_buffer = NULL;
    pack_reader->zip_buffer = NULL;
}

neko_private(void) neko_pack_remove_item(u64 item_count, pack_item *packItems) {
    neko_assert(item_count == 0 || (item_count > 0 && packItems));

    for (u64 i = 0; i < item_count; i++) remove(packItems[i].path);
}

neko_pack_result neko_pack_unzip(const_str file_path, b8 printProgress) {
    neko_assert(file_path);

    neko_packreader_t pack_reader;

    neko_pack_result pack_result = neko_pack_read(file_path, 128, false, &pack_reader);

    if (pack_result != 0) return pack_result;

    u64 totalRawSize = 0, totalZipSize = 0;

    u64 item_count = pack_reader.item_count;
    pack_item *items = pack_reader.items;

    for (u64 i = 0; i < item_count; i++) {
        pack_item *item = &items[i];

        if (printProgress) {
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

        for (u8 j = 0; j < path_size; j++) {
            if (item_path[j] == '/' || item_path[j] == '\\' || (item_path[j] == '.' && j == 0)) item_path[j] = '-';
        }

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

        if (printProgress) {
            u32 rawFileSize = item->info.data_size;
            u32 zipFileSize = item->info.zip_size > 0 ? item->info.zip_size : item->info.data_size;

            totalRawSize += rawFileSize;
            totalZipSize += zipFileSize;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", rawFileSize, zipFileSize, progress);
        }
    }

    neko_pack_destroy(&pack_reader);

    if (printProgress) {
        printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)totalRawSize, (long long unsigned int)totalZipSize);
    }

    return 0;
}

neko_private(neko_pack_result) neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, b8 printProgress) {
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

    u64 totalZipSize = 0, totalRawSize = 0;

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (printProgress) {
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

        int seekResult = seekFile(item_file, 0, SEEK_END);

        if (seekResult != 0) {
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

        seekResult = seekFile(item_file, 0, SEEK_SET);

        if (seekResult != 0) {
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

            // const int max_dst_size = LZ4_compressBound(itemSize);
            // zipSize = LZ4_compress_fast((char *)itemData, (char *)zipData, itemSize, max_dst_size, 10);

            const int max_dst_size = neko_lz_bounds(item_size, 0);
            zip_size = neko_lz_encode(item_data, item_size, zip_data, max_dst_size, 9);

            // memcpy(zip_data, item_data, item_size);
            // zip_size = item_size;

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

        if (printProgress) {
            u32 zipFileSize = zip_size > 0 ? (u32)zip_size : (u32)item_size;
            u32 rawFileSize = (u32)item_size;

            totalZipSize += zipFileSize;
            totalRawSize += rawFileSize;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", zipFileSize, rawFileSize, progress);
            fflush(stdout);
        }
    }

    neko_safe_free(zip_data);
    neko_safe_free(item_data);

    if (printProgress) {
        int compression = (int)((1.0 - (f64)(totalZipSize) / (f64)totalRawSize) * 100.0);
        printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)totalZipSize, (long long unsigned int)totalRawSize, compression);
    }

    return 0;
}

neko_private(int) neko_pack_compare_item_paths(const void *_a, const void *_b) {
    // NOTE: a and b should not be NULL!
    // Skipping here neko_assertions for debug build speed.

    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);

    int difference = al - bl;

    if (difference != 0) return difference;

    return memcmp(a, b, al * sizeof(u8));
}

neko_pack_result neko_pack_files(const_str file_path, u64 fileCount, const_str *filePaths, b8 printProgress) {
    neko_assert(file_path);
    neko_assert(fileCount > 0);
    neko_assert(filePaths);

    char **item_paths = (char **)neko_safe_malloc(fileCount * sizeof(char *));

    if (!item_paths) return -1; /*FAILED_TO_ALLOCATE_PACK_RESULT*/

    u64 item_count = 0;

    for (u64 i = 0; i < fileCount; i++) {
        b8 alreadyAdded = false;

        for (u64 j = 0; j < item_count; j++) {
            if (i != j && strcmp(filePaths[i], item_paths[j]) == 0) alreadyAdded = true;
        }

        if (!alreadyAdded) item_paths[item_count++] = (char *)filePaths[i];
    }

    qsort(item_paths, item_count, sizeof(char *), neko_pack_compare_item_paths);

    FILE *packFile = openFile(file_path, "wb");

    if (!packFile) {
        neko_safe_free(item_paths);
        return -1; /*FAILED_TO_CREATE_FILE_PACK_RESULT*/
    }

    char header[neko_pack_head_size] = {
            'P', 'A', 'C', 'K', 0, 0, 0, !neko_little_endian,
    };

    s32 buildnum = neko_buildnum();

    size_t writeResult = fwrite(header, sizeof(u8), neko_pack_head_size, packFile);
    writeResult += fwrite(&buildnum, sizeof(s32), 1, packFile);

    if (writeResult != neko_pack_head_size + 1) {
        neko_safe_free(item_paths);
        closeFile(packFile);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    writeResult = fwrite(&item_count, sizeof(u64), 1, packFile);

    if (writeResult != 1) {
        neko_safe_free(item_paths);
        closeFile(packFile);
        remove(file_path);
        return -1; /*FAILED_TO_WRITE_FILE_PACK_RESULT*/
    }

    neko_pack_result pack_result = neko_write_pack_items(packFile, item_count, item_paths, printProgress);

    neko_safe_free(item_paths);
    closeFile(packFile);

    if (pack_result != 0) {
        remove(file_path);
        return pack_result;
    }

    return 0;
}

neko_pack_result neko_pack_info(const_str file_path, u8 *majorVersion, u8 *minorVersion, u8 *patchVersion, b8 *isLittleEndian, u64 *_item_count) {
    neko_assert(file_path);
    neko_assert(majorVersion);
    neko_assert(minorVersion);
    neko_assert(patchVersion);
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

    *majorVersion = header[4];
    *minorVersion = header[5];
    *patchVersion = header[6];
    *isLittleEndian = !header[7];
    *_item_count = item_count;
    return 0;
}

#pragma endregion

#pragma region xml

#define __neko_xml_expect(c_, e_)                 \
    if (*c_ != e_) {                              \
        neko_xml_emit_error("Expected " #e_ "."); \
        return NULL;                              \
    }

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

#pragma region nbt

typedef struct {
    u8 *buffer;
    size_t buffer_offset;
} __neko_nbt_read_stream_t;

static u8 __neko_nbt_get_byte(__neko_nbt_read_stream_t *stream) { return stream->buffer[stream->buffer_offset++]; }

static s16 __neko_nbt_get_int16(__neko_nbt_read_stream_t *stream) {
    u8 bytes[2];
    for (int i = 1; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s16 *)(bytes);
}

static s32 __neko_nbt_get_int32(__neko_nbt_read_stream_t *stream) {
    u8 bytes[4];
    for (int i = 3; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s32 *)(bytes);
}

static s64 __neko_nbt_get_int64(__neko_nbt_read_stream_t *stream) {
    u8 bytes[8];
    for (int i = 7; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(s64 *)(bytes);
}

static float __neko_nbt_get_float(__neko_nbt_read_stream_t *stream) {
    u8 bytes[4];
    for (int i = 3; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(float *)(bytes);
}

static double __neko_nbt_get_double(__neko_nbt_read_stream_t *stream) {
    u8 bytes[8];
    for (int i = 7; i >= 0; i--) {
        bytes[i] = __neko_nbt_get_byte(stream);
    }
    return *(double *)(bytes);
}

static neko_nbt_tag_t *__neko_nbt_parse(__neko_nbt_read_stream_t *stream, int parse_name, neko_nbt_tag_type_t override_type) {

    neko_nbt_tag_t *tag = (neko_nbt_tag_t *)neko_malloc(sizeof(neko_nbt_tag_t));

    if (override_type == NBT_NO_OVERRIDE) {
        tag->type = (neko_nbt_tag_type_t)__neko_nbt_get_byte(stream);
    } else {
        tag->type = override_type;
    }

    if (parse_name && tag->type != NBT_TYPE_END) {
        tag->name_size = __neko_nbt_get_int16(stream);
        tag->name = (char *)neko_malloc(tag->name_size + 1);
        for (size_t i = 0; i < tag->name_size; i++) {
            tag->name[i] = __neko_nbt_get_byte(stream);
        }
        tag->name[tag->name_size] = '\0';
    } else {
        tag->name = NULL;
        tag->name_size = 0;
    }

    switch (tag->type) {
        case NBT_TYPE_END: {
            break;
        }
        case NBT_TYPE_BYTE: {
            tag->tag_byte.value = __neko_nbt_get_byte(stream);
            break;
        }
        case NBT_TYPE_SHORT: {
            tag->tag_short.value = __neko_nbt_get_int16(stream);
            break;
        }
        case NBT_TYPE_INT: {
            tag->tag_int.value = __neko_nbt_get_int32(stream);
            break;
        }
        case NBT_TYPE_LONG: {
            tag->tag_long.value = __neko_nbt_get_int64(stream);
            break;
        }
        case NBT_TYPE_FLOAT: {
            tag->tag_float.value = __neko_nbt_get_float(stream);
            break;
        }
        case NBT_TYPE_DOUBLE: {
            tag->tag_double.value = __neko_nbt_get_double(stream);
            break;
        }
        case NBT_TYPE_BYTE_ARRAY: {
            tag->tag_byte_array.size = __neko_nbt_get_int32(stream);
            tag->tag_byte_array.value = (s8 *)neko_malloc(tag->tag_byte_array.size);
            for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
                tag->tag_byte_array.value[i] = __neko_nbt_get_byte(stream);
            }
            break;
        }
        case NBT_TYPE_STRING: {
            tag->tag_string.size = __neko_nbt_get_int16(stream);
            tag->tag_string.value = (char *)neko_malloc(tag->tag_string.size + 1);
            for (size_t i = 0; i < tag->tag_string.size; i++) {
                tag->tag_string.value[i] = __neko_nbt_get_byte(stream);
            }
            tag->tag_string.value[tag->tag_string.size] = '\0';
            break;
        }
        case NBT_TYPE_LIST: {
            tag->tag_list.type = (neko_nbt_tag_type_t)__neko_nbt_get_byte(stream);
            tag->tag_list.size = __neko_nbt_get_int32(stream);
            tag->tag_list.value = (neko_nbt_tag_t **)neko_malloc(tag->tag_list.size * sizeof(neko_nbt_tag_t *));
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                tag->tag_list.value[i] = __neko_nbt_parse(stream, 0, tag->tag_list.type);
            }
            break;
        }
        case NBT_TYPE_COMPOUND: {
            tag->tag_compound.size = 0;
            tag->tag_compound.value = NULL;
            for (;;) {
                neko_nbt_tag_t *inner_tag = __neko_nbt_parse(stream, 1, NBT_NO_OVERRIDE);

                if (inner_tag->type == NBT_TYPE_END) {
                    neko_nbt_free_tag(inner_tag);
                    break;
                } else {
                    tag->tag_compound.value = (neko_nbt_tag_t **)neko_realloc(tag->tag_compound.value, (tag->tag_compound.size + 1) * sizeof(neko_nbt_tag_t *));
                    tag->tag_compound.value[tag->tag_compound.size] = inner_tag;
                    tag->tag_compound.size++;
                }
            }
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            tag->tag_int_array.size = __neko_nbt_get_int32(stream);
            tag->tag_int_array.value = (s32 *)neko_malloc(tag->tag_int_array.size * sizeof(s32));
            for (size_t i = 0; i < tag->tag_int_array.size; i++) {
                tag->tag_int_array.value[i] = __neko_nbt_get_int32(stream);
            }
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            tag->tag_long_array.size = __neko_nbt_get_int32(stream);
            tag->tag_long_array.value = (s64 *)neko_malloc(tag->tag_long_array.size * sizeof(s64));
            for (size_t i = 0; i < tag->tag_long_array.size; i++) {
                tag->tag_long_array.value[i] = __neko_nbt_get_int64(stream);
            }
            break;
        }
        default: {
            neko_free(tag);
            return NULL;
        }
    }

    return tag;
}

neko_nbt_tag_t *neko_nbt_parse(neko_nbt_reader_t reader, int parse_flags) {

    u8 *buffer = NULL;
    size_t buffer_size = 0;

    __neko_nbt_read_stream_t stream;

    u8 in_buffer[NEKO_NBT_BUFFER_SIZE];
    size_t bytes_read;
    do {
        bytes_read = reader.read(reader.userdata, in_buffer, NEKO_NBT_BUFFER_SIZE);
        buffer = (u8 *)neko_realloc(buffer, buffer_size + bytes_read);
        memcpy(buffer + buffer_size, in_buffer, bytes_read);
        buffer_size += bytes_read;
    } while (bytes_read == NEKO_NBT_BUFFER_SIZE);

    stream.buffer = buffer;
    stream.buffer_offset = 0;

    neko_nbt_tag_t *tag = __neko_nbt_parse(&stream, 1, NBT_NO_OVERRIDE);

    neko_free(buffer);

    return tag;
}

typedef struct {
    u8 *buffer;
    size_t offset;
    size_t size;
    size_t alloc_size;
} __neko_nbt_write_stream_t;

void __neko_nbt_put_byte(__neko_nbt_write_stream_t *stream, u8 value) {
    if (stream->offset >= stream->alloc_size - 1) {
        stream->buffer = (u8 *)neko_realloc(stream->buffer, stream->alloc_size * 2);
        stream->alloc_size *= 2;
    }

    stream->buffer[stream->offset++] = value;
    stream->size++;
}

void __neko_nbt_put_int16(__neko_nbt_write_stream_t *stream, s16 value) {
    u8 *value_array = (u8 *)&value;
    for (int i = 1; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_int32(__neko_nbt_write_stream_t *stream, s32 value) {
    u8 *value_array = (u8 *)&value;
    for (int i = 3; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_int64(__neko_nbt_write_stream_t *stream, s64 value) {
    u8 *value_array = (u8 *)&value;
    for (int i = 7; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_float(__neko_nbt_write_stream_t *stream, float value) {
    u8 *value_array = (u8 *)&value;
    for (int i = 3; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_put_double(__neko_nbt_write_stream_t *stream, double value) {
    u8 *value_array = (u8 *)&value;
    for (int i = 7; i >= 0; i--) {
        __neko_nbt_put_byte(stream, value_array[i]);
    }
}

void __neko_nbt_write_tag(__neko_nbt_write_stream_t *stream, neko_nbt_tag_t *tag, int write_name, int write_type) {

    if (write_type) {
        __neko_nbt_put_byte(stream, tag->type);
    }

    if (write_name && tag->type != NBT_TYPE_END) {
        __neko_nbt_put_int16(stream, tag->name_size);
        for (size_t i = 0; i < tag->name_size; i++) {
            __neko_nbt_put_byte(stream, tag->name[i]);
        }
    }

    switch (tag->type) {
        case NBT_TYPE_END: {
            break;
        }
        case NBT_TYPE_BYTE: {
            __neko_nbt_put_byte(stream, tag->tag_byte.value);
            break;
        }
        case NBT_TYPE_SHORT: {
            __neko_nbt_put_int16(stream, tag->tag_short.value);
            break;
        }
        case NBT_TYPE_INT: {
            __neko_nbt_put_int32(stream, tag->tag_int.value);
            break;
        }
        case NBT_TYPE_LONG: {
            __neko_nbt_put_int64(stream, tag->tag_long.value);
            break;
        }
        case NBT_TYPE_FLOAT: {
            __neko_nbt_put_float(stream, tag->tag_float.value);
            break;
        }
        case NBT_TYPE_DOUBLE: {
            __neko_nbt_put_double(stream, tag->tag_double.value);
            break;
        }
        case NBT_TYPE_BYTE_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_byte_array.size);
            for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
                __neko_nbt_put_byte(stream, tag->tag_byte_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_STRING: {
            __neko_nbt_put_int16(stream, tag->tag_string.size);
            for (size_t i = 0; i < tag->tag_string.size; i++) {
                __neko_nbt_put_byte(stream, tag->tag_string.value[i]);
            }
            break;
        }
        case NBT_TYPE_LIST: {
            __neko_nbt_put_byte(stream, tag->tag_list.type);
            __neko_nbt_put_int32(stream, tag->tag_list.size);
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                __neko_nbt_write_tag(stream, tag->tag_list.value[i], 0, 0);
            }
            break;
        }
        case NBT_TYPE_COMPOUND: {
            for (size_t i = 0; i < tag->tag_compound.size; i++) {
                __neko_nbt_write_tag(stream, tag->tag_compound.value[i], 1, 1);
            }
            __neko_nbt_put_byte(stream, 0);  // 结束标识
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_int_array.size);
            for (size_t i = 0; i < tag->tag_int_array.size; i++) {
                __neko_nbt_put_int32(stream, tag->tag_int_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            __neko_nbt_put_int32(stream, tag->tag_long_array.size);
            for (size_t i = 0; i < tag->tag_long_array.size; i++) {
                __neko_nbt_put_int64(stream, tag->tag_long_array.value[i]);
            }
            break;
        }
        default: {
            break;
        }
    }
}

u32 __neko_nbt_crc_table[256];

int __neko_nbt_crc_table_computed = 0;

void __neko_nbt_make_crc_table(void) {
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (u32)n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        __neko_nbt_crc_table[n] = c;
    }
    __neko_nbt_crc_table_computed = 1;
}

static u32 __neko_nbt_update_crc(u32 crc, u8 *buf, size_t len) {
    u32 c = crc ^ 0xffffffffL;
    size_t n;

    if (!__neko_nbt_crc_table_computed) {
        __neko_nbt_make_crc_table();
    }

    for (n = 0; n < len; n++) {
        c = __neko_nbt_crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c ^ 0xffffffffL;
}

void neko_nbt_write(neko_nbt_writer_t writer, neko_nbt_tag_t *tag, int write_flags) {

    __neko_nbt_write_stream_t write_stream;
    write_stream.buffer = (u8 *)neko_malloc(NEKO_NBT_BUFFER_SIZE);
    write_stream.offset = 0;
    write_stream.size = 0;
    write_stream.alloc_size = NEKO_NBT_BUFFER_SIZE;

    __neko_nbt_write_tag(&write_stream, tag, 1, 1);

    size_t bytes_left = write_stream.size;
    size_t offset = 0;
    while (bytes_left > 0) {
        size_t bytes_written = writer.write(writer.userdata, write_stream.buffer + offset, bytes_left);
        offset += bytes_written;
        bytes_left -= bytes_written;
    }

    neko_free(write_stream.buffer);
}

static neko_nbt_tag_t *__neko_nbt_new_tag_base(void) {
    neko_nbt_tag_t *tag = (neko_nbt_tag_t *)neko_malloc(sizeof(neko_nbt_tag_t));
    tag->name = NULL;
    tag->name_size = 0;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_byte(s8 value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_BYTE;
    tag->tag_byte.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_short(s16 value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_SHORT;
    tag->tag_short.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_int(s32 value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_INT;
    tag->tag_int.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_long(s64 value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LONG;
    tag->tag_long.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_float(float value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_FLOAT;
    tag->tag_float.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_double(double value) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_DOUBLE;
    tag->tag_double.value = value;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_byte_array(s8 *value, size_t size) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_BYTE_ARRAY;
    tag->tag_byte_array.size = size;
    tag->tag_byte_array.value = (s8 *)neko_malloc(size);

    memcpy(tag->tag_byte_array.value, value, size);

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_string(const char *value, size_t size) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_STRING;
    tag->tag_string.size = size;
    tag->tag_string.value = (char *)neko_malloc(size + 1);

    memcpy(tag->tag_string.value, value, size);
    tag->tag_string.value[tag->tag_string.size] = '\0';

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_list(neko_nbt_tag_type_t type) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LIST;
    tag->tag_list.type = type;
    tag->tag_list.size = 0;
    tag->tag_list.value = NULL;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_compound(void) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_COMPOUND;
    tag->tag_compound.size = 0;
    tag->tag_compound.value = NULL;

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_int_array(s32 *value, size_t size) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_INT_ARRAY;
    tag->tag_int_array.size = size;
    tag->tag_int_array.value = (s32 *)neko_malloc(size * sizeof(s32));

    memcpy(tag->tag_int_array.value, value, size * sizeof(s32));

    return tag;
}

neko_nbt_tag_t *neko_nbt_new_tag_long_array(s64 *value, size_t size) {
    neko_nbt_tag_t *tag = __neko_nbt_new_tag_base();

    tag->type = NBT_TYPE_LONG_ARRAY;
    tag->tag_long_array.size = size;
    tag->tag_long_array.value = (s64 *)neko_malloc(size * sizeof(s64));

    memcpy(tag->tag_long_array.value, value, size * sizeof(s64));

    return tag;
}

void neko_nbt_set_tag_name(neko_nbt_tag_t *tag, const char *name, size_t size) {
    if (tag->name) {
        neko_free(tag->name);
    }
    tag->name_size = size;
    tag->name = (char *)neko_malloc(size + 1);
    memcpy(tag->name, name, size);
    tag->name[tag->name_size] = '\0';
}

void neko_nbt_tag_list_append(neko_nbt_tag_t *list, neko_nbt_tag_t *value) {
    list->tag_list.value = (neko_nbt_tag_t **)neko_realloc(list->tag_list.value, (list->tag_list.size + 1) * sizeof(neko_nbt_tag_t *));
    list->tag_list.value[list->tag_list.size] = value;
    list->tag_list.size++;
}

neko_nbt_tag_t *neko_nbt_tag_list_get(neko_nbt_tag_t *tag, size_t index) { return tag->tag_list.value[index]; }

void neko_nbt_tag_compound_append(neko_nbt_tag_t *compound, neko_nbt_tag_t *value) {
    compound->tag_compound.value = (neko_nbt_tag_t **)neko_realloc(compound->tag_compound.value, (compound->tag_compound.size + 1) * sizeof(neko_nbt_tag_t *));
    compound->tag_compound.value[compound->tag_compound.size] = value;
    compound->tag_compound.size++;
}

neko_nbt_tag_t *neko_nbt_tag_compound_get(neko_nbt_tag_t *tag, const char *key) {
    for (size_t i = 0; i < tag->tag_compound.size; i++) {
        neko_nbt_tag_t *compare_tag = tag->tag_compound.value[i];

        if (memcmp(compare_tag->name, key, compare_tag->name_size) == 0) {
            return compare_tag;
        }
    }

    return NULL;
}

void neko_nbt_free_tag(neko_nbt_tag_t *tag) {
    switch (tag->type) {
        case NBT_TYPE_BYTE_ARRAY: {
            neko_free(tag->tag_byte_array.value);
            break;
        }
        case NBT_TYPE_STRING: {
            neko_free(tag->tag_string.value);
            break;
        }
        case NBT_TYPE_LIST: {
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                neko_nbt_free_tag(tag->tag_list.value[i]);
            }
            neko_free(tag->tag_list.value);
            break;
        }
        case NBT_TYPE_COMPOUND: {
            for (size_t i = 0; i < tag->tag_compound.size; i++) {
                neko_nbt_free_tag(tag->tag_compound.value[i]);
            }
            neko_free(tag->tag_compound.value);
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            neko_free(tag->tag_int_array.value);
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            neko_free(tag->tag_long_array.value);
            break;
        }
        default: {
            break;
        }
    }

    if (tag->name) {
        neko_free(tag->name);
    }

    neko_free(tag);
}

static size_t reader_read(void *userdata, uint8_t *data, size_t size) { return fread(data, 1, size, (FILE *)userdata); }

static size_t writer_write(void *userdata, uint8_t *data, size_t size) { return fwrite(data, 1, size, (FILE *)userdata); }

neko_nbt_tag_t *neko_read_nbt_file_default(const char *name, int flags) {

    FILE *file = fopen(name, "rb");

    neko_nbt_reader_t reader;

    reader.read = reader_read;
    reader.userdata = file;

    neko_nbt_tag_t *tag = neko_nbt_parse(reader, flags);

    fclose(file);

    return tag;
}

void neko_write_nbt_file_default(const char *name, neko_nbt_tag_t *tag, int flags) {

    FILE *file = fopen(name, "wb");

    neko_nbt_writer_t writer;

    writer.write = writer_write;
    writer.userdata = file;

    neko_nbt_write(writer, tag, flags);

    fclose(file);
}

void neko_nbt_print_tree(neko_nbt_tag_t *tag, int indentation) {
    for (int i = 0; i < indentation; i++) {
        printf(" ");
    }

    if (tag->name) {
        printf("%s: ", tag->name);
    }

    switch (tag->type) {
        case NBT_TYPE_END: {
            printf("[end]");
            break;
        }
        case NBT_TYPE_BYTE: {
            printf("%hhd", tag->tag_byte.value);
            break;
        }
        case NBT_TYPE_SHORT: {
            printf("%hd", tag->tag_short.value);
            break;
        }
        case NBT_TYPE_INT: {
            printf("%d", tag->tag_int.value);
            break;
        }
        case NBT_TYPE_LONG: {
            printf("%lld", tag->tag_long.value);
            break;
        }
        case NBT_TYPE_FLOAT: {
            printf("%f", tag->tag_float.value);
            break;
        }
        case NBT_TYPE_DOUBLE: {
            printf("%f", tag->tag_double.value);
            break;
        }
        case NBT_TYPE_BYTE_ARRAY: {
            printf("[byte array]");
            break;
            for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
                printf("%hhd ", tag->tag_byte_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_STRING: {
            printf("%s", tag->tag_string.value);
            break;
        }
        case NBT_TYPE_LIST: {
            printf("\n");
            for (size_t i = 0; i < tag->tag_list.size; i++) {
                neko_nbt_print_tree(tag->tag_list.value[i], indentation /*+ tag->name_size*/ + 2);
            }
            break;
        }
        case NBT_TYPE_COMPOUND: {
            printf("\n");
            for (size_t i = 0; i < tag->tag_compound.size; i++) {
                neko_nbt_print_tree(tag->tag_compound.value[i], indentation /*+ tag->name_size*/ + 2);
            }
            break;
        }
        case NBT_TYPE_INT_ARRAY: {
            printf("[int array]");
            break;
            for (size_t i = 0; i < tag->tag_int_array.size; i++) {
                printf("%d ", tag->tag_int_array.value[i]);
            }
            break;
        }
        case NBT_TYPE_LONG_ARRAY: {
            printf("[long array]");
            break;
            for (size_t i = 0; i < tag->tag_long_array.size; i++) {
                printf("%lld ", tag->tag_long_array.value[i]);
            }
            break;
        }
        default: {
            printf("[error]");
        }
    }

    printf("\n");
}

#pragma endregion

#pragma region font

#if !defined(CUTE_FONT_ALLOC)
#include <stdlib.h>
#define CUTE_FONT_ALLOC(size, ctx) malloc(size)
#define CUTE_FONT_FREE(mem, ctx) free(mem)
#endif

#if !defined(CUTE_FONT_MEMSET)
#include <string.h>
#define CUTE_FONT_MEMSET memset
#endif

#if !defined(CUTE_FONT_MEMCPY)
#include <string.h>
#define CUTE_FONT_MEMCPY memcpy
#endif

#if !defined(CUTE_FONT_STRNCMP)
#include <string.h>
#define CUTE_FONT_STRNCMP strncmp
#endif

#if !defined(CUTE_FONT_STRTOLL)
#include <stdlib.h>
#define CUTE_FONT_STRTOLL strtoll
#endif

#if !defined(CUTE_FONT_STRTOD)
#include <stdlib.h>
#define CUTE_FONT_STRTOD strtod
#endif

#if !defined(CUTE_FONT_STRCHR)
#include <string.h>
#define CUTE_FONT_STRCHR strchr
#endif

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

#define CUTE_FONT_CHECK(X, Y)           \
    do {                                \
        if (!(X)) {                     \
            neko_font_error_reason = Y; \
            goto neko_font_err;         \
        }                               \
    } while (0)
#define CUTE_FONT_FAIL_IF(X)    \
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
    neko_font_t *font = (neko_font_t *)CUTE_FONT_ALLOC(sizeof(neko_font_t), mem_ctx);
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
            CUTE_FONT_CHECK(0, "Unknown codepage encountered.");
    }
    font->codes = (int *)CUTE_FONT_ALLOC(sizeof(int) * font->glyph_count, mem_ctx);
    font->glyphs = (neko_font_glyph_t *)CUTE_FONT_ALLOC(sizeof(neko_font_glyph_t) * font->glyph_count, mem_ctx);
    font->atlas_id = atlas_id;
    font->kern = 0;

    for (int i = 32; i < font->glyph_count + 32; ++i) {
        neko_font_glyph_t *glyph = NULL;
        int w = 0, h = 0;
        neko_font_scan(&img, &x, &y, &font_height);
        CUTE_FONT_CHECK(y < img.w, "Unable to properly scan glyph width. Are the text borders drawn properly?");

        while (!neko_font_is_border(&img, x + w, y)) ++w;
        while (!neko_font_is_border(&img, x, y + h)) ++h;

        glyph = font->glyphs + i - 32;
        if (i < 128)
            font->codes[i - 32] = i;
        else if (codepage == 1252)
            font->codes[i - 32] = neko_font_cp1252[i - 128];
        else
            CUTE_FONT_CHECK(0, "Unknown glyph index found.");

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
    CUTE_FONT_FREE(font->glyphs, mem_ctx);
    CUTE_FONT_FREE(font->codes, mem_ctx);
    CUTE_FONT_FREE(font, mem_ctx);
    return 0;
}

neko_font_t *neko_font_load_ascii(neko_font_u64 atlas_id, const void *pixels, int w, int h, int stride, void *mem_ctx) { return neko_font_load(atlas_id, pixels, w, h, stride, mem_ctx, 0); }

neko_font_t *neko_font_load_1252(neko_font_u64 atlas_id, const void *pixels, int w, int h, int stride, void *mem_ctx) { return neko_font_load(atlas_id, pixels, w, h, stride, mem_ctx, 1252); }

#define CUTE_FONT_INTERNAL_BUFFER_MAX 1024

typedef struct neko_font_parse_t {
    const char *in;
    const char *end;
    int scratch_len;
    char scratch[CUTE_FONT_INTERNAL_BUFFER_MAX];
} neko_font_parse_t;

static int neko_font_isspace(char c) { return (c == ' ') | (c == '\t') | (c == '\n') | (c == '\v') | (c == '\f') | (c == '\r'); }

static int neko_font_next_internal(neko_font_parse_t *p, char *c) {
    CUTE_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
    while (neko_font_isspace(*c = *p->in++)) CUTE_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_next(p, c)                               \
    do {                                                   \
        CUTE_FONT_FAIL_IF(!neko_font_next_internal(p, c)); \
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
        CUTE_FONT_CHECK(neko_font_char == expect, "Found unexpected token."); \
    } while (0)

static int neko_font_read_string_internal(neko_font_parse_t *p) {
    int count = 0;
    int done = 0;
    neko_font_expect(p, '"');

    while (!done) {
        char c = 0;
        CUTE_FONT_CHECK(count < CUTE_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
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
        CUTE_FONT_FAIL_IF(!neko_font_read_string_internal(p)); \
    } while (0)

static int neko_font_read_identifier_internal(neko_font_parse_t *p) {
    int count = 0;
    int done = 0;

    while (1) {
        char c = 0;
        CUTE_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
        CUTE_FONT_CHECK(count < CUTE_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
        c = *p->in;
        if (!neko_font_isspace(c)) break;
        p->in++;
    }

    while (!done) {
        char c = 0;
        CUTE_FONT_CHECK(p->in < p->end, "Attempted to read past input buffer.");
        CUTE_FONT_CHECK(count < CUTE_FONT_INTERNAL_BUFFER_MAX, "String too large to parse.");
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
        CUTE_FONT_FAIL_IF(!neko_font_read_identifier_internal(p)); \
    } while (0)

static int neko_font_read_int_internal(neko_font_parse_t *p, int *out) {
    char *end;
    int val = (int)CUTE_FONT_STRTOLL(p->in, &end, 10);
    CUTE_FONT_CHECK(p->in != end, "Invalid integer found during parse.");
    p->in = end;
    *out = val;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_int(p, num)                               \
    do {                                                         \
        CUTE_FONT_FAIL_IF(!neko_font_read_int_internal(p, num)); \
    } while (0)

static int neko_font_read_float_internal(neko_font_parse_t *p, float *out) {
    char *end;
    float val = (float)CUTE_FONT_STRTOD(p->in, &end);
    CUTE_FONT_CHECK(p->in != end, "Error reading float.");
    p->in = end;
    *out = val;
    return 1;

neko_font_err:
    return 0;
}

#define neko_font_read_float(p, num)                                                   \
    do {                                                                               \
        CUTE_FONT_FAIL_IF(neko_font_read_float_internal(p, num) != CUTE_FONT_SUCCESS); \
    } while (0)

int neko_font_expect_string_internal(neko_font_parse_t *p, const char *str) {
    neko_font_read_string(p);
    if (CUTE_FONT_STRNCMP(p->scratch, str, p->scratch_len))
        return 0;
    else
        return 1;
neko_font_err:
    return 0;
}

#define neko_font_expect_string(p, str)                               \
    do {                                                              \
        CUTE_FONT_FAIL_IF(!neko_font_expect_string_internal(p, str)); \
    } while (0)

int neko_font_expect_identifier_internal(neko_font_parse_t *p, const char *str) {
    neko_font_read_identifier(p);
    if (CUTE_FONT_STRNCMP(p->scratch, str, p->scratch_len))
        return 0;
    else
        return 1;
neko_font_err:
    return 0;
}

#define neko_font_expect_identifier(p, str)                               \
    do {                                                                  \
        CUTE_FONT_FAIL_IF(!neko_font_expect_identifier_internal(p, str)); \
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

    neko_font_t *font = (neko_font_t *)CUTE_FONT_ALLOC(sizeof(neko_font_t), mem_ctx);
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
    font->glyphs = (neko_font_glyph_t *)CUTE_FONT_ALLOC(sizeof(neko_font_glyph_t) * font->glyph_count, mem_ctx);
    font->codes = (int *)CUTE_FONT_ALLOC(sizeof(int) * font->glyph_count, mem_ctx);

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
        kern = (neko_font_kern_t *)CUTE_FONT_ALLOC(sizeof(neko_font_kern_t), mem_ctx);
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
    CUTE_FONT_FREE(font->glyphs, mem_ctx);
    CUTE_FONT_FREE(font->codes, mem_ctx);
    if (font->kern) hashtable_term(&font->kern->table);
    CUTE_FONT_FREE(font->kern, mem_ctx);
    CUTE_FONT_FREE(font, mem_ctx);
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
    neko_font_t *font = (neko_font_t *)CUTE_FONT_ALLOC(sizeof(neko_font_t), font->mem_ctx);
    font->glyph_count = glyph_count;
    font->font_height = font_height;
    font->kern = 0;
    font->glyphs = (neko_font_glyph_t *)CUTE_FONT_ALLOC(sizeof(neko_font_glyph_t) * font->glyph_count, font->mem_ctx);
    font->codes = (int *)CUTE_FONT_ALLOC(sizeof(int) * font->glyph_count, font->mem_ctx);
    return font;
}

void neko_font_add_kerning_pair(neko_font_t *font, int code0, int code1, int kerning) {
    if (!font->kern) {
        neko_font_kern_t *kern = (neko_font_kern_t *)CUTE_FONT_ALLOC(sizeof(neko_font_kern_t), font->mem_ctx);
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

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = a;

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = b;

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = d;

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = d;

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
            buffer[i++] = b;

            CUTE_FONT_CHECK(i < buffer_max, "`buffer_max` is too small.");
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

#pragma endregion
