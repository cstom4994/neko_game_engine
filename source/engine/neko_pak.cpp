
#include "engine/neko_pak.h"

#include "engine/neko_app.h"

// miniz
#include <miniz.h>

/*==========================
// NEKO_PACK
==========================*/

static void destroy_pack_items(u64 item_count, neko_pak::item *items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) mem_free(items[i].path);
    mem_free(items);
}

bool create_pack_items(vfs_file *pak, u64 item_count, neko_pak::item **_items) {
    NEKO_ASSERT(pak);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(_items);

    neko_pak::item *items = (neko_pak::item *)mem_alloc(item_count * sizeof(neko_pak::item));

    if (!items) return false;  // FAILED_TO_ALLOCATE_PACK_RESULT

    for (u64 i = 0; i < item_count; i++) {
        neko_pak::iteminfo info;

        size_t result = neko_capi_vfs_fread(&info, sizeof(neko_pak::iteminfo), 1, pak);

        if (result != 1) {
            destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        if (info.data_size == 0 || info.path_size == 0) {
            destroy_pack_items(i, items);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        char *path = (char *)mem_alloc((info.path_size + 1) * sizeof(char));

        if (!path) {
            destroy_pack_items(i, items);
            return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
        }

        result = neko_capi_vfs_fread(path, sizeof(char), info.path_size, pak);

        path[info.path_size] = 0;

        if (result != info.path_size) {
            destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        i64 fileOffset = info.zip_size > 0 ? info.zip_size : info.data_size;

        int seekResult = neko_capi_vfs_fseek(pak, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            destroy_pack_items(i, items);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        neko_pak::item *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return true;
}

bool neko_pak::load(const_str file_path, u32 data_buffer_capacity, bool is_resources_directory) {
    NEKO_ASSERT(file_path);

    // memset(this, 0, sizeof(neko_pak));

    this->zip_buffer = NULL;
    this->zip_size = 0;

    this->vf = neko_capi_vfs_fopen(file_path);

    if (!this->vf.data) return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT

    char header[NEKO_PAK_HEAD_SIZE];
    i32 buildnum;

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &this->vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &this->vf);

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
        this->fini();
        return false;
    }

    u64 item_count;

    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &this->vf);

    if (result != 1 ||  //
        item_count == 0) {
        this->fini();
        return false;
    }

    neko_pak::item *items;

    bool ok = create_pack_items(&this->vf, item_count, &items);

    if (!ok) {
        this->fini();
        return false;
    }

    this->item_count = item_count;
    this->items = items;

    u8 *_data_buffer = NULL;

    if (data_buffer_capacity > 0) {
        _data_buffer = (u8 *)mem_alloc(data_buffer_capacity * sizeof(u8));
    } else {
        _data_buffer = NULL;
    }

    this->data_buffer = _data_buffer;
    this->data_size = data_buffer_capacity;

    NEKO_TRACE("load pack %s buildnum: %d (engine %d)", neko_util_get_filename(file_path), buildnum, neko_buildnum());

    return true;
}
void neko_pak::fini() {
    if (this->file_ref_count != 0) {
        NEKO_WARN("assets loader leaks detected %d refs", this->file_ref_count);
    }

    free_buffer();

    destroy_pack_items(this->item_count, this->items);
    if (this->vf.data) neko_capi_vfs_fclose(&this->vf);
}

static int neko_compare_pack_items(const void *_a, const void *_b) {
    const neko_pak::item *a = (neko_pak::item *)_a;
    const neko_pak::item *b = (neko_pak::item *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

u64 neko_pak::get_item_index(const_str path) {
    NEKO_ASSERT(path);
    NEKO_ASSERT(strlen(path) <= u8_max);

    neko_pak::item *search_item = &this->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    neko_pak::item *items = (neko_pak::item *)bsearch(search_item, this->items, this->item_count, sizeof(neko_pak::item), neko_compare_pack_items);

    if (!items) return u64_max;

    u64 index = items - this->items;
    return index;
}

bool neko_pak::get_data(u64 index, String *out, u32 *size) {
    NEKO_ASSERT((index < this->item_count) && out && size);

    neko_pak::iteminfo info = this->items[index].info;

    u8 *_data_buffer = this->data_buffer;
    if (_data_buffer) {
        if (info.data_size > this->data_size) {
            _data_buffer = (u8 *)mem_realloc(_data_buffer, info.data_size * sizeof(u8));
            this->data_buffer = _data_buffer;
            this->data_size = info.data_size;
        }
    } else {
        _data_buffer = (u8 *)mem_alloc(info.data_size * sizeof(u8));
        this->data_buffer = _data_buffer;
        this->data_size = info.data_size;
    }

    u8 *_zip_buffer = this->zip_buffer;
    if (this->zip_buffer) {
        if (info.zip_size > this->zip_size) {
            _zip_buffer = (u8 *)mem_realloc(this->zip_buffer, info.zip_size * sizeof(u8));
            this->zip_buffer = _zip_buffer;
            this->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            _zip_buffer = (u8 *)mem_alloc(info.zip_size * sizeof(u8));
            this->zip_buffer = _zip_buffer;
            this->zip_size = info.zip_size;
        }
    }

    vfs_file *vf = &this->vf;

    i64 file_offset = (i64)(info.file_offset + sizeof(neko_pak::iteminfo) + info.path_size);

    int seek_result = neko_capi_vfs_fseek(vf, file_offset, SEEK_SET);

    if (seek_result != 0) return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT

    if (info.zip_size > 0) {
        size_t result = neko_capi_vfs_fread(_zip_buffer, sizeof(u8), info.zip_size, vf);

        if (result != info.zip_size) return false;  // FAILED_TO_READ_FILE_PACK_RESULT

        // result = neko_lz_decode(_zip_buffer, info.zip_size, _data_buffer, info.data_size);

        unsigned long decompress_buf_len = info.data_size;
        int decompress_status = uncompress(_data_buffer, &decompress_buf_len, _zip_buffer, info.zip_size);

        result = decompress_buf_len;

        NEKO_TRACE("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

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

    this->file_ref_count++;
    return true;
}

bool neko_pak::get_data(const_str path, String *out, u32 *size) {
    NEKO_ASSERT(path && out && size);
    NEKO_ASSERT(strlen(path) <= u8_max);
    u64 index = this->get_item_index(path);
    if (index == u64_max) return false;  // FAILED_TO_GET_ITEM_PACK_RESULT
    return this->get_data(index, out, size);
}

void neko_pak::free_item(String data) {
    mem_free(data.data);
    this->file_ref_count--;
}

void neko_pak::free_buffer() {

    mem_free(this->data_buffer);
    mem_free(this->zip_buffer);
    this->data_buffer = NULL;
    this->zip_buffer = NULL;
}

static void neko_pak_remove_item(u64 item_count, neko_pak::item *pack_items) {
    NEKO_ASSERT(item_count == 0 || (item_count > 0 && pack_items));
    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

bool neko_pak_unzip(const_str file_path, bool print_progress) {
    NEKO_ASSERT(file_path);

    neko_pak pak;

    int pack_result = pak.load(file_path, 128, false);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pak.item_count;
    neko_pak::item *items = pak.items;

    for (u64 i = 0; i < item_count; i++) {
        neko_pak::item *item = &items[i];

        if (print_progress) {
            NEKO_INFO("Unpacking %s", item->path);
        }

        String data;
        u32 data_size;

        pack_result = pak.get_data(i, &data, &data_size);

        if (pack_result != 0) {
            neko_pak_remove_item(i, items);
            pak.fini();
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
            neko_pak_remove_item(i, items);
            pak.fini();
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        size_t result = fwrite(data.data, sizeof(u8), data_size, item_file);

        neko_fclose(item_file);

        if (result != data_size) {
            neko_pak_remove_item(i, items);
            pak.fini();
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

    pak.fini();

    if (print_progress) {
        neko_printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return true;
}

bool neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, bool print_progress) {
    NEKO_ASSERT(pack_file);
    NEKO_ASSERT(item_count > 0);
    NEKO_ASSERT(item_paths);

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

        neko_pak::iteminfo info = {
                (u32)zip_size,
                (u32)item_size,
                (u64)file_offset,
                (u8)path_size,
        };

        result = fwrite(&info, sizeof(neko_pak::iteminfo), 1, pack_file);

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

static int neko_pak_compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

bool neko_pak_build(const_str file_path, u64 file_count, const_str *file_paths, bool print_progress) {
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(file_count > 0);
    NEKO_ASSERT(file_paths);

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

    qsort(item_paths, item_count, sizeof(char *), neko_pak_compare_item_paths);

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
    NEKO_ASSERT(file_path);
    NEKO_ASSERT(buildnum);
    NEKO_ASSERT(item_count);

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
