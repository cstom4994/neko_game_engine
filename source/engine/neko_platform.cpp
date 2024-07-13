

/*=============================
// NEKO_PF_IMPL
=============================*/

#include "engine/neko_platform.h"

#include "engine/neko.h"
#include "engine/neko_api.hpp"
#include "engine/neko_engine.h"

#ifndef NEKO_PF_IMPL_CUSTOM
#if (defined NEKO_PF_WIN || defined NEKO_PF_APPLE || defined NEKO_PF_LINUX)
#define NEKO_PF_IMPL_GLFW
#elif (defined NEKO_PF_WEB)
#define NEKO_PF_IMPL_EMSCRIPTEN
#endif
#endif

#ifdef NEKO_PF_IMPL_FILE
#include NEKO_PF_IMPL_FILE
#endif

#ifndef NEKO_PF_IMPL_H
#define NEKO_PF_IMPL_H

/*=================================
// Default Platform Implemenattion
=================================*/

// Define default platform implementation if certain platforms are enabled
#if (!defined NEKO_PF_IMPL_NO_DEFAULT)
#define NEKO_PF_IMPL_DEFAULT
#endif

/*=============================
// Default Impl
=============================*/

#ifdef NEKO_PF_IMPL_DEFAULT

#if !(defined NEKO_PF_WIN)
#include <dirent.h>
#include <dlfcn.h>  // dlopen, RTLD_LAZY, dlsym
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>  // for chdir
#include <utime.h>

#else

// direct.h
#include <direct.h>

#pragma comment(lib, "opengl32")  // glViewport
#pragma comment(lib, "shell32")   // CommandLineToArgvW
#pragma comment(lib, "user32")    // SetWindowLong
#pragma comment(lib, "DbgHelp")   // DbgHelp
#pragma comment(lib, "winmm")     // timeBeginPeriod
#pragma comment(lib, "ws2_32")    // socket

#endif

#if defined(NEKO_PF_APPLE)
#include <mach/mach.h>
#include <mach/task_info.h>
#endif

/*== Platform Window ==*/

NEKO_API_DECL neko_pf_t* neko_pf_create() {
    // Construct new platform interface
    neko_pf_t* platform = neko_malloc_init(neko_pf_t);

    // Initialize windows
    platform->windows = neko_slot_array_new(neko_pf_window_t);

    // neko_job_init();

    return platform;
}

NEKO_API_DECL void neko_pf_fini(neko_pf_t* platform) {
    if (platform == NULL) return;

    // #if defined(NEKO_PF_WIN)
    //     neko_thread_win32_fini();
    // #endif

    // Free all resources
    neko_slot_array_free(platform->windows);

    // Free platform
    neko_free(platform);
    platform = NULL;
}

NEKO_API_DECL u32 neko_pf_window_create(const neko_pf_running_desc_t* desc) {
    NEKO_ASSERT(neko_instance() != NULL);
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t win = neko_pf_window_create_internal(desc);

    // Insert and return handle
    return (neko_slot_array_insert(platform->windows, win));
}

NEKO_API_DECL u32 neko_pf_main_window() {
    // Should be the first element of the slot array...Great assumption to make.
    return 0;
}

/*== Platform Time ==*/

NEKO_API_DECL const neko_pf_time_t* neko_pf_time() { return &neko_subsystem(platform)->time; }

NEKO_API_DECL f32 neko_pf_delta_time() { return neko_pf_time()->delta; }

NEKO_API_DECL f32 neko_pf_frame_time() { return neko_pf_time()->frame; }

/*== Platform UUID ==*/

NEKO_API_DECL struct neko_uuid_t neko_pf_uuid_generate() {
    neko_uuid_t uuid;

    srand(clock());
    char guid[40];
    s32 t = 0;
    const char* sz_temp = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
    const char* sz_hex = "0123456789abcdef-";
    s32 n_len = (s32)strlen(sz_temp);

    for (t = 0; t < n_len + 1; t++) {
        s32 r = rand() % 16;
        char c = ' ';

        switch (sz_temp[t]) {
            case 'x': {
                c = sz_hex[r];
            } break;
            case 'y': {
                c = sz_hex[(r & 0x03) | 0x08];
            } break;
            case '-': {
                c = '-';
            } break;
            case '4': {
                c = '4';
            } break;
        }

        guid[t] = (t < n_len) ? c : 0x00;
    }

    // Convert to uuid bytes from string
    const char *hex_string = guid, *pos = hex_string;

    /* WARNING: no sanitization or error-checking whatsoever */
    for (size_t count = 0; count < 16; count++) {
        sscanf(pos, "%2hhx", &uuid.bytes[count]);
        pos += 2;
    }

    return uuid;
}

// Mutable temp buffer 'tmp_buffer'
NEKO_API_DECL void neko_pf_uuid_to_string(char* tmp_buffer, const neko_uuid_t* uuid) {
    neko_snprintf(tmp_buffer, 32, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", uuid->bytes[0], uuid->bytes[1], uuid->bytes[2], uuid->bytes[3], uuid->bytes[4], uuid->bytes[5],
                  uuid->bytes[6], uuid->bytes[7], uuid->bytes[8], uuid->bytes[9], uuid->bytes[10], uuid->bytes[11], uuid->bytes[12], uuid->bytes[13], uuid->bytes[14], uuid->bytes[15]);
}

u32 neko_pf_uuid_hash(const neko_uuid_t* uuid) {
    char temp_buffer[] = neko_uuid_temp_str_buffer();
    neko_pf_uuid_to_string(temp_buffer, uuid);
    return (neko_hash_str(temp_buffer));
}

#define __neko_input() (&neko_subsystem(platform)->input)

/*=== Platform Input ===*/

NEKO_API_DECL neko_pf_input_t* neko_pf_input() { return &neko_subsystem(platform)->input; }

void neko_pf_update_input(neko_pf_input_t* input) {
    // Update all input and mouse keys from previous frame
    // Previous key presses
    NEKO_FOR_RANGE_N(NEKO_KEYCODE_COUNT, i) { input->prev_key_map[i] = input->key_map[i]; }

    // Previous mouse button presses
    NEKO_FOR_RANGE_N(NEKO_MOUSE_BUTTON_CODE_COUNT, i) { input->mouse.prev_button_map[i] = input->mouse.button_map[i]; }

    input->mouse.wheel = neko_v2s(0.0f);
    input->mouse.delta = neko_v2s(0.f);
    input->mouse.moved_this_frame = false;

    // Update all touch deltas
    for (u32 i = 0; i < NEKO_PF_MAX_TOUCH; ++i) {
        input->touch.points[i].delta = neko_v2s(0.f);
        input->touch.points[i].down = input->touch.points[i].pressed;
    }
}

void neko_pf_poll_all_events() {
    neko_pf_t* platform = neko_subsystem(platform);

    platform->input.mouse.delta.x = 0;
    platform->input.mouse.delta.y = 0;

    // Iterate through events, don't consume
    neko_pf_event_t evt = NEKO_DEFAULT_VAL();
    while (neko_pf_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PF_EVENT_MOUSE: {
                switch (evt.mouse.action) {
                    case NEKO_PF_MOUSE_MOVE: {
                        // If locked, then movement amount will be applied to delta,
                        // otherwise set position
                        if (neko_pf_mouse_locked()) {
                            platform->input.mouse.delta = neko_vec2_add(platform->input.mouse.delta, evt.mouse.move);
                        } else {
                            platform->input.mouse.delta = neko_vec2_sub(evt.mouse.move, platform->input.mouse.position);
                            platform->input.mouse.position = evt.mouse.move;
                        }
                    } break;

                    case NEKO_PF_MOUSE_WHEEL: {
                        platform->input.mouse.wheel = evt.mouse.wheel;
                    } break;

                    case NEKO_PF_MOUSE_BUTTON_PRESSED: {
                        neko_pf_press_mouse_button(evt.mouse.button);
                    } break;

                    case NEKO_PF_MOUSE_BUTTON_RELEASED: {
                        neko_pf_release_mouse_button(evt.mouse.button);
                    } break;

                    case NEKO_PF_MOUSE_BUTTON_DOWN: {
                        neko_pf_press_mouse_button(evt.mouse.button);
                    } break;

                    case NEKO_PF_MOUSE_ENTER: {
                        // If there are user callbacks, could trigger them here
                    } break;

                    case NEKO_PF_MOUSE_LEAVE: {
                        // If there are user callbacks, could trigger them here
                    } break;
                }

            } break;

            case NEKO_PF_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PF_KEY_PRESSED: {
                        neko_pf_press_key(evt.key.keycode);
                    } break;

                    case NEKO_PF_KEY_DOWN: {
                        neko_pf_press_key(evt.key.keycode);
                    } break;

                    case NEKO_PF_KEY_RELEASED: {
                        neko_pf_release_key(evt.key.keycode);
                    } break;
                }

            } break;

            case NEKO_PF_EVENT_WINDOW: {
                switch (evt.window.action) {
                    default:
                        break;
                }

            } break;

            case NEKO_PF_EVENT_TOUCH: {
                neko_pf_point_event_data_t* point = &evt.touch.point;

                switch (evt.touch.action) {
                    case NEKO_PF_TOUCH_DOWN: {
                        uintptr_t id = point->id;
                        neko_vec2* pos = &point->position;
                        neko_vec2* p = &platform->input.touch.points[id].position;
                        neko_vec2* d = &platform->input.touch.points[id].delta;
                        neko_pf_press_touch(id);
                        *p = *pos;
                        neko_subsystem(platform)->input.touch.size++;
                    } break;

                    case NEKO_PF_TOUCH_UP: {
                        uintptr_t id = point->id;
                        neko_println("Releasing ID: %zu", id);
                        neko_pf_release_touch(id);
                        neko_subsystem(platform)->input.touch.size--;
                    } break;

                    case NEKO_PF_TOUCH_MOVE: {
                        uintptr_t id = point->id;
                        neko_vec2* pos = &point->position;
                        neko_vec2* p = &platform->input.touch.points[id].position;
                        neko_vec2* d = &platform->input.touch.points[id].delta;
                        neko_pf_press_touch(id);  // Not sure if this is causing issues...
                        *d = neko_vec2_sub(*pos, *p);
                        *p = *pos;
                    } break;

                    case NEKO_PF_TOUCH_CANCEL: {
                        uintptr_t id = point->id;
                        neko_pf_release_touch(id);
                        neko_subsystem(platform)->input.touch.size--;
                    } break;
                }
            } break;

            default:
                break;
        }
    }
}

void neko_pf_update(neko_pf_t* platform) {
    // neko_profiler_scope_begin(platform_update);

    // Update platform input from previous frame
    neko_pf_update_input(&platform->input);

    // Process input for this frame (user dependent update)
    neko_pf_process_input(&platform->input);

    // Poll all events
    neko_pf_poll_all_events();

    neko_pf_update_internal(platform);

    // neko_profiler_scope_end(platform_update);
}

bool neko_pf_poll_events(neko_pf_event_t* evt, b32 consume) {
    neko_pf_t* platform = neko_subsystem(platform);

    if (!evt) return false;
    if (neko_dyn_array_empty(platform->events)) return false;
    if (evt->idx >= neko_dyn_array_size(platform->events)) return false;

    if (consume) {
        // Back event
        *evt = neko_dyn_array_back(platform->events);
        // Pop back
        neko_dyn_array_pop(platform->events);
    } else {
        u32 idx = evt->idx;
        *evt = platform->events[idx++];
        evt->idx = idx;
    }

    return true;
}

void neko_pf_add_event(neko_pf_event_t* evt) {
    neko_pf_t* platform = neko_subsystem(platform);
    if (!evt) return;
    neko_dyn_array_push(platform->events, *evt);
}

bool neko_pf_was_key_down(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    return (input->prev_key_map[code]);
}

bool neko_pf_key_down(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    return (input->key_map[code]);
}

bool neko_pf_key_pressed(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    return (neko_pf_key_down(code) && !neko_pf_was_key_down(code));
}

bool neko_pf_key_released(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    return (neko_pf_was_key_down(code) && !neko_pf_key_down(code));
}

bool neko_pf_touch_down(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        return input->touch.points[idx].pressed;
    }
    return false;
}

bool neko_pf_touch_pressed(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        return (neko_pf_was_touch_down(idx) && !neko_pf_touch_down(idx));
    }
    return false;
}

bool neko_pf_touch_released(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        return (neko_pf_was_touch_down(idx) && !neko_pf_touch_down(idx));
    }
    return false;
}

bool neko_pf_was_mouse_down(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    return (input->mouse.prev_button_map[code]);
}

void neko_pf_press_mouse_button(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    if ((u32)code < (u32)NEKO_MOUSE_BUTTON_CODE_COUNT) {
        input->mouse.button_map[code] = true;
    }
}

void neko_pf_release_mouse_button(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    if ((u32)code < (u32)NEKO_MOUSE_BUTTON_CODE_COUNT) {
        input->mouse.button_map[code] = false;
    }
}

bool neko_pf_mouse_down(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    return (input->mouse.button_map[code]);
}

bool neko_pf_mouse_pressed(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    if (neko_pf_mouse_down(code) && !neko_pf_was_mouse_down(code)) {
        return true;
    }
    return false;
}

bool neko_pf_mouse_released(neko_pf_mouse_button_code code) {
    neko_pf_input_t* input = __neko_input();
    return (neko_pf_was_mouse_down(code) && !neko_pf_mouse_down(code));
}

bool neko_pf_mouse_moved() {
    neko_pf_input_t* input = __neko_input();
    return (input->mouse.delta.x != 0.f || input->mouse.delta.y != 0.f);
}

void neko_pf_mouse_delta(f32* x, f32* y) {
    neko_pf_input_t* input = __neko_input();
    *x = input->mouse.delta.x;
    *y = input->mouse.delta.y;
}

neko_vec2 neko_pf_mouse_deltav() {
    neko_pf_input_t* input = __neko_input();
    neko_vec2 delta = NEKO_DEFAULT_VAL();
    neko_pf_mouse_delta(&delta.x, &delta.y);
    return delta;
}

neko_vec2 neko_pf_mouse_positionv() {
    neko_pf_input_t* input = __neko_input();

    return neko_v2(input->mouse.position.x, input->mouse.position.y);
}

void neko_pf_mouse_position(s32* x, s32* y) {
    neko_pf_input_t* input = __neko_input();
    *x = (s32)input->mouse.position.x;
    *y = (s32)input->mouse.position.y;
}

void neko_pf_mouse_wheel(f32* x, f32* y) {
    neko_pf_input_t* input = __neko_input();
    *x = input->mouse.wheel.x;
    *y = input->mouse.wheel.y;
}

NEKO_API_DECL neko_vec2 neko_pf_mouse_wheelv() {
    neko_vec2 wheel = NEKO_DEFAULT_VAL();
    neko_pf_mouse_wheel(&wheel.x, &wheel.y);
    return wheel;
}

bool neko_pf_mouse_locked() { return (__neko_input())->mouse.locked; }

void neko_pf_touch_delta(u32 idx, f32* x, f32* y) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        *x = input->touch.points[idx].delta.x;
        *y = input->touch.points[idx].delta.y;
    }
}

neko_vec2 neko_pf_touch_deltav(u32 idx) {
    neko_vec2 delta = neko_v2s(0.f);
    neko_pf_touch_delta(idx, &delta.x, &delta.y);
    return delta;
}

void neko_pf_touch_position(u32 idx, f32* x, f32* y) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        *x = input->touch.points[idx].position.x;
        *y = input->touch.points[idx].position.y;
    }
}

neko_vec2 neko_pf_touch_positionv(u32 idx) {
    neko_vec2 p = NEKO_DEFAULT_VAL();
    neko_pf_touch_position(idx, &p.x, &p.y);
    return p;
}

void neko_pf_press_touch(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        input->touch.points[idx].pressed = true;
    }
}

void neko_pf_release_touch(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        neko_println("releasing: %zu", idx);
        input->touch.points[idx].pressed = false;
    }
}

bool neko_pf_was_touch_down(u32 idx) {
    neko_pf_input_t* input = __neko_input();
    if (idx < NEKO_PF_MAX_TOUCH) {
        return input->touch.points[idx].down;
    }
    return false;
}

void neko_pf_press_key(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    if (code < NEKO_KEYCODE_COUNT) {
        input->key_map[code] = true;
    }
}

void neko_pf_release_key(neko_pf_keycode code) {
    neko_pf_input_t* input = __neko_input();
    if (code < NEKO_KEYCODE_COUNT) {
        input->key_map[code] = false;
    }
}

// Platform File IO
char* neko_pf_read_file_contents(const char* file_path, const char* mode, size_t* sz) {

#ifdef NEKO_PF_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    char* buffer = 0;
    FILE* fp = neko_fopen(file_path, mode);
    size_t read_sz = 0;
    if (fp) {
        read_sz = neko_pf_file_size_in_bytes(file_path);
        buffer = (char*)neko_capi_safe_malloc(read_sz + 1);
        if (buffer) {
            size_t _r = fread(buffer, 1, read_sz, fp);
        }
        buffer[read_sz] = '\0';
        neko_fclose(fp);
        if (sz) *sz = read_sz;
    }

    // NEKO_WINDOWS_ConvertPath_end(path);

    return buffer;
}

neko_result neko_pf_write_file_contents(const char* file_path, const char* mode, void* data, size_t sz) {
    const char* path = file_path;

#ifdef NEKO_PF_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE* fp = neko_fopen(file_path, mode);
    if (fp) {
        size_t ret = fwrite(data, sizeof(u8), sz, fp);
        if (ret == sz) {
            neko_fclose(fp);
            return NEKO_RESULT_SUCCESS;
        }
        neko_fclose(fp);
    }
    return NEKO_RESULT_FAILURE;
}

NEKO_API_DECL bool neko_pf_dir_exists(const char* dir_path) {
#if defined(NEKO_PF_WIN)
    DWORD attrib = GetFileAttributes((LPCWSTR)dir_path);  // TODO: unicode 路径修复
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(NEKO_PF_LINUX) || defined(NEKO_PF_APPLE)
    struct stat st;
    if (stat(dir_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;
        }
    }
    return false;
#endif
}

NEKO_API_DECL s32 neko_pf_mkdir(const char* dir_path, s32 opt) {
#ifdef NEKO_PF_WIN
    return _mkdir(dir_path);
#else
    return mkdir(dir_path, opt);
#endif
}

NEKO_API_DECL bool neko_pf_file_exists(const char* file_path) {
    const char* path = file_path;

#ifdef NEKO_PF_ANDROID
    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    path = tmp_path;
#endif

    FILE* fp = neko_fopen(file_path, "r");
    if (fp) {
        neko_fclose(fp);
        return true;
    }
    return false;
}

s32 neko_pf_file_size_in_bytes(const char* file_path) {
#ifdef NEKO_PF_WIN

    HANDLE hFile = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;  // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;  // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return neko_util_safe_truncate_u64(size.QuadPart);

#elif (defined NEKO_PF_ANDROID)

    const char* internal_data_path = neko_app()->android.internal_data_path;
    neko_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
    struct stat st;
    stat(tmp_path, &st);
    return (s32)st.st_size;

#else

    struct stat st;
    stat(file_path, &st);
    return (s32)st.st_size;

#endif
}

void neko_pf_file_extension(char* buffer, size_t buffer_sz, const char* file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

NEKO_API_DECL s32 neko_pf_file_delete(const char* file_path) {
#if (defined NEKO_PF_WIN)

    // Non-zero if successful
    return DeleteFileA(file_path);

#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)

    // Returns 0 if successful
    return !remove(file_path);

#endif

    return 0;
}

NEKO_API_DECL s32 neko_pf_file_copy(const char* src_path, const char* dst_path) {
#if (defined NEKO_PF_WIN)

    return CopyFileA(src_path, dst_path, false);

#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)

    FILE* file_w = NULL;
    FILE* file_r = NULL;
    char buffer[2048] = NEKO_DEFAULT_VAL();

    if ((file_w = neko_fopen(src_path, "wb")) == NULL) {
        return 0;
    }
    if ((file_r = neko_fopen(dst_path, "rb")) == NULL) {
        return 0;
    }

    // Read file in 2kb chunks to write to location
    s32 len = 0;
    while ((len = fread(buffer, sizeof(buffer), 1, file_r)) > 0) {
        fwrite(buffer, len, 1, file_w);
    }

    // Close both files
    neko_fclose(file_r);
    neko_fclose(file_w);

#endif

    return 0;
}

NEKO_API_DECL s32 neko_pf_file_compare_time(u64 time_a, u64 time_b) { return time_a < time_b ? -1 : time_a == time_b ? 0 : 1; }

NEKO_API_DECL neko_pf_file_stats_t neko_pf_file_stats(const char* file_path) {
    neko_pf_file_stats_t stats = NEKO_DEFAULT_VAL();

#if (defined NEKO_PF_WIN)

    WIN32_FILE_ATTRIBUTE_DATA data = NEKO_DEFAULT_VAL();
    FILETIME ftime = NEKO_DEFAULT_VAL();
    FILETIME ctime = NEKO_DEFAULT_VAL();
    FILETIME atime = NEKO_DEFAULT_VAL();
    if (GetFileAttributesExA(file_path, GetFileExInfoStandard, &data)) {
        ftime = data.ftLastWriteTime;
        ctime = data.ftCreationTime;
        atime = data.ftLastAccessTime;
    }

    stats.modified_time = *((u64*)&ftime);
    stats.access_time = *((u64*)&atime);
    stats.creation_time = *((u64*)&ctime);

#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)
    struct stat attr = NEKO_DEFAULT_VAL();
    stat(file_path, &attr);
    stats.modified_time = *((u64*)&attr.st_mtime);

#endif

    return stats;
}

NEKO_API_DECL void* neko_pf_library_load(const char* lib_path) {
#if (defined NEKO_PF_WIN)
    return (void*)LoadLibraryA(lib_path);
#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)
    return (void*)dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);  // RTLD_LAZY
#endif
    return NULL;
}

NEKO_API_DECL void neko_pf_library_unload(void* lib) {
    if (!lib) return;
#if (defined NEKO_PF_WIN)
    FreeLibrary((HMODULE)lib);
#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)
    dlclose(lib);
#endif
}

NEKO_API_DECL void* neko_pf_library_proc_address(void* lib, const char* func) {
    if (!lib) return NULL;
#if (defined NEKO_PF_WIN)
    return (void*)GetProcAddress((HMODULE)lib, func);
#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)
    return (void*)dlsym(lib, func);
#endif
    return NULL;
}

NEKO_API_DECL int neko_pf_chdir(const char* path) {

#if (defined NEKO_PF_WIN)

    return chdir(path);

#elif (defined NEKO_PF_LINUX || defined NEKO_PF_APPLE || defined NEKO_PF_ANDROID)

    return chdir(path);

#endif

    return 1;
}

#undef NEKO_PF_IMPL_DEFAULT
#endif  // NEKO_PF_IMPL_DEFAULT

/*======================
// GLFW Implemenation
======================*/

#include "engine/neko_render.h"

// glfw
#include <GLFW/glfw3.h>

#ifdef NEKO_PF_IMPL_GLFW

#if (defined NEKO_PF_APPLE || defined NEKO_PF_LINUX)

#include <sched.h>
#include <unistd.h>

#elif (defined NEKO_PF_WIN)

#define WIN32_LEAN_AND_MEAN
#include <locale.h>
#include <windows.h>

#endif

#ifdef NEKO_PF_WIN

#pragma warning(push)
#pragma warning(disable : 4091)
#include <Psapi.h>  // windows GetProcessMemoryInfo
#include <Shobjidl_core.h>
#include <shlobj_core.h>
#include <sysinfoapi.h>
#include <windowsx.h>
#pragma warning(pop)
#pragma warning(disable : 4996)

typedef enum PROCESS_DPI_AWARENESS { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;

typedef HRESULT (*SetProcessDpiAwareness_func)(PROCESS_DPI_AWARENESS value);

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#endif  // NEKO_PF_WIN

// Forward Decls.
void __glfw_key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods);
void __glfw_char_callback(GLFWwindow* window, u32 codepoint);
void __glfw_mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods);
void __glfw_mouse_cursor_position_callback(GLFWwindow* window, f64 x, f64 y);
void __glfw_mouse_scroll_wheel_callback(GLFWwindow* window, f64 xoffset, f64 yoffset);
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered);
void __glfw_frame_buffer_size_callback(GLFWwindow* window, s32 width, s32 height);
void __glfw_drop_callback(GLFWwindow* window);

b32 __glfw_set_window_center(GLFWwindow* window);

/*
#define __glfw_window_from_handle(platform, handle)\
    ((GLFWwindow*)(neko_slot_array_get((platform)->windows, (handle))))
*/

/*== Platform Init / Shutdown == */

void neko_glfw_error_callback(int code, const_str description) { NEKO_WARN("glfw error %d : %s", code, description); }

void neko_pf_init(neko_pf_t* pf) {
    NEKO_ASSERT(pf);

    NEKO_TRACE("initializing glfw");

#ifdef NEKO_PF_WIN
    setlocale(LC_ALL, "en_us.utf8");
    SetConsoleOutputCP(CP_UTF8);
    SetProcessDPIAware();
    neko_pf_symbol_handler_init();
#elif defined(NEKO_PF_LINUX)
    // handle linux symbol
#endif

    glfwInit();

    CVAR_REF(settings_video_render_debug, s32);
    CVAR_REF(settings_video_render_hdpi, bool);

    if (settings_video_render_debug.get<s32>()) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    // 设定 OpenGL 版本
#if defined(NEKO_GL33)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

#if defined(NEKO_PF_APPLE)
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    if (settings_video_render_hdpi.get<bool>()) glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    // glfwSwapInterval(pf->settings.video.vsync_enabled);

    //    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    //    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // 设置窗口无边框

    glfwSetErrorCallback(neko_glfw_error_callback);

    // Construct cursors
    pf->cursors[(u32)NEKO_PF_CURSOR_ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_IBEAM] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_SIZE_NW_SE] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_SIZE_NE_SW] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_SIZE_NS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_SIZE_WE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_SIZE_ALL] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_HAND] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    pf->cursors[(u32)NEKO_PF_CURSOR_NO] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    // Poll joysticks here
    for (u32 i = 0; i < NEKO_PF_GAMEPAD_MAX; ++i) {
        pf->input.gamepads[i].present = glfwJoystickPresent(GLFW_JOYSTICK_1 + i);
        if (pf->input.gamepads[i].present) {
            NEKO_TRACE("controller %d connected.", i);
        }
    }
}

neko_memory_info_t glfw_platform_meminfo() {
    neko_memory_info_t meminfo = {0};

#if defined(NEKO_PF_WIN)
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        meminfo.virtual_memory_used = pmc.PrivateUsage;
        meminfo.physical_memory_used = pmc.WorkingSetSize;
        meminfo.peak_virtual_memory_used = pmc.PeakPagefileUsage;
        meminfo.peak_physical_memory_used = pmc.PeakWorkingSetSize;

    } else {
    }
#elif defined(NEKO_PF_APPLE)
    task_t task;
    task_for_pid(current_task(), getpid(), &task);

    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) != KERN_SUCCESS) {
        // printf("failed to get task info");
    }

    meminfo.virtual_memory_used = t_info.virtual_size;
    meminfo.physical_memory_used = t_info.resident_size;
    meminfo.peak_virtual_memory_used = t_info.virtual_size;    // macOS 不直接提供虚拟内存使用峰值
    meminfo.peak_physical_memory_used = t_info.resident_size;  // macOS 不直接提供峰值物理内存使用情况
#else
    // TODO:: linux 读取内存使用情况
#endif

    // TODO:: 支持AMD显卡获取显存信息

#if defined(NEKO_DISCRETE_GPU)

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

    GLint total_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);

    GLint cur_avail_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);

    meminfo.gpu_memory_used = total_mem_kb - cur_avail_mem_kb;
    meminfo.gpu_total_memory = cur_avail_mem_kb;

#endif

    return meminfo;
}

void* glfw_proc_handle() {
#if defined(NEKO_PF_WIN)
    struct neko_pf_t* platform = neko_subsystem(platform);
    GLFWwindow* win = (GLFWwindow*)(neko_slot_array_getp(platform->windows, neko_pf_main_window()))->hndl;
    HWND hwnd = glfwGetWin32Window(win);
    return hwnd;
#elif defined(NEKO_PF_LINUX)
    return NULL;
#else
    return NULL;
#endif
}

void* neko_pf_hwnd() { return glfw_proc_handle(); }

neko_memory_info_t neko_pf_memory_info() { return glfw_platform_meminfo(); }

void neko_pf_msgbox(const_str msg) {
#if defined(NEKO_PF_WIN)
    MessageBoxA((HWND)glfw_proc_handle(), msg, "Neko Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
#elif defined(NEKO_PF_LINUX)
    char info[128];
    neko_snprintf(info, 128, "notify-send \"%s\"", msg);
    system(info);
#elif defined(NEKO_PF_APPLE)

#else  // TODO:: wasm
    Android_MessageBox("Neko Error", x);
#endif
}

NEKO_API_DECL void neko_pf_update_internal(neko_pf_t* platform) {
    neko_pf_input_t* input = &platform->input;

    // Platform time
    platform->time.elapsed = (glfwGetTime() * 1000.0f);

    // Update all window/framebuffer state
    for (neko_slot_array_iter it = neko_slot_array_iter_new(platform->windows); neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
        // Cache all necessary window information
        s32 wx = 0, wy = 0, fx = 0, fy = 0, wpx = 0, wpy = 0;
        neko_pf_window_t* win = neko_slot_array_getp(platform->windows, it);
        glfwGetWindowSize((GLFWwindow*)win->hndl, &wx, &wy);
        glfwGetFramebufferSize((GLFWwindow*)win->hndl, &fx, &fy);
        glfwGetWindowPos((GLFWwindow*)win->hndl, &wpx, &wpy);
        win->window_size = neko_v2((f32)wx, (f32)wy);
        win->window_position = neko_v2((f32)wpx, (f32)wpy);
        win->framebuffer_size = neko_v2((f32)fx, (f32)fy);
    }

    // Update all gamepad state
    for (u32 i = 0; i < NEKO_PF_GAMEPAD_MAX; ++i) {

        neko_pf_gamepad_t* gp = &input->gamepads[i];
        gp->present = glfwJoystickPresent(GLFW_JOYSTICK_1 + i);

        if (gp->present) {
            s32 count = 0;
            const f32* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + i, &count);
            count = NEKO_MIN(count, NEKO_PF_JOYSTICK_AXIS_COUNT);

            for (u32 a = 0; a < count; ++a) {
                gp->axes[a] = axes[a];
            }

            const u8* buttons = (u8*)glfwGetJoystickButtons(GLFW_JOYSTICK_1 + i, &count);
            count = NEKO_MIN(count, NEKO_PF_GAMEPAD_BUTTON_COUNT);

            for (u32 b = 0; b < count; ++b) {
                gp->buttons[b] = buttons[b];
            }
        }
    }
}

void neko_pf_shutdown(neko_pf_t* pf) {
    // Free all windows in glfw
    // TODO: Figure out crash with glfwDestroyWindow && glfwTerminate
    for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(pf->windows, it); neko_slot_array_iter_advance(pf->windows, it)) {
        GLFWwindow* win = (GLFWwindow*)neko_slot_array_getp(pf->windows, it)->hndl;
        glfwDestroyWindow(win);
    }

    glfwTerminate();
}

u32 neko_pf_key_to_codepoint(neko_pf_keycode key) {
    u32 code = 0;
    switch (key) {
        default:
        case NEKO_KEYCODE_COUNT:
        case NEKO_KEYCODE_INVALID:
            code = 0;
            break;
        case NEKO_KEYCODE_SPACE:
            code = 32;
            break;
        case NEKO_KEYCODE_APOSTROPHE:
            code = 39;
            break;
        case NEKO_KEYCODE_COMMA:
            code = 44;
            break;
        case NEKO_KEYCODE_MINUS:
            code = 45;
            break;
        case NEKO_KEYCODE_PERIOD:
            code = 46;
            break;
        case NEKO_KEYCODE_SLASH:
            code = 47;
            break;
        case NEKO_KEYCODE_0:
            code = 48;
            break;
        case NEKO_KEYCODE_1:
            code = 49;
            break;
        case NEKO_KEYCODE_2:
            code = 50;
            break;
        case NEKO_KEYCODE_3:
            code = 51;
            break;
        case NEKO_KEYCODE_4:
            code = 52;
            break;
        case NEKO_KEYCODE_5:
            code = 53;
            break;
        case NEKO_KEYCODE_6:
            code = 54;
            break;
        case NEKO_KEYCODE_7:
            code = 55;
            break;
        case NEKO_KEYCODE_8:
            code = 56;
            break;
        case NEKO_KEYCODE_9:
            code = 57;
            break;
        case NEKO_KEYCODE_SEMICOLON:
            code = 59;
            break; /* ; */
        case NEKO_KEYCODE_EQUAL:
            code = 61;
            break; /* code = */
        case NEKO_KEYCODE_A:
            code = 65;
            break;
        case NEKO_KEYCODE_B:
            code = 66;
            break;
        case NEKO_KEYCODE_C:
            code = 67;
            break;
        case NEKO_KEYCODE_D:
            code = 68;
            break;
        case NEKO_KEYCODE_E:
            code = 69;
            break;
        case NEKO_KEYCODE_F:
            code = 70;
            break;
        case NEKO_KEYCODE_G:
            code = 71;
            break;
        case NEKO_KEYCODE_H:
            code = 72;
            break;
        case NEKO_KEYCODE_I:
            code = 73;
            break;
        case NEKO_KEYCODE_J:
            code = 74;
            break;
        case NEKO_KEYCODE_K:
            code = 75;
            break;
        case NEKO_KEYCODE_L:
            code = 76;
            break;
        case NEKO_KEYCODE_M:
            code = 77;
            break;
        case NEKO_KEYCODE_N:
            code = 78;
            break;
        case NEKO_KEYCODE_O:
            code = 79;
            break;
        case NEKO_KEYCODE_P:
            code = 80;
            break;
        case NEKO_KEYCODE_Q:
            code = 81;
            break;
        case NEKO_KEYCODE_R:
            code = 82;
            break;
        case NEKO_KEYCODE_S:
            code = 83;
            break;
        case NEKO_KEYCODE_T:
            code = 84;
            break;
        case NEKO_KEYCODE_U:
            code = 85;
            break;
        case NEKO_KEYCODE_V:
            code = 86;
            break;
        case NEKO_KEYCODE_W:
            code = 87;
            break;
        case NEKO_KEYCODE_X:
            code = 88;
            break;
        case NEKO_KEYCODE_Y:
            code = 89;
            break;
        case NEKO_KEYCODE_Z:
            code = 90;
            break;
        case NEKO_KEYCODE_LEFT_BRACKET:
            code = 91;
            break; /* [ */
        case NEKO_KEYCODE_BACKSLASH:
            code = 92;
            break; /* \ */
        case NEKO_KEYCODE_RIGHT_BRACKET:
            code = 93;
            break; /* ] */
        case NEKO_KEYCODE_GRAVE_ACCENT:
            code = 96;
            break; /* ` */
        case NEKO_KEYCODE_WORLD_1:
            code = 161;
            break; /* non-US #1 */
        case NEKO_KEYCODE_WORLD_2:
            code = 162;
            break; /* non-US #2 */
        case NEKO_KEYCODE_ESC:
            code = 256;
            break;
        case NEKO_KEYCODE_ENTER:
            code = 257;
            break;
        case NEKO_KEYCODE_TAB:
            code = 258;
            break;
        case NEKO_KEYCODE_BACKSPACE:
            code = 259;
            break;
        case NEKO_KEYCODE_INSERT:
            code = 260;
            break;
        case NEKO_KEYCODE_DELETE:
            code = GLFW_KEY_DELETE;
            break;
        case NEKO_KEYCODE_RIGHT:
            code = 262;
            break;
        case NEKO_KEYCODE_LEFT:
            code = 263;
            break;
        case NEKO_KEYCODE_DOWN:
            code = 264;
            break;
        case NEKO_KEYCODE_UP:
            code = 265;
            break;
        case NEKO_KEYCODE_PAGE_UP:
            code = 266;
            break;
        case NEKO_KEYCODE_PAGE_DOWN:
            code = 267;
            break;
        case NEKO_KEYCODE_HOME:
            code = 268;
            break;
        case NEKO_KEYCODE_END:
            code = 269;
            break;
        case NEKO_KEYCODE_CAPS_LOCK:
            code = 280;
            break;
        case NEKO_KEYCODE_SCROLL_LOCK:
            code = 281;
            break;
        case NEKO_KEYCODE_NUM_LOCK:
            code = 282;
            break;
        case NEKO_KEYCODE_PRINT_SCREEN:
            code = 283;
            break;
        case NEKO_KEYCODE_PAUSE:
            code = 284;
            break;
        case NEKO_KEYCODE_F1:
            code = 290;
            break;
        case NEKO_KEYCODE_F2:
            code = 291;
            break;
        case NEKO_KEYCODE_F3:
            code = 292;
            break;
        case NEKO_KEYCODE_F4:
            code = 293;
            break;
        case NEKO_KEYCODE_F5:
            code = 294;
            break;
        case NEKO_KEYCODE_F6:
            code = 295;
            break;
        case NEKO_KEYCODE_F7:
            code = 296;
            break;
        case NEKO_KEYCODE_F8:
            code = 297;
            break;
        case NEKO_KEYCODE_F9:
            code = 298;
            break;
        case NEKO_KEYCODE_F10:
            code = 299;
            break;
        case NEKO_KEYCODE_F11:
            code = 300;
            break;
        case NEKO_KEYCODE_F12:
            code = 301;
            break;
        case NEKO_KEYCODE_F13:
            code = 302;
            break;
        case NEKO_KEYCODE_F14:
            code = 303;
            break;
        case NEKO_KEYCODE_F15:
            code = 304;
            break;
        case NEKO_KEYCODE_F16:
            code = 305;
            break;
        case NEKO_KEYCODE_F17:
            code = 306;
            break;
        case NEKO_KEYCODE_F18:
            code = 307;
            break;
        case NEKO_KEYCODE_F19:
            code = 308;
            break;
        case NEKO_KEYCODE_F20:
            code = 309;
            break;
        case NEKO_KEYCODE_F21:
            code = 310;
            break;
        case NEKO_KEYCODE_F22:
            code = 311;
            break;
        case NEKO_KEYCODE_F23:
            code = 312;
            break;
        case NEKO_KEYCODE_F24:
            code = 313;
            break;
        case NEKO_KEYCODE_F25:
            code = 314;
            break;
        case NEKO_KEYCODE_KP_0:
            code = 320;
            break;
        case NEKO_KEYCODE_KP_1:
            code = 321;
            break;
        case NEKO_KEYCODE_KP_2:
            code = 322;
            break;
        case NEKO_KEYCODE_KP_3:
            code = 323;
            break;
        case NEKO_KEYCODE_KP_4:
            code = 324;
            break;
        case NEKO_KEYCODE_KP_5:
            code = 325;
            break;
        case NEKO_KEYCODE_KP_6:
            code = 326;
            break;
        case NEKO_KEYCODE_KP_7:
            code = 327;
            break;
        case NEKO_KEYCODE_KP_8:
            code = 328;
            break;
        case NEKO_KEYCODE_KP_9:
            code = 329;
            break;
        case NEKO_KEYCODE_KP_DECIMAL:
            code = 330;
            break;
        case NEKO_KEYCODE_KP_DIVIDE:
            code = 331;
            break;
        case NEKO_KEYCODE_KP_MULTIPLY:
            code = 332;
            break;
        case NEKO_KEYCODE_KP_SUBTRACT:
            code = 333;
            break;
        case NEKO_KEYCODE_KP_ADD:
            code = 334;
            break;
        case NEKO_KEYCODE_KP_ENTER:
            code = 335;
            break;
        case NEKO_KEYCODE_KP_EQUAL:
            code = 336;
            break;
        case NEKO_KEYCODE_LEFT_SHIFT:
            code = 340;
            break;
        case NEKO_KEYCODE_LEFT_CONTROL:
            code = 341;
            break;
        case NEKO_KEYCODE_LEFT_ALT:
            code = 342;
            break;
        case NEKO_KEYCODE_LEFT_SUPER:
            code = 343;
            break;
        case NEKO_KEYCODE_RIGHT_SHIFT:
            code = 344;
            break;
        case NEKO_KEYCODE_RIGHT_CONTROL:
            code = 345;
            break;
        case NEKO_KEYCODE_RIGHT_ALT:
            code = 346;
            break;
        case NEKO_KEYCODE_RIGHT_SUPER:
            code = 347;
            break;
        case NEKO_KEYCODE_MENU:
            code = 348;
            break;
    }
    return code;
}

// This doesn't work. Have to set up keycodes for emscripten instead. FUN.
neko_pf_keycode neko_pf_codepoint_to_key(u32 code) {
    neko_pf_keycode key = NEKO_KEYCODE_INVALID;
    switch (code) {
        default:
        case 0:
            key = NEKO_KEYCODE_INVALID;
            break;
        case 32:
            key = NEKO_KEYCODE_SPACE;
            break;
        case 39:
            key = NEKO_KEYCODE_APOSTROPHE;
            break;
        case 44:
            key = NEKO_KEYCODE_COMMA;
            break;
        case 45:
            key = NEKO_KEYCODE_MINUS;
            break;
        case 46:
            key = NEKO_KEYCODE_PERIOD;
            break;
        case 47:
            key = NEKO_KEYCODE_SLASH;
            break;
        case 48:
            key = NEKO_KEYCODE_0;
            break;
        case 49:
            key = NEKO_KEYCODE_1;
            break;
        case 50:
            key = NEKO_KEYCODE_2;
            break;
        case 51:
            key = NEKO_KEYCODE_3;
            break;
        case 52:
            key = NEKO_KEYCODE_4;
            break;
        case 53:
            key = NEKO_KEYCODE_5;
            break;
        case 54:
            key = NEKO_KEYCODE_6;
            break;
        case 55:
            key = NEKO_KEYCODE_7;
            break;
        case 56:
            key = NEKO_KEYCODE_8;
            break;
        case 57:
            key = NEKO_KEYCODE_9;
            break;
        case 59:
            key = NEKO_KEYCODE_SEMICOLON;
            break;
        case 61:
            key = NEKO_KEYCODE_EQUAL;
            break;
        case 65:
            key = NEKO_KEYCODE_A;
            break;
        case 66:
            key = NEKO_KEYCODE_B;
            break;
        case 67:
            key = NEKO_KEYCODE_C;
            break;
        case 68:
            key = NEKO_KEYCODE_D;
            break;
        case 69:
            key = NEKO_KEYCODE_E;
            break;
        case 70:
            key = NEKO_KEYCODE_F;
            break;
        case 71:
            key = NEKO_KEYCODE_G;
            break;
        case 72:
            key = NEKO_KEYCODE_H;
            break;
        case 73:
            key = NEKO_KEYCODE_I;
            break;
        case 74:
            key = NEKO_KEYCODE_J;
            break;
        case 75:
            key = NEKO_KEYCODE_K;
            break;
        case 76:
            key = NEKO_KEYCODE_L;
            break;
        case 77:
            key = NEKO_KEYCODE_M;
            break;
        case 78:
            key = NEKO_KEYCODE_N;
            break;
        case 79:
            key = NEKO_KEYCODE_O;
            break;
        case 80:
            key = NEKO_KEYCODE_P;
            break;
        case 81:
            key = NEKO_KEYCODE_Q;
            break;
        case 82:
            key = NEKO_KEYCODE_R;
            break;
        case 83:
            key = NEKO_KEYCODE_S;
            break;
        case 84:
            key = NEKO_KEYCODE_T;
            break;
        case 85:
            key = NEKO_KEYCODE_U;
            break;
        case 86:
            key = NEKO_KEYCODE_V;
            break;
        case 87:
            key = NEKO_KEYCODE_W;
            break;
        case 88:
            key = NEKO_KEYCODE_X;
            break;
        case 89:
            key = NEKO_KEYCODE_Y;
            break;
        case 90:
            key = NEKO_KEYCODE_Z;
            break;
        case 91:
            key = NEKO_KEYCODE_LEFT_BRACKET;
            break;
        case 92:
            key = NEKO_KEYCODE_BACKSLASH;
            break;
        case 93:
            key = NEKO_KEYCODE_RIGHT_BRACKET;
            break;
        case 96:
            key = NEKO_KEYCODE_GRAVE_ACCENT;
            break;
        case 161:
            key = NEKO_KEYCODE_WORLD_1;
            break;
        case 162:
            key = NEKO_KEYCODE_WORLD_2;
            break;
        case 256:
            key = NEKO_KEYCODE_ESC;
            break;
        case 257:
            key = NEKO_KEYCODE_ENTER;
            break;
        case 258:
            key = NEKO_KEYCODE_TAB;
            break;
        case 259:
            key = NEKO_KEYCODE_BACKSPACE;
            break;
        case 260:
            key = NEKO_KEYCODE_INSERT;
            break;
        case GLFW_KEY_DELETE:
            key = NEKO_KEYCODE_DELETE;
            break;
        case 262:
            key = NEKO_KEYCODE_RIGHT;
            break;
        case 263:
            key = NEKO_KEYCODE_LEFT;
            break;
        case 264:
            key = NEKO_KEYCODE_DOWN;
            break;
        case 265:
            key = NEKO_KEYCODE_UP;
            break;
        case 266:
            key = NEKO_KEYCODE_PAGE_UP;
            break;
        case 267:
            key = NEKO_KEYCODE_PAGE_DOWN;
            break;
        case 268:
            key = NEKO_KEYCODE_HOME;
            break;
        case 269:
            key = NEKO_KEYCODE_END;
            break;
        case 280:
            key = NEKO_KEYCODE_CAPS_LOCK;
            break;
        case 281:
            key = NEKO_KEYCODE_SCROLL_LOCK;
            break;
        case 282:
            key = NEKO_KEYCODE_NUM_LOCK;
            break;
        case 283:
            key = NEKO_KEYCODE_PRINT_SCREEN;
            break;
        case 284:
            key = NEKO_KEYCODE_PAUSE;
            break;
        case 290:
            key = NEKO_KEYCODE_F1;
            break;
        case 291:
            key = NEKO_KEYCODE_F2;
            break;
        case 292:
            key = NEKO_KEYCODE_F3;
            break;
        case 293:
            key = NEKO_KEYCODE_F4;
            break;
        case 294:
            key = NEKO_KEYCODE_F5;
            break;
        case 295:
            key = NEKO_KEYCODE_F6;
            break;
        case 296:
            key = NEKO_KEYCODE_F7;
            break;
        case 297:
            key = NEKO_KEYCODE_F8;
            break;
        case 298:
            key = NEKO_KEYCODE_F9;
            break;
        case 299:
            key = NEKO_KEYCODE_F10;
            break;
        case 300:
            key = NEKO_KEYCODE_F11;
            break;
        case 301:
            key = NEKO_KEYCODE_F12;
            break;
        case 302:
            key = NEKO_KEYCODE_F13;
            break;
        case 303:
            key = NEKO_KEYCODE_F14;
            break;
        case 304:
            key = NEKO_KEYCODE_F15;
            break;
        case 305:
            key = NEKO_KEYCODE_F16;
            break;
        case 306:
            key = NEKO_KEYCODE_F17;
            break;
        case 307:
            key = NEKO_KEYCODE_F18;
            break;
        case 308:
            key = NEKO_KEYCODE_F19;
            break;
        case 309:
            key = NEKO_KEYCODE_F20;
            break;
        case 310:
            key = NEKO_KEYCODE_F21;
            break;
        case 311:
            key = NEKO_KEYCODE_F22;
            break;
        case 312:
            key = NEKO_KEYCODE_F23;
            break;
        case 313:
            key = NEKO_KEYCODE_F24;
            break;
        case 314:
            key = NEKO_KEYCODE_F25;
            break;
        case 320:
            key = NEKO_KEYCODE_KP_0;
            break;
        case 321:
            key = NEKO_KEYCODE_KP_1;
            break;
        case 322:
            key = NEKO_KEYCODE_KP_2;
            break;
        case 323:
            key = NEKO_KEYCODE_KP_3;
            break;
        case 324:
            key = NEKO_KEYCODE_KP_4;
            break;
        case 325:
            key = NEKO_KEYCODE_KP_5;
            break;
        case 326:
            key = NEKO_KEYCODE_KP_6;
            break;
        case 327:
            key = NEKO_KEYCODE_KP_7;
            break;
        case 328:
            key = NEKO_KEYCODE_KP_8;
            break;
        case 329:
            key = NEKO_KEYCODE_KP_9;
            break;
        case 330:
            key = NEKO_KEYCODE_KP_DECIMAL;
            break;
        case 331:
            key = NEKO_KEYCODE_KP_DIVIDE;
            break;
        case 332:
            key = NEKO_KEYCODE_KP_MULTIPLY;
            break;
        case 333:
            key = NEKO_KEYCODE_KP_SUBTRACT;
            break;
        case 334:
            key = NEKO_KEYCODE_KP_ADD;
            break;
        case 335:
            key = NEKO_KEYCODE_KP_ENTER;
            break;
        case 336:
            key = NEKO_KEYCODE_KP_EQUAL;
            break;
        case 340:
            key = NEKO_KEYCODE_LEFT_SHIFT;
            break;
        case 341:
            key = NEKO_KEYCODE_LEFT_CONTROL;
            break;
        case 342:
            key = NEKO_KEYCODE_LEFT_ALT;
            break;
        case 343:
            key = NEKO_KEYCODE_LEFT_SUPER;
            break;
        case 344:
            key = NEKO_KEYCODE_RIGHT_SHIFT;
            break;
        case 345:
            key = NEKO_KEYCODE_RIGHT_CONTROL;
            break;
        case 346:
            key = NEKO_KEYCODE_RIGHT_ALT;
            break;
        case 347:
            key = NEKO_KEYCODE_RIGHT_SUPER;
            break;
        case 348:
            key = NEKO_KEYCODE_MENU;
            break;
    }
    return key;
}

/*=== GLFW Callbacks ===*/

neko_pf_keycode glfw_key_to_neko_keycode(u32 code) {
    switch (code) {
        case GLFW_KEY_A:
            return NEKO_KEYCODE_A;
            break;
        case GLFW_KEY_B:
            return NEKO_KEYCODE_B;
            break;
        case GLFW_KEY_C:
            return NEKO_KEYCODE_C;
            break;
        case GLFW_KEY_D:
            return NEKO_KEYCODE_D;
            break;
        case GLFW_KEY_E:
            return NEKO_KEYCODE_E;
            break;
        case GLFW_KEY_F:
            return NEKO_KEYCODE_F;
            break;
        case GLFW_KEY_G:
            return NEKO_KEYCODE_G;
            break;
        case GLFW_KEY_H:
            return NEKO_KEYCODE_H;
            break;
        case GLFW_KEY_I:
            return NEKO_KEYCODE_I;
            break;
        case GLFW_KEY_J:
            return NEKO_KEYCODE_J;
            break;
        case GLFW_KEY_K:
            return NEKO_KEYCODE_K;
            break;
        case GLFW_KEY_L:
            return NEKO_KEYCODE_L;
            break;
        case GLFW_KEY_M:
            return NEKO_KEYCODE_M;
            break;
        case GLFW_KEY_N:
            return NEKO_KEYCODE_N;
            break;
        case GLFW_KEY_O:
            return NEKO_KEYCODE_O;
            break;
        case GLFW_KEY_P:
            return NEKO_KEYCODE_P;
            break;
        case GLFW_KEY_Q:
            return NEKO_KEYCODE_Q;
            break;
        case GLFW_KEY_R:
            return NEKO_KEYCODE_R;
            break;
        case GLFW_KEY_S:
            return NEKO_KEYCODE_S;
            break;
        case GLFW_KEY_T:
            return NEKO_KEYCODE_T;
            break;
        case GLFW_KEY_U:
            return NEKO_KEYCODE_U;
            break;
        case GLFW_KEY_V:
            return NEKO_KEYCODE_V;
            break;
        case GLFW_KEY_W:
            return NEKO_KEYCODE_W;
            break;
        case GLFW_KEY_X:
            return NEKO_KEYCODE_X;
            break;
        case GLFW_KEY_Y:
            return NEKO_KEYCODE_Y;
            break;
        case GLFW_KEY_Z:
            return NEKO_KEYCODE_Z;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            return NEKO_KEYCODE_LEFT_SHIFT;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
            return NEKO_KEYCODE_RIGHT_SHIFT;
            break;
        case GLFW_KEY_LEFT_ALT:
            return NEKO_KEYCODE_LEFT_ALT;
            break;
        case GLFW_KEY_RIGHT_ALT:
            return NEKO_KEYCODE_RIGHT_ALT;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            return NEKO_KEYCODE_LEFT_CONTROL;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
            return NEKO_KEYCODE_RIGHT_CONTROL;
            break;
        case GLFW_KEY_BACKSPACE:
            return NEKO_KEYCODE_BACKSPACE;
            break;
        case GLFW_KEY_BACKSLASH:
            return NEKO_KEYCODE_BACKSLASH;
            break;
        case GLFW_KEY_SLASH:
            return NEKO_KEYCODE_SLASH;
            break;
        case GLFW_KEY_GRAVE_ACCENT:
            return NEKO_KEYCODE_GRAVE_ACCENT;
            break;
        case GLFW_KEY_COMMA:
            return NEKO_KEYCODE_COMMA;
            break;
        case GLFW_KEY_PERIOD:
            return NEKO_KEYCODE_PERIOD;
            break;
        case GLFW_KEY_ESCAPE:
            return NEKO_KEYCODE_ESC;
            break;
        case GLFW_KEY_SPACE:
            return NEKO_KEYCODE_SPACE;
            break;
        case GLFW_KEY_LEFT:
            return NEKO_KEYCODE_LEFT;
            break;
        case GLFW_KEY_UP:
            return NEKO_KEYCODE_UP;
            break;
        case GLFW_KEY_RIGHT:
            return NEKO_KEYCODE_RIGHT;
            break;
        case GLFW_KEY_DOWN:
            return NEKO_KEYCODE_DOWN;
            break;
        case GLFW_KEY_0:
            return NEKO_KEYCODE_0;
            break;
        case GLFW_KEY_1:
            return NEKO_KEYCODE_1;
            break;
        case GLFW_KEY_2:
            return NEKO_KEYCODE_2;
            break;
        case GLFW_KEY_3:
            return NEKO_KEYCODE_3;
            break;
        case GLFW_KEY_4:
            return NEKO_KEYCODE_4;
            break;
        case GLFW_KEY_5:
            return NEKO_KEYCODE_5;
            break;
        case GLFW_KEY_6:
            return NEKO_KEYCODE_6;
            break;
        case GLFW_KEY_7:
            return NEKO_KEYCODE_7;
            break;
        case GLFW_KEY_8:
            return NEKO_KEYCODE_8;
            break;
        case GLFW_KEY_9:
            return NEKO_KEYCODE_9;
            break;
        case GLFW_KEY_KP_0:
            return NEKO_KEYCODE_KP_0;
            break;
        case GLFW_KEY_KP_1:
            return NEKO_KEYCODE_KP_1;
            break;
        case GLFW_KEY_KP_2:
            return NEKO_KEYCODE_KP_2;
            break;
        case GLFW_KEY_KP_3:
            return NEKO_KEYCODE_KP_3;
            break;
        case GLFW_KEY_KP_4:
            return NEKO_KEYCODE_KP_4;
            break;
        case GLFW_KEY_KP_5:
            return NEKO_KEYCODE_KP_5;
            break;
        case GLFW_KEY_KP_6:
            return NEKO_KEYCODE_KP_6;
            break;
        case GLFW_KEY_KP_7:
            return NEKO_KEYCODE_KP_7;
            break;
        case GLFW_KEY_KP_8:
            return NEKO_KEYCODE_KP_8;
            break;
        case GLFW_KEY_KP_9:
            return NEKO_KEYCODE_KP_9;
            break;
        case GLFW_KEY_CAPS_LOCK:
            return NEKO_KEYCODE_CAPS_LOCK;
            break;
        case GLFW_KEY_DELETE:
            return NEKO_KEYCODE_DELETE;
            break;
        case GLFW_KEY_END:
            return NEKO_KEYCODE_END;
            break;
        case GLFW_KEY_F1:
            return NEKO_KEYCODE_F1;
            break;
        case GLFW_KEY_F2:
            return NEKO_KEYCODE_F2;
            break;
        case GLFW_KEY_F3:
            return NEKO_KEYCODE_F3;
            break;
        case GLFW_KEY_F4:
            return NEKO_KEYCODE_F4;
            break;
        case GLFW_KEY_F5:
            return NEKO_KEYCODE_F5;
            break;
        case GLFW_KEY_F6:
            return NEKO_KEYCODE_F6;
            break;
        case GLFW_KEY_F7:
            return NEKO_KEYCODE_F7;
            break;
        case GLFW_KEY_F8:
            return NEKO_KEYCODE_F8;
            break;
        case GLFW_KEY_F9:
            return NEKO_KEYCODE_F9;
            break;
        case GLFW_KEY_F10:
            return NEKO_KEYCODE_F10;
            break;
        case GLFW_KEY_F11:
            return NEKO_KEYCODE_F11;
            break;
        case GLFW_KEY_F12:
            return NEKO_KEYCODE_F12;
            break;
        case GLFW_KEY_HOME:
            return NEKO_KEYCODE_HOME;
            break;
        case GLFW_KEY_EQUAL:
            return NEKO_KEYCODE_EQUAL;
            break;
        case GLFW_KEY_MINUS:
            return NEKO_KEYCODE_MINUS;
            break;
        case GLFW_KEY_LEFT_BRACKET:
            return NEKO_KEYCODE_LEFT_BRACKET;
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            return NEKO_KEYCODE_RIGHT_BRACKET;
            break;
        case GLFW_KEY_SEMICOLON:
            return NEKO_KEYCODE_SEMICOLON;
            break;
        case GLFW_KEY_ENTER:
            return NEKO_KEYCODE_ENTER;
            break;
        case GLFW_KEY_INSERT:
            return NEKO_KEYCODE_INSERT;
            break;
        case GLFW_KEY_PAGE_UP:
            return NEKO_KEYCODE_PAGE_UP;
            break;
        case GLFW_KEY_PAGE_DOWN:
            return NEKO_KEYCODE_PAGE_DOWN;
            break;
        case GLFW_KEY_NUM_LOCK:
            return NEKO_KEYCODE_NUM_LOCK;
            break;
        case GLFW_KEY_TAB:
            return NEKO_KEYCODE_TAB;
            break;
        case GLFW_KEY_KP_MULTIPLY:
            return NEKO_KEYCODE_KP_MULTIPLY;
            break;
        case GLFW_KEY_KP_DIVIDE:
            return NEKO_KEYCODE_KP_DIVIDE;
            break;
        case GLFW_KEY_KP_ADD:
            return NEKO_KEYCODE_KP_ADD;
            break;
        case GLFW_KEY_KP_SUBTRACT:
            return NEKO_KEYCODE_KP_SUBTRACT;
            break;
        case GLFW_KEY_KP_ENTER:
            return NEKO_KEYCODE_KP_ENTER;
            break;
        case GLFW_KEY_KP_DECIMAL:
            return NEKO_KEYCODE_KP_DECIMAL;
            break;
        case GLFW_KEY_PAUSE:
            return NEKO_KEYCODE_PAUSE;
            break;
        case GLFW_KEY_PRINT_SCREEN:
            return NEKO_KEYCODE_PRINT_SCREEN;
            break;
        default:
            return NEKO_KEYCODE_COUNT;
            break;
    }

    // Shouldn't reach here
    return NEKO_KEYCODE_COUNT;
}

neko_pf_mouse_button_code __glfw_button_to_neko_mouse_button(s32 code) {
    switch (code) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return NEKO_MOUSE_LBUTTON;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return NEKO_MOUSE_RBUTTON;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return NEKO_MOUSE_MBUTTON;
            break;
    }

    // Shouldn't reach here
    return NEKO_MOUSE_BUTTON_CODE_COUNT;
}

void __glfw_char_callback(GLFWwindow* window, u32 codepoint) {
    // Grab platform instance from engine
    neko_pf_t* platform = neko_subsystem(platform);

    neko_pf_event_t evt = NEKO_DEFAULT_VAL();
    evt.type = NEKO_PF_EVENT_TEXT;
    evt.text.codepoint = codepoint;

    // Add action
    neko_pf_add_event(&evt);
}

void __glfw_key_callback(GLFWwindow* window, s32 code, s32 scancode, s32 action, s32 mods) {
    // Grab platform instance from engine
    neko_pf_t* platform = neko_subsystem(platform);

    // Get keycode from key
    neko_pf_keycode key = glfw_key_to_neko_keycode(code);

    // Push back event into platform events
    neko_pf_event_t evt = NEKO_DEFAULT_VAL();
    evt.type = NEKO_PF_EVENT_KEY;
    evt.key.codepoint = code;
    evt.key.scancode = scancode;
    evt.key.keycode = key;
    evt.key.modifier = (neko_pf_key_modifier_type)mods;

    switch (action) {
        // Released
        case 0: {
            neko_pf_release_key(key);
            evt.key.action = NEKO_PF_KEY_RELEASED;
        } break;

        // Pressed
        case 1: {
            neko_pf_press_key(key);
            evt.key.action = NEKO_PF_KEY_PRESSED;
        } break;

        // Down
        case 2: {
            neko_pf_press_key(key);
            evt.key.action = NEKO_PF_KEY_DOWN;
        } break;

        default: {
        } break;
    }

    // Add action
    neko_pf_add_event(&evt);
}

void __glfw_mouse_button_callback(GLFWwindow* window, s32 code, s32 action, s32 mods) {
    // Grab platform instance from engine
    neko_pf_t* platform = neko_subsystem(platform);

    // Get mouse code from key
    neko_pf_mouse_button_code button = __glfw_button_to_neko_mouse_button(code);

    // Push back event into platform events
    neko_pf_event_t evt = NEKO_DEFAULT_VAL();
    evt.type = NEKO_PF_EVENT_MOUSE;
    evt.mouse.codepoint = code;
    evt.mouse.button = button;

    switch (action) {
        // Released
        case 0: {
            neko_pf_release_mouse_button(button);
            evt.mouse.action = NEKO_PF_MOUSE_BUTTON_RELEASED;
        } break;

        // Pressed
        case 1: {
            neko_pf_press_mouse_button(button);
            evt.mouse.action = NEKO_PF_MOUSE_BUTTON_PRESSED;
        } break;

        // Down
        case 2: {
            neko_pf_press_mouse_button(button);
            evt.mouse.action = NEKO_PF_MOUSE_BUTTON_DOWN;
        } break;
    }

    // Add action
    neko_pf_add_event(&evt);
}

void __glfw_mouse_cursor_position_callback(GLFWwindow* window, f64 x, f64 y) {
    neko_pf_t* platform = neko_subsystem(platform);
    // platform->input.mouse.position = neko_v2((f32)x, (f32)y);
    // platform->input.mouse.moved_this_frame = true;

    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_MOUSE;
    neko_evt.mouse.action = NEKO_PF_MOUSE_MOVE;

    // neko_println("pos: <%.2f, %.2f>, old: <%.2f, %.2f>", x, y, platform->input.mouse.position.x, platform->input.mouse.position.y);

    // neko_evt.mouse.move = neko_v2((f32)x, (f32)y);

    // Calculate mouse move based on whether locked or not
    if (neko_pf_mouse_locked()) {
        neko_evt.mouse.move.x = x - platform->input.mouse.position.x;
        neko_evt.mouse.move.y = y - platform->input.mouse.position.y;
        platform->input.mouse.position.x = x;
        platform->input.mouse.position.y = y;
    } else {
        neko_evt.mouse.move = neko_v2((f32)x, (f32)y);
    }

    // Push back event into platform events
    neko_pf_add_event(&neko_evt);
}

// GLFW窗口焦点回调函数
void __glfw_window_focus_callback(GLFWwindow* window, int focused) {

    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = (neko_slot_array_getp(platform->windows, neko_pf_main_window()));

    if (focused == GLFW_TRUE) {
        win->focus = true;
    } else {
        win->focus = false;
    }
}

void __glfw_mouse_scroll_wheel_callback(GLFWwindow* window, f64 x, f64 y) {
    neko_pf_t* platform = neko_subsystem(platform);
    platform->input.mouse.wheel = neko_v2((f32)x, (f32)y);

    // Push back event into platform events
    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_MOUSE;
    neko_evt.mouse.action = NEKO_PF_MOUSE_WHEEL;
    neko_evt.mouse.wheel = neko_v2((f32)x, (f32)y);
    neko_pf_add_event(&neko_evt);
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_MOUSE;
    neko_evt.mouse.action = entered ? NEKO_PF_MOUSE_ENTER : NEKO_PF_MOUSE_LEAVE;
    neko_pf_add_event(&neko_evt);
}

void __glfw_frame_buffer_size_callback(GLFWwindow* window, s32 width, s32 height) {
    // Nothing for now
}

/*== Platform Input == */

void neko_pf_process_input(neko_pf_input_t* input) { glfwPollEvents(); }

/*== Platform Util == */

void neko_pf_sleep(f32 ms) {
    if (ms < 0.f) {
        return;
    }
#if (defined NEKO_PF_WIN)
    timeBeginPeriod(1);
    Sleep((u64)ms);
    timeEndPeriod(1);
#elif (defined NEKO_PF_APPLE)
    usleep(ms * 1000.f);  // unistd.h
#else
    int usleep(__useconds_t useconds);
    usleep(ms * 1000.f);  // unistd.h
#endif
}

NEKO_API_DECL double neko_pf_elapsed_time() {
    neko_pf_t* platform = neko_subsystem(platform);
    return platform->time.elapsed;
}

/*== Platform Video == */

NEKO_API_DECL void neko_pf_enable_vsync(s32 enabled) { glfwSwapInterval(enabled ? 1 : 0); }

/*== OpenGL debug callback == */
void GLAPIENTRY __neko_pf_gl_debug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* msg, const void* user) {
    // if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    //     NEKO_WARN("GL: %s", msg);
    // }
}

/*== Platform Window == */

NEKO_API_DECL neko_pf_window_t neko_pf_window_create_internal(const neko_pf_running_desc_t* desc) {
    neko_pf_window_t win = NEKO_DEFAULT_VAL();

    if (!desc) {
        // Log warning
        NEKO_WARN("window descriptor is null");
        return win;
    }

    // Grab window hints from application desc
    u32 window_hints = desc->flags;

    // Set whether or not the screen is resizable
    glfwWindowHint(GLFW_RESIZABLE, (window_hints & NEKO_WINDOW_FLAGS_NO_RESIZE) != NEKO_WINDOW_FLAGS_NO_RESIZE);

    // Set multi-samples
    if (desc->num_samples) {
        glfwWindowHint(GLFW_SAMPLES, desc->num_samples);
    } else {
        glfwWindowHint(GLFW_SAMPLES, 0);
    }

    // Get monitor if fullscreen
    GLFWmonitor* monitor = NULL;
    if ((window_hints & NEKO_WINDOW_FLAGS_FULLSCREEN) == NEKO_WINDOW_FLAGS_FULLSCREEN) {
        int monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
        if (desc->monitor_index < monitor_count) {
            monitor = monitors[desc->monitor_index];
        }
    }

    GLFWwindow* window = glfwCreateWindow(desc->width, desc->height, desc->title, monitor, NULL);
    if (window == NULL) {
        NEKO_ERROR("%s", "failed to create window");
        glfwTerminate();
        return win;
    }

    win.hndl = window;

    // Callbacks for window
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, &__glfw_key_callback);
    glfwSetCharCallback(window, &__glfw_char_callback);
    glfwSetMouseButtonCallback(window, &__glfw_mouse_button_callback);
    glfwSetCursorEnterCallback(window, &__glfw_mouse_cursor_enter_callback);
    glfwSetCursorPosCallback(window, &__glfw_mouse_cursor_position_callback);
    glfwSetScrollCallback(window, &__glfw_mouse_scroll_wheel_callback);
    glfwSetWindowFocusCallback(window, &__glfw_window_focus_callback);

    // Cache all necessary window information
    s32 wx = 0, wy = 0, fx = 0, fy = 0, wpx = 0, wpy = 0;
    glfwGetWindowSize((GLFWwindow*)win.hndl, &wx, &wy);
    glfwGetFramebufferSize((GLFWwindow*)win.hndl, &fx, &fy);
    glfwGetWindowPos((GLFWwindow*)win.hndl, &wpx, &wpy);
    win.window_size = neko_v2((f32)wx, (f32)wy);
    win.window_position = neko_v2((f32)wpx, (f32)wpy);
    win.framebuffer_size = neko_v2((f32)fx, (f32)fy);
    win.focus = true;

    CVAR_REF(settings_video_render_debug, s32);

    // 只执行一次
    if (neko_slot_array_empty(neko_subsystem(platform)->windows)) {
        neko_gl_load_all();
        NEKO_INFO("opengl %s (glsl %s)", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        if (settings_video_render_debug.get<s32>()) {
            // glDebugMessageCallback(__neko_pf_gl_debug, NULL);
        }
    }

    if (desc->center) __glfw_set_window_center(window);

    return win;
}

// Platform callbacks
NEKO_API_DECL void neko_pf_set_dropped_files_callback(u32 handle, neko_dropped_files_callback_t cb) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSetDropCallback((GLFWwindow*)win->hndl, (GLFWdropfun)cb);
}

NEKO_API_DECL void neko_pf_set_window_close_callback(u32 handle, neko_window_close_callback_t cb) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSetWindowCloseCallback((GLFWwindow*)win->hndl, (GLFWwindowclosefun)cb);
}

NEKO_API_DECL void neko_pf_set_character_callback(u32 handle, neko_character_callback_t cb) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSetCharCallback((GLFWwindow*)win->hndl, (GLFWcharfun)cb);
}

NEKO_API_DECL void neko_pf_set_framebuffer_resize_callback(u32 handle, neko_framebuffer_resize_callback_t cb) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSetFramebufferSizeCallback((GLFWwindow*)win->hndl, (GLFWframebuffersizefun)cb);
}

NEKO_API_DECL void neko_pf_mouse_set_position(u32 handle, f32 x, f32 y) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSetCursorPos((GLFWwindow*)win->hndl, x, y);
}

NEKO_API_DECL void* neko_pf_raw_window_handle(u32 handle) {
    // Grab instance of platform from engine
    neko_pf_t* platform = neko_subsystem(platform);

    // Grab window from handle
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    return (void*)win->hndl;
}

NEKO_API_DECL void neko_pf_window_swap_buffer(u32 handle) {
    // Grab instance of platform from engine
    neko_pf_t* platform = neko_subsystem(platform);

    // Grab window from handle
    neko_pf_window_t* win = neko_slot_array_getp(platform->windows, handle);
    glfwSwapBuffers((GLFWwindow*)win->hndl);
}

NEKO_API_DECL neko_vec2 neko_pf_window_sizev(u32 handle) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return window->window_size;
}

NEKO_API_DECL void neko_pf_window_size(u32 handle, u32* w, u32* h) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    *w = (s32)window->window_size.x;
    *h = (s32)window->window_size.y;
}

u32 neko_pf_window_width(u32 handle) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return (u32)window->window_size.x;
}

u32 neko_pf_window_height(u32 handle) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return (u32)window->window_size.y;
}

b32 neko_pf_window_fullscreen(u32 handle) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return glfwGetWindowMonitor((GLFWwindow*)window->hndl) != NULL;
}

void neko_pf_window_position(u32 handle, u32* x, u32* y) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    *x = (u32)window->window_position.x;
    *y = (u32)window->window_position.y;
}

neko_vec2 neko_pf_window_positionv(u32 handle) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return window->window_position;
}

void neko_pf_set_window_title(u32 handle, const_str title) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetWindowTitle((GLFWwindow*)win->hndl, title);
}

void neko_pf_set_window_size(u32 handle, u32 w, u32 h) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetWindowSize((GLFWwindow*)window->hndl, (s32)w, (s32)h);
}

void neko_pf_set_window_sizev(u32 handle, neko_vec2 v) {
    neko_pf_window_t* window = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetWindowSize((GLFWwindow*)window->hndl, (u32)v.x, (u32)v.y);
}

void neko_pf_set_window_fullscreen(u32 handle, b32 fullscreen) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    GLFWmonitor* monitor = NULL;

    s32 x, y, w, h;
    glfwGetWindowPos((GLFWwindow*)win->hndl, &x, &y);
    glfwGetWindowSize((GLFWwindow*)win->hndl, &w, &h);

    if (fullscreen) {
        CVAR_REF(settings_window_monitor_index, s32);
        u32 monitor_index = settings_window_monitor_index.get<s32>();
        int monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
        if (monitor_index < monitor_count) {
            monitor = monitors[monitor_index];
        }
    }

    glfwSetWindowMonitor((GLFWwindow*)win->hndl, monitor, x, y, w, h, GLFW_DONT_CARE);
}

void neko_pf_set_window_position(u32 handle, u32 x, u32 y) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetWindowPos((GLFWwindow*)win->hndl, (s32)x, (s32)y);
}

void neko_pf_set_window_positionv(u32 handle, neko_vec2 v) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetWindowPos((GLFWwindow*)win->hndl, (s32)v.x, (s32)v.y);
}

void neko_pf_framebuffer_size(u32 handle, u32* w, u32* h) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    *w = (u32)win->framebuffer_size.x;
    *h = (u32)win->framebuffer_size.y;
}

neko_vec2 neko_pf_framebuffer_sizev(u32 handle) {
    u32 w = 0, h = 0;
    neko_pf_framebuffer_size(handle, &w, &h);
    return neko_v2((f32)w, (f32)h);
}

u32 neko_pf_framebuffer_width(u32 handle) {
    u32 w = 0, h = 0;
    neko_pf_framebuffer_size(handle, &w, &h);
    return w;
}

u32 neko_pf_framebuffer_height(u32 handle) {
    u32 w = 0, h = 0;
    neko_pf_framebuffer_size(handle, &w, &h);
    return h;
}

NEKO_API_DECL neko_vec2 neko_pf_monitor_sizev(u32 id) {
    neko_vec2 ms = neko_v2s(0.f);
    s32 width, height, xpos, ypos;
    s32 count;
    GLFWmonitor* monitor = NULL;
    neko_pf_t* platform = neko_subsystem(platform);

    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if (count && id < count) {
        monitor = monitors[id];
    } else {
        monitor = glfwGetPrimaryMonitor();
    }
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
    ms.x = (f32)width;
    ms.y = (f32)height;
    return ms;
}

neko_vec2 neko_pf_get_window_dpi() {
    neko_vec2 v = NEKO_DEFAULT_VAL();
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &v.x, &v.y);
    return v;
}

void neko_pf_window_set_clipboard(u32 handle, const_str str) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetClipboardString((GLFWwindow*)win->hndl, str);
}

const_str neko_pf_window_get_clipboard(u32 handle) {
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    return glfwGetClipboardString((GLFWwindow*)win->hndl);
}

void neko_pf_set_cursor(u32 handle, neko_pf_cursor cursor) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    GLFWcursor* cp = ((GLFWcursor*)platform->cursors[(u32)cursor]);
    glfwSetCursor((GLFWwindow*)win->hndl, cp);
}

void neko_pf_lock_mouse(u32 handle, b32 lock) {
    __neko_input()->mouse.locked = lock;
    neko_pf_t* platform = neko_subsystem(platform);
    neko_pf_window_t* win = neko_slot_array_getp(neko_subsystem(platform)->windows, handle);
    glfwSetInputMode((GLFWwindow*)win->hndl, GLFW_CURSOR, lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    // Not sure if I want to support this or not
    // if (glfwRawMouseMotionSupported()) {
    //     glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, log_lock ? GLFW_TRUE : GLFW_FALSE);
    // }
}

// 设置 glfw 在屏幕中央
b32 __glfw_set_window_center(GLFWwindow* window) {
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
    GLFWmonitor** m = glfwGetMonitors(&monitor_count);
    if (!m) return false;

    for (int j = 0; j < monitor_count; ++j) {

        glfwGetMonitorPos(m[j], &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
        if (!mode) continue;

        // Get intersection of two rectangles - screen and window
        int minX = NEKO_MAX(mx, px);
        int minY = NEKO_MAX(my, py);

        int maxX = NEKO_MIN(mx + mode->width, px + sx);
        int maxY = NEKO_MIN(my + mode->height, py + sy);

        // Calculate area of the intersection
        int area = NEKO_MAX(maxX - minX, 0) * NEKO_MAX(maxY - minY, 0);

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
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (primary) {
            const GLFWvidmode* desktop = glfwGetVideoMode(primary);

            if (desktop)
                glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
            else
                return false;
        } else
            return false;
    }

    return true;
}

#ifdef NEKO_PF_WIN

//
#include <windows.h>
//
#include <dbghelp.h>

// 定义用于堆栈跟踪的最大深度
#define MAX_STACK_DEPTH 64

// 初始化符号处理器
void neko_pf_symbol_handler_init() {
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
}

// 打印函数调用栈信息 包括函数名和源码文件名
const_str neko_pf_stacktrace() {

    static char trace_info[4096];

    void* stack[MAX_STACK_DEPTH];
    USHORT frames = CaptureStackBackTrace(0, MAX_STACK_DEPTH, stack, NULL);

    for (USHORT i = 0; i < frames; i++) {
        DWORD64 address = (DWORD64)(stack[i]);

        // 获取函数名
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)neko_calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        SymFromAddr(GetCurrentProcess(), address, 0, symbol);

        // 获取源码文件名和行号
        IMAGEHLP_LINE64 lineInfo;
        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement;

        bool is_main = !strcmp(symbol->Name, "main");

        char trace_tmp[512];
        if (SymGetLineFromAddr64(GetCurrentProcess(), address, &displacement, &lineInfo)) {
            neko_snprintf(trace_tmp, 512, "#%u %s - File: %s, Line: %u\n", i, symbol->Name, lineInfo.FileName, lineInfo.LineNumber);
        } else {
            neko_snprintf(trace_tmp, 512, "#%u %s - Source information not available\n", i, symbol->Name);
        }

        strcat(trace_info, trace_tmp);
        neko_printf("%s", trace_tmp);

        neko_free(symbol);

        if (is_main) break;
    }

    return trace_info;
}

#else

void neko_pf_symbol_handler_init() {}
const_str neko_pf_stacktrace() { return ""; }

#endif

#undef NEKO_PF_IMPL_GLFW
#endif  // NEKO_PF_IMPL_GLFW

/*==========================
// Emscripten Implemenation
==========================*/

#ifdef NEKO_PF_IMPL_EMSCRIPTEN

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Emscripten context data
typedef struct neko_ems_t {
    const char* canvas_name;
    double canvas_width;
    double canvas_height;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    b32 mouse_down[NEKO_MOUSE_BUTTON_CODE_COUNT];
} neko_ems_t;

#define NEKO_EMS_DATA() ((neko_ems_t*)(neko_subsystem(platform)->user_data))

u32 neko_pf_key_to_codepoint(neko_pf_keycode key) {
    u32 code = 0;
    switch (key) {
        default:
        case NEKO_KEYCODE_COUNT:
        case NEKO_KEYCODE_INVALID:
            code = 0;
            break;
        case NEKO_KEYCODE_SPACE:
            code = 32;
            break;
        case NEKO_KEYCODE_APOSTROPHE:
            code = 222;
            break;
        case NEKO_KEYCODE_COMMA:
            code = 44;
            break;
        case NEKO_KEYCODE_MINUS:
            code = 45;
            break;
        case NEKO_KEYCODE_PERIOD:
            code = 46;
            break;
        case NEKO_KEYCODE_SLASH:
            code = 47;
            break;
        case NEKO_KEYCODE_0:
            code = 48;
            break;
        case NEKO_KEYCODE_1:
            code = 49;
            break;
        case NEKO_KEYCODE_2:
            code = 50;
            break;
        case NEKO_KEYCODE_3:
            code = 51;
            break;
        case NEKO_KEYCODE_4:
            code = 52;
            break;
        case NEKO_KEYCODE_5:
            code = 53;
            break;
        case NEKO_KEYCODE_6:
            code = 54;
            break;
        case NEKO_KEYCODE_7:
            code = 55;
            break;
        case NEKO_KEYCODE_8:
            code = 56;
            break;
        case NEKO_KEYCODE_9:
            code = 57;
            break;
        case NEKO_KEYCODE_SEMICOLON:
            code = 59;
            break; /* ; */
        case NEKO_KEYCODE_EQUAL:
            code = 61;
            break; /* code = */
        case NEKO_KEYCODE_A:
            code = 65 + 32;
            break;
        case NEKO_KEYCODE_B:
            code = 66 + 32;
            break;
        case NEKO_KEYCODE_C:
            code = 67 + 32;
            break;
        case NEKO_KEYCODE_D:
            code = 68 + 32;
            break;
        case NEKO_KEYCODE_E:
            code = 69 + 32;
            break;
        case NEKO_KEYCODE_F:
            code = 70 + 32;
            break;
        case NEKO_KEYCODE_G:
            code = 71 + 32;
            break;
        case NEKO_KEYCODE_H:
            code = 72 + 32;
            break;
        case NEKO_KEYCODE_I:
            code = 73 + 32;
            break;
        case NEKO_KEYCODE_J:
            code = 74 + 32;
            break;
        case NEKO_KEYCODE_K:
            code = 75 + 32;
            break;
        case NEKO_KEYCODE_L:
            code = 76 + 32;
            break;
        case NEKO_KEYCODE_M:
            code = 77 + 32;
            break;
        case NEKO_KEYCODE_N:
            code = 78 + 32;
            break;
        case NEKO_KEYCODE_O:
            code = 79 + 32;
            break;
        case NEKO_KEYCODE_P:
            code = 80 + 32;
            break;
        case NEKO_KEYCODE_Q:
            code = 81 + 32;
            break;
        case NEKO_KEYCODE_R:
            code = 82 + 32;
            break;
        case NEKO_KEYCODE_S:
            code = 83 + 32;
            break;
        case NEKO_KEYCODE_T:
            code = 84 + 32;
            break;
        case NEKO_KEYCODE_U:
            code = 85 + 32;
            break;
        case NEKO_KEYCODE_V:
            code = 86 + 32;
            break;
        case NEKO_KEYCODE_W:
            code = 87 + 32;
            break;
        case NEKO_KEYCODE_X:
            code = 88 + 32;
            break;
        case NEKO_KEYCODE_Y:
            code = 89 + 32;
            break;
        case NEKO_KEYCODE_Z:
            code = 90 + 32;
            break;
        case NEKO_KEYCODE_LEFT_BRACKET:
            code = 91;
            break; /* [ */
        case NEKO_KEYCODE_BACKSLASH:
            code = 92;
            break; /* \ */
        case NEKO_KEYCODE_RIGHT_BRACKET:
            code = 93;
            break; /* ] */
        case NEKO_KEYCODE_GRAVE_ACCENT:
            code = 96;
            break; /* ` */
        case NEKO_KEYCODE_WORLD_1:
            code = 161;
            break; /* non-US #1 */
        case NEKO_KEYCODE_WORLD_2:
            code = 162;
            break; /* non-US #2 */
        case NEKO_KEYCODE_ESC:
            code = 27;
            break;
        case NEKO_KEYCODE_ENTER:
            code = 13;
            break;
        case NEKO_KEYCODE_TAB:
            code = 9;
            break;
        case NEKO_KEYCODE_BACKSPACE:
            code = 8;
            break;
        case NEKO_KEYCODE_INSERT:
            code = 260;
            break;
        case NEKO_KEYCODE_DELETE:
            code = 261;
            break;
        case NEKO_KEYCODE_LEFT:
            code = 37;
            break;
        case NEKO_KEYCODE_UP:
            code = 38;
            break;
        case NEKO_KEYCODE_RIGHT:
            code = 39;
            break;
        case NEKO_KEYCODE_DOWN:
            code = 40;
            break;
        case NEKO_KEYCODE_PAGE_UP:
            code = 266;
            break;
        case NEKO_KEYCODE_PAGE_DOWN:
            code = 267;
            break;
        case NEKO_KEYCODE_HOME:
            code = 268;
            break;
        case NEKO_KEYCODE_END:
            code = 269;
            break;
        case NEKO_KEYCODE_CAPS_LOCK:
            code = 280;
            break;
        case NEKO_KEYCODE_SCROLL_LOCK:
            code = 281;
            break;
        case NEKO_KEYCODE_NUM_LOCK:
            code = 282;
            break;
        case NEKO_KEYCODE_PRINT_SCREEN:
            code = 283;
            break;
        case NEKO_KEYCODE_PAUSE:
            code = 284;
            break;
        case NEKO_KEYCODE_F1:
            code = 290;
            break;
        case NEKO_KEYCODE_F2:
            code = 291;
            break;
        case NEKO_KEYCODE_F3:
            code = 292;
            break;
        case NEKO_KEYCODE_F4:
            code = 293;
            break;
        case NEKO_KEYCODE_F5:
            code = 294;
            break;
        case NEKO_KEYCODE_F6:
            code = 295;
            break;
        case NEKO_KEYCODE_F7:
            code = 296;
            break;
        case NEKO_KEYCODE_F8:
            code = 297;
            break;
        case NEKO_KEYCODE_F9:
            code = 298;
            break;
        case NEKO_KEYCODE_F10:
            code = 299;
            break;
        case NEKO_KEYCODE_F11:
            code = 300;
            break;
        case NEKO_KEYCODE_F12:
            code = 301;
            break;
        case NEKO_KEYCODE_F13:
            code = 302;
            break;
        case NEKO_KEYCODE_F14:
            code = 303;
            break;
        case NEKO_KEYCODE_F15:
            code = 304;
            break;
        case NEKO_KEYCODE_F16:
            code = 305;
            break;
        case NEKO_KEYCODE_F17:
            code = 306;
            break;
        case NEKO_KEYCODE_F18:
            code = 307;
            break;
        case NEKO_KEYCODE_F19:
            code = 308;
            break;
        case NEKO_KEYCODE_F20:
            code = 309;
            break;
        case NEKO_KEYCODE_F21:
            code = 310;
            break;
        case NEKO_KEYCODE_F22:
            code = 311;
            break;
        case NEKO_KEYCODE_F23:
            code = 312;
            break;
        case NEKO_KEYCODE_F24:
            code = 313;
            break;
        case NEKO_KEYCODE_F25:
            code = 314;
            break;
        case NEKO_KEYCODE_KP_0:
            code = 320;
            break;
        case NEKO_KEYCODE_KP_1:
            code = 321;
            break;
        case NEKO_KEYCODE_KP_2:
            code = 322;
            break;
        case NEKO_KEYCODE_KP_3:
            code = 323;
            break;
        case NEKO_KEYCODE_KP_4:
            code = 324;
            break;
        case NEKO_KEYCODE_KP_5:
            code = 325;
            break;
        case NEKO_KEYCODE_KP_6:
            code = 326;
            break;
        case NEKO_KEYCODE_KP_7:
            code = 327;
            break;
        case NEKO_KEYCODE_KP_8:
            code = 328;
            break;
        case NEKO_KEYCODE_KP_9:
            code = 329;
            break;
        case NEKO_KEYCODE_KP_DECIMAL:
            code = 330;
            break;
        case NEKO_KEYCODE_KP_DIVIDE:
            code = 331;
            break;
        case NEKO_KEYCODE_KP_MULTIPLY:
            code = 332;
            break;
        case NEKO_KEYCODE_KP_SUBTRACT:
            code = 333;
            break;
        case NEKO_KEYCODE_KP_ADD:
            code = 334;
            break;
        case NEKO_KEYCODE_KP_ENTER:
            code = 335;
            break;
        case NEKO_KEYCODE_KP_EQUAL:
            code = 336;
            break;
        case NEKO_KEYCODE_LEFT_SHIFT:
            code = 16;
            break;
        case NEKO_KEYCODE_LEFT_CONTROL:
            code = 17;
            break;
        case NEKO_KEYCODE_LEFT_ALT:
            code = 18;
            break;
        case NEKO_KEYCODE_LEFT_SUPER:
            code = 343;
            break;
        case NEKO_KEYCODE_RIGHT_SHIFT:
            code = 16;
            break;
        case NEKO_KEYCODE_RIGHT_CONTROL:
            code = 17;
            break;
        case NEKO_KEYCODE_RIGHT_ALT:
            code = 18;
            break;
        case NEKO_KEYCODE_RIGHT_SUPER:
            code = 347;
            break;
        case NEKO_KEYCODE_MENU:
            code = 348;
            break;
    }
    return code;
}

/*
    key_to_code_map[count] = NEKO_DEFAULT_VAL();
    code_to_key_map[count] = NEKO_DEFAULT_VAL();
*/

// This doesn't work. Have to set up keycodes for emscripten instead. FUN.
neko_pf_keycode neko_pf_codepoint_to_key(u32 code) {
    neko_pf_keycode key = NEKO_KEYCODE_INVALID;
    switch (code) {
        default:
        case 0:
            key = NEKO_KEYCODE_INVALID;
            break;
        case 32:
            key = NEKO_KEYCODE_SPACE;
            break;
        case 222:
            key = NEKO_KEYCODE_APOSTROPHE;
            break;
        case 44:
            key = NEKO_KEYCODE_COMMA;
            break;
        case 45:
            key = NEKO_KEYCODE_MINUS;
            break;
        case 46:
            key = NEKO_KEYCODE_PERIOD;
            break;
        case 47:
            key = NEKO_KEYCODE_SLASH;
            break;
        case 48:
            key = NEKO_KEYCODE_0;
            break;
        case 49:
            key = NEKO_KEYCODE_1;
            break;
        case 50:
            key = NEKO_KEYCODE_2;
            break;
        case 51:
            key = NEKO_KEYCODE_3;
            break;
        case 52:
            key = NEKO_KEYCODE_4;
            break;
        case 53:
            key = NEKO_KEYCODE_5;
            break;
        case 54:
            key = NEKO_KEYCODE_6;
            break;
        case 55:
            key = NEKO_KEYCODE_7;
            break;
        case 56:
            key = NEKO_KEYCODE_8;
            break;
        case 57:
            key = NEKO_KEYCODE_9;
            break;
        case 59:
            key = NEKO_KEYCODE_SEMICOLON;
            break;
        case 61:
            key = NEKO_KEYCODE_EQUAL;
            break;
        case 65:
        case 65 + 32:
            key = NEKO_KEYCODE_A;
            break;
        case 66:
        case 66 + 32:
            key = NEKO_KEYCODE_B;
            break;
        case 67:
        case 67 + 32:
            key = NEKO_KEYCODE_C;
            break;
        case 68:
        case 68 + 32:
            key = NEKO_KEYCODE_D;
            break;
        case 69:
        case 69 + 32:
            key = NEKO_KEYCODE_E;
            break;
        case 70:
        case 70 + 32:
            key = NEKO_KEYCODE_F;
            break;
        case 71:
        case 71 + 32:
            key = NEKO_KEYCODE_G;
            break;
        case 72:
        case 72 + 32:
            key = NEKO_KEYCODE_H;
            break;
        case 73:
        case 73 + 32:
            key = NEKO_KEYCODE_I;
            break;
        case 74:
        case 74 + 32:
            key = NEKO_KEYCODE_J;
            break;
        case 75:
        case 75 + 32:
            key = NEKO_KEYCODE_K;
            break;
        case 76:
        case 76 + 32:
            key = NEKO_KEYCODE_L;
            break;
        case 77:
        case 77 + 32:
            key = NEKO_KEYCODE_M;
            break;
        case 78:
        case 78 + 32:
            key = NEKO_KEYCODE_N;
            break;
        case 79:
        case 79 + 32:
            key = NEKO_KEYCODE_O;
            break;
        case 80:
        case 80 + 32:
            key = NEKO_KEYCODE_P;
            break;
        case 81:
        case 81 + 32:
            key = NEKO_KEYCODE_Q;
            break;
        case 82:
        case 82 + 32:
            key = NEKO_KEYCODE_R;
            break;
        case 83:
        case 83 + 32:
            key = NEKO_KEYCODE_S;
            break;
        case 84:
        case 84 + 32:
            key = NEKO_KEYCODE_T;
            break;
        case 85:
        case 85 + 32:
            key = NEKO_KEYCODE_U;
            break;
        case 86:
        case 86 + 32:
            key = NEKO_KEYCODE_V;
            break;
        case 87:
        case 87 + 32:
            key = NEKO_KEYCODE_W;
            break;
        case 88:
        case 88 + 32:
            key = NEKO_KEYCODE_X;
            break;
        case 89:
        case 89 + 32:
            key = NEKO_KEYCODE_Y;
            break;
        case 90:
        case 90 + 32:
            key = NEKO_KEYCODE_Z;
            break;
        case 91:
            key = NEKO_KEYCODE_LEFT_BRACKET;
            break;
        case 92:
            key = NEKO_KEYCODE_BACKSLASH;
            break;
        case 93:
            key = NEKO_KEYCODE_RIGHT_BRACKET;
            break;
        case 96:
            key = NEKO_KEYCODE_GRAVE_ACCENT;
            break;
        case 161:
            key = NEKO_KEYCODE_WORLD_1;
            break;
        case 162:
            key = NEKO_KEYCODE_WORLD_2;
            break;
        case 27:
            key = NEKO_KEYCODE_ESC;
            break;
        case 13:
            key = NEKO_KEYCODE_ENTER;
            break;
        case 9:
            key = NEKO_KEYCODE_TAB;
            break;
        case 8:
            key = NEKO_KEYCODE_BACKSPACE;
            break;
        case 260:
            key = NEKO_KEYCODE_INSERT;
            break;
        case 261:
            key = NEKO_KEYCODE_DELETE;
            break;
        case 37:
            key = NEKO_KEYCODE_LEFT;
            break;
        case 38:
            key = NEKO_KEYCODE_UP;
            break;
        case 39:
            key = NEKO_KEYCODE_RIGHT;
            break;
        case 40:
            key = NEKO_KEYCODE_DOWN;
            break;
        case 266:
            key = NEKO_KEYCODE_PAGE_UP;
            break;
        case 267:
            key = NEKO_KEYCODE_PAGE_DOWN;
            break;
        case 268:
            key = NEKO_KEYCODE_HOME;
            break;
        case 269:
            key = NEKO_KEYCODE_END;
            break;
        case 280:
            key = NEKO_KEYCODE_CAPS_LOCK;
            break;
        case 281:
            key = NEKO_KEYCODE_SCROLL_LOCK;
            break;
        case 282:
            key = NEKO_KEYCODE_NUM_LOCK;
            break;
        case 283:
            key = NEKO_KEYCODE_PRINT_SCREEN;
            break;
        case 284:
            key = NEKO_KEYCODE_PAUSE;
            break;
        case 290:
            key = NEKO_KEYCODE_F1;
            break;
        case 291:
            key = NEKO_KEYCODE_F2;
            break;
        case 292:
            key = NEKO_KEYCODE_F3;
            break;
        case 293:
            key = NEKO_KEYCODE_F4;
            break;
        case 294:
            key = NEKO_KEYCODE_F5;
            break;
        case 295:
            key = NEKO_KEYCODE_F6;
            break;
        case 296:
            key = NEKO_KEYCODE_F7;
            break;
        case 297:
            key = NEKO_KEYCODE_F8;
            break;
        case 298:
            key = NEKO_KEYCODE_F9;
            break;
        case 299:
            key = NEKO_KEYCODE_F10;
            break;
        case 300:
            key = NEKO_KEYCODE_F11;
            break;
        case 301:
            key = NEKO_KEYCODE_F12;
            break;
        case 302:
            key = NEKO_KEYCODE_F13;
            break;
        case 303:
            key = NEKO_KEYCODE_F14;
            break;
        case 304:
            key = NEKO_KEYCODE_F15;
            break;
        case 305:
            key = NEKO_KEYCODE_F16;
            break;
        case 306:
            key = NEKO_KEYCODE_F17;
            break;
        case 307:
            key = NEKO_KEYCODE_F18;
            break;
        case 308:
            key = NEKO_KEYCODE_F19;
            break;
        case 309:
            key = NEKO_KEYCODE_F20;
            break;
        case 310:
            key = NEKO_KEYCODE_F21;
            break;
        case 311:
            key = NEKO_KEYCODE_F22;
            break;
        case 312:
            key = NEKO_KEYCODE_F23;
            break;
        case 313:
            key = NEKO_KEYCODE_F24;
            break;
        case 314:
            key = NEKO_KEYCODE_F25;
            break;
        case 320:
            key = NEKO_KEYCODE_KP_0;
            break;
        case 321:
            key = NEKO_KEYCODE_KP_1;
            break;
        case 322:
            key = NEKO_KEYCODE_KP_2;
            break;
        case 323:
            key = NEKO_KEYCODE_KP_3;
            break;
        case 324:
            key = NEKO_KEYCODE_KP_4;
            break;
        case 325:
            key = NEKO_KEYCODE_KP_5;
            break;
        case 326:
            key = NEKO_KEYCODE_KP_6;
            break;
        case 327:
            key = NEKO_KEYCODE_KP_7;
            break;
        case 328:
            key = NEKO_KEYCODE_KP_8;
            break;
        case 329:
            key = NEKO_KEYCODE_KP_9;
            break;
        case 330:
            key = NEKO_KEYCODE_KP_DECIMAL;
            break;
        case 331:
            key = NEKO_KEYCODE_KP_DIVIDE;
            break;
        case 332:
            key = NEKO_KEYCODE_KP_MULTIPLY;
            break;
        case 333:
            key = NEKO_KEYCODE_KP_SUBTRACT;
            break;
        case 334:
            key = NEKO_KEYCODE_KP_ADD;
            break;
        case 335:
            key = NEKO_KEYCODE_KP_ENTER;
            break;
        case 336:
            key = NEKO_KEYCODE_KP_EQUAL;
            break;
        case 16:
            key = NEKO_KEYCODE_LEFT_SHIFT;
            break;
        case 17:
            key = NEKO_KEYCODE_LEFT_CONTROL;
            break;
        case 18:
            key = NEKO_KEYCODE_LEFT_ALT;
            break;
        case 343:
            key = NEKO_KEYCODE_LEFT_SUPER;
            break;
        case 347:
            key = NEKO_KEYCODE_RIGHT_SUPER;
            break;
        case 348:
            key = NEKO_KEYCODE_MENU;
            break;
    }
    return key;
}

EM_BOOL neko_ems_size_changed_cb(s32 type, const EmscriptenUiEvent* evt, void* user_data) {
    neko_println("size changed");
    neko_pf_t* platform = neko_subsystem(platform);
    neko_ems_t* ems = (neko_ems_t*)platform->user_data;
    (void)type;
    (void)evt;
    (void)user_data;
    // neko_println("was: <%.2f, %.2f>", (f32)ems->canvas_width, (f32)ems->canvas_height);
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    // neko_println("is: <%.2f, %.2f>", (f32)ems->canvas_width, (f32)ems->canvas_height);
    return true;
}

EM_BOOL neko_ems_fullscreenchange_cb(s32 type, const EmscriptenFullscreenChangeEvent* evt, void* user_data) {
    (void)user_data;
    (void)evt;
    (void)type;
    neko_ems_t* ems = NEKO_EMS_DATA();
    // emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    if (evt->isFullscreen) {
        EmscriptenFullscreenStrategy strategy;
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
        strategy.canvasResizedCallback = (int (*)(int, const void*, void*))neko_ems_size_changed_cb;
        emscripten_enter_soft_fullscreen(ems->canvas_name, &strategy);
        // neko_println("fullscreen!");
        // emscripten_enter_soft_fullscreen(ems->canvas_name, NULL);
        // ems->canvas_width = (f32)evt->screenWidth;
        // ems->canvas_height = (f32)evt->screenHeight;
        // emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    } else {
        emscripten_exit_fullscreen();
        emscripten_set_canvas_element_size(ems->canvas_name, 800, 600);
    }
}

EM_BOOL neko_ems_key_cb(s32 type, const EmscriptenKeyboardEvent* evt, void* user_data) {
    (void)user_data;

    // Push back event into platform events
    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_KEY;
    neko_evt.key.codepoint = evt->which;
    neko_evt.key.keycode = neko_pf_codepoint_to_key(evt->which);

    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS: {
            neko_evt.type = NEKO_PF_EVENT_TEXT;
            neko_evt.text.codepoint = evt->which;
        } break;

        case EMSCRIPTEN_EVENT_KEYDOWN: {
            neko_evt.key.action = NEKO_PF_KEY_DOWN;
        } break;

        case EMSCRIPTEN_EVENT_KEYUP: {
            neko_evt.key.action = NEKO_PF_KEY_RELEASED;
        } break;

        default:
            break;
    }

    // Add action
    neko_pf_add_event(&neko_evt);

    return evt->which < 32;
}

EM_BOOL neko_ems_mouse_cb(s32 type, const EmscriptenMouseEvent* evt, void* user_data) {
    (void)user_data;

    neko_pf_t* platform = neko_subsystem(platform);
    neko_ems_t* ems = NEKO_EMS_DATA();

    neko_pf_mouse_button_code button = NEKO_MOUSE_LBUTTON;
    switch (evt->button) {
        case 0:
            button = NEKO_MOUSE_LBUTTON;
            break;
        case 1:
            button = NEKO_MOUSE_MBUTTON;
            break;
        case 2:
            button = NEKO_MOUSE_RBUTTON;
            break;
    }

    // Push back event into platform events
    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_MOUSE;
    neko_evt.mouse.codepoint = evt->button;
    neko_evt.mouse.button = button;
    bool add = true;

    switch (type) {
        case EMSCRIPTEN_EVENT_CLICK: {
            // neko_evt.mouse.action = NEKO_PF_MOUSE_BUTTON_PRESSED;
            // neko_println("EMS_PRESSED");
            add = false;
        } break;

        // Emscripten doesn't register continuous presses, so have to manually store this state
        case EMSCRIPTEN_EVENT_MOUSEDOWN: {
            neko_evt.mouse.action = NEKO_PF_MOUSE_BUTTON_DOWN;
            // ems->mouse_down[(s32)button] = true;
        } break;

        case EMSCRIPTEN_EVENT_MOUSEUP: {
            neko_evt.mouse.action = NEKO_PF_MOUSE_BUTTON_RELEASED;
            // ems->mouse_down[(s32)button] = false;
        } break;

        case EMSCRIPTEN_EVENT_MOUSEMOVE: {
            neko_evt.mouse.action = NEKO_PF_MOUSE_MOVE;
            if (platform->input.mouse.locked) {
                neko_evt.mouse.move = neko_v2((f32)evt->movementX, (f32)evt->movementY);
            } else {
                neko_evt.mouse.move = neko_v2((f32)evt->targetX, (f32)evt->targetY);
            }
        } break;

        case EMSCRIPTEN_EVENT_MOUSEENTER: {
            neko_evt.mouse.action = NEKO_PF_MOUSE_ENTER;
            // Release all buttons
            /*
            ems->mouse_down[0] = false;
            ems->mouse_down[1] = false;
            ems->mouse_down[2] = false;
            */
        } break;

        case EMSCRIPTEN_EVENT_MOUSELEAVE: {
            neko_evt.mouse.action = NEKO_PF_MOUSE_LEAVE;
            // Release all buttons
            /*
            ems->mouse_down[0] = false;
            ems->mouse_down[1] = false;
            ems->mouse_down[2] = false;
            */
        } break;

        default: {
        } break;
    }

    if (add) neko_pf_add_event(&neko_evt);

    return true;
}

EM_BOOL neko_ems_mousewheel_cb(s32 type, const EmscriptenWheelEvent* evt, void* user_data) {
    (void)type;
    (void)user_data;

    // Push back event into platform events
    neko_pf_event_t neko_evt = NEKO_DEFAULT_VAL();
    neko_evt.type = NEKO_PF_EVENT_MOUSE;
    neko_evt.mouse.action = NEKO_PF_MOUSE_WHEEL;
    neko_evt.mouse.wheel = neko_v2((f32)evt->deltaX, -(f32)evt->deltaY);
    neko_pf_add_event(&neko_evt);

    return true;
}

EM_BOOL neko_ems_pointerlock_cb(s32 type, const EmscriptenPointerlockChangeEvent* evt, void* user_data) {
    (void)type;
    (void)user_data;
    neko_pf_t* platform = neko_subsystem(platform);
    platform->input.mouse.locked = evt->isActive;
    // neko_println("lock: %zu", platform->input.mouse.locked);
}

NEKO_API_DECL void neko_pf_init(neko_pf_t* platform) {
    neko_println("Initializing Emscripten.");

    neko_app_desc_t* app = neko_app();
    platform->user_data = neko_malloc_init(neko_ems_t);
    neko_ems_t* ems = (neko_ems_t*)platform->user_data;

    // ems->canvas_width = app->window_width;
    // ems->canvas_height = app->window_height;
    // double dpi = emscripten_get_device_pixel_ratio();

    // Just set this to defaults for now
    ems->canvas_name = "#canvas";
    emscripten_set_canvas_element_size(ems->canvas_name, app->window.width, app->window.height);
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);

    // Set up callbacks
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, neko_ems_fullscreenchange_cb);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, neko_ems_size_changed_cb);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, neko_ems_key_cb);
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, neko_ems_key_cb);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, neko_ems_key_cb);
    emscripten_set_click_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_mouseenter_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_mouseleave_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_mousedown_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_mouseup_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_mousemove_callback(ems->canvas_name, NULL, true, neko_ems_mouse_cb);
    emscripten_set_wheel_callback(ems->canvas_name, NULL, true, neko_ems_mousewheel_cb);
    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, true, neko_ems_pointerlock_cb);

    // Set up webgl context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.antialias = false;
    attrs.depth = true;
    attrs.premultipliedAlpha = false;
    attrs.stencil = true;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.enableExtensionsByDefault = true;
    ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    if (!ems->ctx) {
        neko_println("Emscripten Init: Unable to create webgl2 context. Reverting to webgl1.");
        attrs.majorVersion = 1;
        ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    } else {
        neko_println("Emscripten Init: Successfully created webgl2 context.");
    }
    if (emscripten_webgl_make_context_current(ems->ctx) != EMSCRIPTEN_RESULT_SUCCESS) {
        neko_println("Emscripten Init: Unable to set current webgl context.");
    }
}

NEKO_API_DECL void neko_pf_lock_mouse(u32 handle, b32 lock) {
    neko_pf_t* platform = neko_subsystem(platform);
    neko_ems_t* ems = (neko_ems_t*)platform->user_data;
    // if (platform->input.mouse.locked == lock) return;
    platform->input.mouse.locked = lock;
    if (log_lock) {
        emscripten_request_pointerlock(ems->canvas_name, true);
    } else {
        emscripten_exit_pointerlock();
    }
}

NEKO_API_DECL void neko_pf_shutdown(neko_pf_t* platform) {
    // Free memory
}

NEKO_API_DECL double neko_pf_elapsed_time() { return emscripten_performance_now(); }

// Platform Video
NEKO_API_DECL void neko_pf_enable_vsync(s32 enabled) {
    // Nothing for now...
}

// Platform Util
NEKO_API_DECL void neko_pf_sleep(f32 ms) { emscripten_sleep((u32)ms); }

NEKO_API_DECL void neko_pf_update_internal(neko_pf_t* platform) {}

// Platform Input
NEKO_API_DECL void neko_pf_process_input(neko_pf_input_t* input) {
    neko_ems_t* ems = NEKO_EMS_DATA();

    // Set mouse buttons
    /*
    for (u32 i = 0; i < NEKO_MOUSE_BUTTON_CODE_COUNT; ++i) {
        if (ems->mouse_down[i]) neko_pf_press_mouse_button((neko_pf_mouse_button_code)i);
        else                    neko_pf_release_mouse_button((neko_pf_mouse_button_code)i);
    }
    */

    // Check for pointerlock, because Chrome is retarded.
    EmscriptenPointerlockChangeEvent evt = NEKO_DEFAULT_VAL();
    emscripten_get_pointerlock_status(&evt);
    if (neko_pf_mouse_locked() && !evt.isActive) {
        neko_subsystem(platform)->input.mouse.locked = false;
    }
}

NEKO_API_DECL void neko_pf_mouse_set_position(u32 handle, f32 x, f32 y) {
    // Not sure this is possible...
    struct neko_pf_t* platform = neko_subsystem(platform);
    platform->input.mouse.position = neko_v2(x, y);
}

NEKO_API_DECL neko_pf_window_t neko_pf_window_create_internal(const neko_pf_window_desc_t* desc) {
    // Nothing for now, since we just create this internally...
    neko_pf_window_t win = NEKO_DEFAULT_VAL();
    return win;
}

NEKO_API_DECL void neko_pf_window_swap_buffer(u32 handle) {
    // Nothing for emscripten...but could handle swapping manually if preferred.
}

NEKO_API_DECL neko_vec2 neko_pf_window_sizev(u32 handle) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    return neko_v2((f32)ems->canvas_width, (f32)ems->canvas_height);
}

NEKO_API_DECL void neko_pf_window_size(u32 handle, u32* w, u32* h) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    *w = (u32)ems->canvas_width;
    *h = (u32)ems->canvas_height;
}

NEKO_API_DECL u32 neko_pf_window_width(u32 handle) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    return (u32)ems->canvas_width;
}

NEKO_API_DECL u32 neko_pf_window_height(u32 handle) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    return (u32)ems->canvas_height;
}

NEKO_API_DECL b32 neko_pf_window_fullscreen(u32 handle) { return false; }

NEKO_API_DECL void neko_pf_window_position(u32 handle, u32* x, u32* y) {}

NEKO_API_DECL neko_vec2 neko_pf_window_positionv(u32 handle) { return neko_v2(0, 0); }

NEKO_API_DECL void neko_pf_set_window_size(u32 handle, u32 width, u32 height) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, width, height);
    ems->canvas_width = (u32)width;
    ems->canvas_height = (u32)height;
}

NEKO_API_DECL void neko_pf_set_window_sizev(u32 handle, neko_vec2 v) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, (u32)v.x, (u32)v.y);
    ems->canvas_width = (u32)v.x;
    ems->canvas_height = (u32)v.y;
}

NEKO_API_DECL void neko_pf_set_window_fullscreen(u32 handle, b32 fullscreen) {}

NEKO_API_DECL void neko_pf_set_window_position(u32 handle, u32 x, u32 y) {}

NEKO_API_DECL void neko_pf_set_window_positionv(u32 handle, neko_vec2 v) {}

NEKO_API_DECL void neko_pf_set_cursor(u32 handle, neko_pf_cursor cursor) {}

NEKO_API_DECL void neko_pf_set_dropped_files_callback(u32 handle, neko_dropped_files_callback_t cb) {}

NEKO_API_DECL void neko_pf_set_window_close_callback(u32 handle, neko_window_close_callback_t cb) {}

NEKO_API_DECL void neko_pf_set_character_callback(u32 handle, neko_character_callback_t cb) {}

NEKO_API_DECL void* neko_pf_raw_window_handle(u32 handle) { return NULL; }

NEKO_API_DECL void neko_pf_framebuffer_size(u32 handle, u32* w, u32* h) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    // double dpi = emscripten_get_device_pixel_ratio();
    *w = (u32)(ems->canvas_width);
    *h = (u32)(ems->canvas_height);
}

NEKO_API_DECL neko_vec2 neko_pf_framebuffer_sizev(u32 handle) {
    u32 w = 0, h = 0;
    neko_pf_framebuffer_size(handle, &w, &h);
    return neko_v2(w, h);
}

NEKO_API_DECL u32 neko_pf_framebuffer_width(u32 handle) {
    // Get ems width for now. Don't use handle.
    neko_ems_t* ems = NEKO_EMS_DATA();
    return (u32)ems->canvas_width;
}

NEKO_API_DECL u32 neko_pf_framebuffer_height(u32 handle) {
    neko_ems_t* ems = NEKO_EMS_DATA();
    return (u32)ems->canvas_height;
}

void neko_pf_symbol_handler_init() {}
void neko_pf_stacktrace() {}

#ifndef NEKO_NO_HIJACK_MAIN
s32 main(s32 argc, char** argv) {
    neko_app_desc_t app = neko_main(argc, argv);
    neko_create(app);
    emscripten_set_main_loop(neko_frame, (s32)app.window.frame_rate, true);
    return 0;
}
#endif  // NEKO_NO_HIJACK_MAIN

#undef NEKO_PF_IMPL_EMSCRIPTEN
#endif  // NEKO_PF_IMPL_EMSCRIPTEN

#endif
