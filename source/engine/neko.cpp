#include "neko.hpp"

#include <cstdio>
#include <filesystem>
#include <new>

#include "engine/neko_imgui.hpp"
#include "engine/neko_lua.hpp"
#include "engine/neko_engine.h"
#include "sandbox/hpp/neko_struct.hpp"

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

        s32 res = neko_platform_chdir(path.data);
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
//     DWORD len = GetModuleFileNameA(NULL, s_buf, NEKO_ARR_SIZE(s_buf));
//     for (s32 i = 0; s_buf[i]; i++) {
//         if (s_buf[i] == '\\') {
//             s_buf[i] = '/';
//         }
//     }
//     return {s_buf, (u64)len};
// }

mount_result vfs_mount(const char *filepath) {

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
        neko_struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { __neko_cvar_gui_internal(value, depth + 1, fieldName, info); });
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
