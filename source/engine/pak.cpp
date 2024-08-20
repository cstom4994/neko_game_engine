
#include "engine/pak.h"

#include "engine/game.h"
#include "engine/vfs.h"

// miniz
#include <miniz.h>

/*==========================
// NEKO_PACK
==========================*/

static int _compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

static void _destroy_pack_items(u64 item_count, PakItem *items) {
    neko_assert(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) mem_free(items[i].path);
    mem_free(items);
}

static bool _create_pack_items(vfs_file *pak, u64 item_count, PakItem **_items) {
    neko_assert(pak);
    neko_assert(item_count > 0);
    neko_assert(_items);

    PakItem *items = (PakItem *)mem_alloc(item_count * sizeof(PakItem));

    if (!items) return false;  // FAILED_TO_ALLOCATE_PACK_RESULT

    for (u64 i = 0; i < item_count; i++) {
        PakItemInfo info;

        size_t result = neko_capi_vfs_fread(&info, sizeof(PakItemInfo), 1, pak);

        if (result != 1) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        if (info.data_size == 0 || info.path_size == 0) {
            _destroy_pack_items(i, items);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        char *path = (char *)mem_alloc((info.path_size + 1) * sizeof(char));

        if (!path) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
        }

        result = neko_capi_vfs_fread(path, sizeof(char), info.path_size, pak);

        path[info.path_size] = 0;

        if (result != info.path_size) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        i64 fileOffset = info.zip_size > 0 ? info.zip_size : info.data_size;

        int seekResult = neko_capi_vfs_fseek(pak, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        PakItem *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return true;
}

static int _compare_pack_items(const void *_a, const void *_b) {
    const PakItem *a = (PakItem *)_a;
    const PakItem *b = (PakItem *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

bool pak_load(Pak *pak, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory) {
    neko_assert(file_path);

    // memset(pak, 0, sizeof(Pak));

    pak->zip_buffer = NULL;
    pak->zip_size = 0;

    pak->vf = neko_capi_vfs_fopen(file_path);

    if (!pak->vf.data) return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT

    char header[NEKO_PAK_HEAD_SIZE];
    i32 buildnum;

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &pak->vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &pak->vf);

    // 检查文件头
    if (result != NEKO_PAK_HEAD_SIZE + 1 ||  //
        header[0] != 'N' ||                  //
        header[1] != 'E' ||                  //
        header[2] != 'K' ||                  //
        header[3] != 'O' ||                  //
        header[4] != 'P' ||                  //
        header[5] != 'A' ||                  //
        header[6] != 'C' ||                  //
        header[7] != 'K') {
        pak_fini(pak);
        return false;
    }

    u64 item_count;

    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &pak->vf);

    if (result != 1 ||  //
        item_count == 0) {
        pak_fini(pak);
        return false;
    }

    PakItem *items;

    bool ok = _create_pack_items(&pak->vf, item_count, &items);

    if (!ok) {
        pak_fini(pak);
        return false;
    }

    pak->item_count = item_count;
    pak->items = items;

    u8 *_data_buffer = NULL;

    if (data_buffer_capacity > 0) {
        _data_buffer = (u8 *)mem_alloc(data_buffer_capacity * sizeof(u8));
    } else {
        _data_buffer = NULL;
    }

    pak->data_buffer = _data_buffer;
    pak->data_size = data_buffer_capacity;

    console_log("load pack %s buildnum: %d (engine %d)", neko_util_get_filename(file_path), buildnum, neko_buildnum());

    return true;
}
void pak_fini(Pak *pak) {
    if (pak->file_ref_count != 0) {
        console_log("assets loader leaks detected %d refs", pak->file_ref_count);
    }

    pak_free_buffer(pak);

    _destroy_pack_items(pak->item_count, pak->items);
    if (pak->vf.data) neko_capi_vfs_fclose(&pak->vf);
}

u64 pak_get_item_index(Pak *pak, const_str path) {
    neko_assert(path);
    neko_assert(strlen(path) <= u8_max);

    PakItem *search_item = &pak->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    PakItem *items = (PakItem *)bsearch(search_item, pak->items, pak->item_count, sizeof(PakItem), _compare_pack_items);

    if (!items) return u64_max;

    u64 index = items - pak->items;
    return index;
}

bool pak_get_data(Pak *pak, u64 index, String *out, u32 *size) {
    neko_assert((index < pak->item_count) && out && size);

    PakItemInfo info = pak->items[index].info;

    u8 *_data_buffer = pak->data_buffer;
    if (_data_buffer) {
        if (info.data_size > pak->data_size) {
            _data_buffer = (u8 *)mem_realloc(_data_buffer, info.data_size * sizeof(u8));
            pak->data_buffer = _data_buffer;
            pak->data_size = info.data_size;
        }
    } else {
        _data_buffer = (u8 *)mem_alloc(info.data_size * sizeof(u8));
        pak->data_buffer = _data_buffer;
        pak->data_size = info.data_size;
    }

    u8 *_zip_buffer = pak->zip_buffer;
    if (pak->zip_buffer) {
        if (info.zip_size > pak->zip_size) {
            _zip_buffer = (u8 *)mem_realloc(pak->zip_buffer, info.zip_size * sizeof(u8));
            pak->zip_buffer = _zip_buffer;
            pak->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            _zip_buffer = (u8 *)mem_alloc(info.zip_size * sizeof(u8));
            pak->zip_buffer = _zip_buffer;
            pak->zip_size = info.zip_size;
        }
    }

    vfs_file *vf = &pak->vf;

    i64 file_offset = (i64)(info.file_offset + sizeof(PakItemInfo) + info.path_size);

    int seek_result = neko_capi_vfs_fseek(vf, file_offset, SEEK_SET);

    if (seek_result != 0) return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT

    if (info.zip_size > 0) {
        size_t result = neko_capi_vfs_fread(_zip_buffer, sizeof(u8), info.zip_size, vf);

        if (result != info.zip_size) return false;  // FAILED_TO_READ_FILE_PACK_RESULT

        // result = neko_lz_decode(_zip_buffer, info.zip_size, _data_buffer, info.data_size);

        unsigned long decompress_buf_len = info.data_size;
        int decompress_status = uncompress(_data_buffer, &decompress_buf_len, _zip_buffer, info.zip_size);

        result = decompress_buf_len;

        console_log("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

        if (result < 0 || result != info.data_size || decompress_status != MZ_OK) {
            return false;  // FAILED_TO_DECOMPRESS_PACK_RESULT
        }
    } else {
        size_t result = neko_capi_vfs_fread(_data_buffer, sizeof(u8), info.data_size, vf);

        if (result != info.data_size) return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    }

    (*size) = info.data_size;

    char *data = (char *)mem_alloc(info.data_size + 1);
    memcpy((void *)(data), _data_buffer, info.data_size);
    data[info.data_size] = 0;
    *out = {data, info.data_size};

    pak->file_ref_count++;
    return true;
}

bool pak_get_data(Pak *pak, const_str path, String *out, u32 *size) {
    neko_assert(path && out && size);
    neko_assert(strlen(path) <= u8_max);
    u64 index = pak_get_item_index(pak, path);
    if (index == u64_max) return false;  // FAILED_TO_GET_ITEM_PACK_RESULT
    return pak_get_data(pak, index, out, size);
}

void pak_free_item(Pak *pak, String data) {
    mem_free(data.data);
    pak->file_ref_count--;
}

void pak_free_buffer(Pak *pak) {

    mem_free(pak->data_buffer);
    mem_free(pak->zip_buffer);
    pak->data_buffer = NULL;
    pak->zip_buffer = NULL;
}

static void pak_remove_item(u64 item_count, PakItem *pack_items) {
    neko_assert(item_count == 0 || (item_count > 0 && pack_items));
    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

bool neko_pak_unzip(const_str file_path, bool print_progress) {
    neko_assert(file_path);

    Pak pak;

    int pack_result = pak_load(&pak, file_path, 128, false);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pak.item_count;
    PakItem *items = pak.items;

    for (u64 i = 0; i < item_count; i++) {
        PakItem *item = &items[i];

        if (print_progress) {
            console_log("Unpacking %s", item->path);
        }

        String data;
        u32 data_size;

        pack_result = pak_get_data(&pak, i, &data, &data_size);

        if (pack_result != 0) {
            pak_remove_item(i, items);
            pak_fini(&pak);
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
            pak_remove_item(i, items);
            pak_fini(&pak);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        size_t result = fwrite(data.data, sizeof(u8), data_size, item_file);

        neko_fclose(item_file);

        if (result != data_size) {
            pak_remove_item(i, items);
            pak_fini(&pak);
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

    pak_fini(&pak);

    if (print_progress) {
        neko_printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return true;
}

bool neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, bool print_progress) {
    neko_assert(pack_file);
    neko_assert(item_count > 0);
    neko_assert(item_paths);

    u32 buffer_size = 128;  // 提高初始缓冲大小

    u8 *item_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    u8 *zip_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    if (!zip_data) {
        mem_free(item_data);
        return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
    }

    u64 total_zip_size = 0, total_raw_size = 0;

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (print_progress) {
            neko_printf("Packing \"%s\" file. ", item_path);
            fflush(stdout);
        }

        size_t path_size = strlen(item_path);

        if (path_size > u8_max) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        FILE *item_file = neko_fopen(item_path, "rb");

        if (!item_file) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        int seek_result = neko_fseek(item_file, 0, SEEK_END);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        u64 item_size = (u64)neko_ftell(item_file);

        if (item_size == 0 || item_size > UINT32_MAX) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        seek_result = neko_fseek(item_file, 0, SEEK_SET);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        if (item_size > buffer_size) {
            u8 *new_buffer = (u8 *)mem_realloc(item_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
            }

            item_data = new_buffer;

            new_buffer = (u8 *)mem_realloc(zip_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
            }

            zip_data = new_buffer;
        }

        size_t result = neko_fread(item_data, sizeof(u8), item_size, item_file);

        neko_fclose(item_file);

        if (result != item_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        size_t zip_size;

        if (item_size > 1) {

            unsigned long compress_buf_len = compressBound(item_size);
            int compress_status = compress(zip_data, &compress_buf_len, (const unsigned char *)item_data, item_size);

            zip_size = compress_buf_len;

            // const int max_dst_size = neko_lz_bounds(item_size, 0);
            // zip_size = neko_lz_encode(item_data, item_size, zip_data, max_dst_size, 9);

            if (zip_size <= 0 || zip_size >= item_size || compress_status != MZ_OK) {
                zip_size = 0;
            }
        } else {
            zip_size = 0;
        }

        i64 file_offset = neko_ftell(pack_file);

        PakItemInfo info = {
                (u32)zip_size,
                (u32)item_size,
                (u64)file_offset,
                (u8)path_size,
        };

        result = fwrite(&info, sizeof(PakItemInfo), 1, pack_file);

        if (result != 1) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        result = fwrite(item_path, sizeof(char), info.path_size, pack_file);

        if (result != info.path_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, pack_file);

            if (result != zip_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        } else {
            result = fwrite(item_data, sizeof(u8), item_size, pack_file);

            if (result != item_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        }

        if (print_progress) {
            u32 zip_file_size = zip_size > 0 ? (u32)zip_size : (u32)item_size;
            u32 raw_file_size = (u32)item_size;

            total_zip_size += zip_file_size;
            total_raw_size += raw_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", zip_file_size, raw_file_size, progress);
            fflush(stdout);
        }
    }

    mem_free(zip_data);
    mem_free(item_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        neko_printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size,
                    compression);
    }

    return true;
}

bool neko_pak_build(const_str file_path, u64 file_count, const_str *file_paths, bool print_progress) {
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

    qsort(item_paths, item_count, sizeof(char *), _compare_item_paths);

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

    bool ok = neko_write_pack_items(pack_file, item_count, item_paths, print_progress);

    mem_free(item_paths);
    neko_fclose(pack_file);

    if (!ok) {
        remove(file_path);
        return ok;
    }

    neko_printf("Packed %s ok\n", file_path);

    return true;
}

bool neko_pak_info(const_str file_path, i32 *buildnum, u64 *item_count) {
    neko_assert(file_path);
    neko_assert(buildnum);
    neko_assert(item_count);

    vfs_file vf = neko_capi_vfs_fopen(file_path);

    if (!vf.data) return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT

    char header[NEKO_PAK_HEAD_SIZE];

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &vf);
    result += neko_capi_vfs_fread(buildnum, sizeof(i32), 1, &vf);

    if (result != NEKO_PAK_HEAD_SIZE + 1) {
        neko_capi_vfs_fclose(&vf);
        return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    }

    if (header[0] != 'N' ||  //
        header[1] != 'E' ||  //
        header[2] != 'K' ||  //
        header[3] != 'O' ||  //
        header[4] != 'P' ||  //
        header[5] != 'A' ||  //
        header[6] != 'C' ||  //
        header[7] != 'K') {
        neko_capi_vfs_fclose(&vf);
        return false;  // BAD_FILE_TYPE_PACK_RESULT
    }

    result = neko_capi_vfs_fread(item_count, sizeof(u64), 1, &vf);

    neko_capi_vfs_fclose(&vf);

    if (result != 1) return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    return true;
}

// mt_pak

struct pak_assets_t {
    const_str name;
    size_t size;
    String data;
};

static Pak *check_pak_udata(lua_State *L, i32 arg) {
    Pak *udata = (Pak *)luaL_checkudata(L, arg, "mt_pak");
    if (!udata || !udata->item_count) {
        luaL_error(L, "cannot read pak");
    }
    return udata;
}

static int mt_pak_gc(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);
    pak_fini(pak);
    console_log("pak __gc %p", pak);
    return 0;
}

static int mt_pak_items(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    u64 item_count = pak_get_item_count(pak);

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, pak_get_item_path(pak, i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int mt_pak_assets_load(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    const_str path = lua_tostring(L, 2);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_newuserdata(L, sizeof(pak_assets_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    bool ok = pak_get_data(pak, path, &assets_user_handle->data, (u32 *)&assets_user_handle->size);

    if (!ok) {
        const_str error_message = "mt_pak_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // asset_write(asset);

    return 1;
}

static int mt_pak_assets_unload(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_touserdata(L, 2);

    if (assets_user_handle && assets_user_handle->data.len)
        pak_free_item(pak, assets_user_handle->data);
    else
        console_log("unknown assets unload %p", assets_user_handle);

    // asset_write(asset);

    return 0;
}

int open_mt_pak(lua_State *L) {
    // clang-format off
    luaL_Reg reg[] = {
            {"__gc", mt_pak_gc},
            {"items", mt_pak_items},
            {"assets_load", mt_pak_assets_load},
            {"assets_unload", mt_pak_assets_unload},
            {nullptr, nullptr},
    };
    // clang-format on

    luax_new_class(L, "mt_pak", reg);
    return 0;
}

int neko_pak_load(lua_State *L) {
    String name = luax_check_string(L, 1);
    String path = luax_check_string(L, 2);

    Pak pak;

    bool ok = pak_load(&pak, path.cstr(), 0, false);

    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, pak, "mt_pak");
    return 1;
}