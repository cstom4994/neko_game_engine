
#if !defined(NEKO_BASE_HPP)
#define NEKO_BASE_HPP

#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <optional>
#include <queue>
#include <string>

#include "engine/base.h"

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

#undef mem_alloc
#undef mem_free
#undef mem_realloc
#undef mem_calloc
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

// #include <intrin.h>

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#define NEKO_DYNAMIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                    \
    type *cast_var_name = dynamic_cast<type *>(input_var); \
    neko_assert(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                   \
    type *cast_var_name = static_cast<type *>(input_var); \
    neko_assert(cast_var_name)

#define NEKO_ENUM_FLAG(T)                                                                                                                                                    \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                       \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T &operator|=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator&=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator^=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define NEKO_MOVEONLY(class_name)                       \
    class_name(const class_name &) = delete;            \
    class_name &operator=(const class_name &) = delete; \
    class_name(class_name &&) = default;                \
    class_name &operator=(class_name &&) = default

namespace detail {
template <typename... Args>
constexpr std::size_t va_count(Args &&...) {
    return sizeof...(Args);
}
}  // namespace detail

#define neko_macro_overload(fun, a, ...)                                               \
    do {                                                                               \
        if (const bool a_ = (a); a_)                                                   \
            [&](auto &&...args) {                                                      \
                const auto t = std::make_tuple(std::forward<decltype(args)>(args)...); \
                constexpr auto N = std::tuple_size<decltype(t)>::value;                \
                                                                                       \
                if constexpr (N == 0) {                                                \
                    fun(a_);                                                           \
                } else {                                                               \
                    fun(a_, __VA_ARGS__);                                              \
                }                                                                      \
            }(__VA_ARGS__);                                                            \
    } while (0)

#define neko_check_is_trivial(type, ...) static_assert(std::is_trivial<type>::value, __VA_ARGS__)

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
inline void neko_addoffset(std::ptrdiff_t offset, T *&ptr) {
    if (!ptr) return;
    ptr = (T *)((unsigned char *)ptr + offset);
}

template <typename T>
inline T *neko_addoffset_r(std::ptrdiff_t offset, T *ptr) {
    if (!ptr) return nullptr;
    return (T *)((unsigned char *)ptr + offset);
}

template <typename T, size_t size>
class size_checker {
    static_assert(sizeof(T) == size, "check the size of integral types");
};

template class size_checker<i64, 8>;
template class size_checker<i32, 4>;
template class size_checker<i16, 2>;
template class size_checker<i8, 1>;
template class size_checker<u64, 8>;
template class size_checker<u32, 4>;
template class size_checker<u16, 2>;
template class size_checker<u8, 1>;

struct lua_State;

namespace neko {

template <typename V, typename Alloc = std::allocator<V>>
using vector = std::vector<V, Alloc>;

template <typename T>
struct cpp_remove_reference {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T &> {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T &&> {
    using type = T;
};

template <typename T>
constexpr typename cpp_remove_reference<T>::type &&cpp_move(T &&arg) noexcept {
    return (typename cpp_remove_reference<T>::type &&)arg;
}

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using function = std::function<T>;

template <typename T>
using scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr scope<T> create_scope(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr ref<T> create_ref(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
concept concept_is_pair = requires(T t) {
    t.first;
    t.second;
};

template <class T>
struct is_pair : public std::false_type {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};

template <class>
inline constexpr bool always_false = false;

}  // namespace neko

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

#define NEKO_VA_UNPACK(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

#if defined(__clang__)
#define NEKO_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
#define NEKO_DEBUGBREAK() __builtin_trap()
#elif defined(_MSC_VER)
#define NEKO_DEBUGBREAK() __debugbreak()
#else
#define NEKO_DEBUGBREAK()
#endif

namespace std {
inline void breakpoint() noexcept { NEKO_DEBUGBREAK(); }
inline bool is_debugger_present() noexcept {
#if defined(_WIN32)
    return IsDebuggerPresent() != 0;
#elif defined(__APPLE__)
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    info.kp_proc.p_flag = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) != 0) {
        return false;
    }
    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}
inline void breakpoint_if_debugging() noexcept {
    if (is_debugger_present()) {
        breakpoint();
    }
}
}  // namespace std

#endif

namespace neko {

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class timer {
public:
    inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] inline f64 get() const noexcept { return duration; }
    ~timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

static inline std::string fs_normalize_path(const std::string &path, char delimiter = '/') {
    static constexpr char delims[] = "/\\";

    std::string norm;
    norm.reserve(path.size() / 2);  // random guess, should be benchmarked

    for (auto it = path.begin(); it != path.end(); it++) {
        if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
            if (norm.empty() || norm.back() != delimiter) norm.push_back(delimiter);
        } else if (*it == '.') {
            if (++it == path.end()) break;
            if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
                continue;
            }
            if (*it != '.') throw std::logic_error("bad path");
            if (norm.empty() || norm.back() != delimiter) throw std::logic_error("bad path");

            norm.pop_back();
            while (!norm.empty()) {
                norm.pop_back();
                if (norm.back() == delimiter) {
                    norm.pop_back();
                    break;
                }
            }
        } else
            norm.push_back(*it);
    }
    if (!norm.empty() && norm.back() != delimiter) norm.push_back(delimiter);
    return norm;
}

}  // namespace neko

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace neko::wtf8

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
std::wstring a2w(std::string_view str) noexcept;
std::string w2a(std::wstring_view wstr) noexcept;
std::string a2u(std::string_view str) noexcept;
std::string u2a(std::string_view str) noexcept;
}  // namespace neko::win

template <typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    using return_type = ReturnType;
    using pointer = ReturnType (*)(Args...);
    using std_function = std::function<ReturnType(Args...)>;
};

template <typename F>
typename function_traits<F>::std_function to_function(F &lambda) {
    return typename function_traits<F>::std_function(lambda);
}

namespace neko {

struct format_str {
    constexpr format_str(const char *str) noexcept : str(str) {}
    const char *str;
};

template <format_str F>
constexpr auto operator""_f() {
    return [=]<typename... T>(T... args) { return std::format(F.str, args...); };
}

// 成员函数返回值类型确定
// https://stackoverflow.com/questions/26107041/how-can-i-determine-the-return-type-of-a-c11-member-function

template <typename T>
struct return_type;
template <typename R, typename... Args>
struct return_type<R (*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) volatile> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const volatile> {
    using type = R;
};
template <typename T>
using return_type_t = typename return_type<T>::type;

// std::function 合并方法

template <typename, typename...>
struct lastFnType;

template <typename F0, typename F1, typename... Fn>
struct lastFnType<F0, F1, Fn...> {
    using type = typename lastFnType<F1, Fn...>::type;
};

template <typename T1, typename T2>
struct lastFnType<function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
function<T1(T2)> func_combine(function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(function<T1(T2)> conv1, function<T2(T3)> conv2, Fn... fn) -> function<T1(typename lastFnType<function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<function<T2(T3)>, Fn...>::type;

    return [=](In const &in) { return conv1(func_combine(conv2, fn...)(in)); };
}

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

template <typename T, std::size_t N>
constexpr bool is_pointer_to_const_char(T (&)[N]) {
    return std::is_same_v<const char, T>;
}

template <typename T>
constexpr bool is_pointer_to_const_char(T &&) {
    return std::is_same_v<const char *, T>;
}

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

}  // namespace neko

namespace detail {
// 某些旧版本的 GCC 需要
template <typename...>
struct voider {
    using type = void;
};

// std::void_t 将成为 C++17 的一部分 但在这里我还是自己实现吧
template <typename... T>
using void_t = typename voider<T...>::type;

template <typename T, typename U = void>
struct is_mappish_impl : std::false_type {};

template <typename T>
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T &>()[std::declval<const typename T::key_type &>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

namespace neko {

// hash 计算相关函数

typedef unsigned long long hash_value;

static_assert(sizeof(hash_value) == 8 && sizeof(hash_value) == sizeof(size_t));

inline uint64_t hash_fnv(const void *data, int size) {
    const_str s = (const_str)data;
    uint64_t h = 14695981039346656037ULL;
    char c = 0;
    while (size--) {
        h = h ^ (uint64_t)(*s++);
        h = h * 1099511628211ULL;
    }
    return h;
}

// http://www.jstatsoft.org/article/view/v008i14/xorshift.pdf page 4
// https://en.wikipedia.org/wiki/Xorshift#xorshift
constexpr hash_value xor64(hash_value h) {
    h ^= 88172645463325252ULL;  // 与常数进行异或 因此种子 0 不会导致无限循环
    h ^= h >> 12;
    h ^= h << 25;
    h ^= h >> 27;
    return h * 0x2545F4914F6CDD1DULL;
}

// https://de.wikipedia.org/wiki/FNV_(Informatik)
constexpr hash_value hash_fnv(const_str string) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; *string; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr hash_value hash(char const *input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string &input) { return hash_fnv(input.c_str()); }

template <typename ForwardIterator, typename SpaceDetector>
constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector &&is_space_pred) {
    auto rend = std::reverse_iterator(begin);
    auto rbegin = std::reverse_iterator(end);

    int sp_size = 0;
    auto is_space = [&sp_size, &is_space_pred, &end](char c) {
        sp_size = is_space_pred(std::string_view{&c, static_cast<unsigned>(&*std::prev(end) - &c)});
        return sp_size > 0;
    };

    auto search = std::find_if(rbegin, rend, is_space);
    if (search == rend) {
        return begin;
    }
    ForwardIterator it = std::prev(search.base());
    it += sp_size;
    return it;
}

template <typename ForwardIt, typename OutputIt>
constexpr void copy(ForwardIt src_beg, ForwardIt src_end, OutputIt dest_beg, OutputIt dest_end) {
    while (src_beg != src_end && dest_beg != dest_end) {
        *dest_beg++ = *src_beg++;
    }
}

}  // namespace neko

typedef struct engine_cvar_t {
    bool show_editor;
    bool show_demo_window;
    bool show_pack_editor;
    bool show_profiler_window;
    bool show_test_window;
    bool show_gui;
    bool shader_inspect;
    bool hello_ai_shit;
    bool vsync;
    bool is_hotfix;

    // 实验性功能开关
    bool enable_nekolua;

    f32 bg[3];  //
} neko_client_cvar_t;

void neko_cvar_gui(neko_client_cvar_t &cvar);

namespace neko {}  // namespace neko

#endif

#ifndef NEKO_REFL_HPP
#define NEKO_REFL_HPP

#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// #include "engine/neko.h"
// #include "engine/engine.h"

namespace neko::reflection {
template <unsigned short N>
struct cstring {
    constexpr explicit cstring(std::string_view str) noexcept : cstring{str, std::make_integer_sequence<unsigned short, N>{}} {}
    constexpr const char *data() const noexcept { return chars_; }
    constexpr unsigned short size() const noexcept { return N; }
    constexpr operator std::string_view() const noexcept { return {data(), size()}; }
    template <unsigned short... I>
    constexpr cstring(std::string_view str, std::integer_sequence<unsigned short, I...>) noexcept : chars_{str[I]..., '\0'} {}
    char chars_[static_cast<size_t>(N) + 1];
};
template <>
struct cstring<0> {
    constexpr explicit cstring(std::string_view) noexcept {}
    constexpr const char *data() const noexcept { return nullptr; }
    constexpr unsigned short size() const noexcept { return 0; }
    constexpr operator std::string_view() const noexcept { return {}; }
};
template <typename T>
constexpr auto name_raw() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    std::string_view name = __PRETTY_FUNCTION__;
    size_t start = name.find('=') + 2;
    size_t end = name.size() - 1;
    return std::string_view{name.data() + start, end - start};
#elif defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
    size_t start = name.find('<') + 1;
    size_t end = name.rfind(">(");
    name = std::string_view{name.data() + start, end - start};
    start = name.find(' ');
    return start == std::string_view::npos ? name : std::string_view{name.data() + start + 1, name.size() - start - 1};
#else
#error Unsupported compiler
#endif
}

template <typename T>
constexpr auto name() noexcept {
    constexpr auto name = name_raw<T>();
    return cstring<name.size()>{name};
}
template <typename T>
constexpr auto name_v = name<T>();

template <auto value>
constexpr auto enum_name() {
    std::string_view name;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    auto start = name.find("value = ") + 8;  // 8 is length of "value = "
    auto end = name.find_last_of(']');
    return std::string_view{name.data() + start, end - start};

#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    auto start = name.find("value = ") + 8;  // 8 is length of "value = "
    auto end = name.find_last_of(']');
    return std::string_view{name.data() + start, end - start};

#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    auto start = name.find("neko::enum_name<") + 16;  // 16 is length of "neko::enum_name<"
    auto end = name.find_last_of('>');
    return std::string_view{name.data() + start, end - start};
#endif
}

template <typename T>
concept enum_check = std::is_enum_v<T>;

// 获取枚举变量数量
template <enum_check T, std::size_t N = 0>
constexpr auto enum_max() {
    constexpr auto value = static_cast<T>(N);
    if constexpr (neko::reflection::enum_name<value>().find("(") == std::string_view::npos)  // 如果超出了连续有名枚举 将会是"(enum the_name)0xN"
        return neko::reflection::enum_max<T, N + 1>();
    else
        return N;
}

// 打表
template <typename T>
    requires std::is_enum_v<T>
constexpr auto enum_name(T value) {
    constexpr auto num = neko::reflection::enum_max<T>();
    constexpr std::array<std::string_view, num> names{[]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::array<std::string_view, num>{neko::reflection::enum_name<static_cast<T>(Is)>()...};
    }(std::make_index_sequence<num>{})};  // 打表获得枚举名称
    return names[static_cast<std::size_t>(value)];
}

}  // namespace neko::reflection

namespace neko::reflection {

struct Type;

class Any {
    Type *type;    // type info, similar to vtable
    void *data;    // pointer to the data
    uint8_t flag;  // special flag

public:
    Any() : type(nullptr), data(nullptr), flag(0) {}

    Any(Type *type, void *data) : type(type), data(data), flag(0B00000001) {}

    Any(const Any &other);
    Any(Any &&other);
    ~Any();

    template <typename T>
    Any(T &&value);  // box value to Any

    template <typename T>
    T &cast();  // unbox Any to value

    Type *GetType() const { return type; }  // get type info

    Any invoke(std::string_view name, std::span<Any> args);  // call method

    void foreach (const std::function<void(std::string_view, Any &)> &fn);  // iterate fields
};

struct Type {
    std::string_view name;        // type name
    void (*destroy)(void *);      // destructor
    void *(*copy)(const void *);  // copy constructor
    void *(*move)(void *);        // move constructor

    using Field = std::pair<Type *, std::size_t>;          // type and offset
    using Method = Any (*)(void *, std::span<Any>);        // method
    std::unordered_map<std::string_view, Field> fields;    // field info
    std::unordered_map<std::string_view, Method> methods;  // method info
};

template <typename T>
Type *type_of();  // type_of<T> returns type info of T

template <typename T>
T &Any::cast() {
    if (type != type_of<T>()) {
        throw std::runtime_error{"type mismatch"};
    }
    return *static_cast<T *>(data);
}

template <typename T>
struct member_fn_traits;

template <typename R, typename C, typename... Args>
struct member_fn_traits<R (C::*)(Args...)> {
    using return_type = R;
    using class_type = C;
    using args_type = std::tuple<Args...>;
};

template <auto ptr>
auto *type_ensure() {
    using traits = member_fn_traits<decltype(ptr)>;
    using class_type = typename traits::class_type;
    using result_type = typename traits::return_type;
    using args_type = typename traits::args_type;

    return +[](void *object, std::span<Any> args) -> Any {
        auto self = static_cast<class_type *>(object);
        return [=]<std::size_t... Is>(std::index_sequence<Is...>) {
            if constexpr (std::is_void_v<result_type>) {
                (self->*ptr)(args[Is].cast<std::tuple_element_t<Is, args_type>>()...);
                return Any{};
            } else {
                return Any{(self->*ptr)(args[Is].cast<std::tuple_element_t<Is, args_type>>()...)};
            }
        }(std::make_index_sequence<std::tuple_size_v<args_type>>{});
    };
}

// template <typename T>
// Type* type_of() {
//     static Type type;
//     type.name = typeid(T).name();
//     type.destroy = [](void* obj) { delete static_cast<T*>(obj); };
//     type.copy = [](const void* obj) { return (void*)(new T(*static_cast<const T*>(obj))); };
//     type.move = [](void* obj) { return (void*)(new T(std::move(*static_cast<T*>(obj)))); };
//     return &type;
// }

inline Any::Any(const Any &other) {
    type = other.type;
    data = type->copy(other.data);
    flag = 0;
}

inline Any::Any(Any &&other) {
    type = other.type;
    data = type->move(other.data);
    flag = 0;
}

template <typename T>
Any::Any(T &&value) {
    type = type_of<std::decay_t<T>>();
    data = new std::decay_t<T>(std::forward<T>(value));
    flag = 0;
}

inline Any::~Any() {
    if (!(flag & 0B00000001) && data && type) {
        type->destroy(data);
    }
}

inline void Any::foreach (const std::function<void(std::string_view, Any &)> &fn) {
    for (auto &[name, field] : type->fields) {
        Any any = Any{field.first, static_cast<char *>(data) + field.second};
        fn(name, any);
    }
}

inline Any Any::invoke(std::string_view name, std::span<Any> args) {
    auto it = type->methods.find(name);
    if (it == type->methods.end()) {
        throw std::runtime_error{"method not found"};
    }
    return it->second(data, args);
}

}  // namespace neko::reflection

#define REGISTER_TYPE_DF(C, ...)                                                                   \
    namespace neko::reflection {                                                                   \
    template <>                                                                                    \
    Type *type_of<C>() {                                                                           \
        static Type type;                                                                          \
        type.name = #C;                                                                            \
        type.destroy = [](void *obj) { delete static_cast<C *>(obj); };                            \
        type.copy = [](const void *obj) { return (void *)(new C(*static_cast<const C *>(obj))); }; \
        type.move = [](void *obj) { return (void *)(new C(std::move(*static_cast<C *>(obj)))); };  \
        __VA_ARGS__                                                                                \
        return &type;                                                                              \
    };                                                                                             \
    }

namespace neko::reflection {

struct __any {
    // 无定义 我们需要一个可以转换为任何类型的在以下特殊语境中使用的辅助类
    template <typename T>
    constexpr operator T() const;
};

// 计算结构体成员数量
#if !defined(_MSC_VER) || 0  // 我不知道为什么 if constexpr (!requires { T{Args...}; }) {...} 方法会导致目前版本的vs代码感知非常卡

template <typename T>
consteval size_t struct_size(auto &&...Args) {
    if constexpr (!requires { T{Args...}; }) {
        return sizeof...(Args) - 1;
    } else {
        return struct_size<T>(Args..., __any{});
    }
}

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>()> {};

#else

template <typename T, typename = void, typename... Ts>
struct struct_size {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename... Ts>
struct struct_size<T, std::void_t<decltype(T{Ts{}...})>, Ts...> {
    constexpr static size_t value = struct_size<T, void, Ts..., __any>::value;
};

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>::value> {};

#endif

// struct_apply 把结构体解包为变长参数调用可调用对象ApplyFunc
template <class T, class F>
auto struct_apply(T &&_struct, F f) {
    return __struct_apply_impl(std::forward<T>(_struct), f, struct_member_count<typename std::decay<T>::type>());
}

// StructTransformMeta 把结构体各成员的类型作为变长参数调用元函数 F
template <class T, template <class...> class F>
struct struct_transform_meta {
    struct __fake_applyer {
        template <class... Args>
        auto operator()(Args... args) -> F<decltype(args)...>;
    };
    using type = decltype(struct_apply(std::declval<T>(), __fake_applyer()));
};

template <typename T>
inline constexpr auto __gen_struct_meta() {
    return std::make_tuple();
}

#define NEKO_STRUCT(__struct, ...)                                          \
    template <>                                                             \
    inline constexpr auto neko::reflection::__gen_struct_meta<__struct>() { \
        using T = __struct;                                                 \
        return std::make_tuple(__VA_ARGS__);                                \
    };

// 这里_F*用于生成__gen_struct_meta内tuple叠入的tuple
#define _F(field) (std::make_tuple(#field, &T::field))
#define _Fs(field, ...) (std::make_tuple(#field, &T::field, std::make_tuple(__VA_ARGS__)))

template <typename T, typename Fields, typename F, size_t... Is>
inline constexpr void struct_foreach_impl(T &&obj, Fields &&fields, F &&f, std::index_sequence<Is...>) {
    auto ff = [&](auto index) {
        auto &t = std::get<index>(fields);
        constexpr static std::size_t t_size = std::tuple_size_v<std::decay_t<decltype(t)>>;
        if constexpr (t_size == 3)
            std::apply([&](auto &&arg1, auto &&arg2, auto &&info) { f(arg1, obj.*arg2, info); }, t);
        else if constexpr (t_size == 2)
            std::apply([&](auto &&arg1, auto &&arg2) { f(arg1, obj.*arg2, std::make_tuple("default")); }, t);
        // std::cout << "^^ 傻逼 " << std::tuple_size_v<std::decay_t<decltype(fields)>> << std::endl;
    };
    // 逗号双层表达式 因为ff没有返回值则Is作为里层逗号表达式的结果
    auto _ = ((ff(std::integral_constant<size_t, Is>{}), Is), ...);
    std::ignore = _;
}

template <typename T, typename F>
inline constexpr void struct_foreach(T &&obj, F &&f) {
    // 获取宏生成的元数据 tuple
    constexpr auto fields = __gen_struct_meta<std::decay_t<T>>();
    // 调用 neko_struct_foreach_impl 函数 并传递 obj/fields/f
    // std::make_index_sequence 来确认范围
    struct_foreach_impl(std::forward<T>(obj), fields, std::forward<F>(f), std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{});
}

#define STRUCT_APPLYER_DEF(N)                                                              \
    template <class T, class F>                                                            \
    auto __struct_apply_impl(T &&my_struct, F f, std::integral_constant<std::size_t, N>) { \
        auto &&[NEKO_PP_PARAMS(x, N)] = std::forward<T>(my_struct);                        \
        return std::invoke(f, NEKO_PP_PARAMS(x, N));                                       \
    }

#if (!defined NEKO_PP_FOR_EACH || !defined NEKO_PP_PARAMS)

// 横向迭代专用，NEKO_PP_PARAMS(x, 3) => x1, x2, x3

#define NEKO_PP_PARAMS_0(x)
#define NEKO_PP_PARAMS_1(x) x##1
#define NEKO_PP_PARAMS_2(x) NEKO_PP_PARAMS_1(x), x##2
#define NEKO_PP_PARAMS_3(x) NEKO_PP_PARAMS_2(x), x##3
#define NEKO_PP_PARAMS_4(x) NEKO_PP_PARAMS_3(x), x##4
#define NEKO_PP_PARAMS_5(x) NEKO_PP_PARAMS_4(x), x##5
#define NEKO_PP_PARAMS_6(x) NEKO_PP_PARAMS_5(x), x##6
#define NEKO_PP_PARAMS_7(x) NEKO_PP_PARAMS_6(x), x##7
#define NEKO_PP_PARAMS_8(x) NEKO_PP_PARAMS_7(x), x##8
#define NEKO_PP_PARAMS_9(x) NEKO_PP_PARAMS_8(x), x##9
#define NEKO_PP_PARAMS_10(x) NEKO_PP_PARAMS_9(x), x##10
#define NEKO_PP_PARAMS_11(x) NEKO_PP_PARAMS_10(x), x##11
#define NEKO_PP_PARAMS_12(x) NEKO_PP_PARAMS_11(x), x##12
#define NEKO_PP_PARAMS_13(x) NEKO_PP_PARAMS_12(x), x##13
#define NEKO_PP_PARAMS_14(x) NEKO_PP_PARAMS_13(x), x##14
#define NEKO_PP_PARAMS_15(x) NEKO_PP_PARAMS_14(x), x##15
#define NEKO_PP_PARAMS_16(x) NEKO_PP_PARAMS_15(x), x##16
#define NEKO_PP_PARAMS_17(x) NEKO_PP_PARAMS_16(x), x##17
#define NEKO_PP_PARAMS_18(x) NEKO_PP_PARAMS_17(x), x##18
#define NEKO_PP_PARAMS_19(x) NEKO_PP_PARAMS_18(x), x##19
#define NEKO_PP_PARAMS_20(x) NEKO_PP_PARAMS_19(x), x##20
#define NEKO_PP_PARAMS_21(x) NEKO_PP_PARAMS_20(x), x##21
#define NEKO_PP_PARAMS_22(x) NEKO_PP_PARAMS_21(x), x##22
#define NEKO_PP_PARAMS_23(x) NEKO_PP_PARAMS_22(x), x##23
#define NEKO_PP_PARAMS_24(x) NEKO_PP_PARAMS_23(x), x##24
#define NEKO_PP_PARAMS_25(x) NEKO_PP_PARAMS_24(x), x##25
#define NEKO_PP_PARAMS_26(x) NEKO_PP_PARAMS_25(x), x##26
#define NEKO_PP_PARAMS_27(x) NEKO_PP_PARAMS_26(x), x##27
#define NEKO_PP_PARAMS_28(x) NEKO_PP_PARAMS_27(x), x##28
#define NEKO_PP_PARAMS_29(x) NEKO_PP_PARAMS_28(x), x##29
#define NEKO_PP_PARAMS_30(x) NEKO_PP_PARAMS_29(x), x##30
#define NEKO_PP_PARAMS_31(x) NEKO_PP_PARAMS_30(x), x##31
#define NEKO_PP_PARAMS_32(x) NEKO_PP_PARAMS_31(x), x##32
#define NEKO_PP_PARAMS_33(x) NEKO_PP_PARAMS_32(x), x##33
#define NEKO_PP_PARAMS_34(x) NEKO_PP_PARAMS_33(x), x##34
#define NEKO_PP_PARAMS_35(x) NEKO_PP_PARAMS_34(x), x##35
#define NEKO_PP_PARAMS_36(x) NEKO_PP_PARAMS_35(x), x##36
#define NEKO_PP_PARAMS_37(x) NEKO_PP_PARAMS_36(x), x##37
#define NEKO_PP_PARAMS_38(x) NEKO_PP_PARAMS_37(x), x##38
#define NEKO_PP_PARAMS_39(x) NEKO_PP_PARAMS_38(x), x##39
#define NEKO_PP_PARAMS_40(x) NEKO_PP_PARAMS_39(x), x##40
#define NEKO_PP_PARAMS_41(x) NEKO_PP_PARAMS_40(x), x##41
#define NEKO_PP_PARAMS_42(x) NEKO_PP_PARAMS_41(x), x##42
#define NEKO_PP_PARAMS_43(x) NEKO_PP_PARAMS_42(x), x##43
#define NEKO_PP_PARAMS_44(x) NEKO_PP_PARAMS_43(x), x##44
#define NEKO_PP_PARAMS_45(x) NEKO_PP_PARAMS_44(x), x##45
#define NEKO_PP_PARAMS_46(x) NEKO_PP_PARAMS_45(x), x##46
#define NEKO_PP_PARAMS_47(x) NEKO_PP_PARAMS_46(x), x##47
#define NEKO_PP_PARAMS_48(x) NEKO_PP_PARAMS_47(x), x##48
#define NEKO_PP_PARAMS_49(x) NEKO_PP_PARAMS_48(x), x##49
#define NEKO_PP_PARAMS_50(x) NEKO_PP_PARAMS_49(x), x##50
#define NEKO_PP_PARAMS_51(x) NEKO_PP_PARAMS_50(x), x##51
#define NEKO_PP_PARAMS_52(x) NEKO_PP_PARAMS_51(x), x##52
#define NEKO_PP_PARAMS_53(x) NEKO_PP_PARAMS_52(x), x##53
#define NEKO_PP_PARAMS_54(x) NEKO_PP_PARAMS_53(x), x##54
#define NEKO_PP_PARAMS_55(x) NEKO_PP_PARAMS_54(x), x##55
#define NEKO_PP_PARAMS_56(x) NEKO_PP_PARAMS_55(x), x##56
#define NEKO_PP_PARAMS_57(x) NEKO_PP_PARAMS_56(x), x##57
#define NEKO_PP_PARAMS_58(x) NEKO_PP_PARAMS_57(x), x##58
#define NEKO_PP_PARAMS_59(x) NEKO_PP_PARAMS_58(x), x##59
#define NEKO_PP_PARAMS_60(x) NEKO_PP_PARAMS_59(x), x##60
#define NEKO_PP_PARAMS_61(x) NEKO_PP_PARAMS_60(x), x##61
#define NEKO_PP_PARAMS_62(x) NEKO_PP_PARAMS_61(x), x##62
#define NEKO_PP_PARAMS_63(x) NEKO_PP_PARAMS_62(x), x##63

#define NEKO_PP_PARAMS(x, N) NEKO_PP_PARAMS_##N(x)

// 纵向迭代专用

#define NEKO_PP_FOR_EACH_0(x)
#define NEKO_PP_FOR_EACH_1(x) NEKO_PP_FOR_EACH_0(x) x(1)
#define NEKO_PP_FOR_EACH_2(x) NEKO_PP_FOR_EACH_1(x) x(2)
#define NEKO_PP_FOR_EACH_3(x) NEKO_PP_FOR_EACH_2(x) x(3)
#define NEKO_PP_FOR_EACH_4(x) NEKO_PP_FOR_EACH_3(x) x(4)
#define NEKO_PP_FOR_EACH_5(x) NEKO_PP_FOR_EACH_4(x) x(5)
#define NEKO_PP_FOR_EACH_6(x) NEKO_PP_FOR_EACH_5(x) x(6)
#define NEKO_PP_FOR_EACH_7(x) NEKO_PP_FOR_EACH_6(x) x(7)
#define NEKO_PP_FOR_EACH_8(x) NEKO_PP_FOR_EACH_7(x) x(8)
#define NEKO_PP_FOR_EACH_9(x) NEKO_PP_FOR_EACH_8(x) x(9)
#define NEKO_PP_FOR_EACH_10(x) NEKO_PP_FOR_EACH_9(x) x(10)
#define NEKO_PP_FOR_EACH_11(x) NEKO_PP_FOR_EACH_10(x) x(11)
#define NEKO_PP_FOR_EACH_12(x) NEKO_PP_FOR_EACH_11(x) x(12)
#define NEKO_PP_FOR_EACH_13(x) NEKO_PP_FOR_EACH_12(x) x(13)
#define NEKO_PP_FOR_EACH_14(x) NEKO_PP_FOR_EACH_13(x) x(14)
#define NEKO_PP_FOR_EACH_15(x) NEKO_PP_FOR_EACH_14(x) x(15)
#define NEKO_PP_FOR_EACH_16(x) NEKO_PP_FOR_EACH_15(x) x(16)
#define NEKO_PP_FOR_EACH_17(x) NEKO_PP_FOR_EACH_16(x) x(17)
#define NEKO_PP_FOR_EACH_18(x) NEKO_PP_FOR_EACH_17(x) x(18)
#define NEKO_PP_FOR_EACH_19(x) NEKO_PP_FOR_EACH_18(x) x(19)
#define NEKO_PP_FOR_EACH_20(x) NEKO_PP_FOR_EACH_19(x) x(20)
#define NEKO_PP_FOR_EACH_21(x) NEKO_PP_FOR_EACH_20(x) x(21)
#define NEKO_PP_FOR_EACH_22(x) NEKO_PP_FOR_EACH_21(x) x(22)
#define NEKO_PP_FOR_EACH_23(x) NEKO_PP_FOR_EACH_22(x) x(23)
#define NEKO_PP_FOR_EACH_24(x) NEKO_PP_FOR_EACH_23(x) x(24)
#define NEKO_PP_FOR_EACH_25(x) NEKO_PP_FOR_EACH_24(x) x(25)
#define NEKO_PP_FOR_EACH_26(x) NEKO_PP_FOR_EACH_25(x) x(26)
#define NEKO_PP_FOR_EACH_27(x) NEKO_PP_FOR_EACH_26(x) x(27)
#define NEKO_PP_FOR_EACH_28(x) NEKO_PP_FOR_EACH_27(x) x(28)
#define NEKO_PP_FOR_EACH_29(x) NEKO_PP_FOR_EACH_28(x) x(29)
#define NEKO_PP_FOR_EACH_30(x) NEKO_PP_FOR_EACH_29(x) x(30)
#define NEKO_PP_FOR_EACH_31(x) NEKO_PP_FOR_EACH_30(x) x(31)
#define NEKO_PP_FOR_EACH_32(x) NEKO_PP_FOR_EACH_31(x) x(32)
#define NEKO_PP_FOR_EACH_33(x) NEKO_PP_FOR_EACH_32(x) x(33)
#define NEKO_PP_FOR_EACH_34(x) NEKO_PP_FOR_EACH_33(x) x(34)
#define NEKO_PP_FOR_EACH_35(x) NEKO_PP_FOR_EACH_34(x) x(35)
#define NEKO_PP_FOR_EACH_36(x) NEKO_PP_FOR_EACH_35(x) x(36)
#define NEKO_PP_FOR_EACH_37(x) NEKO_PP_FOR_EACH_36(x) x(37)
#define NEKO_PP_FOR_EACH_38(x) NEKO_PP_FOR_EACH_37(x) x(38)
#define NEKO_PP_FOR_EACH_39(x) NEKO_PP_FOR_EACH_38(x) x(39)
#define NEKO_PP_FOR_EACH_40(x) NEKO_PP_FOR_EACH_39(x) x(40)
#define NEKO_PP_FOR_EACH_41(x) NEKO_PP_FOR_EACH_40(x) x(41)
#define NEKO_PP_FOR_EACH_42(x) NEKO_PP_FOR_EACH_41(x) x(42)
#define NEKO_PP_FOR_EACH_43(x) NEKO_PP_FOR_EACH_42(x) x(43)
#define NEKO_PP_FOR_EACH_44(x) NEKO_PP_FOR_EACH_43(x) x(44)
#define NEKO_PP_FOR_EACH_45(x) NEKO_PP_FOR_EACH_44(x) x(45)
#define NEKO_PP_FOR_EACH_46(x) NEKO_PP_FOR_EACH_45(x) x(46)
#define NEKO_PP_FOR_EACH_47(x) NEKO_PP_FOR_EACH_46(x) x(47)
#define NEKO_PP_FOR_EACH_48(x) NEKO_PP_FOR_EACH_47(x) x(48)
#define NEKO_PP_FOR_EACH_49(x) NEKO_PP_FOR_EACH_48(x) x(49)
#define NEKO_PP_FOR_EACH_50(x) NEKO_PP_FOR_EACH_49(x) x(50)
#define NEKO_PP_FOR_EACH_51(x) NEKO_PP_FOR_EACH_50(x) x(51)
#define NEKO_PP_FOR_EACH_52(x) NEKO_PP_FOR_EACH_51(x) x(52)
#define NEKO_PP_FOR_EACH_53(x) NEKO_PP_FOR_EACH_52(x) x(53)
#define NEKO_PP_FOR_EACH_54(x) NEKO_PP_FOR_EACH_53(x) x(54)
#define NEKO_PP_FOR_EACH_55(x) NEKO_PP_FOR_EACH_54(x) x(55)
#define NEKO_PP_FOR_EACH_56(x) NEKO_PP_FOR_EACH_55(x) x(56)
#define NEKO_PP_FOR_EACH_57(x) NEKO_PP_FOR_EACH_56(x) x(57)
#define NEKO_PP_FOR_EACH_58(x) NEKO_PP_FOR_EACH_57(x) x(58)
#define NEKO_PP_FOR_EACH_59(x) NEKO_PP_FOR_EACH_58(x) x(59)
#define NEKO_PP_FOR_EACH_60(x) NEKO_PP_FOR_EACH_59(x) x(60)
#define NEKO_PP_FOR_EACH_61(x) NEKO_PP_FOR_EACH_60(x) x(61)
#define NEKO_PP_FOR_EACH_62(x) NEKO_PP_FOR_EACH_61(x) x(62)
#define NEKO_PP_FOR_EACH_63(x) NEKO_PP_FOR_EACH_62(x) x(63)

#define NEKO_PP_FOR_EACH(x, N) NEKO_PP_FOR_EACH_##N(x)

#endif

// 使用宏打表 STRUCT_APPLYER_DEF 内使用结构化绑定得到结构体成员序列
NEKO_PP_FOR_EACH(STRUCT_APPLYER_DEF, 63)

}  // namespace neko::reflection

#endif  // NEKO_ENGINE_NEKO_REFL_HPP
