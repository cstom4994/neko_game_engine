#include "neko.hpp"

#include <cstdio>
#include <filesystem>
#include <new>

#include "engine/neko_platform.h"

// deps
#include <direct.h>

#include "deps/miniz.h"
#include "deps/tinydir.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace neko {

static char s_empty[1] = {0};

string_builder::string_builder() {
    data = s_empty;
    len = 0;
    capacity = 0;
}

void string_builder::trash() {
    if (data != s_empty) {
        neko_safe_free(data);
    }
}

void string_builder::reserve(u64 cap) {
    if (cap > capacity) {
        char *buf = (char *)neko_safe_malloc(cap);
        memset(buf, 0, cap);
        memcpy(buf, data, len);

        if (data != s_empty) {
            neko_safe_free(data);
        }

        data = buf;
        capacity = cap;
    }
}

void string_builder::clear() {
    len = 0;
    if (data != s_empty) {
        data[0] = 0;
    }
}

void string_builder::swap_filename(string filepath, string file) {
    clear();

    u64 slash = filepath.last_of('/');
    if (slash != (u64)-1) {
        string path = filepath.substr(0, slash + 1);
        *this << path;
    }

    *this << file;
}

void string_builder::concat(string str, s32 times) {
    for (s32 i = 0; i < times; i++) {
        *this << str;
    }
}

string_builder &string_builder::operator<<(string str) {
    u64 desired = len + str.len + 1;
    u64 cap = capacity;

    if (desired >= cap) {
        u64 growth = cap > 0 ? cap * 2 : 8;
        if (growth <= desired) {
            growth = desired;
        }

        reserve(growth);
    }

    memcpy(&data[len], str.data, str.len);
    len += str.len;
    data[len] = 0;
    return *this;
}

string_builder::operator string() { return {data, len}; }

struct arena_node {
    arena_node *next;
    u64 capacity;
    u64 allocd;
    u64 prev;
    u8 buf[1];
};

static u64 align_forward(u64 p, u32 align) {
    if ((p & (align - 1)) != 0) {
        p += align - (p & (align - 1));
    }
    return p;
}

static arena_node *arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(arena_node, buf);
    if (capacity < page) {
        capacity = page;
    }

    arena_node *a = (arena_node *)neko_safe_malloc(offsetof(arena_node, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

void arena::trash() {
    arena_node *a = head;
    while (a != nullptr) {
        arena_node *rm = a;
        a = a->next;
        neko_safe_free(rm);
    }
}

void *arena::bump(u64 size) {
    if (head == nullptr) {
        head = arena_block_make(size);
    }

    u64 next = 0;
    do {
        next = align_forward(head->allocd, 16);
        if (next + size <= head->capacity) {
            break;
        }

        arena_node *block = arena_block_make(size);
        block->next = head;

        head = block;
    } while (true);

    void *ptr = &head->buf[next];
    head->allocd = next + size;
    head->prev = next;
    return ptr;
}

void *arena::rebump(void *ptr, u64 old, u64 size) {
    if (head == nullptr || ptr == nullptr || old == 0) {
        return bump(size);
    }

    if (&head->buf[head->prev] == ptr) {
        u64 resize = head->prev + size;
        if (resize <= head->capacity) {
            head->allocd = resize;
            return ptr;
        }
    }

    void *new_ptr = bump(size);

    u64 copy = old < size ? old : size;
    memmove(new_ptr, ptr, copy);

    return new_ptr;
}

string arena::bump_string(string s) {
    if (s.len > 0) {
        char *cstr = (char *)bump(s.len + 1);
        memcpy(cstr, s.data, s.len);
        cstr[s.len] = '\0';
        return {cstr, s.len};
    } else {
        return {};
    }
}

static u32 read4(char *bytes) {
    u32 n;
    memcpy(&n, bytes, 4);
    return n;
}

static bool read_entire_file_raw(string *out, string filepath) {
    // PROFILE_FUNC();

    string path = to_cstr(filepath);
    neko_defer(neko_safe_free(path.data));

    FILE *file = fopen(path.data, "rb");
    if (file == nullptr) {
        return false;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = (char *)neko_safe_malloc(size + 1);
    size_t read = fread(buf, sizeof(char), size, file);
    fclose(file);

    if (read != size) {
        neko_safe_free(buf);
        return false;
    }

    buf[size] = 0;
    *out = {buf, size};
    return true;
}

static bool list_all_files_help(array<string> *files, string path) {
    // PROFILE_FUNC();

    tinydir_dir dir;
    if (path.len == 0) {
        tinydir_open(&dir, ".");
    } else {
        tinydir_open(&dir, path.data);
    }
    neko_defer(tinydir_close(&dir));

    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0) {
            if (file.is_dir) {
                string s = str_fmt("%s%s/", path.data, file.name);
                neko_defer(neko_safe_free(s.data));
                list_all_files_help(files, s);
            } else {
                files->push(str_fmt("%s%s", path.data, file.name));
            }
        }

        tinydir_next(&dir);
    }

    return true;
}

struct IFileSystem {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual bool mount(string filepath) = 0;
    virtual bool file_exists(string filepath) = 0;
    virtual bool read_entire_file(string *out, string filepath) = 0;
    virtual bool list_all_files(array<string> *files) = 0;
};

static IFileSystem *g_filesystem;

struct DirectoryFileSystem : IFileSystem {
    void make() {}
    void trash() {}

    bool mount(string filepath) {
        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        s32 res = chdir(path.data);
        return res == 0;
    }

    bool file_exists(string filepath) {
        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        FILE *fp = fopen(path.data, "r");
        if (fp != nullptr) {
            fclose(fp);
            return true;
        }

        return false;
    }

    bool read_entire_file(string *out, string filepath) { return read_entire_file_raw(out, filepath); }

    bool list_all_files(array<string> *files) { return list_all_files_help(files, ""); }
};

struct ZipFileSystem : IFileSystem {
    std::mutex mtx;
    mz_zip_archive zip = {};
    string zip_contents = {};

    void make() {}

    void trash() {
        if (zip_contents.data != nullptr) {
            mz_zip_reader_end(&zip);
            neko_safe_free(zip_contents.data);
        }
    }

    bool mount(string filepath) {
        // PROFILE_FUNC();

        string contents = {};
        bool contents_ok = read_entire_file_raw(&contents, filepath);
        if (!contents_ok) {
            return false;
        }

        bool success = false;
        neko_defer({
            if (!success) {
                neko_safe_free(contents.data);
            }
        });

        char *data = contents.data;
        char *end = &data[contents.len];

        constexpr s32 eocd_size = 22;
        char *eocd = end - eocd_size;
        if (read4(eocd) != 0x06054b50) {
            fprintf(stderr, "can't find EOCD record\n");
            return false;
        }

        u32 central_size = read4(&eocd[12]);
        if (read4(eocd - central_size) != 0x02014b50) {
            fprintf(stderr, "can't find central directory\n");
            return false;
        }

        u32 central_offset = read4(&eocd[16]);
        char *begin = eocd - central_size - central_offset;
        u64 zip_len = end - begin;
        if (read4(begin) != 0x04034b50) {
            fprintf(stderr, "can't read local file header\n");
            return false;
        }

        mz_bool zip_ok = mz_zip_reader_init_mem(&zip, begin, zip_len, 0);
        if (!zip_ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read zip: %s\n", mz_zip_get_error_string(err));
            return false;
        }

        zip_contents = contents;

        success = true;
        return true;
    }

    bool file_exists(string filepath) {
        // PROFILE_FUNC();

        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        std::unique_lock<std::mutex> lock(mtx);

        s32 i = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
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

    bool read_entire_file(string *out, string filepath) {
        // PROFILE_FUNC();

        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        std::unique_lock<std::mutex> lock(mtx);

        s32 file_index = mz_zip_reader_locate_file(&zip, path.data, nullptr, 0);
        if (file_index == -1) {
            return false;
        }

        mz_zip_archive_file_stat stat;
        mz_bool ok = mz_zip_reader_file_stat(&zip, file_index, &stat);
        if (!ok) {
            return false;
        }

        size_t size = stat.m_uncomp_size;
        char *buf = (char *)neko_safe_malloc(size + 1);

        ok = mz_zip_reader_extract_to_mem(&zip, file_index, buf, size, 0);
        if (!ok) {
            mz_zip_error err = mz_zip_get_last_error(&zip);
            fprintf(stderr, "failed to read file '%s': %s\n", path.data, mz_zip_get_error_string(err));
            neko_safe_free(buf);
            return false;
        }

        buf[size] = 0;
        *out = {buf, size};
        return true;
    }

    bool list_all_files(array<string> *files) {
        // PROFILE_FUNC();

        std::unique_lock<std::mutex> lock(mtx);

        for (u32 i = 0; i < mz_zip_reader_get_num_files(&zip); i++) {
            mz_zip_archive_file_stat file_stat;
            mz_bool ok = mz_zip_reader_file_stat(&zip, i, &file_stat);
            if (!ok) {
                return false;
            }

            string name = {file_stat.m_filename, strlen(file_stat.m_filename)};
            files->push(to_cstr(name));
        }

        return true;
    }
};

#ifdef __EMSCRIPTEN__
EM_JS(char *, web_mount_dir, (), { return stringToNewUTF8(spryMount); });

EM_ASYNC_JS(void, web_load_zip, (), {
    var dirs = spryMount.split("/");
    dirs.pop();

    var path = [];
    for (var dir of dirs) {
        path.push(dir);
        FS.mkdir(path.join("/"));
    }

    await fetch(spryMount).then(async function(res) {
        if (!res.ok) {
            throw new Error("failed to fetch " + spryMount);
        }

        var data = await res.arrayBuffer();
        FS.writeFile(spryMount, new Uint8Array(data));
    });
});

EM_ASYNC_JS(void, web_load_files, (), {
    var jobs = [];

    function spryWalkFiles(files, leading) {
        var path = leading.join("/");
        if (path != "") {
            FS.mkdir(path);
        }

        for (var entry of Object.entries(files)) {
            var key = entry[0];
            var value = entry[1];
            var filepath = [... leading, key ];
            if (typeof value == "object") {
                spryWalkFiles(value, filepath);
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
    spryWalkFiles(spryFiles, []);

    await Promise.all(jobs);
});
#endif

auto os_program_path() { return std::filesystem::current_path().string(); }

template <typename T>
static bool vfs_mount_type(string mount) {
    void *ptr = neko_safe_malloc(sizeof(T));
    T *vfs = new (ptr) T();

    vfs->make();
    bool ok = vfs->mount(mount);
    if (!ok) {
        vfs->trash();
        neko_safe_free(vfs);
        return false;
    }

    g_filesystem = vfs;
    return true;
}

// string os_program_path2() {
//     static char s_buf[2048];
//     DWORD len = GetModuleFileNameA(NULL, s_buf, neko_arr_size(s_buf));
//     for (s32 i = 0; s_buf[i]; i++) {
//         if (s_buf[i] == '\\') {
//             s_buf[i] = '/';
//         }
//     }
//     return {s_buf, (u64)len};
// }

mount_result vfs_mount(const char *filepath) {
    // PROFILE_FUNC();

    mount_result res = {};

#ifdef __EMSCRIPTEN__
    string mount_dir = web_mount_dir();
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
        string path = os_program_path().c_str();

#ifndef NDEBUG
        printf("program path: %s\n", path.data);
#endif

        res.ok = vfs_mount_type<DirectoryFileSystem>(path);
    } else {
        string mount_dir = filepath;

        if (mount_dir.ends_with(".zip")) {
            res.ok = vfs_mount_type<ZipFileSystem>(mount_dir);
            res.is_fused = true;
        } else {
            res.ok = vfs_mount_type<DirectoryFileSystem>(mount_dir);
            res.can_hot_reload = res.ok;
        }
    }
#endif

    if (filepath != nullptr && !res.ok) {
        neko_log_error("%s", tmp_fmt("failed to load: %s", filepath));
    }

    return res;
}

void vfs_trash() {
    if (g_filesystem != nullptr) {
        g_filesystem->trash();
        neko_safe_free(g_filesystem);
    }
}

bool vfs_file_exists(string filepath) { return g_filesystem->file_exists(filepath); }

bool vfs_read_entire_file(string *out, string filepath) { return g_filesystem->read_entire_file(out, filepath); }

bool vfs_list_all_files(array<string> *files) { return g_filesystem->list_all_files(files); }

struct AudioFile {
    u8 *buf;
    u64 cursor;
    u64 len;
};

#if 0
void *vfs_for_miniaudio() {
    ma_vfs_callbacks vtbl = {};

    vtbl.onOpen = [](ma_vfs *pVFS, const char *pFilePath, ma_uint32 openMode, ma_vfs_file *pFile) -> ma_result {
        string contents = {};

        if (openMode & MA_OPEN_MODE_WRITE) {
            return MA_ERROR;
        }

        bool ok = vfs_read_entire_file(&contents, pFilePath);
        if (!ok) {
            return MA_ERROR;
        }

        AudioFile *file = (AudioFile *)neko_safe_malloc(sizeof(AudioFile));
        file->buf = (u8 *)contents.data;
        file->len = contents.len;
        file->cursor = 0;

        *pFile = file;
        return MA_SUCCESS;
    };

    vtbl.onClose = [](ma_vfs *pVFS, ma_vfs_file file) -> ma_result {
        AudioFile *f = (AudioFile *)file;
        neko_safe_free(f->buf);
        neko_safe_free(f);
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

    ma_vfs_callbacks *ptr = (ma_vfs_callbacks *)neko_safe_malloc(sizeof(ma_vfs_callbacks));
    *ptr = vtbl;
    return ptr;
}
#endif

}  // namespace neko