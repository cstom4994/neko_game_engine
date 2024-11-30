#include "base/common/os.hpp"

#include "base/common/util.hpp"
#include "base/common/array.hpp"

#if defined(NEKO_IS_WIN32)
#include <direct.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <shellapi.h>

#elif defined(NEKO_IS_WEB)
#include <errno.h>
#include <unistd.h>

#elif defined(NEKO_IS_LINUX) || defined(NEKO_IS_APPLE)
#include <dlfcn.h>
#include <errno.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#endif

/*========================
// os
========================*/

namespace Neko {

Array<String> BaseGetCommandLine(int argc) {

    // 转储CommandLine到argsArray
    Array<String> argsArray{};
    wchar_t **argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv_w == nullptr) {
        console_log("Failed to parse command line arguments");
        return {};
    }
    for (int i = 0; i < argc; i++) argsArray.push(to_cstr(win::w2u(argv_w[i])));
    LocalFree(argv_w);  // 释放 CommandLineToArgvW 分配的内存

    return argsArray;
}

#ifdef NEKO_IS_WIN32

uint64_t this_thread_id() { return GetCurrentThreadId(); }

#elif defined(NEKO_IS_LINUX) || defined(NEKO_IS_APPLE)

uint64_t this_thread_id() {
    thread_local uint64_t s_tid = syscall(SYS_gettid);
    return s_tid;
}

#endif  // NEKO_IS_LINUX

#ifdef NEKO_IS_WEB

uint64_t this_thread_id() { return 0; }

#endif  // NEKO_IS_WEB

String os_program_dir() {
    String str = os_program_path();
    char *buf = str.data;

    for (i32 i = (i32)str.len; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = 0;
            return {str.data, (u64)i + 1};
        }
    }

    return str;
}

#ifdef NEKO_IS_WIN32

String os_program_path() {
    static char s_buf[2048];

    DWORD len = GetModuleFileNameA(NULL, s_buf, array_size(s_buf));

    for (i32 i = 0; s_buf[i]; i++) {
        if (s_buf[i] == '\\') {
            s_buf[i] = '/';
        }
    }

    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    HANDLE handle = CreateFileW(win::u2w(filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        DWORD error_code = GetLastError();
        return 0;
    }
    neko_defer(CloseHandle(handle));

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

#endif  // NEKO_IS_WIN32

#if defined(NEKO_IS_LINUX) || defined(NEKO_IS_APPLE)

String os_program_path() {
    static char s_buf[2048];
    i32 len = (i32)readlink("/proc/self/exe", s_buf, array_size(s_buf));
    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char *filename) {
    struct stat attrib = {};
    i32 err = stat(filename, &attrib);
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

#endif  // NEKO_IS_LINUX

#ifdef NEKO_IS_WEB

String os_program_path() { return {}; }
u64 os_file_modtime(const char *filename) { return 0; }
void os_high_timer_resolution() {}
void os_sleep(u32 ms) {}
void os_yield() {}

#endif  // NEKO_IS_WEB

// Platform File IO
char *neko_os_read_file_contents(const char *file_path, const char *mode, size_t *sz) {

#ifdef NEKO_IS_ANDROID
    const char *internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    char *buffer = 0;
    FILE *fp = neko_fopen(file_path, mode);
    size_t read_sz = 0;
    if (fp) {
        read_sz = neko_os_file_size_in_bytes(file_path);
        buffer = (char *)mem_alloc(read_sz + 1);
        if (buffer) {
            size_t _r = neko_fread(buffer, 1, read_sz, fp);
        }
        buffer[read_sz] = '\0';
        neko_fclose(fp);
        if (sz) *sz = read_sz;
    }

    // NEKO_WINDOWS_ConvertPath_end(path);

    return buffer;
}

bool neko_os_write_file_contents(const char *file_path, const char *mode, void *data, size_t sz) {
    const char *path = file_path;

#ifdef NEKO_IS_ANDROID
    const char *internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE *fp = neko_fopen(file_path, mode);
    if (fp) {
        size_t ret = fwrite(data, sizeof(u8), sz, fp);
        if (ret == sz) {
            neko_fclose(fp);
            return true;
        }
        neko_fclose(fp);
    }
    return false;
}

bool neko_os_dir_exists(const char *dir_path) {
#if defined(NEKO_IS_WIN32)
    DWORD attrib = GetFileAttributesW(win::u2w(dir_path).c_str());  // TODO: unicode 路径修复
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(NEKO_IS_LINUX) || defined(NEKO_IS_APPLE)
    struct stat st;
    if (stat(dir_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;
        }
    }
    return false;
#endif
}

i32 neko_os_mkdir(const char *dir_path, i32 opt) {
#ifdef NEKO_IS_WIN32
    return _mkdir(dir_path);
#else
    return mkdir(dir_path, opt);
#endif
}

bool neko_os_file_exists(const char *file_path) {
    const char *path = file_path;

#ifdef NEKO_IS_ANDROID
    const char *internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE *fp = neko_fopen(file_path, "r");
    if (fp) {
        neko_fclose(fp);
        return true;
    }
    return false;
}

static u32 util_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

i32 neko_os_file_size_in_bytes(const char *file_path) {
#ifdef NEKO_IS_WIN32

    HANDLE hFile = CreateFileW(win::u2w(file_path).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;  // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;  // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return util_safe_truncate_u64(size.QuadPart);

#elif (defined NEKO_IS_ANDROID)

    const char *internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    struct stat st;
    stat(tmp_path, &st);
    return (i32)st.st_size;

#else

    struct stat st;
    stat(file_path, &st);
    return (i32)st.st_size;

#endif
}

static void util_get_file_extension(char *buffer, size_t buffer_size, const_str file_path) {
    neko_assert(buffer && buffer_size);
    const_str extension = strrchr(file_path, '.');
    if (extension) {
        size_t extension_len = strlen(extension + 1);
        size_t len = (extension_len >= buffer_size) ? buffer_size - 1 : extension_len;
        memcpy(buffer, extension + 1, len);
        buffer[len] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

void neko_os_file_extension(char *buffer, size_t buffer_sz, const char *file_path) { util_get_file_extension(buffer, buffer_sz, file_path); }

i32 neko_os_file_delete(const char *file_path) {
#if (defined NEKO_IS_WIN32)

    // Non-zero if successful
    return DeleteFileW(win::u2w(file_path).c_str());

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)

    // Returns 0 if successful
    return !remove(file_path);

#endif

    return 0;
}

i32 neko_os_file_copy(const char *src_path, const char *dst_path) {
#if (defined NEKO_IS_WIN32)

    return CopyFileW(win::u2w(src_path).c_str(), win::u2w(dst_path).c_str(), false);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)

    FILE *file_w = NULL;
    FILE *file_r = NULL;
    char buffer[2048] = NEKO_DEFAULT_VAL();

    if ((file_w = neko_fopen(src_path, "wb")) == NULL) {
        return 0;
    }
    if ((file_r = neko_fopen(dst_path, "rb")) == NULL) {
        return 0;
    }

    // Read file in 2kb chunks to write to location
    i32 len = 0;
    while ((len = neko_fread(buffer, sizeof(buffer), 1, file_r)) > 0) {
        fwrite(buffer, len, 1, file_w);
    }

    // Close both files
    neko_fclose(file_r);
    neko_fclose(file_w);

#endif

    return 0;
}

i32 neko_os_file_compare_time(u64 time_a, u64 time_b) { return time_a < time_b ? -1 : time_a == time_b ? 0 : 1; }

neko_os_file_stats_t neko_os_file_stats(const char *file_path) {
    neko_os_file_stats_t stats = NEKO_DEFAULT_VAL();

#if (defined NEKO_IS_WIN32)

    WIN32_FILE_ATTRIBUTE_DATA data = NEKO_DEFAULT_VAL();
    FILETIME ftime = NEKO_DEFAULT_VAL();
    FILETIME ctime = NEKO_DEFAULT_VAL();
    FILETIME atime = NEKO_DEFAULT_VAL();
    if (GetFileAttributesExW(win::u2w(file_path).c_str(), GetFileExInfoStandard, &data)) {
        ftime = data.ftLastWriteTime;
        ctime = data.ftCreationTime;
        atime = data.ftLastAccessTime;
    }

    stats.modified_time = *((u64 *)&ftime);
    stats.access_time = *((u64 *)&atime);
    stats.creation_time = *((u64 *)&ctime);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    struct stat attr = NEKO_DEFAULT_VAL();
    stat(file_path, &attr);
    stats.modified_time = *((u64 *)&attr.st_mtime);

#endif

    return stats;
}

void *neko_os_library_load(const char *lib_path) {
#if (defined NEKO_IS_WIN32)
    return (void *)LoadLibraryW(win::u2w(lib_path).c_str());
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    return (void *)dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);  // RTLD_LAZY
#endif
    return NULL;
}

void neko_os_library_unload(void *lib) {
    if (!lib) return;
#if (defined NEKO_IS_WIN32)
    FreeLibrary((HMODULE)lib);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    dlclose(lib);
#endif
}

void *neko_os_library_proc_address(void *lib, const char *func) {
    if (!lib) return NULL;
#if (defined NEKO_IS_WIN32)
    return (void *)GetProcAddress((HMODULE)lib, func);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    return (void *)dlsym(lib, func);
#endif
    return NULL;
}

int neko_os_chdir(const char *path) {
#if (defined NEKO_IS_WIN32)
    return _chdir(path);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    return chdir(path);
#endif
    return 1;
}

String neko_os_homedir() {
#ifdef _WIN32
#define HOMEDIR "USERPROFILE"
#else
#define HOMEDIR (char *)"HOME"
#endif
    const_str path = std::getenv(HOMEDIR);
    return {path};
}

#if (defined(_WIN32) || defined(_WIN64))
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) win_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) win_def
#elif defined(__APPLE__)
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) mac_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) other_def
#else
#define NEKO_DLL_LOADER_WIN_MAC_OTHER(win_def, mac_def, other_def) other_def
#define NEKO_DLL_LOADER_WIN_OTHER(win_def, other_def) other_def
#endif

neko_dynlib neko_dylib_open(const_str name) {
    neko_dynlib module;
    char filename[64] = {};
    const_str prefix = NEKO_DLL_LOADER_WIN_OTHER("", "lib");
    const_str suffix = NEKO_DLL_LOADER_WIN_MAC_OTHER(".dll", ".dylib", ".so");
    neko_snprintf(filename, 64, "%s%s%s", prefix, name, suffix);
    module.hndl = (void *)neko_os_library_load(filename);
    return module;
}

void neko_dylib_close(neko_dynlib lib) { neko_os_library_unload(lib.hndl); }

void *neko_dylib_get_symbol(neko_dynlib lib, const_str symbol_name) {
    void *symbol = (void *)neko_os_library_proc_address(lib.hndl, symbol_name);
    return symbol;
}

bool neko_dylib_has_symbol(neko_dynlib lib, const_str symbol_name) {
    if (!lib.hndl || !symbol_name) return false;
    return neko_os_library_proc_address(lib.hndl, symbol_name) != NULL;
}

#if 0
static std::string get_error_description() noexcept {
#if (defined(_WIN32) || defined(_WIN64))
    constexpr const size_t BUF_SIZE = 512;
    const auto error_code = GetLastError();
    if (!error_code) return "No error reported by GetLastError";
    char description[BUF_SIZE];
    const auto lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    const DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, lang, description, BUF_SIZE, nullptr);
    return (length == 0) ? "Unknown error (FormatMessage failed)" : description;
#else
    const auto description = dlerror();
    return (description == nullptr) ? "No error reported by dlerror" : description;
#endif
}
#endif

}  // namespace Neko

#if defined(NEKO_IS_WIN32)

namespace Neko::win {
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

// 遍历指定模块的导出表
BOOL EnumerateExports(HMODULE hModule, BOOL (*Callback)(LPCSTR pszFuncName, PVOID pFuncAddr)) {
    if (!hModule) return FALSE;

    BYTE *baseAddress = reinterpret_cast<BYTE *>(hModule);  // 模块基址

    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(baseAddress);  // 模块 DOS 头
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;

    auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS *>(baseAddress + dosHeader->e_lfanew);  // NT 头
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return FALSE;

    auto exportDirRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;  // 导出表地址
    if (!exportDirRVA) return FALSE;

    // 获取导出表数据
    auto exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY *>(baseAddress + exportDirRVA);
    auto nameRVAs = reinterpret_cast<DWORD *>(baseAddress + exportDir->AddressOfNames);
    auto funcRVAs = reinterpret_cast<DWORD *>(baseAddress + exportDir->AddressOfFunctions);
    auto nameOrdinals = reinterpret_cast<WORD *>(baseAddress + exportDir->AddressOfNameOrdinals);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i) {
        auto funcName = reinterpret_cast<LPCSTR>(baseAddress + nameRVAs[i]);
        auto ordinal = nameOrdinals[i];
        auto funcAddr = reinterpret_cast<PVOID>(baseAddress + funcRVAs[ordinal]);
        if (!Callback(funcName, funcAddr)) return TRUE;
    }
    return TRUE;
}

}  // namespace Neko::win

#endif
