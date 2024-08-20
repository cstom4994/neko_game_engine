
#ifndef NEKO_PAK_H
#define NEKO_PAK_H

#include "engine/asset.h"
#include "engine/prelude.h"
#include "engine/vfs.h"

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
    PakItem *items;
    u8 *data_buffer;
    u8 *zip_buffer;
    u32 data_size;
    u32 zip_size;
    PakItem search_item;
    u32 file_ref_count;
};

bool pak_load(Pak *pak, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory);
void pak_fini(Pak *pak);

inline u64 pak_get_item_count(Pak *pak) { return pak->item_count; }

inline u32 pak_get_item_size(Pak *pak, u64 index) {
    neko_assert(index < pak->item_count);
    return pak->items[index].info.data_size;
}

inline const_str pak_get_item_path(Pak *pak, u64 index) {
    neko_assert(index < pak->item_count);
    return pak->items[index].path;
}

u64 pak_get_item_index(Pak *pak, const_str path);

bool pak_get_data(Pak *pak, u64 index, String *out, u32 *size);
bool pak_get_data(Pak *pak, const_str path, String *out, u32 *size);
void pak_free_item(Pak *pak, String data);
void pak_free_buffer(Pak *pak);

bool neko_pak_unzip(const_str file_path, bool print_progress);
bool neko_pak_build(const_str pack_path, u64 file_count, const_str *file_paths, bool print_progress);
bool neko_pak_info(const_str file_path, i32 *buildnum, u64 *item_count);

int open_mt_pak(lua_State *L);
int neko_pak_load(lua_State *L);

#endif