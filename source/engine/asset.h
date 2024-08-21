#ifndef NEKO_ASSET_H
#define NEKO_ASSET_H

#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/entity.h"
#include "engine/gfx.h"
#include "engine/glew_glfw.h"
#include "engine/math.h"
#include "engine/prelude.h"

typedef struct AssetTexture {
    GLuint id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;
    bool flip_image_vertical;
} AssetTexture;

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
LuaVec2 texture_get_size(const char* filename);  // (width, height)
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

struct batch_renderer;

struct AseSprite {
    batch_renderer* batch;
    u64 sprite;  // index into assets
    u64 loop;    // index into AseSpriteData::by_tag
    float elapsed;
    i32 current_frame;

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

NEKO_SCRIPT(sprite,

            NEKO_EXPORT void sprite_set_atlas(const char* filename);

            NEKO_EXPORT const char* sprite_get_atlas();

            NEKO_EXPORT void sprite_add(Entity ent);

            NEKO_EXPORT void sprite_remove(Entity ent);

            NEKO_EXPORT bool sprite_has(Entity ent);

            // size to draw in world units, centered at transform position
            NEKO_EXPORT void sprite_set_size(Entity ent, LuaVec2 size);

            NEKO_EXPORT LuaVec2 sprite_get_size(Entity ent);

            // bottom left corner of atlas region in pixels
            NEKO_EXPORT void sprite_set_texcell(Entity ent, LuaVec2 texcell);

            NEKO_EXPORT LuaVec2 sprite_get_texcell(Entity ent);

            // size of atlas region in pixels
            NEKO_EXPORT void sprite_set_texsize(Entity ent, LuaVec2 texsize);

            NEKO_EXPORT LuaVec2 sprite_get_texsize(Entity ent);

            // lower depth drawn on top
            NEKO_EXPORT void sprite_set_depth(Entity ent, int depth);

            NEKO_EXPORT int sprite_get_depth(Entity ent);

)

void sprite_init();
void sprite_fini();
void sprite_update_all();
void sprite_draw_all();
void sprite_save_all(Store* s);
void sprite_load_all(Store* s);

int open_mt_sprite(lua_State* L);
int neko_sprite_load(lua_State* L);

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

NEKO_SCRIPT(
        fs,

        // remember to *_close(...) when done to free resources!

        // NEKO_EXPORT Dir * fs_dir_open(const char *path);

        // NEKO_EXPORT const char *fs_dir_next_file(Dir *dir);  // NULL after last file

        // NEKO_EXPORT void fs_dir_close(Dir *dir);

        typedef struct vfs_file {
            const_str data;
            size_t len;
            u64 offset;
        } vfs_file;

        NEKO_EXPORT size_t neko_capi_vfs_fread(void* dest, size_t size, size_t count, vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fseek(vfs_file* vf, u64 of, int whence);

        NEKO_EXPORT u64 neko_capi_vfs_ftell(vfs_file * vf);

        NEKO_EXPORT vfs_file neko_capi_vfs_fopen(const_str path);

        NEKO_EXPORT int neko_capi_vfs_fclose(vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fscanf(vfs_file* vf, const char* format, ...);

        NEKO_EXPORT bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);

        NEKO_EXPORT const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t* size);

)

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

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_AseSprite,
    AssetKind_Tilemap,
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
        // MapLdtk tilemap;
        // Pak pak;
    };
};

typedef enum neko_resource_type_t {
    NEKO_RESOURCE_STRING,
    NEKO_RESOURCE_BINARY,
    NEKO_RESOURCE_TEXTURE,
    NEKO_RESOURCE_SHADER,
    NEKO_RESOURCE_ASSEMBLY,
    NEKO_RESOURCE_SCRIPT,
    NEKO_RESOURCE_MODEL,
    NEKO_RESOURCE_MATERIAL,
    NEKO_RESOURCE_FONT
} neko_resource_type_t;

typedef struct neko_resource_t {
    neko_resource_type_t type;

    void* payload;
    u32 payload_size;

    i64 modtime;

    char* file_name;
    u32 file_name_length;
    u32 file_name_hash;
} neko_resource_t;

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

void assets_shutdown();
void assets_start_hot_reload();
void assets_perform_hot_reload_changes();

bool asset_load_kind(AssetKind kind, String filepath, Asset* out);
bool asset_load(AssetLoadData desc, String filepath, Asset* out);

bool asset_read(u64 key, Asset* out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State* L, u64 key);
Asset check_asset_mt(lua_State* L, i32 arg, const char* mt);

char* file_pathabs(const char* pathfile);

#endif