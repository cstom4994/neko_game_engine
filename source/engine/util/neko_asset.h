
#ifndef NEKO_ASSET_H
#define NEKO_ASSET_H

#include "engine/neko_engine.h"

// Asset handle
typedef struct neko_asset_s {
    uint64_t type_id;
    uint32_t asset_id;
    uint32_t importer_id;  // 'Unique' id of importer, used for type safety
} neko_asset_t;

NEKO_API_DECL neko_asset_t __neko_asset_handle_create_impl(uint64_t type_id, uint32_t asset_id, uint32_t importer_id);

#define neko_asset_handle_create(T, ID, IMPID) __neko_asset_handle_create_impl(neko_hash_str64(neko_to_str(T)), ID, IMPID)

typedef void (*neko_asset_load_func)(const char*, void*, ...);
typedef neko_asset_t (*neko_asset_default_func)(void*);

typedef struct neko_asset_importer_desc_t {
    void (*load_from_file)(const char* path, void* out, ...);
    neko_asset_t (*default_asset)(void* out);
} neko_asset_importer_desc_t;

typedef struct neko_asset_importer_t {
    void* slot_array;
    void* slot_array_data_ptr;
    void* slot_array_indices_ptr;
    void* tmp_ptr;
    uint32_t tmpid;
    size_t data_size;
    neko_asset_importer_desc_t desc;
    uint32_t importer_id;
    neko_asset_t default_asset;
} neko_asset_importer_t;

NEKO_API_DECL void neko_asset_default_load_from_file(const char* path, void* out);
NEKO_API_DECL neko_asset_t neko_asset_default_asset();
NEKO_API_DECL void neko_asset_importer_set_desc(neko_asset_importer_t* imp, neko_asset_importer_desc_t* desc);

#define neko_assets_get_importerp(AM, T) (neko_hash_table_getp((AM)->importers, neko_hash_str64(neko_to_str(T))))

#ifdef __cplusplus
#define gsa_imsa(IMPORTER, T) (decltype(neko_slot_array(T))(IMPORTER)->slot_array)
#else
#define gsa_imsa(IMPORTER, T) ((neko_slot_array(T))(IMPORTER)->slot_array)
#endif

#define neko_assets_register_importer(AM, T, DESC)                                             \
    do {                                                                                       \
        neko_asset_importer_t ai = neko_default_val();                                         \
        ai.data_size = sizeof(T);                                                              \
        ai.importer_id = (AM)->free_importer_id++;                                             \
        neko_asset_importer_set_desc(&ai, (neko_asset_importer_desc_t*)DESC);                  \
        size_t sz = 2 * sizeof(void*) + sizeof(T);                                             \
        neko_slot_array(T) sa = NULL;                                                          \
        neko_slot_array_init((void**)&sa, sizeof(*sa));                                        \
        neko_dyn_array_init((void**)&sa->indices, sizeof(uint32_t));                           \
        neko_dyn_array_init((void**)&sa->data, sizeof(T));                                     \
        ai.slot_array = (void*)sa;                                                             \
        ai.tmp_ptr = (void*)&sa->tmp;                                                          \
        ai.slot_array_indices_ptr = (void*)sa->indices;                                        \
        ai.slot_array_data_ptr = (void*)sa->data;                                              \
        if (!ai.desc.load_from_file) {                                                         \
            ai.desc.load_from_file = (neko_asset_load_func)&neko_asset_default_load_from_file; \
        }                                                                                      \
        neko_hash_table_insert((AM)->importers, neko_hash_str64(neko_to_str(T)), ai);          \
    } while (0)

// Need a way to be able to print upon assert
#define neko_assets_load_from_file(AM, T, PATH, ...)                                                                                                                           \
    (/*neko_assert(neko_hash_table_key_exists((AM)->importers, neko_hash_str64(neko_to_str(T)))),*/                                                                            \
     (AM)->tmpi = neko_hash_table_getp((AM)->importers, neko_hash_str64(neko_to_str(T))), (AM)->tmpi->desc.load_from_file(PATH, (AM)->tmpi->tmp_ptr, ##__VA_ARGS__),           \
     (AM)->tmpi->tmpid = neko_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL), \
     neko_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id))

#define neko_assets_create_asset(AM, T, DATA)                                                                                                                                  \
    (/*neko_assert(neko_hash_table_key_exists((AM)->importers, neko_hash_str64(neko_to_str(T)))),*/                                                                            \
     (AM)->tmpi = neko_hash_table_getp((AM)->importers, neko_hash_str64(neko_to_str(T))), (AM)->tmpi->tmp_ptr = (DATA),                                                        \
     (AM)->tmpi->tmpid = neko_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL), \
     neko_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id))

typedef struct neko_asset_manager_t {
    neko_hash_table(uint64_t, neko_asset_importer_t) importers;  // Maps hashed types to importer
    neko_asset_importer_t* tmpi;                                 // Temporary importer for caching
    uint32_t free_importer_id;
} neko_asset_manager_t;

NEKO_API_DECL neko_asset_manager_t neko_asset_manager_new();
NEKO_API_DECL void neko_asset_manager_free(neko_asset_manager_t* am);
NEKO_API_DECL void* __neko_assets_getp_impl(neko_asset_manager_t* am, uint64_t type_id, neko_asset_t hndl);

#define neko_assets_getp(AM, T, HNDL) (T*)(__neko_assets_getp_impl(AM, neko_hash_str64(neko_to_str(T)), HNDL))

#define neko_assets_get(AM, T, HNDL) *(neko_assets_getp(AM, T, HNDL));

/** @} */  // end of neko_asset_util

/*==== Implementation ====*/

#ifdef NEKO_ASSET_IMPL

neko_asset_t __neko_asset_handle_create_impl(uint64_t type_id, uint32_t asset_id, uint32_t importer_id) {
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
    neko_asset_importer_desc_t audio_desc = neko_default_val();
    neko_asset_importer_desc_t mesh_desc = neko_default_val();
    neko_asset_importer_desc_t asset_desc = neko_default_val();

    tex_desc.load_from_file = (neko_asset_load_func)&neko_asset_texture_load_from_file;
    font_desc.load_from_file = (neko_asset_load_func)&neko_asset_font_load_from_file;
    audio_desc.load_from_file = (neko_asset_load_func)&neko_asset_audio_load_from_file;
    mesh_desc.load_from_file = (neko_asset_load_func)&neko_asset_mesh_load_from_file;

    neko_assets_register_importer(&assets, neko_asset_t, &asset_desc);
    neko_assets_register_importer(&assets, neko_asset_texture_t, &tex_desc);
    neko_assets_register_importer(&assets, neko_asset_font_t, &font_desc);
    neko_assets_register_importer(&assets, neko_asset_audio_t, &audio_desc);
    neko_assets_register_importer(&assets, neko_asset_mesh_t, &mesh_desc);

    return assets;
}

void neko_asset_manager_free(neko_asset_manager_t* am) {
    // Free all data
}

void* __neko_assets_getp_impl(neko_asset_manager_t* am, uint64_t type_id, neko_asset_t hndl) {
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

    neko_asset_importer_t* imp = neko_hash_table_getp(am->importers, type_id);

    // Vertify that importer id and handle importer id align
    if (imp->importer_id != hndl.importer_id) {
        neko_println("Warning: Importer id: %zu does not match handle importer id: %zu.", imp->importer_id, hndl.importer_id);
        neko_assert(false);
        return NULL;
    }

    // Need to get data index from slot array using hndl asset id
    size_t offset = (((sizeof(uint32_t) * hndl.asset_id) + 3) & (~3));
    uint32_t idx = *(uint32_t*)((char*)(imp->slot_array_indices_ptr) + offset);
    // Then need to return pointer to data at index
    size_t data_sz = imp->data_size;
    size_t s = data_sz == 8 ? 7 : 3;
    offset = (((data_sz * idx) + s) & (~s));
    return ((char*)(imp->slot_array_data_ptr) + offset);
}

void neko_asset_importer_set_desc(neko_asset_importer_t* imp, neko_asset_importer_desc_t* desc) {
    imp->desc = desc ? *desc : imp->desc;
    imp->desc.load_from_file = imp->desc.load_from_file ? (neko_asset_load_func)imp->desc.load_from_file : (neko_asset_load_func)&neko_asset_default_load_from_file;
    imp->desc.default_asset = imp->desc.default_asset ? (neko_asset_default_func)imp->desc.default_asset : (neko_asset_default_func)&neko_asset_default_asset;
}

neko_asset_t neko_asset_default_asset() {
    neko_asset_t a = neko_default_val();
    return a;
}

void neko_asset_default_load_from_file(const char* path, void* out) {
    // Nothing...
}

#undef NEKO_ASSET_IMPL
#endif  // NEKO_ASSET_IMPL

#endif  // NEKO_ASSET_H
