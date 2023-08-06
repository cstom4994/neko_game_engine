#include "engine/platform/neko_platform.h"

#include "engine/base/neko_engine.h"
#include "engine/common/neko_mem.h"
#include "engine/utility/hash.hpp"

/*============================
// Platform Window
============================*/

neko_resource_handle __neko_platform_create_window(const char* title, u32 width, u32 height) {
    struct neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    void* win = platform->create_window_internal(title, width, height);
    neko_assert(win);
    neko_resource_handle handle = neko_slot_array_insert(platform->windows, win);
    neko_dyn_array_push(platform->active_window_handles, handle);
    return handle;
}

neko_resource_handle __neko_platform_main_window() {
    // Should be the first element of the slot array...Great assumption to make.
    return 0;
}

/*============================
// Platform Input
============================*/

#define __input() (&neko_engine_instance()->ctx.platform->input)

void __neko_platform_update_input(neko_platform_input*) {
    neko_platform_input* input = __input();

    // Update all input and mouse keys from previous frame
    // Previous key presses
    neko_for_range_i(neko_keycode_count) { input->prev_key_map[i] = input->key_map[i]; }

    // Previous mouse button presses
    neko_for_range_i(neko_mouse_button_code_count) { input->mouse.prev_button_map[i] = input->mouse.button_map[i]; }

    input->mouse.prev_position = input->mouse.position;
    input->mouse.wheel = neko_vec2{0.0f, 0.0f};
    input->mouse.moved_this_frame = false;
}

b32 __neko_platform_was_key_down(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (input->prev_key_map[code]);
}

b32 __neko_platform_key_down(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (input->key_map[code]);
}

b32 __neko_platform_key_pressed(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (__neko_platform_key_down(code) && !__neko_platform_was_key_down(code)) {
        return true;
    }
    return false;
}

b32 __neko_platform_key_released(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (__neko_platform_was_key_down(code) && !__neko_platform_key_down(code));
}

b32 __neko_platform_was_mouse_down(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (input->mouse.prev_button_map[code]);
}

void __neko_platform_press_mouse_button(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if ((u32)code < (u32)neko_mouse_button_code_count) {
        input->mouse.button_map[code] = true;
    }
}

void __neko_platform_release_mouse_button(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if ((u32)code < (u32)neko_mouse_button_code_count) {
        input->mouse.button_map[code] = false;
    }
}

b32 __neko_platform_mouse_down(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (input->mouse.button_map[code]);
}

b32 __neko_platform_mouse_pressed(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if (__neko_platform_mouse_down(code) && !__neko_platform_was_mouse_down(code)) {
        return true;
    }
    return false;
}

b32 __neko_platform_mouse_released(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (__neko_platform_was_mouse_down(code) && !__neko_platform_mouse_down(code));
}

neko_vec2 __neko_platform_mouse_delta() {
    neko_platform_input* input = __input();

    if (input->mouse.prev_position.x < 0.0f || input->mouse.prev_position.y < 0.0f || input->mouse.position.x < 0.0f || input->mouse.position.y < 0.0f) {
        return neko_vec2{0.0f, 0.0f};
    }

    return neko_vec2{input->mouse.position.x - input->mouse.prev_position.x, input->mouse.position.y - input->mouse.prev_position.y};
}

neko_vec2 __neko_platform_mouse_position() {
    neko_platform_input* input = __input();

    return neko_vec2{input->mouse.position.x, input->mouse.position.y};
}

void __neko_platform_mouse_position_x_y(f32* x, f32* y) {
    neko_platform_input* input = __input();
    *x = input->mouse.position.x;
    *y = input->mouse.position.y;
}

void __neko_platform_mouse_wheel(f32* x, f32* y) {
    neko_platform_input* input = __input();
    *x = input->mouse.wheel.x;
    *y = input->mouse.wheel.y;
}

void __neko_platform_press_key(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (code < neko_keycode_count) {
        input->key_map[code] = true;
    }
}

void __neko_platform_release_key(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (code < neko_keycode_count) {
        input->key_map[code] = false;
    }
}

/*============================
// Platform File I/O
============================*/

#define __ctx() (&neko_engine_instance()->ctx.platform->ctx)

b32 __neko_platform_file_exists(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

u32 __neko_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

#ifdef NEKO_PLATFORM_WIN
#else
#include <sys/stat.h>
#endif

s32 __neko_platform_file_size_in_bytes(const char* file_path) {
#ifdef NEKO_PLATFORM_WIN

    HANDLE hFile = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;  // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;  // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return __neko_safe_truncate_u64(size.QuadPart);

#else

    struct stat st;
    stat(file_path, &st);
    return (s32)st.st_size;

#endif
}

neko_result __neko_write_file_contents(const char* file_path, const char* mode, void* data, usize data_type_size, usize data_size) {
    FILE* fp = fopen(file_path, "wb");
    if (fp) {
        s32 ret = fwrite(data, data_type_size, data_size, fp);
        fclose(fp);
        if (ret == data_size) {
            return neko_result_success;
        }
    }

    return neko_result_failure;
}

char* __neko_platform_read_file_contents_into_string_null_term(const char* file_path, const char* mode, s32* sz) {
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    usize _sz = 0;
    if (fp) {
        _sz = __neko_platform_file_size_in_bytes(file_path);
        // fseek(fp, 0, SEEK_END);
        // _sz = ftell(fp);
        // fseek(fp, 0, SEEK_SET);
        buffer = (char*)neko_safe_malloc(_sz);
        if (buffer) {
            fread(buffer, 1, _sz, fp);
        }
        fclose(fp);
        // buffer[_sz] = '\0';
    }
    if (sz) *sz = _sz;
    return buffer;
}

neko_result __neko_platform_write_str_to_file(const char* contents, const char* mode, usize sz, const char* output_path) {
    FILE* fp = fopen(output_path, mode);
    if (fp) {
        s32 ret = fwrite(contents, sizeof(u8), sz, fp);
        if (ret == sz) {
            return neko_result_success;
        }
    }
    return neko_result_failure;
}

void __neko_platform_file_extension(char* buffer, usize buffer_sz, const char* file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

neko_string __neko_platform_get_path(const neko_string& path) {

    neko_platform_ctx* ctx = __ctx();

    if (!ctx->gamepath) {
        neko_assert(ctx->gamepath, "gamepath not detected");
        return path;
    }

    neko_string get_path(ctx->gamepath);

    switch (neko::hash(path.substr(0, 4))) {
        case neko::hash("data"):
            get_path.append(path);
            break;
        default:
            break;
    }

    return get_path;
}

/*============================
// Platform UUID
============================*/

struct neko_uuid __neko_platform_generate_uuid() {
    struct neko_uuid uuid;

    srand(clock());
    char guid[40];
    s32 t = 0;
    char* sz_temp = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
    char* sz_hex = "0123456789abcdef-";
    s32 n_len = strlen(sz_temp);

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
    const char *hex_string = sz_temp, *pos = hex_string;

    /* WARNING: no sanitization or error-checking whatsoever */
    for (usize count = 0; count < 16; count++) {
        sscanf(pos, "%2hhx", &uuid.bytes[count]);
        pos += 2;
    }

    return uuid;
}

// Mutable temp buffer 'tmp_buffer'
void __neko_platform_uuid_to_string(char* tmp_buffer, const struct neko_uuid* uuid) {
    neko_snprintf(tmp_buffer, 32, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", uuid->bytes[0], uuid->bytes[1], uuid->bytes[2], uuid->bytes[3], uuid->bytes[4], uuid->bytes[5],
                  uuid->bytes[6], uuid->bytes[7], uuid->bytes[8], uuid->bytes[9], uuid->bytes[10], uuid->bytes[11], uuid->bytes[12], uuid->bytes[13], uuid->bytes[14], uuid->bytes[15]);
}

u32 __neko_platform_hash_uuid(const struct neko_uuid* uuid) {
    char temp_buffer[] = neko_uuid_temp_str_buffer();
    __neko_platform_uuid_to_string(temp_buffer, uuid);
    return (neko_hash_str(temp_buffer));
}

void __neko_default_init_platform(struct neko_platform_i* platform) {
    neko_assert(platform);

    // Just assert these for now
    __neko_verify_platform_correctness(platform);

    // Initialize random with time
    srand(time(0));

    /*============================
    // Platform Window
    ============================*/
    platform->windows = neko_slot_array_new(neko_platform_window_ptr);
    platform->active_window_handles = neko_dyn_array_new(neko_resource_handle);
    platform->create_window = &__neko_platform_create_window;
    platform->main_window = &__neko_platform_main_window;

    /*============================
    // Platform Input
    ============================*/
    platform->update_input = &__neko_platform_update_input;
    platform->press_key = &__neko_platform_press_key;
    platform->release_key = &__neko_platform_release_key;
    platform->was_key_down = &__neko_platform_was_key_down;
    platform->key_pressed = &__neko_platform_key_pressed;
    platform->key_down = &__neko_platform_key_down;
    platform->key_released = &__neko_platform_key_released;

    platform->press_mouse_button = &__neko_platform_press_mouse_button;
    platform->release_mouse_button = &__neko_platform_release_mouse_button;
    platform->was_mouse_down = &__neko_platform_was_mouse_down;
    platform->mouse_pressed = &__neko_platform_mouse_pressed;
    platform->mouse_down = &__neko_platform_mouse_down;
    platform->mouse_released = &__neko_platform_mouse_released;

    platform->mouse_delta = &__neko_platform_mouse_delta;
    platform->mouse_position = &__neko_platform_mouse_position;
    platform->mouse_position_x_y = &__neko_platform_mouse_position_x_y;
    platform->mouse_wheel = &__neko_platform_mouse_wheel;

    /*============================
    // Platform UUID
    ============================*/
    platform->generate_uuid = &__neko_platform_generate_uuid;
    platform->uuid_to_string = &__neko_platform_uuid_to_string;
    platform->hash_uuid = &__neko_platform_hash_uuid;

    /*============================
    // Platform File IO
    ============================*/

    platform->file_exists = &__neko_platform_file_exists;
    platform->read_file_contents = &__neko_platform_read_file_contents_into_string_null_term;
    platform->write_file_contents = &__neko_write_file_contents;
    platform->write_str_to_file = &__neko_platform_write_str_to_file;
    platform->file_size_in_bytes = &__neko_platform_file_size_in_bytes;
    platform->file_extension = &__neko_platform_file_extension;
    platform->get_path = &__neko_platform_get_path;

    // Default world time initialization
    platform->time.max_fps = 60.0;
    platform->time.current = 0.0;
    platform->time.delta = 0.0;
    platform->time.update = 0.0;
    platform->time.render = 0.0;
    platform->time.previous = 0.0;
    platform->time.frame = 0.0;

    // Custom initialize plaform layer
    platform->init(platform);
}

void __neko_verify_platform_correctness(struct neko_platform_i* platform) {
    neko_assert(platform);
    neko_assert(platform->init);
    neko_assert(platform->shutdown);
    neko_assert(platform->sleep);
    neko_assert(platform->elapsed_time);
    neko_assert(platform->process_input);
    neko_assert(platform->create_window_internal);
    neko_assert(platform->window_swap_buffer);
    neko_assert(platform->set_window_size);
    neko_assert(platform->window_size);
    neko_assert(platform->window_size_w_h);
    neko_assert(platform->set_cursor);
    neko_assert(platform->enable_vsync);
}
