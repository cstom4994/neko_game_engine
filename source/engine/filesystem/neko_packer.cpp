
#include "neko_packer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/common/neko_mem.h"
#include "engine/common/neko_util.h"
#include "engine/utility/logger.hpp"
#include "libs/lz4/lz4.h"

inline static FILE *openFile(const char *filePath, const char *mode) {
    FILE *file;
    errno_t error = fopen_s(&file, filePath, mode);
    if (error != 0) return NULL;
    return file;
}

#define seekFile(file, offset, whence) _fseeki64(file, offset, whence)
#define tellFile(file) _ftelli64(file)
#define closeFile(file) fclose(file)

typedef struct pack_item {
    pack_iteminfo info;
    char *path;
} pack_item;

struct neko_packreader_t {
    FILE *file;
    u64 itemCount;
    pack_item *items;
    u8 *dataBuffer;
    u8 *zipBuffer;
    u32 dataSize;
    u32 zipSize;
    pack_item searchItem;
};

neko_private(void) destroy_pack_items(u64 itemCount, pack_item *items) {
    neko_assert(itemCount == 0 || (itemCount > 0 && items));

    for (u64 i = 0; i < itemCount; i++) neko_safe_free(items[i].path);
    neko_safe_free(items);
}

neko_private(neko_pack_result) create_pack_items(FILE *packFile, u64 itemCount, pack_item **_items) {
    neko_assert(packFile);
    neko_assert(itemCount > 0);
    neko_assert(_items);

    pack_item *items = (pack_item *)neko_safe_malloc(itemCount * sizeof(pack_item));

    if (!items) return FAILED_TO_ALLOCATE_PACK_RESULT;

    for (u64 i = 0; i < itemCount; i++) {
        pack_iteminfo info;

        size_t result = fread(&info, sizeof(pack_iteminfo), 1, packFile);

        if (result != 1) {
            destroy_pack_items(i, items);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        if (info.dataSize == 0 || info.pathSize == 0) {
            destroy_pack_items(i, items);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        char *path = (char *)neko_safe_malloc((info.pathSize + 1) * sizeof(char));

        if (!path) {
            destroy_pack_items(i, items);
            return FAILED_TO_ALLOCATE_PACK_RESULT;
        }

        result = fread(path, sizeof(char), info.pathSize, packFile);

        path[info.pathSize] = 0;

        if (result != info.pathSize) {
            destroy_pack_items(i, items);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        int64_t fileOffset = info.zipSize > 0 ? info.zipSize : info.dataSize;

        int seekResult = seekFile(packFile, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            destroy_pack_items(i, items);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        pack_item *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return SUCCESS_PACK_RESULT;
}

neko_pack_result neko_pack_read(const char *filePath, u32 dataBufferCapacity, bool isResourcesDirectory, neko_packreader_t **pack_reader) {
    neko_assert(filePath);
    neko_assert(pack_reader);

    neko_packreader_t *pack = (neko_packreader_t *)neko_safe_calloc(1, sizeof(neko_packreader_t));

    if (!pack) return FAILED_TO_ALLOCATE_PACK_RESULT;

    pack->zipBuffer = NULL;
    pack->zipSize = 0;

    char *path;

    path = (char *)filePath;

    FILE *file = openFile(path, "rb");

    if (!file) {
        neko_pack_destroy(pack);
        return FAILED_TO_OPEN_FILE_PACK_RESULT;
    }

    pack->file = file;

    char header[PACK_HEADER_SIZE];

    size_t result = fread(header, sizeof(char), PACK_HEADER_SIZE, file);

    if (result != PACK_HEADER_SIZE) {
        neko_pack_destroy(pack);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        neko_pack_destroy(pack);
        return BAD_FILE_TYPE_PACK_RESULT;
    }

    if (header[4] != PACK_VERSION_MAJOR || header[5] != PACK_VERSION_MINOR) {
        neko_pack_destroy(pack);
        return BAD_FILE_VERSION_PACK_RESULT;
    }

    // Skipping PATCH version check

    if (header[7] != !neko_little_endian) {
        neko_pack_destroy(pack);
        return BAD_FILE_ENDIANNESS_PACK_RESULT;
    }

    u64 itemCount;

    result = fread(&itemCount, sizeof(u64), 1, file);

    if (result != 1) {
        neko_pack_destroy(pack);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (itemCount == 0) {
        neko_pack_destroy(pack);
        return BAD_DATA_SIZE_PACK_RESULT;
    }

    pack_item *items;

    neko_pack_result packResult = create_pack_items(file, itemCount, &items);

    if (packResult != SUCCESS_PACK_RESULT) {
        neko_pack_destroy(pack);
        ;
        return packResult;
    }

    pack->itemCount = itemCount;
    pack->items = items;

    u8 *dataBuffer;

    if (dataBufferCapacity > 0) {
        dataBuffer = (u8 *)neko_safe_malloc(dataBufferCapacity * sizeof(u8));

        if (!dataBuffer) {
            neko_pack_destroy(pack);
            return FAILED_TO_ALLOCATE_PACK_RESULT;
        }
    } else {
        dataBuffer = NULL;
    }

    pack->dataBuffer = dataBuffer;
    pack->dataSize = dataBufferCapacity;

    *pack_reader = pack;
    return SUCCESS_PACK_RESULT;
}
void neko_pack_destroy(neko_packreader_t *pack_reader) {
    if (!pack_reader) return;

    neko_safe_free(pack_reader->dataBuffer);
    neko_safe_free(pack_reader->zipBuffer);
    destroy_pack_items(pack_reader->itemCount, pack_reader->items);
    if (pack_reader->file) closeFile(pack_reader->file);
    neko_safe_free(pack_reader);
}

u64 neko_pack_item_count(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    return pack_reader->itemCount;
}

neko_private(int) neko_compare_pack_items(const void *_a, const void *_b) {
    // NOTE: a and b should not be NULL!
    // Skipping here neko_assertions for debug build speed.

    const pack_item *a = (pack_item *)_a;
    const pack_item *b = (pack_item *)_b;

    int difference = (int)a->info.pathSize - (int)b->info.pathSize;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.pathSize * sizeof(char));
}

b8 neko_pack_item_index(neko_packreader_t *pack_reader, const char *path, u64 *index) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(index);
    neko_assert(strlen(path) <= UINT8_MAX);

    pack_item *searchItem = &pack_reader->searchItem;

    searchItem->info.pathSize = (u8)strlen(path);
    searchItem->path = (char *)path;

    pack_item *item = (pack_item *)bsearch(searchItem, pack_reader->items, pack_reader->itemCount, sizeof(pack_item), neko_compare_pack_items);

    if (!item) return false;

    *index = item - pack_reader->items;
    return true;
}

u32 neko_pack_item_size(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    return pack_reader->items[index].info.dataSize;
}

const char *neko_pack_item_path(neko_packreader_t *pack_reader, u64 index) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    return pack_reader->items[index].path;
}

neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, u64 index, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(index < pack_reader->itemCount);
    neko_assert(data);
    neko_assert(size);

    pack_iteminfo info = pack_reader->items[index].info;
    u8 *dataBuffer = pack_reader->dataBuffer;

    if (dataBuffer) {
        if (info.dataSize > pack_reader->dataSize) {
            dataBuffer = (u8 *)neko_safe_realloc(dataBuffer, info.dataSize * sizeof(u8));

            if (!dataBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->dataBuffer = dataBuffer;
            pack_reader->dataSize = info.dataSize;
        }
    } else {
        dataBuffer = (u8 *)neko_safe_malloc(info.dataSize * sizeof(u8));

        if (!dataBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

        pack_reader->dataBuffer = dataBuffer;
        pack_reader->dataSize = info.dataSize;
    }

    u8 *zipBuffer = pack_reader->zipBuffer;

    if (zipBuffer) {
        if (info.zipSize > pack_reader->zipSize) {
            zipBuffer = (u8 *)neko_safe_realloc(zipBuffer, info.zipSize * sizeof(u8));

            if (!zipBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->zipBuffer = zipBuffer;
            pack_reader->zipSize = info.zipSize;
        }
    } else {
        if (info.zipSize > 0) {
            zipBuffer = (u8 *)neko_safe_malloc(info.zipSize * sizeof(u8));

            if (!zipBuffer) return FAILED_TO_ALLOCATE_PACK_RESULT;

            pack_reader->zipBuffer = zipBuffer;
            pack_reader->zipSize = info.zipSize;
        }
    }

    FILE *file = pack_reader->file;

    int64_t fileOffset = (int64_t)(info.fileOffset + sizeof(pack_iteminfo) + info.pathSize);

    int seekResult = seekFile(file, fileOffset, SEEK_SET);

    if (seekResult != 0) return FAILED_TO_SEEK_FILE_PACK_RESULT;

    if (info.zipSize > 0) {
        size_t result = fread(zipBuffer, sizeof(u8), info.zipSize, file);

        if (result != info.zipSize) return FAILED_TO_READ_FILE_PACK_RESULT;

        result = LZ4_decompress_safe((char *)zipBuffer, (char *)dataBuffer, info.zipSize, info.dataSize);

        // METADOT_BUG("[Assets] LZ4_decompress_safe ", result, " ", info.dataSize);

        if (result < 0 || result != info.dataSize) {
            return FAILED_TO_DECOMPRESS_PACK_RESULT;
        }
    } else {
        size_t result = fread(dataBuffer, sizeof(u8), info.dataSize, file);

        if (result != info.dataSize) return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    *data = dataBuffer;
    *size = info.dataSize;
    return SUCCESS_PACK_RESULT;
}

neko_pack_result neko_pack_item_data(neko_packreader_t *pack_reader, const char *path, const u8 **data, u32 *size) {
    neko_assert(pack_reader);
    neko_assert(path);
    neko_assert(data);
    neko_assert(size);
    neko_assert(strlen(path) <= UINT8_MAX);

    u64 index;

    if (!neko_pack_item_index(pack_reader, path, &index)) {
        return FAILED_TO_GET_ITEM_PACK_RESULT;
    }

    return neko_pack_item_data(pack_reader, index, data, size);
}

void neko_pack_free_buffers(neko_packreader_t *pack_reader) {
    neko_assert(pack_reader);
    neko_safe_free(pack_reader->dataBuffer);
    neko_safe_free(pack_reader->zipBuffer);
    pack_reader->dataBuffer = NULL;
    pack_reader->zipBuffer = NULL;
}

neko_private(void) neko_removePackItemFiles(u64 itemCount, pack_item *packItems) {
    neko_assert(itemCount == 0 || (itemCount > 0 && packItems));

    for (u64 i = 0; i < itemCount; i++) remove(packItems[i].path);
}

neko_pack_result neko_unpack_files(const char *filePath, b8 printProgress) {
    neko_assert(filePath);

    neko_packreader_t *pack_reader;

    neko_pack_result packResult = neko_pack_read(filePath, 0, false, &pack_reader);

    if (packResult != SUCCESS_PACK_RESULT) return packResult;

    u64 totalRawSize = 0, totalZipSize = 0;

    u64 itemCount = pack_reader->itemCount;
    pack_item *items = pack_reader->items;

    for (u64 i = 0; i < itemCount; i++) {
        pack_item *item = &items[i];

        if (printProgress) {
            // METADOT_BUG("Unpacking ", item->path);
        }

        const u8 *dataBuffer;
        u32 dataSize;

        packResult = neko_pack_item_data(pack_reader, i, &dataBuffer, &dataSize);

        if (packResult != SUCCESS_PACK_RESULT) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return packResult;
        }

        u8 pathSize = item->info.pathSize;

        char itemPath[UINT8_MAX + 1];

        memcpy(itemPath, item->path, pathSize * sizeof(char));
        itemPath[pathSize] = 0;

        for (u8 j = 0; j < pathSize; j++) {
            if (itemPath[j] == '/' || itemPath[j] == '\\') {
                itemPath[j] = '-';
            }
        }

        FILE *itemFile = openFile(itemPath, "wb");

        if (!itemFile) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        size_t result = fwrite(dataBuffer, sizeof(u8), dataSize, itemFile);

        closeFile(itemFile);

        if (result != dataSize) {
            neko_removePackItemFiles(i, items);
            neko_pack_destroy(pack_reader);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        if (printProgress) {
            u32 rawFileSize = item->info.dataSize;
            u32 zipFileSize = item->info.zipSize > 0 ? item->info.zipSize : item->info.dataSize;

            totalRawSize += rawFileSize;
            totalZipSize += zipFileSize;

            int progress = (int)(((float)(i + 1) / (float)itemCount) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", rawFileSize, zipFileSize, progress);
            fflush(stdout);
        }
    }

    neko_pack_destroy(pack_reader);

    if (printProgress) {
        printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)itemCount, (long long unsigned int)totalRawSize, (long long unsigned int)totalZipSize);
    }

    return SUCCESS_PACK_RESULT;
}

neko_private(neko_pack_result) neko_write_pack_items(FILE *packFile, u64 itemCount, char **itemPaths, b8 printProgress) {
    neko_assert(packFile);
    neko_assert(itemCount > 0);
    neko_assert(itemPaths);

    u32 bufferSize = 1;

    u8 *itemData = (u8 *)neko_safe_malloc(sizeof(u8));

    if (!itemData) return FAILED_TO_ALLOCATE_PACK_RESULT;

    u8 *zipData = (u8 *)neko_safe_malloc(sizeof(u8));

    if (!zipData) {
        neko_safe_free(itemData);
        return FAILED_TO_ALLOCATE_PACK_RESULT;
    }

    u64 totalZipSize = 0, totalRawSize = 0;

    for (u64 i = 0; i < itemCount; i++) {
        char *itemPath = itemPaths[i];

        if (printProgress) {
            printf("Packing \"%s\" file. ", itemPath);
            fflush(stdout);
        }

        size_t pathSize = strlen(itemPath);

        if (pathSize > UINT8_MAX) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        FILE *itemFile = openFile(itemPath, "rb");

        if (!itemFile) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_OPEN_FILE_PACK_RESULT;
        }

        int seekResult = seekFile(itemFile, 0, SEEK_END);

        if (seekResult != 0) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        u64 itemSize = (u64)tellFile(itemFile);

        if (itemSize == 0 || itemSize > UINT32_MAX) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return BAD_DATA_SIZE_PACK_RESULT;
        }

        seekResult = seekFile(itemFile, 0, SEEK_SET);

        if (seekResult != 0) {
            closeFile(itemFile);
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_SEEK_FILE_PACK_RESULT;
        }

        if (itemSize > bufferSize) {
            u8 *newBuffer = (u8 *)neko_safe_realloc(itemData, itemSize * sizeof(u8));

            if (!newBuffer) {
                closeFile(itemFile);
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_ALLOCATE_PACK_RESULT;
            }

            itemData = newBuffer;

            newBuffer = (u8 *)neko_safe_realloc(zipData, itemSize * sizeof(u8));

            if (!newBuffer) {
                closeFile(itemFile);
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_ALLOCATE_PACK_RESULT;
            }

            zipData = newBuffer;
        }

        size_t result = fread(itemData, sizeof(u8), itemSize, itemFile);

        closeFile(itemFile);

        if (result != itemSize) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_READ_FILE_PACK_RESULT;
        }

        size_t zipSize;

        if (itemSize > 1) {

            const int max_dst_size = LZ4_compressBound(itemSize);

            zipSize = LZ4_compress_fast((char *)itemData, (char *)zipData, itemSize, max_dst_size, 10);

            if (zipSize <= 0 || zipSize >= itemSize) {
                zipSize = 0;
            }
        } else {
            zipSize = 0;
        }

        int64_t fileOffset = tellFile(packFile);

        pack_iteminfo info = {
                (u32)zipSize,
                (u32)itemSize,
                (u64)fileOffset,
                (u8)pathSize,
        };

        result = fwrite(&info, sizeof(pack_iteminfo), 1, packFile);

        if (result != 1) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_WRITE_FILE_PACK_RESULT;
        }

        result = fwrite(itemPath, sizeof(char), info.pathSize, packFile);

        if (result != info.pathSize) {
            neko_safe_free(zipData);
            neko_safe_free(itemData);
            return FAILED_TO_WRITE_FILE_PACK_RESULT;
        }

        if (zipSize > 0) {
            result = fwrite(zipData, sizeof(u8), zipSize, packFile);

            if (result != zipSize) {
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_WRITE_FILE_PACK_RESULT;
            }
        } else {
            result = fwrite(itemData, sizeof(u8), itemSize, packFile);

            if (result != itemSize) {
                neko_safe_free(zipData);
                neko_safe_free(itemData);
                return FAILED_TO_WRITE_FILE_PACK_RESULT;
            }
        }

        if (printProgress) {
            u32 zipFileSize = zipSize > 0 ? (u32)zipSize : (u32)itemSize;
            u32 rawFileSize = (u32)itemSize;

            totalZipSize += zipFileSize;
            totalRawSize += rawFileSize;

            int progress = (int)(((float)(i + 1) / (float)itemCount) * 100.0f);

            printf("(%u/%u bytes) [%d%%]\n", zipFileSize, rawFileSize, progress);
            fflush(stdout);
        }
    }

    neko_safe_free(zipData);
    neko_safe_free(itemData);

    if (printProgress) {
        int compression = (int)((1.0 - (double)(totalZipSize) / (double)totalRawSize) * 100.0);
        printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)itemCount, (long long unsigned int)totalZipSize, (long long unsigned int)totalRawSize, compression);
    }

    return SUCCESS_PACK_RESULT;
}

neko_private(int) neko_comparePackItemPaths(const void *_a, const void *_b) {
    // NOTE: a and b should not be NULL!
    // Skipping here neko_assertions for debug build speed.

    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);

    int difference = al - bl;

    if (difference != 0) return difference;

    return memcmp(a, b, al * sizeof(u8));
}

neko_pack_result neko_pack_files(const char *filePath, u64 fileCount, const char **filePaths, b8 printProgress) {
    neko_assert(filePath);
    neko_assert(fileCount > 0);
    neko_assert(filePaths);

    char **itemPaths = (char **)neko_safe_malloc(fileCount * sizeof(char *));

    if (!itemPaths) return FAILED_TO_ALLOCATE_PACK_RESULT;

    u64 itemCount = 0;

    for (u64 i = 0; i < fileCount; i++) {
        b8 alreadyAdded = false;

        for (u64 j = 0; j < itemCount; j++) {
            if (i != j && strcmp(filePaths[i], itemPaths[j]) == 0) alreadyAdded = true;
        }

        if (!alreadyAdded) itemPaths[itemCount++] = (char *)filePaths[i];
    }

    qsort(itemPaths, itemCount, sizeof(char *), neko_comparePackItemPaths);

    FILE *packFile = openFile(filePath, "wb");

    if (!packFile) {
        neko_safe_free(itemPaths);
        return FAILED_TO_CREATE_FILE_PACK_RESULT;
    }

    char header[PACK_HEADER_SIZE] = {
            'P', 'A', 'C', 'K', PACK_VERSION_MAJOR, PACK_VERSION_MINOR, PACK_VERSION_PATCH, !neko_little_endian,
    };

    size_t writeResult = fwrite(header, sizeof(char), PACK_HEADER_SIZE, packFile);

    if (writeResult != PACK_HEADER_SIZE) {
        neko_safe_free(itemPaths);
        closeFile(packFile);
        remove(filePath);
        return FAILED_TO_WRITE_FILE_PACK_RESULT;
    }

    writeResult = fwrite(&itemCount, sizeof(u64), 1, packFile);

    if (writeResult != 1) {
        neko_safe_free(itemPaths);
        closeFile(packFile);
        remove(filePath);
        return FAILED_TO_WRITE_FILE_PACK_RESULT;
    }

    neko_pack_result packResult = neko_write_pack_items(packFile, itemCount, itemPaths, printProgress);

    neko_safe_free(itemPaths);
    closeFile(packFile);

    if (packResult != SUCCESS_PACK_RESULT) {
        remove(filePath);
        return packResult;
    }

    return SUCCESS_PACK_RESULT;
}

void neko_pack_version(u8 *majorVersion, u8 *minorVersion, u8 *patchVersion) {
    neko_assert(majorVersion);
    neko_assert(minorVersion);
    neko_assert(patchVersion);

    *majorVersion = PACK_VERSION_MAJOR;
    *minorVersion = PACK_VERSION_MINOR;
    *patchVersion = PACK_VERSION_PATCH;
}

neko_pack_result neko_pack_info(const char *filePath, u8 *majorVersion, u8 *minorVersion, u8 *patchVersion, b8 *isLittleEndian, u64 *_itemCount) {
    neko_assert(filePath);
    neko_assert(majorVersion);
    neko_assert(minorVersion);
    neko_assert(patchVersion);
    neko_assert(isLittleEndian);
    neko_assert(_itemCount);

    FILE *file = openFile(filePath, "rb");

    if (!file) return FAILED_TO_OPEN_FILE_PACK_RESULT;

    char header[PACK_HEADER_SIZE];

    size_t result = fread(header, sizeof(char), PACK_HEADER_SIZE, file);

    if (result != PACK_HEADER_SIZE) {
        closeFile(file);
        return FAILED_TO_READ_FILE_PACK_RESULT;
    }

    if (header[0] != 'P' || header[1] != 'A' || header[2] != 'C' || header[3] != 'K') {
        closeFile(file);
        return BAD_FILE_TYPE_PACK_RESULT;
    }

    u64 itemCount;

    result = fread(&itemCount, sizeof(u64), 1, file);

    closeFile(file);

    if (result != 1) return FAILED_TO_READ_FILE_PACK_RESULT;

    *majorVersion = header[4];
    *minorVersion = header[5];
    *patchVersion = header[6];
    *isLittleEndian = !header[7];
    *_itemCount = itemCount;
    return SUCCESS_PACK_RESULT;
}

neko_assets_handle_t neko_assets_get(neko_packreader_t *pack_reader, const neko_string &path) {
    neko_pack_result pack_result;
    neko_assets_handle_t handle;
    pack_result = neko_pack_item_data(pack_reader, path.c_str(), &handle.data, &handle.size);
    if (pack_result != SUCCESS_PACK_RESULT) {
        neko_pack_destroy(pack_reader);
        neko_error(std::format("get assets \"{0}\" failed: {1}", path, pack_result));
        neko_assert(0);
    }
    neko_assert(handle.data && handle.size);
    return handle;
}