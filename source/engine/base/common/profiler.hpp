#pragma once

#include "base/common/base.hpp"

namespace Neko {

void profile_setup();
void profile_shutdown();

#ifdef USE_PROFILER

struct TraceEvent {
    const char *cat;
    const char *name;
    u64 ts;
    u16 tid;
    char ph;
};

struct Instrument {
    const char *cat;
    const char *name;
    i32 tid;

    Instrument(const char *cat, const char *name);
    ~Instrument();
};

#define PROFILE_FUNC() auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name) auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif  // USE_PROFILER

#ifndef USE_PROFILER
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif

}  // namespace Neko