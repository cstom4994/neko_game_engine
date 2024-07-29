#pragma once

#include "neko_base.h"
#include "neko_image.h"
#include "neko_math.h"
#include "neko_tilemap.h"

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
        MapLdtk tilemap;
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
