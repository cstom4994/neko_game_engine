
#ifndef NEKO_PAK_H
#define NEKO_PAK_H

#include "engine/neko_asset.h"
#include "engine/neko_prelude.h"

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

#endif