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
    static inline const_str DEFAULT_FONT = "assets/fonts/Monocraft.ttf";
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

NEKO_SCRIPT(
        fs,

        typedef struct vfs_file {
            const_str data;
            size_t len;
            u64 offset;
        } vfs_file;

        NEKO_EXPORT size_t neko_capi_vfs_fread(void* dest, size_t size, size_t count, vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fseek(vfs_file* vf, u64 of, int whence);

        NEKO_EXPORT u64 neko_capi_vfs_ftell(vfs_file * vf);

        NEKO_EXPORT vfs_file neko_capi_vfs_fopen(const_str path);

        NEKO_EXPORT int neko_capi_vfs_fclose(vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fscanf(vfs_file* vf, const char* format, ...);

        NEKO_EXPORT bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);

        NEKO_EXPORT const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t* size);

)

}  // namespace Neko

struct file_dateinfo_t {
    file_dateinfo_t() : year(0), month(0), day(0), hour(0), minute(0), second(0) {}

    Int32 year;
    Int32 month;
    Int32 day;
    Int32 hour;
    Int32 minute;
    Int32 second;
};

struct file_interface_t {
    const u8* (*pfnLoadFile)(const Char* pstrpath, Uint32* psize);
    bool (*pfnWriteFile)(const u8* pdata, Uint32 size, const Char* pstrpath, bool append);
    void (*pfnFreeFile)(const void* pfile);
    bool (*pfnFileExists)(const Char* pstrpath);
    bool (*pfnDeleteFile)(const Char* pstrpath);
    bool (*pfnCreateDirectory)(const Char* pstrpath);
    bool (*pfnGetFileDate)(const Char* pstrFile, file_dateinfo_t& dateinfo);
    Int32 (*pfnCompareFileDates)(const file_dateinfo_t& d1, const file_dateinfo_t& d2);
    const Char* (*pfnGetGameDirectory)(void);
};