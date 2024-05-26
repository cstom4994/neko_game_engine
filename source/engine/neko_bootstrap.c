
#include "neko.h"
#include "neko_engine.h"

s32 main(s32 argv, char **argc) {
    neko_t *inst = neko_create(argv, argc);
    while (neko_instance()->game.is_running) {
        neko_frame();
    }
    neko_free(inst);
    return 0;
}