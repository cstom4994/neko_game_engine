
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <optional>
#include <queue>
#include <string>

#include "engine/base.h"
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

template <typename F>
struct Defer {
    F f;
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
};

template <typename F>
Defer<F> defer_func(F f) {
    return Defer<F>(f);
}

#define neko_defer(code) auto JOIN_2(_defer_, __COUNTER__) = defer_func([&]() { code; })

#define DeferLoop(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))

struct String {
    char *data = nullptr;
    u64 len = 0;

    String() = default;
    String(const char *cstr) : data((char *)cstr), len(strlen(cstr)) {}
    String(const char *cstr, u64 n) : data((char *)cstr), len(n) {}

    bool is_cstr();
    String substr(u64 i, u64 j);
    bool starts_with(String match);
    bool ends_with(String match);
    u64 first_of(char c);
    u64 last_of(char c);

    inline const_str cstr() const { return data; }

    char *begin() { return data; }
    char *end() { return &data[len]; }
};

String to_cstr(String str);

constexpr u64 fnv1a(const char *str, u64 len) {
    u64 hash = 14695981039346656037u;
    for (u64 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 1099511628211;
    }
    return hash;
}

inline u64 fnv1a(String str) { return fnv1a(str.data, str.len); }

constexpr u64 operator"" _hash(const char *str, size_t len) { return fnv1a(str, len); }

inline bool operator==(String lhs, String rhs) {
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

inline bool operator!=(String lhs, String rhs) { return !(lhs == rhs); }

struct SplitLinesIterator {
    String data;
    String view;

    String operator*() const { return view; }
    SplitLinesIterator &operator++();
};

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs);

struct SplitLines {
    String str;
    SplitLines(String s) : str(s) {}

    SplitLinesIterator begin();
    SplitLinesIterator end();
};

i32 utf8_size(u8 c);

struct Rune {
    u32 value;

    u32 charcode();
    bool is_whitespace();
    bool is_digit();
};

Rune rune_from_string(const char *buf);

struct UTF8Iterator {
    String str;
    u64 cursor;
    Rune rune;

    Rune operator*() const { return rune; }
    UTF8Iterator &operator++();
};

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs);

struct UTF8 {
    String str;
    UTF8(String s) : str(s) {}

    UTF8Iterator begin();
    UTF8Iterator end();
};

struct StringBuilder {
    char *data;
    u64 len;       // does not include null term
    u64 capacity;  // includes null term

    StringBuilder();

    void trash();
    void reserve(u64 capacity);
    void clear();
    void swap_filename(String filepath, String file);
    void concat(String str, i32 times);

    StringBuilder &operator<<(String str);
    explicit operator String();
};

FORMAT_ARGS(1) String str_fmt(const char *fmt, ...);
FORMAT_ARGS(1) String tmp_fmt(const char *fmt, ...);

double string_to_double(String str);

/*=============================
//
=============================*/

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

/*=============================
// Memory
=============================*/

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
    void dump_allocs(bool detailed);
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

inline void *neko_malloc_init_impl(size_t sz) {
    void *data = mem_alloc(sz);
    memset(data, 0, sz);
    return data;
}

#define neko_malloc_init(__T) (__T *)neko_malloc_init_impl(sizeof(__T))

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

/*=============================
//
=============================*/

template <typename T>
struct Queue {
    Mutex mtx = {};
    Cond cv = {};

    T *data = nullptr;
    u64 front = 0;
    u64 back = 0;
    u64 len = 0;
    u64 capacity = 0;

    void make() {
        mtx.make();
        cv.make();
    }

    void trash() {
        mtx.trash();
        cv.trash();
        mem_free(data);
    }

    void reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        T *buf = (T *)mem_alloc(sizeof(T) * cap);

        if (front < back) {
            memcpy(buf, &data[front], sizeof(T) * len);
        } else {
            u64 lhs = back;
            u64 rhs = (capacity - front);

            memcpy(buf, &data[front], sizeof(T) * rhs);
            memcpy(&buf[rhs], &data[0], sizeof(T) * lhs);
        }

        mem_free(data);

        data = buf;
        front = 0;
        back = len;
        capacity = cap;
    }

    void enqueue(T item) {
        LockGuard lock{&mtx};

        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }

        data[back] = item;
        back = (back + 1) % capacity;
        len++;

        cv.signal();
    }

    T demand() {
        LockGuard lock{&mtx};

        while (len == 0) {
            cv.wait(&mtx);
        }

        T item = data[front];
        front = (front + 1) % capacity;
        len--;

        return item;
    }
};

template <typename T>
struct Array {
    T *data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T &operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void trash() { mem_free(data); }

    void reserve(u64 cap) {
        if (cap > capacity) {
            T *buf = (T *)mem_alloc(sizeof(T) * cap);
            memcpy(buf, data, sizeof(T) * len);
            mem_free(data);
            data = buf;
            capacity = cap;
        }
    }

    void resize(u64 n) {
        reserve(n);
        len = n;
    }

    void push(T item) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }
        data[len] = item;
        len++;
    }

    T *begin() { return data; }
    T *end() { return &data[len]; }
};

template <typename T>
struct PriorityQueue {
    T *data = nullptr;
    float *costs = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    void trash() {
        mem_free(data);
        mem_free(costs);
    }

    void reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        T *buf = (T *)mem_alloc(sizeof(T) * cap);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;

        float *cbuf = (float *)mem_alloc(sizeof(float) * cap);
        memcpy(cbuf, costs, sizeof(float) * len);
        mem_free(costs);
        costs = cbuf;

        capacity = cap;
    }

    void swap(i32 i, i32 j) {
        T t = data[i];
        data[i] = data[j];
        data[j] = t;

        float f = costs[i];
        costs[i] = costs[j];
        costs[j] = f;
    }

    void shift_up(i32 j) {
        while (j > 0) {
            i32 i = (j - 1) / 2;
            if (i == j || costs[i] < costs[j]) {
                break;
            }

            swap(i, j);
            j = i;
        }
    }

    void shift_down(i32 i, i32 n) {
        if (i < 0 || i > n) {
            return;
        }

        i32 j = 2 * i + 1;
        while (j >= 0 && j < n) {
            if (j + 1 < n && costs[j + 1] < costs[j]) {
                j = j + 1;
            }

            if (costs[i] < costs[j]) {
                break;
            }

            swap(i, j);
            i = j;
            j = 2 * i + 1;
        }
    }

    void push(T item, float cost) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }

        data[len] = item;
        costs[len] = cost;
        len++;

        shift_up(len - 1);
    }

    bool pop(T *item) {
        if (len == 0) {
            return false;
        }

        *item = data[0];

        data[0] = data[len - 1];
        costs[0] = costs[len - 1];
        len--;

        shift_down(0, len);
        return true;
    }
};

enum HashMapKind : u8 {
    HashMapKind_None,
    HashMapKind_Some,
    HashMapKind_Tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

template <typename T>
struct HashMap {
    u64 *keys = nullptr;
    T *values = nullptr;
    HashMapKind *kinds = nullptr;
    u64 load = 0;
    u64 capacity = 0;

    void trash() {
        mem_free(keys);
        mem_free(values);
        mem_free(kinds);
    }

    u64 find_entry(u64 key) const {
        u64 index = key & (capacity - 1);
        u64 tombstone = (u64)-1;
        while (true) {
            HashMapKind kind = kinds[index];
            if (kind == HashMapKind_None) {
                return tombstone != (u64)-1 ? tombstone : index;
            } else if (kind == HashMapKind_Tombstone) {
                tombstone = index;
            } else if (keys[index] == key) {
                return index;
            }

            index = (index + 1) & (capacity - 1);
        }
    }

    void real_reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        HashMap<T> map = {};
        map.capacity = cap;

        size_t bytes = sizeof(u64) * cap;
        map.keys = (u64 *)mem_alloc(bytes);
        memset(map.keys, 0, bytes);

        map.values = (T *)mem_alloc(sizeof(T) * cap);
        memset(map.values, 0, sizeof(T) * cap);

        map.kinds = (HashMapKind *)mem_alloc(sizeof(HashMapKind) * cap);
        memset(map.kinds, 0, sizeof(HashMapKind) * cap);

        for (u64 i = 0; i < capacity; i++) {
            HashMapKind kind = kinds[i];
            if (kind != HashMapKind_Some) {
                continue;
            }

            u64 index = map.find_entry(keys[i]);
            map.keys[index] = keys[i];
            map.values[index] = values[i];
            map.kinds[index] = HashMapKind_Some;
            map.load++;
        }

        mem_free(keys);
        mem_free(values);
        mem_free(kinds);
        *this = map;
    }

    void reserve(u64 capacity) {
        u64 n = (u64)(capacity / HASH_MAP_LOAD_FACTOR) + 1;

        // next pow of 2
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;

        real_reserve(n);
    }

    T *get(u64 key) {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    const T *get(u64 key) const {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    bool find_or_insert(u64 key, T **value) {
        if (load >= capacity * HASH_MAP_LOAD_FACTOR) {
            real_reserve(capacity > 0 ? capacity * 2 : 16);
        }

        u64 index = find_entry(key);
        bool exists = kinds[index] == HashMapKind_Some;
        if (!exists) {
            values[index] = {};
        }

        if (kinds[index] == HashMapKind_None) {
            load++;
            keys[index] = key;
            kinds[index] = HashMapKind_Some;
        }

        *value = &values[index];
        return exists;
    }

    T &operator[](u64 key) {
        T *value;
        find_or_insert(key, &value);
        return *value;
    }

    void unset(u64 key) {
        if (load == 0) {
            return;
        }

        u64 index = find_entry(key);
        if (kinds[index] != HashMapKind_None) {
            kinds[index] = HashMapKind_Tombstone;
        }
    }

    void clear() {
        memset(keys, 0, sizeof(u64) * capacity);
        memset(values, 0, sizeof(T) * capacity);
        memset(kinds, 0, sizeof(HashMapKind) * capacity);
        load = 0;
    }
};

template <typename T>
struct HashMapKV {
    u64 key;
    T *value;
};

template <typename T>
struct HashMapIterator {
    HashMap<T> *map;
    u64 cursor;

    HashMapKV<T> operator*() const {
        HashMapKV<T> kv;
        kv.key = map->keys[cursor];
        kv.value = &map->values[cursor];
        return kv;
    }

    HashMapIterator &operator++() {
        cursor++;
        while (cursor != map->capacity) {
            if (map->kinds[cursor] == HashMapKind_Some) {
                return *this;
            }
            cursor++;
        }

        return *this;
    }
};

template <typename T>
bool operator!=(HashMapIterator<T> lhs, HashMapIterator<T> rhs) {
    return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T>
HashMapIterator<T> begin(HashMap<T> &map) {
    HashMapIterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;

    for (u64 i = 0; i < map.capacity; i++) {
        if (map.kinds[i] == HashMapKind_Some) {
            it.cursor = i;
            break;
        }
    }

    return it;
}

template <typename T>
HashMapIterator<T> end(HashMap<T> &map) {
    HashMapIterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;
    return it;
}

struct ArenaNode;
struct Arena {
    ArenaNode *head;

    void trash();
    void *bump(u64 size);
    void *rebump(void *ptr, u64 old, u64 size);
    String bump_string(String s);
};

template <typename T>
struct Slice {
    T *data = nullptr;
    u64 len = 0;

    Slice() = default;
    explicit Slice(Array<T> arr) : data(arr.data), len(arr.len) {}

    T &operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    const T &operator[](size_t i) const {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void resize(u64 n) {
        T *buf = (T *)mem_alloc(sizeof(T) * n);
        memcpy(buf, data, sizeof(T) * len);
        mem_free(data);
        data = buf;
        len = n;
    }

    void resize(Arena *arena, u64 n) {
        T *buf = (T *)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
        data = buf;
        len = n;
    }

    T *begin() { return data; }
    T *end() { return &data[len]; }
    const T *begin() const { return data; }
    const T *end() const { return &data[len]; }
};

struct Scanner {
    char *data;
    u64 len;
    u64 pos;
    u64 end;

    Scanner(String str);
    String next_string();
    i32 next_int();
};