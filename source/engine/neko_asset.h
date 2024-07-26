#pragma once

#include <stb_truetype.h>

#include <optional>

#include "neko_base.h"
#include "neko_math.h"

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

bool read_entire_file_raw(String *out, String filepath);

MountResult vfs_mount(const_str fsname, const char *filepath);
void vfs_fini();

u64 vfs_file_modtime(String filepath);
bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String *out, String filepath);
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
// NEKO_PACK
==========================*/

#define NEKO_PAK_HEAD_SIZE 8

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

    bool get_data(u64 index, String *out, u32 *size);
    bool get_data(const_str path, String *out, u32 *size);
    void free_item(String data);

    void free_buffer();

} neko_pak;

bool neko_pak_unzip(const_str file_path, bool print_progress);
bool neko_pak_build(const_str pack_path, u64 file_count, const_str *file_paths, bool print_progress);
bool neko_pak_info(const_str file_path, i32 *buildnum, u64 *item_count);

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

/*==========================
// Tiled draw
==========================*/

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    Image texture;

    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_t {
    tile_t *tiles;
    u32 width;
    u32 height;

    Color tint;
} layer_t;

typedef struct object_t {
    u32 id;
    i32 x, y, width, height;
    // C2_TYPE phy_type;
    // c2AABB aabb;
    // union {
    //     c2AABB box;
    //     c2Poly poly;
    // } phy;
} object_t;

typedef struct object_group_t {
    neko_dyn_array(object_t) objects;

    Color color;

    const_str name;
} object_group_t;

typedef struct map_t {
    neko_xml_document_t *doc;  // xml doc
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

void neko_tiled_load(map_t *map, const_str tmx_path, const_str res_path);
void neko_tiled_unload(map_t *map);

typedef struct neko_tiled_quad_t {
    u32 tileset_id;
    Image texture;
    neko_vec2 texture_size;
    neko_vec2 position;
    neko_vec2 dimentions;
    neko_vec4 rectangle;
    Color color;
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
    // neko_handle(neko_render_vertex_buffer_t) vb;
    // neko_handle(neko_render_index_buffer_t) ib;
    // neko_handle(neko_render_pipeline_t) pip;
    // neko_handle(neko_render_shader_t) shader;
    // neko_handle(neko_render_uniform_t) u_camera;
    // neko_handle(neko_render_uniform_t) u_batch_tex;
    // neko_handle(neko_render_texture_t) batch_texture;         // 当前绘制所用贴图
    neko_hash_table(u32, neko_tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    Matrix4 camera_mat;
} neko_tiled_renderer;

void neko_tiled_render_init(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, const_str vert_src, const_str frag_src);
void neko_tiled_render_deinit(neko_tiled_renderer *renderer);
void neko_tiled_render_begin(neko_command_buffer_t *cb, neko_tiled_renderer *renderer);
void neko_tiled_render_flush(neko_command_buffer_t *cb, neko_tiled_renderer *renderer);
void neko_tiled_render_push(neko_command_buffer_t *cb, neko_tiled_renderer *renderer, neko_tiled_quad_t quad);
void neko_tiled_render_draw(neko_command_buffer_t *cb, neko_tiled_renderer *renderer);

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_Sprite,
    AssetKind_Tilemap,
    // AssetKind_Pak,
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
        // neko_pak pak;
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
