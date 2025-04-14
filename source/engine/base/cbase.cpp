
#include "base/cbase.hpp"

#include "base/common/mem.hpp"
#include "base/common/vfs.hpp"
#include "base/common/profiler.hpp"
#include "base/common/string.hpp"
#include "base/common/logger.hpp"
#include "engine/scripting/scripting.h"
#include "engine/base/common/job.hpp"

#include <cstdlib>

#ifdef NEKO_IS_WIN32
#define HACK_CALL __cdecl
#else
#define HACK_CALL
#endif

extern "C" void __cxa_pure_virtual() { abort(); }

#if 0
#ifdef HACK_MEM_CHECK
void* HACK_CALL operator new(size_t size) { return ::Neko::g_allocator->alloc(size, "[operator new]", 0); }
void* HACK_CALL operator new[](size_t size) { return ::Neko::g_allocator->alloc(size, "[operator new[]]", 0); }
void HACK_CALL operator delete(void* p) noexcept { mem_free(p); }
void HACK_CALL operator delete[](void* p) noexcept { mem_free(p); }
void HACK_CALL operator delete(void* p, size_t) { mem_free(p); }
#else
void* HACK_CALL operator new(size_t size) { return malloc(size); }
void* HACK_CALL operator new[](size_t size) { return malloc(size); }
void HACK_CALL operator delete(void* p) noexcept { free(p); }
void HACK_CALL operator delete[](void* p) noexcept { free(p); }
void HACK_CALL operator delete(void* p, size_t) { free(p); }
#endif
#endif

namespace Neko {
Allocator* g_allocator = []() -> Allocator* {
#ifndef NDEBUG
    static DebugAllocator alloc;
#else
    static HeapAllocator alloc;
#endif
    return &alloc;
}();
}  // namespace Neko

namespace Neko {

void CBase::Init(int argc, const char* argv[]) {

    os_high_timer_resolution();
    TimeUtil::initialize();

    Logger::init();

    Job::init();

    profile_setup();
    PROFILE_FUNC();

    this->argc = argc;
    this->argv = argv;

    this->args.resize(argc);
    for (i32 i = 0; i < argc; i++) {
        this->args[i] = to_cstr(String(argv[i]));
    }

#if defined(NDEBUG)
    LOG_INFO("neko {}", neko_buildnum());
#else
    LOG_INFO("neko {} (debug build) (Lua {}.{}.{}, {})", neko_buildnum(), LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE, LUAJIT_VERSION);
#endif

    Neko::modules::initialize<Scripting>();
    Neko::modules::initialize<VFS>();

    the<Scripting>().init_lua();

    hot_reload_enabled.store(true);
}

void CBase::Fini() {

    for (String arg : this->args) {
        mem_free(arg.data);
    }
    mem_free(this->args.data);

#ifndef NDEBUG
    DebugAllocator* allocator = dynamic_cast<DebugAllocator*>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs(true);
    }
#endif

    Logger::shutdown();

    neko_println("see ya");
}

void CBase::UnLoadVFS() { the<VFS>().vfs_fini(); }

bool CBase::InitLuaBase() { return true; }

void CBase::FiniLuaBase() { return; }

}  // namespace Neko

i32 neko_buildnum(void) {
    static const char* __build_date = __DATE__;
    static const char* mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    i32 m = 0, d = 0, y = 0;
    static i32 b = 0;
    if (b != 0) return b;  // 优化
    for (m = 0; m < 11; m++) {
        if (!strncmp(&__build_date[0], mon[m], 3)) break;
        d += mond[m];
    }
    d += atoi(&__build_date[4]) - 1;
    y = atoi(&__build_date[7]) - 2023;
    b = d + (i32)((y - 1) * 365.25f);
    if (((y % 4) == 0) && m > 1) b += 1;
    b -= 211;
    return b;
}

void DebugAllocator::dump_allocs(bool detailed) {
    i32 allocs = 0;
    for (DebugAllocInfo* info = head; info != nullptr; info = info->next) {
        if (detailed) LOG_TRACE("  {} bytes: {}:{}", (unsigned long long)info->size, info->file, info->line);
        allocs++;
    }
    LOG_TRACE("leaks {} allocation(s) with {} bytes", allocs, alloc_size);
}