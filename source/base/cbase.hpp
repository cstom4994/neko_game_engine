
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
    std::atomic<bool> is_fused;

    Mutex error_mtx;
    String fatal_error;
    String traceback;

    std::atomic<bool> hot_reload_enabled;
    std::atomic<f32> reload_interval;

public:
    void SetArgs(int argc, const char** argv);
    inline const Slice<String>& GetArgs() const { return args; };

    inline int GetArgc() const { return argc; };
    inline const char** GetArgv() const { return argv; };

public:
    void Init();
    void Fini();

    MountResult LoadVFS(const_str path);
    void UnLoadVFS();

    bool InitLuaBase();
    void FiniLuaBase();
};

i32 keyboard_lookup(String str);

}  // namespace Neko

using Neko::CBase;

extern CBase gBase;

i32 neko_buildnum(void);

inline void fatal_error(Neko::String str) {
    if (!gBase.error_mode.load()) {
        Neko::LockGuard<Neko::Mutex> lock{gBase.error_mtx};

        gBase.fatal_error = to_cstr(str);
        fprintf(stderr, "%s\n", gBase.fatal_error.data);
        gBase.error_mode.store(true);
    }
}
