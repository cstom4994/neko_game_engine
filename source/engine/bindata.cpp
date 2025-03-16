
#include "bindata.h"

#include "base/cbase.hpp"
#include "base/common/util.hpp"
#include "base/common/profiler.hpp"
#include "base/scripting/lua_wrapper.hpp"
#include "base/common/logger.hpp"

// miniz
#include "extern/miniz.h"

namespace Neko {

using namespace Neko::luabind;

static int BinDataCompareItemPath(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

static int BinDataCompareItem(const void *_a, const void *_b) {
    const BinDataItem *a = (BinDataItem *)_a;
    const BinDataItem *b = (BinDataItem *)_b;
    int difference = (int)a->info.path_size - (int)b->info.path_size;
    if (difference != 0) return difference;
    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

static void BinDataItemDestroy(u64 count, Array<BinDataItem> &items) {
    PROFILE_FUNC();

    neko_assert(count == 0 || (count > 0));
    for (u64 i = 0; i < count; i++) mem_free(items[i].path);
    items.trash();
}

static bool BinDataItemCreate(vfs_file *bindata, u64 count, Array<BinDataItem> &items) {
    PROFILE_FUNC();

    neko_assert(bindata && count > 0);

    for (u64 i = 0; i < count; i++) {
        BinDataItemInfo info;
        size_t result = neko_capi_vfs_fread(&info, sizeof(BinDataItemInfo), 1, bindata);  // 读取文件信息
        if (result != 1 || info.data_size == 0 || info.path_size == 0) {
            BinDataItemDestroy(i, items);
            return false;
        }
        char *path = (char *)mem_alloc((info.path_size + 1) * sizeof(char));
        result = neko_capi_vfs_fread(path, sizeof(char), info.path_size, bindata);  // 读取文件路径
        path[info.path_size] = 0;
        if (result != info.path_size) {
            BinDataItemDestroy(i, items);
            return false;
        }

        i64 file_offset = info.zip_size > 0 ? info.zip_size : info.data_size;
        int seekResult = neko_capi_vfs_fseek(bindata, file_offset, SEEK_CUR);  // 检查文件数据
        if (seekResult != 0) {
            BinDataItemDestroy(i, items);
            return false;
        }

        BinDataItem item{.info = info, .path = path};
        items.push(item);
    }

    return true;
}

bool BinDataLoad(BinData *bindata, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory) {
    PROFILE_FUNC();

    neko_assert(file_path && bindata);
    bindata->zip_buffer = NULL;
    bindata->zip_buffer_size = 0;
    bindata->vf = neko_capi_vfs_fopen(file_path);
    if (!bindata->vf.data) return false;

    char header[NEKO_PAK_HEAD_SIZE];  // 检查文件头
    i32 buildnum;                     // 构建版本

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &bindata->vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &bindata->vf);

    // 检查文件头
    if (result != NEKO_PAK_HEAD_SIZE + 1 || header[0] != 'N' || header[1] != 'E' || header[2] != 'K' || header[3] != 'O' || header[4] != 'P' || header[5] != 'A' || header[6] != 'C' ||
        header[7] != 'K') {
        BinDataUnload(bindata);
        return false;
    }

    // 获取打包文件数量
    u64 item_count;
    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &bindata->vf);
    if (result != 1 || item_count == 0) {
        BinDataUnload(bindata);
        return false;
    }

    // 获取打包文件
    Array<BinDataItem> items;
    bool ok = BinDataItemCreate(&bindata->vf, item_count, items);
    if (!ok) {
        BinDataUnload(bindata);
        return false;
    }

    bindata->items = items;

    u8 *_data_buffer = NULL;
    if (data_buffer_capacity > 0) {
        _data_buffer = (u8 *)mem_alloc(data_buffer_capacity * sizeof(u8));
    } else {
        _data_buffer = NULL;
    }

    bindata->data_buffer = _data_buffer;
    bindata->data_buffer_size = data_buffer_capacity;

    LOG_INFO("load bindata {} buildnum: {} (engine {})", file_path, buildnum, neko_buildnum());

    return true;
}
void BinDataUnload(BinData *bindata) {
    PROFILE_FUNC();

    if (bindata->file_ref_count != 0) {
        LOG_INFO("assets loader leaks detected {} refs", bindata->file_ref_count);
    }

    BinDataFreeBuffer(bindata);

    BinDataItemDestroy(bindata->items.len, bindata->items);
    if (bindata->vf.data) neko_capi_vfs_fclose(&bindata->vf);
}

u64 BinDataGetItemIndex(BinData *bindata, const_str path) {
    neko_assert(path && neko_strlen(path) <= u64_max);

    BinDataItem *search_item = &bindata->search_item;
    search_item->info.path_size = neko_strlen(path);
    search_item->path = path;

    BinDataItem *items = (BinDataItem *)std::bsearch(search_item, bindata->items.data, bindata->items.len, sizeof(BinDataItem), BinDataCompareItem);  // 二分查找

    if (!items) return u64_max;

    u64 index = items - bindata->items.data;
    return index;
}

bool BinDataGetItem(BinData *bindata, u64 index, String *out, u64 *size) {
    PROFILE_FUNC();

    neko_assert((index < bindata->items.len) && out && size);

    BinDataItemInfo info = bindata->items[index].info;

    // data缓冲区大小是否需要扩大
    u8 *_data_buffer = bindata->data_buffer;
    if (_data_buffer) {
        if (info.data_size > bindata->data_buffer_size) {
            _data_buffer = (u8 *)mem_realloc(_data_buffer, info.data_size * sizeof(u8));
            bindata->data_buffer = _data_buffer;
            bindata->data_buffer_size = info.data_size;
        }
    } else {
        _data_buffer = (u8 *)mem_alloc(info.data_size * sizeof(u8));
        bindata->data_buffer = _data_buffer;
        bindata->data_buffer_size = info.data_size;
    }

    // zip缓冲区大小是否需要扩大
    u8 *_zip_buffer = bindata->zip_buffer;
    if (bindata->zip_buffer) {
        if (info.zip_size > bindata->zip_buffer_size) {
            _zip_buffer = (u8 *)mem_realloc(bindata->zip_buffer, info.zip_size * sizeof(u8));
            bindata->zip_buffer = _zip_buffer;
            bindata->zip_buffer_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            _zip_buffer = (u8 *)mem_alloc(info.zip_size * sizeof(u8));
            bindata->zip_buffer = _zip_buffer;
            bindata->zip_buffer_size = info.zip_size;
        }
    }

    vfs_file *vf = &bindata->vf;
    i64 file_offset = (i64)(info.file_offset + sizeof(BinDataItemInfo) + info.path_size);
    int seek_result = neko_capi_vfs_fseek(vf, file_offset, SEEK_SET);  // 先检查偏移长度
    if (seek_result != 0) return false;

    // 判断是否为zip数据
    if (info.zip_size > 0) {
        size_t result = neko_capi_vfs_fread(_zip_buffer, sizeof(u8), info.zip_size, vf);
        if (result != info.zip_size) return false;

        // 解压zip数据到data缓冲区
        unsigned long decompress_buf_len = info.data_size;
        int decompress_status = uncompress(_data_buffer, &decompress_buf_len, _zip_buffer, info.zip_size);
        result = decompress_buf_len;
        LOG_INFO("uncompress {} {}", info.zip_size, info.data_size);
        if (result < 0 || result != info.data_size || decompress_status != MZ_OK) {
            return false;
        }
    } else {
        size_t result = neko_capi_vfs_fread(_data_buffer, sizeof(u8), info.data_size, vf);
        if (result != info.data_size) return false;
    }

    (*size) = info.data_size;

    // 将data缓冲区数据复制
    char *data = (char *)mem_alloc(info.data_size + 1);
    memcpy((void *)(data), _data_buffer, info.data_size);
    data[info.data_size] = 0;
    *out = {data, info.data_size};

    bindata->file_ref_count++;
    return true;
}

bool BinDataGetItem(BinData *bindata, const_str path, String *out, u64 *size) {
    neko_assert(path && out && size);
    neko_assert(strlen(path) <= u8_max);
    u64 index = BinDataGetItemIndex(bindata, path);
    if (index == u64_max) return false;  // FAILED_TO_GET_ITEM_PACK_RESULT
    return BinDataGetItem(bindata, index, out, size);
}

void BinDataFreeItem(BinData *bindata, String data) {
    mem_free(data.data);
    bindata->file_ref_count--;
}

void BinDataFreeBuffer(BinData *bindata) {
    mem_free(bindata->data_buffer);
    mem_free(bindata->zip_buffer);
    bindata->data_buffer = NULL;
    bindata->zip_buffer = NULL;
}

static void BinDataRemoveItem(u64 item_count, Array<BinDataItem> &items) {
    neko_assert(item_count == 0 || (item_count > 0));
    for (u64 i = 0; i < item_count; i++) remove(items[i].path);
}

bool BinDataUnpack(const_str file_path, bool print_progress) {
    neko_assert(file_path);

    BinData bindata;
    int pack_result = BinDataLoad(&bindata, file_path, 128, false);
    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;
    u64 item_count = bindata.items.len;
    for (u64 i = 0; i < item_count; i++) {
        BinDataItem *item = &bindata.items[i];

        if (print_progress) LOG_INFO("Unpacking {}", item->path);

        String data;
        u64 data_size;
        pack_result = BinDataGetItem(&bindata, i, &data, &data_size);

        if (pack_result != 0) {
            BinDataRemoveItem(i, bindata.items);
            BinDataUnload(&bindata);
            return pack_result;
        }

        u8 path_size = item->info.path_size;
        char item_path[u8_max + 1];
        memcpy(item_path, item->path, path_size * sizeof(char));
        item_path[path_size] = 0;

        // 解压的时候路径节替换为 '-'
        for (u8 j = 0; j < path_size; j++)
            if (item_path[j] == '/' || item_path[j] == '\\' || (item_path[j] == '.' && j == 0)) item_path[j] = '-';

        FILE *item_file = neko_fopen(item_path, "wb");

        if (!item_file) {
            BinDataRemoveItem(i, bindata.items);
            BinDataUnload(&bindata);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        size_t result = fwrite(data.data, sizeof(u8), data_size, item_file);

        neko_fclose(item_file);

        if (result != data_size) {
            BinDataRemoveItem(i, bindata.items);
            BinDataUnload(&bindata);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        if (print_progress) {
            u32 raw_file_size = item->info.data_size;
            u32 zip_file_size = item->info.zip_size > 0 ? item->info.zip_size : item->info.data_size;

            total_raw_size += raw_file_size;
            total_zip_size += zip_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", raw_file_size, zip_file_size, progress);
        }
    }

    BinDataUnload(&bindata);

    if (print_progress) {
        neko_printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return true;
}

bool BinDataWriteItem(FILE *binfile, u64 item_count, char **item_paths, bool print_progress) {
    neko_assert(binfile && item_count > 0 && item_paths);

    u32 buffer_size = 128;  // 初始缓冲大小
    u64 total_zip_size = 0, total_raw_size = 0;

    u8 *zip_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (print_progress) {
            LOG_INFO("Packing \"{}\" file. ", item_path);
        }

        size_t path_size = neko_strlen(item_path);
        neko_assert(path_size <= u8_max);

        // FILE *item_file = neko_fopen(item_path, "rb");
        // int seek_result = neko_fseek(item_file, 0, SEEK_END);
        // u64 item_size = (u64)neko_ftell(item_file);
        // seek_result = neko_fseek(item_file, 0, SEEK_SET);

        String content{};
        bool ok = vfs_read_entire_file(&content, item_path);
        neko_defer(mem_free(content.data));
        neko_assert(ok);

        if (content.len > buffer_size) {
            u8 *new_buffer = (u8 *)mem_realloc(zip_data, content.len * sizeof(u8));
            zip_data = new_buffer;
        }

        size_t zip_size;

        if (content.len > 1) {

            unsigned long compress_buf_len = compressBound(content.len);
            int compress_status = compress(zip_data, &compress_buf_len, (const unsigned char *)content.data, content.len);
            zip_size = compress_buf_len;

            if (zip_size <= 0 || zip_size >= content.len || compress_status != MZ_OK) {
                zip_size = 0;
            }
        } else {
            zip_size = 0;
        }

        i64 file_offset = neko_ftell(binfile);

        BinDataItemInfo info = {
                (u64)zip_size,
                (u64)content.len,
                (u64)file_offset,
                (u64)path_size,
        };

        size_t result = fwrite(&info, sizeof(BinDataItemInfo), 1, binfile);

        if (result != 1) {
            mem_free(zip_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        result = fwrite(item_path, sizeof(char), info.path_size, binfile);

        if (result != info.path_size) {
            mem_free(zip_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, binfile);

            if (result != zip_size) {
                mem_free(zip_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        } else {
            result = fwrite(content.data, sizeof(u8), content.len, binfile);

            if (result != content.len) {
                mem_free(zip_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        }

        if (print_progress) {
            u64 zip_file_size = zip_size > 0 ? (u64)zip_size : (u64)content.len;
            u64 raw_file_size = (u64)content.len;

            total_zip_size += zip_file_size;
            total_raw_size += raw_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            LOG_INFO("({}/{} bytes) [{}%]", zip_file_size, raw_file_size, progress);
        }
    }

    mem_free(zip_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        LOG_INFO("Packed {} files. ({}/{} bytes, {}% saved)", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size, compression);
    }

    return true;
}

bool BinDataBuild(const_str file_path, u64 file_count, const_str *file_paths, bool print_progress) {
    neko_assert(file_path);
    neko_assert(file_count > 0);
    neko_assert(file_paths);

    char **item_paths = (char **)mem_alloc(file_count * sizeof(char *));

    if (!item_paths) return false;  // FAILED_TO_ALLOCATE_PACK_RESULT

    u64 item_count = 0;

    for (u64 i = 0; i < file_count; i++) {
        bool already_added = false;

        for (u64 j = 0; j < item_count; j++) {
            if (i != j && strcmp(file_paths[i], item_paths[j]) == 0) already_added = true;
        }

        if (!already_added) item_paths[item_count++] = (char *)file_paths[i];
    }

    std::qsort(item_paths, item_count, sizeof(char *), BinDataCompareItemPath);

    FILE *pack_file = neko_fopen(file_path, "wb");

    if (!pack_file) {
        mem_free(item_paths);
        return false;  // FAILED_TO_CREATE_FILE_PACK_RESULT
    }

    char header[NEKO_PAK_HEAD_SIZE] = {
            'N', 'E', 'K', 'O', 'P', 'A', 'C', 'K',
    };

    i32 buildnum = neko_buildnum();

    size_t write_result = fwrite(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, pack_file);
    write_result += fwrite(&buildnum, sizeof(i32), 1, pack_file);

    if (write_result != NEKO_PAK_HEAD_SIZE + 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
    }

    write_result = fwrite(&item_count, sizeof(u64), 1, pack_file);

    if (write_result != 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
    }

    bool ok = BinDataWriteItem(pack_file, item_count, item_paths, print_progress);

    mem_free(item_paths);
    neko_fclose(pack_file);

    if (!ok) {
        remove(file_path);
        return ok;
    }

    neko_printf("Packed %s ok\n", file_path);

    return true;
}

bool BinDataInfo(const_str file_path, i32 *buildnum, u64 *item_count) {
    neko_assert(file_path);
    neko_assert(buildnum);
    neko_assert(item_count);

    vfs_file vf = neko_capi_vfs_fopen(file_path);
    if (!vf.data) return false;

    char header[NEKO_PAK_HEAD_SIZE];

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &vf);
    result += neko_capi_vfs_fread(buildnum, sizeof(i32), 1, &vf);

    if (result != NEKO_PAK_HEAD_SIZE + 1) {
        neko_capi_vfs_fclose(&vf);
        return false;
    }

    if (header[0] != 'N' || header[1] != 'E' || header[2] != 'K' || header[3] != 'O' || header[4] != 'P' || header[5] != 'A' || header[6] != 'C' || header[7] != 'K') {
        neko_capi_vfs_fclose(&vf);
        return false;
    }

    result = neko_capi_vfs_fread(item_count, sizeof(u64), 1, &vf);

    neko_capi_vfs_fclose(&vf);

    if (result != 1) return false;
    return true;
}

namespace mt_bindata {

static BinData &to(lua_State *L, int idx) { return luabind::checkudata<BinData>(L, idx); }

struct BinDataAssets {
    const_str name;
    u64 size;
    String data;
};

static BinData &check_bindata_udata(lua_State *L, i32 arg) {
    BinData &udata = to(L, 1);
    if (!udata.items.len) {
        luaL_error(L, "cannot read bindata");
    }
    return udata;
}

static int mt_bindata_gc(lua_State *L) {
    BinData &bindata = check_bindata_udata(L, 1);
    lua_getiuservalue(L, 1, 1);
    const_str name = lua_tostring(L, -1);
    BinDataUnload(&bindata);
    LOG_INFO("bindata __gc {}", name);
    return 0;
}

static int mt_bindata_items(lua_State *L) {
    BinData &bindata = check_bindata_udata(L, 1);

    u64 item_count = bindata_get_item_count(&bindata);

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, bindata_get_item_path(&bindata, i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int mt_bindata_assets_load(lua_State *L) {
    BinData &bindata = check_bindata_udata(L, 1);

    const_str path = lua_tostring(L, 2);

    BinDataAssets *assets_user_handle = (BinDataAssets *)lua_newuserdata(L, sizeof(BinDataAssets));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    bool ok = BinDataGetItem(&bindata, path, &assets_user_handle->data, (u64 *)&assets_user_handle->size);

    if (!ok) {
        const_str error_message = "mt_bindata_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // asset_write(asset);

    return 1;
}

static int mt_bindata_assets_unload(lua_State *L) {
    BinData &bindata = check_bindata_udata(L, 1);

    BinDataAssets *assets_user_handle = (BinDataAssets *)lua_touserdata(L, 2);

    if (assets_user_handle && assets_user_handle->data.len)
        BinDataFreeItem(&bindata, assets_user_handle->data);
    else
        LOG_INFO("unknown assets unload {}", assets_user_handle->name);

    // asset_write(asset);

    return 0;
}

int open_mt_bindata(lua_State *L) { return 0; }

int neko_bindata_load(lua_State *L) {
    String name = luax_check_string(L, 1);
    String path = luax_check_string(L, 2);

    BinData &bindata = luabind::newudata<BinData>(L);
    lua_pushstring(L, name.cstr());
    lua_setiuservalue(L, -2, 1);

    bool ok = BinDataLoad(&bindata, path.cstr(), 0, false);

    if (!ok) {
        return 0;
    }

    return 1;
}

static int mt_close(lua_State *L) {
    auto &self = to(L, 1);
    return 0;
}

static void metatable(lua_State *L) {
    static luaL_Reg lib[] = {
            {"items", mt_bindata_items},
            {"assets_load", mt_bindata_assets_load},
            {"assets_unload", mt_bindata_assets_unload},
            {nullptr, nullptr},
    };
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {"__gc", mt_bindata_gc}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

}  // namespace mt_bindata

}  // namespace Neko

namespace Neko::luabind {
template <>
struct udata<BinData> {
    static inline int nupvalue = 1;
    static inline auto metatable = Neko::mt_bindata::metatable;
};
}  // namespace Neko::luabind