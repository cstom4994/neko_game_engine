
#ifndef GAME_SCRIPTS_H
#define GAME_SCRIPTS_H

#include "engine/neko.h"
#include "engine/neko_csharp_native.h"
#include "engine/util/neko_vm.h"

typedef struct game_vm_s {
    neko_script_ctx_t *ctx;
    neko_script_vector(neko_script_binary_t *) modules;
} game_vm_t;

class game_csharp {

public:
    void init(std::string managed_path);
    void shutdown();

private:
    neko_cs::HostInstance hostInstance;
    neko_cs::AssemblyLoadContext loadContext;
};

#endif