
#ifndef GAME_SCRIPTING_H
#define GAME_SCRIPTING_H

#include "engine/neko.h"
#include "engine/util/neko_vm.h"

typedef struct game_scripting_s {
    neko_script_ctx_t *ctx;
    neko_script_vector(neko_script_binary_t *) modules;
} game_scripting_t;

#endif