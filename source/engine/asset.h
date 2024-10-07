#ifndef NEKO_ASSET_H
#define NEKO_ASSET_H

#include <bitset>

#include "engine/base.hpp"
#include "engine/base/color.hpp"
#include "engine/ecs/entity.h"
#include "engine/event.h"
#include "engine/graphics.h"

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
vec2 texture_get_size(const char* filename);  // (width, height)
AssetTexture texture_get_ptr(const char* filename);
bool texture_update(AssetTexture* tex, String filename);
bool texture_update_data(AssetTexture* tex, u8* data);

u64 generate_texture_handle(void* pixels, int w, int h, void* udata);
void destroy_texture_handle(u64 texture_id, void* udata);

gfx_texture_t neko_aseprite_simple(String filename);

typedef struct neko_asset_texture_t {
    neko_handle(gfx_texture_t) hndl;
    gfx_texture_desc_t desc;
} neko_asset_texture_t;

// AssetTexture

bool load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

// #if 1
// bool neko_asset_texture_load_from_file(const_str path, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// #endif

#if 0

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

#endif

struct AseSpriteFrame {
    i32 duration;
    float u0, v0, u1, v1;
};

struct AseSpriteLoop {
    Slice<i32> indices;
};

struct AseSpriteData {
    Arena arena;
    Slice<AseSpriteFrame> frames;
    HashMap<AseSpriteLoop> by_tag;
    gfx_texture_t tex;
    i32 width;
    i32 height;

    bool load(String filepath);
    void trash();
};

struct AseSprite {
    u64 sprite;  // index into assets
    u64 loop;    // index into AseSpriteData::by_tag
    float elapsed;
    i32 current_frame;

    std::bitset<3> effects;

    void make();
    bool play(String tag);
    void update(float dt);
    void set_frame(i32 frame);
};

struct AseSpriteView {
    AseSprite* sprite;
    AseSpriteData data;
    AseSpriteLoop loop;

    bool make(AseSprite* spr);
    i32 frame();
    u64 len();
};

struct MountResult {
    bool ok;
    bool can_hot_reload;
    bool is_fused;
};

bool read_entire_file_raw(String* out, String filepath);

MountResult vfs_mount(const_str fsname, const char* filepath);
void vfs_fini();

u64 vfs_file_modtime(String filepath);
bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String* out, String filepath);
bool vfs_write_entire_file(String fsname, String filepath, String contents);
bool vfs_list_all_files(String fsname, Array<String>* files);

void* vfs_for_miniaudio();

/*==========================
// NEKO_PACK
==========================*/

#define NEKO_PAK_HEAD_SIZE 8

struct PakItemInfo {
    u32 zip_size;
    u32 data_size;
    u64 file_offset;
    u8 path_size;
};

struct PakItem {
    PakItemInfo info;
    const_str path;
};

struct Pak {
    vfs_file vf;
    u64 item_count;
    PakItem* items;
    u8* data_buffer;
    u8* zip_buffer;
    u32 data_size;
    u32 zip_size;
    PakItem search_item;
    u32 file_ref_count;
};

bool pak_load(Pak* pak, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory);
void pak_fini(Pak* pak);

inline u64 pak_get_item_count(Pak* pak) { return pak->item_count; }

inline u32 pak_get_item_size(Pak* pak, u64 index) {
    neko_assert(index < pak->item_count);
    return pak->items[index].info.data_size;
}

inline const_str pak_get_item_path(Pak* pak, u64 index) {
    neko_assert(index < pak->item_count);
    return pak->items[index].path;
}

u64 pak_get_item_index(Pak* pak, const_str path);

bool pak_get_data(Pak* pak, u64 index, String* out, u32* size);
bool pak_get_data(Pak* pak, const_str path, String* out, u32* size);
void pak_free_item(Pak* pak, String data);
void pak_free_buffer(Pak* pak);

bool neko_pak_unzip(const_str file_path, bool print_progress);
bool neko_pak_build(const_str pack_path, u64 file_count, const_str* file_paths, bool print_progress);
bool neko_pak_info(const_str file_path, i32* buildnum, u64* item_count);

int open_mt_pak(lua_State* L);
int neko_pak_load(lua_State* L);

inline bool neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
inline bool neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
inline bool neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
inline bool neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

typedef enum xml_attribute_type_t {
    NEKO_XML_ATTRIBUTE_NUMBER,
    NEKO_XML_ATTRIBUTE_BOOLEAN,
    NEKO_XML_ATTRIBUTE_STRING,
} xml_attribute_type_t;

typedef struct xml_attribute_t {
    const_str name;
    xml_attribute_type_t type;

    union {
        double number;
        bool boolean;
        const_str string;
    } value;
} xml_attribute_t;

typedef struct xml_node_t {
    const_str name;
    const_str text;

    HashMap<xml_attribute_t> attributes;
    Array<xml_node_t> children;

} xml_node_t;

typedef struct xml_document_t {
    Array<xml_node_t> nodes;
} xml_document_t;

typedef struct xml_node_iter_t {
    xml_document_t* doc;
    xml_node_t* node;
    const_str name;
    u32 idx;

    xml_node_t* current;
} xml_node_iter_t;

xml_document_t* xml_parse(const_str source);
xml_document_t* xml_parse_vfs(const_str path);
void xml_free(xml_document_t* document);

xml_attribute_t* xml_find_attribute(xml_node_t* node, const_str name);
xml_node_t* xml_find_node(xml_document_t* doc, const_str name);
xml_node_t* xml_find_node_child(xml_node_t* node, const_str name);

const_str xml_get_error();

xml_node_iter_t xml_new_node_iter(xml_document_t* doc, const_str name);
xml_node_iter_t xml_new_node_child_iter(xml_node_t* node, const_str name);
bool xml_node_iter_next(xml_node_iter_t* iter);

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    neko_handle(gfx_texture_t) texture;
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

    Color256 tint;
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
    Array<object_t> objects;

    Color256 color;

    const_str name;
} object_group_t;

typedef struct TiledMap {
    xml_document_t* doc;  // xml doc
    Array<tileset_t> tilesets;
    Array<object_group_t> object_groups;
    Array<layer_t> layers;
} TiledMap;

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
        JSONObject* object;
        JSONArray* array;
        String string;
        double number;
        bool boolean;
    };
    JSONKind kind;

    JSON lookup(String key, bool* ok);
    JSON index(i32 i, bool* ok);

    JSONObject* as_object(bool* ok);
    JSONArray* as_array(bool* ok);
    String as_string(bool* ok);
    double as_number(bool* ok);

    JSONObject* lookup_object(String key, bool* ok);
    JSONArray* lookup_array(String key, bool* ok);
    String lookup_string(String key, bool* ok);
    double lookup_number(String key, bool* ok);

    double index_number(i32 i, bool* ok);
};

struct JSONObject {
    JSON value;
    String key;
    JSONObject* next;
    u64 hash;
};

struct JSONArray {
    JSON value;
    JSONArray* next;
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
void json_write_string(StringBuilder* sb, JSON* json);
void json_print(JSON* json);

struct lua_State;
void json_to_lua(lua_State* L, JSON* json);
String lua_to_json_string(lua_State* L, i32 arg, String* contents, i32 width);

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_AseSprite,
    AssetKind_Tiledmap,
    AssetKind_Shader,
    // AssetKind_Pak,
};

struct AssetLoadData {
    AssetKind kind;
    bool flip_image_vertical;
};

struct Asset {
    String name;
    u64 hash;
    u64 modtime;
    AssetKind kind;
    union {
        i32 lua_ref;
        AssetTexture texture;
        AseSpriteData sprite;
        AssetShader shader;
        TiledMap tiledmap;
        // Pak pak;
    };
};

struct FileChange {
    u64 key;
    u64 modtime;
};

struct Assets {
    HashMap<Asset> table;
    RWLock rw_lock;

    Mutex shutdown_mtx;
    Cond shutdown_notify;
    bool shutdown;

    Thread reload_thread;

    Mutex changes_mtx;
    Array<FileChange> changes;
    Array<FileChange> tmp_changes;
};

struct App;

void assets_shutdown();
void assets_start_hot_reload();
int assets_perform_hot_reload_changes(App* app, event_t evt);

bool asset_load_kind(AssetKind kind, String filepath, Asset* out);
bool asset_load(AssetLoadData desc, String filepath, Asset* out);

bool asset_read(u64 key, Asset* out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State* L, u64 key);
Asset check_asset_mt(lua_State* L, i32 arg, const char* mt);

char* file_pathabs(const char* pathfile);

#endif