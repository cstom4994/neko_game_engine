#pragma once

#include "engine/base/array.hpp"
#include "engine/base/base.hpp"
#include "engine/base/os.hpp"
#include "engine/base/string.hpp"

struct NEKO_PACKS {
    static constexpr const_str GAMEDATA = "default_pack";
    static constexpr const_str LUACODE = "luacode";
    static constexpr const_str DEFAULT_FONT = "assets/fonts/Monocraft.ttf";
};

struct MountResult {
    bool ok;
    bool can_hot_reload;
    bool is_fused;
};

bool read_entire_file_raw(String *out, String filepath);

MountResult vfs_mount(const_str fsname, const char *filepath);
void vfs_fini();

u64 vfs_file_modtime(String filepath);
bool vfs_file_exists(String filepath);
bool vfs_read_entire_file(String *out, String filepath);
bool vfs_write_entire_file(String fsname, String filepath, String contents);
bool vfs_list_all_files(String fsname, Array<String> *files);

void *vfs_for_miniaudio();

NEKO_SCRIPT(
        fs,

        typedef struct vfs_file {
            const_str data;
            size_t len;
            u64 offset;
        } vfs_file;

        NEKO_EXPORT size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf);

        NEKO_EXPORT int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence);

        NEKO_EXPORT u64 neko_capi_vfs_ftell(vfs_file * vf);

        NEKO_EXPORT vfs_file neko_capi_vfs_fopen(const_str path);

        NEKO_EXPORT int neko_capi_vfs_fclose(vfs_file *vf);

        NEKO_EXPORT int neko_capi_vfs_fscanf(vfs_file *vf, const char *format, ...);

        NEKO_EXPORT bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);

        NEKO_EXPORT const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size);

)