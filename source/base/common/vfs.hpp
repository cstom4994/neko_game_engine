#pragma once

#include "base/common/array.hpp"
#include "base/common/base.hpp"
#include "base/common/os.hpp"
#include "base/common/string.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/singleton.hpp"
#include "base/common/map.hpp"
#include "base/common/logger.hpp"
#include "base/common/reflection.hpp"

// miniz
#include "extern/miniz.h"

struct lua_State;

namespace Neko {

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

class FileSystem {
public:
    FileSystem() = default;
    virtual ~FileSystem() = default;

    virtual bool mount() { return false; };
    virtual void trash() = 0;
    virtual bool file_exists(String filepath) = 0;
    virtual bool read_entire_file(String* out, String filepath) = 0;
    virtual bool list_all_files(Array<String>* files) = 0;
    virtual u64 file_modtime(String filepath) = 0;
};

class DirectoryFileSystem : public FileSystem {
public:
    String basepath;

    ~DirectoryFileSystem();

    void trash();
    bool mount(String filepath);
    bool file_exists(String filepath);
    bool read_entire_file(String* out, String filepath);
    bool list_all_files(Array<String>* files);
    u64 file_modtime(String filepath);
};

class ZipFileSystem : public FileSystem {
public:
    Mutex mtx{};
    mz_zip_archive zip{};
    String zip_contents{};

    ~ZipFileSystem();

    void trash();
    bool mount(String filepath);
    bool file_exists(String filepath);
    bool read_entire_file(String* out, String filepath);
    bool list_all_files(Array<String>* files);
    u64 file_modtime(String filepath);
};

bool read_entire_file_raw(String* out, String filepath);

class VFS : public SingletonClass<VFS> {
private:
    UnorderedMap<String, FileSystem*> vfs_map;

    std::unordered_map<std::string, std::string> vfs_redirect;

public:
    template <typename T, typename... Args>
    inline bool mount_type(String fsname, String redirect, Args&&... args)
        requires(std::is_base_of_v<FileSystem, T>)
    {
        T* fs = mem_new<T>();
        bool ok = fs->mount(std::forward<Args>(args)...);
        if (!ok) {
            fs->trash();
            mem_del(fs);
            return false;
        }
        vfs_map[to_cstr(fsname)] = fs;
        vfs_redirect[fsname.cstr()] = redirect.cstr();
        LOG_TRACE("vfs mount {} as {}", reflection::GetTypeName<T>(), fsname.cstr());
        return true;
    }

    inline bool is_mount(String fsname) { return vfs_map.find(fsname) != nullptr; }

    void vfs_fini();

    String filepath_redirect(String file);
    u64 file_modtime(String filepath);
    bool file_exists(String filepath);
    bool read_entire_file(String* out, String filepath);
    // bool vfs_write_entire_file(String fsname, String filepath, String contents);
    bool list_all_files(String fsname, Array<String>* files);
};

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
