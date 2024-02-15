#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include "sandbox/neko_imgui_auto.hpp"
#include "sandbox/neko_profiler.h"

#define __neko_desired_frame_rate 30.0f
#define __neko_minimum_frame_rate 20.0f
#define __neko_flash_time_in_ms 333.0

struct pan_and_zoon {
    f32 offset;
    f32 start_pan;
    f32 zoom;

    pan_and_zoon() {
        offset = 0.0f;
        start_pan = 0.0f;
        zoom = 1.0f;
    }

    inline f32 w2s(f32 wld, f32 minX, f32 maxX) { return minX + wld * (maxX - minX) * zoom - offset; }
    inline f32 w2sdelta(f32 wld, f32 minX, f32 maxX) { return wld * (maxX - minX) * zoom; }
    inline f32 s2w(f32 scr, f32 minX, f32 maxX) { return (scr + offset - minX) / ((maxX - minX) * zoom); }
};

struct frame_info {
    f32 time;
    u32 offset;
    u32 size;
};

int neko_profiler_draw_frame(neko_profiler_frame_t *_data, void *_buffer = 0, size_t _bufferSize = 0, bool _inGame = true, bool _multi = false);
void neko_profiler_draw_stats(neko_profiler_frame_t *_data, bool _multi = false);

enum log_type { LOG_TYPE_WARNING = 1, LOG_TYPE_ERROR = 2, LOG_TYPE_TRACE = 4, LOG_TYPE_SUCCESS = 0, LOG_TYPE_MESSAGE = 3 };

class game_editor_console {
public:
    void display_full(bool *bInteractingWithTextbox) noexcept;
    void display(bool *bInteractingWithTextbox) noexcept;

    static void add_to_message_log(const std::string &msg, log_type type) noexcept;
    // static void add_command(const command_type &cmd) noexcept;

    neko_inline void log(const std::string &output, log_type type) noexcept { message_log.emplace_back(std::make_pair(output, type)); }

private:
    std::vector<std::pair<std::string, log_type>> message_log;

    ImVec4 success = {0.0f, 1.0f, 0.0f, 1.0f};
    ImVec4 warning = {1.0f, 1.0f, 0.0f, 1.0f};
    ImVec4 error = {1.0f, 0.0f, 0.0f, 1.0f};
    ImVec4 note = rgba_to_imvec(0, 183, 255, 255);
    ImVec4 message = {1.0f, 1.0f, 1.0f, 1.0f};
};

// 生成宏 以避免始终重复代码
#define R_INTROSPECTION_GENERATE_VARIABLE_RENDER(cputype, count, gltype, glread, glwrite, imguifunc) \
    {                                                                                                \
        ImGui::Text(#gltype " %s:", name);                                                           \
        cputype value[count];                                                                        \
        glread(program, location, &value[0]);                                                        \
        if (imguifunc("", &value[0], 0.25f)) glwrite(program, location, 1, &value[0]);               \
    }

#define R_INTROSPECTION_GENERATE_MATRIX_RENDER(cputype, rows, columns, gltype, glread, glwrite, imguifunc) \
    {                                                                                                      \
        ImGui::Text(#gltype " %s:", name);                                                                 \
        cputype value[rows * columns];                                                                     \
        int size = rows * columns;                                                                         \
        glread(program, location, &value[0]);                                                              \
        int modified = 0;                                                                                  \
        for (int i = 0; i < size; i += rows) {                                                             \
            ImGui::PushID(i);                                                                              \
            modified += imguifunc("", &value[i], 0.25f);                                                   \
            ImGui::PopID();                                                                                \
        }                                                                                                  \
        if (modified) glwrite(program, location, 1, GL_FALSE, value);                                      \
    }

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location);

float get_scrollable_height();

void inspect_shader(const char *label, GLuint program);
void inspect_vertex_array(const char *label, GLuint vao);

#endif