

#ifndef NEKO_FILEWATCH_H
#define NEKO_FILEWATCH_H

#include "engine/neko.h"
#include "engine/neko_common.h"

#ifndef FILESYSTEM_U64
#define FILESYSTEM_U64 unsigned long long
#endif

#define NEKO_FILES_MAX_PATH 1024
#define NEKO_FILES_MAX_FILENAME 256
#define NEKO_FILES_MAX_EXT 32

struct neko_file_file_t;
struct neko_file_dir_t;
struct neko_file_time_t;
typedef struct neko_file_file_t neko_file_file_t;
typedef struct neko_file_dir_t neko_file_dir_t;
typedef struct neko_file_time_t neko_file_time_t;
typedef void(neko_file_callback_t)(neko_file_file_t* file, void* udata);

#if NEKO_FILES_PLATFORM == NEKO_FILES_WINDOWS

#if !defined _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <windows.h>

struct neko_file_file_t {
    char path[NEKO_FILES_MAX_PATH];
    char name[NEKO_FILES_MAX_FILENAME];
    char ext[NEKO_FILES_MAX_EXT];
    int is_dir;
    int is_reg;
    size_t size;
};

struct neko_file_dir_t {
    char path[NEKO_FILES_MAX_PATH];
    int has_next;
    HANDLE handle;
    WIN32_FIND_DATAA fdata;
};

struct neko_file_time_t {
    FILETIME time;
};

#elif NEKO_FILES_PLATFORM == NEKO_FILES_MAC || NEKO_FILES_PLATFORM == NEKO_FILES_UNIX

#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

struct neko_file_file_t {
    char path[NEKO_FILES_MAX_PATH];
    char name[NEKO_FILES_MAX_FILENAME];
    char ext[NEKO_FILES_MAX_EXT];
    int is_dir;
    int is_reg;
    int size;
    struct stat info;
};

struct neko_file_dir_t {
    char path[NEKO_FILES_MAX_PATH];
    int has_next;
    DIR* dir;
    struct dirent* entry;
};

struct neko_file_time_t {
    time_t time;
};

#endif

typedef enum neko_filesystem_error_t {
    FILESYSTEM_SUCCESS = 0,
    FILESYSTEM_ERROR_INVALID_PATH = -1,
    FILESYSTEM_ERROR_INVALID_MOUNT = -2,
    FILESYSTEM_ERROR_FAILED_TO_READ_ZIP = -3,
    FILESYSTEM_ERROR_FAILED_TO_CLOSE_ZIP = -4,
    FILESYSTEM_ERROR_FAILED_TO_READ_FILE = -5,
    FILESYSTEM_ERROR_FILE_NOT_FOUND = -6,
    FILESYSTEM_ERROR_DIR_NOT_FOUND = -7,
    FILESYSTEM_ERROR_INVALID_PARAMETER = -8,
    FILESYSTEM_ERROR_BUFFER_TOO_SMALL = -9,
} neko_filesystem_error_t;

struct neko_filesystem_internal_file_t {
    int size;
    int zip_index;
    int collated_index;
};

struct neko_filesystem_internal_folder_t {
    int collated_index;
};

enum neko_filesystem_internal_mount_type_t {
    FILESYSTEM_INTERNAL_MOUNT_TYPE_DIR,
};

struct neko_filesystem_internal_mount_t {
    FILESYSTEM_U64 path;
    FILESYSTEM_U64 mounted_as;
    int mount_len;
    enum neko_filesystem_internal_mount_type_t type;

    struct neko_filesystem_internal_file_t* files;
    int files_count;
    int files_capacity;

    struct neko_filesystem_internal_folder_t* dirs;
    int dirs_count;
    int dirs_capacity;
};

struct neko_filesystem_internal_collated_t {
    FILESYSTEM_U64 path;
    int parent;
    int ref_count;
    int is_file;
};

struct neko_filesystem_t {
    void* memctx;
    strpool_t strpool;

    struct neko_filesystem_internal_mount_t* mounts;
    int mounts_count;
    int mounts_capacity;

    struct neko_filesystem_internal_collated_t* collated;
    int collated_count;
    int collated_capacity;

    char temp[260];
};

typedef struct neko_filesystem_t neko_filesystem_t;

NEKO_API_DECL void neko_filesystem_create_internal(neko_filesystem_t* sys, void* memctx);
NEKO_API_DECL neko_filesystem_t* neko_filesystem_create(void* memctx);
NEKO_API_DECL void neko_filesystem_destroy_internal(neko_filesystem_t* sys);
NEKO_API_DECL void neko_filesystem_destroy(neko_filesystem_t* sys);

NEKO_API_DECL neko_filesystem_error_t neko_filesystem_mount(neko_filesystem_t* sys, char const* path, char const* mount_as);
NEKO_API_DECL neko_filesystem_error_t neko_filesystem_mount_from_memory(neko_filesystem_t* sys, void const* data, int size, char const* mount_as);
NEKO_API_DECL neko_filesystem_error_t neko_filesystem_dismount(neko_filesystem_t* sys, char const* path, char const* mounted_as);

typedef struct neko_filesystem_file_t {
    FILESYSTEM_U64 mount;
    FILESYSTEM_U64 path;
    int index;
} neko_filesystem_file_t;

NEKO_API_DECL neko_filesystem_error_t neko_filesystem_file(neko_filesystem_t* sys, char const* path, neko_filesystem_file_t* file);
NEKO_API_DECL neko_filesystem_error_t neko_filesystem_file_load(neko_filesystem_t* sys, neko_filesystem_file_t file, int* size, void* buffer, int capacity);
NEKO_API_DECL int neko_filesystem_file_size(neko_filesystem_t* sys, neko_filesystem_file_t file);
NEKO_API_DECL int neko_filesystem_file_count(neko_filesystem_t* sys, char const* path);
NEKO_API_DECL char const* neko_filesystem_file_name(neko_filesystem_t* sys, char const* path, int index);
NEKO_API_DECL char const* neko_filesystem_file_path(neko_filesystem_t* sys, char const* path, int index);
NEKO_API_DECL int neko_filesystem_subdir_count(neko_filesystem_t* sys, char const* path);
NEKO_API_DECL char const* neko_filesystem_subdir_name(neko_filesystem_t* sys, char const* path, int index);
NEKO_API_DECL char const* neko_filesystem_subdir_path(neko_filesystem_t* sys, char const* path, int index);

extern const char* neko_filewatch_error_reason;

typedef struct neko_filewatch_t neko_filewatch_t;

NEKO_API_DECL void neko_filewatch_create_internal(neko_filewatch_t* filewatch, struct neko_filesystem_t* assetsys, void* mem_ctx);
NEKO_API_DECL neko_filewatch_t* neko_filewatch_create(struct neko_filesystem_t* assetsys, void* mem_ctx);
NEKO_API_DECL void neko_filewatch_free_internal(neko_filewatch_t* filewatch);
NEKO_API_DECL void neko_filewatch_free(neko_filewatch_t* filewatch);

#define FILEWATCH_MAX_MOUNTS (128)

NEKO_API_DECL int neko_filewatch_mount(neko_filewatch_t* filewatch, const char* actual_path, const char* mount_as_virtual_path);
NEKO_API_DECL void neko_filewatch_dismount(neko_filewatch_t* filewatch, const char* actual_path, const char* virtual_path);
NEKO_API_DECL int neko_filewatch_update(neko_filewatch_t* filewatch);
NEKO_API_DECL void neko_filewatch_notify(neko_filewatch_t* filewatch);

typedef enum neko_filewatch_update_t {
    FILEWATCH_DIR_ADDED,
    FILEWATCH_DIR_REMOVED,
    FILEWATCH_FILE_ADDED,
    FILEWATCH_FILE_REMOVED,
    FILEWATCH_FILE_MODIFIED,
} neko_filewatch_update_t;

typedef void(neko_filewatch_callback_t)(neko_filewatch_update_t change, const char* virtual_path, void* udata);

NEKO_API_DECL int neko_filewatch_start_watching(neko_filewatch_t* filewatch, const char* virtual_path, neko_filewatch_callback_t* cb, void* udata);
NEKO_API_DECL void neko_filewatch_stop_watching(neko_filewatch_t* filewatch, const char* virtual_path);
NEKO_API_DECL int neko_filewatch_actual_path_to_virtual_path(neko_filewatch_t* filewatch, const char* actual_path, char* virtual_path, int virtual_path_capacity);
NEKO_API_DECL int neko_filewatch_virtual_path_to_actual_path(neko_filewatch_t* filewatch, const char* virtual_path, char* actual_path, int actual_path_capacity);

typedef struct neko_filewatch_path_t {
    u64 actual_id;
    u64 virtual_id;
} neko_filewatch_path_t;

typedef struct neko_filewatch_entry_internal_t {
    neko_filewatch_path_t path;
    u64 name_id;
    int is_dir;
    neko_file_time_t time;
} neko_filewatch_entry_internal_t;

typedef struct neko_filewatch_watched_dir_internal_t {
    neko_filewatch_path_t dir_path;
    neko_filewatch_callback_t* cb;
    void* udata;
    hashtable_t entries;
} neko_filewatch_watched_dir_internal_t;

typedef struct neko_filewatch_notification_internal_t {
    neko_filewatch_callback_t* cb;
    void* udata;
    neko_filewatch_update_t change;
    neko_filewatch_path_t path;
} neko_filewatch_notification_internal_t;

struct neko_filewatch_t {
    neko_filesystem_t* assetsys;
    int mount_count;
    neko_filewatch_path_t mount_paths[FILEWATCH_MAX_MOUNTS];

    int watch_count;
    int watch_capacity;
    neko_filewatch_watched_dir_internal_t* watches;

    int notification_count;
    int notification_capacity;
    neko_filewatch_notification_internal_t* notifications;

    void* mem_ctx;
    strpool_t strpool;
};

#define NEKO_FILEWATCH_H
#endif
