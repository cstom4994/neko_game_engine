
#ifndef NEKO_ASSET_IMPL
#define NEKO_ASSET_IMPL

#include "engine/neko_engine.h"

// Asset handle
typedef struct neko_asset_t {
    u64 type_id;
    u32 asset_id;
    u32 importer_id;  // 'Unique' id of importer, used for type safety
} neko_asset_t;

NEKO_API_DECL neko_asset_t __neko_asset_handle_create_impl(u64 type_id, u32 asset_id, u32 importer_id);

#define neko_asset_handle_create(T, ID, IMPID) __neko_asset_handle_create_impl(neko_hash_str64(NEKO_TO_STR(T)), ID, IMPID)

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

#define neko_assets_get_importerp(AM, T) (neko_hash_table_getp((AM)->importers, neko_hash_str64(NEKO_TO_STR(T))))

#ifdef __cplusplus
#define gsa_imsa(IMPORTER, T) (decltype(neko_slot_array(T))(IMPORTER)->slot_array)
#else
#define gsa_imsa(IMPORTER, T) ((neko_slot_array(T))(IMPORTER)->slot_array)
#endif

#define neko_assets_register_importer(AM, T, DESC)                                               \
    do {                                                                                         \
        neko_asset_importer_t ai = NEKO_DEFAULT_VAL();                                           \
        ai.data_size = sizeof(T);                                                                \
        ai.importer_id = (AM)->free_importer_id++;                                               \
        neko_asset_importer_set_desc(&ai, (neko_asset_importer_desc_t*)DESC);                    \
        size_t sz = 2 * sizeof(void*) + sizeof(T);                                               \
        neko_slot_array(T) sa = NULL;                                                            \
        neko_slot_array_init((void**)&sa, sizeof(*sa));                                          \
        neko_dyn_array_init((void**)&sa->indices, sizeof(u32));                                  \
        neko_dyn_array_init((void**)&sa->data, sizeof(T));                                       \
        ai.slot_array = (void*)sa;                                                               \
        ai.tmp_ptr = (void*)&sa->tmp;                                                            \
        ai.slot_array_indices_ptr = (void*)sa->indices;                                          \
        ai.slot_array_data_ptr = (void*)sa->data;                                                \
        if (!ai.desc.load_from_file) {                                                           \
            ai.desc.load_from_file = (neko_asset_load_func) & neko_asset_default_load_from_file; \
        }                                                                                        \
        neko_hash_table_insert((AM)->importers, neko_hash_str64(NEKO_TO_STR(T)), ai);            \
    } while (0)

// Need a way to be able to print upon assert
#define neko_assets_load_from_file(AM, T, PATH, ...)                                                                                                                           \
    (/*NEKO_ASSERT(neko_hash_table_key_exists((AM)->importers, neko_hash_str64(NEKO_TO_STR(T)))),*/                                                                            \
     (AM)->tmpi = neko_hash_table_getp((AM)->importers, neko_hash_str64(NEKO_TO_STR(T))), (AM)->tmpi->desc.load_from_file(PATH, (AM)->tmpi->tmp_ptr, ##__VA_ARGS__),           \
     (AM)->tmpi->tmpid = neko_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL), \
     neko_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id))

#define neko_assets_create_asset(AM, T, DATA)                                                                                                                                  \
    (/*NEKO_ASSERT(neko_hash_table_key_exists((AM)->importers, neko_hash_str64(NEKO_TO_STR(T)))),*/                                                                            \
     (AM)->tmpi = neko_hash_table_getp((AM)->importers, neko_hash_str64(NEKO_TO_STR(T))), (AM)->tmpi->tmp_ptr = (DATA),                                                        \
     (AM)->tmpi->tmpid = neko_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL), \
     neko_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id))

typedef struct neko_asset_manager_t {
    neko_hash_table(u64, neko_asset_importer_t) importers;  // Maps hashed types to importer
    neko_asset_importer_t* tmpi;                            // Temporary importer for caching
    u32 free_importer_id;
} neko_asset_manager_t;

NEKO_API_DECL neko_asset_manager_t neko_asset_manager_new();
NEKO_API_DECL void neko_asset_manager_free(neko_asset_manager_t* am);
NEKO_API_DECL void* __neko_assets_getp_impl(neko_asset_manager_t* am, u64 type_id, neko_asset_t hndl);

#define neko_assets_getp(AM, T, HNDL) (T*)(__neko_assets_getp_impl(AM, neko_hash_str64(NEKO_TO_STR(T)), HNDL))

#define neko_assets_get(AM, T, HNDL) *(neko_assets_getp(AM, T, HNDL));

/*==========================
// LZ77
==========================*/

NEKO_API_DECL u32 neko_lz_encode(const void* in, u32 inlen, void* out, u32 outlen, u32 flags);  // [0..(6)..9]
NEKO_API_DECL u32 neko_lz_decode(const void* in, u32 inlen, void* out, u32 outlen);
NEKO_API_DECL u32 neko_lz_bounds(u32 inlen, u32 flags);

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

typedef struct neko_packreader_t {
    FILE* file;
    u64 item_count;
    pack_item* items;
    u8* data_buffer;
    u8* zip_buffer;
    u32 data_size;
    u32 zip_size;
    pack_item search_item;
    u32 file_ref_count;
} neko_packreader_t;

NEKO_API_DECL int neko_pack_read(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory, neko_packreader_t* pack_reader);
NEKO_API_DECL void neko_pack_destroy(neko_packreader_t* pack_reader);

NEKO_API_DECL u64 neko_pack_item_count(neko_packreader_t* pack_reader);
NEKO_API_DECL b8 neko_pack_item_index(neko_packreader_t* pack_reader, const_str path, u64* index);
NEKO_API_DECL u32 neko_pack_item_size(neko_packreader_t* pack_reader, u64 index);
NEKO_API_DECL const_str neko_pack_item_path(neko_packreader_t* pack_reader, u64 index);
NEKO_API_DECL int neko_pack_item_data_with_index(neko_packreader_t* pack_reader, u64 index, const u8** data, u32* size);
NEKO_API_DECL int neko_pack_item_data(neko_packreader_t* pack_reader, const_str path, const u8** data, u32* size);
NEKO_API_DECL void neko_pack_item_free(neko_packreader_t* pack_reader, void* data);
NEKO_API_DECL void neko_pack_free_buffers(neko_packreader_t* pack_reader);
NEKO_API_DECL int neko_pack_unzip(const_str file_path, b8 print_progress);
NEKO_API_DECL int neko_pack_build(const_str pack_path, u64 file_count, const_str* file_paths, b8 print_progress);
NEKO_API_DECL int neko_pack_info(const_str file_path, u8* pack_version, b8* isLittleEndian, u64* item_count);

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

// 解码 utf-8 代码点并返回字符串指针
NEKO_API_DECL const char* neko_font_decode_utf8(const char* text, int* cp);

/*==========================
// Text draw
==========================*/

typedef struct neko_fontbatch_t {
    f32 font_projection[16];
    neko_render_batch_context_t* font_render;
    neko_render_batch_shader_t font_shader;
    neko_render_batch_renderable_t font_renderable;
    f32 font_scale;
    s32 font_vert_count;
    neko_font_vert_t* font_verts;
    neko_font_u64 font_tex_id;
    neko_font_t* font;
} neko_fontbatch_t;

NEKO_API_DECL void neko_fontbatch_init(neko_fontbatch_t* font_batch, const neko_vec2_t fbs, const_str img_path, char* content, int content_size);
NEKO_API_DECL void neko_fontbatch_end(neko_fontbatch_t* font_batch);
NEKO_API_DECL void neko_fontbatch_draw(neko_fontbatch_t* font_batch, const neko_vec2_t fbs, const char* text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale);

/*==========================
// Aseprite draw
==========================*/

typedef struct neko_aseprite_frame {
    s32 duration;
    f32 u0, v0, u1, v1;
} neko_aseprite_frame;

typedef struct neko_aseprite_loop {
    neko_dyn_array(s32) indices;
} neko_aseprite_loop;

typedef struct neko_aseprite {
    // neko_array<neko_sprite_frame> frames;
    neko_dyn_array(neko_aseprite_frame) frames;
    neko_hash_table(u64, neko_aseprite_loop) by_tag;
    neko_texture_t img;
    s32 width;
    s32 height;

    // #ifdef NEKO_DEBUG
    u64 mem_used;
    // #endif
} neko_aseprite;

typedef struct neko_aseprite_renderer {
    neko_aseprite* sprite;
    neko_aseprite_loop* loop;
    f32 elapsed;
    s32 current_frame;
} neko_aseprite_renderer;

NEKO_API_DECL neko_texture_t neko_aseprite_simple(const void* memory, int size);
NEKO_API_DECL bool neko_aseprite_load(neko_aseprite* spr, const_str filepath);
NEKO_API_DECL void neko_aseprite_end(neko_aseprite* spr);
NEKO_API_DECL void neko_aseprite_renderer_play(neko_aseprite_renderer* sr, const_str tag);
NEKO_API_DECL void neko_aseprite_renderer_update(neko_aseprite_renderer* sr, f32 dt);
NEKO_API_DECL void neko_aseprite_renderer_set_frame(neko_aseprite_renderer* sr, s32 frame);

#define SPRITE_SCALE 3

/*==========================
// Tiled draw
==========================*/

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    neko_handle(neko_render_texture_t) texture;
    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_t {
    tile_t* tiles;
    u32 width;
    u32 height;

    neko_color_t tint;
} layer_t;

typedef struct object_t {
    u32 id;
    s32 x, y, width, height;
    // C2_TYPE phy_type;
    // c2AABB aabb;
    // union {
    //     c2AABB box;
    //     c2Poly poly;
    // } phy;
} object_t;

typedef struct object_group_t {
    neko_dyn_array(object_t) objects;

    neko_color_t color;

    const_str name;
} object_group_t;

typedef struct map_t {
    neko_xml_document_t* doc;  // xml doc
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

NEKO_API_DECL void neko_tiled_load(map_t* map, const_str tmx_path, const_str res_path);
NEKO_API_DECL void neko_tiled_unload(map_t* map);

typedef struct neko_tiled_quad_t {
    u32 tileset_id;
    neko_handle(neko_render_texture_t) texture;
    neko_vec2 texture_size;
    neko_vec2 position;
    neko_vec2 dimentions;
    neko_vec4 rectangle;
    neko_color_t color;
    bool use_texture;
} neko_tiled_quad_t;

#define BATCH_SIZE 2048

#define IND_PER_QUAD 6

#define VERTS_PER_QUAD 4   // 一次发送多少个verts数据
#define FLOATS_PER_VERT 9  // 每个verts数据的大小

typedef struct neko_tiled_quad_list_t {
    neko_dyn_array(neko_tiled_quad_t) quad_list;  // quad 绘制队列
} neko_tiled_quad_list_t;

typedef struct neko_tiled_renderer {
    neko_handle(neko_render_vertex_buffer_t) vb;
    neko_handle(neko_render_index_buffer_t) ib;
    neko_handle(neko_render_pipeline_t) pip;
    neko_handle(neko_render_shader_t) shader;
    neko_handle(neko_render_uniform_t) u_camera;
    neko_handle(neko_render_uniform_t) u_batch_tex;
    neko_handle(neko_render_texture_t) batch_texture;         // 当前绘制所用贴图
    neko_hash_table(u32, neko_tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    neko_mat4 camera_mat;
} neko_tiled_renderer;

NEKO_API_DECL void neko_tiled_render_init(neko_command_buffer_t* cb, neko_tiled_renderer* renderer, const_str vert_src, const_str frag_src);
NEKO_API_DECL void neko_tiled_render_deinit(neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_begin(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_flush(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);
NEKO_API_DECL void neko_tiled_render_push(neko_command_buffer_t* cb, neko_tiled_renderer* renderer, neko_tiled_quad_t quad);
NEKO_API_DECL void neko_tiled_render_draw(neko_command_buffer_t* cb, neko_tiled_renderer* renderer);

typedef struct ase_t ase_t;

NEKO_API_DECL ase_t* neko_aseprite_load_from_file(const char* path);
NEKO_API_DECL ase_t* neko_aseprite_load_from_memory(const void* memory, int size);
NEKO_API_DECL void neko_aseprite_free(ase_t* aseprite);
NEKO_API_DECL void neko_aseprite_default_blend_bind(ase_t* aseprite);  // 默认图块混合管线

#define __NEKO_ASEPRITE_MAX_LAYERS (64)
#define __NEKO_ASEPRITE_MAX_SLICES (128)
#define __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES (1024)
#define __NEKO_ASEPRITE_MAX_TAGS (256)

typedef struct ase_frame_t ase_frame_t;
typedef struct ase_layer_t ase_layer_t;
typedef struct ase_cel_t ase_cel_t;
typedef struct ase_tag_t ase_tag_t;
typedef struct ase_slice_t ase_slice_t;
typedef struct ase_palette_entry_t ase_palette_entry_t;
typedef struct ase_palette_t ase_palette_t;
typedef struct ase_udata_t ase_udata_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;
typedef struct ase_color_profile_t ase_color_profile_t;
typedef struct ase_fixed_t ase_fixed_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;

struct ase_fixed_t {
    u16 a;
    u16 b;
};

struct ase_udata_t {
    int has_color;
    neko_color_t color;
    int has_text;
    const char* text;
};

typedef enum ase_layer_flags_t {
    NEKO_ASE_LAYER_FLAGS_VISIBLE = 0x01,
    NEKO_ASE_LAYER_FLAGS_EDITABLE = 0x02,
    NEKO_ASE_LAYER_FLAGS_LOCK_MOVEMENT = 0x04,
    NEKO_ASE_LAYER_FLAGS_BACKGROUND = 0x08,
    NEKO_ASE_LAYER_FLAGS_PREFER_LINKED_CELS = 0x10,
    NEKO_ASE_LAYER_FLAGS_COLLAPSED = 0x20,
    NEKO_ASE_LAYER_FLAGS_REFERENCE = 0x40,
} ase_layer_flags_t;

typedef enum ase_layer_type_t {
    NEKO_ASE_LAYER_TYPE_NORMAL,
    NEKO_ASE_LAYER_TYPE_GROUP,
    NEKO_ASE_LAYER_TYPE_TILEMAP,
} ase_layer_type_t;

struct ase_layer_t {
    ase_layer_flags_t flags;
    ase_layer_type_t type;
    const char* name;
    ase_layer_t* parent;
    float opacity;
    ase_udata_t udata;
};

struct ase_cel_extra_chunk_t {
    int precise_bounds_are_set;
    ase_fixed_t precise_x;
    ase_fixed_t precise_y;
    ase_fixed_t w, h;
};

struct ase_cel_t {
    ase_layer_t* layer;
    void* cel_pixels;  // 图块的pixels数据是唯一的
    int w, h;
    int x, y;
    float opacity;
    int is_linked;
    u16 linked_frame_index;
    int has_extra;
    ase_cel_extra_chunk_t extra;
    ase_udata_t udata;
};

struct ase_frame_t {
    ase_t* ase;
    int duration_milliseconds;
    int pixel_count;
    neko_color_t* pixels[__NEKO_ASEPRITE_MAX_LAYERS];  // 支持每层一个pixels数据
    int cel_count;
    ase_cel_t cels[__NEKO_ASEPRITE_MAX_LAYERS];
};

typedef enum ase_animation_direction_t {
    NEKO_ASE_ANIMATION_DIRECTION_FORWARDS,
    NEKO_ASE_ANIMATION_DIRECTION_BACKWORDS,
    NEKO_ASE_ANIMATION_DIRECTION_PINGPONG,
} ase_animation_direction_t;

struct ase_tag_t {
    int from_frame;
    int to_frame;
    ase_animation_direction_t loop_animation_direction;
    int repeat;
    u8 r, g, b;
    const char* name;
    ase_udata_t udata;
};

struct ase_slice_t {
    const char* name;
    int frame_number;
    int origin_x;
    int origin_y;
    int w, h;

    int has_center_as_9_slice;
    int center_x;
    int center_y;
    int center_w;
    int center_h;

    int has_pivot;
    int pivot_x;
    int pivot_y;

    ase_udata_t udata;
};

struct ase_palette_entry_t {
    neko_color_t color;
    const char* color_name;
};

struct ase_palette_t {
    int entry_count;
    ase_palette_entry_t entries[__NEKO_ASEPRITE_MAX_PALETTE_ENTRIES];
};

typedef enum ase_color_profile_type_t {
    NEKO_ASE_COLOR_PROFILE_TYPE_NONE,
    NEKO_ASE_COLOR_PROFILE_TYPE_SRGB,
    NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC,
} ase_color_profile_type_t;

struct ase_color_profile_t {
    ase_color_profile_type_t type;
    int use_fixed_gamma;
    ase_fixed_t gamma;
    u32 icc_profile_data_length;
    void* icc_profile_data;
};

typedef enum ase_mode_t { NEKO_ASE_MODE_RGBA, NEKO_ASE_MODE_GRAYSCALE, NEKO_ASE_MODE_INDEXED } ase_mode_t;

struct ase_t {
    ase_mode_t mode;
    int w, h;
    int transparent_palette_entry_index;
    int number_of_colors;
    int pixel_w;
    int pixel_h;
    int grid_x;
    int grid_y;
    int grid_w;
    int grid_h;
    int has_color_profile;
    ase_color_profile_t color_profile;
    ase_palette_t palette;

    int layer_count;
    ase_layer_t layers[__NEKO_ASEPRITE_MAX_LAYERS];

    int frame_count;
    ase_frame_t* frames;

    int tag_count;
    ase_tag_t tags[__NEKO_ASEPRITE_MAX_TAGS];

    int slice_count;
    ase_slice_t slices[__NEKO_ASEPRITE_MAX_SLICES];
};

NEKO_INLINE int s_mul_un8(int a, int b) {
    int t = (a * b) + 0x80;
    return (((t >> 8) + t) >> 8);
}

NEKO_INLINE neko_color_t s_blend(neko_color_t src, neko_color_t dst, u8 opacity) {
    src.a = (u8)s_mul_un8(src.a, opacity);
    int a = src.a + dst.a - s_mul_un8(src.a, dst.a);
    int r, g, b;
    if (a == 0) {
        r = g = b = 0;
    } else {
        r = dst.r + (src.r - dst.r) * src.a / a;
        g = dst.g + (src.g - dst.g) * src.a / a;
        b = dst.b + (src.b - dst.b) * src.a / a;
    }
    neko_color_t ret = {(u8)r, (u8)g, (u8)b, (u8)a};
    return ret;
}

NEKO_INLINE neko_color_t s_color(ase_t* ase, void* src, int index) {
    neko_color_t result;
    if (ase->mode == NEKO_ASE_MODE_RGBA) {
        result = ((neko_color_t*)src)[index];
    } else if (ase->mode == NEKO_ASE_MODE_GRAYSCALE) {
        u8 saturation = ((u8*)src)[index * 2];
        u8 a = ((u8*)src)[index * 2 + 1];
        result.r = result.g = result.b = saturation;
        result.a = a;
    } else {
        NEKO_ASSERT(ase->mode == NEKO_ASE_MODE_INDEXED);
        u8 palette_index = ((u8*)src)[index];
        if (palette_index == ase->transparent_palette_entry_index) {
            result = neko_color_ctor(0, 0, 0, 0);
        } else {
            result = ase->palette.entries[palette_index].color;
        }
    }
    return result;
}

typedef struct neko_image_t {
    int w;
    int h;
    unsigned char* pix;
} neko_image_t;

NEKO_API_DECL neko_image_t neko_image_load(const_str path);
NEKO_API_DECL void neko_image_free(neko_image_t img);

#endif  // NEKO_ASSET_H
