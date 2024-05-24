
#include "neko.h"
#include "neko_engine.h"

s32 main(s32 argv, char **argc) {
    neko_t *inst = neko_create(neko_main(argv, argc));
    while (neko_app()->is_running) {
        neko_frame();
    }
    neko_free(inst);
    return 0;
}