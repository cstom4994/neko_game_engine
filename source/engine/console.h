#ifndef CONSOLE_H
#define CONSOLE_H

#include "engine/base.h"
#include "engine/ecs.h"
#include "engine/prelude.h"

SCRIPT(console,

       // 打印到此实体 添加到 gui_text 系统 (如果尚未在其中) 设置为entity_nil以禁用
       NEKO_EXPORT void console_set_entity(Entity ent);

       NEKO_EXPORT Entity console_get_entity();  // 如果没有设置则entity_nil

       NEKO_EXPORT void console_set_visible(bool visible);

       NEKO_EXPORT bool console_get_visible();

       NEKO_EXPORT void console_puts(const char *s);

       NEKO_EXPORT void console_printf(const char *fmt, ...);

)

void console_init();
void console_fini();

#endif
