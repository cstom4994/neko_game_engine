
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

NEKO_API_DECL u32 neko_lz_encode(const void* in, u32 inlen, void* out, u32 outlen, u32 flags);  // [0..(6)..9]
NEKO_API_DECL u32 neko_lz_decode(const void* in, u32 inlen, void* out, u32 outlen);
NEKO_API_DECL u32 neko_lz_bounds(u32 inlen, u32 flags);

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
NEKO_API_DECL neko_pack_result neko_pack_info(const_str filePath, u8* pack_version, b8* isLittleEndian, u64* itemCount);

neko_inline bool neko_pack_check(neko_pack_result result) {
    if (result == 0) return true;
    neko_log_warning("read pack faild: %d", result);
    return false;
}

typedef enum neko_xml_attribute_type_t {
    NEKO_XML_ATTRIBUTE_NUMBER,
    NEKO_XML_ATTRIBUTE_BOOLEAN,
    NEKO_XML_ATTRIBUTE_STRING,
} neko_xml_attribute_type_t;

typedef struct neko_xml_attribute_t {
    char* name;
    neko_xml_attribute_type_t type;

    union {
        double number;
        bool boolean;
        char* string;
    } value;
} neko_xml_attribute_t;

// neko_hash_table_decl(u64, neko_xml_attribute_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_xml_node_t {
    char* name;
    char* text;

    neko_hash_table(u64, neko_xml_attribute_t) attributes;
    neko_dyn_array(struct neko_xml_node_t) children;

} neko_xml_node_t;

typedef struct neko_xml_document_t {
    neko_dyn_array(neko_xml_node_t) nodes;
} neko_xml_document_t;

typedef struct neko_xml_node_iter_t {
    neko_xml_document_t* doc;
    neko_xml_node_t* node;
    const_str name;
    u32 idx;

    neko_xml_node_t* current;
} neko_xml_node_iter_t;

NEKO_API_DECL neko_xml_document_t* neko_xml_parse(const_str source);
NEKO_API_DECL neko_xml_document_t* neko_xml_parse_file(const_str path);
NEKO_API_DECL void neko_xml_free(neko_xml_document_t* document);

NEKO_API_DECL neko_xml_attribute_t* neko_xml_find_attribute(neko_xml_node_t* node, const_str name);
NEKO_API_DECL neko_xml_node_t* neko_xml_find_node(neko_xml_document_t* doc, const_str name);
NEKO_API_DECL neko_xml_node_t* neko_xml_find_node_child(neko_xml_node_t* node, const_str name);

NEKO_API_DECL const_str neko_xml_get_error();

NEKO_API_DECL neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t* doc, const_str name);
NEKO_API_DECL neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t* node, const_str name);
NEKO_API_DECL bool neko_xml_node_iter_next(neko_xml_node_iter_t* iter);

/*==========================
// NEKO_NBT
==========================*/

// Based on https://wiki.vg/NBT (Named binary tag be used in Minecraft)

#define NEKO_NBT_BUFFER_SIZE 32768

typedef enum {
    NBT_TYPE_END,
    NBT_TYPE_BYTE,
    NBT_TYPE_SHORT,
    NBT_TYPE_INT,
    NBT_TYPE_LONG,
    NBT_TYPE_FLOAT,
    NBT_TYPE_DOUBLE,
    NBT_TYPE_BYTE_ARRAY,
    NBT_TYPE_STRING,
    NBT_TYPE_LIST,
    NBT_TYPE_COMPOUND,
    NBT_TYPE_INT_ARRAY,
    NBT_TYPE_LONG_ARRAY,
    NBT_NO_OVERRIDE
} neko_nbt_tag_type_t;

typedef struct neko_nbt_tag_t neko_nbt_tag_t;

struct neko_nbt_tag_t {

    neko_nbt_tag_type_t type;

    char* name;
    size_t name_size;

    union {
        struct {
            s8 value;
        } tag_byte;
        struct {
            s16 value;
        } tag_short;
        struct {
            s32 value;
        } tag_int;
        struct {
            s64 value;
        } tag_long;
        struct {
            float value;
        } tag_float;
        struct {
            double value;
        } tag_double;
        struct {
            s8* value;
            size_t size;
        } tag_byte_array;
        struct {
            char* value;
            size_t size;
        } tag_string;
        struct {
            neko_nbt_tag_t** value;
            neko_nbt_tag_type_t type;
            size_t size;
        } tag_list;
        struct {
            neko_nbt_tag_t** value;
            size_t size;
        } tag_compound;
        struct {
            s32* value;
            size_t size;
        } tag_int_array;
        struct {
            s64* value;
            size_t size;
        } tag_long_array;
    };
};

typedef struct {
    size_t (*read)(void* userdata, u8* data, size_t size);
    void* userdata;
} neko_nbt_reader_t;

typedef struct {
    size_t (*write)(void* userdata, u8* data, size_t size);
    void* userdata;
} neko_nbt_writer_t;

typedef enum {
    NBT_PARSE_FLAG_USE_RAW = 1,
} neko_nbt_parse_flags_t;

typedef enum { NBT_WRITE_FLAG_USE_RAW = 1 } neko_nbt_write_flags_t;

NEKO_API_DECL neko_nbt_tag_t* neko_nbt_parse(neko_nbt_reader_t reader, int parse_flags);
NEKO_API_DECL void neko_nbt_write(neko_nbt_writer_t writer, neko_nbt_tag_t* tag, int write_flags);

NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_byte(s8 value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_short(s16 value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_int(s32 value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_long(s64 value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_float(float value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_double(double value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_byte_array(s8* value, size_t size);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_string(const char* value, size_t size);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_list(neko_nbt_tag_type_t type);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_compound(void);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_int_array(s32* value, size_t size);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_new_tag_long_array(s64* value, size_t size);

NEKO_API_DECL void neko_nbt_set_tag_name(neko_nbt_tag_t* tag, const char* name, size_t size);

NEKO_API_DECL void neko_nbt_tag_list_append(neko_nbt_tag_t* list, neko_nbt_tag_t* value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_tag_list_get(neko_nbt_tag_t* tag, size_t index);
NEKO_API_DECL void neko_nbt_tag_compound_append(neko_nbt_tag_t* compound, neko_nbt_tag_t* value);
NEKO_API_DECL neko_nbt_tag_t* neko_nbt_tag_compound_get(neko_nbt_tag_t* tag, const char* key);

NEKO_API_DECL void neko_nbt_free_tag(neko_nbt_tag_t* tag);

NEKO_API_DECL neko_nbt_tag_t* neko_read_nbt_file_default(const char* name, int flags);
NEKO_API_DECL void neko_write_nbt_file_default(const char* name, neko_nbt_tag_t* tag, int flags);

#define neko_nbt_readfile neko_read_nbt_file_default
#define neko_nbt_writefile neko_write_nbt_file_default

NEKO_API_DECL void neko_nbt_print_tree(neko_nbt_tag_t* tag, int indentation);

/*==========================
// NEKO_FONT
==========================*/

typedef u64 neko_font_u64;

extern const char* neko_font_error_reason;

typedef struct neko_font_glyph_t {
    float minx, miny;
    float maxx, maxy;
    float w, h;
    int xoffset, yoffset;
    int xadvance;
} neko_font_glyph_t;

typedef struct neko_font_t {
    int font_height;
    int glyph_count;
    neko_font_glyph_t* glyphs;
    int* codes;
    int atlas_w;
    int atlas_h;
    neko_font_u64 atlas_id;
    struct neko_font_kern_t* kern;
    void* mem_ctx;
} neko_font_t;

NEKO_API_DECL neko_font_t* neko_font_load_ascii(neko_font_u64 atlas_id, const void* pixels, int w, int h, int stride, void* mem_ctx);
NEKO_API_DECL neko_font_t* neko_font_load_1252(neko_font_u64 atlas_id, const void* pixels, int w, int h, int stride, void* mem_ctx);
NEKO_API_DECL neko_font_t* neko_font_load_bmfont(neko_font_u64 atlas_id, const void* fnt, int size, void* mem_ctx);
NEKO_API_DECL void neko_font_free(neko_font_t* font);

NEKO_API_DECL int neko_font_text_width(neko_font_t* font, const char* text);
NEKO_API_DECL int neko_font_text_height(neko_font_t* font, const char* text);
NEKO_API_DECL int neko_font_max_glyph_height(neko_font_t* font, const char* text);

NEKO_API_DECL int neko_font_get_glyph_index(neko_font_t* font, int code);            // returns run-time glyph index associated with a utf32 codepoint (unicode)
NEKO_API_DECL neko_font_glyph_t* neko_font_get_glyph(neko_font_t* font, int index);  // returns a glyph, given run-time glyph index
NEKO_API_DECL int neko_font_kerning(neko_font_t* font, int code0, int code1);

// Here just in case someone wants to load up a custom file format.
NEKO_API_DECL neko_font_t* neko_font_create_blank(int font_height, int glyph_count);
NEKO_API_DECL void neko_font_add_kerning_pair(neko_font_t* font, int code0, int code1, int kerning);

typedef struct neko_font_vert_t {
    float x, y;
    float u, v;
} neko_font_vert_t;

typedef struct neko_font_rect_t {
    float left;
    float right;
    float top;
    float bottom;
} neko_font_rect_t;

// Fills in an array of triangles, two triangles for each quad, one quad for each text glyph.
// Will return 0 if the function tries to overrun the vertex buffer. Quads are setup in 2D where
// the y axis points up, x axis points left. The top left of the first glyph is placed at the
// coordinate {`x`, `y`}. Newlines move quads downward by the text height added with `line_height`.
// `count_written` contains the number of outputted vertices. `wrap_w` is used for word wrapping if
// positive, and ignored if negative. `clip_rect`, if not NULL, will be used to perform CPU-side
// clipping to make sure quads are only output within the `clip_rect` bounding box. Clipping is
// useful to implement scrollable text, and keep multiple different text instances within a single
// vertex buffer (to reduce draw calls), as opposed to using a GPU-side scissor box, which would
// require a different draw call for each scissor.
NEKO_API_DECL int neko_font_fill_vertex_buffer(neko_font_t* font, const char* text, float x, float y, float wrap_w, float line_height, neko_font_rect_t* clip_rect, neko_font_vert_t* buffer,
                                               int buffer_max, int* count_written);

// Decodes a utf8 codepoint and returns the advanced string pointer.
NEKO_API_DECL const char* neko_font_decode_utf8(const char* text, int* cp);

#endif  // NEKO_ASSET_H
