#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <optional>
#include <queue>
#include <string>

#include "engine/prelude.h"
#include "engine/prelude.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

#if defined(_WIN32)
#include <list>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>

#include <mutex>
#include <set>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <map>
#else
#error unsupport platform
#endif

struct Mutex {
#ifdef _WIN32
    SRWLOCK srwlock;
#else
    pthread_mutex_t pt;
#endif

    void make();
    void trash();
    void lock();
    void unlock();
    bool try_lock();
};

struct Cond {
#ifdef _WIN32
    CONDITION_VARIABLE cv;
#else
    pthread_cond_t pt;
#endif

    void make();
    void trash();
    void signal();
    void broadcast();
    void wait(Mutex *mtx);
    bool timed_wait(Mutex *mtx, uint32_t ms);
};

struct RWLock {
#if _WIN32
    SRWLOCK srwlock;
#else
    pthread_rwlock_t pt;
#endif

    void make();
    void trash();
    void shared_lock();
    void shared_unlock();
    void unique_lock();
    void unique_unlock();
};

struct Sema {
#ifdef _WIN32
    HANDLE handle;
#else
    sem_t *sem;
#endif

    void make(int n = 0);
    void trash();
    void post(int n = 1);
    void wait();
};

typedef void (*ThreadProc)(void *);

struct Thread {
    void *ptr = nullptr;

    void make(ThreadProc fn, void *udata);
    void join();
};

struct LockGuard {
    Mutex *mtx;

    LockGuard(Mutex *mtx) : mtx(mtx) { mtx->lock(); };
    ~LockGuard() { mtx->unlock(); };
    LockGuard(LockGuard &&) = delete;
    LockGuard &operator=(LockGuard &&) = delete;

    operator bool() { return true; }
};

uint64_t this_thread_id();

struct Allocator {
    size_t alloc_size;
    virtual void make() = 0;
    virtual void trash() = 0;
    virtual void *alloc(size_t bytes, const char *file, i32 line) = 0;
    virtual void *realloc(void *ptr, size_t new_size, const char *file, i32 line) = 0;
    virtual void free(void *ptr) = 0;
};

struct HeapAllocator : Allocator {
    void make() {}
    void trash() {}
    void *alloc(size_t bytes, const char *, i32) { return malloc(bytes); }
    void *realloc(void *ptr, size_t new_size, const char *, i32) { return ::realloc(ptr, new_size); }
    void free(void *ptr) { ::free(ptr); }
};

struct DebugAllocInfo {
    const char *file;
    i32 line;
    size_t size;
    DebugAllocInfo *prev;
    DebugAllocInfo *next;
    alignas(16) u8 buf[1];
};

struct DebugAllocator : Allocator {
    DebugAllocInfo *head = nullptr;
    Mutex mtx = {};

    void make() { mtx.make(); }
    void trash() { mtx.trash(); }
    void *alloc(size_t bytes, const char *file, i32 line);
    void *realloc(void *ptr, size_t new_size, const char *file, i32 line);
    void free(void *ptr);
    void dump_allocs();
};

extern Allocator *g_allocator;

inline void *__neko_mem_calloc(size_t count, size_t element_size, const char *file, int line) {
    size_t size = count * element_size;
    void *mem = g_allocator->alloc(size, file, line);
    memset(mem, 0, size);
    return mem;
}

#define mem_alloc(bytes) g_allocator->alloc(bytes, __FILE__, __LINE__)
#define mem_free(ptr) g_allocator->free((void *)ptr)
#define mem_realloc(ptr, size) g_allocator->realloc(ptr, size, __FILE__, __LINE__)
#define mem_calloc(count, element_size) __neko_mem_calloc(count, element_size, (char *)__FILE__, __LINE__)

// inline void *operator new(std::size_t, void *p) noexcept { return p; }
// inline void *operator new[](std::size_t, void *p) noexcept { return p; }
// inline void operator delete(void *, void *) noexcept {}
// inline void operator delete[](void *, void *) noexcept {}

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

typedef struct Store Store;

SCRIPT(timing,

       NEKO_EXPORT f32 timing_dt;

       NEKO_EXPORT f32 timing_true_dt;  // 实际增量时间 不受 scale/pause 影响

       NEKO_EXPORT void timing_set_scale(f32 s);

       NEKO_EXPORT f32 timing_get_scale();

       NEKO_EXPORT void timing_set_paused(bool p);  // 暂停将刻度设置为 0 并在恢复时恢复它

       NEKO_EXPORT bool timing_get_paused();

)

void timing_update();
void timing_save_all(Store *s);
void timing_load_all(Store *s);

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

static inline u64 neko_get_thread_id() {
#if defined(NEKO_IS_WIN32)
    return (u64)GetCurrentThreadId();
#elif defined(NEKO_IS_LINUX)
    return (u64)syscall(SYS_gettid);
#elif defined(NEKO_IS_APPLE)
    return (mach_port_t)pthread_mach_thread_np(pthread_self());
#elif defined(NEKO_IS_WEB)
    return 0;
#else
#error "Unsupported platform!"
#endif
}

typedef struct neko_dynlib {
    void *hndl;
} neko_dynlib;

neko_dynlib neko_dylib_open(const_str name);
void neko_dylib_close(neko_dynlib lib);
void *neko_dylib_get_symbol(neko_dynlib lib, const_str symbol_name);
bool neko_dylib_has_symbol(neko_dynlib lib, const_str symbol_name);

namespace neko::filewatch {
class task;

struct notify {
    enum class flag {
        modify,
        rename,
    };
    flag flags;
    std::string path;
    notify(const flag &flags, const std::string &path) noexcept : flags(flags), path(path) {}
};

class watch {
public:
#if defined(_WIN32)
    using string_type = std::wstring;
#else
    using string_type = std::string;
#endif
    using filter = std::function<bool(const char *)>;
    static inline filter DefaultFilter = [](const char *) { return true; };

    watch() noexcept;
    ~watch() noexcept;
    void stop() noexcept;
    void add(const string_type &path) noexcept;
    void set_recursive(bool enable) noexcept;
    bool set_follow_symlinks(bool enable) noexcept;
    bool set_filter(filter f = DefaultFilter) noexcept;
    std::optional<notify> select() noexcept;
#if defined(__APPLE__)
    void event_update(const char *paths[], const FSEventStreamEventFlags flags[], size_t n) noexcept;
#endif

private:
#if defined(_WIN32)
    bool event_update(task &task) noexcept;
#elif defined(__APPLE__)
    bool create_stream(CFArrayRef cf_paths) noexcept;
    void destroy_stream() noexcept;
    void update_stream() noexcept;
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    void event_update(void *event) noexcept;
#endif

private:
    std::queue<notify> m_notify;
    bool m_recursive = true;
#if defined(_WIN32)
    std::list<task> m_tasks;
#elif defined(__APPLE__)
    std::mutex m_mutex;
    std::set<std::string> m_paths;
    FSEventStreamRef m_stream;
    dispatch_queue_t m_fsevent_queue;
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    std::map<int, std::string> m_fd_path;
    int m_inotify_fd;
    bool m_follow_symlinks = false;
    filter m_filter = DefaultFilter;
#endif
};
}  // namespace neko::filewatch
