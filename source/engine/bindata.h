#pragma once

#include "base/common/vfs.hpp"

struct lua_State;

namespace Neko {

static constexpr int NEKO_PAK_HEAD_SIZE = 8;

struct BinDataItemInfo {
    u64 zip_size;
    u64 data_size;
    u64 file_offset;
    u64 path_size;
};

struct BinDataItem {
    BinDataItemInfo info;
    const_str path;
};

struct BinData {
    vfs_file vf;
    Array<BinDataItem> items;
    u8* data_buffer;
    u8* zip_buffer;
    u32 data_buffer_size;
    u32 zip_buffer_size;
    BinDataItem search_item;
    u32 file_ref_count;
};

bool BinDataLoad(BinData* bindata, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory);
void BinDataUnload(BinData* bindata);

inline u64 bindata_get_item_count(BinData* bindata) { return bindata->items.len; }

inline u64 bindata_get_item_size(BinData* bindata, u64 index) {
    neko_assert(index < bindata_get_item_count(bindata));
    return bindata->items[index].info.data_size;
}

inline const_str bindata_get_item_path(BinData* bindata, u64 index) {
    neko_assert(index < bindata_get_item_count(bindata));
    return bindata->items[index].path;
}

u64 BinDataGetItemIndex(BinData* bindata, const_str path);

bool BinDataGetItem(BinData* bindata, u64 index, String* out, u64* size);
bool BinDataGetItem(BinData* bindata, const_str path, String* out, u64* size);
void BinDataFreeItem(BinData* bindata, String data);
void BinDataFreeBuffer(BinData* bindata);
bool BinDataUnpack(const_str file_path, bool print_progress);
bool BinDataBuild(const_str pack_path, u64 file_count, const_str* file_paths, bool print_progress);
bool BinDataInfo(const_str file_path, i32* buildnum, u64* item_count);

namespace mt_bindata {

int open_mt_bindata(lua_State* L);
int neko_bindata_load(lua_State* L);

}  // namespace mt_bindata

}  // namespace Neko