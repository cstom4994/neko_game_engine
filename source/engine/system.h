#ifndef SYSTEM_H
#define SYSTEM_H

#include "engine/base.h"
#include "engine/prelude.h"

NEKO_SCRIPT(system,

       NEKO_EXPORT void system_load_all(Store *f);
       NEKO_EXPORT void system_save_all(Store *f);

)

void system_init();
void system_fini();
void system_update_all();
void system_draw_all();

#endif
