#include "engine/os.h"

#include <array>
#include <new>

#include "engine/api.hpp"
#include "engine/base.h"
#include "engine/game.h"
#include "engine/glew_glfw.h"
#include "engine/luax.h"
#include "engine/os.h"
#include "engine/prelude.h"
#include "engine/script.h"
#include "vendor/luaalloc.h"

#if defined(NEKO_IS_WIN32)
#include <direct.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#elif defined(NEKO_IS_WEB)
#include <errno.h>
#include <unistd.h>

#elif defined(NEKO_IS_LINUX) || defined(NEKO_IS_APPLE)
#include <errno.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#endif

#ifdef NEKO_IS_WIN32

void Mutex::make() { srwlock = {}; }
void Mutex::trash() {}
void Mutex::lock() { AcquireSRWLockExclusive(&srwlock); }
void Mutex::unlock() { ReleaseSRWLockExclusive(&srwlock); }

bool Mutex::try_lock() {
    BOOLEAN ok = TryAcquireSRWLockExclusive(&srwlock);
    return ok != 0;
}

void Cond::make() { InitializeConditionVariable(&cv); }
void Cond::trash() {}
void Cond::signal() { WakeConditionVariable(&cv); }
void Cond::broadcast() { WakeAllConditionVariable(&cv); }

void Cond::wait(Mutex* mtx) { SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0); }

bool Cond::timed_wait(Mutex* mtx, uint32_t ms) { return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0); }

void RWLock::make() { srwlock = {}; }
void RWLock::trash() {}
void RWLock::shared_lock() { AcquireSRWLockShared(&srwlock); }
void RWLock::shared_unlock() { ReleaseSRWLockShared(&srwlock); }
void RWLock::unique_lock() { AcquireSRWLockExclusive(&srwlock); }
void RWLock::unique_unlock() { ReleaseSRWLockExclusive(&srwlock); }

void Sema::make(int n) { handle = CreateSemaphoreA(nullptr, n, LONG_MAX, nullptr); }
void Sema::trash() { CloseHandle(handle); }
void Sema::post(int n) { ReleaseSemaphore(handle, n, nullptr); }
void Sema::wait() { WaitForSingleObjectEx(handle, INFINITE, false); }

void Thread::make(ThreadProc fn, void* udata) {
    DWORD id = 0;
    HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
    ptr = (void*)handle;
}

void Thread::join() {
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

void Mutex::make() { pthread_mutex_init(&pt, nullptr); }
void Mutex::trash() { pthread_mutex_destroy(&pt); }
void Mutex::lock() { pthread_mutex_lock(&pt); }
void Mutex::unlock() { pthread_mutex_unlock(&pt); }

bool Mutex::try_lock() {
    int res = pthread_mutex_trylock(&pt);
    return res == 0;
}

void Cond::make() { pthread_cond_init(&pt, nullptr); }
void Cond::trash() { pthread_cond_destroy(&pt); }
void Cond::signal() { pthread_cond_signal(&pt); }
void Cond::broadcast() { pthread_cond_broadcast(&pt); }
void Cond::wait(Mutex* mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool Cond::timed_wait(Mutex* mtx, uint32_t ms) {
    struct timespec ts = ms_from_now(ms);
    int res = pthread_cond_timedwait(&pt, &mtx->pt, &ts);
    return res == 0;
}

void RWLock::make() { pthread_rwlock_init(&pt, nullptr); }
void RWLock::trash() { pthread_rwlock_destroy(&pt); }
void RWLock::shared_lock() { pthread_rwlock_rdlock(&pt); }
void RWLock::shared_unlock() { pthread_rwlock_unlock(&pt); }
void RWLock::unique_lock() { pthread_rwlock_wrlock(&pt); }
void RWLock::unique_unlock() { pthread_rwlock_unlock(&pt); }

void Sema::make(int n) {
    sem = (sem_t*)mem_alloc(sizeof(sem_t));
    sem_init(sem, 0, n);
}

void Sema::trash() {
    sem_destroy(sem);
    mem_free(sem);
}

void Sema::post(int n) {
    for (int i = 0; i < n; i++) {
        sem_post(sem);
    }
}

void Sema::wait() { sem_wait(sem); }

void Thread::make(ThreadProc fn, void* udata) {
    pthread_t pt = {};
    pthread_create(&pt, nullptr, (void* (*)(void*))fn, udata);
    ptr = (void*)pt;
}

void Thread::join() { pthread_join((pthread_t)ptr, nullptr); }

#endif

#ifdef NEKO_IS_LINUX

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
    char* buf = str.data;

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

u64 os_file_modtime(const char* filename) {
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
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

#ifdef NEKO_IS_LINUX

String os_program_path() {
    static char s_buf[2048];
    i32 len = (i32)readlink("/proc/self/exe", s_buf, array_size(s_buf));
    return {s_buf, (u64)len};
}

u64 os_file_modtime(const char* filename) {
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
u64 os_file_modtime(const char* filename) { return 0; }
void os_high_timer_resolution() {}
void os_sleep(u32 ms) {}
void os_yield() {}

#endif  // NEKO_IS_WEB

void* DebugAllocator::alloc(size_t bytes, const char* file, i32 line) {
    LockGuard lock{&mtx};

    DebugAllocInfo* info = (DebugAllocInfo*)::malloc(offsetof(DebugAllocInfo, buf[bytes]));
    NEKO_ASSERT(info, "FAILED_TO_ALLOCATE");
    info->file = file;
    info->line = line;
    info->size = bytes;
    info->prev = nullptr;
    info->next = head;
    if (head != nullptr) {
        head->prev = info;
    }
    head = info;

    alloc_size += info->size;

    return info->buf;
}

void* DebugAllocator::realloc(void* ptr, size_t new_size, const char* file, i32 line) {
    if (ptr == nullptr) {
        // 如果指针为空 则分配新内存
        return this->alloc(new_size, file, line);
    }

    if (new_size == 0) {
        // 如果新大小为零 则释放内存并返回 null
        this->free(ptr);
        return nullptr;
    }

    LockGuard lock{&mtx};

    DebugAllocInfo* old_info = (DebugAllocInfo*)((u8*)ptr - offsetof(DebugAllocInfo, buf));

    alloc_size -= old_info->size;

    // 分配新大小的新内存块
    DebugAllocInfo* new_info = (DebugAllocInfo*)::malloc(NEKO_OFFSET(DebugAllocInfo, buf[new_size]));
    NEKO_ASSERT(new_info, "FAILED_TO_ALLOCATE");
    if (new_info == nullptr) {
        return nullptr;  // 分配失败
    }

    // 将数据从旧内存块复制到新内存块
    size_t copy_size = old_info->size < new_size ? old_info->size : new_size;
    memcpy(new_info->buf, old_info->buf, copy_size);

    // 更新新的内存块信息
    new_info->file = file;
    new_info->line = line;
    new_info->size = new_size;
    new_info->prev = old_info->prev;
    new_info->next = old_info->next;
    if (new_info->prev != nullptr) {
        new_info->prev->next = new_info;
    } else {
        head = new_info;
    }
    if (new_info->next != nullptr) {
        new_info->next->prev = new_info;
    }

    alloc_size += new_size;

    // 释放旧内存块
    ::free(old_info);

    return new_info->buf;
}

void DebugAllocator::free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }

    LockGuard lock{&mtx};

    DebugAllocInfo* info = (DebugAllocInfo*)((u8*)ptr - offsetof(DebugAllocInfo, buf));

    alloc_size -= info->size;

    if (info->prev == nullptr) {
        head = info->next;
    } else {
        info->prev->next = info->next;
    }

    if (info->next) {
        info->next->prev = info->prev;
    }

    ::free(info);
}

void DebugAllocator::dump_allocs() {
    i32 allocs = 0;
    for (DebugAllocInfo* info = head; info != nullptr; info = info->next) {
        printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size, info->file, info->line);
        allocs++;
    }
    neko_println("  --- %d allocation(s) with %lld bytes ---", allocs, alloc_size);
}

// Platform File IO
char* neko_os_read_file_contents(const char* file_path, const char* mode, size_t* sz) {

#ifdef NEKO_IS_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    char* buffer = 0;
    FILE* fp = neko_fopen(file_path, mode);
    size_t read_sz = 0;
    if (fp) {
        read_sz = neko_os_file_size_in_bytes(file_path);
        buffer = (char*)mem_alloc(read_sz + 1);
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

bool neko_os_write_file_contents(const char* file_path, const char* mode, void* data, size_t sz) {
    const char* path = file_path;

#ifdef NEKO_IS_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE* fp = neko_fopen(file_path, mode);
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

bool neko_os_dir_exists(const char* dir_path) {
#if defined(NEKO_IS_WIN32)
    DWORD attrib = GetFileAttributes((LPCWSTR)dir_path);  // TODO: unicode 路径修复
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

i32 neko_os_mkdir(const char* dir_path, i32 opt) {
#ifdef NEKO_IS_WIN32
    return _mkdir(dir_path);
#else
    return mkdir(dir_path, opt);
#endif
}

bool neko_os_file_exists(const char* file_path) {
    const char* path = file_path;

#ifdef NEKO_IS_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE* fp = neko_fopen(file_path, "r");
    if (fp) {
        neko_fclose(fp);
        return true;
    }
    return false;
}

i32 neko_os_file_size_in_bytes(const char* file_path) {
#ifdef NEKO_IS_WIN32

    HANDLE hFile = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;  // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;  // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return neko_util_safe_truncate_u64(size.QuadPart);

#elif (defined NEKO_IS_ANDROID)

    const char* internal_data_path = neko_app()->android.internal_data_path;
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

void neko_os_file_extension(char* buffer, size_t buffer_sz, const char* file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

i32 neko_os_file_delete(const char* file_path) {
#if (defined NEKO_IS_WIN32)

    // Non-zero if successful
    return DeleteFileA(file_path);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)

    // Returns 0 if successful
    return !remove(file_path);

#endif

    return 0;
}

i32 neko_os_file_copy(const char* src_path, const char* dst_path) {
#if (defined NEKO_IS_WIN32)

    return CopyFileA(src_path, dst_path, false);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)

    FILE* file_w = NULL;
    FILE* file_r = NULL;
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

neko_os_file_stats_t neko_os_file_stats(const char* file_path) {
    neko_os_file_stats_t stats = NEKO_DEFAULT_VAL();

#if (defined NEKO_IS_WIN32)

    WIN32_FILE_ATTRIBUTE_DATA data = NEKO_DEFAULT_VAL();
    FILETIME ftime = NEKO_DEFAULT_VAL();
    FILETIME ctime = NEKO_DEFAULT_VAL();
    FILETIME atime = NEKO_DEFAULT_VAL();
    if (GetFileAttributesExA(file_path, GetFileExInfoStandard, &data)) {
        ftime = data.ftLastWriteTime;
        ctime = data.ftCreationTime;
        atime = data.ftLastAccessTime;
    }

    stats.modified_time = *((u64*)&ftime);
    stats.access_time = *((u64*)&atime);
    stats.creation_time = *((u64*)&ctime);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    struct stat attr = NEKO_DEFAULT_VAL();
    stat(file_path, &attr);
    stats.modified_time = *((u64*)&attr.st_mtime);

#endif

    return stats;
}

void* neko_os_library_load(const char* lib_path) {
#if (defined NEKO_IS_WIN32)
    return (void*)LoadLibraryA(lib_path);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    return (void*)dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);  // RTLD_LAZY
#endif
    return NULL;
}

void neko_os_library_unload(void* lib) {
    if (!lib) return;
#if (defined NEKO_IS_WIN32)
    FreeLibrary((HMODULE)lib);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    dlclose(lib);
#endif
}

void* neko_os_library_proc_address(void* lib, const char* func) {
    if (!lib) return NULL;
#if (defined NEKO_IS_WIN32)
    return (void*)GetProcAddress((HMODULE)lib, func);
#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)
    return (void*)dlsym(lib, func);
#endif
    return NULL;
}

int neko_os_chdir(const char* path) {
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
#define HOMEDIR (char*)"HOME"
#endif
    const_str path = std::getenv(HOMEDIR);
    return {path};
}

f32 timing_dt;
f32 timing_true_dt;
static f32 scale = 1.0f;
static bool paused = false;

void timing_set_scale(f32 s) { scale = s; }
f32 timing_get_scale() { return scale; }

void timing_set_paused(bool p) { paused = p; }
bool timing_get_paused() { return paused; }

static void _dt_update() {
    static double last_time = -1;
    double curr_time;

    // first update?
    if (last_time < 0) last_time = glfwGetTime();

    curr_time = glfwGetTime();
    timing_true_dt = curr_time - last_time;
    timing_dt = paused ? 0.0f : scale * timing_true_dt;
    last_time = curr_time;
}

void timing_update() { _dt_update(); }

void timing_save_all(Store* s) {
    Store* t;

    if (store_child_save(&t, "timing", s)) scalar_save(&scale, "scale", t);
}
void timing_load_all(Store* s) {
    Store* t;

    if (store_child_load(&t, "timing", s)) scalar_load(&scale, "scale", 1, t);
}

void neko_log(int level, const char* file, int line, const char* fmt, ...) {

    LockGuard lock{&g_app->log_mtx};

    typedef struct {
        va_list ap;
        const char* fmt;
        const char* file;
        u32 time;
        FILE* udata;
        int line;
        int level;
    } neko_log_event;

    static auto init_event = [](neko_log_event* ev, void* udata) {
        static u32 t = 0;
        if (!ev->time) {
            ev->time = ++t;
        }
        ev->udata = (FILE*)udata;
    };

    static const char* level_strings[] = {"T", "D", "I", "W", "E"};

    neko_log_event ev = {
            .fmt = fmt,
            .file = file,
            .line = line,
            .level = level,
    };

    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    fprintf(ev.udata, "%-1s %s:%d: ", level_strings[ev.level], neko_util_get_filename(ev.file), ev.line);
    vfprintf(ev.udata, ev.fmt, ev.ap);
    fprintf(ev.udata, "\n");
    fflush(ev.udata);
    va_end(ev.ap);

    // console callback
    // if (NULL != neko_instance() && NULL != neko_instance()->console) {
    //     va_start(ev.ap, fmt);
    //     char buffer[512] = NEKO_DEFAULT_VAL();
    //     vsnprintf(buffer, 512, ev.fmt, ev.ap);
    //     neko_console_printf(neko_instance()->console, "%-1s %s:%d: ", level_strings[ev.level], neko_util_get_filename(ev.file), ev.line);
    //     neko_console_printf(neko_instance()->console, buffer);
    //     neko_console_printf(neko_instance()->console, "\n");
    //     va_end(ev.ap);
    // }
}

static void _error(const char* s) { script_error(s); }

void errorf(const char* fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char* s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char*)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);
    _error(s);
    mem_free(s);
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
    module.hndl = (void*)neko_os_library_load(filename);
    return module;
}

void neko_dylib_close(neko_dynlib lib) { neko_os_library_unload(lib.hndl); }

void* neko_dylib_get_symbol(neko_dynlib lib, const_str symbol_name) {
    void* symbol = (void*)neko_os_library_proc_address(lib.hndl, symbol_name);
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

#if defined(NEKO_IS_WIN32)

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

#ifdef NEKO_IS_WIN32

#include <Windows.h>

namespace neko::filewatch {

class task : public OVERLAPPED {
    static const size_t kBufSize = 16 * 1024;

public:
    task() noexcept;
    ~task() noexcept;

    enum class result {
        success,
        wait,
        failed,
        zero,
    };

    bool open(const std::wstring& path) noexcept;
    bool start(bool recursive) noexcept;
    void cancel() noexcept;
    result try_read() noexcept;
    const std::wstring& path() const noexcept;
    const std::byte* data() const noexcept;

private:
    std::wstring m_path;
    HANDLE m_directory;
    std::array<std::byte, kBufSize> m_buffer;
};

task::task() noexcept : m_path(), m_directory(INVALID_HANDLE_VALUE), m_buffer() {
    memset((OVERLAPPED*)this, 0, sizeof(OVERLAPPED));
    hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
}

task::~task() noexcept { assert(m_directory == INVALID_HANDLE_VALUE); }

bool task::open(const std::wstring& path) noexcept {
    if (m_directory != INVALID_HANDLE_VALUE) {
        return true;
    }
    if (path.back() != L'/') {
        m_path = path + L"/";
    } else {
        m_path = path;
    }
    m_directory =
            ::CreateFileW(m_path.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (m_directory == INVALID_HANDLE_VALUE) {
        return false;
    }
    return true;
}

void task::cancel() noexcept {
    if (m_directory != INVALID_HANDLE_VALUE) {
        ::CancelIo(m_directory);
        ::CloseHandle(m_directory);
        m_directory = INVALID_HANDLE_VALUE;
    }
}

bool task::start(bool recursive) noexcept {
    if (m_directory == INVALID_HANDLE_VALUE) {
        return false;
    }
    if (!ResetEvent(hEvent)) {
        return false;
    }
    if (!::ReadDirectoryChangesW(m_directory, &m_buffer[0], static_cast<DWORD>(m_buffer.size()), recursive,
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, NULL, this, NULL)) {
        ::CloseHandle(m_directory);
        m_directory = INVALID_HANDLE_VALUE;
        return false;
    }
    return true;
}

task::result task::try_read() noexcept {
    DWORD dwNumberOfBytesTransfered = 0;
    const bool ok = GetOverlappedResult(m_directory, this, &dwNumberOfBytesTransfered, FALSE);
    const DWORD dwErrorCode = ::GetLastError();
    if (!ok) {
        if (dwErrorCode == ERROR_IO_INCOMPLETE) {
            return result::wait;
        }
    }
    if (dwErrorCode != 0) {
        if (dwErrorCode == ERROR_NOTIFY_ENUM_DIR) {
            // TODO 通知溢出
            return result::zero;
        }
        cancel();
        return result::failed;
    }
    if (!dwNumberOfBytesTransfered) {
        return result::zero;
    }
    assert(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));
    assert(dwNumberOfBytesTransfered <= m_buffer.size());
    return result::success;
}

const std::wstring& task::path() const noexcept { return m_path; }

const std::byte* task::data() const noexcept { return m_buffer.data(); }

watch::watch() noexcept : m_notify(), m_tasks() {}

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    if (m_tasks.empty()) {
        return;
    }
    for (auto& task : m_tasks) {
        task.cancel();
    }
    m_tasks.clear();
}

void watch::add(const string_type& path) noexcept {
    auto& t = m_tasks.emplace_back();
    if (t.open(path)) {
        if (t.start(m_recursive)) {
            return;
        }
    }
    m_tasks.pop_back();
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept { return false; }

bool watch::set_filter(filter f) noexcept { return false; }

bool watch::event_update(task& task) noexcept {
    switch (task.try_read()) {
        case task::result::wait:
            return true;
        case task::result::failed:
            task.cancel();
            return false;
        case task::result::zero:
            return task.start(m_recursive);
        case task::result::success:
            break;
    }
    const std::byte* data = task.data();
    for (;;) {
        const FILE_NOTIFY_INFORMATION& fni = (const FILE_NOTIFY_INFORMATION&)*data;
        std::wstring path(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
        path = task.path() + path;
        switch (fni.Action) {
            case FILE_ACTION_MODIFIED:
                m_notify.emplace(notify::flag::modify, wtf8::w2u(path));
                break;
            case FILE_ACTION_ADDED:
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_RENAMED_OLD_NAME:
            case FILE_ACTION_RENAMED_NEW_NAME:
                m_notify.emplace(notify::flag::rename, wtf8::w2u(path));
                break;
            default:
                NEKO_ASSERT(0, "unreachable");
                break;
        }
        if (!fni.NextEntryOffset) {
            break;
        }
        data += fni.NextEntryOffset;
    }
    return task.start(m_recursive);
}

std::optional<notify> watch::select() noexcept {
    for (auto iter = m_tasks.begin(); iter != m_tasks.end();) {
        if (event_update(*iter)) {
            ++iter;
        } else {
            iter = m_tasks.erase(iter);
        }
    }
    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto n = m_notify.front();
    m_notify.pop();
    return n;
}
}  // namespace neko::filewatch

#elif defined(NEKO_IS_LINUX)

#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <functional>

namespace neko::filewatch {
watch::watch() noexcept : m_notify(), m_fd_path(), m_inotify_fd(inotify_init1(IN_NONBLOCK | IN_CLOEXEC)) { assert(m_inotify_fd != -1); }

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    if (m_inotify_fd == -1) {
        return;
    }
    for (auto& [desc, _] : m_fd_path) {
        (void)_;
        inotify_rm_watch(m_inotify_fd, desc);
    }
    m_fd_path.clear();
    close(m_inotify_fd);
    m_inotify_fd = -1;
}

void watch::add(const string_type& str) noexcept {
    if (m_inotify_fd == -1) {
        return;
    }
    if (!m_filter(str.c_str())) {
        return;
    }
    std::filesystem::path path = str;
    if (m_follow_symlinks) {
        std::error_code ec;
        path = std::filesystem::canonical(path, ec);
        if (ec) {
            return;
        }
    }
    int desc = inotify_add_watch(m_inotify_fd, path.c_str(), IN_ALL_EVENTS);
    if (desc != -1) {
        const auto& emplace_result = m_fd_path.emplace(std::make_pair(desc, path.string()));
        if (!emplace_result.second) {
            return;
        }
    }
    if (!m_recursive) {
        return;
    }
    std::error_code ec;
    std::filesystem::directory_iterator iter{path, std::filesystem::directory_options::skip_permission_denied, ec};
    std::filesystem::directory_iterator end{};
    for (; !ec && iter != end; iter.increment(ec)) {
        std::error_code file_status_ec;
        if (std::filesystem::is_directory(m_follow_symlinks ? iter->status(file_status_ec) : iter->symlink_status(file_status_ec))) {
            add(iter->path());
        }
    }
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept {
    m_follow_symlinks = enable;
    return true;
}

bool watch::set_filter(filter f) noexcept {
    m_filter = f;
    return true;
}

void watch::event_update(void* e) noexcept {
    inotify_event* event = (inotify_event*)e;
    if (event->mask & IN_Q_OVERFLOW) {
        // TODO?
    }

    auto filename = m_fd_path[event->wd];
    if (event->len > 1) {
        filename += "/";
        filename += std::string(event->name);
    }
    if (event->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)) {
        m_notify.emplace(notify::flag::rename, filename);
    } else if (event->mask & (IN_MOVE_SELF | IN_ATTRIB | IN_CLOSE_WRITE | IN_MODIFY)) {
        m_notify.emplace(notify::flag::modify, filename);
    }

    if (event->mask & (IN_IGNORED | IN_DELETE_SELF)) {
        m_fd_path.erase(event->wd);
    }
    if (event->mask & IN_MOVE_SELF) {
        inotify_rm_watch(m_inotify_fd, event->wd);
        m_fd_path.erase(event->wd);
    }
    if (m_recursive && (event->mask & IN_ISDIR) && (event->mask & IN_CREATE)) {
        add(filename);
    }
}

std::optional<notify> watch::select() noexcept {
    do {
        if (m_inotify_fd == -1) {
            break;
        }

        struct pollfd pfd_read;
        pfd_read.fd = m_inotify_fd;
        pfd_read.events = POLLIN;
        if (poll(&pfd_read, 1, 0) != 1) {
            break;
        }

        std::byte buf[4096];
        ssize_t n = read(m_inotify_fd, buf, sizeof buf);
        if (n == 0 || n == -1) {
            break;
        }
        for (std::byte* p = buf; p < buf + n;) {
            auto event = (struct inotify_event*)p;
            event_update(event);
            p += sizeof(*event) + event->len;
        }
    } while (false);

    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto msg = m_notify.front();
    m_notify.pop();
    return msg;
}
}  // namespace neko::filewatch

#elif defined(NEKO_IS_APPLE)

namespace neko::filewatch {
static void event_cb(ConstFSEventStreamRef streamRef, void* info, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) noexcept {
    (void)streamRef;
    (void)eventIds;
    watch* self = (watch*)info;
    self->event_update((const char**)eventPaths, eventFlags, numEvents);
}

watch::watch() noexcept : m_notify(), m_paths(), m_stream(NULL) {}

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    destroy_stream();
    m_paths.clear();
}

bool watch::create_stream(CFArrayRef cf_paths) noexcept {
    if (m_stream) {
        return false;
    }
    FSEventStreamContext ctx = {0, this, NULL, NULL, NULL};

    FSEventStreamRef ref = FSEventStreamCreate(NULL, &event_cb, &ctx, cf_paths, kFSEventStreamEventIdSinceNow, 0.05, kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents);
    if (ref == NULL) {
        return false;
    }
    m_fsevent_queue = dispatch_queue_create("fsevent_queue", NULL);
    FSEventStreamSetDispatchQueue(ref, m_fsevent_queue);
    if (!FSEventStreamStart(ref)) {
        FSEventStreamInvalidate(ref);
        FSEventStreamRelease(ref);
        return false;
    }
    m_stream = ref;
    return true;
}

void watch::destroy_stream() noexcept {
    if (!m_stream) {
        return;
    }
    FSEventStreamStop(m_stream);
    FSEventStreamInvalidate(m_stream);
    FSEventStreamRelease(m_stream);
    dispatch_release(m_fsevent_queue);
    m_stream = NULL;
}

void watch::add(const string_type& path) noexcept {
    m_paths.emplace(path);
    update_stream();
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept { return false; }

bool watch::set_filter(filter f) noexcept { return false; }

void watch::update_stream() noexcept {
    destroy_stream();
    if (m_paths.empty()) {
        return;
    }
    std::unique_ptr<CFStringRef[]> paths(new CFStringRef[m_paths.size()]);
    size_t i = 0;
    for (auto& path : m_paths) {
        paths[i] = CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
        if (paths[i] == NULL) {
            while (i != 0) {
                CFRelease(paths[--i]);
            }
            return;
        }
        i++;
    }
    CFArrayRef cf_paths = CFArrayCreate(NULL, (const void**)&paths[0], m_paths.size(), NULL);
    if (create_stream(cf_paths)) {
        return;
    }
    CFRelease(cf_paths);
}

void watch::event_update(const char* paths[], const FSEventStreamEventFlags flags[], size_t n) noexcept {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (size_t i = 0; i < n; ++i) {
        const char* path = paths[i];
        if (!m_recursive && path[0] != '\0' && strchr(path + 1, '/') != NULL) {
            continue;
        }
        if (flags[i] & (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagItemRenamed)) {
            m_notify.emplace(notify::flag::rename, path);
        } else if (flags[i] & (kFSEventStreamEventFlagItemFinderInfoMod | kFSEventStreamEventFlagItemModified | kFSEventStreamEventFlagItemInodeMetaMod | kFSEventStreamEventFlagItemChangeOwner |
                               kFSEventStreamEventFlagItemXattrMod)) {
            m_notify.emplace(notify::flag::modify, path);
        }
    }
}

std::optional<notify> watch::select() noexcept {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto n = m_notify.front();
    m_notify.pop();
    return n;
}
}  // namespace neko::filewatch

#else

#endif

#if NEKO_NEW

void* operator new(std::size_t n) {
    void* p = mem_alloc(n);
    if (!p) {
        abort(); /* FIXME: do not abort */
    }
    return p;
}

void* operator new[](std::size_t n) {
    void* p = mem_alloc(n);
    if (!p) {
        abort();
    }
    return p;
}

void operator delete(void* p) { mem_free(p); }

void operator delete[](void* p) { mem_free(p); }

void operator delete(void* p, std::size_t) { mem_free(p); }

void operator delete[](void* p, std::size_t) { mem_free(p); }

#endif