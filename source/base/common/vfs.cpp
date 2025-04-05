#include "vfs.hpp"

#include <filesystem>
#include <format>

#include "base/common/hashmap.hpp"
#include "base/common/profiler.hpp"
#include "base/common/util.hpp"
#include "base/common/logger.hpp"
#include "base/scripting/luax.h"
#include "base/scripting/lua_wrapper.hpp"

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
        LOG_INFO("failed to load file {}", path.data);
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

DirectoryFileSystem::~DirectoryFileSystem() {}

void DirectoryFileSystem::trash() { mem_free(basepath.data); }

bool DirectoryFileSystem::mount(String filepath) {
    // String path = to_cstr(filepath);
    // neko_defer(mem_free(path.data));
    // i32 res = os_change_dir(path.data);
    // return res == 0;
    basepath = to_cstr(filepath);
    return true;
}

bool DirectoryFileSystem::file_exists(String filepath) {
    String path = str_fmt("%s/%s", basepath.cstr(), filepath.cstr());
    neko_defer(mem_free(path.data));

    FILE *fp = neko_fopen(path.cstr(), "r");
    if (fp != nullptr) {
        neko_fclose(fp);
        return true;
    }

    return false;
}

bool DirectoryFileSystem::read_entire_file(String *out, String filepath) {
    if (!file_exists(filepath)) return false;
    String path = str_fmt("%s/%s", basepath.cstr(), filepath.cstr());
    neko_defer(mem_free(path.data));
    return read_entire_file_raw(out, path);
}

bool DirectoryFileSystem::list_all_files(Array<String> *files) { return list_all_files_help(files, basepath.cstr()); }

u64 DirectoryFileSystem::file_modtime(String filepath) {
    if (!file_exists(filepath)) return 0;
    String path = tmp_fmt("%s/%s", basepath.cstr(), filepath.cstr());
    u64 modtime = os_file_modtime(path.cstr());
    return modtime;
}

ZipFileSystem::~ZipFileSystem() {}

void ZipFileSystem::trash() {
    if (zip_contents.data != nullptr) {
        mz_zip_reader_end(&zip);
        mem_free(zip_contents.data);
    }
}

bool ZipFileSystem::mount(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool contents_ok = false;

    // 判断是否已经挂载gamedata
    // 如果已经挂载gamedata则其余所有ZipFileSystem均加载自gamedata
    if (the<VFS>().is_mount("gamedata")) {
        contents_ok = the<VFS>().read_entire_file(&contents, filepath);
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
        LOG_INFO("zip_fs failed to find EOCD record");
        return false;
    }

    u32 central_size = read4(&eocd[12]);
    if (read4(eocd - central_size) != 0x02014b50) {
        LOG_INFO("zip_fs failed to find central directory");
        return false;
    }

    u32 central_offset = read4(&eocd[16]);
    char *begin = eocd - central_size - central_offset;
    u64 zip_len = end - begin;
    if (read4(begin) != 0x04034b50) {
        LOG_INFO("zip_fs failed to read local file header");
        return false;
    }

    mz_bool zip_ok = mz_zip_reader_init_mem(&zip, begin, zip_len, 0);
    if (!zip_ok) {
        mz_zip_error err = mz_zip_get_last_error(&zip);
        LOG_INFO("zip_fs failed to read zip: {}", mz_zip_get_error_string(err));
        return false;
    }

    zip_contents = contents;

    success = true;
    return true;
}

bool ZipFileSystem::file_exists(String filepath) {
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

bool ZipFileSystem::read_entire_file(String *out, String filepath) {
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

bool ZipFileSystem::list_all_files(Array<String> *files) {
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

u64 ZipFileSystem::file_modtime(String filepath) { return 0; }

void VFS::vfs_fini() {
    for (auto &[name, vfs] : vfs_map) {
        LOG_INFO("vfs_fini({})", name.cstr());
        FileSystem *fs = vfs;
        fs->trash();
        mem_del(fs);
        mem_free(name.data);
    }

    vfs_map.trash();
}

String VFS::filepath_redirect(String file) {
    if (!file.starts_with("@")) return file;
    for (auto &[name, redirect] : vfs_redirect) {
        if (!file.starts_with("@" + name + "/")) continue;
        return str_fmt("%s/%s", redirect.c_str(), file.substr(2 + name.size(), file.len).data);
    }
    return file;
}

u64 VFS::file_modtime(String filepath) {
    String file = filepath_redirect(filepath);
    for (auto &[name, vfs_] : vfs_map) {
        FileSystem *vfs = static_cast<FileSystem *>(vfs_);
        if (vfs->file_exists(file)) {
            return vfs->file_modtime(file);
        }
    }
    return 0;
}

bool VFS::file_exists(String filepath) {
    String file = filepath_redirect(filepath);
    for (auto &[name, vfs_] : vfs_map) {
        FileSystem *vfs = static_cast<FileSystem *>(vfs_);
        if (vfs->file_exists(file)) return true;
    }
    return false;
}

bool VFS::read_entire_file(String *out, String filepath) {
    String file = filepath_redirect(filepath);
    for (auto &[name, vfs_] : vfs_map) {
        FileSystem *vfs = static_cast<FileSystem *>(vfs_);
        if (vfs->file_exists(file)) return vfs->read_entire_file(out, file);
    }
    return false;
}

bool VFS::list_all_files(String fsname, Array<String> *files) { return vfs_map[fsname]->list_all_files(files); }

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
    vf.data = neko_capi_vfs_read_file("gamedata", path, &vf.len);
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

bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath) { return the<VFS>().file_exists(filepath); }

const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size) {
    String out;
    bool ok = the<VFS>().read_entire_file(&out, filepath);
    if (!ok) return NULL;
    *size = out.len;
    return out.data;
}

}  // namespace Neko