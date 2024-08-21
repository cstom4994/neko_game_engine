#include "engine/prelude.h"

#include "engine/base.hpp"

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

#ifndef USE_PROFILER
void profile_setup() {}
void profile_shutdown() {}
#endif

#ifdef USE_PROFILER

#include "engine/base.h"
#include "engine/base.hpp"
#include "vendor/sokol_time.h"

struct Profile {
    Queue<TraceEvent> events;
    Thread recv_thread;
};

static Profile g_profile = {};

static void profile_recv_thread(void *) {
    StringBuilder sb = {};
    sb.swap_filename(os_program_path(), "profile.json");

    FILE *f = neko_fopen(sb.data, "w");
    sb.trash();

    neko_defer(neko_fclose(f));

    fputs("[", f);
    while (true) {
        TraceEvent e = g_profile.events.demand();
        if (e.name == nullptr) {
            return;
        }

        fprintf(f,
                R"({"name":"%s","cat":"%s","ph":"%c","ts":%.3f,"pid":0,"tid":%hu},)"
                "\n",
                e.name, e.cat, e.ph, stm_us(e.ts), e.tid);
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

Instrument::Instrument(const char *cat, const char *name) : cat(cat), name(name), tid(this_thread_id()) {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'B';
    e.ts = stm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

Instrument::~Instrument() {
    TraceEvent e = {};
    e.cat = cat;
    e.name = name;
    e.ph = 'E';
    e.ts = stm_now();
    e.tid = tid;

    g_profile.events.enqueue(e);
}

#endif  // USE_PROFILER