#pragma once

#include "base/common/array.hpp"
#include "base/common/base.hpp"
#include "base/common/os.hpp"
#include "base/common/string.hpp"

namespace Neko {

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

struct NEKO_PACKS {
    static inline const_str GAMEDATA = "default_pack";
    static inline const_str LUACODE = "luacode";
};

struct MountResult {
    bool ok;
    bool can_hot_reload;
    bool is_fused;
};

bool read_entire_file_raw(String* out, String filepath);

MountResult vfs_mount(const_str fsname, const char* filepath);
void vfs_fini();

u64 vfs_file_modtime(String filepath);
bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String* out, String filepath);
bool vfs_write_entire_file(String fsname, String filepath, String contents);
bool vfs_list_all_files(String fsname, Array<String>* files);

void* vfs_for_miniaudio();

typedef struct vfs_file {
    const_str data;
    size_t len;
    u64 offset;
} vfs_file;

size_t neko_capi_vfs_fread(void* dest, size_t size, size_t count, vfs_file* vf);
int neko_capi_vfs_fseek(vfs_file* vf, u64 of, int whence);
u64 neko_capi_vfs_ftell(vfs_file* vf);
vfs_file neko_capi_vfs_fopen(const_str path);
int neko_capi_vfs_fclose(vfs_file* vf);
int neko_capi_vfs_fscanf(vfs_file* vf, const char* format, ...);
bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);
const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t* size);

}  // namespace Neko
