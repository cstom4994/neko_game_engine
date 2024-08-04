//

#include "engine/api_core.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/game.h"
#include "engine/math.h"
#include "engine/os.h"
#include "engine/prelude.h"

int neko_api_get_monitor_count(int *val) {
    // TODO
    (void)val;
    return 0;
}

int neko_api_get_monitor_name(int index, const char **val) {
    // TODO
    (void)index;
    (void)val;
    return 0;
}

int neko_api_get_monitor_width(int index, int *val) {
    // TODO
    (void)index;
    (void)val;
    return 0;
}

int neko_api_get_monitor_height(int index, int *val) {
    // TODO
    return 0;
}

int neko_api_set_window_monitor(int index) {
    // TODO
    (void)index;
    return 0;
}

int neko_api_set_window_resizable(bool resizable) { return 0; }

int neko_api_set_window_minsize(int width, int height) {
    glfwSetWindowSizeLimits(g_app->game_window, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
    return 0;
}

int neko_api_set_window_size(int width, int height) {
    glfwSetWindowSize(g_app->game_window, width, height);
    return 0;
}

int neko_api_get_window_width(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (g_app->game_window) {
        glfwGetWindowSize(g_app->game_window, &w, &h);
    }
#endif
    *val = w;
    return 0;
}

int neko_api_get_window_height(int *val) {
    int w = 0;
    int h = 0;
#if defined(__EMSCRIPTEN__)
    w = emsc_width();
    h = emsc_height();
#else
    if (g_app->game_window) {
        glfwGetWindowSize(g_app->game_window, &w, &h);
    }
#endif
    *val = h;
    return 0;
}

int neko_api_set_window_position(int x, int y) {
#if !defined(__EMSCRIPTEN__)
    if (g_app->game_window) {
        glfwSetWindowPos(g_app->game_window, x, y);
    }
#endif
    return 0;
}

static int get_current_monitor(GLFWmonitor **monitor, GLFWwindow *window) {
    int winpos[2] = {0};
    glfwGetWindowPos(window, &winpos[0], &winpos[1]);

    int monitors_size = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitors_size);

    for (int i = 0; i < monitors_size; ++i) {
        int monitorpos[2] = {0};
        glfwGetMonitorPos(monitors[i], &monitorpos[0], &monitorpos[1]);
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitors[i]);
        if (winpos[0] >= monitorpos[0] && winpos[0] < (monitorpos[0] + vidmode->width) && winpos[1] >= monitorpos[1] && winpos[1] < (monitorpos[1] + vidmode->height)) {
            *monitor = monitors[i];
            return 0;
        }
    }

    return 1;
}

int neko_api_set_fullscreen(bool fullscreen) { return 0; }

int neko_api_is_fullscreen(bool *val) { return 0; }

int neko_api_set_window_title(const char *title) {
#if defined(__EMSCRIPTEN__)
    emscripten_set_window_title(title);
#else
    glfwSetWindowTitle(g_app->game_window, title);
#endif
    return 0;
}

int neko_api_set_window_icon_file(const char *path) {
    // PHYSFS_File *file = PHYSFS_openRead(path);
    // if (file == NULL) {
    //     int errcode = PHYSFS_getLastErrorCode();
    //     const char *errstr = PHYSFS_getErrorByCode(errcode);
    //     fprintf(stderr, "\nFile '%s': '%s'\n", path, errstr);
    //     return errcode;
    // }
    // size_t len = PHYSFS_fileLength(file);
    // uint8_t *buf = malloc(len);
    // size_t read_len = PHYSFS_readBytes(file, buf, len);
    // if (len != read_len) {
    //     fprintf(stderr, "\nFile not fully read. Path: %s. File size is %zu bytes, but read %zu bytes.\n", path, len, read_len);
    //     return 1;
    // }
    // int width;
    // int height;
    // int channels;
    // uint8_t *data = stbi_load_from_memory(buf, read_len, &width, &height, &channels, 4);
    // if (!data) {
    //     fprintf(stderr, "\nImage file failed to load: %s\n", path);
    //     return 2;
    // }

    // GLFWimage images[1];
    // images[0].pixels = stbi_load_from_memory(buf, read_len, &images[0].width, &images[0].height, 0, 4);
    // glfwSetWindowIcon(g_app->game_window, 1, images);

    // stbi_image_free(data);
    // free(buf);
    // PHYSFS_close(file);
    return 0;
}

int neko_api_set_window_vsync(bool vsync) {
    if (vsync) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    return 0;
}

int neko_api_is_window_vsync(bool *val) { return 0; }

int neko_api_set_window_margins(int left, int right, int top, int bottom) { return 0; }

int neko_api_set_window_paddings(int left, int right, int top, int bottom) { return 0; }
