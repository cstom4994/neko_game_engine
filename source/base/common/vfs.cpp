#include "vfs.hpp"

#include <filesystem>

#include "base/common/hashmap.hpp"
#include "base/common/profiler.hpp"
#include "base/common/util.hpp"

// miniz
#include "vendor/miniz.h"

void fatal_error(Neko::String str);

namespace Neko {

static u32 read4(char *bytes) {
    u32 n;
    memcpy(&n, bytes, 4);
    return n;
}

bool read_entire_file_raw(String *out, String filepath) {
    PROFILE_FUNC();

    String path = to_cstr(filepath);
    neko_defer(mem_free(path.data));

    FILE *file = neko_fopen(path.data, "rb");
    if (file == nullptr) {
        console_log("failed to load file %s", path.data);
        return false;
    }

    neko_fseek(file, 0L, SEEK_END);
    size_t size = neko_ftell(file);
    rewind(file);

    char *buf = (char *)mem_alloc(size + 1);
    size_t read = neko_fread(buf, sizeof(char), size, file);
    neko_fclose(file);

    if (read != size) {
        mem_free(buf);
        return false;
    }

    buf[size] = 0;
    *out = {buf, size};
    return true;
}

static bool list_all_files_help(Array<String> *files, const_str path) {
    PROFILE_FUNC();
    std::filesystem::path search_path = (path != nullptr) ? path : std::filesystem::current_path();
    for (const auto &entry : std::filesystem::recursive_directory_iterator(search_path)) {
        if (!entry.is_regular_file()) continue;
        std::filesystem::path relative_path = std::filesystem::relative(entry.path(), search_path);
        files->push(str_fmt("%s", relative_path.generic_string().c_str()));
    }
    return true;
}

struct FileSystem {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual bool mount(String filepath) = 0;
    virtual bool file_exists(String filepath) = 0;
    virtual bool read_entire_file(String *out, String filepath) = 0;
    virtual bool list_all_files(Array<String> *files) = 0;
    virtual u64 file_modtime(String filepath) = 0;
};

static HashMap<FileSystem *> g_vfs;

struct DirectoryFileSystem : FileSystem {
    String basepath;

    void make() {}
    void trash() { mem_free(basepath.data); }

    bool mount(String filepath) {
        // String path = to_cstr(filepath);
        // neko_defer(mem_free(path.data));
        // i32 res = os_change_dir(path.data);
        // return res == 0;
        basepath = to_cstr(filepath);
        return true;
    }

    bool file_exists(String filepath) {
        String path = str_fmt("%s/%s", basepath.cstr(), filepath.cstr());
        neko_defer(mem_free(path.data));

        FILE *fp = neko_fopen(path.cstr(), "r");
        if (fp != nullptr) {
            neko_fclose(fp);
            return true;
        }

        return false;
    }

    bool read_entire_file(String *out, String filepath) {
        if (!file_exists(filepath)) return false;
        String path = str_fmt("%s/%s", basepath.cstr(), filepath.cstr());
        neko_defer(mem_free(path.data));
        return read_entire_file_raw(out, path);
    }

    bool list_all_files(Array<String> *files) { return list_all_files_help(files, basepath.cstr()); }

    u64 file_modtime(String filepath) {
        if (!file_exists(filepath)) return 0;
        String path = tmp_fmt("%s/%s", basepath.cstr(), filepath.cstr());
        u64 modtime = os_file_modtime(path.cstr());
        return modtime;
    }
};

struct ZipFileSystem : FileSystem {
    Mutex mtx = {};
    mz_zip_archive zip = {};
    String zip_contents = {};

    void make() {}

    void trash() {
        if (zip_contents.data != nullptr) {
            mz_zip_reader_end(&zip);
            mem_free(zip_contents.data);
        }
    }

    bool mount(String filepath) {
        PROFILE_FUNC();

        String contents = {};
        bool contents_ok = false;

        // 判断是否已经挂载gamedata
        // 如果已经挂载gamedata则其余所有ZipFileSystem均加载自gamedata
        if (g_vfs.get(fnv1a(NEKO_PACKS::GAMEDATA))) {
            contents_ok = vfs_read_entire_file(&contents, filepath);
        } else {
            contents_ok = read_entire_file_raw(&contents, filepath);
        }

        if (!contents_ok) {
            return false;
        }

        bool success = false;
        neko_defer({
            if (!success) {
                mem_free(contents.data);
            }
        });

        char *data = contents.data;
        char *end = &data[contents.len];

        constexpr i32 eocd_size = 22;
        char *eocd = end - eocd_size;
        if (read4(eocd) != 0x06054b50) {
            console_log("zip_fs failed to find EOCD record");
            return false;
        }

        u32 central_size = read4(&eocd[12]);
        if (read4(eocd - central_size) != 0x02014b50) {
            console_log("zip_fs failed to find central directory");
            return false;
        }

        u32 central_offset = read4(&eocd[16]);
        char *begin = eocd - central_size - central_offset;
        u64 zip_len = end - begin;
        if (read4(begin) != 0x04034b50) {
            console_log("zip_fs failed to read local file header");
            return false;
        }

        mz_bool zip_ok = mz_zip_reader_init_mem(&zip, begin, zip_len, 0);
        if (!zip_ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            console_log("zip_fs failed to read zip: %s", mz_zip_get_error_string(err));
            return false;
        }

        zip_contents = contents;

        success = true;
        return true;
    }

    bool file_exists(String filepath) {
        PROFILE_FUNC();

        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        LockGuard<Mutex> lock{mtx};

        i32 i = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (i == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, i, &stat);
        if (!ok) {
            return false;
        }

        return true;
    }

    bool read_entire_file(String *out, String filepath) {
        PROFILE_FUNC();

        String path = to_cstr(filepath);
        neko_defer(mem_free(path.data));

        LockGuard<Mutex> lock{mtx};

        i32 file_index = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (file_index == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, file_index, &stat);
        if (!ok) {
            return false;
        }

        size_t size = stat.m_uncomp_size;
        char *buf = (char *)mem_alloc(size + 1);

        ok = mz_zip_reader_extract_to_mem(&zip, file_index, buf, size, 0);
        if (!ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read file '%s': %s\n", path.data, mz_zip_get_error_string(err));
            mem_free(buf);
            return false;
        }

        buf[size] = 0;
        *out = {buf, size};
        return true;
    }

    bool list_all_files(Array<String> *files) {
        PROFILE_FUNC();

        LockGuard<Mutex> lock{mtx};

        for (u32 i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
            mz_zip_archive_file_stat file_stat;
            mz_bool ok = mz_zip_reader_file_stat(&zip, i, &file_stat);
            if (!ok) {
                return false;
            }

            String name = {file_stat.m_filename, strlen(file_stat.m_filename)};
            files->push(to_cstr(name));
        }

        return true;
    }

    u64 file_modtime(String filepath) { return 0; }
};

#if defined(NEKO_IS_WEB)
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(nekoMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
    var dirs = nekoMount.split("/");
    dirs.pop();

    var path = [];
    for (var dir of dirs) {
        path.push(dir);
        FS.mkdir(path.join("/"));
    }

    await fetch(nekoMount).then(async function(res) {
        if (!res.ok) {
            throw new Error("failed to fetch " + nekoMount);
        }

        var data = await res.arrayBuffer();
        FS.writeFile(nekoMount, new Uint8Array(data));
    });
});

EM_ASYNC_JS(void, web_load_files, (), {
    var jobs = [];

    function nekoWalkFiles(files, leading) {
        var path = leading.join("/");
        if (path != "") {
            FS.mkdir(path);
        }

        for (var entry of Object.entries(files)) {
            var key = entry[0];
            var value = entry[1];
            var filepath = [... leading, key ];
            if (typeof value == "object") {
                nekoWalkFiles(value, filepath);
            } else if (value == 1) {
                var file = filepath.join("/");

                var job = fetch(file).then(async function(res) {
                    if (!res.ok) {
                        throw new Error("failed to fetch " + file);
                    }
                    var data = await res.arrayBuffer();
                    FS.writeFile(file, new Uint8Array(data));
                });

                jobs.push(job);
            }
        }
    }
    nekoWalkFiles(nekoFiles, []);

    await Promise.all(jobs);
});
#endif

template <typename T>
static bool vfs_mount_type(String fsname, String mount) {
    void *ptr = mem_alloc(sizeof(T));
    T *vfs = new (ptr) T();

    vfs->make();
    bool ok = vfs->mount(mount);
    if (!ok) {
        vfs->trash();
        mem_free(vfs);
        return false;
    }

    g_vfs[fnv1a(fsname)] = vfs;

    return true;
}

MountResult vfs_mount(const_str fsname, const char *filepath) {
    PROFILE_FUNC();

    MountResult res = {};

#if defined(NEKO_IS_WEB)
    String mount_dir = web_mount_dir();
    neko_defer(free(mount_dir.data));

    if (mount_dir.ends_with(".zip")) {
        web_load_zip();
        res.ok = vfs_mount_type<ZipFileSystem>(mount_dir);
    } else {
        web_load_files();
        res.ok = vfs_mount_type<DirectoryFileSystem>(mount_dir);
    }
#else
    if (filepath == nullptr) {

        String path = os_program_path();
        console_log("program path: %s", path.data);

        res.ok = vfs_mount_type<ZipFileSystem>(fsname, path);
        if (!res.ok) {
            res.ok = vfs_mount_type<ZipFileSystem>(fsname, "./gamedata.zip");
            res.is_fused = true;
            console_log("zip_fs load with gamedata.zip");
        }
    } else {
        String mount_dir = filepath;

        if (mount_dir.ends_with(".zip")) {
            res.ok = vfs_mount_type<ZipFileSystem>(fsname, mount_dir);
            res.is_fused = true;
        } else {
            res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, mount_dir);
            res.can_hot_reload = res.ok;
        }
    }
#endif

    if (filepath != nullptr && !res.ok) {
        fatal_error(tmp_fmt("failed to load: %s", filepath));
    }

    return res;
}

void vfs_fini() {
    for (auto vfs : g_vfs) {
        console_log("vfs_fini(%llu)", vfs.key);
        (*vfs.value)->trash();
        mem_free((*vfs.value));
    }

    g_vfs.trash();
}

u64 vfs_file_modtime(String filepath) {
    for (auto v : g_vfs) {
        FileSystem *vfs = static_cast<FileSystem *>(*v.value);
        if (vfs->file_exists(filepath)) {
            return vfs->file_modtime(filepath);
        }
    }
    return 0;
}

bool vfs_file_exists(String filepath) {
    for (auto v : g_vfs) {
        FileSystem *vfs = static_cast<FileSystem *>(*v.value);
        if (vfs->file_exists(filepath)) return true;
    }
    return false;
}

bool vfs_read_entire_file(String *out, String filepath) {
    for (auto v : g_vfs) {
        FileSystem *vfs = static_cast<FileSystem *>(*v.value);
        if (vfs->file_exists(filepath)) return vfs->read_entire_file(out, filepath);
    }
    return false;
}

bool vfs_list_all_files(String fsname, Array<String> *files) { return g_vfs[fnv1a(fsname)]->list_all_files(files); }

struct AudioFile {
    u8 *buf;
    u64 cursor;
    u64 len;
};

void *vfs_for_miniaudio() {
#if NEKO_AUDIO == 1
    ma_vfs_callbacks vtbl = {};

    vtbl.onOpen = [](ma_vfs *pVFS, const char *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile) -> ma_result {
        String contents = {};

        if (openMode & MA_OPEN_MODE_WRITE) {
            return MA_ERROR;
        }

        bool ok = vfs_read_entire_file(&contents, pFilePath);
        if (!ok) {
            return MA_ERROR;
        }

        AudioFile *file = (AudioFile *)mem_alloc(sizeof(AudioFile));
        file->buf = (u8 *)contents.data;
        file->len = contents.len;
        file->cursor = 0;

        *pFile = file;
        return MA_SUCCESS;
    };

    vtbl.onClose = [](ma_vfs *pVFS, ma_vfs_file file) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        mem_free(f->buf);
        mem_free(f);
        return MA_SUCCESS;
    };

    vtbl.onRead = [](ma_vfs *pVFS, ma_vfs_file file, void *pDst, size_t sizeInBytes, size_t *pBytesRead) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        u64 remaining = f->len - f->cursor;
        u64 len = remaining < sizeInBytes ? remaining : sizeInBytes;
        memcpy(pDst, &f->buf[f->cursor], len);

        if (pBytesRead != nullptr) {
            *pBytesRead = len;
        }

        if (len != sizeInBytes) {
            return MA_AT_END;
        }

        return MA_SUCCESS;
    };

    vtbl.onWrite = [](ma_vfs *pVFS, ma_vfs_file file, const void *pSrc, size_t sizeInBytes, size_t *pBytesWritten) -> ma_result { return MA_NOT_IMPLEMENTED; };

    vtbl.onSeek = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) -> ma_result {
        AudioFile *f = (AudioFile *)file;

        i64 seek = 0;
        switch (origin) {
            case ma_seek_origin_start:
                seek = offset;
                break;
            case ma_seek_origin_end:
                seek = f->len + offset;
                break;
            case ma_seek_origin_current:
            default:
                seek = f->cursor + offset;
                break;
        }

        if (seek < 0 || seek > f->len) {
            return MA_ERROR;
        }

        f->cursor = (u64)seek;
        return MA_SUCCESS;
    };

    vtbl.onTell = [](ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        *pCursor = f->cursor;
        return MA_SUCCESS;
    };

    vtbl.onInfo = [](ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        pInfo->sizeInBytes = f->len;
        return MA_SUCCESS;
    };

    ma_vfs_callbacks *ptr = (ma_vfs_callbacks *)mem_alloc(sizeof(ma_vfs_callbacks));
    *ptr = vtbl;
    return ptr;
#else
    return NULL;
#endif
}

// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2

size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf) {
    size_t bytes_to_read = size * count;
    std::memcpy(dest, static_cast<const char *>(vf->data) + vf->offset, bytes_to_read);
    vf->offset += bytes_to_read;
    return count;
}

int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence) {
    u64 new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = of;
            break;
        case SEEK_CUR:
            new_offset = vf->offset + of;
            break;
        case SEEK_END:
            new_offset = vf->len + of;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    if (new_offset < 0 || new_offset > vf->len) {
        errno = EINVAL;
        return -1;
    }
    vf->offset = new_offset;
    return 0;
}

u64 neko_capi_vfs_ftell(vfs_file *vf) { return vf->offset; }

vfs_file neko_capi_vfs_fopen(const_str path) {
    vfs_file vf{};
    vf.data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path, &vf.len);
    return vf;
}

int neko_capi_vfs_fclose(vfs_file *vf) {
    neko_assert(vf);
    mem_free(vf->data);
    return 0;
}

int neko_capi_vfs_fscanf(vfs_file *vf, const char *format, ...) {
    if (!vf || !format) {
        errno = EINVAL;
        return -1;
    }
    va_list args;
    va_start(args, format);

    const char *fmt = format;
    int count = 0;

    while (*fmt) {
        // 跳过格式字符串中的空格
        while (isspace(*fmt)) ++fmt;
        if (*fmt == '\0') break;
        if (*fmt != '%') {
            // 匹配单个字符文字
            if (*fmt != static_cast<const char *>(vf->data)[vf->offset]) {
                va_end(args);
                return count;
            }
            ++fmt;
            ++vf->offset;
            continue;
        }

        ++fmt;  // 跳过 '%'
        if (*fmt == '\0') {
            break;
        }

        // 支持的格式说明符: %d, %u, %s, %c
        switch (*fmt) {
            case 'd': {
                int *int_arg = va_arg(args, int *);
                int scanned;
                int bytes_read = sscanf(static_cast<const char *>(vf->data) + vf->offset, "%d", &scanned);
                if (bytes_read == 1) {
                    *int_arg = scanned;
                    vf->offset += std::to_string(scanned).length();
                    ++count;
                }
                break;
            }
            case 'u': {
                unsigned int *uint_arg = va_arg(args, unsigned int *);
                unsigned int scanned;
                int bytes_read = sscanf(static_cast<const char *>(vf->data) + vf->offset, "%u", &scanned);
                if (bytes_read == 1) {
                    *uint_arg = scanned;
                    vf->offset += std::to_string(scanned).length();
                    ++count;
                }
                break;
            }
            case 's': {
                char *str_arg = va_arg(args, char *);
                int read = 0;
                while (!isspace(static_cast<const char *>(vf->data)[vf->offset]) && static_cast<const char *>(vf->data)[vf->offset] != '\0') {
                    str_arg[read++] = static_cast<const char *>(vf->data)[vf->offset++];
                }
                str_arg[read] = '\0';
                ++count;
                break;
            }
            case 'c': {
                char *char_arg = va_arg(args, char *);
                *char_arg = static_cast<const char *>(vf->data)[vf->offset++];
                ++count;
                break;
            }
            default:
                // 不支持的格式说明符
                va_end(args);
                return count;
        }
        ++fmt;
    }

    va_end(args);
    return count;
}

bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath) { return vfs_file_exists(filepath); }

const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size) {
    String out;
    bool ok = vfs_read_entire_file(&out, filepath);
    if (!ok) return NULL;
    *size = out.len;
    return out.data;
}

}  // namespace Neko