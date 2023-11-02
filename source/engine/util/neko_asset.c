
#include "neko_asset.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

neko_static_inline FILE *openFile(const_str file_path, const_str mode) {
    FILE *file;
    errno_t error = fopen_s(&file, file_path, mode);
    if (error != 0) return NULL;
    return file;
}

#define openFile openFile
#define seekFile(file, offset, whence) _fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
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

    memcpy((void*)(*data), data_buffer, info.data_size);

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
