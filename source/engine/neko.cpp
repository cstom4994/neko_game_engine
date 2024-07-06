#include "neko.hpp"

#include <cstdio>
#include <filesystem>
#include <new>

#include "engine/neko_engine.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.hpp"
#include "engine/neko_reflection.hpp"

// deps
#if defined(NEKO_PF_WIN)
#include <Windows.h>
#include <direct.h>
#endif

#include <miniz.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(NEKO_PF_WIN)
#include <direct.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#elif defined(NEKO_PF_WEB)
#include <unistd.h>

#elif defined(NEKO_PF_LINUX)
#include <sched.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#endif

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    size_t wlen = wtf8_to_utf16_length(str.data(), str.size());
    if (wlen == (size_t)-1) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    wtf8_to_utf16(str.data(), str.size(), wresult.data(), wlen);
    return wresult;
}

std::string w2u(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    size_t len = wtf8_from_utf16_length(wstr.data(), wstr.size());
    std::string result(len, '\0');
    wtf8_from_utf16(wstr.data(), wstr.size(), result.data(), len);
    return result;
}
}  // namespace neko::wtf8

#if defined(NEKO_PF_WIN)

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    const int wlen = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (wlen <= 0) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wresult.data(), static_cast<int>(wresult.size()));
    return wresult;
}

std::string w2u(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    const int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, 0, 0);
    if (len <= 0) {
        return "";
    }
    std::string result(len, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), static_cast<int>(result.size()), 0, 0);
    return result;
}

std::wstring a2w(std::string_view str) noexcept {
    if (str.empty()) {
        return L"";
    }
    const int wlen = ::MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (wlen <= 0) {
        return L"";
    }
    std::wstring wresult(wlen, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), wresult.data(), static_cast<int>(wresult.size()));
    return wresult;
}

std::string w2a(std::wstring_view wstr) noexcept {
    if (wstr.empty()) {
        return "";
    }
    const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, 0, 0);
    if (len <= 0) {
        return "";
    }
    std::string result(len, '\0');
    ::WideCharToMultiByte(CP_ACP, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), static_cast<int>(result.size()), 0, 0);
    return result;
}

std::string a2u(std::string_view str) noexcept { return w2u(a2w(str)); }

std::string u2a(std::string_view str) noexcept { return w2a(u2w(str)); }
}  // namespace neko::win

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

    arena_node *a = (arena_node *)neko_safe_malloc(NEKO_OFFSET(arena_node, buf[capacity]));
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

struct IFileSystem {
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual bool mount(string filepath) = 0;
    virtual bool file_exists(string filepath) = 0;
    virtual bool read_entire_file(string *out, string filepath) = 0;
};

// TODO 统一管理全局变量
static std::unordered_map<std::string, IFileSystem *> g_filesystem_list;

struct DirectoryFileSystem : IFileSystem {
    void make() {}
    void trash() {}

    bool mount(string filepath) {
        string path = to_cstr(filepath);
        neko_defer(neko_safe_free(path.data));

        s32 res = neko_pf_chdir(path.data);
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

    // bool list_all_files(array<string> *files) { return list_all_files_help(files, ""); }
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

// auto os_program_path() { return std::filesystem::current_path().string(); }

template <typename T>
static bool vfs_mount_type(std::string fsname, string mount) {
    void *ptr = neko_safe_malloc(sizeof(T));
    T *vfs = new (ptr) T();

    vfs->make();
    bool ok = vfs->mount(mount);
    if (!ok) {
        vfs->trash();
        neko_safe_free(vfs);
        return false;
    }

    g_filesystem_list.insert(std::make_pair(fsname, vfs));
    return true;
}

// string os_program_path2() {
//     static char s_buf[2048];
//     DWORD len = GetModuleFileNameA(NULL, s_buf, NEKO_ARR_SIZE(s_buf));
//     for (s32 i = 0; s_buf[i]; i++) {
//         if (s_buf[i] == '\\') {
//             s_buf[i] = '/';
//         }
//     }
//     return {s_buf, (u64)len};
// }

mount_result vfs_mount(const_str fsname, const_str filepath) {

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
        string path = os_program_path();

#ifndef NDEBUG
        NEKO_DEBUG_LOG("program path: %s", path.data);
#endif

        res.ok = vfs_mount_type<DirectoryFileSystem>(fsname, path);
    } else {
        string mount_dir = filepath;

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
        NEKO_ERROR("%s", tmp_fmt("failed to load: %s", filepath).data);
    }

    return res;
}

void vfs_fini(std::optional<std::string> name) {
    auto fini_fs = []<typename T>(T fs) {
        if constexpr (!is_pair<T>::value) {
            fs->trash();
            neko_safe_free(fs);
            NEKO_DEBUG_LOG("vfs_fini(%p)", fs);
        } else {
            fs.second->trash();
            neko_safe_free(fs.second);
            NEKO_DEBUG_LOG("vfs_fini(%s)", fs.first.c_str());
        }
    };
    if (!name.has_value()) {
        for (auto vfs : g_filesystem_list) fini_fs(vfs);
    } else {
        auto vfs = g_filesystem_list[name.value()];
        fini_fs(vfs);
    }
}

bool vfs_file_exists(std::string fsname, string filepath) { return g_filesystem_list[fsname]->file_exists(filepath); }

bool vfs_read_entire_file(std::string fsname, string *out, string filepath) { return g_filesystem_list[fsname]->read_entire_file(out, filepath); }

NEKO_API_DECL size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf) {
    size_t bytes_to_read = size * count;
    std::memcpy(dest, static_cast<const char *>(vf->data) + vf->offset, bytes_to_read);
    vf->offset += bytes_to_read;
    return count;
}

// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2

NEKO_API_DECL int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence) {
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

NEKO_API_DECL u64 neko_capi_vfs_ftell(vfs_file *vf) { return vf->offset; }

NEKO_API_DECL vfs_file neko_capi_vfs_fopen(const_str path) {
    vfs_file vf{};
    vf.data = neko_capi_vfs_read_file(NEKO_PACK_GAMEDATA, path, &vf.len);
    return vf;
}

NEKO_API_DECL int neko_capi_vfs_fclose(vfs_file *vf) {
    NEKO_ASSERT(vf);
    neko_safe_free(vf->data);
    return 0;
}

NEKO_API_DECL bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath) { return vfs_file_exists(fsname, filepath); }

NEKO_API_DECL const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size) {
    string out;
    bool ok = vfs_read_entire_file(fsname, &out, filepath);
    if (!ok) return NULL;
    *size = out.len;
    return out.data;
}

s64 luax_len(lua_State *L, s32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

string luax_check_string(lua_State *L, s32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

inline bool is_whitespace(char c) {
    switch (c) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

double string_to_double(string str) {
    double n = 0;
    double sign = 1;

    if (str.len == 0) {
        return n;
    }

    u64 i = 0;
    if (str.data[0] == '-' && str.len > 1 && is_digit(str.data[1])) {
        i++;
        sign = -1;
    }

    while (i < str.len) {
        if (!is_digit(str.data[i])) {
            break;
        }

        n = n * 10 + (str.data[i] - '0');
        i++;
    }

    if (i < str.len && str.data[i] == '.') {
        i++;
        double place = 10;
        while (i < str.len) {
            if (!is_digit(str.data[i])) {
                break;
            }

            n += (str.data[i] - '0') / place;
            place *= 10;
            i++;
        }
    }

    return n * sign;
}

enum JSONTok : s32 {
    JSONTok_Invalid,
    JSONTok_LBrace,    // {
    JSONTok_RBrace,    // }
    JSONTok_LBracket,  // [
    JSONTok_RBracket,  // ]
    JSONTok_Colon,     // :
    JSONTok_Comma,     // ,
    JSONTok_True,      // true
    JSONTok_False,     // false
    JSONTok_Null,      // null
    JSONTok_String,    // "[^"]*"
    JSONTok_Number,    // [0-9]+\.?[0-9]*
    JSONTok_Error,
    JSONTok_EOF,
};

const char *json_tok_string(JSONTok tok) {
    switch (tok) {
        case JSONTok_Invalid:
            return "Invalid";
        case JSONTok_LBrace:
            return "LBrace";
        case JSONTok_RBrace:
            return "RBrace";
        case JSONTok_LBracket:
            return "LBracket";
        case JSONTok_RBracket:
            return "RBracket";
        case JSONTok_Colon:
            return "Colon";
        case JSONTok_Comma:
            return "Comma";
        case JSONTok_True:
            return "True";
        case JSONTok_False:
            return "False";
        case JSONTok_Null:
            return "Null";
        case JSONTok_String:
            return "string";
        case JSONTok_Number:
            return "Number";
        case JSONTok_Error:
            return "Error";
        case JSONTok_EOF:
            return "EOF";
        default:
            return "?";
    }
}

const char *json_kind_string(JSONKind kind) {
    switch (kind) {
        case JSONKind_Null:
            return "Null";
        case JSONKind_Object:
            return "Object";
        case JSONKind_Array:
            return "Array";
        case JSONKind_String:
            return "string";
        case JSONKind_Number:
            return "Number";
        case JSONKind_Boolean:
            return "Boolean";
        default:
            return "?";
    }
};

struct JSONToken {
    JSONTok kind;
    string str;
    u32 line;
    u32 column;
};

struct JSONScanner {
    string contents;
    JSONToken token;
    u64 begin;
    u64 end;
    u32 line;
    u32 column;
};

static char json_peek(JSONScanner *scan, u64 offset) { return scan->contents.data[scan->end + offset]; }

static bool json_at_end(JSONScanner *scan) { return scan->end == scan->contents.len; }

static void json_next_char(JSONScanner *scan) {
    if (!json_at_end(scan)) {
        scan->end++;
        scan->column++;
    }
}

static void json_skip_whitespace(JSONScanner *scan) {
    while (true) {
        switch (json_peek(scan, 0)) {
            case '\n':
                scan->column = 0;
                scan->line++;
            case ' ':
            case '\t':
            case '\r':
                json_next_char(scan);
                break;
            default:
                return;
        }
    }
}

static string json_lexeme(JSONScanner *scan) { return scan->contents.substr(scan->begin, scan->end); }

static JSONToken json_make_tok(JSONScanner *scan, JSONTok kind) {
    JSONToken t = {};
    t.kind = kind;
    t.str = json_lexeme(scan);
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_err_tok(JSONScanner *scan, string msg) {
    JSONToken t = {};
    t.kind = JSONTok_Error;
    t.str = msg;
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_scan_ident(arena *a, JSONScanner *scan) {
    while (is_alpha(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    JSONToken t = {};
    t.str = json_lexeme(scan);

    if (t.str == "true") {
        t.kind = JSONTok_True;
    } else if (t.str == "false") {
        t.kind = JSONTok_False;
    } else if (t.str == "null") {
        t.kind = JSONTok_Null;
    } else {
        string_builder sb = {};
        neko_defer(sb.trash());

        string s = string(sb << "unknown identifier: '" << t.str << "'");
        return json_err_tok(scan, a->bump_string(s));
    }

    scan->token = t;
    return t;
}

static JSONToken json_scan_number(JSONScanner *scan) {
    if (json_peek(scan, 0) == '-' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '-'
    }

    while (is_digit(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    if (json_peek(scan, 0) == '.' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '.'

        while (is_digit(json_peek(scan, 0))) {
            json_next_char(scan);
        }
    }

    return json_make_tok(scan, JSONTok_Number);
}

static JSONToken json_scan_string(JSONScanner *scan) {
    while (json_peek(scan, 0) != '"' && !json_at_end(scan)) {
        json_next_char(scan);
    }

    if (json_at_end(scan)) {
        return json_err_tok(scan, "unterminated string");
    }

    json_next_char(scan);
    return json_make_tok(scan, JSONTok_String);
}

static JSONToken json_scan_next(arena *a, JSONScanner *scan) {
    json_skip_whitespace(scan);

    scan->begin = scan->end;

    if (json_at_end(scan)) {
        return json_make_tok(scan, JSONTok_EOF);
    }

    char c = json_peek(scan, 0);
    json_next_char(scan);

    if (is_alpha(c)) {
        return json_scan_ident(a, scan);
    }

    if (is_digit(c) || (c == '-' && is_digit(json_peek(scan, 0)))) {
        return json_scan_number(scan);
    }

    if (c == '"') {
        return json_scan_string(scan);
    }

    switch (c) {
        case '{':
            return json_make_tok(scan, JSONTok_LBrace);
        case '}':
            return json_make_tok(scan, JSONTok_RBrace);
        case '[':
            return json_make_tok(scan, JSONTok_LBracket);
        case ']':
            return json_make_tok(scan, JSONTok_RBracket);
        case ':':
            return json_make_tok(scan, JSONTok_Colon);
        case ',':
            return json_make_tok(scan, JSONTok_Comma);
    }

    string msg = tmp_fmt("unexpected character: '%c' (%d)", c, (int)c);
    string s = a->bump_string(msg);
    return json_err_tok(scan, s);
}

static string json_parse_next(arena *a, JSONScanner *scan, JSON *out);

static string json_parse_object(arena *a, JSONScanner *scan, JSONObject **out) {

    JSONObject *obj = nullptr;

    json_scan_next(a, scan);  // eat brace

    while (true) {
        if (scan->token.kind == JSONTok_RBrace) {
            *out = obj;
            json_scan_next(a, scan);
            return {};
        }

        string err = {};

        JSON key = {};
        err = json_parse_next(a, scan, &key);
        if (err.data != nullptr) {
            return err;
        }

        if (key.kind != JSONKind_String) {
            string msg = tmp_fmt("expected string as object key on line: %d. got: %s", (s32)scan->token.line, json_kind_string(key.kind));
            return a->bump_string(msg);
        }

        if (scan->token.kind != JSONTok_Colon) {
            string msg = tmp_fmt("expected colon on line: %d. got %s", (s32)scan->token.line, json_tok_string(scan->token.kind));
            return a->bump_string(msg);
        }

        json_scan_next(a, scan);

        JSON value = {};
        err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONObject *entry = (JSONObject *)a->bump(sizeof(JSONObject));
        entry->next = obj;
        entry->hash = fnv1a(key.str);
        entry->key = key.str;
        entry->value = value;

        obj = entry;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static string json_parse_array(arena *a, JSONScanner *scan, JSONArray **out) {

    JSONArray *arr = nullptr;

    json_scan_next(a, scan);  // eat bracket

    while (true) {
        if (scan->token.kind == JSONTok_RBracket) {
            *out = arr;
            json_scan_next(a, scan);
            return {};
        }

        JSON value = {};
        string err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONArray *el = (JSONArray *)a->bump(sizeof(JSONArray));
        el->next = arr;
        el->value = value;
        el->index = 0;

        if (arr != nullptr) {
            el->index = arr->index + 1;
        }

        arr = el;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static string json_parse_next(arena *a, JSONScanner *scan, JSON *out) {
    switch (scan->token.kind) {
        case JSONTok_LBrace: {
            out->kind = JSONKind_Object;
            return json_parse_object(a, scan, &out->object);
        }
        case JSONTok_LBracket: {
            out->kind = JSONKind_Array;
            return json_parse_array(a, scan, &out->array);
        }
        case JSONTok_String: {
            out->kind = JSONKind_String;
            out->str = scan->token.str.substr(1, scan->token.str.len - 1);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Number: {
            out->kind = JSONKind_Number;
            out->number = string_to_double(scan->token.str);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_True: {
            out->kind = JSONKind_Boolean;
            out->boolean = true;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_False: {
            out->kind = JSONKind_Boolean;
            out->boolean = false;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Null: {
            out->kind = JSONKind_Null;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Error: {
            string_builder sb = {};
            neko_defer(sb.trash());

            sb << scan->token.str << tmp_fmt(" on line %d:%d", (s32)scan->token.line, (s32)scan->token.column);

            return a->bump_string(string(sb));
        }
        default: {
            string msg = tmp_fmt("unknown json token: %s on line %d:%d", json_tok_string(scan->token.kind), (s32)scan->token.line, (s32)scan->token.column);
            return a->bump_string(msg);
        }
    }
}

void JSONDocument::parse(string contents) {

    arena = {};

    JSONScanner scan = {};
    scan.contents = contents;
    scan.line = 1;

    json_scan_next(&arena, &scan);

    string err = json_parse_next(&arena, &scan, &root);
    if (err.data != nullptr) {
        error = err;
        return;
    }

    if (scan.token.kind != JSONTok_EOF) {
        error = "expected EOF";
        return;
    }
}

void JSONDocument::trash() { arena.trash(); }

JSON JSON::lookup(string key, bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        for (JSONObject *o = object; o != nullptr; o = o->next) {
            if (o->hash == fnv1a(key)) {
                return o->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSON JSON::index(s32 i, bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        for (JSONArray *a = array; a != nullptr; a = a->next) {
            if (a->index == i) {
                return a->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSONObject *JSON::as_object(bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        return object;
    }

    *ok = false;
    return {};
}

JSONArray *JSON::as_array(bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        return array;
    }

    *ok = false;
    return {};
}

string JSON::as_string(bool *ok) {
    if (*ok && kind == JSONKind_String) {
        return str;
    }

    *ok = false;
    return {};
}

double JSON::as_number(bool *ok) {
    if (*ok && kind == JSONKind_Number) {
        return number;
    }

    *ok = false;
    return {};
}

JSONObject *JSON::lookup_object(string key, bool *ok) { return lookup(key, ok).as_object(ok); }

JSONArray *JSON::lookup_array(string key, bool *ok) { return lookup(key, ok).as_array(ok); }

string JSON::lookup_string(string key, bool *ok) { return lookup(key, ok).as_string(ok); }

double JSON::lookup_number(string key, bool *ok) { return lookup(key, ok).as_number(ok); }

double JSON::index_number(s32 i, bool *ok) { return index(i, ok).as_number(ok); }

static void json_write_string(string_builder &sb, JSON *json, s32 level) {
    switch (json->kind) {
        case JSONKind_Object: {
            sb << "{\n";
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                sb.concat("  ", level);
                sb << o->key;
                json_write_string(sb, &o->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "}";
            break;
        }
        case JSONKind_Array: {
            sb << "[\n";
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                sb.concat("  ", level);
                json_write_string(sb, &a->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "]";
            break;
        }
        case JSONKind_String:
            sb << "\"" << json->str << "\"";
            break;
        case JSONKind_Number:
            sb << tmp_fmt("%g", json->number);
            break;
        case JSONKind_Boolean:
            sb << (json->boolean ? "true" : "false");
            break;
        case JSONKind_Null:
            sb << "null";
            break;
        default:
            break;
    }
}

void json_write_string(string_builder *sb, JSON *json) { json_write_string(*sb, json, 1); }

void json_print(JSON *json) {
    string_builder sb = {};
    neko_defer(sb.trash());
    json_write_string(&sb, json);
    printf("%s\n", sb.data);
}

void json_to_lua(lua_State *L, JSON *json) {
    switch (json->kind) {
        case JSONKind_Object: {
            lua_newtable(L);
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                lua_pushlstring(L, o->key.data, o->key.len);
                json_to_lua(L, &o->value);
                lua_rawset(L, -3);
            }
            break;
        }
        case JSONKind_Array: {
            lua_newtable(L);
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                json_to_lua(L, &a->value);
                lua_rawseti(L, -2, a->index + 1);
            }
            break;
        }
        case JSONKind_String: {
            lua_pushlstring(L, json->str.data, json->str.len);
            break;
        }
        case JSONKind_Number: {
            lua_pushnumber(L, json->number);
            break;
        }
        case JSONKind_Boolean: {
            lua_pushboolean(L, json->boolean);
            break;
        }
        case JSONKind_Null: {
            lua_pushnil(L);
            break;
        }
        default:
            break;
    }
}

static void lua_to_json_string(string_builder &sb, lua_State *L, hashmap<bool> *visited, string *err, s32 width, s32 level) {
    auto indent = [&](s32 offset) {
        if (width > 0) {
            sb << "\n";
            sb.concat(" ", width * (level + offset));
        }
    };

    if (err->len != 0) {
        return;
    }

    s32 top = lua_gettop(L);
    switch (lua_type(L, top)) {
        case LUA_TTABLE: {
            uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

            bool *visit = nullptr;
            bool exist = visited->find_or_insert(ptr, &visit);
            if (exist && *visit) {
                *err = "table has cycles";
                return;
            }

            *visit = true;

            lua_pushnil(L);
            if (lua_next(L, -2) == 0) {
                sb << "[]";
                return;
            }

            s32 key_type = lua_type(L, -2);

            if (key_type == LUA_TNUMBER) {
                sb << "[";

                indent(0);
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                s32 len = luax_len(L, top);
                assert(len > 0);
                s32 i = 1;
                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be numbers";
                        return;
                    }

                    sb << ",";
                    indent(0);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    i++;
                }
                indent(-1);
                sb << "]";

                if (i != len) {
                    *err = "array is not continuous";
                    return;
                }
            } else if (key_type == LUA_TSTRING) {
                sb << "{";
                indent(0);

                lua_pushvalue(L, -2);
                lua_to_json_string(sb, L, visited, err, width, level + 1);
                lua_pop(L, 1);
                sb << ":";
                if (width > 0) {
                    sb << " ";
                }
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TSTRING) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be strings";
                        return;
                    }

                    sb << ",";
                    indent(0);

                    lua_pushvalue(L, -2);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    lua_pop(L, 1);
                    sb << ":";
                    if (width > 0) {
                        sb << " ";
                    }
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                }
                indent(-1);
                sb << "}";
            } else {
                lua_pop(L, 2);  // key, value
                *err = "expected table keys to be strings or numbers";
                return;
            }

            visited->unset(ptr);
            break;
        }
        case LUA_TNIL:
            sb << "null";
            break;
        case LUA_TNUMBER:
            sb << tmp_fmt("%g", lua_tonumber(L, top));
            break;
        case LUA_TSTRING:
            sb << "\"" << luax_check_string(L, top) << "\"";
            break;
        case LUA_TBOOLEAN:
            sb << (lua_toboolean(L, top) ? "true" : "false");
            break;
        default:
            *err = "type is not serializable";
    }
}

string lua_to_json_string(lua_State *L, s32 arg, string *contents, s32 width) {
    string_builder sb = {};

    hashmap<bool> visited = {};
    neko_defer(visited.trash());

    string err = {};
    lua_pushvalue(L, arg);
    lua_to_json_string(sb, L, &visited, &err, width, 1);
    lua_pop(L, 1);

    if (err.len != 0) {
        sb.trash();
    }

    *contents = string(sb);
    return err;
}

#ifndef NEKO_PF_WIN
#include <errno.h>
#endif

#ifdef NEKO_PF_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef NEKO_PF_WIN

void mutex::make() { srwlock = {}; }
void mutex::trash() {}
void mutex::lock() { AcquireSRWLockExclusive(&srwlock); }
void mutex::unlock() { ReleaseSRWLockExclusive(&srwlock); }

bool mutex::try_lock() {
    BOOLEAN ok = TryAcquireSRWLockExclusive(&srwlock);
    return ok != 0;
}

void cond::make() { InitializeConditionVariable(&cv); }
void cond::trash() {}
void cond::signal() { WakeConditionVariable(&cv); }
void cond::broadcast() { WakeAllConditionVariable(&cv); }

void cond::wait(mutex *mtx) { SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0); }

bool cond::timed_wait(mutex *mtx, uint32_t ms) { return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0); }

void rwlock::make() { srwlock = {}; }
void rwlock::trash() {}
void rwlock::shared_lock() { AcquireSRWLockShared(&srwlock); }
void rwlock::shared_unlock() { ReleaseSRWLockShared(&srwlock); }
void rwlock::unique_lock() { AcquireSRWLockExclusive(&srwlock); }
void rwlock::unique_unlock() { ReleaseSRWLockExclusive(&srwlock); }

void sema::make(int n) { handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr); }
void sema::trash() { CloseHandle(handle); }
void sema::post(int n) { ReleaseSemaphore(handle, n, nullptr); }
void sema::wait() { WaitForSingleObjectEx(handle, INFINITE, false); }

void thread::make(ThreadProc fn, void *udata) {
    DWORD id = 0;
    HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
    ptr = (void *)handle;
}

void thread::join() {
    WaitForSingleObject((HANDLE)ptr, INFINITE);
    CloseHandle((HANDLE)ptr);
}

uint64_t this_thread_id() { return GetCurrentThreadId(); }

#else

static struct timespec ms_from_now(u32 ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    unsigned long long tally = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
    tally += ms;

    ts.tv_sec = tally / 1000LL;
    ts.tv_nsec = (tally % 1000LL) * 1000000LL;

    return ts;
}

void mutex::make() { pthread_mutex_init(&pt, nullptr); }
void mutex::trash() { pthread_mutex_destroy(&pt); }
void mutex::lock() { pthread_mutex_lock(&pt); }
void mutex::unlock() { pthread_mutex_unlock(&pt); }

bool mutex::try_lock() {
    int res = pthread_mutex_trylock(&pt);
    return res == 0;
}

void cond::make() { pthread_cond_init(&pt, nullptr); }
void cond::trash() { pthread_cond_destroy(&pt); }
void cond::signal() { pthread_cond_signal(&pt); }
void cond::broadcast() { pthread_cond_broadcast(&pt); }
void cond::wait(mutex *mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool cond::timed_wait(mutex *mtx, uint32_t ms) {
    struct timespec ts = ms_from_now(ms);
    int res = pthread_cond_timedwait(&pt, &mtx->pt, &ts);
    return res == 0;
}

void rwlock::make() { pthread_rwlock_init(&pt, nullptr); }
void rwlock::trash() { pthread_rwlock_destroy(&pt); }
void rwlock::shared_lock() { pthread_rwlock_rdlock(&pt); }
void rwlock::shared_unlock() { pthread_rwlock_unlock(&pt); }
void rwlock::unique_lock() { pthread_rwlock_wrlock(&pt); }
void rwlock::unique_unlock() { pthread_rwlock_unlock(&pt); }

void sema::make(int n) {
    sem = (sem_t *)neko_safe_malloc(sizeof(sem_t));
    sem_init(sem, 0, n);
}

void sema::trash() {
    sem_destroy(sem);
    neko_safe_free(sem);
}

void sema::post(int n) {
    for (int i = 0; i < n; i++) {
        sem_post(sem);
    }
}

void sema::wait() { sem_wait(sem); }

void thread::make(thread_proc fn, void *udata) {
    pthread_t pt = {};
    pthread_create(&pt, nullptr, (void *(*)(void *))fn, udata);
    ptr = (void *)pt;
}

void thread::join() { pthread_join((pthread_t)ptr, nullptr); }

#endif

#ifdef NEKO_PF_LINUX

uint64_t this_thread_id() {
    thread_local uint64_t s_tid = syscall(SYS_gettid);
    return s_tid;
}

#endif  // NEKO_PF_LINUX

#ifdef NEKO_PF_WEB

uint64_t this_thread_id() { return 0; }

#endif  // NEKO_PF_WEB

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // LuaAlloc *LA = luaalloc_create(nullptr, nullptr);
    // defer(luaalloc_delete(LA));

    // lua_State *L = luaL_newstate();
    // neko_defer(lua_close(L));

    lua_State *L = neko_lua_bootstrap(0, NULL);
    neko_defer(neko_lua_fini(L));

    {
        // PROFILE_BLOCK("open libs");
        // luaL_openlibs(L);
    }

    {
        // PROFILE_BLOCK("open api");
        // open_spry_api(L);
    }

    {
        // PROFILE_BLOCK("open luasocket");
        // open_luasocket(L);
    }

    {
        // PROFILE_BLOCK("run bootstrap");
        // luax_run_bootstrap(L);
    }

    string contents = lt->contents;

    {
        // PROFILE_BLOCK("load chunk");
        if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) != LUA_OK) {
            string err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);

            neko_safe_free(contents.data);
            neko_safe_free(lt->name.data);
            return;
        }
    }

    neko_safe_free(contents.data);
    neko_safe_free(lt->name.data);

    {
        // PROFILE_BLOCK("run chunk");
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            string err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);
        }
    }
}

void LuaThread::make(string code, string thread_name) {
    mtx.make();
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    lock_guard lock{&mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (lock_guard lock{&mtx}) {
        thread.join();
    }

    mtx.trash();
}

//

void lua_variant::make(lua_State *L, s32 arg) {
    type = lua_type(L, arg);

    switch (type) {
        case LUA_TBOOLEAN:
            boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            number = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            neko::string s = luax_check_string(L, arg);
            string = to_cstr(s);
            break;
        }
        case LUA_TTABLE: {
            array<lua_table_entry> entries = {};
            entries.resize(luax_len(L, arg));

            lua_pushvalue(L, arg);
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                lua_variant key = {};
                key.make(L, -2);

                lua_variant value = {};
                value.make(L, -1);

                entries.push({key, value});
            }
            lua_pop(L, 1);

            table = slice(entries);
            break;
        }
        case LUA_TUSERDATA: {
            s32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TSTRING) {
                return;
            }

            kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TNUMBER) {
                return;
            }

            neko::string tname = luax_check_string(L, -2);
            u64 size = luaL_checkinteger(L, -1);

            if (size != sizeof(void *)) {
                return;
            }

            udata.ptr = *(void **)lua_touserdata(L, arg);
            udata.tname = to_cstr(tname);

            break;
        }
        default:
            break;
    }
}

void lua_variant::trash() {
    switch (type) {
        case LUA_TSTRING: {
            neko_safe_free(string.data);
            break;
        }
        case LUA_TTABLE: {
            for (lua_table_entry e : table) {
                e.key.trash();
                e.value.trash();
            }
            neko_safe_free(table.data);
        }
        case LUA_TUSERDATA: {
            neko_safe_free(udata.tname.data);
        }
        default:
            break;
    }
}

void lua_variant::push(lua_State *L) {
    switch (type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, boolean);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, number);
            break;
        case LUA_TSTRING:
            lua_pushlstring(L, string.data, string.len);
            break;
        case LUA_TTABLE: {
            lua_newtable(L);
            for (lua_table_entry e : table) {
                e.key.push(L);
                e.value.push(L);
                lua_rawset(L, -3);
            }
            break;
        }
        case LUA_TUSERDATA: {
            luax_ptr_userdata(L, udata.ptr, udata.tname.data);
            break;
        }
        default:
            break;
    }
}

//

struct LuaChannels {
    neko::mutex mtx;
    neko::cond select;
    neko::hashmap<lua_channel *> by_name;
};

static LuaChannels g_channels = {};

void lua_channel::make(string n, u64 buf) {
    mtx.make();
    sent.make();
    received.make();
    items.data = (lua_variant *)neko_safe_malloc(sizeof(lua_variant) * (buf + 1));
    items.len = (buf + 1);
    front = 0;
    back = 0;
    len = 0;

    name.store(to_cstr(n).data);
}

void lua_channel::trash() {
    for (s32 i = 0; i < len; i++) {
        items[front].trash();
        front = (front + 1) % items.len;
    }

    neko_safe_free(items.data);
    neko_safe_free(name.exchange(nullptr));
    mtx.trash();
    sent.trash();
    received.trash();
}

void lua_channel::send(lua_variant item) {
    lock_guard lock{&mtx};

    while (len == items.len) {
        received.wait(&mtx);
    }

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.broadcast();
    sent.signal();
    sent_total++;

    while (sent_total >= received_total + items.len) {
        received.wait(&mtx);
    }
}

static lua_variant lua_channel_dequeue(lua_channel *ch) {
    lua_variant item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.broadcast();
    ch->received_total++;

    return item;
}

lua_variant lua_channel::recv() {
    lock_guard lock{&mtx};

    while (len == 0) {
        sent.wait(&mtx);
    }

    return lua_channel_dequeue(this);
}

bool lua_channel::try_recv(lua_variant *v) {
    lock_guard lock{&mtx};

    if (len == 0) {
        return false;
    }

    *v = lua_channel_dequeue(this);
    return true;
}

lua_channel *lua_channel_make(string name, u64 buf) {
    lua_channel *chan = (lua_channel *)neko_safe_malloc(sizeof(lua_channel));
    new (&chan->name) std::atomic<char *>();
    chan->make(name, buf);

    lock_guard lock{&g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

lua_channel *lua_channel_get(string name) {
    lock_guard lock{&g_channels.mtx};

    lua_channel **chan = g_channels.by_name.get(fnv1a(name));
    if (chan == nullptr) {
        return nullptr;
    }

    return *chan;
}

lua_channel *lua_channels_select(lua_State *L, lua_variant *v) {
    s32 len = lua_gettop(L);
    if (len == 0) {
        return nullptr;
    }

    lua_channel *buf[16] = {};
    for (s32 i = 0; i < len; i++) {
        buf[i] = *(lua_channel **)luaL_checkudata(L, i + 1, "mt_channel");
    }

    mutex mtx = {};
    mtx.make();
    lock_guard lock{&mtx};

    while (true) {
        for (s32 i = 0; i < len; i++) {
            lock_guard lock{&buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(&mtx);
    }
}

void lua_channels_setup() {
    g_channels.select.make();
    g_channels.mtx.make();
}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        lua_channel *chan = *v;
        chan->trash();
        neko_safe_free(chan);
    }
    g_channels.by_name.trash();
    g_channels.select.trash();
    g_channels.mtx.trash();
}

s32 os_change_dir(const char *path) { return chdir(path); }

string os_program_dir() {
    string str = os_program_path();
    char *buf = str.data;

    for (s32 i = (s32)str.len; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = 0;
            return {str.data, (u64)i + 1};
        }
    }

    return str;
}

#ifdef NEKO_PF_WIN

string os_program_path() {
    static char s_buf[2048];

    DWORD len = GetModuleFileNameA(NULL, s_buf, array_size(s_buf));

    for (s32 i = 0; s_buf[i]; i++) {
        if (s_buf[i] == '\\') {
            s_buf[i] = '/';
        }
    }

    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    defer(CloseHandle(handle));

    FILETIME create = {};
    FILETIME access = {};
    FILETIME write = {};
    bool ok = GetFileTime(handle, &create, &access, &write);
    if (!ok) {
        return 0;
    }

    ULARGE_INTEGER time = {};
    time.LowPart = write.dwLowDateTime;
    time.HighPart = write.dwHighDateTime;

    return time.QuadPart;
}

void os_high_timer_resolution() { timeBeginPeriod(8); }
void os_sleep(u32 ms) { Sleep(ms); }
void os_yield() { YieldProcessor(); }

#endif  // NEKO_PF_WIN

#ifdef NEKO_PF_LINUX

string os_program_path() {
    static char s_buf[2048];
    s32 len = (s32)readlink("/proc/self/exe", s_buf, NEKO_ARR_SIZE(s_buf));
    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    struct stat attrib = {};
    s32 err = stat(filename, &attrib);
    if (err == 0) {
        return (u64)attrib.st_mtime;
    } else {
        return 0;
    }
}

void os_high_timer_resolution() {}

void os_sleep(u32 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, &ts);
}

void os_yield() { sched_yield(); }

#endif  // NEKO_PF_LINUX

#ifdef NEKO_PF_WEB

string os_program_path() { return {}; }
u64 os_file_modtime(const char *filename) { return 0; }
void os_high_timer_resolution() {}
void os_sleep(u32 ms) {}
void os_yield() {}

#endif  // NEKO_PF_WEB

struct Profile {
    queue<TraceEvent> events;
    thread recv_thread;
};

static Profile g_profile = {};

static void profile_recv_thread(void *) {
    string_builder sb = {};
    sb.swap_filename(os_program_path(), "profile.json");

    FILE *f = fopen(sb.data, "w");
    sb.trash();

    neko_defer(fclose(f));

    fputs("[", f);
    while (true) {
        TraceEvent e = g_profile.events.demand();
        if (e.name == nullptr) {
            fputs("]", f);
            return;
        }

        fprintf(f,
                R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%hu},)"
                "\n",
                e.name, e.cat, e.ph, e.ts / 1000.f, e.tid);
    }
}

void profile_setup() {
    g_profile.events.make();
    g_profile.events.reserve(256);
    g_profile.recv_thread.make(profile_recv_thread, nullptr);
}

void profile_shutdown() {
    g_profile.events.enqueue({});
    g_profile.recv_thread.join();
    g_profile.events.trash();
}

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
typedef struct {
    uint32_t initialized;
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
} neko_tm_state_t;
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
typedef struct {
    uint32_t initialized;
    mach_timebase_info_data_t timebase;
    uint64_t start;
} neko_tm_state_t;
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
typedef struct {
    uint32_t initialized;
    double start;
} neko_tm_state_t;
#else  // linux
#include <time.h>
typedef struct {
    uint32_t initialized;
    uint64_t start;
} neko_tm_state_t;
#endif
static neko_tm_state_t g_tm;

NEKO_API_DECL void neko_tm_init(void) {
    memset(&g_tm, 0, sizeof(g_tm));
    g_tm.initialized = 0xABCDEF01;
#if defined(_WIN32)
    QueryPerformanceFrequency(&_stm.freq);
    QueryPerformanceCounter(&_stm.start);
#elif defined(__APPLE__) && defined(__MACH__)
    mach_timebase_info(&_stm.timebase);
    _stm.start = mach_absolute_time();
#elif defined(__EMSCRIPTEN__)
    _stm.start = emscripten_get_now();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_tm.start = (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
#endif
}

NEKO_API_DECL uint64_t neko_tm_now(void) {
    NEKO_ASSERT(g_tm.initialized == 0xABCDEF01);
    uint64_t now;
#if defined(_WIN32)
    LARGE_INTEGER qpc_t;
    QueryPerformanceCounter(&qpc_t);
    now = (uint64_t)_stm_int64_muldiv(qpc_t.QuadPart - _stm.start.QuadPart, 1000000000, _stm.freq.QuadPart);
#elif defined(__APPLE__) && defined(__MACH__)
    const uint64_t mach_now = mach_absolute_time() - _stm.start;
    now = (uint64_t)_stm_int64_muldiv((int64_t)mach_now, (int64_t)_stm.timebase.numer, (int64_t)_stm.timebase.denom);
#elif defined(__EMSCRIPTEN__)
    double js_now = emscripten_get_now() - _stm.start;
    now = (uint64_t)(js_now * 1000000.0);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    now = ((uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec) - g_tm.start;
#endif
    return now;
}

Instrument::Instrument(const char *cat, const char *name) : cat(cat), name(name), tid(this_thread_id()) {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'B';
    e.ts = neko_tm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

Instrument::~Instrument() {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'E';
    e.ts = neko_tm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

}  // namespace neko

neko_struct(neko_client_cvar_t,                         //
            _Fs(show_editor, "Is show editor"),         //
            _Fs(show_demo_window, "Is show nui demo"),  //
            _Fs(show_pack_editor, "pack editor"),       //
            _Fs(show_profiler_window, "profiler"),      //
            _Fs(show_gui, "neko gui"),                  //
            _Fs(shader_inspect, "shaders"),             //
            _Fs(hello_ai_shit, "Test AI"),              //
            _Fs(bg, "bg color")                         //
);

template <typename T, typename Fields = std::tuple<>>
void __neko_cvar_gui_internal(T &&obj, int depth = 0, const char *fieldName = "", Fields &&fields = std::make_tuple()) {
    if constexpr (std::is_class_v<std::decay_t<T>>) {
        neko::reflection::struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { __neko_cvar_gui_internal(value, depth + 1, fieldName, info); });
    } else {

        auto ff = [&]<typename S>(const char *name, auto &var, S &t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, S>) {
                neko::imgui::Auto(var, name);
                ImGui::Text("    [%s]", std::get<0>(fields));
            }
        };

        std::apply([&](auto &&...args) { (ff(fieldName, obj, args), ...); }, std::tuple<CVAR_TYPES()>());
    }
}

void neko_cvar_gui(neko_client_cvar_t &cvar) {
    __neko_cvar_gui_internal(cvar);

    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        {
            switch ((&neko_cv()->cvars[i])->type) {
                default:
                case __NEKO_CONFIG_TYPE_STRING:
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.s, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.f, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_INT:
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.i, (&neko_cv()->cvars[i])->name);
                    break;
            };
        };
    }
}
