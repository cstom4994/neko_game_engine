
#ifndef NEKO_PACKER_H
#define NEKO_PACKER_H

#include "engine/common/neko_util.h"
#include "engine/utility/logger.hpp"

#define PACK_HEADER_SIZE 8
#define PACK_VERSION_MAJOR 0
#define PACK_VERSION_MINOR 0
#define PACK_VERSION_PATCH 1

typedef enum neko_packresult_t {
    SUCCESS_PACK_RESULT = 0,
    FAILED_TO_ALLOCATE_PACK_RESULT = 1,
    FAILED_TO_CREATE_LZ4_PACK_RESULT = 2,
    FAILED_TO_CREATE_FILE_PACK_RESULT = 3,
    FAILED_TO_OPEN_FILE_PACK_RESULT = 4,
    FAILED_TO_WRITE_FILE_PACK_RESULT = 5,
    FAILED_TO_READ_FILE_PACK_RESULT = 6,
    FAILED_TO_SEEK_FILE_PACK_RESULT = 7,
    FAILED_TO_GET_DIRECTORY_PACK_RESULT = 8,
    FAILED_TO_DECOMPRESS_PACK_RESULT = 9,
    FAILED_TO_GET_ITEM_PACK_RESULT = 10,
    BAD_DATA_SIZE_PACK_RESULT = 11,
    BAD_FILE_TYPE_PACK_RESULT = 12,
    BAD_FILE_VERSION_PACK_RESULT = 13,
    BAD_FILE_ENDIANNESS_PACK_RESULT = 14,
    PACK_RESULT_COUNT = 15,
} neko_packresult_t;

typedef u8 neko_pack_result;

typedef struct pack_iteminfo {
    u32 zipSize;
    u32 dataSize;
    u64 fileOffset;
    u8 pathSize;
} pack_iteminfo;

neko_private(const char *const) pack_result_strings[PACK_RESULT_COUNT] = {
        "Success",
        "Failed to allocate",
        "Failed to create LZ4",
        "Failed to create file",
        "Failed to open file",
        "Failed to write file",
        "Failed to read file",
        "Failed to seek file",
        "Failed to get directory",
        "Failed to decompress",
        "Failed to get item",
        "Bad data size",
        "Bad file type",
        "Bad file version",
        "Bad file endianness",
};

inline static const char *__neko_pack_result(neko_pack_result result) {
    if (result >= PACK_RESULT_COUNT) return "Unknown PACK result";
    return pack_result_strings[result];
}

typedef struct neko_packreader_t neko_packreader_t;

neko_pack_result neko_pack_read(const char *filePath, u32 dataBufferCapacity, bool isResourcesDirectory, neko_packreader_t **pack_reader);
void neko_pack_destroy(neko_packreader_t *pack_reader);

u64 neko_pack_item_count(neko_packreader_t *pack_reader);
b8 neko_pack_item_index(neko_packreader_t *pack_reader, const char *path, u64 *index);
u32 neko_pack_item_size(neko_packreader_t *pack_reader, u64 index);
const char *neko_pack_item_path(neko_packreader_t *pack_reader, u64 index);
neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, u64 index, const u8 **data, u32 *size);
neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, const char *path, const u8 **data, u32 *size);
void neko_pack_free_buffers(neko_packreader_t *pack_reader);
neko_pack_result neko_unpack_files(const char *filePath, b8 printProgress);
neko_pack_result neko_pack_files(const char *packPath, u64 fileCount, const char **filePaths, b8 printProgress);

void neko_pack_version(u8 *majorVersion, u8 *minorVersion, u8 *patchVersion);
neko_pack_result neko_pack_info(const char *filePath, u8 *majorVersion, u8 *minorVersion, u8 *patchVersion, b8 *isLittleEndian, u64 *itemCount);

neko_inline bool neko_pack_check(neko_pack_result result) {
    if (result == SUCCESS_PACK_RESULT) return true;
    neko_error(std::format("read pack faild: {0}", __neko_pack_result(result)));
    return false;
}

// 资源管理

struct neko_assets_handle_t {
    const u8 *data;
    u32 size;
};

neko_assets_handle_t neko_assets_get(neko_packreader_t *pack_reader, const neko_string &path);

struct neko_lua_handle_t {
    const_str name;
    u64 size;
    void *data;
};

#endif