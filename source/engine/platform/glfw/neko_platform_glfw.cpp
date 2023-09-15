
#include <filesystem>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_mem.h"
#include "engine/platform/neko_platform.h"
#include "engine/utility/logger.hpp"

// opengl
#include "libs/glad/glad.h"

// glfw
#include "GLFW/glfw3.h"

#ifdef NEKO_PLATFORM_WIN
#include <Psapi.h>  // windows GetProcessMemoryInfo
#include <Windows.h>


#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#endif  // NEKO_PLATFORM_WIN

// Forward Decls.
void __glfw_key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods);
void __glfw_mouse_button_callback(GLFWwindow *window, s32 button, s32 action, s32 mods);
void __glfw_mouse_cursor_position_callback(GLFWwindow *window, f64 x, f64 y);
void __glfw_mouse_scroll_wheel_callback(GLFWwindow *window, f64 xoffset, f64 yoffset);
void __glfw_mouse_cursor_enter_callback(GLFWwindow *window, s32 entered);
void __glfw_frame_buffer_size_callback(GLFWwindow *window, s32 width, s32 height);
void __glfw_drop_callback(GLFWwindow *window);

#define __window_from_handle(platform, handle) ((GLFWwindow *)(neko_slot_array_get((platform)->windows, (handle))))

/*============================
// Platform Initialization
============================*/

neko_result glfw_platform_init(struct neko_platform_i *platform) {
    neko_info("Initializing GLFW");

#ifdef NEKO_PLATFORM_WIN
    SetConsoleOutputCP(65001);
#endif

    glfwInit();

    // Verify platform is valid
    neko_assert(platform);

    switch (platform->settings.video.driver) {
        case neko_platform_video_driver_type_opengl: {
#if (defined NEKO_PLATFORM_APPLE)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#else
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, platform->settings.video.graphics.opengl.major_version);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, platform->settings.video.graphics.opengl.minor_version);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

        } break;

        default: {
            // Default to no output at all.
            neko_error("Video format not supported.");
        } break;
    }

    // Construct cursors
    platform->cursors[(u32)neko_platform_cursor_arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_ibeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_size_nw_se] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_size_ne_sw] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_size_ns] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_size_we] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_size_all] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    platform->cursors[(u32)neko_platform_cursor_no] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    // 确定引擎运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 3; ++i) {
        if (std::filesystem::exists(current_dir / "data") && std::filesystem::exists(current_dir / "data" / "scripts")) {
            neko_string tmp = neko_fs_normalize_path(current_dir.string());
            std::strcpy(platform->ctx.gamepath, tmp.c_str());
            neko_info(std::format("game data path detected: {0} (base: {1})", platform->ctx.gamepath, std::filesystem::current_path().string()));
            break;
        }
        current_dir = current_dir.parent_path();
    }

    return neko_result_success;
}

neko_result glfw_platform_shutdown(struct neko_platform_i *platform) { return neko_result_success; }

/*============================
// Platform Util
============================*/

// Returns in milliseconds
f64 glfw_platform_time() { return (glfwGetTime() * 1000.0); }

#if (defined NEKO_PLATFORM_APPLE || defined NEKO_PLATFORM_LINUX)

#include <sched.h>
#include <unistd.h>

#elif (defined NEKO_PLATFORM_WIN)

#include <windows.h>
#endif

void glfw_platform_sleep(f32 ms) {
#if (defined NEKO_PLATFORM_WIN)

    Sleep(ms);

#elif (defined NEKO_PLATFORM_APPLE)

    usleep(ms * 1000.f);  // unistd.h
#else

    if (ms < 0.f) {
        return;
    }
    struct timespec ts = gs_default_val();
    int32_t res = 0;
    ts.tv_sec = ms / 1000.f;
    ts.tv_nsec = ((uint64_t)ms % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    // usleep(ms * 1000.f); // unistd.h
#endif
}

neko_platform_meminfo glfw_platform_get_meminfo() {
    neko_platform_meminfo meminfo = {};

#ifdef NEKO_PLATFORM_WIN
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc))) {
        meminfo.virtual_memory_used = pmc.PrivateUsage;
        meminfo.physical_memory_used = pmc.WorkingSetSize;
        meminfo.peak_virtual_memory_used = pmc.PeakPagefileUsage;
        meminfo.peak_physical_memory_used = pmc.PeakWorkingSetSize;

    } else {
    }
#endif

    return meminfo;
}

neko_platform_keycode glfw_key_to_neko_keycode(u32 code) {
    switch (code) {
        case GLFW_KEY_A:
            return neko_keycode_a;
            break;
        case GLFW_KEY_B:
            return neko_keycode_b;
            break;
        case GLFW_KEY_C:
            return neko_keycode_c;
            break;
        case GLFW_KEY_D:
            return neko_keycode_d;
            break;
        case GLFW_KEY_E:
            return neko_keycode_e;
            break;
        case GLFW_KEY_F:
            return neko_keycode_f;
            break;
        case GLFW_KEY_G:
            return neko_keycode_g;
            break;
        case GLFW_KEY_H:
            return neko_keycode_h;
            break;
        case GLFW_KEY_I:
            return neko_keycode_i;
            break;
        case GLFW_KEY_J:
            return neko_keycode_j;
            break;
        case GLFW_KEY_K:
            return neko_keycode_k;
            break;
        case GLFW_KEY_L:
            return neko_keycode_l;
            break;
        case GLFW_KEY_M:
            return neko_keycode_m;
            break;
        case GLFW_KEY_N:
            return neko_keycode_n;
            break;
        case GLFW_KEY_O:
            return neko_keycode_o;
            break;
        case GLFW_KEY_P:
            return neko_keycode_p;
            break;
        case GLFW_KEY_Q:
            return neko_keycode_q;
            break;
        case GLFW_KEY_R:
            return neko_keycode_r;
            break;
        case GLFW_KEY_S:
            return neko_keycode_s;
            break;
        case GLFW_KEY_T:
            return neko_keycode_t;
            break;
        case GLFW_KEY_U:
            return neko_keycode_u;
            break;
        case GLFW_KEY_V:
            return neko_keycode_v;
            break;
        case GLFW_KEY_W:
            return neko_keycode_w;
            break;
        case GLFW_KEY_X:
            return neko_keycode_x;
            break;
        case GLFW_KEY_Y:
            return neko_keycode_y;
            break;
        case GLFW_KEY_Z:
            return neko_keycode_z;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            return neko_keycode_lshift;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
            return neko_keycode_rshift;
            break;
        case GLFW_KEY_LEFT_ALT:
            return neko_keycode_lalt;
            break;
        case GLFW_KEY_RIGHT_ALT:
            return neko_keycode_ralt;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            return neko_keycode_lctrl;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
            return neko_keycode_rctrl;
            break;
        case GLFW_KEY_BACKSPACE:
            return neko_keycode_bspace;
            break;
        case GLFW_KEY_BACKSLASH:
            return neko_keycode_bslash;
            break;
        case GLFW_KEY_SLASH:
            return neko_keycode_qmark;
            break;
        case GLFW_KEY_GRAVE_ACCENT:
            return neko_keycode_tilde;
            break;
        case GLFW_KEY_COMMA:
            return neko_keycode_comma;
            break;
        case GLFW_KEY_PERIOD:
            return neko_keycode_period;
            break;
        case GLFW_KEY_ESCAPE:
            return neko_keycode_esc;
            break;
        case GLFW_KEY_SPACE:
            return neko_keycode_space;
            break;
        case GLFW_KEY_LEFT:
            return neko_keycode_left;
            break;
        case GLFW_KEY_UP:
            return neko_keycode_up;
            break;
        case GLFW_KEY_RIGHT:
            return neko_keycode_right;
            break;
        case GLFW_KEY_DOWN:
            return neko_keycode_down;
            break;
        case GLFW_KEY_0:
            return neko_keycode_zero;
            break;
        case GLFW_KEY_1:
            return neko_keycode_one;
            break;
        case GLFW_KEY_2:
            return neko_keycode_two;
            break;
        case GLFW_KEY_3:
            return neko_keycode_three;
            break;
        case GLFW_KEY_4:
            return neko_keycode_four;
            break;
        case GLFW_KEY_5:
            return neko_keycode_five;
            break;
        case GLFW_KEY_6:
            return neko_keycode_six;
            break;
        case GLFW_KEY_7:
            return neko_keycode_seven;
            break;
        case GLFW_KEY_8:
            return neko_keycode_eight;
            break;
        case GLFW_KEY_9:
            return neko_keycode_nine;
            break;
        case GLFW_KEY_KP_0:
            return neko_keycode_npzero;
            break;
        case GLFW_KEY_KP_1:
            return neko_keycode_npone;
            break;
        case GLFW_KEY_KP_2:
            return neko_keycode_nptwo;
            break;
        case GLFW_KEY_KP_3:
            return neko_keycode_npthree;
            break;
        case GLFW_KEY_KP_4:
            return neko_keycode_npfour;
            break;
        case GLFW_KEY_KP_5:
            return neko_keycode_npfive;
            break;
        case GLFW_KEY_KP_6:
            return neko_keycode_npsix;
            break;
        case GLFW_KEY_KP_7:
            return neko_keycode_npseven;
            break;
        case GLFW_KEY_KP_8:
            return neko_keycode_npeight;
            break;
        case GLFW_KEY_KP_9:
            return neko_keycode_npnine;
            break;
        case GLFW_KEY_CAPS_LOCK:
            return neko_keycode_caps;
            break;
        case GLFW_KEY_DELETE:
            return neko_keycode_delete;
            break;
        case GLFW_KEY_END:
            return neko_keycode_end;
            break;
        case GLFW_KEY_F1:
            return neko_keycode_f1;
            break;
        case GLFW_KEY_F2:
            return neko_keycode_f2;
            break;
        case GLFW_KEY_F3:
            return neko_keycode_f3;
            break;
        case GLFW_KEY_F4:
            return neko_keycode_f4;
            break;
        case GLFW_KEY_F5:
            return neko_keycode_f5;
            break;
        case GLFW_KEY_F6:
            return neko_keycode_f6;
            break;
        case GLFW_KEY_F7:
            return neko_keycode_f7;
            break;
        case GLFW_KEY_F8:
            return neko_keycode_f8;
            break;
        case GLFW_KEY_F9:
            return neko_keycode_f9;
            break;
        case GLFW_KEY_F10:
            return neko_keycode_f10;
            break;
        case GLFW_KEY_F11:
            return neko_keycode_f11;
            break;
        case GLFW_KEY_F12:
            return neko_keycode_f12;
            break;
        case GLFW_KEY_HOME:
            return neko_keycode_home;
            break;
        case GLFW_KEY_EQUAL:
            return neko_keycode_plus;
            break;
        case GLFW_KEY_MINUS:
            return neko_keycode_minus;
            break;
        case GLFW_KEY_LEFT_BRACKET:
            return neko_keycode_lbracket;
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            return neko_keycode_rbracket;
            break;
        case GLFW_KEY_SEMICOLON:
            return neko_keycode_semi_colon;
            break;
        case GLFW_KEY_ENTER:
            return neko_keycode_enter;
            break;
        case GLFW_KEY_INSERT:
            return neko_keycode_insert;
            break;
        case GLFW_KEY_PAGE_UP:
            return neko_keycode_pgup;
            break;
        case GLFW_KEY_PAGE_DOWN:
            return neko_keycode_pgdown;
            break;
        case GLFW_KEY_NUM_LOCK:
            return neko_keycode_numlock;
            break;
        case GLFW_KEY_TAB:
            return neko_keycode_tab;
            break;
        case GLFW_KEY_KP_MULTIPLY:
            return neko_keycode_npmult;
            break;
        case GLFW_KEY_KP_DIVIDE:
            return neko_keycode_npdiv;
            break;
        case GLFW_KEY_KP_ADD:
            return neko_keycode_npplus;
            break;
        case GLFW_KEY_KP_SUBTRACT:
            return neko_keycode_npminus;
            break;
        case GLFW_KEY_KP_ENTER:
            return neko_keycode_npenter;
            break;
        case GLFW_KEY_KP_DECIMAL:
            return neko_keycode_npdel;
            break;
        case GLFW_KEY_PAUSE:
            return neko_keycode_pause;
            break;
        case GLFW_KEY_PRINT_SCREEN:
            return neko_keycode_print;
            break;
        case GLFW_KEY_UNKNOWN:
            return neko_keycode_count;
            break;
    }

    // Shouldn't reach here
    return neko_keycode_count;
}

neko_platform_mouse_button_code __glfw_button_to_neko_mouse_button(s32 code) {
    switch (code) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return neko_mouse_lbutton;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return neko_mouse_rbutton;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return neko_mouse_mbutton;
            break;
    }

    // Shouldn't reach here
    return neko_mouse_button_code_count;
}

void __glfw_key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
    // Grab platform instance from engine
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Get keycode from key
    neko_platform_keycode code = glfw_key_to_neko_keycode(key);

    switch (action) {
            // Released
        case 0: {
            platform->release_key(code);
        } break;

            // Pressed
        case 1: {
            platform->press_key(code);
        } break;

        default: {
        } break;
    }
}

void __glfw_mouse_button_callback(GLFWwindow *window, s32 button, s32 action, s32 mods) {
    // Grab platform instance from engine
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;

    // Get mouse code from key
    neko_platform_mouse_button_code code = __glfw_button_to_neko_mouse_button(button);

    switch (action) {
            // Released
        case 0: {
            platform->release_mouse_button(code);
        } break;

        // Pressed
        case 1: {
            platform->press_mouse_button(code);
        } break;
    }
}

void __glfw_mouse_cursor_position_callback(GLFWwindow *window, f64 x, f64 y) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    platform->input.mouse.position = neko_vec2{(f32)x, (f32)y};
    platform->input.mouse.moved_this_frame = true;
}

void __glfw_mouse_scroll_wheel_callback(GLFWwindow *window, f64 x, f64 y) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    platform->input.mouse.wheel = neko_vec2{(f32)x, (f32)y};
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback(GLFWwindow *window, s32 entered) {
    // Nothing for now, will capture state for windows later
}

void __glfw_frame_buffer_size_callback(GLFWwindow *window, s32 width, s32 height) {
    struct neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
    if (gfx) {
        // gfx->set_viewport(cb, 0.f, 0.f, width, height);
        // glViewport(0, 0, width, height);
    }
}

// 设置 glfw 在屏幕中央
bool __glfw_set_window_center(GLFWwindow *window) {
    if (!window) return false;

    int sx = 0, sy = 0;
    int px = 0, py = 0;
    int mx = 0, my = 0;
    int monitor_count = 0;
    int best_area = 0;
    int final_x = 0, final_y = 0;

    glfwGetWindowSize(window, &sx, &sy);
    glfwGetWindowPos(window, &px, &py);

    // Iterate throug all monitors
    GLFWmonitor **m = glfwGetMonitors(&monitor_count);
    if (!m) return false;

    for (int j = 0; j < monitor_count; ++j) {

        glfwGetMonitorPos(m[j], &mx, &my);
        const GLFWvidmode *mode = glfwGetVideoMode(m[j]);
        if (!mode) continue;

        // Get intersection of two rectangles - screen and window
        int minX = neko_max(mx, px);
        int minY = neko_max(my, py);

        int maxX = neko_min(mx + mode->width, px + sx);
        int maxY = neko_min(my + mode->height, py + sy);

        // Calculate area of the intersection
        int area = neko_max(maxX - minX, 0) * neko_max(maxY - minY, 0);

        // If its bigger than actual (window covers more space on this monitor)
        if (area > best_area) {
            // Calculate proper position in this monitor
            final_x = mx + (mode->width - sx) / 2;
            final_y = my + (mode->height - sy) / 2;

            best_area = area;
        }
    }

    // We found something
    if (best_area) glfwSetWindowPos(window, final_x, final_y);

    // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
    else {
        GLFWmonitor *primary = glfwGetPrimaryMonitor();
        if (primary) {
            const GLFWvidmode *desktop = glfwGetVideoMode(primary);

            if (desktop)
                glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
            else
                return false;
        } else
            return false;
    }

    return true;
}

neko_result glfw_process_input(struct neko_platform_input *input) {
    glfwPollEvents();

    return neko_result_in_progress;
}

void *glfw_create_window(const char *title, u32 width, u32 height) {
    // Grab window hints from application desc
    neko_window_flags window_hints = neko_engine_instance()->ctx.app.window_flags;

    // Set whether or not the screen is resizable
    glfwWindowHint(GLFW_RESIZABLE, (window_hints & neko_window_flags::resizable) == neko_window_flags::resizable);

    // 禁用高 DPI 缩放
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, (window_hints & neko_window_flags::highdpi) == neko_window_flags::highdpi);

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) {
        neko_error("Failed to create window.");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, &__glfw_key_callback);
    glfwSetMouseButtonCallback(window, &__glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, &__glfw_mouse_cursor_position_callback);
    glfwSetScrollCallback(window, &__glfw_mouse_scroll_wheel_callback);
    glfwSetCursorEnterCallback(window, &__glfw_mouse_cursor_enter_callback);
    glfwSetFramebufferSizeCallback(window, &__glfw_frame_buffer_size_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        neko_error("Failed to initialize OpenGL.");
        return NULL;
    }

    __glfw_set_window_center(window);

    return window;
}

void *glfw_raw_window_handle(neko_resource_handle handle) {
    // Grab instance of platform from engine
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    neko_assert(platform);

    // Grab window from handle
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);

    return (void *)win;
}

void glfw_window_swap_buffer(neko_resource_handle handle) {
    // Grab instance of platform from engine
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    neko_assert(platform);

    // Grab window from handle
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);

    glfwSwapBuffers(win);
}

void glfw_set_window_size(neko_resource_handle handle, s32 w, s32 h) {
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);
    glfwSetWindowSize(win, w, h);
}

neko_vec2 glfw_window_size(neko_resource_handle handle) {
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);
    s32 w, h;
    glfwGetWindowSize(win, &w, &h);

    return neko_vec2{(f32)w, (f32)h};
}

void glfw_window_size_w_h(neko_resource_handle handle, s32 *w, s32 *h) {
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);
    glfwGetWindowSize(win, w, h);
}

void glfw_frame_buffer_size_w_h(neko_resource_handle handle, s32 *w, s32 *h) {
    GLFWwindow *win = __window_from_handle(neko_engine_instance()->ctx.platform, handle);
    neko_assert(win);
    f32 xscale = 0.f, yscale = 0.f;
    glfwGetWindowContentScale(win, &xscale, &yscale);
    glfwGetWindowSize(win, w, h);
    *w = (s32)((f32)*w * xscale);
    *h = (s32)((f32)*h * yscale);
}

neko_vec2 glfw_frame_buffer_size(neko_resource_handle handle) {
    s32 w = 0, h = 0;
    glfw_frame_buffer_size_w_h(handle, &w, &h);
    return neko_vec2{(f32)w, (f32)h};
}

void glfw_set_cursor(neko_resource_handle handle, neko_platform_cursor cursor) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, handle);
    GLFWcursor *cp = ((GLFWcursor *)platform->cursors[(u32)cursor]);
    glfwSetCursor(win, cp);
}

void glfw_set_vsync_enabled(bool enabled) { glfwSwapInterval(enabled ? 1 : 0); }

void glfw_set_dropped_files_callback(neko_resource_handle handle, dropped_files_callback_t callback) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, handle);
    glfwSetDropCallback(win, (GLFWdropfun)callback);
}

void glfw_set_window_close_callback(neko_resource_handle handle, window_close_callback_t callback) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, handle);
    glfwSetWindowCloseCallback(win, (GLFWwindowclosefun)callback);
}

void glfw_set_cursor_position(neko_resource_handle handle, f64 x, f64 y) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, handle);
    glfwSetCursorPos(win, x, y);
}

neko_vec2 glfw_get_opengl_version() {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, neko_engine_instance()->ctx.platform->main_window());
    int major = glfwGetWindowAttrib(win, GLFW_CONTEXT_VERSION_MAJOR);
    int minor = glfwGetWindowAttrib(win, GLFW_CONTEXT_VERSION_MINOR);
    return {(f32)major, (f32)minor};
}

void *glfw_get_sys_handle(neko_resource_handle handle) {
    struct neko_platform_i *platform = neko_engine_instance()->ctx.platform;
    GLFWwindow *win = __window_from_handle(platform, handle);
    HWND hwnd = glfwGetWin32Window(win);
    return hwnd;
}

// Method for creating platform layer for GLFW
struct neko_platform_i *neko_platform_construct() {
    // Construct new platform interface
    neko_malloc_init_ex(platform, neko_platform_i);

    /*
        Initialize platform interface with all appropriate function pointers
    */

    /*============================
    // Platform Initialization
    ============================*/
    platform->init = &glfw_platform_init;
    platform->shutdown = &glfw_platform_shutdown;

    /*============================
    // Platform Util
    ============================*/
    platform->sleep = &glfw_platform_sleep;
    platform->elapsed_time = &glfw_platform_time;
    platform->get_meminfo = &glfw_platform_get_meminfo;
    platform->get_opengl_ver = &glfw_get_opengl_version;
    platform->get_sys_handle = &glfw_get_sys_handle;

    /*============================
    // Platform Video
    ============================*/
    platform->enable_vsync = &glfw_set_vsync_enabled;

    /*============================
    // Platform Input
    ============================*/
    platform->process_input = &glfw_process_input;
    platform->set_mouse_position = &glfw_set_cursor_position;

    /*============================
    // Platform Window
    ============================*/
    platform->create_window_internal = &glfw_create_window;
    platform->window_swap_buffer = &glfw_window_swap_buffer;
    platform->window_size = &glfw_window_size;
    platform->window_size_w_h = &glfw_window_size_w_h;
    platform->set_window_size = &glfw_set_window_size;
    platform->set_cursor = &glfw_set_cursor;
    platform->set_dropped_files_callback = &glfw_set_dropped_files_callback;
    platform->set_window_close_callback = &glfw_set_window_close_callback;
    platform->raw_window_handle = &glfw_raw_window_handle;
    platform->frame_buffer_size = &glfw_frame_buffer_size;
    platform->frame_buffer_size_w_h = &glfw_frame_buffer_size_w_h;

    // TODO:
    platform->settings.video.driver = neko_platform_video_driver_type_opengl;
    platform->settings.video.graphics.opengl.major_version = 4;
    platform->settings.video.graphics.opengl.minor_version = 6;

    return platform;
}
