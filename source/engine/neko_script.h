#ifndef SCRIPT_H
#define SCRIPT_H

#include "engine/neko_base.h"
#include "engine/neko_input.h"
#include "engine/neko_prelude.h"

void script_run_string(const char *s);
void script_run_file(const char *filename);
void script_error(const char *s);

void script_init();
void script_deinit();
void script_update_all();
void script_post_update_all();
void script_draw_all();
void script_key_down(KeyCode key);
void script_key_up(KeyCode key);
void script_mouse_down(MouseCode mouse);
void script_mouse_up(MouseCode mouse);
void script_mouse_move(CVec2 pos);
void script_scroll(CVec2 scroll);
void script_save_all(Store *s);
void script_load_all(Store *s);

#endif
