#pragma once

#include "base/common/base.hpp"
#include "base/common/mutex.hpp"
#include "base/common/string.hpp"
#include "base/common/array.hpp"

#if __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_debugging)
#include <debugging>
#else
#if defined(_WIN32)
#include <intrin.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#endif
#endif

#if defined(__clang__)
#define NEKO_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
#define NEKO_DEBUGBREAK() __builtin_trap()
#elif defined(_MSC_VER)
#define NEKO_DEBUGBREAK() __debugbreak()
#else
#define NEKO_DEBUGBREAK()
#endif

namespace Neko {

Array<String> BaseGetCommandLine(int argc);

String os_program_dir();
String os_program_path();
u64 os_file_modtime(const char *filename);
void os_high_timer_resolution();
void os_sleep(u32 ms);
void os_yield();
String neko_os_homedir();

typedef struct neko_os_file_stats_t {
    u64 modified_time;
    u64 creation_time;
    u64 access_time;
} neko_os_file_stats_t;

// Platform File IO (this all needs to be made available for impl rewrites)
char *neko_os_read_file_contents(const char *file_path, const char *mode, size_t *sz);
bool neko_os_write_file_contents(const char *file_path, const char *mode, void *data, size_t data_size);
bool neko_os_file_exists(const char *file_path);
bool neko_os_dir_exists(const char *dir_path);
i32 neko_os_mkdir(const char *dir_path, i32 opt);
i32 neko_os_file_size_in_bytes(const char *file_path);
void neko_os_file_extension(char *buffer, size_t buffer_sz, const char *file_path);
i32 neko_os_file_delete(const char *file_path);
i32 neko_os_file_copy(const char *src_path, const char *dst_path);
i32 neko_os_file_compare_time(u64 time_a, u64 time_b);
neko_os_file_stats_t neko_os_file_stats(const char *file_path);
void *neko_os_library_load(const char *lib_path);
void neko_os_library_unload(void *lib);
void *neko_os_library_proc_address(void *lib, const char *func);
int neko_os_chdir(const char *path);

#if defined(NEKO_IS_APPLE)
#define neko_fopen(filePath, mode) fopen(filePath, mode)
#define neko_fseek(file, offset, whence) fseeko(file, offset, whence)
#define neko_ftell(file) ftello(file)
#elif defined(NEKO_IS_LINUX)
#define neko_fopen(filePath, mode) fopen(filePath, mode)
#define neko_fseek(file, offset, whence) fseek(file, offset, whence)
#define neko_ftell(file) ftell(file)
#elif defined(NEKO_IS_WIN32)
static inline FILE *neko_fopen(const char *filePath, const char *mode) {
    FILE *file;
    errno_t error = fopen_s(&file, filePath, mode);
    if (error != 0) return NULL;
    return file;
}
#define neko_fseek(file, offset, whence) _fseeki64(file, offset, whence)
#define neko_ftell(file) _ftelli64(file)
#elif defined(NEKO_IS_WEB)

#else
#error Unsupported operating system
#endif
#define neko_fwrite(buffer, size, count, file) fwrite(buffer, size, count, file)
#define neko_fread(buffer, size, count, file) fread(buffer, size, count, file)
#define neko_fclose(file) fclose(file)

#if defined MAX_PATH
#define DIR_MAX MAX_PATH
#elif defined PATH_MAX
#define DIR_MAX PATH_MAX
#else
#define DIR_MAX 260
#endif

typedef struct neko_dynlib {
    void *hndl;
} neko_dynlib;

neko_dynlib neko_dylib_open(const_str name);
void neko_dylib_close(neko_dynlib lib);
void *neko_dylib_get_symbol(neko_dynlib lib, const_str symbol_name);
bool neko_dylib_has_symbol(neko_dynlib lib, const_str symbol_name);

}  // namespace Neko

namespace Neko::win {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
std::wstring a2w(std::string_view str) noexcept;
std::string w2a(std::wstring_view wstr) noexcept;
std::string a2u(std::string_view str) noexcept;
std::string u2a(std::string_view str) noexcept;

BOOL EnumerateExports(HMODULE hModule, BOOL (*Callback)(LPCSTR pszFuncName, PVOID pFuncAddr));

}  // namespace Neko::win