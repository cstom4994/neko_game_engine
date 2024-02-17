

#ifndef NEKO_FILEWATCH_H
#define NEKO_FILEWATCH_H

#include "engine/neko.h"

// strpool
#include "strpool.h"

#ifndef ASSETSYS_U64
#define ASSETSYS_U64 unsigned long long
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

typedef enum neko_assetsys_error_t {
    ASSETSYS_SUCCESS = 0,
    ASSETSYS_ERROR_INVALID_PATH = -1,
    ASSETSYS_ERROR_INVALID_MOUNT = -2,
    ASSETSYS_ERROR_FAILED_TO_READ_ZIP = -3,
    ASSETSYS_ERROR_FAILED_TO_CLOSE_ZIP = -4,
    ASSETSYS_ERROR_FAILED_TO_READ_FILE = -5,
    ASSETSYS_ERROR_FILE_NOT_FOUND = -6,
    ASSETSYS_ERROR_DIR_NOT_FOUND = -7,
    ASSETSYS_ERROR_INVALID_PARAMETER = -8,
    ASSETSYS_ERROR_BUFFER_TOO_SMALL = -9,
} neko_assetsys_error_t;

struct strpool_embedded_internal_hash_slot_t;
struct strpool_embedded_internal_entry_t;
struct strpool_embedded_internal_handle_t;
struct strpool_embedded_internal_block_t;

struct strpool_embedded_t {
    void* memctx;
    int ignore_case;
    int counter_shift;
    u64 counter_mask;
    u64 index_mask;

    int initial_entry_capacity;
    int initial_block_capacity;
    int block_size;
    int min_data_size;

    struct strpool_embedded_internal_hash_slot_t* hash_table;
    int hash_capacity;

    struct strpool_embedded_internal_entry_t* entries;
    int entry_capacity;
    int entry_count;

    struct strpool_embedded_internal_handle_t* handles;
    int handle_capacity;
    int handle_count;
    int handle_freelist_head;
    int handle_freelist_tail;

    struct strpool_embedded_internal_block_t* blocks;
    int block_capacity;
    int block_count;
    int current_block;
};

typedef struct strpool_embedded_t strpool_embedded_t;

typedef struct strpool_embedded_config_t {
    void* memctx;
    int ignore_case;
    int counter_bits;
    int index_bits;
    int entry_capacity;
    int block_capacity;
    int block_size;
    int min_length;
} strpool_embedded_config_t;

extern strpool_embedded_config_t const strpool_embedded_default_config;

NEKO_API_DECL void strpool_embedded_init(strpool_embedded_t* pool, strpool_embedded_config_t const* config);
NEKO_API_DECL void strpool_embedded_term(strpool_embedded_t* pool);

NEKO_API_DECL u64 strpool_embedded_inject(strpool_embedded_t* pool, char const* string, int length);
NEKO_API_DECL char const* strpool_embedded_cstr(strpool_embedded_t const* pool, u64 handle);

struct neko_assetsys_internal_file_t {
    int size;
    int zip_index;
    int collated_index;
};

struct neko_assetsys_internal_folder_t {
    int collated_index;
};

enum neko_assetsys_internal_mount_type_t {
    ASSETSYS_INTERNAL_MOUNT_TYPE_DIR,
};

struct neko_assetsys_internal_mount_t {
    ASSETSYS_U64 path;
    ASSETSYS_U64 mounted_as;
    int mount_len;
    enum neko_assetsys_internal_mount_type_t type;

    struct neko_assetsys_internal_file_t* files;
    int files_count;
    int files_capacity;

    struct neko_assetsys_internal_folder_t* dirs;
    int dirs_count;
    int dirs_capacity;
};

struct neko_assetsys_internal_collated_t {
    ASSETSYS_U64 path;
    int parent;
    int ref_count;
    int is_file;
};

struct neko_assetsys_t {
    void* memctx;
    strpool_t strpool;

    struct neko_assetsys_internal_mount_t* mounts;
    int mounts_count;
    int mounts_capacity;

    struct neko_assetsys_internal_collated_t* collated;
    int collated_count;
    int collated_capacity;

    char temp[260];
};

typedef struct neko_assetsys_t neko_assetsys_t;

NEKO_API_DECL void neko_assetsys_create_internal(neko_assetsys_t* sys, void* memctx);
NEKO_API_DECL neko_assetsys_t* neko_assetsys_create(void* memctx);
NEKO_API_DECL void neko_assetsys_destroy_internal(neko_assetsys_t* sys);
NEKO_API_DECL void neko_assetsys_destroy(neko_assetsys_t* sys);

NEKO_API_DECL neko_assetsys_error_t neko_assetsys_mount(neko_assetsys_t* sys, char const* path, char const* mount_as);
NEKO_API_DECL neko_assetsys_error_t neko_assetsys_mount_from_memory(neko_assetsys_t* sys, void const* data, int size, char const* mount_as);
NEKO_API_DECL neko_assetsys_error_t neko_assetsys_dismount(neko_assetsys_t* sys, char const* path, char const* mounted_as);

typedef struct neko_assetsys_file_t {
    ASSETSYS_U64 mount;
    ASSETSYS_U64 path;
    int index;
} neko_assetsys_file_t;

NEKO_API_DECL neko_assetsys_error_t neko_assetsys_file(neko_assetsys_t* sys, char const* path, neko_assetsys_file_t* file);
NEKO_API_DECL neko_assetsys_error_t neko_assetsys_file_load(neko_assetsys_t* sys, neko_assetsys_file_t file, int* size, void* buffer, int capacity);
NEKO_API_DECL int neko_assetsys_file_size(neko_assetsys_t* sys, neko_assetsys_file_t file);

NEKO_API_DECL int neko_assetsys_file_count(neko_assetsys_t* sys, char const* path);
NEKO_API_DECL char const* neko_assetsys_file_name(neko_assetsys_t* sys, char const* path, int index);
NEKO_API_DECL char const* neko_assetsys_file_path(neko_assetsys_t* sys, char const* path, int index);

NEKO_API_DECL int neko_assetsys_subdir_count(neko_assetsys_t* sys, char const* path);
NEKO_API_DECL char const* neko_assetsys_subdir_name(neko_assetsys_t* sys, char const* path, int index);
NEKO_API_DECL char const* neko_assetsys_subdir_path(neko_assetsys_t* sys, char const* path, int index);

extern const char* neko_filewatch_error_reason;

typedef struct neko_filewatch_t neko_filewatch_t;

NEKO_API_DECL void neko_filewatch_create_internal(neko_filewatch_t* filewatch, struct neko_assetsys_t* assetsys, void* mem_ctx);
NEKO_API_DECL neko_filewatch_t* neko_filewatch_create(struct neko_assetsys_t* assetsys, void* mem_ctx);

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
    neko_assetsys_t* assetsys;
    int mount_count;
    neko_filewatch_path_t mount_paths[FILEWATCH_MAX_MOUNTS];

    int watch_count;
    int watch_capacity;
    neko_filewatch_watched_dir_internal_t* watches;

    int notification_count;
    int notification_capacity;
    neko_filewatch_notification_internal_t* notifications;

    void* mem_ctx;
    strpool_embedded_t strpool;
};

#define NEKO_FILEWATCH_H
#endif
