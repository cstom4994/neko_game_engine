
#include "shader.h"

#include "base/common/util.hpp"
#include "base/common/logger.hpp"
#include "base/common/vfs.hpp"

namespace Neko {

bool neko_init_shader(AssetShader* shader, char* source) {
    neko_assert(shader);

    shader->panic_mode = false;

    const u32 source_len = (u32)strlen(source);

    char* vertex_source = (char*)mem_alloc(source_len);
    char* fragment_source = (char*)mem_alloc(source_len);
    char* geometry_source = (char*)mem_alloc(source_len);
    neko_defer(mem_free(vertex_source));
    neko_defer(mem_free(fragment_source));
    neko_defer(mem_free(geometry_source));

    memset(vertex_source, 0, source_len);
    memset(fragment_source, 0, source_len);
    memset(geometry_source, 0, source_len);

    bool has_geometry = false;

    u32 count = 0;
    i32 adding_to = -1;
    for (char* current = source; *current != '\0'; current++) {
        if (*current == '\n' && *(current + 1) != '\0') {
            i32 minus = 1;

            current++;

            char* line = current - count - minus;
            line[count] = '\0';

            if (strstr(line, "#begin VERTEX")) {
                adding_to = 0;
            } else if (strstr(line, "#begin FRAGMENT")) {
                adding_to = 1;
            } else if (strstr(line, "#begin GEOMETRY")) {
                adding_to = 2;
                has_geometry = true;
            } else if (strstr(line, "#end VERTEX") || strstr(line, "#end FRAGMENT") || strstr(line, "#end GEOMETRY")) {
                adding_to = -1;
            } else if (adding_to == 0) {
                strcat(vertex_source, line);
                strcat(vertex_source, "\n");
            } else if (adding_to == 1) {
                strcat(fragment_source, line);
                strcat(fragment_source, "\n");
            } else if (adding_to == 2) {
                strcat(geometry_source, line);
                strcat(geometry_source, "\n");
            }

            count = 0;
        }

        count++;
    }

    const char* vsp = vertex_source;
    const char* fsp = fragment_source;
    const char* gsp = geometry_source;

    i32 success;
    u32 v, f, g;
    v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vsp, NULL);
    glCompileShader(v);

    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(v, 1024, 0x0, info_log);
        LOG_INFO("error {}", info_log);
        shader->panic_mode = true;
    }

    f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fsp, NULL);
    glCompileShader(f);

    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(f, 1024, 0x0, info_log);
        LOG_INFO("error {}", info_log);
        shader->panic_mode = true;
    }

    if (has_geometry) {
        g = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(g, 1, &gsp, NULL);
        glCompileShader(g);

        glGetShaderiv(g, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[1024];
            glGetShaderInfoLog(g, 1024, 0x0, info_log);
            LOG_INFO("error {}", info_log);
            shader->panic_mode = true;
        }
    }

    u32 id;
    id = glCreateProgram();
    glAttachShader(id, v);
    glAttachShader(id, f);
    if (has_geometry) {
        glAttachShader(id, g);
    }
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        shader->panic_mode = true;
    }

    glDeleteShader(v);
    glDeleteShader(f);

    if (has_geometry) {
        glDeleteShader(g);
    }

    shader->id = id;
    return !shader->panic_mode;
}

void neko_unload_shader(AssetShader* shader) {
    neko_assert(shader);
    if (shader->panic_mode) return;
    glDeleteProgram(shader->id);
}

bool neko_load_shader(AssetShader* shader, String path) {
    neko_assert(shader);

    String source = {};
    bool ok = vfs_read_entire_file(&source, path);
    neko_defer(mem_free(source.data));

    if (ok) {
        ok = neko_init_shader(shader, source.data);
    }

    return ok;
}

void neko_bind_shader(u32 shader) { glUseProgram(shader); }

void neko_shader_set_int(u32 shader, const char* name, i32 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1i(location, v);
}

void neko_shader_set_uint(u32 shader, const char* name, u32 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1ui(location, v);
}

void neko_shader_set_float(u32 shader, const char* name, float v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1f(location, v);
}

void neko_shader_set_color(u32 shader, const char* name, neko_color_t color) {

    neko_rgb_color_t rgb = neko_rgb_color_from_color(color);

    neko_shader_set_v3f(shader, name, vec3{rgb.r, rgb.g, rgb.b});
}

void neko_shader_set_rgb_color(u32 shader, const char* name, neko_rgb_color_t color) { neko_shader_set_v3f(shader, name, vec3{color.r, color.g, color.b}); }

void neko_shader_set_v2f(u32 shader, const char* name, vec2 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform2f(location, v.x, v.y);
}

void neko_shader_set_v3f(u32 shader, const char* name, vec3 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform3f(location, v.x, v.y, v.z);
}

void neko_shader_set_v4f(u32 shader, const char* name, vec4 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void neko_shader_set_m4f(u32 shader, const char* name, mat4 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)v.elements);
}

}  // namespace Neko