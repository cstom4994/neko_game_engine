#include "engine/edit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <typeindex>
#include <vector>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/luax.hpp"
#include "engine/scripting.h"
#include "engine/ui.h"

// deps

#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_rect_pack.h>

#if 1

// 生成宏 以避免始终重复代码
#define INSPECTOR_GENERATE_VARIABLE(cputype, count, gltype, glread, glwrite, imguifunc)               \
    {                                                                                                 \
        ui_labelf(#gltype " %s:", name);                                                              \
        cputype value[count];                                                                         \
        glread(program, location, &value[0]);                                                         \
        if (imguifunc(ui, (ui_real *)&value[0], -1.f, 1.f)) glwrite(program, location, 1, &value[0]); \
    }

#define INSPECTOR_GENERATE_MATRIX(cputype, rows, columns, gltype, glread, glwrite, imguifunc) \
    {                                                                                         \
        ui_labelf(#gltype " %s:", name);                                                      \
        cputype value[rows * columns];                                                        \
        int size = rows * columns;                                                            \
        glread(program, location, &value[0]);                                                 \
        int modified = 0;                                                                     \
        for (int i = 0; i < size; i += rows) {                                                \
            /*ImGui::PushID(i);*/                                                             \
            modified += imguifunc(ui, (ui_real *)&value[i], -1.f, 1.f);                       \
            /*ImGui::PopID();*/                                                               \
        }                                                                                     \
        if (modified) glwrite(program, location, 1, GL_FALSE, value);                         \
    }

void render_uniform_variable(GLuint program, GLenum type, const char *name, GLint location) {
    ui_context_t *ui = g_app->ui;
    static bool is_color = false;
    switch (type) {
        case GL_FLOAT:
            INSPECTOR_GENERATE_VARIABLE(GLfloat, 1, GL_FLOAT, glGetUniformfv, glProgramUniform1fv, ui_slider);
            break;

        case GL_FLOAT_VEC2:
            INSPECTOR_GENERATE_VARIABLE(GLfloat, 2, GL_FLOAT_VEC2, glGetUniformfv, glProgramUniform2fv, ui_slider);
            break;

        case GL_FLOAT_VEC3: {
            ui_checkbox(ui, "##is_color", (i32 *)&is_color);
            // ImGui::SameLine();
            ui_labelf("GL_FLOAT_VEC3 %s", name);
            // ImGui::SameLine();
            float value[3];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ui_slider(ui, &value[0], -1.f, 1.f)) || (is_color && ui_slider(ui, &value[0], -1.f, 1.f))) {
                glProgramUniform3fv(program, location, 1, &value[0]);
            }
        } break;

        case GL_FLOAT_VEC4: {
            ui_checkbox(ui, "##is_color", (i32 *)&is_color);
            // ImGui::SameLine();
            ui_labelf("GL_FLOAT_VEC4 %s", name);
            // ImGui::SameLine();
            float value[4];
            glGetUniformfv(program, location, &value[0]);
            if ((!is_color && ui_slider(ui, &value[0], -1.f, 1.f)) || (is_color && ui_slider(ui, &value[0], -1.f, 1.f))) {
                glProgramUniform4fv(program, location, 1, &value[0]);
            }
        } break;

        case GL_INT:
            INSPECTOR_GENERATE_VARIABLE(GLint, 1, GL_INT, glGetUniformiv, glProgramUniform1iv, ui_slider);
            break;

        case GL_INT_VEC2:
            INSPECTOR_GENERATE_VARIABLE(GLint, 2, GL_INT, glGetUniformiv, glProgramUniform2iv, ui_slider);
            break;

        case GL_INT_VEC3:
            INSPECTOR_GENERATE_VARIABLE(GLint, 3, GL_INT, glGetUniformiv, glProgramUniform3iv, ui_slider);
            break;

        case GL_INT_VEC4:
            INSPECTOR_GENERATE_VARIABLE(GLint, 4, GL_INT, glGetUniformiv, glProgramUniform4iv, ui_slider);
            break;

        case GL_UNSIGNED_INT: {
            ui_labelf("GL_UNSIGNED_INT %s:", name);
            // ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ui_slider(ui, (ui_real *)&value[0], -1.f, 1.f)) glProgramUniform1uiv(program, location, 1, &value[0]);
        } break;

        case GL_UNSIGNED_INT_VEC3: {
            ui_labelf("GL_UNSIGNED_INT_VEC3 %s:", name);
            // ImGui::SameLine();
            GLuint value[1];
            glGetUniformuiv(program, location, &value[0]);
            if (ui_slider(ui, (ui_real *)&value[0], -1.f, 1.f)) glProgramUniform3uiv(program, location, 1, &value[0]);
        } break;

        case GL_SAMPLER_2D:
            INSPECTOR_GENERATE_VARIABLE(GLint, 1, GL_SAMPLER_2D, glGetUniformiv, glProgramUniform1iv, ui_slider);
            break;

        case GL_FLOAT_MAT2:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 2, 2, GL_FLOAT_MAT2, glGetUniformfv, glProgramUniformMatrix2fv, ui_slider);
            break;

        case GL_FLOAT_MAT3:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 3, 3, GL_FLOAT_MAT3, glGetUniformfv, glProgramUniformMatrix3fv, ui_slider);
            break;

        case GL_FLOAT_MAT4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 4, GL_FLOAT_MAT4, glGetUniformfv, glProgramUniformMatrix4fv, ui_slider);
            break;

        case GL_FLOAT_MAT2x3:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 3, 2, GL_FLOAT_MAT2x3, glGetUniformfv, glProgramUniformMatrix2x3fv, ui_slider);
            break;

        case GL_FLOAT_MAT2x4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 2, GL_FLOAT_MAT2x4, glGetUniformfv, glProgramUniformMatrix2x4fv, ui_slider);
            break;

        case GL_FLOAT_MAT3x2:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 2, 3, GL_FLOAT_MAT3x2, glGetUniformfv, glProgramUniformMatrix3x2fv, ui_slider);
            break;

        case GL_FLOAT_MAT3x4:
            INSPECTOR_GENERATE_MATRIX(GLfloat, 4, 3, GL_FLOAT_MAT3x4, glGetUniformfv, glProgramUniformMatrix3x2fv, ui_slider);
            break;

        case GL_BOOL: {
            ui_labelf("GL_BOOL %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            if (ui_checkbox(ui, "", (i32 *)&value)) glProgramUniform1uiv(program, location, 1, &value);
        } break;

            // #if !defined(NEKO_IS_APPLE)
            //         case GL_IMAGE_2D: {
            //              ui_labelf("GL_IMAGE_2D %s:", name);
            //             // ImGui::SameLine();
            //             GLuint value;
            //             glGetUniformuiv(program, location, &value);
            //             // if (ImGui::Checkbox("", (bool*)&value)) glProgramUniform1iv(program, location, 1, &value);
            //             ImGui::Image((void*)(intptr_t)value, ImVec2(256, 256));
            //         } break;
            // #endif

        case GL_SAMPLER_CUBE: {
            ui_labelf("GL_SAMPLER_CUBE %s:", name);
            // ImGui::SameLine();
            GLuint value;
            glGetUniformuiv(program, location, &value);
            // ImGui::Image((void*)(intptr_t)value, ImVec2(256, 256));
            // ui_image(ui, (void *)(intptr_t)value);
        } break;

        default:
            ui_labelf("%s has type %s, which isn't supported yet!", name, opengl_string(type));
            break;
    }
}

// float get_scrollable_height() { return ImGui::GetTextLineHeight() * 16; }

void inspect_shader(const char *label, GLuint program) {
    neko_assert(label != nullptr);

    ui_context_t *ui = g_app->ui;

    // ImGui::PushID(label);
    if (ui_header(ui, label)) {
        if (!glIsProgram(program)) {
            ui_labelf("%d glIsProgram failed", program);
        } else {
            // Uniforms

            if (ui_header(ui, "Uniforms")) {
                GLint uniform_count;
                glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

                // Read the length of the longest active uniform.
                GLint max_name_length;
                glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);

                static std::vector<char> name;
                name.resize(max_name_length);

                for (int i = 0; i < uniform_count; i++) {
                    GLint ignored;
                    GLenum type;
                    glGetActiveUniform(program, i, max_name_length, nullptr, &ignored, &type, name.data());

                    const auto location = glGetUniformLocation(program, name.data());

                    // ImGui::PushID(i);
                    // ImGui::PushItemWidth(-1.0f);
                    render_uniform_variable(program, type, name.data(), location);
                    // ImGui::PopItemWidth();
                    // ImGui::PopID();
                    //
                }
            }
            //

            // Shaders

            if (ui_header(ui, "Shaders")) {
                GLint shader_count;
                glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);

                static std::vector<GLuint> attached_shaders;
                attached_shaders.resize(shader_count);
                glGetAttachedShaders(program, shader_count, nullptr, attached_shaders.data());

                for (const auto &shader : attached_shaders) {
                    GLint source_length = 0;
                    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &source_length);
                    static std::vector<char> source;
                    source.resize(source_length);
                    glGetShaderSource(shader, source_length, nullptr, source.data());

                    GLint type = 0;
                    glGetShaderiv(shader, GL_SHADER_TYPE, &type);

                    auto string_type = opengl_string(type);
                    // ImGui::PushID(string_type);
                    if (ui_header(ui, string_type)) {
                        ui_text(ui, source.data());
                    }
                    // ImGui::PopID();
                    //
                }
            }
            //
        }
    }
    // ImGui::PopID();
}

void inspect_vertex_array(const char *label, GLuint vao) {
    neko_assert(label != nullptr);
    neko_assert(glIsVertexArray(vao));

    ui_context_t *ui = g_app->ui;

    // ImGui::PushID(label);
    if (ui_header(ui, label)) {

        // 获取当前绑定的顶点缓冲区对象 以便我们可以在完成后将其重置回来
        GLint current_vbo = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vbo);

        // 获取当前绑定的顶点数组对象 以便我们可以在完成后将其重置回来
        GLint current_vao = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
        glBindVertexArray(vao);

        // 获取顶点属性的最大数量
        // 无论这里有多少个属性 迭代都应该是合理的
        GLint max_vertex_attribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

        GLint ebo = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);

        // EBO Visualization
        char buffer[128];
        std::snprintf(buffer, 128, "Element Array Buffer: %d", ebo);
        // ImGui::PushID(buffer);
        if (ui_header(ui, buffer)) {

            // 假设为 unsigned int
            int size = 0;
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            size /= sizeof(GLuint);
            ui_labelf("Size: %d", size);

            if (ui_header(ui, "Buffer Contents")) {
                // TODO 找到一种更好的方法将其显示在屏幕上 因为当我们获得大量索引时 该解决方案可能不会有很好的伸缩性
                // 可能的解决方案 像VBO一样将其做成列 并将索引显示为三角形
                auto ptr = (GLuint *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
                for (int i = 0; i < size; i++) {
                    ui_labelf("%u", ptr[i]);
                    // ImGui::SameLine();
                    if ((i + 1) % 3 == 0) {
                        // ImGui::NewLine();
                    }
                }

                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

                // ImGui::TreePop();
            }
        }
        // ImGui::PopID();

        // VBO Visualization
        for (intptr_t i = 0; i < max_vertex_attribs; i++) {
            GLint enabled = 0;
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

            if (!enabled) continue;

            std::snprintf(buffer, 128, "Attribute: %" PRIdPTR "", i);
            // ImGui::PushID(buffer);
            if (ui_header(ui, buffer)) {

                // 元数据显示
                GLint buffer = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
                ui_labelf("Buffer: %d", buffer);

                GLint type = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                ui_labelf("Type: %s", opengl_string(type));

                GLint dimensions = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &dimensions);
                ui_labelf("Dimensions: %d", dimensions);

                // 需要绑定缓冲区以访问 parameteriv 并在以后进行映射
                glBindBuffer(GL_ARRAY_BUFFER, buffer);

                GLint size = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                ui_labelf("Size in bytes: %d", size);

                GLint stride = 0;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
                ui_labelf("Stride in bytes: %d", stride);

                GLvoid *offset = nullptr;
                glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
                ui_labelf("Offset in bytes: %" PRIdPTR "", (intptr_t)offset);

                GLint usage = 0;
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
                ui_labelf("Usage: %s", opengl_string(usage));

                // 创建包含索引和实际内容的表
                if (ui_header(ui, "Buffer Contents")) {
                    // ImGui::BeginChild(ImGui::GetID("vbo contents"), ImVec2(-1.0f, get_scrollable_height()), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    // ImGui::Columns(dimensions + 1);
                    // const char *descriptors[] = {"index", "x", "y", "z", "w"};
                    // for (int j = 0; j < dimensions + 1; j++) {
                    //     ui_labelf("%s", descriptors[j]);
                    //     ImGui::NextColumn();
                    // }
                    // ImGui::Separator();

                    auto ptr = (char *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY) + (intptr_t)offset;
                    for (int j = 0, c = 0; j < size; j += stride, c++) {
                        ui_labelf("%d", c);
                        // ImGui::NextColumn();
                        // for (int k = 0; k < dimensions; k++) {
                        //     switch (type) {
                        //         case GL_BYTE:
                        //             ui_labelf("% d", *(GLbyte *)&ptr[j + k * sizeof(GLbyte)]);
                        //             break;
                        //         case GL_UNSIGNED_BYTE:
                        //             ui_labelf("%u", *(GLubyte *)&ptr[j + k * sizeof(GLubyte)]);
                        //             break;
                        //         case GL_SHORT:
                        //             ui_labelf("% d", *(GLshort *)&ptr[j + k * sizeof(GLshort)]);
                        //             break;
                        //         case GL_UNSIGNED_SHORT:
                        //             ui_labelf("%u", *(GLushort *)&ptr[j + k * sizeof(GLushort)]);
                        //             break;
                        //         case GL_INT:
                        //             ui_labelf("% d", *(GLint *)&ptr[j + k * sizeof(GLint)]);
                        //             break;
                        //         case GL_UNSIGNED_INT:
                        //             ui_labelf("%u", *(GLuint *)&ptr[j + k * sizeof(GLuint)]);
                        //             break;
                        //         case GL_FLOAT:
                        //             ui_labelf("% f", *(GLfloat *)&ptr[j + k * sizeof(GLfloat)]);
                        //             break;
                        //         case GL_DOUBLE:
                        //             ui_labelf("% f", *(GLdouble *)&ptr[j + k * sizeof(GLdouble)]);
                        //             break;
                        //     }
                        //     ImGui::NextColumn();
                        // }
                    }
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                    // ImGui::EndChild();
                    // ImGui::TreePop();
                }
            }
            // ImGui::PopID();
        }

        // Cleanup
        glBindVertexArray(current_vao);
        glBindBuffer(GL_ARRAY_BUFFER, current_vbo);
    }
    // ImGui::PopID();
}

#endif

static int limginfo(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int x, y, comp;
    if (!stbi_info(filename, &x, &y, &comp)) {
        return luaL_error(L, "%s", stbi_failure_reason());
    }
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, comp);
    return 3;
}

static int find_firstline(unsigned char *data, int w, int h) {
    int i;
    for (i = 0; i < w * h; i++) {
        if (data[i * 4 + 3] != 0) break;
    }
    return i / w;
}

static int find_lastline(unsigned char *data, int w, int h) {
    int i;
    for (i = w * h - 1; i >= 0; i--) {
        if (data[i * 4 + 3] != 0) break;
    }
    return i / w + 1;
}

static int find_firstrow(unsigned char *data, int w, int h) {
    int i, j;
    unsigned char *row = data;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            if (row[j * w * 4 + 3] != 0) {
                return i;
            }
        }
        row += 4;
    }
    return w;
}

static int find_lastrow(unsigned char *data, int w, int h) {
    int i, j;
    unsigned char *row = data + 4 * w;
    for (i = w - 1; i >= 0; i--) {
        row -= 4;
        for (j = 0; j < h; j++) {
            if (row[j * w * 4 + 3] != 0) {
                return i + 1;
            }
        }
    }
    return 0;
}

static int limgcrop(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);  // todo : UTF-8 support
    int w, h, n;
    unsigned char *data = stbi_load(filename, &w, &h, &n, 0);
    if (data == NULL) {
        return luaL_error(L, "Can't parse %s", filename);
    }
    if (n != 4) {
        stbi_image_free(data);
        return luaL_error(L, "No alpha channel for %s", filename);
    }
    int firstline = find_firstline(data, w, h);
    if (firstline >= h) {
        stbi_image_free(data);
        // empty
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 4;
    }
    int lastline = find_lastline(data, w, h);
    int firstrow = find_firstrow(data, w, h);
    int lastrow = find_lastrow(data, w, h);
    stbi_image_free(data);
    lua_pushinteger(L, lastrow - firstrow);
    lua_pushinteger(L, lastline - firstline);
    lua_pushinteger(L, firstrow);
    lua_pushinteger(L, firstline);
    return 4;
}

static void read_rect(lua_State *L, int index, int id, stbrp_rect *rect) {
    if (lua_geti(L, index, id + 1) != LUA_TTABLE) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    if (lua_geti(L, -1, 1) != LUA_TNUMBER) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    int w = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (w <= 0) luaL_error(L, "Invalid rect %d width %d", id + 1, w);

    if (lua_geti(L, -1, 2) != LUA_TNUMBER) {
        luaL_error(L, "Invalid rect at %d", id + 1);
    }
    int h = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (h <= 0) luaL_error(L, "Invalid rect %d height %d", id + 1, h);
    rect->id = id;
    rect->w = w;
    rect->h = h;
    rect->x = 0;
    rect->y = 0;
    rect->was_packed = 0;
    lua_pop(L, 1);
}

static void write_rect(lua_State *L, int index, int id, stbrp_rect *rect) {
    if (rect->was_packed) {
        lua_geti(L, index, id + 1);
        lua_pushinteger(L, rect->x);
        lua_setfield(L, -2, "x");
        lua_pushinteger(L, rect->y);
        lua_setfield(L, -2, "y");
        lua_pop(L, 1);
    }
}

static int lrectpack(lua_State *L) {
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    int n = lua_rawlen(L, 3);
    stbrp_rect *rect = (stbrp_rect *)lua_newuserdata(L, n * sizeof(stbrp_rect));
    int i;
    for (i = 0; i < n; i++) read_rect(L, 3, i, &rect[i]);

    stbrp_context ctx;
    stbrp_node *nodes = (stbrp_node *)lua_newuserdata(L, w * sizeof(stbrp_node));
    stbrp_init_target(&ctx, w, h, nodes, w);

    int r = stbrp_pack_rects(&ctx, rect, n);
    for (i = 0; i < n; i++) write_rect(L, 3, i, &rect[i]);
    lua_pushboolean(L, r);
    return 1;
}

static int geti(lua_State *L, int idx) {
    if (lua_geti(L, -1, idx) != LUA_TNUMBER) {
        luaL_error(L, "Invalid image");
    }
    int r = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return r;
}

static int geti_field(lua_State *L, const char *key) {
    if (lua_getfield(L, -1, key) != LUA_TNUMBER) {
        luaL_error(L, "Invalid image");
    }
    int r = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return r;
}

static inline int rect_invalid(int w, int h, int x, int y, int width, int height) { return !(x + w >= width || y + h >= height); }

static unsigned char *read_img(lua_State *L, int *w, int *h, int *x, int *y, int *stride, int *px, int *py) {
    if (lua_getfield(L, -1, "filename") != LUA_TSTRING) luaL_error(L, "No filename");
    const char *filename = lua_tostring(L, -1);
    lua_pop(L, 1);
    *w = geti(L, 1);
    *h = geti(L, 2);
    *x = geti(L, 3);
    *y = geti(L, 4);
    *px = geti_field(L, "x");
    *py = geti_field(L, "y");

    int width, height, comp;
    unsigned char *img = stbi_load(filename, &width, &height, &comp, 0);
    if (comp != 4 || rect_invalid(*w, *h, *x, *y, width, height)) {
        stbi_image_free(img);
        luaL_error(L, "Load %s failed", filename);
    }
    *stride = width;
    return img;
}

static void copy_img(lua_State *L, int index, int id, unsigned char *canvas, int canvas_w, int canvas_h) {
    if (lua_geti(L, index, id + 1) != LUA_TTABLE) {
        luaL_error(L, "Invalid image at %d", id + 1);
    }
    int x, y, w, h, stride, px, py;
    unsigned char *img = read_img(L, &w, &h, &x, &y, &stride, &px, &py);
    int i;
    if (w + px >= canvas_w || h + py >= canvas_h) {
        stbi_image_free(img);
        luaL_error(L, "Invalid image %dx%d+%d+%d", w, h, px, py);
    }
    for (i = 0; i < h; i++) {
        memcpy(canvas + (canvas_w * (py + i) + px) * 4, img + (stride * (y + i) + x) * 4, w * 4);
    }

    stbi_image_free(img);
    lua_pop(L, 1);
}

static int limgpack(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    luaL_checktype(L, 4, LUA_TTABLE);
    unsigned char *data = (unsigned char *)lua_newuserdata(L, w * h * 4);
    memset(data, 0, w * h * 4);
    int i;
    int n = lua_rawlen(L, 4);
    for (i = 0; i < n; i++) copy_img(L, 4, i, data, w, h);
    if (!stbi_write_png(filename, w, h, 4, data, w * 4)) return luaL_error(L, "Can't write to %s", filename);
    return 0;
}

int open_tools_spritepack(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"imginfo", limginfo}, {"imgcrop", limgcrop}, {"rectpack", lrectpack}, {"imgpack", limgpack}, {NULL, NULL},
    };
    luaL_newlib(L, l);
    return 1;
}

#if 1

static int __luainspector_echo(lua_State *L) {
    neko::luainspector *m = *static_cast<neko::luainspector **>(lua_touserdata(L, lua_upvalueindex(1)));
    if (m) m->print_line(luaL_checkstring(L, 1), neko::LUACON_LOG_TYPE_MESSAGE);
    return 0;
}

static int __luainspector_gc(lua_State *L) {
    neko::luainspector *m = *static_cast<neko::luainspector **>(lua_touserdata(L, 1));
    if (m) {
        m->setL(0x0);
    }
    console_log("luainspector __gc %p", m);
    return 0;
}

namespace neko {

std::string luainspector_hints::clean_table_list(const std::string &str) {
    std::string ret;
    bool got_dot = false, got_white = false;
    std::size_t whitespace_start = 0u;
    for (std::size_t i = 0u; i < str.size(); ++i) {
        const char c = str[i] == ':' ? '.' : str[i];
        if (!got_white && c == ' ') {
            got_white = true;
            whitespace_start = i;
        }
        if (c == '.' && got_white) {
            for (std::size_t j = 0u; j < (i - whitespace_start); ++j) ret.erase(--ret.end());
        }
        if (c != ' ') got_white = false;
        if (c != ' ' || !got_dot) ret += c;
        if (c == '.') got_dot = true;
        if (c != '.' && c != ' ') got_dot = false;
    }

    const std::string specials = "()[]{}\"'+-=/*^%#~,";
    for (std::size_t i = 0u; i < specials.size(); ++i) std::replace(ret.begin(), ret.end(), specials[i], ' ');

    ret = ret.substr(ret.find_last_of(' ') + 1u);
    return ret;
}

void luainspector_hints::prepare_hints(lua_State *L, std::string str, std::string &last) {
    str = clean_table_list(str);

    std::vector<std::string> tables;
    int begin = 0;
    for (std::size_t i = 0u; i < str.size(); ++i) {
        if (str[i] == '.') {
            tables.push_back(str.substr(begin, i - begin));
            begin = i + 1;
        }
    }
    last = str.substr(begin);

    lua_pushglobaltable(L);
    for (std::size_t i = 0u; i < tables.size(); ++i) {
        if (lua_type(L, -1) != LUA_TTABLE) {
            lua_getmetatable(L, -1);
        }

        if (lua_type(L, -1) != LUA_TTABLE && !luaL_getmetafield(L, -1, "__index") && !lua_getmetatable(L, -1)) break;
        if (lua_type(L, -1) != LUA_TTABLE) break;  // no
        lua_pushlstring(L, tables[i].c_str(), tables[i].size());
        lua_gettable(L, -2);
    }
}

bool luainspector_hints::collect_hints_recurse(lua_State *L, std::vector<std::string> &possible, const std::string &last, bool usehidden, unsigned left) {
    if (left == 0u) return true;

    const bool skip_under_score = last.empty() && !usehidden;

    lua_pushnil(L);
    while (lua_next(L, -2)) {
        std::size_t keylen;
        const char *key;
        bool match = true;
        lua_pop(L, 1);
        lua_pushvalue(L, -1);  // for lua_next
        key = lua_tolstring(L, -1, &keylen);
        if (last.size() > keylen) {
            lua_pop(L, 1);
            continue;
        }
        for (std::size_t i = 0u; i < last.size(); ++i)
            if (key[i] != last[i]) match = false;
        if (match && (!skip_under_score || key[0] != '_')) possible.push_back(key);
        lua_pop(L, 1);  //
    }

    // Check whether the table itself has an index for linking elements
    if (luaL_getmetafield(L, -1, "__index")) {
        if (lua_istable(L, -1)) return collect_hints_recurse(L, possible, last, usehidden, left - 1);
        lua_pop(L, 1);  // pop
    }
    lua_pop(L, 1);  // pop table
    return true;
}

// Replace the value at the top of the stack with the __index TABLE from the metatable
bool luainspector_hints::try_replace_with_metaindex(lua_State *L) {
    if (!luaL_getmetafield(L, -1, "__index")) return false;

    if (lua_type(L, -1) != LUA_TTABLE) {
        lua_pop(L, 2);  // pop value and key
        return false;
    }

    lua_insert(L, -2);  // move table under value
    lua_pop(L, 1);      // pop value
    return true;
}

bool luainspector_hints::collect_hints(lua_State *L, std::vector<std::string> &possible, const std::string &last, bool usehidden) {
    if (lua_type(L, -1) != LUA_TTABLE && !luainspector_hints::try_replace_with_metaindex(L)) return false;
    // table so just collect on it
    return collect_hints_recurse(L, possible, last, usehidden, 10u);
}

std::string luainspector_hints::common_prefix(const std::vector<std::string> &possible) {
    std::string ret;
    std::size_t maxindex = 1000000000u;
    for (std::size_t i = 0u; i < possible.size(); ++i) maxindex = std::min(maxindex, possible[i].size());
    for (std::size_t checking = 0u; checking < maxindex; ++checking) {
        const char c = possible[0u][checking];
        for (std::size_t i = 1u; i < possible.size(); ++i)
            if (c != possible[i][checking]) {
                checking = maxindex;
                break;
            }
        if (checking != maxindex) ret += c;
    }
    return ret;
}

}  // namespace neko

const char *const kMetaname = "__neko_lua_inspector_meta";

static void *__neko_lua_inspector_lightkey() {
    static char KEY;
    return &KEY;
}

static void *__neko_lua_inspector_print_func_lightkey() {
    static char KEY;
    return &KEY;
}

neko::luainspector *neko::luainspector::get_from_registry(lua_State *L) {
    lua_pushlightuserdata(L, __neko_lua_inspector_lightkey());  // # -1
    lua_gettable(L, LUA_REGISTRYINDEX);

    neko::luainspector *ret = nullptr;

    if (lua_type(L, -1) == LUA_TUSERDATA && lua_getmetatable(L, -1)) {
        // # -1 = metatable
        // # -2 = userdata
        lua_getfield(L, LUA_REGISTRYINDEX, kMetaname);  // get inspector metatable from registry
        // # -1 = metatable
        // # -2 = metatable
        // # -3 = userdata
        if (neko_lua_equal(L, -1, -2)) {                                       // determine is two metatable equal
            ret = *static_cast<neko::luainspector **>(lua_touserdata(L, -3));  // inspector userdata
        }

        lua_pop(L, 2);  // pop two
    }

    lua_pop(L, 1);  // pop inspector userdata
    return ret;
}

void neko::luainspector::print_luastack(int first, int last, luainspector_logtype logtype) {
    std::stringstream ss;
    for (int i = first; i <= last; ++i) {
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                ss << lua_tostring(L, i);
                break;
            case LUA_TSTRING:
                ss << "'" << lua_tostring(L, i) << "'";
                break;
            case LUA_TBOOLEAN:
                ss << (lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNIL:
                ss << "nil";
                break;
            default:
                ss << luaL_typename(L, i) << ": " << lua_topointer(L, i);
                break;
        }
        ss << ' ';
    }
    print_line(ss.str(), logtype);
}

bool neko::luainspector::try_eval(std::string m_buffcmd, bool addreturn) {
    if (addreturn) {
        const std::string code = "return " + m_buffcmd;
        if (LUA_OK == luaL_loadstring(L, code.c_str())) {
            return true;
        } else {
            lua_pop(L, 1);  // pop error
            return false;
        }
    } else {
        return LUA_OK == luaL_loadstring(L, m_buffcmd.c_str());
    }
}

// 避免使用非字符串调用时错误
static inline std::string adjust_error_msg(lua_State *L, int idx) {
    const int t = lua_type(L, idx);
    if (t == LUA_TSTRING) return lua_tostring(L, idx);
    return std::string("(non string error value - ") + lua_typename(L, t) + ")";
}

void neko::luainspector::setL(lua_State *L) {
    this->L = L;

    if (!L) return;

    neko::luainspector **ptr = static_cast<neko::luainspector **>(lua_newuserdata(L, sizeof(neko::luainspector *)));
    (*ptr) = this;

    luaL_newmetatable(L, kMetaname);  // table
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &__luainspector_gc);
    lua_settable(L, -3);  // table[gc]=ConsoleModel_gc
    lua_setmetatable(L, -2);

    lua_pushlightuserdata(L, __neko_lua_inspector_lightkey());
    lua_pushvalue(L, -2);
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_pushcclosure(L, &__luainspector_echo, 1);
    lua_setglobal(L, "echo");
}

std::string neko::luainspector::read_history(int change) {
    const bool was_promp = static_cast<std::size_t>(m_hindex) == m_history.size();

    m_hindex += change;
    m_hindex = std::max<int>(m_hindex, 0);
    m_hindex = std::min<int>(m_hindex, m_history.size());

    if (static_cast<std::size_t>(m_hindex) == m_history.size()) {
        return m_history[m_hindex - 1];
    } else {
        return m_history[m_hindex];
    }
}

std::string neko::luainspector::try_complete(std::string inputbuffer) {
    if (!L) {
        print_line("Lua state pointer is NULL, no completion available", LUACON_LOG_TYPE_ERROR);
        return inputbuffer;
    }

    std::vector<std::string> possible;  // possible match
    std::string last;

    const std::string lastbeg = inputbuffer;
    luainspector_hints::prepare_hints(L, lastbeg, last);
    if (!luainspector_hints::collect_hints(L, possible, last, false)) {
        lua_pushglobaltable(L);
        luainspector_hints::collect_hints(L, possible, last, false);
    }

    lua_settop(L, 0);  // Pop all

    if (possible.size() > 1u) {
        const std::string common_prefix = luainspector_hints::common_prefix(possible);
        if (common_prefix.empty() || common_prefix.size() <= last.size()) {
            std::string msg = possible[0];
            for (std::size_t i = 1u; i < possible.size(); ++i) msg += " " + possible[i];
            print_line(msg, LUACON_LOG_TYPE_NOTE);
            m_current_autocomplete_strings = possible;
        } else {
            const std::string added = common_prefix.substr(last.size());
            inputbuffer = lastbeg + added;
            m_current_autocomplete_strings.clear();
        }
    } else if (possible.size() == 1) {
        const std::string added = possible[0].substr(last.size());
        inputbuffer = lastbeg + added;
        m_current_autocomplete_strings.clear();
    }
    return inputbuffer;
}

// void neko::luainspector::set_print_eval_prettifier(lua_State* L) {
//     if (lua_gettop(L) == 0) return;

//     const int t = lua_type(L, -1);
//     if (!(t == LUA_TFUNCTION || t == LUA_TNIL)) return;

//     lua_pushlightuserdata(L, __neko_lua_inspector_print_func_lightkey());
//     lua_insert(L, -2);
//     lua_settable(L, LUA_REGISTRYINDEX);
// }

// void neko::luainspector::get_print_eval_prettifier(lua_State* L) const {
//     lua_pushlightuserdata(L, __neko_lua_inspector_print_func_lightkey());
//     lua_gettable(L, LUA_REGISTRYINDEX);
// }

// bool neko::luainspector::apply_prettifier(int index) {
//     get_print_eval_prettifier(L);
//     if (lua_type(L, -1) == LUA_TNIL) {
//         lua_pop(L, 1);
//         return false;
//     }

//     assert(lua_type(L, -1) == LUA_TFUNCTION);
//     lua_pushvalue(L, index);
//     if (LUA_OK == luax_pcall(L, 1, 1)) {
//         lua_remove(L, index);
//         lua_insert(L, index);
//         return true;
//     } else {
//         print_line(adjust_error_msg(L, -1), LUACON_LOG_TYPE_ERROR);
//         lua_pop(L, 1);
//         return false;
//     }
//     return true;
// }

#if 0

int neko::luainspector::command_line_callback_st(ImGuiInputTextCallbackData* data) noexcept {
    command_line_input_callback_UserData* user_data = (command_line_input_callback_UserData*)data->UserData;
    return reinterpret_cast<neko::luainspector*>(user_data->luainspector_ptr)->command_line_input_callback(data);
}

int neko::luainspector::command_line_input_callback(ImGuiInputTextCallbackData* data) {
    command_line_input_callback_UserData* user_data = (command_line_input_callback_UserData*)data->UserData;

    auto paste_buffer = [data](auto begin, auto end, auto buffer_shift) {
        neko::copy(begin, end, data->Buf + buffer_shift, data->Buf + data->BufSize - 1);
        data->BufTextLen = std::min(static_cast<int>(std::distance(begin, end) + buffer_shift), data->BufSize - 1);
        data->Buf[data->BufTextLen] = '\0';
        data->BufDirty = true;
        data->SelectionStart = data->SelectionEnd;
        data->CursorPos = data->BufTextLen;
    };

    const char* command_beg = nullptr;
    command_beg = neko::find_terminating_word(cmd.data(), cmd.data() + cmd.size(), [this](std::string_view sv) { return sv[0] == ' ' ? 1 : 0; });

    if (data->EventKey == ImGuiKey_Tab) {
        std::string complete = this->try_complete(cmd);
        if (!complete.empty()) {
            paste_buffer(complete.data(), complete.data() + complete.size(), command_beg - cmd.data());
            console_log("%s", complete.c_str());
        }
    }
    if (data->EventKey == ImGuiKey_UpArrow) {
        cmd2 = this->read_history(-1);
        paste_buffer(cmd2.data(), cmd2.data() + cmd2.size(), command_beg - cmd.data());
        console_log("h:%s", cmd2.c_str());
    }
    if (data->EventKey == ImGuiKey_DownArrow) {
        cmd2 = this->read_history(1);
        paste_buffer(cmd2.data(), cmd2.data() + cmd2.size(), command_beg - cmd.data());
        console_log("h:%s", cmd2.c_str());
    }

    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        std::string* str = user_data->Str;
        neko_assert(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    } else if (user_data->ChainCallback) {
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool neko::luainspector::command_line_input(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    command_line_input_callback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    cb_user_data.luainspector_ptr = this;
    return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, command_line_callback_st, &cb_user_data);
}

void neko::luainspector::show_autocomplete() noexcept {
    constexpr ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                                               ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings;

    if ((m_input_text_id == ImGui::GetActiveID() || m_should_take_focus) && (!m_current_autocomplete_strings.empty())) {

        ImGui::SetNextWindowBgAlpha(0.9f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::SetNextWindowFocus();

        ImVec2 auto_complete_pos = ImGui::GetItemRectMin();

        auto_complete_pos.y = ImGui::GetItemRectMax().y;

        ImVec2 auto_complete_max_size = ImGui::GetItemRectSize();
        auto_complete_max_size.y = -1.f;
        ImGui::SetNextWindowPos(auto_complete_pos);
        ImGui::SetNextWindowSizeConstraints({0.f, 0.f}, auto_complete_max_size);
        if (ImGui::Begin("##terminal:auto_complete", nullptr, overlay_flags)) {
            ImGui::SetActiveID(m_input_text_id, ImGui::GetCurrentWindow());

            auto print_separator = [this]() {
                ImGui::SameLine(0.f, 0.f);
                int pop = try_push_style(ImGuiCol_Text, ImVec4{0.000f, 0.000f, 0.000f, 0.392f});
                ImGui::TextUnformatted(m_autocomlete_separator.data(), m_autocomlete_separator.data() + m_autocomlete_separator.size());
                ImGui::PopStyleColor(pop);
                ImGui::SameLine(0.f, 0.f);
            };

            int max_displayable_sv = 0;
            float separator_length = ImGui::CalcTextSize(m_autocomlete_separator.data(), m_autocomlete_separator.data() + m_autocomlete_separator.size()).x;
            float total_text_length = ImGui::CalcTextSize("...").x;

            std::vector<std::string_view> autocomplete_text;

            if (m_current_autocomplete_strings.empty()) {
            } else {
                autocomplete_text.reserve(m_current_autocomplete_strings.size());
                for (const std::string& str : m_current_autocomplete_strings) {
                    autocomplete_text.emplace_back(str);
                }
            }

            for (const std::string_view& sv : autocomplete_text) {
                float t_len = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x + separator_length;
                if (t_len + total_text_length < auto_complete_max_size.x) {
                    total_text_length += t_len;
                    ++max_displayable_sv;
                } else {
                    break;
                }
            }

            std::string_view last;
            int pop_count = 0;

            if (max_displayable_sv != 0) {
                const std::string_view& first = autocomplete_text[0];
                pop_count += try_push_style(ImGuiCol_Text, ImVec4{1.000f, 1.000f, 1.000f, 1.000f});
                ImGui::TextUnformatted(first.data(), first.data() + first.size());
                pop_count += try_push_style(ImGuiCol_Text, ImVec4{0.500f, 0.450f, 0.450f, 1.000f});
                for (int i = 1; i < max_displayable_sv; ++i) {
                    const std::string_view vs = autocomplete_text[i];
                    print_separator();
                    ImGui::TextUnformatted(vs.data(), vs.data() + vs.size());
                }
                ImGui::PopStyleColor(pop_count);
                if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) last = autocomplete_text[max_displayable_sv];
            }

            pop_count = 0;
            if (max_displayable_sv < static_cast<long>(autocomplete_text.size())) {

                if (max_displayable_sv == 0) {
                    last = autocomplete_text.front();
                    pop_count += try_push_style(ImGuiCol_Text, ImVec4{1.000f, 1.000f, 1.000f, 1.000f});
                    total_text_length -= separator_length;
                } else {
                    pop_count += try_push_style(ImGuiCol_Text, ImVec4{0.500f, 0.450f, 0.450f, 1.000f});
                    print_separator();
                }

                std::vector<char> buf;
                buf.resize(last.size() + 4);
                std::copy(last.begin(), last.end(), buf.begin());
                std::fill(buf.begin() + last.size(), buf.end(), '.');
                auto size = static_cast<unsigned>(last.size() + 3);
                while (size >= 4 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
                    buf[size - 4] = '.';
                    --size;
                }
                while (size != 0 && total_text_length + ImGui::CalcTextSize(buf.data(), buf.data() + size).x >= auto_complete_max_size.x) {
                    --size;
                }
                ImGui::TextUnformatted(buf.data(), buf.data() + size);
                ImGui::PopStyleColor(pop_count);
            }

            if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
                cmd.clear();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}


void neko::luainspector::console_draw(bool& textbox_react) noexcept {

    // ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    const float TEXT_BASE_HIGHT = 6.f;

    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 80 - TEXT_BASE_HIGHT * 2);
    if (ImGui::BeginChild("##console_log", size)) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

        neko_assert(&messageLog);

        for (auto& a : messageLog) {
            ImVec4 colour;
            switch (a.second) {
                case LUACON_LOG_TYPE_WARNING:
                    colour = {1.0f, 1.0f, 0.0f, 1.0f};
                    break;
                case LUACON_LOG_TYPE_ERROR:
                    colour = {1.0f, 0.0f, 0.0f, 1.0f};
                    break;
                case LUACON_LOG_TYPE_NOTE:
                    colour = {0.13f, 0.44f, 0.61f, 1.0f};
                    break;
                case LUACON_LOG_TYPE_SUCCESS:
                    colour = {0.0f, 1.0f, 0.0f, 1.0f};
                    break;
                case LUACON_LOG_TYPE_MESSAGE:
                    colour = {1.0f, 1.0f, 1.0f, 1.0f};
                    break;
            }
            ImGui::TextColored(colour, "%s", a.first.c_str());
        }
        ImGui::PopTextWrapPos();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor();

    if (m_should_take_focus) {
        ImGui::SetKeyboardFocusHere();
        m_should_take_focus = false;
    }

    ImGui::PushItemWidth(-1.f);
    if (this->command_line_input("##Input", &cmd, ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory) || ImGui::IsItemActive()) textbox_react = true;
    ImGui::PopItemWidth();

    if (m_input_text_id == 0u) {
        m_input_text_id = ImGui::GetItemID();
    }

    auto call_command = [&]() {
        if (!m_history.empty() && m_history.back() != cmd) {
            m_history.push_back(cmd);
            m_history.erase(m_history.begin());
        }

        m_hindex = m_history.size();

        if (L) {
            const int oldtop = lua_gettop(L);
            bool evalok = try_eval(cmd, true) || try_eval(cmd, false);

            if (evalok && LUA_OK == luax_pcall(L, 0, LUA_MULTRET)) {
                if (oldtop != lua_gettop(L)) print_luastack(oldtop + 1, lua_gettop(L), LUACON_LOG_TYPE_MESSAGE);

                lua_settop(L, oldtop);
            } else {
                const std::string err = adjust_error_msg(L, -1);
                if (evalok || !neko::incomplete_chunk_error(err.c_str(), err.length())) {
                    print_line(err, LUACON_LOG_TYPE_ERROR);
                }
                lua_pop(L, 1);
            }
        } else {
            print_line("Lua state pointer is NULL, commands have no effect", LUACON_LOG_TYPE_ERROR);
        }
        cmd.clear();
    };

    if (m_previously_active_id == m_input_text_id && ImGui::GetActiveID() != m_input_text_id) {
        if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
            call_command();
            m_should_take_focus = true;
        } else {
            m_should_take_focus = false;
        }
    }
    m_previously_active_id = ImGui::GetActiveID();

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

    show_autocomplete();
}

#endif

void neko::luainspector::print_line(const std::string &msg, luainspector_logtype type) noexcept { messageLog.emplace_back(msg, type); }

// void neko::luainspector::inspect_table(lua_State* L, inspect_table_config& cfg) {}

#if 1

void neko::luainspector::inspect_table(lua_State *L, inspect_table_config &cfg) {
    auto is_multiline = [](const_str str) -> bool {
        while (*str != '\0') {
            if (*str == '\n') return true;
            str++;
        }
        return false;
    };

    ui_context_t *ui = g_app->ui;

    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {

        if (lua_isnil(L, -2)) {
            lua_pop(L, 2);
            break;
        }

        bool edited = false;
        auto name = neko_lua_to<const_str>(L, -2);
        if (cfg.search_str != 0 && !strstr(name, cfg.search_str)) {
            lua_pop(L, 1);
            continue;
        }

        int type = lua_type(L, -1);

        if (cfg.is_non_function && type == LUA_TFUNCTION) {
            goto skip;
        }

        // ImGui::TableNextRow();
        // ImGui::TableNextColumn();

        // static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

        ui_layout_row(ui, 3, ui_widths(200, 200, 200), 0);

        if (type == LUA_TSTRING) {

            ui_text(ui, name);
            // ImGui::TableNextColumn();
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            // ImGui::TableNextColumn();
            // ImGui::Text("%p", lua_topointer(L, -1));

            const_str str_mem = neko_lua_to<const_str>(L, -1);
            size_t buffer_size = 256;
            for (; buffer_size < strlen(str_mem);) buffer_size += 128;
            std::string v(neko_lua_to<const_str>(L, -1), buffer_size);
            if (!is_multiline(str_mem) && strlen(str_mem) < 32) {
                Color256 col = color256(40, 220, 55, 255);
                ui_text_colored(ui, str_mem, col);
            } else {
                Color256 col = color256(40, 220, 55, 255);
                ui_text_colored(ui, "\"...\"", col);
            }

            // if (open) {
            // ImGui::InputTextMultiline("value", const_cast<char*>(v.c_str()), buffer_size);
            // if (ImGui::IsKeyDown(ImGuiKey_Enter) && v != neko_lua_to<const_str>(L, -1)) {
            //     edited = true;
            //     lua_pop(L, 1);                 // # -1 pop value
            //     lua_pushstring(L, v.c_str());  // # -1 push new value
            //     lua_setfield(L, -3, name);     // -3 table
            //     console_log("改 %s = %s", name, v.c_str());
            // }
            // ImGui::TreePop();
            // }

        } else if (type == LUA_TNUMBER) {

            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            ui_labelf("%f", neko_lua_to<f64>(L, -1));

            if (open) {
                f64 v = neko_lua_to<f64>(L, -1);
                ui_labelf("lua_v: %f", v);
                // ImGui::InputDouble("value", &v);
                // if (ImGui::IsKeyDown(ImGuiKey_Enter) && v != neko_lua_to<f64>(L, -1)) {
                //     edited = true;
                //     lua_pop(L, 1);              // # -1 pop value
                //     lua_pushnumber(L, v);       // # -1 push new value
                //     lua_setfield(L, -3, name);  // -3 table
                // }
                // ImGui::TreePop();
            }

        } else if (type == LUA_TFUNCTION) {
            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            Color256 col = color256(110, 180, 255, 255);
            ui_textf_colored(ui, col, "%p", lua_topointer(L, -1));

        } else if (type == LUA_TTABLE) {
            // ImGui::Text("lua_v: %p", lua_topointer(L, -1));
            // ImGui::Indent();
            // inspect_table(L);
            // ImGui::Unindent();

            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            // ImGui::TextDisabled("--");
            ui_labelf("--");
            if (open) {
                inspect_table(L, cfg);
                // ImGui::TreePop();
            }

        } else if (type == LUA_TUSERDATA) {

            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            Color256 col = color256(75, 230, 250, 255);
            ui_textf_colored(ui, col, "%p", lua_topointer(L, -1));

            if (open) {

                ui_labelf("lua_v: %p", lua_topointer(L, -1));

                const_str metafields[] = {"__name", "__index"};

                if (lua_getmetatable(L, -1)) {
                    for (int i = 0; i < NEKO_ARR_SIZE(metafields); i++) {
                        lua_pushstring(L, metafields[i]);
                        lua_gettable(L, -2);
                        if (lua_isstring(L, -1)) {
                            const char *name = lua_tostring(L, -1);
                            ui_labelf("%s: %s", metafields[i], name);
                        } else if (lua_isnumber(L, -1)) {
                            f64 name = lua_tonumber(L, -1);
                            ui_labelf("%s: %lf", metafields[i], name);
                        } else {
                            ui_labelf("%s field not exist", metafields[i]);
                        }
                        // pop value and table
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);
                } else {
                    Color256 col = color256(240, 0, 0, 255);
                    ui_textf_colored(ui, col, "Unknown Metatable");
                }
            }
        } else if (type == LUA_TBOOLEAN) {
            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            Color256 col = color256(220, 160, 40, 255);
            ui_textf_colored(ui, col, "%s", NEKO_BOOL_STR(lua_toboolean(L, -1)));
        } else if (type == LUA_TCDATA) {
            bool open = false;
            ui_text(ui, name);
            ui_labelf("%s", lua_typename(L, lua_type(L, -1)));
            Color256 col = color256(240, 0, 0, 255);
            ui_textf_colored(ui, col, "Unknown Metatable");
        } else {
            bool open = false;
            ui_text(ui, name);
            ui_labelf("Unknown %d", type);
            ui_labelf("Unknown");
        }

    skip:
        if (edited) {
        } else {
            lua_pop(L, 1);
        }
    }
}

extern Assets g_assets;

int neko::luainspector::luainspector_init(lua_State *L) {

    void *model_mem = lua_newuserdata(L, sizeof(neko::luainspector));

    neko::luainspector *inspector = new (model_mem) neko::luainspector();

    inspector->setL(L);
    inspector->m_history.resize(8);

    return 1;
}

int neko::luainspector::luainspector_get(lua_State *L) {
    neko::luainspector *inspector = neko::luainspector::get_from_registry(L);
    lua_pushlightuserdata(L, inspector);
    return 1;
}

bool neko::luainspector::visible = false;

int neko::luainspector::luainspector_draw(lua_State *L) {
    neko::luainspector *model = (neko::luainspector *)lua_touserdata(L, 1);

    if (!visible) {
        return 0;
    }

    ui_context_t *ui = g_app->ui;

    // ui_dock_ex(&g_app->ui, "Style_Editor", "Demo_Window", UI_SPLIT_TAB, 0.5f);

    const vec2 ss_ws = neko_v2(500.f, 300.f);
    if (ui_begin_window_ex(g_app->ui, "检查器", neko_rect((g_app->cfg.width - ss_ws.x) * 0.5f, (g_app->cfg.height - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y), UI_OPT_NOCLOSE)) {

        if (ui_header(ui, "Console")) {
            // bool textbox_react;
            // model->console_draw(textbox_react);
            // ImGui::EndTabItem();
        }
        if (ui_header(ui, "Registry")) {
            lua_pushglobaltable(L);  // _G
            static char search_text[256] = "";
            static inspect_table_config config;
            config.search_str = search_text;

            // ImGui::InputTextWithHint("Search", "Search...", search_text, IM_ARRAYSIZE(search_text));

            ui_checkbox(ui, "Non-Function", (i32 *)&config.is_non_function);

            ui_labelf("Registry contents:");

            const float TEXT_BASE_WIDTH = 22.f;
            const float TEXT_BASE_HIGHT = 22.f;

            // ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 100 - TEXT_BASE_HIGHT * 4);
            // if (ImGui::BeginChild("##lua_registry", size)) {
            // ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

            // static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

            // Cache the current container
            ui_container_t *cnt = ui_get_current_container(g_app->ui);

            ui_layout_row(g_app->ui, 3, ui_widths(200, 0), 0);

            ui_labelf("Name");
            ui_labelf("Type");
            ui_labelf("Value");

            inspect_table(L, config);

            ui_layout_row(g_app->ui, 1, ui_widths(0), 0);

            // if (ImGui::BeginTable("lua_inspector_reg", 3, flags)) {
            //     ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
            //     ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
            //     ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 32.0f);
            //     ImGui::TableHeadersRow();

            //     inspect_table(L, config);

            //     ImGui::EndTable();
            // }

            // ImGui::PopTextWrapPos();

            // if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 50) ImGui::SetScrollY(ImGui::GetScrollMaxY());
            // }
            // ImGui::EndChild();

            lua_pop(L, 1);  // pop _G
            // ImGui::EndTabItem();
        }

        // if (ImGui::BeginTabItem("Property")) {

        //     auto& properties = model->m_property_map;

        //     ImGui::Text("Property count: %lld\nType count: %lld", properties.size(), model->m_type_render_functions.size());

        //     for (auto& property_it : properties) {

        //         auto prop_render_func_it = model->m_type_render_functions.find(property_it.param_type);
        //         neko_assert(prop_render_func_it != model->m_type_render_functions.end());  // unsupported type, render function not found
        //         const_str label = property_it.label.c_str();
        //         void* value = property_it.param;
        //         if (ImGui::CollapsingHeader(std::format("{0} | {1}", label, property_it.param_type.name()).c_str())) {
        //             ImGui::Indent();
        //             prop_render_func_it->second(label, value);
        //             ImGui::Unindent();
        //         }
        //     }

        //     ImGui::EndTabItem();
        // }

        if (ui_header(ui, "Info")) {
            lua_Integer kb = lua_gc(L, LUA_GCCOUNT, 0);
            lua_Integer bytes = lua_gc(L, LUA_GCCOUNTB, 0);

            ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
            ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

            if (ui_button(ui, "GC")) lua_gc(L, LUA_GCCOLLECT, 0);

            if (ui_button(ui, "Memory")) {
                const size_t *alive, *total, *blocks;
                unsigned step, n = luaalloc_getstats(ENGINE_LS().LA, &alive, &total, &blocks, &step);
                if (n) {
                    for (unsigned i = 0, a = 1, b = step; i < n - 1; ++i, a = b + 1, b += step)
                        printf("%zu blocks of %u..%u bytes: %zu allocations alive, %zu done all-time\n", blocks[i], a, b, alive[i], total[i]);
                    printf("large allocations: %zu alive, %zu done all-time\n", alive[n - 1], total[n - 1]);
                }
            }

            ui_layout_row(g_app->ui, 3, ui_widths(150, 200, 200), 0);
            for (auto kv : g_assets.table) {
                AssetKind kind = kv.value->kind;

                // 这应该很慢
                neko_luabind_push(L, AssetKind, &kind);
                const_str kind_name = lua_tostring(L, -1);
                lua_pop(L, 1);

                ui_labelf("%s", kind_name);
                ui_labelf("%llu", kv.value->hash);
                ui_labelf("%s", kv.value->name.cstr());
            }
            ui_layout_row(g_app->ui, 1, ui_widths(0), 0);
        }

        if (ui_header(ui, "Shader")) {
            for (auto kv : g_assets.table) {
                if (kv.value->kind == AssetKind_Shader) {
                    inspect_shader(kv.value->name.cstr(), kv.value->shader.id);
                }
            }
        }

        ui_end_window(g_app->ui);
    }

    return 0;
}

void inspector_set_visible(bool visible) { neko::luainspector::visible = visible; }
bool inspector_get_visible() { return neko::luainspector::visible; }

#endif

#endif

#define LINE_LEN 128  // including newline, null char
#define NUM_LINES 20

// circular buffer of lines, 'top' is current top line
static char lines[NUM_LINES][LINE_LEN] = {{0}};
static int top = 0;

// text that displays console contents
static NativeEntity text;

void neko_console_printf(neko_console_t *console, const char *fmt, ...);

static void _update_text() {
    unsigned int i;
    char *buf, *c, *r;

    // entity exists?
    if (native_entity_eq(text, entity_nil)) return;

    // accumulate non-empty lines and set text string

    buf = (char *)mem_alloc(NUM_LINES * LINE_LEN);
    for (i = 1, c = buf; i <= NUM_LINES; ++i)
        for (r = lines[(top + i) % NUM_LINES]; *r; ++r) *c++ = *r;
    *c = '\0';

    gui_text_set_str(text, buf);

    mem_free(buf);
}

void console_set_entity(NativeEntity ent) {
    text = ent;
    gui_text_add(text);
    _update_text();
}
NativeEntity console_get_entity() { return text; }

void console_set_visible(bool visible) {
    if (!native_entity_eq(text, entity_nil)) gui_set_visible(transform_get_parent(text), visible);
}
bool console_get_visible() {
    if (!native_entity_eq(text, entity_nil)) return gui_get_visible(transform_get_parent(text));
    return false;
}

// write a string to console with wrapping
static void _write(const char *s) {
    static unsigned int curs = 0;  // cursor position
    static const char wrap_prefix[] = {26, ' ', '\0'};
    unsigned int width, tabstop;
    char c;
    bool wrap;

    // wrap at window width, but max out at alloc'd size
    width = game_get_window_size().x / 10;
    if (width > LINE_LEN - 2) width = LINE_LEN - 2;

    while (*s) {
        c = *s;

        // tab? jump to tabstop
        if (c == '\t') {
            c = ' ';
            tabstop = 4 * ((curs + 4) / 4);
            if (tabstop >= width) {
                // wrap at tab?
                curs = width;
                ++s;
            } else
                for (; curs < tabstop - 1 && curs < width; ++curs) lines[top][curs] = c;
        }

        // write char
        wrap = curs >= width && c != '\n';
        c = wrap ? '\n' : c;
        lines[top][curs++] = c;

        // if newline, close this line and go to next
        if (c == '\n') {
            lines[top][curs] = '\0';

            top = (top + 1) % NUM_LINES;
            curs = 0;
        }

        // add a prefix to wrapped lines
        if (wrap)
            _write(wrap_prefix);
        else
            ++s;
    }

    // close this line
    lines[top][curs] = '\0';

    _update_text();
}

// write a string to both stdout and the console
static void _print(const char *s) {
    // copy to stdout
    printf("%s", s);
    fflush(stdout);

    // write it
    _write(s);
}

void console_puts(const char *s) {
    _print(s);
    _print("\n");
}

void console_printf(const char *fmt, ...) {
    va_list ap1, ap2;
    unsigned int n;
    char *s;

    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    // how much space do we need?
    n = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    // allocate, sprintf, print
    s = (char *)mem_alloc(n + 1);
    vsprintf(s, fmt, ap1);
    va_end(ap1);

    _print(s);

    neko_console_printf(&g_console, s);

    mem_free(s);
}

void console_init() {
    PROFILE_FUNC();

    text = entity_nil;
}

void console_fini() {}

void neko_console(neko_console_t *console, ui_context_t *ctx, rect_t *r, const ui_selector_desc_t *desc) {
    rect_t screen = *r;
    if (console->open)
        console->y += (screen.h * console->size - console->y) * console->open_speed;
    else if (!console->open && console->y >= 1.0f)
        console->y += (0 - console->y) * console->close_speed;
    else
        return;

    if (ui_begin_window(ctx, "Console", neko_rect(g_app->cfg.width - 210, 80, 200, 200))) {
        ui_layout_row(ctx, 1, ui_widths(-1), 0);
        ui_text(ctx, console->tb);
        ui_end_window(ctx);
    }

    const f32 sz = NEKO_MIN(console->y, 26);
    if (ui_begin_window_ex(ctx, "neko_console_content", neko_rect(screen.x, screen.y, screen.w, console->y - sz), UI_OPT_NOTITLE | UI_OPT_NORESIZE | UI_OPT_HOLDFOCUS)) {
        ui_layout_row(ctx, 1, ui_widths(-1), 0);
        ui_text(ctx, console->tb);
        // neko_imgui_draw_text(console->tb, NEKO_COLOR_WHITE, 10.f, 10.f, true, NEKO_COLOR_BLACK);
        if (console->autoscroll) ui_get_current_container(ctx)->scroll.y = sizeof(console->tb) * 7 + 100;
        ui_container_t *ctn = ui_get_current_container(ctx);
        ui_bring_to_front(ctx, ctn);
        ui_end_window(ctx);
    }

    if (ui_begin_window_ex(ctx, "neko_console_input", neko_rect(screen.x, screen.y + console->y - sz, screen.w, sz), UI_OPT_NOTITLE | UI_OPT_NORESIZE | UI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        ui_layout_row(ctx, 3, ui_widths(14, len * 7 + 2, 10), 0);
        ui_text(ctx, "$>");
        ui_text(ctx, console->cb[0]);

        i32 n;

        if (!console->open || !console->last_open_state) {
            goto console_input_handling_done;
        }

        // 处理文本输入
        n = NEKO_MIN(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (input_key_down(KC_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= NEKO_ARR_SIZE(console->cb)) {
                console->current_cb_idx = NEKO_ARR_SIZE(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (input_key_down(KC_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (input_key_down(KC_ENTER)) {
            console->current_cb_idx = 0;
            neko_console_printf(console, "$ %s\n", console->cb[0]);

            memmove((uint8_t *)console->cb + sizeof(*console->cb), (uint8_t *)console->cb, sizeof(console->cb) - sizeof(*console->cb));

            if (console->cb[0][0] && console->commands) {
                char *tmp = console->cb[0];
                int argc = 1;
                while ((tmp = strchr(tmp, ' '))) {
                    argc++;
                    tmp++;
                }

                tmp = console->cb[0];
                char *last_pos = console->cb[0];
                char **argv = (char **)mem_alloc(argc * sizeof(char *));
                int i = 0;
                while ((tmp = strchr(tmp, ' '))) {
                    *tmp = 0;
                    argv[i++] = last_pos;
                    last_pos = ++tmp;
                }
                argv[argc - 1] = last_pos;

                for (int i = 0; i < console->commands_len; i++) {
                    if (console->commands[i].name && console->commands[i].func && strcmp(argv[0], console->commands[i].name) == 0) {
                        console->commands[i].func(argc, argv);
                        goto console_command_found;
                    }
                }
                neko_console_printf(console, "[neko_console]: unrecognized command '%s'\n", argv[0]);
            console_command_found:
                console->cb[0][0] = '\0';
                mem_free(argv);
            }
        } else if (input_key_down(KC_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0);
            console->cb[0][len] = '\0';
        } else if (n > 0 && !input_key_down(KC_GRAVE_ACCENT)) {
            console->current_cb_idx = 0;
            if (len + n + 1 < sizeof(*console->cb)) {
                memcpy(console->cb[0] + len, ctx->input_text, n);
                len += n;
                console->cb[0][len] = '\0';
            }
        }

    console_input_handling_done:

        // 闪烁光标
        // ui_get_layout(ctx)->body.x += len * 7 - 5;
        if ((int)(timing_get_elapsed() / 666.0f) & 1) ui_text(ctx, "|");

        ui_container_t *ctn = ui_get_current_container(ctx);
        ui_bring_to_front(ctx, ctn);

        ui_end_window(ctx);
    }

    console->last_open_state = console->open;
}

void neko_console_printf(neko_console_t *console, const char *fmt, ...) {
    char tmp[512] = {0};
    va_list args;

    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    int n = sizeof(console->tb) - strlen(console->tb) - 1;
    int resize = strlen(tmp) - n;
    if (resize > 0) {
        memmove(console->tb, console->tb + resize, sizeof(console->tb) - resize);
        n = sizeof(console->tb) - strlen(console->tb) - 1;
    }
    strncat(console->tb, tmp, n);
}

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
// static void crash(int argc, char **argv);
void summon(int argc, char **argv);
void exec(int argc, char **argv);
void sz(int argc, char **argv);

neko_console_command_t commands[] = {{
                                             .func = echo,
                                             .name = "echo",
                                             .desc = "repeat what was entered",
                                     },
                                     {
                                             .func = spam,
                                             .name = "spam",
                                             .desc = "send the word arg1, arg2 amount of times",
                                     },
                                     {
                                             .func = help,
                                             .name = "help",
                                             .desc = "sends a list of commands",
                                     },
                                     {
                                             .func = toggle_window,
                                             .name = "window",
                                             .desc = "toggles gui window",
                                     },
                                     {
                                             .func = toggle_embedded,
                                             .name = "embed",
                                             .desc = "places the console inside the window",
                                     },
                                     {
                                             .func = summon,
                                             .name = "summon",
                                             .desc = "summons a gui window",
                                     },
                                     {
                                             .func = sz,
                                             .name = "sz",
                                             .desc = "change console size",
                                     },
                                     //  {
                                     //          .func = crash,
                                     //          .name = "crash",
                                     //          .desc = "test crashhhhhhhhh.....",
                                     //  },
                                     {
                                             .func = exec,
                                             .name = "exec",
                                             .desc = "run nekoscript",
                                     }};

neko_console_t g_console = {
        .tb = "",
        .cb = {},
        .size = 0.4,
        .open_speed = 0.2,
        .close_speed = 0.3,
        .autoscroll = true,
        .commands = commands,
        .commands_len = NEKO_ARR_SIZE(commands),
};

void sz(int argc, char **argv) {
    if (argc != 2) {
        neko_console_printf(&g_console, "[sz]: needs 1 argument!\n");
        return;
    }
    f32 sz = atof(argv[1]);
    if (sz > 1 || sz < 0) {
        neko_console_printf(&g_console, "[sz]: number needs to be between (0, 1)");
        return;
    }
    g_console.size = sz;

    neko_console_printf(&g_console, "console size is now %f\n", sz);
}

void toggle_window(int argc, char **argv) {
    if (window && embeded)
        neko_console_printf(&g_console, "Unable to turn off window, console is embeded!\n");
    else
        neko_console_printf(&g_console, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char **argv) {
    if (!window && !embeded)
        neko_console_printf(&g_console, "Unable to embed into window, open window first!\n");
    else
        neko_console_printf(&g_console, "console embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char **argv) {
    neko_console_printf(&g_console, "A summoner has cast his spell! A window has appeared!!!!\n");
    summons++;
}

// void crash(int argc, char **argv) {
//     // const_str trace_info = neko_os_stacktrace();
//     // neko_os_msgbox(std::format("Crash...\n{0}", trace_info).c_str());
// }

void spam(int argc, char **argv) {
    int count;
    if (argc != 3) goto spam_invalid_command;
    count = atoi(argv[2]);
    if (!count) goto spam_invalid_command;
    while (count--) neko_console_printf(&g_console, "%s\n", argv[1]);
    return;
spam_invalid_command:
    neko_console_printf(&g_console, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) neko_console_printf(&g_console, "%s ", argv[i]);
    neko_console_printf(&g_console, "\n");
}

void exec(int argc, char **argv) {
    if (argc != 2) return;
    // neko_vm_interpreter(argv[1]);
}

void help(int argc, char **argv) {
    for (int i = 0; i < NEKO_ARR_SIZE(commands); i++) {
        if (commands[i].name) neko_console_printf(&g_console, "* Command: %s\n", commands[i].name);
        if (commands[i].desc) neko_console_printf(&g_console, "- desc: %s\n", commands[i].desc);
    }
}