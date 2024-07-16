#pragma once

#include <stb_truetype.h>

#include <optional>

#include "neko_base.h"

struct Image {
    u32 id;
    i32 width;
    i32 height;
    bool has_mips;

    bool load(String filepath, bool generate_mips);
    void trash();
};

struct SpriteFrame {
    i32 duration;
    float u0, v0, u1, v1;
};

struct SpriteLoop {
    Slice<i32> indices;
};

struct SpriteData {
    Arena arena;
    Slice<SpriteFrame> frames;
    HashMap<SpriteLoop> by_tag;
    Image img;
    i32 width;
    i32 height;

    bool load(String filepath);
    void trash();
};

struct Sprite {
    u64 sprite;  // index into assets
    u64 loop;    // index into SpriteData::by_tag
    float elapsed;
    i32 current_frame;

    bool play(String tag);
    void update(float dt);
    void set_frame(i32 frame);
};

struct SpriteView {
    Sprite *sprite;
    SpriteData data;
    SpriteLoop loop;

    bool make(Sprite *spr);
    i32 frame();
    u64 len();
};

struct Tile {
    float x, y, u, v;
    float u0, v0, u1, v1;
    i32 flip_bits;
};

struct TilemapEntity {
    String identifier;
    float x, y;
};

using TilemapInt = unsigned char;

struct TilemapLayer {
    String identifier;
    Image image;
    Slice<Tile> tiles;
    Slice<TilemapEntity> entities;
    i32 c_width;
    i32 c_height;
    Slice<TilemapInt> int_grid;
    float grid_size;
};

struct TilemapLevel {
    String identifier;
    String iid;
    float world_x, world_y;
    float px_width, px_height;
    Slice<TilemapLayer> layers;
};

enum TileNodeFlags {
    TileNodeFlags_Open = 1 << 0,
    TileNodeFlags_Closed = 1 << 1,
};

struct TileNode {
    TileNode *prev;
    float g;  // cost so far
    u32 flags;

    i32 x, y;
    float cost;
    Slice<TileNode *> neighbors;
};

struct TileCost {
    TilemapInt cell;
    float value;
};

struct TilePoint {
    float x, y;
};

inline u64 tile_key(i32 x, i32 y) { return ((u64)x << 32) | (u64)y; }

class b2Body;
class b2World;

struct map_ldtk {
    Arena arena;
    Slice<TilemapLevel> levels;
    HashMap<Image> images;     // key: filepath
    HashMap<b2Body *> bodies;  // key: layer name
    HashMap<TileNode> graph;   // key: x, y
    PriorityQueue<TileNode *> frontier;
    float graph_grid_size;

    bool load(String filepath);
    void trash();
    void destroy_bodies(b2World *world);
    void make_collision(b2World *world, float meter, String layer_name, Slice<TilemapInt> walls);
    void make_graph(i32 bloom, String layer_name, Slice<TileCost> costs);
    TileNode *astar(TilePoint start, TilePoint goal);
};

struct FontRange {
    stbtt_bakedchar chars[256];
    Image image;
};

struct FontQuad {
    stbtt_aligned_quad quad;
};

struct FontFamily {
    String ttf;
    HashMap<FontRange> ranges;
    StringBuilder sb;

    bool load(String filepath);
    void load_default();
    void trash();

    stbtt_aligned_quad quad(u32 *img, float *x, float *y, float size, i32 ch);
    float width(float size, String text);
};

struct AtlasImage {
    float u0, v0, u1, v1;
    float width;
    float height;
    Image img;
};

struct Atlas {
    HashMap<AtlasImage> by_name;
    Image img;

    bool load(String filepath, bool generate_mips);
    void trash();
    AtlasImage *get(String name);
};

struct MountResult {
    bool ok;
    bool can_hot_reload;
    bool is_fused;
};

MountResult vfs_mount(const_str fsname, const char *filepath);
void vfs_fini(std::optional<String> name);

bool vfs_file_exists(String fsname, String filepath);
bool vfs_read_entire_file(String fsname, String *out, String filepath);
bool vfs_write_entire_file(String fsname, String filepath, String contents);
bool vfs_list_all_files(String fsname, Array<String> *files);

void *vfs_for_miniaudio();

typedef struct vfs_file {
    const_str data;
    size_t len;
    u64 offset;
} vfs_file;

size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf);
int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence);
u64 neko_capi_vfs_ftell(vfs_file *vf);
vfs_file neko_capi_vfs_fopen(const_str path);
int neko_capi_vfs_fclose(vfs_file *vf);

bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);
const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size);

enum JSONKind : i32 {
    JSONKind_Null,
    JSONKind_Object,
    JSONKind_Array,
    JSONKind_String,
    JSONKind_Number,
    JSONKind_Boolean,
};

struct JSONObject;
struct JSONArray;
struct JSON {
    union {
        JSONObject *object;
        JSONArray *array;
        String string;
        double number;
        bool boolean;
    };
    JSONKind kind;

    JSON lookup(String key, bool *ok);
    JSON index(i32 i, bool *ok);

    JSONObject *as_object(bool *ok);
    JSONArray *as_array(bool *ok);
    String as_string(bool *ok);
    double as_number(bool *ok);

    JSONObject *lookup_object(String key, bool *ok);
    JSONArray *lookup_array(String key, bool *ok);
    String lookup_string(String key, bool *ok);
    double lookup_number(String key, bool *ok);

    double index_number(i32 i, bool *ok);
};

struct JSONObject {
    JSON value;
    String key;
    JSONObject *next;
    u64 hash;
};

struct JSONArray {
    JSON value;
    JSONArray *next;
    u64 index;
};

struct JSONDocument {
    JSON root;
    String error;
    Arena arena;

    void parse(String contents);
    void trash();
};

struct StringBuilder;
void json_write_string(StringBuilder *sb, JSON *json);
void json_print(JSON *json);

struct lua_State;
void json_to_lua(lua_State *L, JSON *json);
String lua_to_json_string(lua_State *L, i32 arg, String *contents, i32 width);

/*==========================
// LZ77
==========================*/

u32 neko_lz_encode(const void *in, u32 inlen, void *out, u32 outlen, u32 flags);  // [0..(6)..9]
u32 neko_lz_decode(const void *in, u32 inlen, void *out, u32 outlen);
u32 neko_lz_bounds(u32 inlen, u32 flags);

/*==========================
// NEKO_PACK
==========================*/

#define neko_pak_head_size 8

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

typedef struct neko_pak {

    typedef struct {
        u32 zip_size;
        u32 data_size;
        u64 file_offset;
        u8 path_size;
    } iteminfo;

    typedef struct {
        iteminfo info;
        const_str path;
    } item;

    vfs_file vf;
    u64 item_count;
    item *items;
    u8 *data_buffer;
    u8 *zip_buffer;
    u32 data_size;
    u32 zip_size;
    item search_item;
    u32 file_ref_count;

    bool load(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory);
    void fini();

    inline u64 get_item_count() const { return this->item_count; }

    inline u32 get_item_size(u64 index) {
        NEKO_ASSERT(index < this->item_count);
        return this->items[index].info.data_size;
    }

    inline const_str get_item_path(u64 index) {
        NEKO_ASSERT(index < this->item_count);
        return this->items[index].path;
    }

    u64 get_item_index(const_str path);

    bool get_data(u64 index, const u8 **data, u32 *size);
    bool get_data(const_str path, const u8 **data, u32 *size);
    void free_item(void *data);

    void free_buffer();

} neko_pak;

bool neko_pak_unzip(const_str file_path, bool print_progress);
bool neko_pak_build(const_str pack_path, u64 file_count, const_str *file_paths, bool print_progress);
bool neko_pak_info(const_str file_path, u8 *pack_version, bool *isLittleEndian, u64 *item_count);

typedef enum neko_xml_attribute_type_t {
    NEKO_XML_ATTRIBUTE_NUMBER,
    NEKO_XML_ATTRIBUTE_BOOLEAN,
    NEKO_XML_ATTRIBUTE_STRING,
} neko_xml_attribute_type_t;

typedef struct neko_xml_attribute_t {
    const_str name;
    neko_xml_attribute_type_t type;

    union {
        double number;
        bool boolean;
        const_str string;
    } value;
} neko_xml_attribute_t;

// neko_hash_table_decl(u64, neko_xml_attribute_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_xml_node_t {
    const_str name;
    const_str text;

    neko_hash_table(u64, neko_xml_attribute_t) attributes;
    neko_dyn_array(struct neko_xml_node_t) children;

} neko_xml_node_t;

typedef struct neko_xml_document_t {
    neko_dyn_array(neko_xml_node_t) nodes;
} neko_xml_document_t;

typedef struct neko_xml_node_iter_t {
    neko_xml_document_t *doc;
    neko_xml_node_t *node;
    const_str name;
    u32 idx;

    neko_xml_node_t *current;
} neko_xml_node_iter_t;

neko_xml_document_t *neko_xml_parse(const_str source);
neko_xml_document_t *neko_xml_parse_file(const_str path);
void neko_xml_free(neko_xml_document_t *document);

neko_xml_attribute_t *neko_xml_find_attribute(neko_xml_node_t *node, const_str name);
neko_xml_node_t *neko_xml_find_node(neko_xml_document_t *doc, const_str name);
neko_xml_node_t *neko_xml_find_node_child(neko_xml_node_t *node, const_str name);

const_str neko_xml_get_error();

neko_xml_node_iter_t neko_xml_new_node_iter(neko_xml_document_t *doc, const_str name);
neko_xml_node_iter_t neko_xml_new_node_child_iter(neko_xml_node_t *node, const_str name);
bool neko_xml_node_iter_next(neko_xml_node_iter_t *iter);

typedef struct ase_t ase_t;

ase_t *neko_aseprite_load_from_memory(const void *memory, int size);
void neko_aseprite_free(ase_t *aseprite);

#define NEKO_ASEPRITE_MAX_LAYERS (64)
#define NEKO_ASEPRITE_MAX_SLICES (128)
#define NEKO_ASEPRITE_MAX_PALETTE_ENTRIES (1024)
#define NEKO_ASEPRITE_MAX_TAGS (256)

typedef struct ase_color_t ase_color_t;
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

struct ase_color_t {
    uint8_t r, g, b, a;
};

struct ase_fixed_t {
    uint16_t a;
    uint16_t b;
};

struct ase_udata_t {
    int has_color;
    ase_color_t color;
    int has_text;
    const char *text;
};

typedef enum ase_layer_flags_t {
    ASE_LAYER_FLAGS_VISIBLE = 0x01,
    ASE_LAYER_FLAGS_EDITABLE = 0x02,
    ASE_LAYER_FLAGS_LOCK_MOVEMENT = 0x04,
    ASE_LAYER_FLAGS_BACKGROUND = 0x08,
    ASE_LAYER_FLAGS_PREFER_LINKED_CELS = 0x10,
    ASE_LAYER_FLAGS_COLLAPSED = 0x20,
    ASE_LAYER_FLAGS_REFERENCE = 0x40,
} ase_layer_flags_t;

typedef enum ase_layer_type_t {
    ASE_LAYER_TYPE_NORMAL,
    ASE_LAYER_TYPE_GROUP,
} ase_layer_type_t;

struct ase_layer_t {
    ase_layer_flags_t flags;
    ase_layer_type_t type;
    const char *name;
    ase_layer_t *parent;
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
    ase_layer_t *layer;
    void *pixels;
    int w, h;
    int x, y;
    float opacity;
    int is_linked;
    uint16_t linked_frame_index;
    int has_extra;
    ase_cel_extra_chunk_t extra;
    ase_udata_t udata;
};

struct ase_frame_t {
    ase_t *ase;
    int duration_milliseconds;
    ase_color_t *pixels;
    int cel_count;
    ase_cel_t cels[NEKO_ASEPRITE_MAX_LAYERS];
};

typedef enum ase_animation_direction_t {
    ASE_ANIMATION_DIRECTION_FORWARDS,
    ASE_ANIMATION_DIRECTION_BACKWORDS,
    ASE_ANIMATION_DIRECTION_PINGPONG,
} ase_animation_direction_t;

struct ase_tag_t {
    int from_frame;
    int to_frame;
    ase_animation_direction_t loop_animation_direction;
    int repeat;
    uint8_t r, g, b;
    const char *name;
    ase_udata_t udata;
};

struct ase_slice_t {
    const char *name;
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
    ase_color_t color;
    const char *color_name;
};

struct ase_palette_t {
    int entry_count;
    ase_palette_entry_t entries[NEKO_ASEPRITE_MAX_PALETTE_ENTRIES];
};

typedef enum ase_color_profile_type_t {
    ASE_COLOR_PROFILE_TYPE_NONE,
    ASE_COLOR_PROFILE_TYPE_SRGB,
    ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC,
} ase_color_profile_type_t;

struct ase_color_profile_t {
    ase_color_profile_type_t type;
    int use_fixed_gamma;
    ase_fixed_t gamma;
    uint32_t icc_profile_data_length;
    void *icc_profile_data;
};

typedef enum ase_mode_t { ASE_MODE_RGBA, ASE_MODE_GRAYSCALE, ASE_MODE_INDEXED } ase_mode_t;

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
    ase_layer_t layers[NEKO_ASEPRITE_MAX_LAYERS];

    int frame_count;
    ase_frame_t *frames;

    int tag_count;
    ase_tag_t tags[NEKO_ASEPRITE_MAX_TAGS];

    int slice_count;
    ase_slice_t slices[NEKO_ASEPRITE_MAX_SLICES];
};

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_Sprite,
    AssetKind_Tilemap,
};

struct AssetLoadData {
    AssetKind kind;
    bool generate_mips;
};

struct Asset {
    String name;
    u64 hash;
    u64 modtime;
    AssetKind kind;
    union {
        i32 lua_ref;
        Image image;
        SpriteData sprite;
        map_ldtk tilemap;
    };
};

void assets_shutdown();
void assets_start_hot_reload();
void assets_perform_hot_reload_changes();

bool asset_load_kind(AssetKind kind, String filepath, Asset *out);
bool asset_load(AssetLoadData desc, String filepath, Asset *out);

bool asset_read(u64 key, Asset *out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State *L, u64 key);
Asset check_asset_mt(lua_State *L, i32 arg, const char *mt);
