
#include "neko_asset.h"

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
    font_desc.load_from_file = (neko_asset_load_func)&neko_asset_font_load_from_file;
    // audio_desc.load_from_file = (neko_asset_load_func)&neko_asset_audio_load_from_file;
    mesh_desc.load_from_file = (neko_asset_load_func)&neko_asset_mesh_load_from_file;

    neko_assets_register_importer(&assets, neko_asset_t, &asset_desc);
    neko_assets_register_importer(&assets, neko_asset_texture_t, &tex_desc);
    neko_assets_register_importer(&assets, neko_asset_font_t, &font_desc);
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

void neko_asset_default_load_from_file(const char *path, void *out) {
    // Nothing...
}

/*==========================
// NEKO_ASSET_TYPES
==========================*/

#ifndef NEKO_NO_STB_RECT_PACK
#define STB_RECT_PACK_IMPLEMENTATION
#endif

#ifndef NEKO_NO_STB_TRUETYPE
// #define STBTT_RASTERIZER_VERSION 0
#define STB_TRUETYPE_IMPLEMENTATION
#endif

#ifndef NEKO_NO_STB_DEFINE
#define STB_DEFINE
#endif

#ifndef NEKO_NO_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#ifndef NEKO_NO_CGLTF
#define CGLTF_IMPLEMENTATION
#endif

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

NEKO_API_DECL bool neko_asset_texture_load_from_file(const char *path, void *out, neko_graphics_texture_desc_t *desc, bool32_t flip_on_load, bool32_t keep_data) {
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

bool neko_asset_font_load_from_file(const char *path, void *out, u32 point_size) {
    size_t len = 0;
    char *ttf = neko_platform_read_file_contents(path, "rb", &len);
    if (!point_size) {
        neko_println("Warning: Font: %s: Point size not declared. Setting to default 16.", path);
        point_size = 16;
    }
    bool ret = neko_asset_font_load_from_memory(ttf, len, out, point_size);
    if (!ret) {
        neko_println("Font Failed to Load: %s", path);
    } else {
        neko_println("Font Successfully Loaded: %s", path);
    }
    neko_safe_free(ttf);
    return ret;
}

bool neko_asset_font_load_from_memory(const void *memory, size_t sz, void *out, u32 point_size) {
    neko_asset_font_t *f = (neko_asset_font_t *)out;

    if (!point_size) {
        neko_println("Warning: Font: Point size not declared. Setting to default 16.");
        point_size = 16;
    }

    // Poor attempt at an auto resized texture
    const u32 point_wh = neko_max(point_size, 32);
    const u32 w = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;
    const u32 h = (point_wh / 32 * 512) + (point_wh / 32 * 512) % 512;

    const u32 num_comps = 4;
    u8 *alpha_bitmap = (u8 *)neko_malloc(w * h);
    u8 *flipmap = (u8 *)neko_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8 *)memory, 0, (f32)point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar *)f->glyphs);  // no guarantee this fits!

    // Flip texture
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

    // Generate atlas texture for bitmap with bitmap data
    f->texture.hndl = neko_graphics_texture_create(&desc);
    f->texture.desc = desc;
    *f->texture.desc.data = NULL;

    bool success = false;
    if (v <= 0) {
        neko_println("Font Failed to Load, Baked Texture Was Too Small: %d", v);
    } else {
        neko_println("Font Successfully Loaded: %d", v);
        success = true;
    }

    neko_free(alpha_bitmap);
    neko_free(flipmap);
    return success;
}

NEKO_API_DECL f32 neko_asset_font_max_height(const neko_asset_font_t *fp) {
    if (!fp) return 0.f;
    f32 h = 0.f, x = 0.f, y = 0.f;
    const char *txt = "1l`'f()ABCDEFGHIJKLMNOjPQqSTU!";
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

NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions(const neko_asset_font_t *fp, const char *text, s32 len) { return neko_asset_font_text_dimensions_ex(fp, text, len, 0); }

NEKO_API_DECL neko_vec2 neko_asset_font_text_dimensions_ex(const neko_asset_font_t *fp, const char *text, s32 len, bool32_t include_past_baseline) {
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

bool neko_util_load_gltf_data_from_file(const char *path, neko_asset_mesh_decl_t *decl, neko_asset_mesh_raw_data_t **out, u32 *mesh_count) {
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

bool neko_asset_mesh_load_from_file(const char *path, void *out, neko_asset_mesh_decl_t *decl, void *data_out, size_t data_size) {
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

#pragma region packer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/lz4/lz4.h"

inline static FILE *openFile(const char *filePath, const char *mode) {
    FILE *file;
    errno_t error = fopen_s(&file, filePath, mode);
    if (error != 0) return NULL;
    return file;
}

#define seekFile(file, offset, whence) _fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
#define closeFile(file) fclose(file)

typedef struct pack_item {
    pack_iteminfo info;
    char *path;
} pack_item;

struct neko_packreader_t {
    FILE *file;
    u64 itemCount;
    pack_item *items;
    u8 *dataBuffer;
    u8 *zipBuffer;
    u32 dataSize;
    u32 zipSize;
    pack_item searchItem;
};

neko_private(void) destroy_pack_items(u64 itemCount, pack_item *items) {
    neko_assert(itemCount == 0 || (itemCount > 0 && items));

    for (u64 i = 0; i < itemCount; i++) neko_safe_free(items[i].path);
    neko_safe_free(items);
}

neko_private(neko_pack_result) create_pack_items(FILE *packFile, u64 itemCount, pack_item **_items) {
    neko_assert(packFile);
    neko_assert(itemCount > 0);
    neko_assert(_items);

    pack_item *items = (pack_item *)neko_safe_malloc(itemCount * sizeof(pack_item));

    if (!items) return FAILED_TO_ALLOCATE_PACK_RESULT;

    for (u64 i = 0; i < itemCount; i++) {
        pack_iteminfo info;

        size_t result = fread(&info, sizeof(pack_iteminfo), 1, packFile);

        if (result != 1) {
            destroy_pack_items(i, items);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        if (info.dataSize == 0 || info.pathSize == 0) {
            destroy_pack_items(i, items);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        char *path = (char *)neko_safe_malloc((info.pathSize + 1) * sizeof(char));

        if (!path) {
            destroy_pack_items(i, items);
            return FAILED_TO_ALLOCATE_PACK_RESULT;
        }

        result = fread(path, sizeof(char), info.pathSize, packFile);

        path[info.pathSize] = 0;

        if (result != info.pathSize) {
            destroy_pack_items(i, items);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        s64 fileOffset = info.zipSize > 0 ? info.zipSize : info.dataSize;

        int seekResult = seekFile(packFile, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            destroy_pack_items(i, items);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        pack_item *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return SUCCESS_PACK_RESULT;
}

neko_pack_result neko_pack_read(const char *filePath, u32 dataBufferCapacity, bool isResourcesDirectory, neko_packreader_t **pack_reader) {
    neko_assert(filePath);
    neko_assert(pack_reader);

    neko_packreader_t *pack = (neko_packreader_t *)neko_safe_calloc(1, sizeof(neko_packreader_t));

    if (!pack) return FAILED_TO_ALLOCATE_PACK_RESULT;

    pack->zipBuffer = NULL;
    pack->zipSize = 0;

    char *path;

    path = (char *)filePath;

    FILE *file = openFile(path, "rb");

    if (!file) {
        neko_pack_destroy(pack);
        return FAILED_TO_OPEN_FILE_PACK_RESULT;
    }

    pack->file = file;

    char header[PACK_HEADER_SIZE];

    size_t result = fread(header, sizeof(char), PACK_HEADER_SIZE, file);

    if (result != PACK_HEADER_SIZE) {
        neko_pack_destroy(pack);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        neko_pack_destroy(pack);
        return BAD_FILE_TYPE_PACK_RESULT;
    }

    if (header[4] != PACK_VERSION_MAJOR || header[5] != PACK_VERSION_MINOR) {
        neko_pack_destroy(pack);
        return BAD_FILE_VERSION_PACK_RESULT;
    }

    // Skipping PATCH version check

    if (header[7] != !neko_little_endian) {
        neko_pack_destroy(pack);
        return BAD_FILE_ENDIANNESS_PACK_RESULT;
    }

    u64 itemCount;

    result = fread(&itemCount, sizeof(u64), 1, file);

    if (result != 1) {
        neko_pack_destroy(pack);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (itemCount == 0) {
        neko_pack_destroy(pack);
        return BAD_DATA_SIZE_PACK_RESULT;
    }

    pack_item *items;

    neko_pack_result packResult = create_pack_items(file, itemCount, &items);

    if (packResult != SUCCESS_PACK_RESULT) {
        neko_pack_destroy(pack);
        ;
        return packResult;
    }

    pack->itemCount = itemCount;
    pack->items = items;

    u8 *dataBuffer;

    if (dataBufferCapacity > 0) {
        dataBuffer = (u8 *)neko_safe_malloc(dataBufferCapacity * sizeof(u8));

        if (!dataBuffer) {
            neko_pack_destroy(pack);
            return FAILED_TO_ALLOCATE_PACK_RESULT;
        }
    } else {
        dataBuffer = NULL;
    }

    pack->dataBuffer = dataBuffer;
    pack->dataSize = dataBufferCapacity;

    *pack_reader = pack;
    return SUCCESS_PACK_RESULT;
}
void neko_pack_destroy(neko_packreader_t *pack_reader) {
    if (!pack_reader) return;

    neko_safe_free(pack_reader->dataBuffer);
    neko_safe_free(pack_reader->zipBuffer);
    destroy_pack_items(pack_reader->itemCount, pack_reader->items);
    if (pack_reader->file) closeFile(pack_reader->file);
    neko_safe_free(pack_reader);
}

u64 neko_pack_item_count(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    return pack_reader->itemCount;
}

neko_private(int) neko_compare_pack_items(const void *_a, const void *_b) {
    // NOTE: a and b should not be NULL!
    // Skipping here neko_assertions for debug build speed.

    const pack_item *a = (pack_item *)_a;
    const pack_item *b = (pack_item *)_b;

    int difference = (int)a->info.pathSize - (int)b->info.pathSize;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.pathSize * sizeof(char));
}

b8 neko_pack_item_index(neko_packreader_t *pack_reader, const char *path, u64 *index) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(index);
    neko_assert(strlen(path) <= UINT8_MAX);

    pack_item *searchItem = &pack_reader->searchItem;

    searchItem->info.pathSize = (u8)strlen(path);
    searchItem->path = (char *)path;

    pack_item *item = (pack_item *)bsearch(searchItem, pack_reader->items, pack_reader->itemCount, sizeof(pack_item), neko_compare_pack_items);

    if (!item) return false;

    *index = item - pack_reader->items;
    return true;
}

u32 neko_pack_item_size(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    return pack_reader->items[index].info.dataSize;
}

const char *neko_pack_item_path(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    return pack_reader->items[index].path;
}

neko_pack_result neko_pack_item_data_with_index(neko_packreader_t *pack_reader, u64 index, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    neko_assert(data);
    neko_assert(size);

    pack_iteminfo info = pack_reader->items[index].info;
    u8 *dataBuffer = pack_reader->dataBuffer;

    if (dataBuffer) {
        if (info.dataSize > pack_reader->dataSize) {
            dataBuffer = (u8 *)neko_safe_realloc(dataBuffer, info.dataSize * sizeof(u8));

            if (!dataBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->dataBuffer = dataBuffer;
            pack_reader->dataSize = info.dataSize;
        }
    } else {
        dataBuffer = (u8 *)neko_safe_malloc(info.dataSize * sizeof(u8));

        if (!dataBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

        pack_reader->dataBuffer = dataBuffer;
        pack_reader->dataSize = info.dataSize;
    }

    u8 *zipBuffer = pack_reader->zipBuffer;

    if (zipBuffer) {
        if (info.zipSize > pack_reader->zipSize) {
            zipBuffer = (u8 *)neko_safe_realloc(zipBuffer, info.zipSize * sizeof(u8));

            if (!zipBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->zipBuffer = zipBuffer;
            pack_reader->zipSize = info.zipSize;
        }
    } else {
        if (info.zipSize > 0) {
            zipBuffer = (u8 *)neko_safe_malloc(info.zipSize * sizeof(u8));

            if (!zipBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->zipBuffer = zipBuffer;
            pack_reader->zipSize = info.zipSize;
        }
    }

    FILE *file = pack_reader->file;

    s64 fileOffset = (s64)(info.fileOffset + sizeof(pack_iteminfo) + info.pathSize);

    int seekResult = seekFile(file, fileOffset, SEEK_SET);

    if (seekResult != 0) return FAILED_TO_SEEK_FILE_PACK_RESULT;

    if (info.zipSize > 0) {
        size_t result = fread(zipBuffer, sizeof(u8), info.zipSize, file);

        if (result != info.zipSize) return FAILED_TO_READ_FILE_PACK_RESULT;

        result = LZ4_decompress_safe((char *)zipBuffer, (char *)dataBuffer, info.zipSize, info.dataSize);

        // METADOT_BUG("[Assets] LZ4_decompress_safe ", result, " ", info.dataSize);

        if (result < 0 || result != info.dataSize) {
            return FAILED_TO_DECOMPRESS_PACK_RESULT;
        }
    } else {
        size_t result = fread(dataBuffer, sizeof(u8), info.dataSize, file);

        if (result != info.dataSize) return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    *data = dataBuffer;
    *size = info.dataSize;
    return SUCCESS_PACK_RESULT;
}

neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, const char *path, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(data);
    neko_assert(size);
    neko_assert(strlen(path) <= UINT8_MAX);

    u64 index;

    if (!neko_pack_item_index(pack_reader, path, &index)) {
        return FAILED_TO_GET_ITEM_PACK_RESULT;
    }

    return neko_pack_item_data_with_index(pack_reader, index, data, size);
}

void neko_pack_free_buffers(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    neko_safe_free(pack_reader->dataBuffer);
    neko_safe_free(pack_reader->zipBuffer);
    pack_reader->dataBuffer = NULL;
    pack_reader->zipBuffer = NULL;
}

neko_private(void) neko_removePackItemFiles(u64 itemCount, pack_item *packItems) {
    neko_assert(itemCount == 0 || (itemCount > 0 && packItems));

    for (u64 i = 0; i < itemCount; i++) remove(packItems[i].path);
}

neko_pack_result neko_unpack_files(const char *filePath, b8 printProgress) {
    neko_assert(filePath);

    neko_packreader_t *pack_reader;

    neko_pack_result packResult = neko_pack_read(filePath, 0, false, &pack_reader);

    if (packResult != SUCCESS_PACK_RESULT) return packResult;

    u64 totalRawSize = 0, totalZipSize = 0;

    u64 itemCount = pack_reader->itemCount;
    pack_item *items = pack_reader->items;

    for (u64 i = 0; i < itemCount; i++) {
        pack_item *item = &items[i];

        if (printProgress) {
            // METADOT_BUG("Unpacking ", item->path);
        }

        const u8 *dataBuffer;
        u32 dataSize;

        packResult = neko_pack_item_data(pack_reader, i, &dataBuffer, &dataSize);

        if (packResult != SUCCESS_PACK_RESULT) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return packResult;
        }

        u8 pathSize = item->info.pathSize;

        char itemPath[UINT8_MAX + 1];

        memcpy(itemPath, item->path, pathSize * sizeof(char));
        itemPath[pathSize] = 0;

        for (u8 j = 0; j < pathSize; j++) {
            if (itemPath[j] == '/' || itemPath[j] == '\\') {
                itemPath[j] = '-';
            }
        }

        FILE *itemFile = openFile(itemPath, "wb");

        if (!itemFile) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        size_t result = fwrite(dataBuffer, sizeof(u8), dataSize, itemFile);

        closeFile(itemFile);

        if (result != dataSize) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        if (printProgress) {
            u32 rawFileSize = item->info.dataSize;
            u32 zipFileSize = item->info.zipSize > 0 ? item->info.zipSize : item->info.dataSize;

            totalRawSize += rawFileSize;
            totalZipSize += zipFileSize;

            int progress = (int)(((f32)(i + 1) / (f32)itemCount) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", rawFileSize, zipFileSize, progress);
            fflush(stdout);
        }
    }

    neko_pack_destroy(pack_reader);

    if (printProgress) {
        printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)itemCount, (long long unsigned int)totalRawSize, (long long unsigned int)totalZipSize);
    }

    return SUCCESS_PACK_RESULT;
}

neko_private(neko_pack_result) neko_write_pack_items(FILE *packFile, u64 itemCount, char **itemPaths, b8 printProgress) {
    neko_assert(packFile);
    neko_assert(itemCount > 0);
    neko_assert(itemPaths);

    u32 bufferSize = 1;

    u8 *itemData = (u8 *)neko_safe_malloc(sizeof(u8));

    if (!itemData) return FAILED_TO_ALLOCATE_PACK_RESULT;

    u8 *zipData = (u8 *)neko_safe_malloc(sizeof(u8));

    if (!zipData) {
        neko_safe_free(itemData);
        return FAILED_TO_ALLOCATE_PACK_RESULT;
    }

    u64 totalZipSize = 0, totalRawSize = 0;

    for (u64 i = 0; i < itemCount; i++) {
        char *itemPath = itemPaths[i];

        if (printProgress) {
            printf("Packing \"%s\" file. ", itemPath);
            fflush(stdout);
        }

        size_t pathSize = strlen(itemPath);

        if (pathSize > UINT8_MAX) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        FILE *itemFile = openFile(itemPath, "rb");

        if (!itemFile) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        int seekResult = seekFile(itemFile, 0, SEEK_END);

        if (seekResult != 0) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        u64 itemSize = (u64)tellFile(itemFile);

        if (itemSize == 0 || itemSize > UINT32_MAX) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        seekResult = seekFile(itemFile, 0, SEEK_SET);

        if (seekResult != 0) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        if (itemSize > bufferSize) {
            u8 *newBuffer = (u8 *)neko_safe_realloc(itemData, itemSize * sizeof(u8));

            if (!newBuffer) {
                closeFile(itemFile);
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_ALLOCATE_PACK_RESULT;
            }

            itemData = newBuffer;

            newBuffer = (u8 *)neko_safe_realloc(zipData, itemSize * sizeof(u8));

            if (!newBuffer) {
                closeFile(itemFile);
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_ALLOCATE_PACK_RESULT;
            }

            zipData = newBuffer;
        }

        size_t result = fread(itemData, sizeof(u8), itemSize, itemFile);

        closeFile(itemFile);

        if (result != itemSize) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        size_t zipSize;

        if (itemSize > 1) {

            const int max_dst_size = LZ4_compressBound(itemSize);

            zipSize = LZ4_compress_fast((char *)itemData, (char *)zipData, itemSize, max_dst_size, 10);

            if (zipSize <= 0 || zipSize >= itemSize) {
                zipSize = 0;
            }
        } else {
            zipSize = 0;
        }

        s64 fileOffset = tellFile(packFile);

        pack_iteminfo info = {
                (u32)zipSize,
                (u32)itemSize,
                (u64)fileOffset,
                (u8)pathSize,
        };

        result = fwrite(&info, sizeof(pack_iteminfo), 1, packFile);

        if (result != 1) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_WRITE_FILE_PACK_RESULT;
        }

        result = fwrite(itemPath, sizeof(char), info.pathSize, packFile);

        if (result != info.pathSize) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_WRITE_FILE_PACK_RESULT;
        }

        if (zipSize > 0) {
            result = fwrite(zipData, sizeof(u8), zipSize, packFile);

            if (result != zipSize) {
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_WRITE_FILE_PACK_RESULT;
            }
        } else {
            result = fwrite(itemData, sizeof(u8), itemSize, packFile);

            if (result != itemSize) {
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_WRITE_FILE_PACK_RESULT;
            }
        }

        if (printProgress) {
            u32 zipFileSize = zipSize > 0 ? (u32)zipSize : (u32)itemSize;
            u32 rawFileSize = (u32)itemSize;

            totalZipSize += zipFileSize;
            totalRawSize += rawFileSize;

            int progress = (int)(((f32)(i + 1) / (f32)itemCount) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", zipFileSize, rawFileSize, progress);
            fflush(stdout);
        }
    }

    neko_safe_free(zipData);
    neko_safe_free(itemData);

    if (printProgress) {
        int compression = (int)((1.0 - (f64)(totalZipSize) / (f64)totalRawSize) * 100.0);
        printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)itemCount, (long long unsigned int)totalZipSize, (long long unsigned int)totalRawSize, compression);
    }

    return SUCCESS_PACK_RESULT;
}

neko_private(int) neko_comparePackItemPaths(const void *_a, const void *_b) {
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

neko_pack_result neko_pack_files(const char *filePath, u64 fileCount, const char **filePaths, b8 printProgress) {
    neko_assert(filePath);
    neko_assert(fileCount > 0);
    neko_assert(filePaths);

    char **itemPaths = (char **)neko_safe_malloc(fileCount * sizeof(char *));

    if (!itemPaths) return FAILED_TO_ALLOCATE_PACK_RESULT;

    u64 itemCount = 0;

    for (u64 i = 0; i < fileCount; i++) {
        b8 alreadyAdded = false;

        for (u64 j = 0; j < itemCount; j++) {
            if (i != j && strcmp(filePaths[i], itemPaths[j]) == 0) alreadyAdded = true;
        }

        if (!alreadyAdded) itemPaths[itemCount++] = (char *)filePaths[i];
    }

    qsort(itemPaths, itemCount, sizeof(char *), neko_comparePackItemPaths);

    FILE *packFile = openFile(filePath, "wb");

    if (!packFile) {
        neko_safe_free(itemPaths);
        return FAILED_TO_CREATE_FILE_PACK_RESULT;
    }

    char header[PACK_HEADER_SIZE] = {
            'P', 'A', 'C', 'K', PACK_VERSION_MAJOR, PACK_VERSION_MINOR, PACK_VERSION_PATCH, !neko_little_endian,
    };

    size_t writeResult = fwrite(header, sizeof(char), PACK_HEADER_SIZE, packFile);

    if (writeResult != PACK_HEADER_SIZE) {
        neko_safe_free(itemPaths);
        closeFile(packFile);
        remove(filePath);
        return FAILED_TO_WRITE_FILE_PACK_RESULT;
    }

    writeResult = fwrite(&itemCount, sizeof(u64), 1, packFile);

    if (writeResult != 1) {
        neko_safe_free(itemPaths);
        closeFile(packFile);
        remove(filePath);
        return FAILED_TO_WRITE_FILE_PACK_RESULT;
    }

    neko_pack_result packResult = neko_write_pack_items(packFile, itemCount, itemPaths, printProgress);

    neko_safe_free(itemPaths);
    closeFile(packFile);

    if (packResult != SUCCESS_PACK_RESULT) {
        remove(filePath);
        return packResult;
    }

    return SUCCESS_PACK_RESULT;
}

void neko_pack_version(u8 *majorVersion, u8 *minorVersion, u8 *patchVersion) {
    neko_assert(majorVersion);
    neko_assert(minorVersion);
    neko_assert(patchVersion);

    *majorVersion = PACK_VERSION_MAJOR;
    *minorVersion = PACK_VERSION_MINOR;
    *patchVersion = PACK_VERSION_PATCH;
}

neko_pack_result neko_pack_info(const char *filePath, u8 *majorVersion, u8 *minorVersion, u8 *patchVersion, b8 *isLittleEndian, u64 *_itemCount) {
    neko_assert(filePath);
    neko_assert(majorVersion);
    neko_assert(minorVersion);
    neko_assert(patchVersion);
    neko_assert(isLittleEndian);
    neko_assert(_itemCount);

    FILE *file = openFile(filePath, "rb");

    if (!file) return FAILED_TO_OPEN_FILE_PACK_RESULT;

    char header[PACK_HEADER_SIZE];

    size_t result = fread(header, sizeof(char), PACK_HEADER_SIZE, file);

    if (result != PACK_HEADER_SIZE) {
        closeFile(file);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        closeFile(file);
        return BAD_FILE_TYPE_PACK_RESULT;
    }

    u64 itemCount;

    result = fread(&itemCount, sizeof(u64), 1, file);

    closeFile(file);

    if (result != 1) return FAILED_TO_READ_FILE_PACK_RESULT;

    *majorVersion = header[4];
    *minorVersion = header[5];
    *patchVersion = header[6];
    *isLittleEndian = !header[7];
    *_itemCount = itemCount;
    return SUCCESS_PACK_RESULT;
}

#pragma endregion
