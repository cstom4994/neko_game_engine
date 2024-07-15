
#include "neko_prelude.h"

#include "neko.hpp"
#include "neko_base.h"
#include "neko_os.h"

bool String::is_cstr() { return data[len] == '\0'; }

String String::substr(u64 i, u64 j) {
    assert(i <= j);
    assert(j <= (i64)len);
    return {&data[i], j - i};
}

bool String::starts_with(String match) {
    if (len < match.len) {
        return false;
    }
    return substr(0, match.len) == match;
}

bool String::ends_with(String match) {
    if (len < match.len) {
        return false;
    }
    return substr(len - match.len, len) == match;
}

u64 String::first_of(char c) {
    for (u64 i = 0; i < len; i++) {
        if (data[i] == c) {
            return i;
        }
    }

    return (u64)-1;
}

u64 String::last_of(char c) {
    for (u64 i = len; i > 0; i--) {
        if (data[i - 1] == c) {
            return i - 1;
        }
    }

    return (u64)-1;
}

String to_cstr(String str) {
    char *buf = (char *)mem_alloc(str.len + 1);
    memcpy(buf, str.data, str.len);
    buf[str.len] = 0;
    return {buf, str.len};
}

using namespace neko;

struct Profile {
    Queue<TraceEvent> events;
    Thread recv_thread;
};

static Profile g_profile = {};

static void profile_recv_thread(void *) {
    StringBuilder sb = {};
    sb.swap_filename(os_program_path(), "profile.json");

    FILE *f = fopen(sb.data, "w");
    sb.trash();

    neko_defer(fclose(f));

    fputs("[", f);
    while (true) {
        TraceEvent e = g_profile.events.demand();
        if (e.name == nullptr) {
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

void profile_fini() {
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
    QueryPerformanceFrequency(&g_tm.freq);
    QueryPerformanceCounter(&g_tm.start);
#elif defined(__APPLE__) && defined(__MACH__)
    mach_timebase_info(&g_tm.timebase);
    g_tm.start = mach_absolute_time();
#elif defined(__EMSCRIPTEN__)
    g_tm.start = emscripten_get_now();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_tm.start = (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
#endif
}

#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
int64_t __int64_muldiv(int64_t value, int64_t numer, int64_t denom) {
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}
#endif

NEKO_API_DECL uint64_t neko_tm_now(void) {
    NEKO_ASSERT(g_tm.initialized == 0xABCDEF01);
    uint64_t now;
#if defined(_WIN32)
    LARGE_INTEGER qpc_t;
    QueryPerformanceCounter(&qpc_t);
    now = (uint64_t)__int64_muldiv(qpc_t.QuadPart - g_tm.start.QuadPart, 1000000000, g_tm.freq.QuadPart);
#elif defined(__APPLE__) && defined(__MACH__)
    const uint64_t mach_now = mach_absolute_time() - g_tm.start;
    now = (uint64_t)_stm_int64_muldiv((int64_t)mach_now, (int64_t)g_tm.timebase.numer, (int64_t)g_tm.timebase.denom);
#elif defined(__EMSCRIPTEN__)
    double js_now = emscripten_get_now() - g_tm.start;
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