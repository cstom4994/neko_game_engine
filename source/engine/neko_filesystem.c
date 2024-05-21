
#include "neko_filesystem.h"

// NEKO_FILEWATCH_IMPL

#ifndef NEKO_FILEWATCH_IMPL_ONCE
#define NEKO_FILEWATCH_IMPL_ONCE

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#define NEKO_FILEWATCH_MALLOC(size, ctx) malloc(size)
#define NEKO_FILEWATCH_FREE(ptr, ctx) free(ptr)
#define NEKO_FILEWATCH_MEMCPY(dst, src, n) memcpy(dst, src, n)
#define NEKO_FILEWATCH_MEMSET(ptr, val, n) memset(ptr, val, n)
#define NEKO_FILEWATCH_STRLEN(str) (int)strlen(str)
#define NEKO_FILEWATCH_ASSERT(condition) assert(condition)

#define NEKO_FILES_IMPLEMENTATION

#if !defined(NEKO_FILES_H)

#define NEKO_FILES_WINDOWS 1
#define NEKO_FILES_MAC 2
#define NEKO_FILES_UNIX 3

#if defined(_WIN32)
#define NEKO_FILES_PLATFORM NEKO_FILES_WINDOWS
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#elif defined(__APPLE__)
#define NEKO_FILES_PLATFORM NEKO_FILES_MAC
#else
#define NEKO_FILES_PLATFORM NEKO_FILES_UNIX
#endif

#include <string.h>

#define NEKO_FILES_DEBUG_CHECKS 1

#if NEKO_FILES_DEBUG_CHECKS

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#define NEKO_FILES_ASSERT assert

#else

#define NEKO_FILES_ASSERT(...)

#endif

const char* neko_file_get_ext(neko_file_file_t* file);
void neko_file_traverse(const char* path, neko_file_callback_t* cb, void* udata);
int neko_file_read_file(neko_file_dir_t* dir, neko_file_file_t* file);
void neko_file_dir_next(neko_file_dir_t* dir);
void neko_file_dir_close(neko_file_dir_t* dir);
int neko_file_dir_open(neko_file_dir_t* dir, const char* path);
int neko_file_compare_file_times_by_path(const char* path_a, const char* path_b);
int neko_file_get_file_time(const char* path, neko_file_time_t* time);
int neko_file_compare_file_times(neko_file_time_t* time_a, neko_file_time_t* time_b);
int neko_file_file_exists(const char* path);
int neko_file_match_ext(neko_file_file_t* file, const char* ext);
void neko_file_do_unit_tests();

#define NEKO_FILES_H
#endif

#ifdef NEKO_FILES_IMPLEMENTATION
#ifndef NEKO_FILES_IMPLEMENTATION_ONCE
#define NEKO_FILES_IMPLEMENTATION_ONCE

#define neko_file_safe_strcpy(dst, src, n, max) neko_file_safe_strcpy_internal(dst, src, n, max, __FILE__, __LINE__)
static int neko_file_safe_strcpy_internal(char* dst, const char* src, int n, int max, const char* file, int line) {
    int c;
    const char* original = src;

    do {
        if (n >= max) {
            if (!NEKO_FILES_DEBUG_CHECKS) break;
            printf("ERROR: String \"%s\" too long to copy on line %d in file %s (max length of %d).\n", original, line, file, max);
            NEKO_FILES_ASSERT(0);
        }

        c = *src++;
        dst[n] = c;
        ++n;
    } while (c);

    return n;
}

const char* neko_file_get_ext(neko_file_file_t* file) {
    char* period = file->name;
    char c;
    while ((c = *period++))
        if (c == '.') break;
    if (c)
        neko_file_safe_strcpy(file->ext, period, 0, NEKO_FILES_MAX_EXT);
    else
        file->ext[0] = 0;
    return file->ext;
}

void neko_file_traverse(const char* path, neko_file_callback_t* cb, void* udata) {
    neko_file_dir_t dir;
    neko_file_dir_open(&dir, path);

    while (dir.has_next) {
        neko_file_file_t file;
        neko_file_read_file(&dir, &file);

        if (file.is_dir && file.name[0] != '.') {
            char path2[NEKO_FILES_MAX_PATH];
            int n = neko_file_safe_strcpy(path2, path, 0, NEKO_FILES_MAX_PATH);
            n = neko_file_safe_strcpy(path2, "/", n - 1, NEKO_FILES_MAX_PATH);
            neko_file_safe_strcpy(path2, file.name, n - 1, NEKO_FILES_MAX_PATH);
            neko_file_traverse(path2, cb, udata);
        }

        if (file.is_reg) cb(&file, udata);
        neko_file_dir_next(&dir);
    }

    neko_file_dir_close(&dir);
}

int neko_file_match_ext(neko_file_file_t* file, const char* ext) {
    if (*ext == '.') ++ext;
    return !strcmp(file->ext, ext);
}

#if NEKO_FILES_PLATFORM == NEKO_FILES_WINDOWS

int neko_file_read_file(neko_file_dir_t* dir, neko_file_file_t* file) {
    NEKO_FILES_ASSERT(dir->handle != INVALID_HANDLE_VALUE);

    int n = 0;
    char* fpath = file->path;
    char* dpath = dir->path;

    n = neko_file_safe_strcpy(fpath, dpath, 0, NEKO_FILES_MAX_PATH);
    n = neko_file_safe_strcpy(fpath, "/", n - 1, NEKO_FILES_MAX_PATH);

    char* dname = dir->fdata.cFileName;
    char* fname = file->name;

    neko_file_safe_strcpy(fname, dname, 0, NEKO_FILES_MAX_FILENAME);
    neko_file_safe_strcpy(fpath, fname, n - 1, NEKO_FILES_MAX_PATH);

    size_t max_dword = MAXDWORD;
    file->size = ((size_t)dir->fdata.nFileSizeHigh * (max_dword + 1)) + (size_t)dir->fdata.nFileSizeLow;
    neko_file_get_ext(file);

    file->is_dir = !!(dir->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    file->is_reg = !!(dir->fdata.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) || !(dir->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    return 1;
}

void neko_file_dir_next(neko_file_dir_t* dir) {
    NEKO_FILES_ASSERT(dir->has_next);

    if (!FindNextFileA(dir->handle, &dir->fdata)) {
        dir->has_next = 0;
        DWORD err = GetLastError();
        NEKO_FILES_ASSERT(err == ERROR_SUCCESS || err == ERROR_NO_MORE_FILES);
    }
}

void neko_file_dir_close(neko_file_dir_t* dir) {
    dir->path[0] = 0;
    dir->has_next = 0;
    if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
}

int neko_file_dir_open(neko_file_dir_t* dir, const char* path) {
    int n = neko_file_safe_strcpy(dir->path, path, 0, NEKO_FILES_MAX_PATH);
    n = neko_file_safe_strcpy(dir->path, "\\*", n - 1, NEKO_FILES_MAX_PATH);
    dir->handle = FindFirstFileA(dir->path, &dir->fdata);
    dir->path[n - 3] = 0;

    if (dir->handle == INVALID_HANDLE_VALUE) {
        printf("ERROR: Failed to open directory (%s): %s.\n", path, strerror(errno));
        neko_file_dir_close(dir);
        NEKO_FILES_ASSERT(0);
        return 0;
    }

    dir->has_next = 1;

    return 1;
}

int neko_file_compare_file_times_by_path(const char* path_a, const char* path_b) {
    FILETIME time_a = {0};
    FILETIME time_b = {0};
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesExA(path_a, GetFileExInfoStandard, &data)) time_a = data.ftLastWriteTime;
    if (GetFileAttributesExA(path_b, GetFileExInfoStandard, &data)) time_b = data.ftLastWriteTime;
    return CompareFileTime(&time_a, &time_b);
}

int neko_file_get_file_time(const char* path, neko_file_time_t* time) {
    FILETIME initialized_to_zero = {0};
    time->time = initialized_to_zero;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &data)) {
        time->time = data.ftLastWriteTime;
        return 1;
    }
    return 0;
}

int neko_file_compare_file_times(neko_file_time_t* time_a, neko_file_time_t* time_b) { return CompareFileTime(&time_a->time, &time_b->time); }

int neko_file_file_exists(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA unused;
    return GetFileAttributesExA(path, GetFileExInfoStandard, &unused);
}

#elif NEKO_FILES_PLATFORM == NEKO_FILES_MAC || NEKO_FILES_PLATFORM == NEKO_FILES_UNIX

int neko_file_read_file(neko_file_dir_t* dir, neko_file_file_t* file) {
    NEKO_FILES_ASSERT(dir->entry);

    int n = 0;
    char* fpath = file->path;
    char* dpath = dir->path;

    n = neko_file_safe_strcpy(fpath, dpath, 0, NEKO_FILES_MAX_PATH);
    n = neko_file_safe_strcpy(fpath, "/", n - 1, NEKO_FILES_MAX_PATH);

    char* dname = dir->entry->d_name;
    char* fname = file->name;

    neko_file_safe_strcpy(fname, dname, 0, NEKO_FILES_MAX_FILENAME);
    neko_file_safe_strcpy(fpath, fname, n - 1, NEKO_FILES_MAX_PATH);

    if (stat(file->path, &file->info)) return 0;

    file->size = file->info.st_size;
    neko_file_get_ext(file);

    file->is_dir = S_ISDIR(file->info.st_mode);
    file->is_reg = S_ISREG(file->info.st_mode);

    return 1;
}

void neko_file_dir_next(neko_file_dir_t* dir) {
    NEKO_FILES_ASSERT(dir->has_next);
    dir->entry = readdir(dir->dir);
    dir->has_next = dir->entry ? 1 : 0;
}

void neko_file_dir_close(neko_file_dir_t* dir) {
    dir->path[0] = 0;
    if (dir->dir) closedir(dir->dir);
    dir->dir = 0;
    dir->has_next = 0;
    dir->entry = 0;
}

int neko_file_dir_open(neko_file_dir_t* dir, const char* path) {
    neko_file_safe_strcpy(dir->path, path, 0, NEKO_FILES_MAX_PATH);
    dir->dir = opendir(path);

    if (!dir->dir) {
        printf("ERROR: Failed to open directory (%s): %s.\n", path, strerror(errno));
        neko_file_dir_close(dir);
        NEKO_FILES_ASSERT(0);
        return 0;
    }

    dir->has_next = 1;
    dir->entry = readdir(dir->dir);
    if (!dir->dir) dir->has_next = 0;

    return 1;
}

int neko_file_compare_file_times_by_path(const char* path_a, const char* path_b) {
    time_t time_a;
    time_t time_b;
    struct stat info;
    if (stat(path_a, &info)) return 0;
    time_a = info.st_mtime;
    if (stat(path_b, &info)) return 0;
    time_b = info.st_mtime;
    return (int)difftime(time_a, time_b);
}

int neko_file_get_file_time(const char* path, neko_file_time_t* time) {
    struct stat info;
    if (stat(path, &info)) return 0;
    time->time = info.st_mtime;
    return 1;
}

int neko_file_compare_file_times(neko_file_time_t* time_a, neko_file_time_t* time_b) { return (int)difftime(time_a->time, time_b->time); }

int neko_file_file_exists(const char* path) { return access(path, F_OK) != -1; }

#endif

#endif
#endif

const char* neko_filewatch_error_reason;

#define NEKO_FILEWATCH_CHECK_BUFFER_GROW(ctx, count, capacity, data, type)                     \
    do {                                                                                       \
        if (ctx->count == ctx->capacity) {                                                     \
            int new_capacity = ctx->capacity ? ctx->capacity * 2 : 64;                         \
            void* new_data = NEKO_FILEWATCH_MALLOC(sizeof(type) * new_capacity, ctx->mem_ctx); \
            if (!new_data) {                                                                   \
                neko_filewatch_error_reason = "Out of memory.";                                \
                return 0;                                                                      \
            }                                                                                  \
            NEKO_FILEWATCH_MEMCPY(new_data, ctx->data, sizeof(type) * ctx->count);             \
            NEKO_FILEWATCH_FREE(ctx->data, ctx->mem_ctx);                                      \
            ctx->data = (type*)new_data;                                                       \
            ctx->capacity = new_capacity;                                                      \
        }                                                                                      \
    } while (0)

void neko_filewatch_create_internal(neko_filewatch_t* filewatch, struct neko_assetsys_t* assetsys, void* mem_ctx) {
    // neko_filewatch_t* filewatch = (neko_filewatch_t*)NEKO_FILEWATCH_MALLOC(sizeof(neko_filewatch_t), mem_ctx);
    NEKO_FILEWATCH_MEMSET(filewatch, 0, sizeof(neko_filewatch_t));
    filewatch->mem_ctx = mem_ctx;
    filewatch->assetsys = assetsys;
    strpool_init(&filewatch->strpool, 0);
    // return filewatch;
}

neko_filewatch_t* neko_filewatch_create(struct neko_assetsys_t* assetsys, void* mem_ctx) {
    neko_filewatch_t* filewatch = (neko_filewatch_t*)NEKO_FILEWATCH_MALLOC(sizeof(neko_filewatch_t), mem_ctx);
    NEKO_FILEWATCH_MEMSET(filewatch, 0, sizeof(neko_filewatch_t));
    filewatch->mem_ctx = mem_ctx;
    filewatch->assetsys = assetsys;
    strpool_init(&filewatch->strpool, 0);
    return filewatch;
}

void neko_filewatch_free_internal(neko_filewatch_t* filewatch) {
    strpool_term(&filewatch->strpool);
    NEKO_FILEWATCH_FREE(filewatch->watches, filewatch->mem_ctx);
    NEKO_FILEWATCH_FREE(filewatch->notifications, filewatch->mem_ctx);
    // NEKO_FILEWATCH_FREE(filewatch, filewatch->mem_ctx);
}

void neko_filewatch_free(neko_filewatch_t* filewatch) {
    neko_filewatch_free_internal(filewatch);
    NEKO_FILEWATCH_FREE(filewatch, filewatch->mem_ctx);
}

#define NEKO_FILEWATCH_INJECT(filewatch, str, len) strpool_inject(&filewatch->strpool, str, len)
#define NEKO_FILEWATCH_CSTR(filewatch, id) strpool_cstr(&filewatch->strpool, id)
#define NEKO_FILEWATCH_CHECK(X, Y)           \
    do {                                     \
        if (!(X)) {                          \
            neko_filewatch_error_reason = Y; \
            goto cute_neko_filewatch_err;    \
        }                                    \
    } while (0)

int neko_filewatch_mount(neko_filewatch_t* filewatch, const char* actual_path, const char* mount_as_virtual_path) {
    u64 actual_id;
    u64 virtual_id;
    neko_assetsys_error_t ret;
    neko_filewatch_path_t path;

    NEKO_FILEWATCH_CHECK(filewatch->mount_count < FILEWATCH_MAX_MOUNTS, "Can not mount more than `FILEWATCH_MAX_MOUNTS` times simultaneously.");

    actual_id = NEKO_FILEWATCH_INJECT(filewatch, actual_path, NEKO_FILEWATCH_STRLEN(actual_path));
    virtual_id = NEKO_FILEWATCH_INJECT(filewatch, mount_as_virtual_path, NEKO_FILEWATCH_STRLEN(mount_as_virtual_path));
    path.actual_id = actual_id;
    path.virtual_id = virtual_id;
    filewatch->mount_paths[filewatch->mount_count++] = path;
    ret = neko_assetsys_mount(filewatch->assetsys, actual_path, mount_as_virtual_path);
    NEKO_FILEWATCH_CHECK(ret == ASSETSYS_SUCCESS, "assetsys failed to initialize.");

    return 1;

cute_neko_filewatch_err:
    return 0;
}

void neko_filewatch_dismount(neko_filewatch_t* filewatch, const char* actual_path, const char* virtual_path) {
    int found = 0;
    neko_filewatch_path_t* mount_paths = filewatch->mount_paths;
    u64 actual_id = NEKO_FILEWATCH_INJECT(filewatch, actual_path, NEKO_FILEWATCH_STRLEN(actual_path));
    u64 virtual_id = NEKO_FILEWATCH_INJECT(filewatch, virtual_path, NEKO_FILEWATCH_STRLEN(virtual_path));

    for (int i = 0; i < filewatch->mount_count; ++i) {
        neko_filewatch_path_t path = mount_paths[i];
        if (path.actual_id == actual_id && path.virtual_id == virtual_id) {
            const char* mount_as = NEKO_FILEWATCH_CSTR(filewatch, path.virtual_id);
            const char* mount_path = NEKO_FILEWATCH_CSTR(filewatch, path.actual_id);
            neko_assetsys_dismount(filewatch->assetsys, mount_path, mount_as);
            mount_paths[i] = mount_paths[--filewatch->mount_count];
            break;
        }
    }

    for (int i = 0; i < filewatch->mount_count; ++i) {
        neko_filewatch_path_t path = mount_paths[i];
        if (path.virtual_id == virtual_id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        neko_filewatch_stop_watching(filewatch, virtual_path);
    }
}

static int neko_filewatch_strncpy_internal(char* dst, const char* src, int n, int max) {
    int c;

    do {
        if (n >= max - 1) {
            dst[max - 1] = 0;
            break;
        }
        c = *src++;
        dst[n] = (char)c;
        ++n;
    } while (c);

    return n;
}

void neko_filewatch_path_concat_internal(const char* path_a, const char* path_b, char* out, int max_buffer_length) {
    int n = neko_filewatch_strncpy_internal(out, path_a, 0, max_buffer_length);
    if (*path_b) {
        n = neko_filewatch_strncpy_internal(out, "/", n - 1, max_buffer_length);
        neko_filewatch_strncpy_internal(out, path_b, n - 1, max_buffer_length);
    }
}

neko_filewatch_path_t neko_filewatch_build_path_internal(neko_filewatch_t* filewatch, neko_filewatch_watched_dir_internal_t* watch, const char* path, const char* name) {
    char virtual_buffer[NEKO_FILES_MAX_PATH];
    neko_filewatch_path_t neko_filewatch_path;
    neko_filewatch_path.actual_id = NEKO_FILEWATCH_INJECT(filewatch, path, NEKO_FILEWATCH_STRLEN(path));
    neko_filewatch_path_concat_internal(NEKO_FILEWATCH_CSTR(filewatch, watch->dir_path.virtual_id), name, virtual_buffer, NEKO_FILES_MAX_PATH);
    neko_filewatch_path.virtual_id = NEKO_FILEWATCH_INJECT(filewatch, virtual_buffer, NEKO_FILEWATCH_STRLEN(virtual_buffer));
    return neko_filewatch_path;
}

int neko_filewatch_add_notification_internal(neko_filewatch_t* filewatch, neko_filewatch_watched_dir_internal_t* watch, neko_filewatch_path_t path, neko_filewatch_update_t change) {
    NEKO_FILEWATCH_CHECK_BUFFER_GROW(filewatch, notification_count, notification_capacity, notifications, neko_filewatch_notification_internal_t);
    neko_filewatch_notification_internal_t notification;
    notification.cb = watch->cb;
    notification.udata = watch->udata;
    notification.change = change;
    notification.path = path;
    filewatch->notifications[filewatch->notification_count++] = notification;
    return 0;
}

void neko_filewatch_add_entry_internal(neko_filewatch_watched_dir_internal_t* watch, neko_filewatch_path_t path, u64 name_id, const char* actual_buffer, int is_dir) {
    neko_filewatch_entry_internal_t entry;
    entry.path = path;
    entry.name_id = name_id;
    entry.is_dir = is_dir;
    neko_file_get_file_time(actual_buffer, &entry.time);
    hashtable_insert(&watch->entries, name_id, &entry);
}

int neko_filewatch_update(neko_filewatch_t* filewatch) {
    int remount_count = 0;
    neko_filewatch_path_t remount_paths[FILEWATCH_MAX_MOUNTS];
    NEKO_FILEWATCH_CHECK(filewatch->mount_count, "`neko_filewatch_t` must be mounted before called `neko_filewatch_update`.");

    for (int i = 0; i < filewatch->watch_count; ++i) {
        neko_filewatch_watched_dir_internal_t* watch = filewatch->watches + i;
        neko_filewatch_path_t watch_path = watch->dir_path;

        int entry_count = hashtable_count(&watch->entries);
        neko_filewatch_entry_internal_t* entries = (neko_filewatch_entry_internal_t*)hashtable_items(&watch->entries);

        for (int j = 0; j < entry_count; ++j) {
            neko_filewatch_entry_internal_t* entry = entries + j;

            int was_removed = !neko_file_file_exists(NEKO_FILEWATCH_CSTR(filewatch, entry->path.actual_id));
            if (was_removed) {

                if (entry->is_dir) {
                    neko_filewatch_add_notification_internal(filewatch, watch, entry->path, FILEWATCH_DIR_REMOVED);
                    NEKO_FILEWATCH_CHECK(remount_count < FILEWATCH_MAX_MOUNTS, "Tried to remount too many mounts in one update. Try using less mounts, or increase `FILEWATCH_MAX_MOUNTS`.");
                    remount_paths[remount_count++] = watch_path;
                }

                else {
                    neko_filewatch_add_notification_internal(filewatch, watch, entry->path, FILEWATCH_FILE_REMOVED);
                    NEKO_FILEWATCH_CHECK(remount_count < FILEWATCH_MAX_MOUNTS, "Tried to remount too many mounts in one update. Try using less mounts, or increase `FILEWATCH_MAX_MOUNTS`.");
                    remount_paths[remount_count++] = watch_path;
                }

                hashtable_remove(&watch->entries, entry->name_id);
                --entry_count;
                --j;
            }
        }

        int dir_was_removed = !neko_file_file_exists(NEKO_FILEWATCH_CSTR(filewatch, watch->dir_path.actual_id));
        if (dir_was_removed) {
            neko_filewatch_add_notification_internal(filewatch, watch, watch->dir_path, FILEWATCH_DIR_REMOVED);
            hashtable_term(&watch->entries);
            filewatch->watches[i--] = filewatch->watches[--filewatch->watch_count];
            NEKO_FILEWATCH_CHECK(remount_count < FILEWATCH_MAX_MOUNTS, "Tried to remount too many mounts in one update. Try using less mounts, or increase `FILEWATCH_MAX_MOUNTS`.");
            remount_paths[remount_count++] = watch_path;
            continue;
        }

        neko_file_dir_t dir;
        const char* actual_path = NEKO_FILEWATCH_CSTR(filewatch, watch->dir_path.actual_id);
        neko_file_dir_open(&dir, actual_path);

        while (dir.has_next) {
            neko_file_file_t file;
            neko_file_read_file(&dir, &file);

            u64 name_id = NEKO_FILEWATCH_INJECT(filewatch, file.name, NEKO_FILEWATCH_STRLEN(file.name));
            neko_filewatch_entry_internal_t* entry = (neko_filewatch_entry_internal_t*)hashtable_find(&watch->entries, name_id);

            neko_filewatch_path_t path = neko_filewatch_build_path_internal(filewatch, watch, file.path, file.name);

            if (!file.is_dir && file.is_reg) {
                if (entry) {
                    NEKO_FILEWATCH_ASSERT(!entry->is_dir);
                    neko_file_time_t prev_time = entry->time;
                    neko_file_time_t now_time;
                    neko_file_get_file_time(file.path, &now_time);

                    if (neko_file_compare_file_times(&now_time, &prev_time) && neko_file_file_exists(file.path)) {
                        neko_filewatch_add_notification_internal(filewatch, watch, path, FILEWATCH_FILE_MODIFIED);
                        entry->time = now_time;
                    }
                }

                else {
                    neko_filewatch_add_entry_internal(watch, path, name_id, file.path, 0);
                    neko_filewatch_add_notification_internal(filewatch, watch, path, FILEWATCH_FILE_ADDED);
                    NEKO_FILEWATCH_CHECK(remount_count < FILEWATCH_MAX_MOUNTS, "Tried to remount too many mounts in one update. Try using less mounts, or increase `FILEWATCH_MAX_MOUNTS`.");
                    remount_paths[remount_count++] = watch_path;
                }
            }

            else if (file.is_dir && file.name[0] != '.') {

                if (!entry) {
                    neko_filewatch_add_entry_internal(watch, path, name_id, file.path, 1);
                    neko_filewatch_add_notification_internal(filewatch, watch, path, FILEWATCH_DIR_ADDED);
                    NEKO_FILEWATCH_CHECK(remount_count < FILEWATCH_MAX_MOUNTS, "Tried to remount too many mounts in one update. Try using less mounts, or increase `FILEWATCH_MAX_MOUNTS`.");
                    remount_paths[remount_count++] = watch_path;
                }
            }

            neko_file_dir_next(&dir);
        }

        neko_file_dir_close(&dir);
    }

    for (int i = 0; i < remount_count; ++i) {
        neko_filewatch_path_t path = remount_paths[i];
        const char* actual_path = NEKO_FILEWATCH_CSTR(filewatch, path.actual_id);
        const char* virtual_path = NEKO_FILEWATCH_CSTR(filewatch, path.virtual_id);
        neko_assetsys_dismount(filewatch->assetsys, actual_path, virtual_path);
        neko_assetsys_mount(filewatch->assetsys, actual_path, virtual_path);
    }

    return 1;

cute_neko_filewatch_err:
    return 0;
}

void neko_filewatch_notify(neko_filewatch_t* filewatch) {
    for (int i = 0; i < filewatch->notification_count; ++i) {
        neko_filewatch_notification_internal_t* notification = filewatch->notifications + i;
        const char* virtual_path = NEKO_FILEWATCH_CSTR(filewatch, notification->path.virtual_id);
        notification->cb(notification->change, virtual_path, notification->udata);
    }
    filewatch->notification_count = 0;
}

void neko_filewatch_actual_path_to_virtual_path_internal(neko_filewatch_t* filewatch, const char* actual_path, char* virtual_path, int virtual_path_capacity, neko_filewatch_path_t path) {
    const char* mount_path = NEKO_FILEWATCH_CSTR(filewatch, path.actual_id);
    const char* mount_as = NEKO_FILEWATCH_CSTR(filewatch, path.virtual_id);
    const char* offset_actual_path = actual_path + NEKO_FILEWATCH_STRLEN(mount_as);
    offset_actual_path = *offset_actual_path == '/' ? offset_actual_path + 1 : offset_actual_path;
    neko_filewatch_path_concat_internal(mount_as, offset_actual_path, virtual_path, virtual_path_capacity);
}

void neko_filewatch_virtual_path_to_actual_path_internal(neko_filewatch_t* filewatch, const char* virtual_path, char* actual_path, int actual_path_capacity, neko_filewatch_path_t path) {
    const char* mount_path = NEKO_FILEWATCH_CSTR(filewatch, path.actual_id);
    const char* mount_as = NEKO_FILEWATCH_CSTR(filewatch, path.virtual_id);
    const char* offset_virtual_path = virtual_path + NEKO_FILEWATCH_STRLEN(mount_as);
    offset_virtual_path = *offset_virtual_path == '/' ? offset_virtual_path + 1 : offset_virtual_path;
    neko_filewatch_path_concat_internal(mount_path, offset_virtual_path, actual_path, actual_path_capacity);
}

int neko_filewatch_start_watching(neko_filewatch_t* filewatch, const char* virtual_path, neko_filewatch_callback_t* cb, void* udata) {
    neko_filewatch_watched_dir_internal_t* watch;
    int success;
    u64 virtual_id = NEKO_FILEWATCH_INJECT(filewatch, virtual_path, NEKO_FILEWATCH_STRLEN(virtual_path));
    NEKO_FILEWATCH_CHECK(filewatch->mount_count, "`neko_filewatch_t` must be mounted before called `neko_filewatch_update`.");

    for (int i = 0; i < filewatch->mount_count; ++i) {
        neko_filewatch_path_t path = filewatch->mount_paths[i];
        if (path.virtual_id != virtual_id) continue;

        NEKO_FILEWATCH_CHECK_BUFFER_GROW(filewatch, watch_count, watch_capacity, watches, neko_filewatch_watched_dir_internal_t);
        watch = filewatch->watches + filewatch->watch_count++;
        watch->cb = cb;
        watch->udata = udata;
        hashtable_init(&watch->entries, sizeof(neko_filewatch_entry_internal_t), 32, filewatch->mem_ctx);

        char actual_path[260];
        neko_filewatch_virtual_path_to_actual_path_internal(filewatch, virtual_path, actual_path, 260, path);
        path.actual_id = NEKO_FILEWATCH_INJECT(filewatch, actual_path, NEKO_FILEWATCH_STRLEN(actual_path));
        path.virtual_id = virtual_id;
        watch->dir_path = path;

        neko_file_dir_t dir;
        success = neko_file_dir_open(&dir, actual_path);
        NEKO_FILEWATCH_CHECK(success, "`virtual_path` is not a valid directory.");

        while (dir.has_next) {
            neko_file_file_t file;
            neko_file_read_file(&dir, &file);
            u64 name_id = NEKO_FILEWATCH_INJECT(filewatch, file.name, NEKO_FILEWATCH_STRLEN(file.name));
            neko_filewatch_path_t file_path = neko_filewatch_build_path_internal(filewatch, watch, file.path, file.name);

            if (!file.is_dir && file.is_reg) {
                neko_filewatch_add_entry_internal(watch, file_path, name_id, file.path, 0);
            }

            else if (file.is_dir && file.name[0] != '.') {
                neko_filewatch_add_entry_internal(watch, file_path, name_id, file.path, 1);
            }

            neko_file_dir_next(&dir);
        }

        neko_file_dir_close(&dir);
    }

    return 1;

cute_neko_filewatch_err:
    return 0;
}

void neko_filewatch_stop_watching(neko_filewatch_t* filewatch, const char* virtual_path) {
    u64 virtual_id = NEKO_FILEWATCH_INJECT(filewatch, virtual_path, NEKO_FILEWATCH_STRLEN(virtual_path));

    for (int i = 0; i < filewatch->watch_count; ++i) {
        neko_filewatch_watched_dir_internal_t* watch = filewatch->watches + i;
        if (virtual_id == watch->dir_path.virtual_id) {
            hashtable_term(&watch->entries);

            for (int j = 0; j < filewatch->notification_count; ++j) {
                neko_filewatch_notification_internal_t* notification = filewatch->notifications + j;
                if (notification->path.virtual_id == virtual_id && notification->cb == watch->cb && notification->udata == watch->udata) {
                    filewatch->notifications[j--] = filewatch->notifications[--filewatch->notification_count];
                }
            }

            filewatch->watches[i--] = filewatch->watches[--filewatch->watch_count];
        }
    }
}

int neko_filewatch_actual_path_to_virtual_path(neko_filewatch_t* filewatch, const char* actual_path, char* virtual_path, int virtual_path_capacity) {
    neko_filewatch_path_t* mount_paths = filewatch->mount_paths;
    u64 actual_id = NEKO_FILEWATCH_INJECT(filewatch, actual_path, NEKO_FILEWATCH_STRLEN(actual_path));

    for (int i = 0; i < filewatch->mount_count; ++i) {
        neko_filewatch_path_t path = mount_paths[i];
        if (path.actual_id == actual_id) {
            neko_filewatch_actual_path_to_virtual_path_internal(filewatch, actual_path, virtual_path, virtual_path_capacity, path);
            return 1;
        }
    }

    return 0;
}

int neko_filewatch_virtual_path_to_actual_path(neko_filewatch_t* filewatch, const char* virtual_path, char* actual_path, int actual_path_capacity) {
    neko_filewatch_path_t* mount_paths = filewatch->mount_paths;
    u64 virtual_id = NEKO_FILEWATCH_INJECT(filewatch, virtual_path, NEKO_FILEWATCH_STRLEN(virtual_path));

    for (int i = 0; i < filewatch->mount_count; ++i) {
        neko_filewatch_path_t path = mount_paths[i];
        if (path.virtual_id == virtual_id) {
            neko_filewatch_virtual_path_to_actual_path_internal(filewatch, virtual_path, actual_path, actual_path_capacity, path);
            return 1;
        }
    }

    return 0;
}

#endif

// NEKO_ASSETSYS_IMPL

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <sys/stat.h>

// #include "strpool.h"

#define ASSETSYS_ASSERT(expression, message) assert((expression) && (message))
#define ASSETSYS_MALLOC(ctx, size) (malloc(size))
#define ASSETSYS_FREE(ctx, ptr) (free(ptr))

#if defined(_WIN32)
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0501
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#endif
#define _WINSOCKAPI_
#pragma warning(push)
#pragma warning(disable : 4668)
#pragma warning(disable : 4768)
#pragma warning(disable : 4255)
#include <windows.h>
#pragma warning(pop)

struct neko_assetsys_internal_dir_entry_t {
    char name[MAX_PATH];
    BOOL is_folder;
};

struct neko_assetsys_internal_dir_t {
    HANDLE handle;
    WIN32_FIND_DATAA data;
    struct neko_assetsys_internal_dir_entry_t entry;
};

static void neko_assetsys_internal_dir_open(struct neko_assetsys_internal_dir_t* dir, char const* path) {
    size_t path_len = strlen(path);
    BOOL trailing_path_separator = path[path_len - 1] == '\\' || path[path_len - 1] == '/';
    const char* string_to_append = "*.*";
    if (path_len + strlen(string_to_append) + (trailing_path_separator ? 0 : 1) >= MAX_PATH) return;
    char search_pattern[MAX_PATH];
    strcpy(search_pattern, path);
    if (!trailing_path_separator) strcat(search_pattern, "\\");
    strcat(search_pattern, string_to_append);

    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(search_pattern, &data);
    if (handle == INVALID_HANDLE_VALUE) return;

    dir->handle = handle;
    dir->data = data;
}

static void neko_assetsys_internal_dir_close(struct neko_assetsys_internal_dir_t* dir) {
    if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
}

static struct neko_assetsys_internal_dir_entry_t* neko_assetsys_internal_dir_read(struct neko_assetsys_internal_dir_t* dir) {
    if (dir->handle == INVALID_HANDLE_VALUE) return NULL;

    strcpy(dir->entry.name, dir->data.cFileName);
    dir->entry.is_folder = (dir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

    BOOL result = FindNextFileA(dir->handle, &dir->data);
    if (!result) {
        FindClose(dir->handle);
        dir->handle = INVALID_HANDLE_VALUE;
    }

    return &dir->entry;
}

static char const* neko_assetsys_internal_dir_name(struct neko_assetsys_internal_dir_entry_t* entry) { return entry->name; }

static int neko_assetsys_internal_dir_is_file(struct neko_assetsys_internal_dir_entry_t* entry) { return entry->is_folder == FALSE; }

static int neko_assetsys_internal_dir_is_folder(struct neko_assetsys_internal_dir_entry_t* entry) { return entry->is_folder == TRUE; }

#else

#include <dirent.h>

struct neko_assetsys_internal_dir_t {
    DIR* dir;
};

typedef struct neko_assetsys_internal_dir_entry_t neko_assetsys_internal_dir_entry_t;

static void neko_assetsys_internal_dir_open(struct neko_assetsys_internal_dir_t* dir, char const* path) { dir->dir = opendir(path); }

static void neko_assetsys_internal_dir_close(struct neko_assetsys_internal_dir_t* dir) { closedir(dir->dir); }

static neko_assetsys_internal_dir_entry_t* neko_assetsys_internal_dir_read(struct neko_assetsys_internal_dir_t* dir) { return (neko_assetsys_internal_dir_entry_t*)readdir(dir->dir); }

static char const* neko_assetsys_internal_dir_name(neko_assetsys_internal_dir_entry_t* entry) { return ((struct dirent*)entry)->d_name; }

static int neko_assetsys_internal_dir_is_file(neko_assetsys_internal_dir_entry_t* entry) { return ((struct dirent*)entry)->d_type == DT_REG; }

static int neko_assetsys_internal_dir_is_folder(neko_assetsys_internal_dir_entry_t* entry) { return ((struct dirent*)entry)->d_type == DT_DIR; }

#endif

static void* neko_assetsys_internal_mz_alloc(void* memctx, size_t items, size_t size) {
    (void)memctx;
    (void)items;
    (void)size;
    ASSETSYS_U64* p = (ASSETSYS_U64*)ASSETSYS_MALLOC(memctx, (items * size) + sizeof(ASSETSYS_U64));
    *p = (items * size);
    return p + 1;
}

static void neko_assetsys_internal_mz_free(void* memctx, void* ptr) {
    (void)memctx;
    (void)ptr;
    if (!ptr) return;
    ASSETSYS_U64* p = ((ASSETSYS_U64*)ptr) - 1;
    ASSETSYS_FREE(memctx, p);
}

static void* neko_assetsys_internal_mz_realloc(void* memctx, void* ptr, size_t items, size_t size) {
    (void)memctx;
    (void)ptr;
    (void)items;
    (void)size;
    if (!ptr) return neko_assetsys_internal_mz_alloc(memctx, items, size);

    ASSETSYS_U64* p = ((ASSETSYS_U64*)ptr) - 1;
    ASSETSYS_U64 prev_size = *p;
    if (prev_size >= (items * size)) return ptr;

    ASSETSYS_U64* new_ptr = (ASSETSYS_U64*)ASSETSYS_MALLOC(memctx, (items * size) + sizeof(ASSETSYS_U64));
    *new_ptr = (items * size);
    ++new_ptr;
    memcpy(new_ptr, ptr, (size_t)prev_size);
    ASSETSYS_FREE(memctx, p);
    return new_ptr;
}

static char* neko_assetsys_internal_dirname(char const* path);

static ASSETSYS_U64 neko_assetsys_internal_add_string(neko_assetsys_t* sys, char const* const str) {
    ASSETSYS_U64 h = strpool_inject(&sys->strpool, str, (int)strlen(str));
    strpool_incref(&sys->strpool, h);
    return h;
}

static char const* neko_assetsys_internal_get_string(neko_assetsys_t* sys, ASSETSYS_U64 const handle) { return strpool_cstr(&sys->strpool, handle); }

void neko_assetsys_create_internal(neko_assetsys_t* sys, void* memctx) {
    // neko_assetsys_t* sys = (neko_assetsys_t*)ASSETSYS_MALLOC(memctx, sizeof(neko_assetsys_t));
    sys->memctx = memctx;

    strpool_config_t config = strpool_default_config;
    config.memctx = memctx;
    strpool_init(&sys->strpool, &config);

    sys->mounts_count = 0;
    sys->mounts_capacity = 16;
    sys->mounts = (struct neko_assetsys_internal_mount_t*)ASSETSYS_MALLOC(memctx, sizeof(*sys->mounts) * sys->mounts_capacity);

    sys->collated_count = 0;
    sys->collated_capacity = 16384;
    sys->collated = (struct neko_assetsys_internal_collated_t*)ASSETSYS_MALLOC(memctx, sizeof(*sys->collated) * sys->collated_capacity);
    // return sys;
}

neko_assetsys_t* neko_assetsys_create(void* memctx) {
    neko_assetsys_t* sys = (neko_assetsys_t*)ASSETSYS_MALLOC(memctx, sizeof(neko_assetsys_t));
    sys->memctx = memctx;

    strpool_config_t config = strpool_default_config;
    config.memctx = memctx;
    strpool_init(&sys->strpool, &config);

    sys->mounts_count = 0;
    sys->mounts_capacity = 16;
    sys->mounts = (struct neko_assetsys_internal_mount_t*)ASSETSYS_MALLOC(memctx, sizeof(*sys->mounts) * sys->mounts_capacity);

    sys->collated_count = 0;
    sys->collated_capacity = 16384;
    sys->collated = (struct neko_assetsys_internal_collated_t*)ASSETSYS_MALLOC(memctx, sizeof(*sys->collated) * sys->collated_capacity);
    return sys;
}

void neko_assetsys_destroy_internal(neko_assetsys_t* sys) {
    while (sys->mounts_count > 0) {
        neko_assetsys_dismount(sys, neko_assetsys_internal_get_string(sys, sys->mounts[0].path), neko_assetsys_internal_get_string(sys, sys->mounts[0].mounted_as));
    }
    ASSETSYS_FREE(sys->memctx, sys->collated);
    ASSETSYS_FREE(sys->memctx, sys->mounts);
    strpool_term(&sys->strpool);
    // ASSETSYS_FREE(sys->memctx, sys);
}

void neko_assetsys_destroy(neko_assetsys_t* sys) {
    neko_assetsys_destroy_internal(sys);
    ASSETSYS_FREE(sys->memctx, sys);
}

static int neko_assetsys_internal_register_collated(neko_assetsys_t* sys, char const* path, int const is_file) {
    if (path[0] == '/' && path[1] == '/') ++path;

    ASSETSYS_U64 handle = strpool_inject(&sys->strpool, path, (int)strlen(path));

    int first_free = -1;
    for (int i = 0; i < sys->collated_count; ++i) {
        if (sys->collated[i].ref_count > 0 && sys->collated[i].path == handle) {
            ASSETSYS_ASSERT(is_file == sys->collated[i].is_file, "Entry type mismatch");
            ++sys->collated[i].ref_count;
            strpool_discard(&sys->strpool, handle);
            return i;
        }
        if (sys->collated[i].ref_count == 0) first_free = i;
    }

    if (first_free < 0) {
        if (sys->collated_count >= sys->collated_capacity) {
            sys->collated_capacity *= 2;
            struct neko_assetsys_internal_collated_t* new_collated = (struct neko_assetsys_internal_collated_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*sys->collated) * sys->collated_capacity);
            memcpy(new_collated, sys->collated, sizeof(*sys->collated) * sys->collated_count);
            ASSETSYS_FREE(sys->memctx, sys->collated);
            sys->collated = new_collated;
        }
        first_free = sys->collated_count++;
    }

    struct neko_assetsys_internal_collated_t* dir = &sys->collated[first_free];
    dir->path = handle;
    strpool_incref(&sys->strpool, handle);
    dir->parent = -1;
    dir->ref_count = 1;
    dir->is_file = is_file;
    return first_free;
}

static void neko_assetsys_internal_collate_directories(neko_assetsys_t* sys, struct neko_assetsys_internal_mount_t* const mount) {
    for (int i = 0; i < mount->dirs_count; ++i) {
        struct neko_assetsys_internal_collated_t* subdir = &sys->collated[mount->dirs[i].collated_index];
        if (subdir->parent < 0) {
            char const* a = neko_assetsys_internal_get_string(sys, subdir->path);
            (void)a;
            char* sub_path = neko_assetsys_internal_dirname(neko_assetsys_internal_get_string(sys, subdir->path));
            ASSETSYS_U64 handle = strpool_inject(&sys->strpool, sub_path, (int)strlen(sub_path) - 1);
            for (int j = 0; j < sys->collated_count; ++j) {
                struct neko_assetsys_internal_collated_t* dir = &sys->collated[j];
                char const* b = neko_assetsys_internal_get_string(sys, dir->path);
                (void)b;
                if (dir->path == handle) {
                    subdir->parent = j;
                    break;
                }
            }
            if (subdir->parent < 0) strpool_discard(&sys->strpool, handle);
        }
    }

    for (int i = 0; i < mount->files_count; ++i) {
        struct neko_assetsys_internal_collated_t* file = &sys->collated[mount->files[i].collated_index];
        if (file->parent < 0) {
            char* file_path = neko_assetsys_internal_dirname(neko_assetsys_internal_get_string(sys, file->path));
            ASSETSYS_U64 handle = strpool_inject(&sys->strpool, file_path, (int)strlen(file_path) - 1);
            for (int j = 0; j < sys->collated_count; ++j) {
                struct neko_assetsys_internal_collated_t* dir = &sys->collated[j];
                if (dir->path == handle) {
                    file->parent = j;
                    break;
                }
            }
            if (file->parent < 0) strpool_discard(&sys->strpool, handle);
        }
    }
}

static void neko_assetsys_internal_recurse_directories(neko_assetsys_t* sys, int const collated_index, struct neko_assetsys_internal_mount_t* const mount) {
    char const* path = neko_assetsys_internal_get_string(sys, sys->collated[collated_index].path);
    path += mount->mount_len;
    if (*path == '/') ++path;

    strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->path));
    strcat(sys->temp, (*path == '\0' || *sys->temp == '\0') ? "" : "/");
    strcat(sys->temp, path);

    struct neko_assetsys_internal_dir_t dir;
    neko_assetsys_internal_dir_open(&dir, *sys->temp == '\0' ? "." : sys->temp);

    struct neko_assetsys_internal_dir_entry_t* dirent;
    for (dirent = neko_assetsys_internal_dir_read(&dir); dirent != NULL; dirent = neko_assetsys_internal_dir_read(&dir)) {
        char const* name = neko_assetsys_internal_dir_name(dirent);
        if (!name || *name == '\0' || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        int is_file = neko_assetsys_internal_dir_is_file(dirent);
        int is_folder = neko_assetsys_internal_dir_is_folder(dirent);
        if (is_file) {
            char const* file_path = neko_assetsys_internal_get_string(sys, sys->collated[collated_index].path);
            file_path += mount->mount_len;
            if (*file_path == '/') ++file_path;

            strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->path));
            strcat(sys->temp, (*file_path == '\0' || *sys->temp == '\0') ? "" : "/");
            strcat(sys->temp, file_path);
            strcat(sys->temp, *sys->temp == '\0' ? "" : "/");
            strcat(sys->temp, name);

            struct stat s;
            if (stat(sys->temp, &s) == 0) {
                strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->mounted_as));
                strcat(sys->temp, "/");
                strcat(sys->temp, file_path);
                strcat(sys->temp, *file_path == '\0' ? "" : "/");
                strcat(sys->temp, name);

                if (mount->files_count >= mount->files_capacity) {
                    mount->files_capacity *= 2;
                    struct neko_assetsys_internal_file_t* new_files = (struct neko_assetsys_internal_file_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*(mount->files)) * mount->files_capacity);
                    memcpy(new_files, mount->files, sizeof(*(mount->files)) * mount->files_count);
                    ASSETSYS_FREE(sys->memctx, mount->files);
                    mount->files = new_files;
                }

                struct neko_assetsys_internal_file_t* file = &mount->files[mount->files_count++];
                file->size = (int)s.st_size;
                file->zip_index = -1;
                file->collated_index = neko_assetsys_internal_register_collated(sys, sys->temp, 1);
            }
        } else if (is_folder) {
            char const* folder_path = neko_assetsys_internal_get_string(sys, sys->collated[collated_index].path);
            folder_path += mount->mount_len;
            if (*folder_path == '/') ++folder_path;

            strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->path));
            strcat(sys->temp, (*folder_path == '\0' || *sys->temp == '\0') ? "" : "/");
            strcat(sys->temp, folder_path);
            strcat(sys->temp, *sys->temp == '\0' ? "" : "/");
            strcat(sys->temp, name);

            struct stat s;
            if (stat(sys->temp, &s) == 0) {
                strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->mounted_as));
                strcat(sys->temp, "/");
                strcat(sys->temp, folder_path);
                strcat(sys->temp, *folder_path == '\0' ? "" : "/");
                strcat(sys->temp, name);

                if (mount->dirs_count >= mount->dirs_capacity) {
                    mount->dirs_capacity *= 2;
                    struct neko_assetsys_internal_folder_t* new_dirs = (struct neko_assetsys_internal_folder_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*(mount->dirs)) * mount->dirs_capacity);
                    memcpy(new_dirs, mount->dirs, sizeof(*(mount->dirs)) * mount->dirs_count);
                    ASSETSYS_FREE(sys->memctx, mount->dirs);
                    mount->dirs = new_dirs;
                }
                struct neko_assetsys_internal_folder_t* as_dir = &mount->dirs[mount->dirs_count++];
                as_dir->collated_index = neko_assetsys_internal_register_collated(sys, sys->temp, 0);
                neko_assetsys_internal_recurse_directories(sys, as_dir->collated_index, mount);
            }
        }
    }
    neko_assetsys_internal_dir_close(&dir);
}

struct neko_assetsys_internal_mount_t* neko_assetsys_internal_create_mount(neko_assetsys_t* sys, enum neko_assetsys_internal_mount_type_t type, char const* path, char const* mount_as) {
    if (sys->mounts_count >= sys->mounts_capacity) {
        sys->mounts_capacity *= 2;
        struct neko_assetsys_internal_mount_t* new_mounts = (struct neko_assetsys_internal_mount_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*sys->mounts) * sys->mounts_capacity);
        memcpy(new_mounts, sys->mounts, sizeof(*sys->mounts) * sys->mounts_count);
        ASSETSYS_FREE(sys->memctx, sys->mounts);
        sys->mounts = new_mounts;
    }

    struct neko_assetsys_internal_mount_t* mount = &sys->mounts[sys->mounts_count];

    mount->mounted_as = neko_assetsys_internal_add_string(sys, mount_as ? mount_as : "");
    mount->mount_len = mount_as ? (int)strlen(mount_as) : 0;
    mount->path = neko_assetsys_internal_add_string(sys, path);
    mount->type = type;

    mount->files_count = 0;
    mount->files_capacity = 4096;
    mount->files = (struct neko_assetsys_internal_file_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*(mount->files)) * mount->files_capacity);

    mount->dirs_count = 0;
    mount->dirs_capacity = 1024;
    mount->dirs = (struct neko_assetsys_internal_folder_t*)ASSETSYS_MALLOC(sys->memctx, sizeof(*(mount->dirs)) * mount->dirs_capacity);

    return mount;
}

neko_assetsys_error_t neko_assetsys_mount(neko_assetsys_t* sys, char const* path, char const* mount_as) {
    if (!path) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if (!mount_as) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if (strchr(path, '\\')) return ASSETSYS_ERROR_INVALID_PATH;
    if (strchr(mount_as, '\\')) return ASSETSYS_ERROR_INVALID_PATH;
    int len = (int)strlen(path);
    if (len > 1 && path[len - 1] == '/') return ASSETSYS_ERROR_INVALID_PATH;
    int mount_len = (int)strlen(mount_as);
    if (mount_len == 0 || mount_as[0] != '/' || (mount_len > 1 && mount_as[mount_len - 1] == '/')) return ASSETSYS_ERROR_INVALID_PATH;

    enum neko_assetsys_internal_mount_type_t type;

#if defined(_MSC_VER) && _MSC_VER >= 1400
    struct _stat64 s;
    int res = __stat64(*path == '\0' ? "." : path, &s);
#else
    struct stat s;
    int res = stat(*path == '\0' ? "." : path, &s);
#endif
    if (res == 0) {
        if (s.st_mode & S_IFDIR) {
            type = ASSETSYS_INTERNAL_MOUNT_TYPE_DIR;
        } else if (s.st_mode & S_IFREG) {
            // type = ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP;
        } else {
            return ASSETSYS_ERROR_INVALID_PATH;
        }
    } else {
        return ASSETSYS_ERROR_INVALID_PATH;
    }

    struct neko_assetsys_internal_mount_t* mount = neko_assetsys_internal_create_mount(sys, type, path, mount_as);

    if (type == ASSETSYS_INTERNAL_MOUNT_TYPE_DIR) {
        struct neko_assetsys_internal_folder_t* dir = &mount->dirs[mount->dirs_count++];
        dir->collated_index = neko_assetsys_internal_register_collated(sys, mount_as, 0);
        neko_assetsys_internal_recurse_directories(sys, dir->collated_index, mount);
    } else {
    }

    neko_assetsys_internal_collate_directories(sys, mount);

    ++sys->mounts_count;
    return ASSETSYS_SUCCESS;
}

static void neko_assetsys_internal_remove_collated(neko_assetsys_t* sys, int const index) {
    struct neko_assetsys_internal_collated_t* coll = &sys->collated[index];
    ASSETSYS_ASSERT(coll->ref_count > 0, "Invalid ref count");
    --coll->ref_count;
    if (coll->ref_count == 0) {
        strpool_decref(&sys->strpool, coll->path);
        strpool_discard(&sys->strpool, coll->path);
    }
}

neko_assetsys_error_t neko_assetsys_dismount(neko_assetsys_t* sys, char const* path, char const* mounted_as) {
    if (!path) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if (!mounted_as) return ASSETSYS_ERROR_INVALID_MOUNT;

    ASSETSYS_U64 path_handle = strpool_inject(&sys->strpool, path, (int)strlen(path));
    ASSETSYS_U64 mount_handle = strpool_inject(&sys->strpool, mounted_as, (int)strlen(mounted_as));

    for (int i = 0; i < sys->mounts_count; ++i) {
        struct neko_assetsys_internal_mount_t* mount = &sys->mounts[i];
        if (mount->mounted_as == mount_handle && mount->path == path_handle) {
            // mz_bool result = 1;
            // if (mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP) result = mz_zip_reader_end(&mount->zip);

            strpool_decref(&sys->strpool, mount->mounted_as);
            strpool_decref(&sys->strpool, mount->path);
            strpool_discard(&sys->strpool, mount_handle);
            strpool_discard(&sys->strpool, path_handle);

            for (int j = 0; j < mount->dirs_count; ++j) neko_assetsys_internal_remove_collated(sys, mount->dirs[j].collated_index);

            for (int j = 0; j < mount->files_count; ++j) neko_assetsys_internal_remove_collated(sys, mount->files[j].collated_index);

            ASSETSYS_FREE(sys->memctx, mount->dirs);
            ASSETSYS_FREE(sys->memctx, mount->files);

            int count = sys->mounts_count - i;
            if (count > 0) memcpy(&sys->mounts[i], &sys->mounts[i + 1], sizeof(*sys->mounts) * count);
            --sys->mounts_count;

            return ASSETSYS_SUCCESS;
        }
    }

    strpool_discard(&sys->strpool, mount_handle);
    strpool_discard(&sys->strpool, path_handle);
    return ASSETSYS_ERROR_INVALID_MOUNT;
}

neko_assetsys_error_t neko_assetsys_file(neko_assetsys_t* sys, char const* path, neko_assetsys_file_t* file) {
    if (!file || !path) return ASSETSYS_ERROR_INVALID_PARAMETER;

    ASSETSYS_U64 handle = strpool_inject(&sys->strpool, path, (int)strlen(path));

    int m = sys->mounts_count;
    while (m > 0) {
        --m;
        struct neko_assetsys_internal_mount_t* mount = &sys->mounts[m];
        for (int i = 0; i < mount->files_count; ++i) {
            ASSETSYS_U64 h = sys->collated[mount->files[i].collated_index].path;
            if (handle == h) {
                file->mount = mount->mounted_as;
                file->path = mount->path;
                file->index = i;
                return ASSETSYS_SUCCESS;
            }
        }
    }

    strpool_discard(&sys->strpool, handle);
    return ASSETSYS_ERROR_FILE_NOT_FOUND;
}

static int neko_assetsys_internal_find_mount_index(neko_assetsys_t* sys, ASSETSYS_U64 const mount, ASSETSYS_U64 const path) {
    for (int i = 0; i < sys->mounts_count; ++i) {
        if (sys->mounts[i].mounted_as == mount && sys->mounts[i].path == path) return i;
    }
    return -1;
}

neko_assetsys_error_t neko_assetsys_file_load(neko_assetsys_t* sys, neko_assetsys_file_t f, int* size, void* buffer, int capacity) {
    int mount_index = neko_assetsys_internal_find_mount_index(sys, f.mount, f.path);
    if (mount_index < 0) return ASSETSYS_ERROR_INVALID_MOUNT;

    struct neko_assetsys_internal_mount_t* mount = &sys->mounts[mount_index];
    struct neko_assetsys_internal_file_t* file = &mount->files[f.index];
    if (mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_DIR) {
        if (size) *size = file->size;
        strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->path));
        strcat(sys->temp, *sys->temp == '\0' ? "" : "/");
        strcat(sys->temp, neko_assetsys_internal_get_string(sys, sys->collated[file->collated_index].path) +
                                  (strcmp(neko_assetsys_internal_get_string(sys, mount->mounted_as), "/") == 0 ? 0 : mount->mount_len + 1));
        FILE* fp = fopen(sys->temp, "rb");
        if (!fp) return ASSETSYS_ERROR_FAILED_TO_READ_FILE;

        fseek(fp, 0, SEEK_END);
        int file_size = (int)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (size) *size = file_size;

        if (file_size > capacity) {
            fclose(fp);
            return ASSETSYS_ERROR_BUFFER_TOO_SMALL;
        }

        int size_read = (int)fread(buffer, 1, (size_t)file_size, fp);
        fclose(fp);
        if (size_read != file_size) return ASSETSYS_ERROR_FAILED_TO_READ_FILE;

        return ASSETSYS_SUCCESS;
    }

    return ASSETSYS_SUCCESS;
}

int neko_assetsys_file_size(neko_assetsys_t* sys, neko_assetsys_file_t file) {
    int mount_index = neko_assetsys_internal_find_mount_index(sys, file.mount, file.path);
    if (mount_index < 0) return 0;

    struct neko_assetsys_internal_mount_t* mount = &sys->mounts[mount_index];
    if (mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_DIR) {
        strcpy(sys->temp, neko_assetsys_internal_get_string(sys, mount->path));
        strcat(sys->temp, *sys->temp == '\0' ? "" : "/");
        strcat(sys->temp, neko_assetsys_internal_get_string(sys, sys->collated[mount->files[file.index].collated_index].path) + mount->mount_len + 1);
        struct stat s;
        if (stat(sys->temp, &s) == 0) mount->files[file.index].size = (int)s.st_size;
    }

    return mount->files[file.index].size;
}

static int neko_assetsys_internal_find_collated(neko_assetsys_t* sys, char const* const path) {
    ASSETSYS_U64 handle = strpool_inject(&sys->strpool, path, (int)strlen(path));

    for (int i = 0; i < sys->collated_count; ++i) {
        if (sys->collated[i].path == handle) {
            return i;
        }
    }

    strpool_discard(&sys->strpool, handle);
    return -1;
}

int neko_assetsys_file_count(neko_assetsys_t* sys, char const* path) {
    if (!path) return 0;
    int dir = neko_assetsys_internal_find_collated(sys, path);
    int count = 0;
    for (int i = 0; i < sys->collated_count; ++i) {
        if (sys->collated[i].is_file && sys->collated[i].parent == dir) {
            ++count;
        }
    }
    return count;
}

char const* neko_assetsys_file_name(neko_assetsys_t* sys, char const* path, int index) {
    char const* file_path = neko_assetsys_file_path(sys, path, index);
    if (file_path) {
        char const* name = strrchr(file_path, '/');
        if (!name) return file_path;
        return name + 1;
    }

    return NULL;
}

char const* neko_assetsys_file_path(neko_assetsys_t* sys, char const* path, int index) {
    if (!path) return 0;
    int dir = neko_assetsys_internal_find_collated(sys, path);
    int count = 0;
    for (int i = 0; i < sys->collated_count; ++i) {
        if (sys->collated[i].is_file && sys->collated[i].parent == dir) {
            if (count == index) return neko_assetsys_internal_get_string(sys, sys->collated[i].path);
            ++count;
        }
    }
    return NULL;
}

int neko_assetsys_subdir_count(neko_assetsys_t* sys, char const* path) {
    if (!path) return 0;
    int dir = neko_assetsys_internal_find_collated(sys, path);
    int count = 0;
    for (int i = 0; i < sys->collated_count; ++i) {
        if (!sys->collated[i].is_file && sys->collated[i].parent == dir) {
            ++count;
        }
    }
    return count;
}

char const* neko_assetsys_subdir_name(neko_assetsys_t* sys, char const* path, int index) {
    char const* subdir_path = neko_assetsys_subdir_path(sys, path, index);
    if (subdir_path) {
        char const* name = strrchr(subdir_path, '/');
        if (!name) return subdir_path;
        return name + 1;
    }

    return NULL;
}

char const* neko_assetsys_subdir_path(neko_assetsys_t* sys, char const* path, int index) {
    if (!path) return 0;
    int dir = neko_assetsys_internal_find_collated(sys, path);
    int count = 0;
    for (int i = 0; i < sys->collated_count; ++i) {
        if (!sys->collated[i].is_file && sys->collated[i].parent == dir) {
            if (count == index) return neko_assetsys_internal_get_string(sys, sys->collated[i].path);
            ++count;
        }
    }
    return NULL;
}

static char* neko_assetsys_internal_dirname(char const* path) {
    static char result[260];
    strncpy(result, path, sizeof(result));

    char* lastForwardSlash = strrchr(result, '/');

    if (lastForwardSlash)
        *(lastForwardSlash + 1) = '\0';
    else
        *result = '\0';

    return result;
}
