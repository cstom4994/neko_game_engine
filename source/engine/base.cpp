#include "engine/base.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <new>

#include "engine/api.hpp"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/game.h"
#include "engine/glew_glfw.h"
#include "engine/luax.hpp"
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

SplitLinesIterator &SplitLinesIterator::operator++() {
    if (&view.data[view.len] == &data.data[data.len]) {
        view = {&data.data[data.len], 0};
        return *this;
    }

    String next = {};
    next.data = view.data + view.len + 1;

    u64 end = 0;
    while (&next.data[end] < &data.data[data.len] && next.data[end] != '\n' && next.data[end] != 0) {
        end++;
    }
    next.len = end;

    view = next;
    return *this;
}

bool operator!=(SplitLinesIterator lhs, SplitLinesIterator rhs) {
    return lhs.data.data != rhs.data.data || lhs.data.len != rhs.data.len || lhs.view.data != rhs.view.data || lhs.view.len != rhs.view.len;
}

SplitLinesIterator SplitLines::begin() {
    char *data = str.data;
    u64 end = 0;
    while (data[end] != '\n' && data[end] != 0) {
        end++;
    }

    String view = {str.data, end};
    return {str, view};
}

SplitLinesIterator SplitLines::end() {
    String view = {str.data + str.len, 0};
    return {str, view};
}

i32 utf8_size(u8 c) {
    if (c == '\0') {
        return 0;
    }

    if ((c & 0xF8) == 0xF0) {
        return 4;
    } else if ((c & 0xF0) == 0xE0) {
        return 3;
    } else if ((c & 0xE0) == 0xC0) {
        return 2;
    } else {
        return 1;
    }
}

u32 Rune::charcode() {
    u32 charcode = 0;

    u8 c0 = value >> 0;
    u8 c1 = value >> 8;
    u8 c2 = value >> 16;
    u8 c3 = value >> 24;

    switch (utf8_size(c0)) {
        case 1:
            charcode = c0;
            break;
        case 2:
            charcode = c0 & 0x1F;
            charcode = (charcode << 6) | (c1 & 0x3F);
            break;
        case 3:
            charcode = c0 & 0x0F;
            charcode = (charcode << 6) | (c1 & 0x3F);
            charcode = (charcode << 6) | (c2 & 0x3F);
            break;
        case 4:
            charcode = c0 & 0x07;
            charcode = (charcode << 6) | (c1 & 0x3F);
            charcode = (charcode << 6) | (c2 & 0x3F);
            charcode = (charcode << 6) | (c3 & 0x3F);
            break;
    }

    return charcode;
}

bool Rune::is_whitespace() {
    switch (value) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

bool Rune::is_digit() { return value >= '0' && value <= '9'; }

Rune rune_from_string(const char *data) {
    u32 rune = 0;
    i32 len = utf8_size(data[0]);
    for (i32 i = len - 1; i >= 0; i--) {
        rune <<= 8;
        rune |= (u8)(data[i]);
    }

    return {rune};
}

static void next_rune(UTF8Iterator *it) {
    if (it->cursor == it->str.len) {
        it->rune.value = 0;
        return;
    }

    char *data = &it->str.data[it->cursor];
    i32 len = utf8_size(data[0]);
    Rune rune = rune_from_string(data);

    it->cursor += len;
    it->rune = rune;
}

UTF8Iterator &UTF8Iterator::operator++() {
    next_rune(this);
    return *this;
}

bool operator!=(UTF8Iterator lhs, UTF8Iterator rhs) { return lhs.str.data != rhs.str.data || lhs.str.len != rhs.str.len || lhs.cursor != rhs.cursor || lhs.rune.value != rhs.rune.value; }

UTF8Iterator UTF8::begin() {
    UTF8Iterator it = {};
    it.str = str;
    next_rune(&it);
    return it;
}

UTF8Iterator UTF8::end() { return {str, str.len, {}}; }

static char s_empty[1] = {0};

StringBuilder::StringBuilder() {
    data = s_empty;
    len = 0;
    capacity = 0;
}

void StringBuilder::trash() {
    if (data != s_empty) {
        mem_free(data);
    }
}

void StringBuilder::reserve(u64 cap) {
    if (cap > capacity) {
        char *buf = (char *)mem_alloc(cap);
        memset(buf, 0, cap);
        memcpy(buf, data, len);

        if (data != s_empty) {
            mem_free(data);
        }

        data = buf;
        capacity = cap;
    }
}

void StringBuilder::clear() {
    len = 0;
    if (data != s_empty) {
        data[0] = 0;
    }
}

void StringBuilder::swap_filename(String filepath, String file) {
    clear();

    u64 slash = filepath.last_of('/');
    if (slash != (u64)-1) {
        String path = filepath.substr(0, slash + 1);
        *this << path;
    }

    *this << file;
}

void StringBuilder::concat(String str, i32 times) {
    for (i32 i = 0; i < times; i++) {
        *this << str;
    }
}

StringBuilder &StringBuilder::operator<<(String str) {
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

StringBuilder::operator String() { return {data, len}; }

String str_fmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len > 0) {
        char *data = (char *)mem_alloc(len + 1);
        va_start(args, fmt);
        vsnprintf(data, len + 1, fmt, args);
        va_end(args);
        return {data, (u64)len};
    }

    return {};
}

String tmp_fmt(const char *fmt, ...) {
    static char s_buf[1024] = {};

    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
    va_end(args);
    return {s_buf, (u64)len};
}

double string_to_double(String str) {
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

struct ArenaNode {
    ArenaNode *next;
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

static ArenaNode *arena_block_make(u64 capacity) {
    u64 page = 4096 - offsetof(ArenaNode, buf);
    if (capacity < page) {
        capacity = page;
    }

    ArenaNode *a = (ArenaNode *)mem_alloc(offsetof(ArenaNode, buf[capacity]));
    a->next = nullptr;
    a->allocd = 0;
    a->capacity = capacity;
    return a;
}

void Arena::trash() {
    ArenaNode *a = head;
    while (a != nullptr) {
        ArenaNode *rm = a;
        a = a->next;
        mem_free(rm);
    }
}

void *Arena::bump(u64 size) {
    if (head == nullptr) {
        head = arena_block_make(size);
    }

    u64 next = 0;
    do {
        next = align_forward(head->allocd, 16);
        if (next + size <= head->capacity) {
            break;
        }

        ArenaNode *block = arena_block_make(size);
        block->next = head;

        head = block;
    } while (true);

    void *ptr = &head->buf[next];
    head->allocd = next + size;
    head->prev = next;
    return ptr;
}

void *Arena::rebump(void *ptr, u64 old, u64 size) {
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

String Arena::bump_string(String s) {
    if (s.len > 0) {
        char *cstr = (char *)bump(s.len + 1);
        memcpy(cstr, s.data, s.len);
        cstr[s.len] = '\0';
        return {cstr, s.len};
    } else {
        return {};
    }
}

Scanner::Scanner(String str) {
    data = str.data;
    len = str.len;
    pos = 0;
    end = 0;
}

static void advance(Scanner *s) { s->end += utf8_size(s->data[s->end]); }
static bool at_end(Scanner *s) { return s->end >= s->len; }

static Rune peek(Scanner *s) {
    if (at_end(s)) {
        return {0};
    } else {
        return rune_from_string(&s->data[s->end]);
    }
}

static void skip_whitespace(Scanner *s) {
    while (peek(s).is_whitespace() && !at_end(s)) {
        advance(s);
    }
}

String Scanner::next_string() {
    skip_whitespace(this);
    pos = end;

    if (at_end(this)) {
        return "";
    }

    while (!peek(this).is_whitespace() && !at_end(this)) {
        advance(this);
    }

    return {&data[pos], end - pos};
}

i32 Scanner::next_int() {
    skip_whitespace(this);
    pos = end;

    if (at_end(this)) {
        return 0;
    }

    i32 sign = 1;
    if (peek(this).value == '-') {
        sign = -1;
        advance(this);
    }

    i32 num = 0;
    while (peek(this).is_digit()) {
        num *= 10;
        num += peek(this).value - '0';
        advance(this);
    }

    return num * sign;
}

/*========================
// os
========================*/

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

void Cond::wait(Mutex *mtx) { SleepConditionVariableSRW(&cv, &mtx->srwlock, INFINITE, 0); }

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) { return SleepConditionVariableSRW(&cv, &mtx->srwlock, ms, 0); }

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

void Thread::make(ThreadProc fn, void *udata) {
    DWORD id = 0;
    HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, udata, 0, &id);
    ptr = (void *)handle;
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
void Cond::wait(Mutex *mtx) { pthread_cond_wait(&pt, &mtx->pt); }

bool Cond::timed_wait(Mutex *mtx, uint32_t ms) {
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
    sem = (sem_t *)mem_alloc(sizeof(sem_t));
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

void Thread::make(ThreadProc fn, void *udata) {
    pthread_t pt = {};
    pthread_create(&pt, nullptr, (void *(*)(void *))fn, udata);
    ptr = (void *)pt;
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

void *DebugAllocator::alloc(size_t bytes, const char *file, i32 line) {
    LockGuard lock{&mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)::malloc(offsetof(DebugAllocInfo, buf[bytes]));
    neko_assert(info, "FAILED_TO_ALLOCATE");
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

void *DebugAllocator::realloc(void *ptr, size_t new_size, const char *file, i32 line) {
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

    DebugAllocInfo *old_info = (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

    alloc_size -= old_info->size;

    // 分配新大小的新内存块
    DebugAllocInfo *new_info = (DebugAllocInfo *)::malloc(NEKO_OFFSET(DebugAllocInfo, buf[new_size]));
    neko_assert(new_info, "FAILED_TO_ALLOCATE");
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

void DebugAllocator::free(void *ptr) {
    if (ptr == nullptr) {
        return;
    }

    LockGuard lock{&mtx};

    DebugAllocInfo *info = (DebugAllocInfo *)((u8 *)ptr - offsetof(DebugAllocInfo, buf));

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

void DebugAllocator::dump_allocs(bool detailed) {
    i32 allocs = 0;
    for (DebugAllocInfo *info = head; info != nullptr; info = info->next) {
        if (detailed) printf("  %10llu bytes: %s:%d\n", (unsigned long long)info->size, info->file, info->line);
        allocs++;
    }
    neko_println("  --- leaks %d allocation(s) with %lld bytes ---", allocs, alloc_size);
}

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

i32 neko_os_file_size_in_bytes(const char *file_path) {
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

void neko_os_file_extension(char *buffer, size_t buffer_sz, const char *file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

i32 neko_os_file_delete(const char *file_path) {
#if (defined NEKO_IS_WIN32)

    // Non-zero if successful
    return DeleteFileA(file_path);

#elif (defined NEKO_IS_LINUX || defined NEKO_IS_APPLE || defined NEKO_IS_ANDROID)

    // Returns 0 if successful
    return !remove(file_path);

#endif

    return 0;
}

i32 neko_os_file_copy(const char *src_path, const char *dst_path) {
#if (defined NEKO_IS_WIN32)

    return CopyFileA(src_path, dst_path, false);

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
    if (GetFileAttributesExA(file_path, GetFileExInfoStandard, &data)) {
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
    return (void *)LoadLibraryA(lib_path);
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

void neko_log(const char *file, int line, const char *fmt, ...) {

    LockGuard lock{&g_app->log_mtx};

    typedef struct {
        va_list ap;
        const char *fmt;
        const char *file;
        u32 time;
        FILE *udata;
        int line;
    } neko_log_event;

    static auto init_event = [](neko_log_event *ev, void *udata) {
        static u32 t = 0;
        if (!ev->time) {
            ev->time = ++t;
        }
        ev->udata = (FILE *)udata;
    };

    neko_log_event ev = {
            .fmt = fmt,
            .file = file,
            .line = line,
    };

    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    // fprintf(ev.udata, "%s:%d: ", neko_util_get_filename(ev.file), ev.line);
    vfprintf(ev.udata, ev.fmt, ev.ap);
    fprintf(ev.udata, "\n");
    fflush(ev.udata);
    va_end(ev.ap);
}

static void _error(const char *s) { script_error(s); }

void errorf(const char *fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char *s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char *)mem_alloc(n + 1);
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

    bool open(const std::wstring &path) noexcept;
    bool start(bool recursive) noexcept;
    void cancel() noexcept;
    result try_read() noexcept;
    const std::wstring &path() const noexcept;
    const std::byte *data() const noexcept;

private:
    std::wstring m_path;
    HANDLE m_directory;
    std::array<std::byte, kBufSize> m_buffer;
};

task::task() noexcept : m_path(), m_directory(INVALID_HANDLE_VALUE), m_buffer() {
    memset((OVERLAPPED *)this, 0, sizeof(OVERLAPPED));
    hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
}

task::~task() noexcept { assert(m_directory == INVALID_HANDLE_VALUE); }

bool task::open(const std::wstring &path) noexcept {
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

const std::wstring &task::path() const noexcept { return m_path; }

const std::byte *task::data() const noexcept { return m_buffer.data(); }

watch::watch() noexcept : m_notify(), m_tasks() {}

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    if (m_tasks.empty()) {
        return;
    }
    for (auto &task : m_tasks) {
        task.cancel();
    }
    m_tasks.clear();
}

void watch::add(const string_type &path) noexcept {
    auto &t = m_tasks.emplace_back();
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

bool watch::event_update(task &task) noexcept {
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
    const std::byte *data = task.data();
    for (;;) {
        const FILE_NOTIFY_INFORMATION &fni = (const FILE_NOTIFY_INFORMATION &)*data;
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
                neko_assert(0, "unreachable");
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
    for (auto &[desc, _] : m_fd_path) {
        (void)_;
        inotify_rm_watch(m_inotify_fd, desc);
    }
    m_fd_path.clear();
    close(m_inotify_fd);
    m_inotify_fd = -1;
}

void watch::add(const string_type &str) noexcept {
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
        const auto &emplace_result = m_fd_path.emplace(std::make_pair(desc, path.string()));
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

void watch::event_update(void *e) noexcept {
    inotify_event *event = (inotify_event *)e;
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
        for (std::byte *p = buf; p < buf + n;) {
            auto event = (struct inotify_event *)p;
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
static void event_cb(ConstFSEventStreamRef streamRef, void *info, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) noexcept {
    (void)streamRef;
    (void)eventIds;
    watch *self = (watch *)info;
    self->event_update((const char **)eventPaths, eventFlags, numEvents);
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

void watch::add(const string_type &path) noexcept {
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
    for (auto &path : m_paths) {
        paths[i] = CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
        if (paths[i] == NULL) {
            while (i != 0) {
                CFRelease(paths[--i]);
            }
            return;
        }
        i++;
    }
    CFArrayRef cf_paths = CFArrayCreate(NULL, (const void **)&paths[0], m_paths.size(), NULL);
    if (create_stream(cf_paths)) {
        return;
    }
    CFRelease(cf_paths);
}

void watch::event_update(const char *paths[], const FSEventStreamEventFlags flags[], size_t n) noexcept {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (size_t i = 0; i < n; ++i) {
        const char *path = paths[i];
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

void *c_mem_alloc(size_t bytes) { return g_allocator->alloc(bytes, __FILE__, __LINE__); }
void c_mem_free(void *ptr) { g_allocator->free((void *)ptr); }

#if NEKO_NEW

void *operator new(std::size_t n) {
    void *p = mem_alloc(n);
    if (!p) {
        abort(); /* FIXME: do not abort */
    }
    return p;
}

void *operator new[](std::size_t n) {
    void *p = mem_alloc(n);
    if (!p) {
        abort();
    }
    return p;
}

void operator delete(void *p) { mem_free(p); }

void operator delete[](void *p) { mem_free(p); }

void operator delete(void *p, std::size_t) { mem_free(p); }

void operator delete[](void *p, std::size_t) { mem_free(p); }

#endif

/*========================
// byte_buffer
========================*/

void byte_buffer_init(byte_buffer_t *buffer) {
    buffer->data = (u8 *)mem_alloc(NEKO_BYTE_BUFFER_DEFAULT_CAPCITY);
    buffer->capacity = NEKO_BYTE_BUFFER_DEFAULT_CAPCITY;
    buffer->size = 0;
    buffer->position = 0;
}

byte_buffer_t byte_buffer_new() {
    byte_buffer_t buffer;
    byte_buffer_init(&buffer);
    return buffer;
}

void byte_buffer_free(byte_buffer_t *buffer) {
    if (buffer && buffer->data) {
        mem_free(buffer->data);
    }
}

void byte_buffer_clear(byte_buffer_t *buffer) {
    buffer->size = 0;
    buffer->position = 0;
}

bool byte_buffer_empty(byte_buffer_t *buffer) { return (buffer->size == 0); }

size_t byte_buffer_size(byte_buffer_t *buffer) { return buffer->size; }

void byte_buffer_resize(byte_buffer_t *buffer, size_t sz) {

    // if (sz == 4096) neko_assert(0);

    u8 *data = (u8 *)mem_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;
    buffer->capacity = (u32)sz;
}

void byte_buffer_copy_contents(byte_buffer_t *dst, byte_buffer_t *src) {
    byte_buffer_seek_to_beg(dst);
    byte_buffer_seek_to_beg(src);
    byte_buffer_write_bulk(dst, src->data, src->size);
}

void byte_buffer_seek_to_beg(byte_buffer_t *buffer) { buffer->position = 0; }

void byte_buffer_seek_to_end(byte_buffer_t *buffer) { buffer->position = buffer->size; }

void byte_buffer_advance_position(byte_buffer_t *buffer, size_t sz) { buffer->position += (u32)sz; }

void byte_buffer_write_bulk(byte_buffer_t *buffer, void *src, size_t size) {
    // 检查是否需要调整大小
    size_t total_write_size = buffer->position + size;
    if (total_write_size >= (size_t)buffer->capacity) {
        size_t capacity = buffer->capacity * 2;
        while (capacity <= total_write_size) {
            capacity *= 2;
        }

        byte_buffer_resize(buffer, capacity);
    }

    // memcpy data
    memcpy((buffer->data + buffer->position), src, size);

    buffer->size += (u32)size;
    buffer->position += (u32)size;
}

void byte_buffer_read_bulk(byte_buffer_t *buffer, void **dst, size_t size) {
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (u32)size;
}

void byte_buffer_write_str(byte_buffer_t *buffer, const char *str) {
    // 写入字符串的大小
    u32 str_len = neko_string_length(str);
    byte_buffer_write(buffer, uint16_t, str_len);

    size_t i;
    for (i = 0; i < str_len; ++i) {
        byte_buffer_write(buffer, u8, str[i]);
    }
}

void byte_buffer_read_str(byte_buffer_t *buffer, char *str) {
    // 从缓冲区读取字符串的大小
    uint16_t sz;
    byte_buffer_read(buffer, uint16_t, &sz);

    u32 i;
    for (i = 0; i < sz; ++i) {
        byte_buffer_read(buffer, u8, &str[i]);
    }
    str[i] = '\0';
}

bool byte_buffer_write_to_file(byte_buffer_t *buffer, const char *output_path) { return neko_os_write_file_contents(output_path, "wb", buffer->data, buffer->size); }

bool byte_buffer_read_from_file(byte_buffer_t *buffer, const char *file_path) {
    if (!buffer) return false;

    if (buffer->data) {
        byte_buffer_free(buffer);
    }

    buffer->data = (u8 *)neko_os_read_file_contents(file_path, "rb", (size_t *)&buffer->size);
    if (!buffer->data) {
        neko_assert(false);
        return false;
    }

    buffer->position = 0;
    buffer->capacity = buffer->size;
    return true;
}

void byte_buffer_memset(byte_buffer_t *buffer, u8 val) { memset(buffer->data, val, buffer->capacity); }

/*========================
// Dynamic Array
========================*/

void *neko_dyn_array_resize_impl(void *arr, size_t sz, size_t amount) {
    size_t capacity;

    if (arr) {
        capacity = amount;
    } else {
        capacity = 0;
    }

    size_t new_size = capacity * sz + sizeof(neko_dyn_array);

    // 仅使用标头信息创建新的 neko_dyn_array
    neko_dyn_array *data = (neko_dyn_array *)mem_realloc(arr ? neko_dyn_array_head(arr) : 0, new_size);

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (i32)capacity;
        return ((i32 *)data + 2);
    }

    return NULL;
}

void **neko_dyn_array_init(void **arr, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array *data = (neko_dyn_array *)mem_alloc(val_len + sizeof(neko_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((i32 *)data + 2);
    }
    return arr;
}

void neko_dyn_array_push_data(void **arr, void *val, size_t val_len) {
    if (*arr == NULL) {
        neko_dyn_array_init(arr, val_len);
    }
    if (neko_dyn_array_need_grow(*arr, 1)) {
        i32 capacity = neko_dyn_array_capacity(*arr) * 2;

        // Create new neko_dyn_array with just the header information
        neko_dyn_array *data = (neko_dyn_array *)mem_realloc(neko_dyn_array_head(*arr), capacity * val_len + sizeof(neko_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((i32 *)data + 2);
        }
    }
    size_t offset = neko_dyn_array_size(*arr);
    memcpy(((u8 *)(*arr)) + offset * val_len, val, val_len);
    neko_dyn_array_head(*arr)->size++;
}

/*========================
// Hash Table
========================*/

void __neko_hash_table_init_impl(void **ht, size_t sz) { *ht = mem_alloc(sz); }

/*========================
// Slot Array
========================*/

void **neko_slot_array_init(void **sa, size_t sz) {
    if (*sa == NULL) {
        *sa = mem_alloc(sz);
        memset(*sa, 0, sz);
        return sa;
    } else {
        return NULL;
    }
}

/*========================
// Slot Map
========================*/

void **neko_slot_map_init(void **sm) {
    if (*sm == NULL) {
        (*sm) = mem_alloc(sizeof(size_t) * 2);
        memset((*sm), 0, sizeof(size_t) * 2);
        return sm;
    }
    return NULL;
}

// ===============================================================

#define MIN_CAPACITY 2

struct CArray {
    char *buf;              // this is a char * for pointer arithmetic
    unsigned int capacity;  // alloc'd size of buf
    unsigned int length;    // number of objects
    size_t object_size;     // size of each element
};

CArray *array_new_(size_t object_size) {
    CArray *arr;

    arr = (CArray *)mem_alloc(sizeof(CArray));
    arr->object_size = object_size;
    arr->capacity = MIN_CAPACITY;
    arr->buf = (char *)mem_alloc(arr->object_size * arr->capacity);
    arr->length = 0;

    return arr;
}
void array_free(CArray *arr) {
    mem_free(arr->buf);
    mem_free(arr);
}

void *array_get(CArray *arr, unsigned int i) { return arr->buf + arr->object_size * i; }
void *array_top(CArray *arr) { return arr->buf + arr->object_size * (arr->length - 1); }
unsigned int array_length(CArray *arr) { return arr->length; }

void *array_begin(CArray *arr) { return arr->buf; }
void *array_end(CArray *arr) { return arr->buf + arr->object_size * arr->length; }

void *array_add(CArray *arr) {
    // too small? double it
    if (++arr->length > arr->capacity) arr->buf = (char *)mem_realloc(arr->buf, arr->object_size * (arr->capacity = arr->capacity << 1));
    return arr->buf + arr->object_size * (arr->length - 1);
}
void array_reset(CArray *arr, unsigned int num) {
    mem_free(arr->buf);

    arr->length = num;
    arr->capacity = num < MIN_CAPACITY ? MIN_CAPACITY : num;
    arr->buf = (char *)mem_alloc(arr->object_size * arr->capacity);
}
void array_pop(CArray *arr) {
    // too big (> four times as is needed)? halve it
    if (--arr->length << 2 < arr->capacity && arr->capacity > MIN_CAPACITY) arr->buf = (char *)mem_realloc(arr->buf, arr->object_size * (arr->capacity = arr->capacity >> 1));
}

bool array_quick_remove(CArray *arr, unsigned int i) {
    bool ret = false;

    if (i + 1 < arr->length) {
        memcpy(arr->buf + arr->object_size * i, arr->buf + arr->object_size * (arr->length - 1), arr->object_size);
        ret = true;
    }

    array_pop(arr);
    return ret;
}

void array_sort(CArray *arr, int (*compar)(const void *, const void *)) { qsort(arr->buf, arr->length, arr->object_size, compar); }

// -------------------------------------------------------------------------

#ifdef ARRAY_TEST

typedef struct {
    int a, b;
} IntPair;

void dump(CArray *arr) {
    printf("{ (%d, %d) -- ", arr->capacity, arr->length);
    for (unsigned int i = 0; i < arr->length; ++i) {
        IntPair *p = array_get(arr, i);
        printf("(%d, %d) ", p->a, p->b);
    }
    printf("}\n");
}

int int_compare(const void *a, const void *b) {
    const int *ia = a, *ib = b;
    return *ia - *ib;
}

void test_sort() {
    int *i;
    CArray *arr = array_new(int);

    array_add_val(int, arr) = 3;
    array_add_val(int, arr) = 5;
    array_add_val(int, arr) = 1;
    array_add_val(int, arr) = 7;
    array_add_val(int, arr) = 1;
    array_add_val(int, arr) = 0;
    array_add_val(int, arr) = 499;
    array_add_val(int, arr) = 200;

    printf("before sort: ");
    array_foreach(i, arr) printf("%d ", *i);
    printf("\n");

    array_sort(arr, int_compare);

    printf("after sort: ");
    array_foreach(i, arr) printf("%d ", *i);
    printf("\n");

    array_free(arr);
}

int main() {
    CArray *arr = array_new(IntPair);

    // add some
    for (unsigned int i = 0; i < 7; ++i) {
        array_add_val(IntPair, arr) = (IntPair){i, i * i};
        dump(arr);
    }

    // remove some
    array_quick_remove(arr, 2);
    dump(arr);
    array_quick_remove(arr, 4);
    dump(arr);
    while (array_length(arr) > 0) {
        array_quick_remove(arr, 0);
        dump(arr);
    }

    array_free(arr);

    test_sort();

    return 0;
}

#endif

LuaVec2 vec2_zero = {0.0, 0.0};

LuaVec2 vec2_add(LuaVec2 u, LuaVec2 v) { return luavec2(u.x + v.x, u.y + v.y); }
LuaVec2 vec2_sub(LuaVec2 u, LuaVec2 v) { return luavec2(u.x - v.x, u.y - v.y); }
LuaVec2 vec2_mul(LuaVec2 u, LuaVec2 v) { return luavec2(u.x * v.x, u.y * v.y); }
LuaVec2 vec2_div(LuaVec2 u, LuaVec2 v) { return luavec2(u.x / v.x, u.y / v.y); }
LuaVec2 vec2_scalar_mul(LuaVec2 v, Scalar f) { return luavec2(v.x * f, v.y * f); }
LuaVec2 vec2_scalar_div(LuaVec2 v, Scalar f) { return luavec2(v.x / f, v.y / f); }
LuaVec2 scalar_vec2_div(Scalar f, LuaVec2 v) { return luavec2(f / v.x, f / v.y); }
LuaVec2 vec2_neg(LuaVec2 v) { return luavec2(-v.x, -v.y); }

Scalar vec2_len(LuaVec2 v) { return scalar_sqrt(v.x * v.x + v.y * v.y); }
LuaVec2 vec2_normalize(LuaVec2 v) {
    if (v.x == 0 && v.y == 0) return v;
    return vec2_scalar_div(v, vec2_len(v));
}
Scalar vec2_dot(LuaVec2 u, LuaVec2 v) { return u.x * v.x + u.y * v.y; }
Scalar vec2_dist(LuaVec2 u, LuaVec2 v) { return vec2_len(vec2_sub(u, v)); }

LuaVec2 vec2_rot(LuaVec2 v, Scalar rot) { return luavec2(v.x * scalar_cos(rot) - v.y * scalar_sin(rot), v.x * scalar_sin(rot) + v.y * scalar_cos(rot)); }
Scalar vec2_atan2(LuaVec2 v) { return scalar_atan2(v.y, v.x); }

void vec2_save(LuaVec2 *v, const char *n, Store *s) {
    Store *t;

    if (store_child_save_compressed(&t, n, s)) {
        scalar_save(&v->x, "x", t);
        scalar_save(&v->y, "y", t);
    }
}
bool vec2_load(LuaVec2 *v, const char *n, LuaVec2 d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        scalar_load(&v->x, "x", 0, t);
        scalar_load(&v->y, "y", 0, t);
    } else
        *v = d;
    return t != NULL;
}

#undef luavec2
LuaVec2 luavec2(Scalar x, Scalar y) { return LuaVec2{x, y}; }

LuaMat3 mat3_mul(LuaMat3 m, LuaMat3 n) {
    return luamat3(m.m[0][0] * n.m[0][0] + m.m[1][0] * n.m[0][1] + m.m[2][0] * n.m[0][2], m.m[0][1] * n.m[0][0] + m.m[1][1] * n.m[0][1] + m.m[2][1] * n.m[0][2],
                   m.m[0][2] * n.m[0][0] + m.m[1][2] * n.m[0][1] + m.m[2][2] * n.m[0][2],

                   m.m[0][0] * n.m[1][0] + m.m[1][0] * n.m[1][1] + m.m[2][0] * n.m[1][2], m.m[0][1] * n.m[1][0] + m.m[1][1] * n.m[1][1] + m.m[2][1] * n.m[1][2],
                   m.m[0][2] * n.m[1][0] + m.m[1][2] * n.m[1][1] + m.m[2][2] * n.m[1][2],

                   m.m[0][0] * n.m[2][0] + m.m[1][0] * n.m[2][1] + m.m[2][0] * n.m[2][2], m.m[0][1] * n.m[2][0] + m.m[1][1] * n.m[2][1] + m.m[2][1] * n.m[2][2],
                   m.m[0][2] * n.m[2][0] + m.m[1][2] * n.m[2][1] + m.m[2][2] * n.m[2][2]);
}

LuaMat3 mat3_scaling_rotation_translation(LuaVec2 scale, Scalar rot, LuaVec2 trans) {
    return luamat3(scale.x * scalar_cos(rot), scale.x * scalar_sin(rot), 0.0f, scale.y * -scalar_sin(rot), scale.y * scalar_cos(rot), 0.0f, trans.x, trans.y, 1.0f);
}

LuaVec2 mat3_get_translation(LuaMat3 m) { return luavec2(m.m[2][0], m.m[2][1]); }
Scalar mat3_get_rotation(LuaMat3 m) { return scalar_atan2(m.m[0][1], m.m[0][0]); }
LuaVec2 mat3_get_scale(LuaMat3 m) { return luavec2(scalar_sqrt(m.m[0][0] * m.m[0][0] + m.m[0][1] * m.m[0][1]), scalar_sqrt(m.m[1][0] * m.m[1][0] + m.m[1][1] * m.m[1][1])); }

LuaMat3 mat3_inverse(LuaMat3 m) {
    Scalar det;
    LuaMat3 inv;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
    inv.m[0][1] = m.m[0][2] * m.m[2][1] - m.m[0][1] * m.m[2][2];
    inv.m[0][2] = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
    inv.m[1][0] = m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2];
    inv.m[1][1] = m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0];
    inv.m[1][2] = m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2];
    inv.m[2][0] = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];
    inv.m[2][1] = m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1];
    inv.m[2][2] = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];

    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + m.m[0][2] * inv.m[2][0];

    if (det <= 10e-8) return inv;  // TODO: figure out what to do if not invertible

    inv.m[0][0] /= det;
    inv.m[0][1] /= det;
    inv.m[0][2] /= det;
    inv.m[1][0] /= det;
    inv.m[1][1] /= det;
    inv.m[1][2] /= det;
    inv.m[2][0] /= det;
    inv.m[2][1] /= det;
    inv.m[2][2] /= det;

    return inv;
}

LuaVec2 mat3_transform(LuaMat3 m, LuaVec2 v) { return luavec2(m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0], m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1]); }

void mat3_save(LuaMat3 *m, const char *n, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_save_compressed(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_save(&m->m[i][j], NULL, t);
}
bool mat3_load(LuaMat3 *m, const char *n, LuaMat3 d, Store *s) {
    Store *t;
    unsigned int i, j;

    if (store_child_load(&t, n, s))
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 3; ++j) scalar_load(&m->m[i][j], NULL, 0, t);
    else
        *m = d;
    return t != NULL;
}

#undef luamat3_identity
LuaMat3 luamat3_identity() { return luamat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f); }

#undef luamat3
LuaMat3 luamat3(Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22) { return LuaMat3{{{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}}}; }

BBox bbox_merge(BBox a, BBox b) { return bbox(luavec2(scalar_min(a.min.x, b.min.x), scalar_min(a.min.y, b.min.y)), luavec2(scalar_max(a.max.x, b.max.x), scalar_max(a.max.y, b.max.y))); }
BBox bbox_bound(LuaVec2 a, LuaVec2 b) { return bbox(luavec2(scalar_min(a.x, b.x), scalar_min(a.y, b.y)), luavec2(scalar_max(a.x, b.x), scalar_max(a.y, b.y))); }
bool bbox_contains(BBox b, LuaVec2 p) { return b.min.x <= p.x && p.x <= b.max.x && b.min.y <= p.y && p.y <= b.max.y; }

BBox bbox(LuaVec2 min, LuaVec2 max) {
    BBox bb;
    bb.min = min;
    bb.max = max;
    return bb;
}

BBox bbox_transform(LuaMat3 m, BBox b) {
    LuaVec2 v1, v2, v3, v4;

    v1 = mat3_transform(m, luavec2(b.min.x, b.min.y));
    v2 = mat3_transform(m, luavec2(b.max.x, b.min.y));
    v3 = mat3_transform(m, luavec2(b.max.x, b.max.y));
    v4 = mat3_transform(m, luavec2(b.min.x, b.max.y));

    return bbox_merge(bbox_bound(v1, v2), bbox_bound(v3, v4));
}

Color color_black = {0.0, 0.0, 0.0, 1.0};
Color color_white = {1.0, 1.0, 1.0, 1.0};
Color color_gray = {0.5, 0.5, 0.5, 1.0};
Color color_red = {1.0, 0.0, 0.0, 1.0};
Color color_green = {0.0, 1.0, 0.0, 1.0};
Color color_blue = {0.0, 0.0, 1.0, 1.0};
Color color_clear = {0.0, 0.0, 0.0, 0.0};

void color_save(Color *c, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) {
        scalar_save(&c->r, "r", t);
        scalar_save(&c->g, "g", t);
        scalar_save(&c->b, "b", t);
        scalar_save(&c->a, "a", t);
    }
}
bool color_load(Color *c, const char *n, Color d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        scalar_load(&c->r, "r", 0, t);
        scalar_load(&c->g, "g", 0, t);
        scalar_load(&c->b, "b", 0, t);
        scalar_load(&c->a, "a", 0, t);
    } else
        *c = d;
    return t != NULL;
}

#undef color_opaque
Color color_opaque(Scalar r, Scalar g, Scalar b) { return color(r, g, b, 1); }

#undef color
Color color(Scalar r, Scalar g, Scalar b, Scalar a) { return Color{r, g, b, a}; }

typedef struct Stream Stream;
struct Stream {
    char *buf;
    size_t pos;
    size_t cap;
};

struct Store {
    char *name;
    Stream sm[1];
    bool compressed;

    Store *child;
    Store *parent;
    Store *sibling;

    Store *iterchild;

    char *str;
};

static void _stream_init(Stream *sm) {
    sm->buf = NULL;
    sm->pos = 0;
    sm->cap = 0;
}

static void _stream_fini(Stream *sm) { mem_free(sm->buf); }

static void _stream_grow(Stream *sm, size_t pos) {
    if (pos >= sm->cap) {
        if (sm->cap < 2) sm->cap = 2;
        while (pos >= sm->cap) sm->cap <<= 1;
        sm->buf = (char *)mem_realloc(sm->buf, sm->cap);
    }
}

static void _stream_printf(Stream *sm, const char *fmt, ...) {
    va_list ap1, ap2;
    size_t new_pos;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    new_pos = sm->pos + vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    _stream_grow(sm, new_pos);
    vsprintf(sm->buf + sm->pos, fmt, ap1);
    sm->pos = new_pos;
    va_end(ap1);
}

static void _stream_scanf_(Stream *sm, const char *fmt, int *n, ...) {
    va_list ap;

    error_assert(sm->buf, "stream buffer should be initialized");

    /*
     * scanf is tricky because we need to move forward by number of
     * scanned characters -- *n will store number of characters read,
     * needs to also be put at end of parameter list (see
     * _stream_scanf(...) macro), and "%n" needs to be appended at
     * end of original fmt
     */

    va_start(ap, n);
    vsscanf(&sm->buf[sm->pos], fmt, ap);
    va_end(ap);
    sm->pos += *n;
}
#define _stream_scanf(sm, fmt, ...)                                        \
    do {                                                                   \
        int n_read__;                                                      \
        _stream_scanf_(sm, fmt "%n", &n_read__, ##__VA_ARGS__, &n_read__); \
    } while (0)

static void _stream_write_string(Stream *sm, const char *s) {

    if (!s) {
        _stream_printf(sm, "n ");
        return;
    }

    _stream_printf(sm, "\"");

    for (; *s; ++s) {
        _stream_grow(sm, sm->pos);

        if (*s == '"') {
            sm->buf[sm->pos++] = '\\';
            _stream_grow(sm, sm->pos);
        }

        sm->buf[sm->pos++] = *s;
    }

    _stream_printf(sm, "\" ");
}

static char *_stream_read_string_(Stream *sm, size_t *plen) {
    Stream rm[1];

    if (sm->buf[sm->pos] == 'n') {
        if (strncmp(&sm->buf[sm->pos], "n ", 2)) error("corrupt save");
        sm->pos += 2;
        return NULL;
    }

    _stream_init(rm);

    if (sm->buf[sm->pos] != '"') error("corrupt save");
    ++sm->pos;

    while (sm->buf[sm->pos] != '"') {
        _stream_grow(rm, rm->pos);

        if (sm->buf[sm->pos] == '\\' && sm->buf[sm->pos + 1] == '"') {
            rm->buf[rm->pos++] = '"';
            sm->pos += 2;
        } else
            rm->buf[rm->pos++] = sm->buf[sm->pos++];
    }
    sm->pos += 2;

    _stream_grow(rm, rm->pos);
    rm->buf[rm->pos] = '\0';

    if (plen) *plen = rm->cap;

    return rm->buf;
}
#define _stream_read_string(sm) _stream_read_string_(sm, NULL)

static Store *_store_new(Store *parent) {
    Store *s = (Store *)mem_alloc(sizeof(Store));

    s->name = NULL;
    _stream_init(s->sm);
    s->compressed = false;

    s->parent = parent;
    s->child = NULL;
    s->sibling = s->parent ? s->parent->child : NULL;
    if (s->parent) s->parent->iterchild = s->parent->child = s;

    s->iterchild = NULL;
    s->str = NULL;

    return s;
}

static void _store_free(Store *s) {
    Store *t;

    while (s->child) {
        t = s->child->sibling;
        _store_free(s->child);
        s->child = t;
    }

    mem_free(s->name);
    _stream_fini(s->sm);
    mem_free(s->str);

    mem_free(s);
}

static void _store_write(Store *s, Stream *sm) {
    Store *c;

    _stream_printf(sm, s->compressed ? "[ " : "{ ");
    _stream_write_string(sm, s->name);
    _stream_write_string(sm, s->sm->buf);
    for (c = s->child; c; c = c->sibling) _store_write(c, sm);
    _stream_printf(sm, s->compressed ? "] " : "} ");
}

#define INDENT 2

static void _store_write_pretty(Store *s, unsigned int indent, Stream *sm) {
    Store *c;

    if (s->compressed) {
        _stream_printf(sm, "%*s", indent, "");
        _store_write(s, sm);
        _stream_printf(sm, "\n");
        return;
    }

    _stream_printf(sm, "%*s{ ", indent, "");

    _stream_write_string(sm, s->name);
    _stream_write_string(sm, s->sm->buf);
    if (s->child) _stream_printf(sm, "\n");

    for (c = s->child; c; c = c->sibling) _store_write_pretty(c, indent + INDENT, sm);

    if (s->child)
        _stream_printf(sm, "%*s}\n", indent, "");
    else
        _stream_printf(sm, "}\n");
}

static Store *_store_read(Store *parent, Stream *sm) {
    char close_brace = '}';
    Store *s = _store_new(parent);

    if (sm->buf[sm->pos] == '[') {
        s->compressed = true;
        close_brace = ']';
    } else if (sm->buf[sm->pos] != '{' && sm->buf[sm->pos] != '\n')
        error("corrupt save");
    while (isspace(sm->buf[++sm->pos]));

    s->name = _stream_read_string(sm);
    s->sm->buf = _stream_read_string_(sm, &s->sm->cap);
    s->sm->pos = 0;

    for (;;) {
        while (isspace(sm->buf[sm->pos])) ++sm->pos;

        if (sm->buf[sm->pos] == close_brace) {
            ++sm->pos;
            break;
        }

        _store_read(s, sm);
    }

    return s;
}

bool store_child_save(Store **sp, const char *name, Store *parent) {
    Store *s;

    if (parent->compressed) return (*sp = parent) != NULL;

    s = _store_new(parent);
    if (name) {
        s->name = (char *)mem_alloc(strlen(name) + 1);
        strcpy(s->name, name);
    }
    return (*sp = s) != NULL;
}

bool store_child_save_compressed(Store **sp, const char *name, Store *parent) {
    bool r = store_child_save(sp, name, parent);
    (*sp)->compressed = true;
    return r;
}

bool store_child_load(Store **sp, const char *name, Store *parent) {
    Store *s;

    if (parent->compressed) return (*sp = parent) != NULL;

    if (!name) {
        s = parent->iterchild;
        if (parent->iterchild) parent->iterchild = parent->iterchild->sibling;
        return (*sp = s) != NULL;
    }

    for (s = parent->child; s && (!s->name || strcmp(s->name, name)); s = s->sibling);
    return (*sp = s) != NULL;
}

Store *store_open() { return _store_new(NULL); }

Store *store_open_str(const char *str) {
    Stream sm = {(char *)str, 0, 0};
    return _store_read(NULL, &sm);
}
const char *store_write_str(Store *s) {
    Stream sm[1];

    _stream_init(sm);
    _store_write_pretty(s, 0, sm);
    mem_free(s->str);
    s->str = sm->buf;
    return s->str;
}

Store *store_open_file(const char *filename) {
    Store *s;
    unsigned int n;
    char *str;

    vfs_file f = neko_capi_vfs_fopen(filename);
    error_assert(f.data, "file '%s' must be open for reading", filename);

    neko_capi_vfs_fscanf(&f, "%u\n", &n);
    str = (char *)mem_alloc(n + 1);
    neko_capi_vfs_fread(str, 1, n, &f);
    neko_capi_vfs_fclose(&f);
    str[n] = '\0';
    s = store_open_str(str);
    mem_free(str);
    return s;
}

void store_write_file(Store *s, const char *filename) {
    FILE *f;
    const char *str;
    unsigned int n;

    f = fopen(filename, "w");
    error_assert(f, "file '%s' must be open for writing", filename);

    str = store_write_str(s);
    n = strlen(str);
    fprintf(f, "%u\n", n);
    fwrite(str, 1, n, f);
    fclose(f);
}

void store_close(Store *s) { _store_free(s); }

#define _store_printf(s, fmt, ...) _stream_printf(s->sm, fmt, ##__VA_ARGS__)
#define _store_scanf(s, fmt, ...) _stream_scanf(s->sm, fmt, ##__VA_ARGS__)

void scalar_save(const Scalar *f, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) {
        if (*f == SCALAR_INFINITY)
            _store_printf(t, "i ");
        else
            _store_printf(t, "%f ", *f);
    }
}
bool scalar_load(Scalar *f, const char *n, Scalar d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        if (t->sm->buf[t->sm->pos] == 'i') {
            *f = SCALAR_INFINITY;
            _store_scanf(t, "i ");
        } else
            _store_scanf(t, "%f ", f);
        return true;
    }

    *f = d;
    return false;
}

void uint_save(const unsigned int *u, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%u ", *u);
}
bool uint_load(unsigned int *u, const char *n, unsigned int d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s))
        _store_scanf(t, "%u ", u);
    else
        *u = d;
    return t != NULL;
}

void int_save(const int *i, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%d ", *i);
}
bool int_load(int *i, const char *n, int d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s))
        _store_scanf(t, "%d ", i);
    else
        *i = d;
    return t != NULL;
}

void bool_save(const bool *b, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _store_printf(t, "%d ", (int)*b);
}
bool bool_load(bool *b, const char *n, bool d, Store *s) {
    int i = d;
    Store *t;

    if (store_child_load(&t, n, s)) _store_scanf(t, "%d ", &i);
    *b = i;
    return t != NULL;
}

void string_save(const char **c, const char *n, Store *s) {
    Store *t;

    if (store_child_save(&t, n, s)) _stream_write_string(t->sm, *c);
}
bool string_load(char **c, const char *n, const char *d, Store *s) {
    Store *t;

    if (store_child_load(&t, n, s)) {
        *c = _stream_read_string(t->sm);
        return true;
    }

    if (d) {
        *c = (char *)mem_alloc(strlen(d) + 1);
        strcpy(*c, d);
    } else
        *c = NULL;
    return false;
}

#if 0

int main()
{
    Store *s, *d, *sprite_s, *pool_s, *elem_s;
    char *c;
    Scalar r;

    s = store_open();
    {
        if (store_child_save(&sprite_s, "sprite", s))
        {
            c = "hello, world";
            string_save(&c, "prop1", sprite_s);

            c = "hello, world ... again";
            string_save(&c, "prop2", sprite_s);

            r = SCALAR_INFINITY;
            scalar_save(&r, "prop6", sprite_s);

            if (store_child_save(&pool_s, "pool", sprite_s))
            {
                store_child_save(&elem_s, "elem1", pool_s);
                store_child_save(&elem_s, "elem2", pool_s);
            }
        }
    }
    store_write_file(s, "test.sav");
    store_close(s);

    // ----

    d = store_open_file("test.sav");
    {
        if (store_child_load(&sprite_s, "sprite", d))
        {
            printf("%s\n", sprite_s->name);

            string_load(&c, "prop1", "hai", sprite_s);
            printf("    prop1: %s\n", c);

            string_load(&c, "prop3", "hai", sprite_s);
            printf("    prop3: %s\n", c);

            string_load(&c, "prop2", "hai", sprite_s);
            printf("    prop2: %s\n", c);

            scalar_load(&r, "prop6", 4.2, sprite_s);
            printf("    prop6: %f\n", r);

            if (store_child_load(&pool_s, "pool", sprite_s))
                while (store_child_load(&elem_s, NULL, pool_s))
                    printf("        %s\n", elem_s->name);
        }
    }
    store_close(d);

    return 0;
}

#endif
