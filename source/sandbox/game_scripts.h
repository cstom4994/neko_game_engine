
#ifndef GAME_SCRIPTS_H
#define GAME_SCRIPTS_H

#include "engine/neko.h"
#include "engine/util/neko_vm.h"
#include "sandbox/neko_csharp_native.h"

typedef struct game_vm_s {
    neko_script_ctx_t *ctx;
    neko_script_vector(neko_script_binary_t *) modules;
} game_vm_t;

class game_csharp {

public:
    void init(std::string _managed_path);
    void shutdown();

    void update();

    void hotfix();

private:
    neko_cs::HostInstance host_instance;
    neko_cs::AssemblyLoadContext load_context;

    neko_cs::ManagedObject game_instance;

    std::string managed_path;
};

#endif