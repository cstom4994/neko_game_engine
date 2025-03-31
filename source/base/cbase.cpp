
#include "base/cbase.hpp"

#include "base/common/mem.hpp"
#include "base/common/vfs.hpp"
#include "base/common/profiler.hpp"
#include "base/common/string.hpp"
#include "base/common/logger.hpp"

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

void CBase::SetArgs(int argc, const char** argv) {
    this->argc = argc;
    this->argv = argv;

    this->args.resize(argc);
    for (i32 i = 0; i < argc; i++) {
        this->args[i] = to_cstr(String(argv[i]));
    }
}

void CBase::Init() {

    os_high_timer_resolution();
    TimeUtil::initialize();

    Logger::init();

    profile_setup();
    PROFILE_FUNC();
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

bool CBase::LoadVFS(String filepath) {

    PROFILE_FUNC();

    // #if defined(_DEBUG) || 1
    //     MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, filepath);
    //     MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "../source/game");
    // #else
    //     MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
    //     MountResult mount_luacode = {true};
    // #endif

    bool ok = false;

    if (filepath.len == 0) {
        String path = os_program_path();
        LOG_INFO("program path: {}", path.data);

        ZipFileSystem* zipfs = mem_new<ZipFileSystem>();
        ok = zipfs->mount(path);
        ok &= vfs_mount_type(NEKO_PACKS::GAMEDATA, zipfs);
        if (!ok) {
            ok = zipfs->mount("./gamedata.zip");
            ok &= vfs_mount_type(NEKO_PACKS::GAMEDATA, zipfs);
            is_fused.store(true);
            LOG_INFO("zip_fs load with gamedata.zip");
        }
    } else {
        String mount_dir = filepath;

        if (mount_dir.ends_with(".zip")) {
            ZipFileSystem* zipfs = mem_new<ZipFileSystem>();
            ok = zipfs->mount(mount_dir);
            ok &= vfs_mount_type(NEKO_PACKS::GAMEDATA, zipfs);
            is_fused.store(true);
        } else {
            DirectoryFileSystem* dirfs1 = mem_new<DirectoryFileSystem>();
            DirectoryFileSystem* dirfs2 = mem_new<DirectoryFileSystem>();
            ok = dirfs1->mount(mount_dir);
            ok &= dirfs2->mount("../source/game");
            ok &= vfs_mount_type(NEKO_PACKS::GAMEDATA, dirfs1);
            ok &= vfs_mount_type(NEKO_PACKS::LUACODE, dirfs2);
            hot_reload_enabled.store(true);
        }
    }

    if (filepath.len != 0 && !ok) {
        fatal_error(tmp_fmt("failed to load: %s", filepath));
    }

    return ok;
}

void CBase::UnLoadVFS() { vfs_fini(); }

bool CBase::InitLuaBase() { return true; }

void CBase::FiniLuaBase() { return; }

i32 keyboard_lookup(String str) {
    switch (fnv1a(str)) {
        case "space"_hash:
            return 32;
        case "'"_hash:
            return 39;
        case ","_hash:
            return 44;
        case "-"_hash:
            return 45;
        case "."_hash:
            return 46;
        case "/"_hash:
            return 47;
        case "0"_hash:
            return 48;
        case "1"_hash:
            return 49;
        case "2"_hash:
            return 50;
        case "3"_hash:
            return 51;
        case "4"_hash:
            return 52;
        case "5"_hash:
            return 53;
        case "6"_hash:
            return 54;
        case "7"_hash:
            return 55;
        case "8"_hash:
            return 56;
        case "9"_hash:
            return 57;
        case ";"_hash:
            return 59;
        case "="_hash:
            return 61;
        case "a"_hash:
            return 65;
        case "b"_hash:
            return 66;
        case "c"_hash:
            return 67;
        case "d"_hash:
            return 68;
        case "e"_hash:
            return 69;
        case "f"_hash:
            return 70;
        case "g"_hash:
            return 71;
        case "h"_hash:
            return 72;
        case "i"_hash:
            return 73;
        case "j"_hash:
            return 74;
        case "k"_hash:
            return 75;
        case "l"_hash:
            return 76;
        case "m"_hash:
            return 77;
        case "n"_hash:
            return 78;
        case "o"_hash:
            return 79;
        case "p"_hash:
            return 80;
        case "q"_hash:
            return 81;
        case "r"_hash:
            return 82;
        case "s"_hash:
            return 83;
        case "t"_hash:
            return 84;
        case "u"_hash:
            return 85;
        case "v"_hash:
            return 86;
        case "w"_hash:
            return 87;
        case "x"_hash:
            return 88;
        case "y"_hash:
            return 89;
        case "z"_hash:
            return 90;
        case "["_hash:
            return 91;
        case "\\"_hash:
            return 92;
        case "]"_hash:
            return 93;
        case "`"_hash:
            return 96;
        case "world_1"_hash:
            return 161;
        case "world_2"_hash:
            return 162;
        case "esc"_hash:
            return 256;
        case "enter"_hash:
            return 257;
        case "tab"_hash:
            return 258;
        case "backspace"_hash:
            return 259;
        case "insert"_hash:
            return 260;
        case "delete"_hash:
            return 261;
        case "right"_hash:
            return 262;
        case "left"_hash:
            return 263;
        case "down"_hash:
            return 264;
        case "up"_hash:
            return 265;
        case "pg_up"_hash:
            return 266;
        case "pg_down"_hash:
            return 267;
        case "home"_hash:
            return 268;
        case "end"_hash:
            return 269;
        case "caps_lock"_hash:
            return 280;
        case "scroll_lock"_hash:
            return 281;
        case "num_lock"_hash:
            return 282;
        case "print_screen"_hash:
            return 283;
        case "pause"_hash:
            return 284;
        case "f1"_hash:
            return 290;
        case "f2"_hash:
            return 291;
        case "f3"_hash:
            return 292;
        case "f4"_hash:
            return 293;
        case "f5"_hash:
            return 294;
        case "f6"_hash:
            return 295;
        case "f7"_hash:
            return 296;
        case "f8"_hash:
            return 297;
        case "f9"_hash:
            return 298;
        case "f10"_hash:
            return 299;
        case "f11"_hash:
            return 300;
        case "f12"_hash:
            return 301;
        case "f13"_hash:
            return 302;
        case "f14"_hash:
            return 303;
        case "f15"_hash:
            return 304;
        case "f16"_hash:
            return 305;
        case "f17"_hash:
            return 306;
        case "f18"_hash:
            return 307;
        case "f19"_hash:
            return 308;
        case "f20"_hash:
            return 309;
        case "f21"_hash:
            return 310;
        case "f22"_hash:
            return 311;
        case "f23"_hash:
            return 312;
        case "f24"_hash:
            return 313;
        case "f25"_hash:
            return 314;
        case "kp0"_hash:
            return 320;
        case "kp1"_hash:
            return 321;
        case "kp2"_hash:
            return 322;
        case "kp3"_hash:
            return 323;
        case "kp4"_hash:
            return 324;
        case "kp5"_hash:
            return 325;
        case "kp6"_hash:
            return 326;
        case "kp7"_hash:
            return 327;
        case "kp8"_hash:
            return 328;
        case "kp9"_hash:
            return 329;
        case "kp."_hash:
            return 330;
        case "kp/"_hash:
            return 331;
        case "kp*"_hash:
            return 332;
        case "kp-"_hash:
            return 333;
        case "kp+"_hash:
            return 334;
        case "kp_enter"_hash:
            return 335;
        case "kp="_hash:
            return 336;
        case "lshift"_hash:
            return 340;
        case "lctrl"_hash:
            return 341;
        case "lalt"_hash:
            return 342;
        case "lsuper"_hash:
            return 343;
        case "rshift"_hash:
            return 344;
        case "rctrl"_hash:
            return 345;
        case "ralt"_hash:
            return 346;
        case "rsuper"_hash:
            return 347;
        case "menu"_hash:
            return 348;
        default:
            return 0;
    }
}

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
