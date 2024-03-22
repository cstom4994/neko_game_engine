
#ifndef GAME_SCRIPTS_H
#define GAME_SCRIPTS_H

#include "engine/neko.h"
#include "engine/util/neko_script.h"

typedef struct game_vm_s {
    neko_script_ctx_t *ctx;
    neko_script_vector(neko_script_binary_t *) modules;
} game_vm_t;

#endif