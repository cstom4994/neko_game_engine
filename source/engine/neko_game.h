#ifndef GAME_H
#define GAME_H

#include "glew_glfw.h"
#include "neko_base.h"
#include "script_export.h"

#define NEKO_C_EXTERN extern "C"

extern GLFWwindow *game_window;

// 入口点
void game_run(int argc, char **argv);

// 获取 argc argv 传递给 game_run(...)
int game_get_argc();
char **game_get_argv();

SCRIPT(game,

       NEKO_EXPORT void game_set_bg_color(Color c);

       // 屏幕空间坐标系:
       // unit: (0, 0) 中间, (1, 1) 右上
       // pixels: (0, 0) 左上方, game_get_window_size() 右下角
       NEKO_EXPORT void game_set_window_size(CVec2 s);  // width, height in pixels
       NEKO_EXPORT CVec2 game_get_window_size(); 
       NEKO_EXPORT CVec2 game_unit_to_pixels(CVec2 p); 
       NEKO_EXPORT CVec2 game_pixels_to_unit(CVec2 p);

       NEKO_EXPORT void game_quit();

)

#endif
