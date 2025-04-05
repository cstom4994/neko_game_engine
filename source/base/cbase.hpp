
#pragma once

#include "base/common/vfs.hpp"
#include "base/common/arena.hpp"

namespace Neko {

class CBase {
    int argc;
    const char** argv;
    Slice<String> args;

public:
    Mutex gpu_mtx;

    std::atomic<bool> error_mode;

    Mutex error_mtx;
    String fatal_error_string;
    String traceback;

    std::atomic<bool> hot_reload_enabled;
    std::atomic<f32> reload_interval;

public:
    inline const Slice<String>& GetArgs() const { return args; };

    inline int GetArgc() const { return argc; };
    inline const char** GetArgv() const { return argv; };

public:
    void Init(int argc, const char* argv[]);
    void Fini();

    void UnLoadVFS();

    bool InitLuaBase();
    void FiniLuaBase();

    inline void fatal_error(Neko::String str) {
        if (!error_mode.load()) {
            Neko::LockGuard<Neko::Mutex> lock{error_mtx};

            fatal_error_string = to_cstr(str);
            fprintf(stderr, "%s\n", fatal_error_string.data);
            error_mode.store(true);
        }
    }
};

i32 keyboard_lookup(String str);

}  // namespace Neko

using Neko::CBase;

extern CBase gBase;

i32 neko_buildnum(void);
