
#ifndef NEKO_API_CORE_H
#define NEKO_API_CORE_H

#include <cstddef>
#include <cstdint>

#include "engine/neko_prelude.h"

int neko_api_core_window_init(void);
int neko_api_core_window_cleanup(void);

int neko_api_get_monitor_count(int *val);
int neko_api_get_monitor_name(int index, const char **val);
int neko_api_get_monitor_width(int index, int *val);
int neko_api_get_monitor_height(int index, int *val);
int neko_api_set_window_monitor(int index);
int neko_api_set_window_resizable(bool resizable);
int neko_api_set_window_minsize(int width, int height);
int neko_api_set_window_size(int width, int height);
int neko_api_get_window_width(int *val);
int neko_api_get_window_height(int *val);
int neko_api_set_window_position(int x, int y);
int neko_api_set_fullscreen(bool fullscreen);
int neko_api_is_fullscreen(bool *val);
int neko_api_set_window_title(const char *title);
int neko_api_set_window_icon_file(const char *icon_path);
int neko_api_set_window_vsync(bool vsync);
int neko_api_is_window_vsync(bool *val);
int neko_api_set_window_margins(int left, int right, int top, int bottom);
int neko_api_set_window_paddings(int left, int right, int top, int bottom);

#endif