
#ifndef NEKO_ASSET_IMPL
#define NEKO_ASSET_IMPL

#include "engine/neko_engine.h"

// Asset handle
typedef struct neko_asset_s {
    u64 type_id;
    u32 asset_id;
    u32 importer_id;  // 'Unique' id of importer, used for type safety
} neko_asset_t;

NEKO_API_DECL neko_asset_t __neko_asset_handle_create_impl(u64 type_id, u32 asset_id, u32 importer_id);

#define neko_asset_handle_create(T, ID, IMPID) __neko_asset_handle_create_impl(neko_hash_str64(neko_to_str(T)), ID, IMPID)

typedef void (*neko_asset_load_func)(const_str, void*, ...);
typedef neko_asset_t (*neko_asset_default_func)(void*);

typedef struct neko_asset_importer_desc_t {
    void (*load_from_file)(const_str path, void* out, ...);
    neko_asset_t (*default_asset)(void* out);
} neko_asset_importer_desc_t;

typedef struct neko_asset_importer_t {
    void* slot_array;
    void* slot_array_data_ptr;
    void* slot_array_indices_ptr;
    void* tmp_ptr;
    u32 tmpid;
    size_t data_size;
    neko_asset_importer_desc_t desc;
    u32 importer_id;
    neko_asset_t default_asset;
} neko_asset_importer_t;

NEKO_API_DECL void neko_asset_default_load_from_file(const_str path, void* out);
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
        neko_dyn_array_init((void**)&sa->indices, sizeof(u32));                                \
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

typedef struct neko_asset_manager_s {
    neko_hash_table(u64, neko_asset_importer_t) importers;  // Maps hashed types to importer
    neko_asset_importer_t* tmpi;                            // Temporary importer for caching
    u32 free_importer_id;
} neko_asset_manager_t;

NEKO_API_DECL neko_asset_manager_t neko_asset_manager_new();
NEKO_API_DECL void neko_asset_manager_free(neko_asset_manager_t* am);
NEKO_API_DECL void* __neko_assets_getp_impl(neko_asset_manager_t* am, u64 type_id, neko_asset_t hndl);

#define neko_assets_getp(AM, T, HNDL) (T*)(__neko_assets_getp_impl(AM, neko_hash_str64(neko_to_str(T)), HNDL))

#define neko_assets_get(AM, T, HNDL) *(neko_assets_getp(AM, T, HNDL));

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// Texture
typedef struct neko_asset_texture_t {
    neko_handle(neko_graphics_texture_t) hndl;
    neko_graphics_texture_desc_t desc;
} neko_asset_texture_t;

NEKO_API_DECL bool neko_asset_texture_load_from_file(const_str path, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data);
NEKO_API_DECL bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, neko_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data);

// Font
typedef struct neko_baked_char_t {
    u32 codepoint;
    u16 x0, y0, x1, y1;
    float xoff, yoff, advance;
    u32 width, height;
} neko_baked_char_t;

typedef struct neko_asset_ascii_font_t {
    void* font_info;
    neko_baked_char_t glyphs[96];
    neko_asset_texture_t texture;
    float ascent;
    float descent;
    float line_gap;
} neko_asset_ascii_font_t;

NEKO_API_DECL bool neko_asset_ascii_font_load_from_file(const_str path, void* out, u32 point_size);
NEKO_API_DECL bool neko_asset_ascii_font_load_from_memory(const void* memory, size_t sz, void* out, u32 point_size);
NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions(const neko_asset_ascii_font_t* font, const_str text, s32 len);
NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions_ex(const neko_asset_ascii_font_t* fp, const_str text, s32 len, bool32_t include_past_baseline);
NEKO_API_DECL float neko_asset_ascii_font_max_height(const neko_asset_ascii_font_t* font);

// Mesh
neko_enum_decl(neko_asset_mesh_attribute_type, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT);

typedef struct neko_asset_mesh_layout_t {
    neko_asset_mesh_attribute_type type;  // Type of attribute
    u32 idx;                              // Optional index (for joint/weight/texcoord/color)
} neko_asset_mesh_layout_t;

typedef struct neko_asset_mesh_decl_t {
    neko_asset_mesh_layout_t* layout;  // Mesh attribute layout array
    size_t layout_size;                // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_asset_mesh_decl_t;

typedef struct neko_asset_mesh_primitive_t {
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
    u32 count;
} neko_asset_mesh_primitive_t;

typedef struct neko_asset_mesh_t {
    neko_dyn_array(neko_asset_mesh_primitive_t) primitives;
} neko_asset_mesh_t;

// Structured/packed raw mesh data
typedef struct neko_asset_mesh_raw_data_t {
    u32 prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} neko_asset_mesh_raw_data_t;

NEKO_API_DECL bool neko_asset_mesh_load_from_file(const_str path, void* out, neko_asset_mesh_decl_t* decl, void* data_out, size_t data_size);
NEKO_API_DECL bool neko_util_load_gltf_data_from_file(const_str path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);
// NEKO_API_DECL bool neko_util_load_gltf_data_from_memory(const void* memory, size_t sz, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);

/*==========================
// LZ77
==========================*/

u32 neko_lz_encode(const void* in, u32 inlen, void* out, u32 outlen, u32 flags);  // [0..(6)..9]
u32 neko_lz_decode(const void* in, u32 inlen, void* out, u32 outlen);
u32 neko_lz_bounds(u32 inlen, u32 flags);

/*==========================
// NEKO_FNT
==========================*/

typedef struct {
    u32 ch;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    s16 offset_x;
    s16 offset_y;
    s16 advance_x;
    u8 page;
    u8 channel;
} neko_fnt_glyph;

typedef struct {
    u32 first_char;
    u32 second_char;
    s16 amount;
} neko_fnt_kerning;

// fnt 二进制文件存储格式见
// http://www.angelcode.com/products/bmfont/doc/file_format.html
typedef struct {
    char* name;
    int size;
    int line_height;
    int base;
    size_t num_pages;
    char** page_names;
    size_t num_glyphs;
    neko_fnt_glyph* glyphs;
    size_t num_kerning_pairs;
    neko_fnt_kerning* kerning_pairs;
    const char* error_message;
} neko_fnt;

typedef size_t (*neko_fnt_read_func_t)(void* user_data, u8* buffer, size_t count);

NEKO_API_DECL neko_fnt* neko_fnt_read(FILE* file);
NEKO_API_DECL void neko_fnt_free(neko_fnt* fnt);
NEKO_API_DECL neko_fnt* neko_fnt_read_from_callbacks(void* user_data, neko_fnt_read_func_t read_func);

/*==========================
// NEKO_PACK
==========================*/

#define neko_pack_head_size 8

// typedef enum neko_packresult_t {
//     SUCCESS_PACK_RESULT = 0,
//     FAILED_TO_ALLOCATE_PACK_RESULT = 1,
//     FAILED_TO_CREATE_LZ4_PACK_RESULT = 2,
//     FAILED_TO_CREATE_FILE_PACK_RESULT = 3,
//     FAILED_TO_OPEN_FILE_PACK_RESULT = 4,
//     FAILED_TO_WRITE_FILE_PACK_RESULT = 5,
//     FAILED_TO_READ_FILE_PACK_RESULT = 6,
//     FAILED_TO_SEEK_FILE_PACK_RESULT = 7,
//     FAILED_TO_GET_DIRECTORY_PACK_RESULT = 8,
//     FAILED_TO_DECOMPRESS_PACK_RESULT = 9,
//     FAILED_TO_GET_ITEM_PACK_RESULT = 10,
//     BAD_DATA_SIZE_PACK_RESULT = 11,
//     BAD_FILE_TYPE_PACK_RESULT = 12,
//     BAD_FILE_VERSION_PACK_RESULT = 13,
//     BAD_FILE_ENDIANNESS_PACK_RESULT = 14,
//     PACK_RESULT_COUNT = 15,
// } neko_packresult_t;

typedef b32 neko_pack_result;

typedef struct pack_iteminfo {
    u32 zip_size;
    u32 data_size;
    u64 file_offset;
    u8 path_size;
} pack_iteminfo;

typedef struct pack_item {
    pack_iteminfo info;
    char* path;
} pack_item;

struct neko_packreader_s {
    FILE* file;
    u64 item_count;
    pack_item* items;
    u8* data_buffer;
    u8* zip_buffer;
    u32 data_size;
    u32 zip_size;
    pack_item search_item;
};

typedef struct neko_packreader_s neko_packreader_t;

NEKO_API_DECL neko_pack_result neko_pack_read(const_str filePath, u32 data_buffer_capacity, bool is_resources_directory, neko_packreader_t* pack_reader);
NEKO_API_DECL void neko_pack_destroy(neko_packreader_t* pack_reader);

NEKO_API_DECL u64 neko_pack_item_count(neko_packreader_t* pack_reader);
NEKO_API_DECL b8 neko_pack_item_index(neko_packreader_t* pack_reader, const_str path, u64* index);
NEKO_API_DECL u32 neko_pack_item_size(neko_packreader_t* pack_reader, u64 index);
NEKO_API_DECL const_str neko_pack_item_path(neko_packreader_t* pack_reader, u64 index);
NEKO_API_DECL neko_pack_result neko_pack_item_data_with_index(neko_packreader_t* pack_reader, u64 index, const u8** data, u32* size);
NEKO_API_DECL neko_pack_result neko_pack_item_data(neko_packreader_t* pack_reader, const_str path, const u8** data, u32* size);
NEKO_API_DECL void neko_pack_free_buffers(neko_packreader_t* pack_reader);
NEKO_API_DECL neko_pack_result neko_pack_unzip(const_str filePath, b8 printProgress);
NEKO_API_DECL neko_pack_result neko_pack_files(const_str packPath, u64 fileCount, const_str* filePaths, b8 printProgress);
NEKO_API_DECL neko_pack_result neko_pack_info(const_str filePath, u8* majorVersion, u8* minorVersion, u8* patchVersion, b8* isLittleEndian, u64* itemCount);

NEKO_API_DECL neko_inline bool neko_pack_check(neko_pack_result result) {
    if (result == 0) return true;
    neko_log_warning("read pack faild: %d", result);
    return false;
}

#endif  // NEKO_ASSET_H
