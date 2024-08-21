#include "engine/edit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <chrono>
#include <filesystem>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/entity.h"
#include "engine/game.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/prelude.h"
#include "engine/transform.h"

static bool enabled;

// editability data
static EntityPool *pool_uneditable; /* Entites are in this pool
                                       iff. not editable */

void edit_set_enabled(bool e) { enabled = e; }
bool edit_get_enabled() { return enabled; }

void edit_set_editable(Entity ent, bool editable) {
    if (editable)
        entitypool_remove(pool_uneditable, ent);
    else
        entitypool_add(pool_uneditable, ent);
}
bool edit_get_editable(Entity ent) { return !entitypool_get(pool_uneditable, ent); }

// --- bboxes --------------------------------------------------------------

// bbox pool
typedef struct BBoxPoolElem BBoxPoolElem;
struct BBoxPoolElem {
    EntityPoolElem pool_elem;

    LuaMat3 wmat;
    BBox bbox;
    Scalar selected;  // > 0.5 if and only if selected
};
static EntityPool *pool_bbox;

void edit_bboxes_update(Entity ent, BBox bbox) {
    BBoxPoolElem *elem;

    // editable?
    if (!edit_get_editable(ent)) return;

    elem = (BBoxPoolElem *)entitypool_get(pool_bbox, ent);

    // merge if already exists, else set
    if (elem)
        elem->bbox = bbox_merge(elem->bbox, bbox);
    else {
        elem = (BBoxPoolElem *)entitypool_add(pool_bbox, ent);
        elem->bbox = bbox;
    }
}

bool edit_bboxes_has(Entity ent) { return entitypool_get(pool_bbox, ent) != NULL; }
BBox edit_bboxes_get(Entity ent) {
    BBoxPoolElem *elem = (BBoxPoolElem *)entitypool_get(pool_bbox, ent);
    error_assert(elem);
    return elem->bbox;
}

unsigned int edit_bboxes_get_num() { return entitypool_size(pool_bbox); }

Entity edit_bboxes_get_nth_ent(unsigned int n) {
    BBoxPoolElem *elem;
    // struct EntityBBoxPair bbpair {};

    error_assert(n < entitypool_size(pool_bbox));
    elem = (BBoxPoolElem *)entitypool_nth(pool_bbox, n);

    // bbpair.ent = elem->pool_elem.ent;
    // bbpair.bbox = elem->bbox;
    return elem->pool_elem.ent;
}

BBox edit_bboxes_get_nth_bbox(unsigned int n) {
    BBoxPoolElem *elem;
    // struct EntityBBoxPair bbpair {};

    error_assert(n < entitypool_size(pool_bbox));
    elem = (BBoxPoolElem *)entitypool_nth(pool_bbox, n);

    // bbpair.ent = elem->pool_elem.ent;
    // bbpair.bbox = elem->bbox;
    return elem->bbox;
}

void edit_bboxes_set_selected(Entity ent, bool selected) {
    BBoxPoolElem *elem = (BBoxPoolElem *)entitypool_get(pool_bbox, ent);
    error_assert(elem);
    elem->selected = selected;
}

static GLuint bboxes_program;
static GLuint bboxes_vao;
static GLuint bboxes_vbo;

static void _bboxes_init() {
    pool_bbox = entitypool_new(BBoxPoolElem);

    // create shader program, load atlas, bind parameters
    bboxes_program = gfx_create_program("bboxes_program", "shader/bbox.vert", "shader/bbox.geom", "shader/bbox.frag");
    glUseProgram(bboxes_program);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &bboxes_vao);
    glBindVertexArray(bboxes_vao);
    glGenBuffers(1, &bboxes_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 3, "wmat1", BBoxPoolElem, wmat.m[0]);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 3, "wmat2", BBoxPoolElem, wmat.m[1]);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 3, "wmat3", BBoxPoolElem, wmat.m[2]);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 2, "bbmin", BBoxPoolElem, bbox.min);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 2, "bbmax", BBoxPoolElem, bbox.max);
    gfx_bind_vertex_attrib(bboxes_program, GL_FLOAT, 1, "selected", BBoxPoolElem, selected);
}
static void _bboxes_fini() {
    // clean up GL stuff
    glDeleteProgram(bboxes_program);
    glDeleteBuffers(1, &bboxes_vbo);
    glDeleteVertexArrays(1, &bboxes_vao);

    entitypool_free(pool_bbox);
}

static void _bboxes_update_all() {
    Entity ent;
    BBoxPoolElem *elem;
    static BBox defaultbb = {{-0.25, -0.25}, {0.25, 0.25}};

    if (!enabled) return;

    entitypool_foreach(elem, pool_bbox) {
        ent = elem->pool_elem.ent;
        if (!transform_has(ent)) continue;

        // update world matrix
        elem->wmat = transform_get_world_matrix(ent);

        // if no bbox, make default
        if (elem->bbox.max.x - elem->bbox.min.x <= SCALAR_EPSILON || elem->bbox.max.y - elem->bbox.min.y <= SCALAR_EPSILON) elem->bbox = defaultbb;
    }
}

// needs bbox shader program to be bound
static void _bboxes_draw_all() {
    LuaVec2 win;
    unsigned int nbboxes;

    glUseProgram(bboxes_program);
    glUniformMatrix3fv(glGetUniformLocation(bboxes_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());
    win = game_get_window_size();
    glUniform1f(glGetUniformLocation(bboxes_program, "aspect"), win.x / win.y);
    glUniform1f(glGetUniformLocation(bboxes_program, "is_grid"), 0);

    glBindVertexArray(bboxes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    nbboxes = entitypool_size(pool_bbox);
    glBufferData(GL_ARRAY_BUFFER, nbboxes * sizeof(BBoxPoolElem), entitypool_begin(pool_bbox), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nbboxes);
}

// --- grid ----------------------------------------------------------------

static LuaVec2 grid_size = {1.0, 1.0};

void edit_set_grid_size(LuaVec2 size) {
    if (size.x < 0.0) size.x = 0.0;
    if (size.y < 0.0) size.y = 0.0;

    grid_size = size;
}
LuaVec2 edit_get_grid_size() { return grid_size; }

static CArray *grid_cells;  // bboxes used for drawing grid

static void _grid_create_cells() {
    BBoxPoolElem *cell;
    BBox cbox, cellbox;
    Entity camera;
    LuaVec2 cur, csize;

    // find camera bounds in world space
    camera = camera_get_current_camera();
    if (entity_eq(camera, entity_nil))
        cbox = bbox(luavec2(-1, -1), luavec2(1, 1));
    else
        cbox = bbox_transform(transform_get_world_matrix(camera), bbox(luavec2(-1, -1), luavec2(1, 1)));
    csize = luavec2(cbox.max.x - cbox.min.x, cbox.max.y - cbox.min.y);

    // create grid cell bbox
    cellbox.min = vec2_zero;
    if (grid_size.x > 0)
        cellbox.max.x = grid_size.x;
    else
        cellbox.max.x = csize.x + 1;
    if (grid_size.y > 0)
        cellbox.max.y = grid_size.y;
    else
        cellbox.max.y = csize.y + 1;

    // make it bigger if it's too small
    while (csize.x / cellbox.max.x > 70 || csize.y / cellbox.max.y > 70) cellbox.max = vec2_scalar_mul(cellbox.max, 2);

    // find lower grid snap for min
    if (grid_size.x > 0)
        cbox.min.x = cellbox.max.x * scalar_floor(cbox.min.x / cellbox.max.x);
    else
        cbox.min.x -= 0.5;
    if (grid_size.y > 0)
        cbox.min.y = cellbox.max.y * scalar_floor(cbox.min.y / cellbox.max.y);
    else
        cbox.min.y -= 0.5;

    // fill in with grid cells
    for (cur.x = cbox.min.x; cur.x < cbox.max.x; cur.x += cellbox.max.x)
        for (cur.y = cbox.min.y; cur.y < cbox.max.y; cur.y += cellbox.max.y) {
            cell = (BBoxPoolElem *)array_add(grid_cells);
            cell->bbox = cellbox;
            cell->wmat = mat3_scaling_rotation_translation(luavec2(1, 1), 0, cur);
            cell->selected = 0;
        }
}

static void _grid_init() { grid_cells = array_new(BBoxPoolElem); }
static void _grid_fini() { array_free(grid_cells); }

static void _grid_draw() {
    LuaVec2 win;
    unsigned int ncells;

    glUseProgram(bboxes_program);
    glUniformMatrix3fv(glGetUniformLocation(bboxes_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());
    win = game_get_window_size();
    glUniform1f(glGetUniformLocation(bboxes_program, "aspect"), win.x / win.y);
    glUniform1f(glGetUniformLocation(bboxes_program, "is_grid"), 1);

    _grid_create_cells();
    glBindVertexArray(bboxes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    ncells = array_length(grid_cells);
    glBufferData(GL_ARRAY_BUFFER, ncells * sizeof(BBoxPoolElem), array_begin(grid_cells), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, ncells);
    array_clear(grid_cells);
}

// --- line ----------------------------------------------------------------

static GLuint line_program;
static GLuint line_vao;
static GLuint line_vbo;

typedef struct LinePoint LinePoint;
struct LinePoint {
    LuaVec2 position;
    Scalar point_size;
    Color color;
};

static CArray *line_points;  // each consecutive pair is a line

void edit_line_add(LuaVec2 a, LuaVec2 b, Scalar point_size, Color color) {
    LinePoint *lp;

    lp = (LinePoint *)array_add(line_points);
    lp->position = a;
    lp->point_size = point_size;
    lp->color = color;

    lp = (LinePoint *)array_add(line_points);
    lp->position = b;
    lp->point_size = point_size;
    lp->color = color;
}

static void _line_init() {
    line_points = array_new(LinePoint);

    // init draw stuff
    line_program = gfx_create_program("line_program", "shader/edit_line.vert", NULL, "shader/edit_line.frag");
    glUseProgram(line_program);
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);
    glGenBuffers(1, &line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    gfx_bind_vertex_attrib(line_program, GL_FLOAT, 2, "position", LinePoint, position);
    gfx_bind_vertex_attrib(line_program, GL_FLOAT, 1, "point_size", LinePoint, point_size);
    gfx_bind_vertex_attrib(line_program, GL_FLOAT, 4, "color", LinePoint, color);
}
static void _line_fini() {
    // clean up draw stuff
    glDeleteProgram(line_program);
    glDeleteBuffers(1, &line_vbo);
    glDeleteVertexArrays(1, &line_vao);

    array_free(line_points);
}

static void _line_draw_all() {
    unsigned int npoints;

    const LuaMat3 *mat = camera_get_inverse_view_matrix_ptr();

    // bind program, update uniforms
    glUseProgram(line_program);
    GLuint inverse_view_matrix_id = glGetUniformLocation(line_program, "inverse_view_matrix");
    glUniformMatrix3fv(inverse_view_matrix_id, 1, GL_FALSE, (const GLfloat *)mat);

    // draw!
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

    npoints = array_length(line_points);
    glBufferData(GL_ARRAY_BUFFER, npoints * sizeof(LinePoint), array_begin(line_points), GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, npoints);
    glDrawArrays(GL_POINTS, 0, npoints);
}

// -------------------------------------------------------------------------

void edit_clear() {
    entitypool_clear(pool_bbox);
    array_clear(line_points);
}

void edit_init() {
    PROFILE_FUNC();

    pool_uneditable = entitypool_new(EntityPoolElem);

    _bboxes_init();
    _grid_init();
    _line_init();
}

void edit_fini() {
    _line_fini();
    _grid_fini();
    _bboxes_fini();

    entitypool_free(pool_uneditable);
}

static void _uneditable_remove(Entity ent) { entitypool_remove(pool_uneditable, ent); }

void edit_update_all() {
    entitypool_remove_destroyed(pool_uneditable, _uneditable_remove);

    _bboxes_update_all();
}

void edit_draw_all() {
    if (!enabled) return;

    _bboxes_draw_all();
    _grid_draw();
    _line_draw_all();
}

void edit_save_all(Store *s) {
    Store *t, *elem_s;
    EntityPoolElem *elem;

    if (store_child_save(&t, "edit", s)) {
        vec2_save(&grid_size, "grid_size", t);
        entitypool_save_foreach(elem, elem_s, pool_uneditable, "uneditable_pool", t);
    }
}
void edit_load_all(Store *s) {
    Store *t, *elem_s;
    EntityPoolElem *elem;

    if (store_child_load(&t, "edit", s)) {
        vec2_load(&grid_size, "grid_size", grid_size, t);
        entitypool_load_foreach(elem, elem_s, pool_uneditable, "uneditable_pool", t);
    }
}

#if 0

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
    ui_context_t *ui = &g_app->ui;
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

    ui_context_t *ui = &g_app->ui;

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

    ui_context_t *ui = &g_app->ui;

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

#ifdef NEKO_LITE

// Lite - A lightweight text editor written in Lua
// modified from https://github.com/rxi/lite (MIT license)
//               https://github.com/r-lyeh/FWK (public domain)
// modified by KaoruXun for NekoEngine

// deps
#include <stb_truetype.h>

// ----------------------------------------------------------------------------

int lt_mx = 0, lt_my = 0, lt_wx = 0, lt_wy = 0, lt_ww = 0, lt_wh = 0;

uint64_t lt_time_now() {
    auto now = std::chrono::steady_clock::now();
    return duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

lt_surface *lt_getsurface(void *window) {
    static lt_surface s = {0};
    return &s;
}

void lt_updatesurfacerects(lt_surface *s, lt_rect *rects, unsigned count) {
    if (0)
        for (int i = 0; i < count; ++i) {
            memset((unsigned *)s->pixels + (rects[i].x + rects[i].y * s->w), 0xFF, rects[i].width * 4);
            memset((unsigned *)s->pixels + (rects[i].x + (rects[i].y + (rects[i].height - 1)) * s->w), 0xFF, rects[i].width * 4);
            for (int y = 1; y < (rects[i].height - 1); ++y) {
                ((unsigned *)s->pixels)[rects[i].x + y * s->w] = ((unsigned *)s->pixels)[rects[i].x + (rects[i].width - 1) + y * s->w] = 0xFFFFFFFF;
            }
        }

    // update contents
    // texture_update(&s->t, s->w, s->h, 4, s->pixels, TEXTURE_LINEAR | TEXTURE_BGRA); // TODO
    s->t.width = s->w;
    s->t.height = s->h;
    texture_update_data(&s->t, (u8 *)s->pixels);
}

void ren_set_clip_rect(struct lt_rect rect);
void rencache_invalidate(void);
int lt_resizesurface(lt_surface *s, int ww, int wh) {
    s->w = ww, s->h = wh;
    // TODO
    if (s->w * s->h <= 0) {
        s->w = 0;
        s->h = 0;
    }
    if (s->t.id == 0 || s->w != s->t.width || s->h != s->t.height) {
        // invalidate tiles
        ren_set_clip_rect(lt_rect{0, 0, s->w, s->h});
        rencache_invalidate();

        // texture clear
        // if (!s->t.id) s->t = texture_create(1, 1, 4, "    ", TEXTURE_LINEAR | TEXTURE_RGBA | TEXTURE_BYTE);
        if (!s->t.id) {
            AssetTexture t = {.width = 1, .height = 1};
            bool ok = texture_update_data(&t, (u8 *)"    ");
            s->t = t;
        }
        s->pixels = mem_realloc(s->pixels, s->w * s->h * 4);
        memset(s->pixels, 0, s->w * s->h * 4);

        // texture update
        lt_updatesurfacerects(s, 0, 0);
        return 1;  // resized
    }
    return 0;  // unchanged
}

void *lt_load_file(const char *filename, int *size) {
    // int datalen;
    // char *data = file_load(filename, &datalen);
    // if (!data || !datalen) {
    //     filename = (const char *)file_normalize(filename);
    //     if (strbegi(filename, app_path())) filename += strlen(app_path());
    //     data = vfs_load(filename, &datalen);
    // }

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);
    if (size) *size = 0;
    if (!ok) {
        return NULL;
    }
    if (size) *size = contents.len;
    return contents.data;
}

const char *lt_button_name(int button) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) return "left";
    if (button == GLFW_MOUSE_BUTTON_RIGHT) return "right";
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) return "middle";
    return "?";
}

char *lt_key_name(char *dst, int key, int vk, int mods) {
    // @todo: "altgr" -> left ctrl + right alt

    if (key == GLFW_KEY_UP) return "up";
    if (key == GLFW_KEY_DOWN) return "down";
    if (key == GLFW_KEY_LEFT) return "left";
    if (key == GLFW_KEY_RIGHT) return "right";
    if (key == GLFW_KEY_LEFT_ALT) return "left alt";
    if (key == GLFW_KEY_RIGHT_ALT) return "right alt";
    if (key == GLFW_KEY_LEFT_SHIFT) return "left shift";
    if (key == GLFW_KEY_RIGHT_SHIFT) return "right shift";
    if (key == GLFW_KEY_LEFT_CONTROL) return "left ctrl";
    if (key == GLFW_KEY_RIGHT_CONTROL) return "right ctrl";
    if (key == GLFW_KEY_LEFT_SUPER) return "left windows";
    if (key == GLFW_KEY_RIGHT_SUPER) return "left windows";
    if (key == GLFW_KEY_MENU) return "menu";

    if (key == GLFW_KEY_ESCAPE) return "escape";
    if (key == GLFW_KEY_BACKSPACE) return "backspace";
    if (key == GLFW_KEY_ENTER) return "return";
    if (key == GLFW_KEY_KP_ENTER) return "keypad enter";
    if (key == GLFW_KEY_TAB) return "tab";
    if (key == GLFW_KEY_CAPS_LOCK) return "capslock";

    if (key == GLFW_KEY_HOME) return "home";
    if (key == GLFW_KEY_END) return "end";
    if (key == GLFW_KEY_INSERT) return "insert";
    if (key == GLFW_KEY_DELETE) return "delete";
    if (key == GLFW_KEY_PAGE_UP) return "pageup";
    if (key == GLFW_KEY_PAGE_DOWN) return "pagedown";

    if (key == GLFW_KEY_F1) return "f1";
    if (key == GLFW_KEY_F2) return "f2";
    if (key == GLFW_KEY_F3) return "f3";
    if (key == GLFW_KEY_F4) return "f4";
    if (key == GLFW_KEY_F5) return "f5";
    if (key == GLFW_KEY_F6) return "f6";
    if (key == GLFW_KEY_F7) return "f7";
    if (key == GLFW_KEY_F8) return "f8";
    if (key == GLFW_KEY_F9) return "f9";
    if (key == GLFW_KEY_F10) return "f10";
    if (key == GLFW_KEY_F11) return "f11";
    if (key == GLFW_KEY_F12) return "f12";

    const char *name = glfwGetKeyName(key, vk);
    strcpy(dst, name ? name : "");
    char *p = dst;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return dst;
}

void lt_globpath(struct lua_State *L, const char *path) {
    unsigned j = 0;

    // if (!strend(path, "/")) path = (const char *)va("%s/", path);
    // for (dir *d = dir_open(path, ""); d; dir_close(d), d = 0) {
    //     for (unsigned i = 0, end = dir_count(d); i < end; ++i) {
    //         char *name = dir_name(d, i);
    //         char *last = name + strlen(name) - 1;
    //         if (*last == '/') *last = '\0';
    //         name = file_name(name);
    //         lua_pushstring(L, name);
    //         lua_rawseti(L, -2, ++j);
    //     }
    // }

    // for (const char *section = strstri(path, LT_DATAPATH); section && section[sizeof(LT_DATAPATH) - 1] == '/'; section = 0) {
    //     array(char *) list = vfs_list("**");
    //     for (unsigned i = 0, end = array_count(list); i < end; ++i) {
    //         char *name = list[i];
    //         if (!strstri(name, section + 1)) continue;
    //         lua_pushstring(L, file_name(name));
    //         lua_rawseti(L, -2, ++j);
    //     }
    //     if (array_count(list)) return;
    // }

    namespace fs = std::filesystem;
    std::string basePath = path;

    if (basePath.back() != '/') {
        basePath += '/';
    }

    // First part: directory iteration using std::filesystem
    for (const auto &entry : fs::directory_iterator(basePath)) {
        if (entry.is_regular_file() || entry.is_directory()) {
            std::string name = entry.path().filename().string();
            lua_pushstring(L, name.c_str());
            lua_rawseti(L, -2, ++j);
        }
    }

    // Second part: handling the LT_DATAPATH section
    const std::string _LT_DATAPATH = LT_DATAPATH;  // Define LT_DATAPATH appropriately
    size_t sectionPos = basePath.find(_LT_DATAPATH);
    if (sectionPos != std::string::npos && basePath[sectionPos + _LT_DATAPATH.size()] == '/') {
        std::vector<std::string> list;
        for (const auto &entry : fs::recursive_directory_iterator(".")) {
            if (entry.is_regular_file() || entry.is_directory()) {
                list.push_back(entry.path().string());
            }
        }

        for (const auto &name : list) {
            if (name.find(basePath.substr(sectionPos + 1)) != std::string::npos) {
                lua_pushstring(L, fs::path(name).filename().string().c_str());
                lua_rawseti(L, -2, ++j);
            }
        }

        if (!list.empty()) return;
    }
}

int lt_emit_event(lua_State *L, const char *event_name, const char *event_fmt, ...) {
    int count = 0;
    lua_pushstring(L, event_name);
    if (event_fmt) {
        va_list va;
        va_start(va, event_fmt);
        for (; event_fmt[count]; ++count) {
            if (event_fmt[count] == 'd') {
                int d = va_arg(va, int);
                lua_pushnumber(L, d);
            } else if (event_fmt[count] == 'f') {
                double f = va_arg(va, double);
                lua_pushnumber(L, f);
            } else if (event_fmt[count] == 's') {
                const char *s = va_arg(va, const char *);
                lua_pushstring(L, s);
            }
        }
        va_end(va);
    }
    return 1 + count;
}

int printi(int i) {
    // printf("clicks: %d\n", i);
    return i;
}

INPUT_WRAP_DEFINE(lt);

static const char *codepoint_to_utf8_(unsigned c);
int lt_poll_event(lua_State *L) {  // init.lua > core.step() wakes on mousemoved || inputtext
    int rc = 0;
    char buf[16];
    static int prevx = 0, prevy = 0;

    static unsigned clicks_time = 0, clicks = 0;
    if ((lt_time_ms() - clicks_time) > 400) clicks = 0;

    for (INPUT_WRAP_event e; input_wrap_next_e(&lt_input_queue, &e); input_wrap_free_e(&e))
        if (e.type) switch (e.type) {
                default:
                    break;
                case INPUT_WRAP_WINDOW_CLOSED:  // it used to be ok. depends on window_swap() flow
                    rc += lt_emit_event(L, "quit", NULL);
                    return rc;

                    break;
                case INPUT_WRAP_WINDOW_MOVED:
                    lt_wx = e.pos.x;
                    lt_wy = e.pos.y;

                    break;
                case INPUT_WRAP_WINDOW_RESIZED:
                    rc += lt_emit_event(L, "resized", "dd", lt_ww = e.size.width, lt_wh = e.size.height);
                    lt_resizesurface(lt_getsurface(lt_window()), lt_ww, lt_wh);

                    break;
                case INPUT_WRAP_WINDOW_REFRESH:
                    rc += lt_emit_event(L, "exposed", NULL);
                    rencache_invalidate();

                    break;
                // case INPUT_WRAP_FILE_DROPPED:
                //     rc += lt_emit_event(L, "filedropped", "sdd", e.file.paths[0], lt_mx, lt_my);
                //     break;
                case INPUT_WRAP_KEY_PRESSED:
                case INPUT_WRAP_KEY_REPEATED:
                    rc += lt_emit_event(L, "keypressed", "s", lt_key_name(buf, e.keyboard.key, e.keyboard.scancode, e.keyboard.mods));
                    goto bottom;

                    break;
                case INPUT_WRAP_KEY_RELEASED:
                    rc += lt_emit_event(L, "keyreleased", "s", lt_key_name(buf, e.keyboard.key, e.keyboard.scancode, e.keyboard.mods));
                    goto bottom;

                    break;
                case INPUT_WRAP_CODEPOINT_INPUT:
                    rc += lt_emit_event(L, "textinput", "s", codepoint_to_utf8_(e.codepoint));

                    break;
                case INPUT_WRAP_BUTTON_PRESSED:
                    rc += lt_emit_event(L, "mousepressed", "sddd", lt_button_name(e.mouse.button), lt_mx, lt_my, printi(1 + clicks));

                    break;
                case INPUT_WRAP_BUTTON_RELEASED:
                    clicks += e.mouse.button == GLFW_MOUSE_BUTTON_1;
                    clicks_time = lt_time_ms();
                    rc += lt_emit_event(L, "mousereleased", "sdd", lt_button_name(e.mouse.button), lt_mx, lt_my);

                    break;
                case INPUT_WRAP_CURSOR_MOVED:
                    lt_mx = e.pos.x - lt_wx, lt_my = e.pos.y - lt_wy;
                    rc += lt_emit_event(L, "mousemoved", "dddd", lt_mx, lt_my, lt_mx - prevx, lt_my - prevy);
                    prevx = lt_mx, prevy = lt_my;

                    break;
                case INPUT_WRAP_SCROLLED:
                    rc += lt_emit_event(L, "mousewheel", "f", e.scroll.y);
            }

bottom:;

    return rc;
}

// ----------------------------------------------------------------------------
// lite/renderer.c

#define MAX_GLYPHSET 256

struct RenImage {
    RenColor *pixels;
    int width, height;
};

typedef struct {
    RenImage *image;
    stbtt_bakedchar glyphs[256];
} GlyphSet;

struct RenFont {
    void *data;
    stbtt_fontinfo stbfont;
    GlyphSet *sets[MAX_GLYPHSET];
    float size;
    int height;
};

static struct {
    int left, top, right, bottom;
} lt_clip;

static const char *codepoint_to_utf8_(unsigned c) {
    static char s[4 + 1];
    lt_memset(s, 0, 5);
    if (c < 0x80)
        s[0] = c, s[1] = 0;
    else if (c < 0x800)
        s[0] = 0xC0 | ((c >> 6) & 0x1F), s[1] = 0x80 | (c & 0x3F), s[2] = 0;
    else if (c < 0x10000)
        s[0] = 0xE0 | ((c >> 12) & 0x0F), s[1] = 0x80 | ((c >> 6) & 0x3F), s[2] = 0x80 | (c & 0x3F), s[3] = 0;
    else if (c < 0x110000)
        s[0] = 0xF0 | ((c >> 18) & 0x07), s[1] = 0x80 | ((c >> 12) & 0x3F), s[2] = 0x80 | ((c >> 6) & 0x3F), s[3] = 0x80 | (c & 0x3F), s[4] = 0;
    return s;
}
static const char *utf8_to_codepoint_(const char *p, unsigned *dst) {
    unsigned res, n;
    switch (*p & 0xf0) {
        case 0xf0:
            res = *p & 0x07;
            n = 3;
            break;
        case 0xe0:
            res = *p & 0x0f;
            n = 2;
            break;
        case 0xd0:
        case 0xc0:
            res = *p & 0x1f;
            n = 1;
            break;
        default:
            res = *p;
            n = 0;
            break;
    }
    while (n-- && *p++) {                //< https://github.com/rxi/lite/issues/262
        res = (res << 6) | (*p & 0x3f);  //< https://github.com/rxi/lite/issues/262
    }
    *dst = res;
    return p + 1;
}

void ren_init(void *win) {
    lt_surface *surf = lt_getsurface(lt_window());
    ren_set_clip_rect(RenRect{0, 0, surf->w, surf->h});
}

void ren_update_rects(RenRect *rects, int count) { lt_updatesurfacerects(lt_getsurface(lt_window()), (lt_rect *)rects, count); }

void ren_set_clip_rect(RenRect rect) {
    lt_clip.left = rect.x;
    lt_clip.top = rect.y;
    lt_clip.right = rect.x + rect.width;
    lt_clip.bottom = rect.y + rect.height;
}

void ren_get_size(int *x, int *y) {
    lt_surface *surf = lt_getsurface(lt_window());
    *x = surf->w;
    *y = surf->h;
}

RenImage *ren_new_image(int width, int height) {
    lt_assert(width > 0 && height > 0);
    RenImage *image = (RenImage *)lt_malloc(sizeof(RenImage) + width * height * sizeof(RenColor));
    image->pixels = (RenColor *)(image + 1);
    image->width = width;
    image->height = height;
    return image;
}

void ren_free_image(RenImage *image) { lt_free(image); }

static GlyphSet *load_glyphset(RenFont *font, int idx) {
    GlyphSet *set = (GlyphSet *)lt_calloc(1, sizeof(GlyphSet));

    // init image
    int width = 128;
    int height = 128;
retry:
    set->image = ren_new_image(width, height);

    // load glyphs
    float s = stbtt_ScaleForMappingEmToPixels(&font->stbfont, 1) / stbtt_ScaleForPixelHeight(&font->stbfont, 1);
    int res = stbtt_BakeFontBitmap((unsigned char *)font->data, 0, font->size * s, (unsigned char *)set->image->pixels, width, height, idx * 256, 256, set->glyphs);

    // retry with a larger image buffer if the buffer wasn't large enough
    if (res < 0) {
        width *= 2;
        height *= 2;
        ren_free_image(set->image);
        goto retry;
    }

    // adjust glyph yoffsets and xadvance
    int ascent, descent, linegap;
    stbtt_GetFontVMetrics(&font->stbfont, &ascent, &descent, &linegap);
    float scale = stbtt_ScaleForMappingEmToPixels(&font->stbfont, font->size);
    int scaled_ascent = ascent * scale + 0.5;
    for (int i = 0; i < 256; i++) {
        set->glyphs[i].yoff += scaled_ascent;
        set->glyphs[i].xadvance = floor(set->glyphs[i].xadvance);
    }

    // convert 8bit data to 32bit
    for (int i = width * height - 1; i >= 0; i--) {
        uint8_t n = *((uint8_t *)set->image->pixels + i);
        set->image->pixels[i] = RenColor{.b = 255, .g = 255, .r = 255, .a = n};
    }

    return set;
}

static GlyphSet *get_glyphset(RenFont *font, int codepoint) {
    int idx = (codepoint >> 8) % MAX_GLYPHSET;
    if (!font->sets[idx]) {
        font->sets[idx] = load_glyphset(font, idx);
    }
    return font->sets[idx];
}

RenFont *ren_load_font(const char *filename, float size) {
    // load font into buffer
    char *fontdata = (char *)lt_load_file(filename, NULL);
    if (!fontdata) return NULL;

    RenFont *font = NULL;

    // init font
    font = (RenFont *)lt_calloc(1, sizeof(RenFont));
    font->size = size;
    font->data = fontdata;

    // init stbfont
    int ok = stbtt_InitFont(&font->stbfont, (unsigned char *)font->data, 0);
    if (!ok) {
        if (font) {
            lt_free(font->data);
        }
        lt_free(font);
        return NULL;
    }

    // get height and scale
    int ascent, descent, linegap;
    stbtt_GetFontVMetrics(&font->stbfont, &ascent, &descent, &linegap);
    float scale = stbtt_ScaleForMappingEmToPixels(&font->stbfont, size);
    font->height = (ascent - descent + linegap) * scale + 0.5;

    // make tab and newline glyphs invisible
    stbtt_bakedchar *g = get_glyphset(font, '\n')->glyphs;
    g['\t'].x1 = g['\t'].x0;
    g['\n'].x1 = g['\n'].x0;

    return font;
}

void ren_free_font(RenFont *font) {
    for (int i = 0; i < MAX_GLYPHSET; i++) {
        GlyphSet *set = font->sets[i];
        if (set) {
            ren_free_image(set->image);
            lt_free(set);
        }
    }
    lt_free(font->data);
    lt_free(font);
}

void ren_set_font_tab_width(RenFont *font, int n) {
    GlyphSet *set = get_glyphset(font, '\t');
    set->glyphs['\t'].xadvance = n;
}

int ren_get_font_tab_width(RenFont *font) {
    GlyphSet *set = get_glyphset(font, '\t');
    return set->glyphs['\t'].xadvance;
}

int ren_get_font_width(RenFont *font, const char *text) {
    int x = 0;
    const char *p = text;
    unsigned codepoint;
    while (*p) {
        p = utf8_to_codepoint_(p, &codepoint);
        GlyphSet *set = get_glyphset(font, codepoint);
        stbtt_bakedchar *g = &set->glyphs[codepoint & 0xff];
        x += g->xadvance;
    }
    return x;
}

int ren_get_font_height(RenFont *font) { return font->height; }

static inline RenColor blend_pixel(RenColor dst, RenColor src) {
    int ia = 0xff - src.a;
    dst.r = ((src.r * src.a) + (dst.r * ia)) >> 8;
    dst.g = ((src.g * src.a) + (dst.g * ia)) >> 8;
    dst.b = ((src.b * src.a) + (dst.b * ia)) >> 8;
    return dst;
}

static inline RenColor blend_pixel2(RenColor dst, RenColor src, RenColor color) {
    src.a = (src.a * color.a) >> 8;
    int ia = 0xff - src.a;
    dst.r = ((src.r * color.r * src.a) >> 16) + ((dst.r * ia) >> 8);
    dst.g = ((src.g * color.g * src.a) >> 16) + ((dst.g * ia) >> 8);
    dst.b = ((src.b * color.b * src.a) >> 16) + ((dst.b * ia) >> 8);
    return dst;
}

#define rect_draw_loop(expr)            \
    for (int j = y1; j < y2; j++) {     \
        for (int i = x1; i < x2; i++) { \
            *d = expr;                  \
            d++;                        \
        }                               \
        d += dr;                        \
    }

void ren_draw_rect(RenRect rect, RenColor color) {
    if (color.a == 0) {
        return;
    }

    int x1 = rect.x < lt_clip.left ? lt_clip.left : rect.x;
    int y1 = rect.y < lt_clip.top ? lt_clip.top : rect.y;
    int x2 = rect.x + rect.width;
    int y2 = rect.y + rect.height;
    x2 = x2 > lt_clip.right ? lt_clip.right : x2;
    y2 = y2 > lt_clip.bottom ? lt_clip.bottom : y2;

    lt_surface *surf = lt_getsurface(lt_window());
    RenColor *d = (RenColor *)surf->pixels;
    d += x1 + y1 * surf->w;
    int dr = surf->w - (x2 - x1);

    if (color.a == 0xff) {
        rect_draw_loop(color);
    } else {
        rect_draw_loop(blend_pixel(*d, color));
    }
}

void ren_draw_image(RenImage *image, RenRect *sub, int x, int y, RenColor color) {
    if (color.a == 0) {
        return;
    }

    // clip
    int n;
    if ((n = lt_clip.left - x) > 0) {
        sub->width -= n;
        sub->x += n;
        x += n;
    }
    if ((n = lt_clip.top - y) > 0) {
        sub->height -= n;
        sub->y += n;
        y += n;
    }
    if ((n = x + sub->width - lt_clip.right) > 0) {
        sub->width -= n;
    }
    if ((n = y + sub->height - lt_clip.bottom) > 0) {
        sub->height -= n;
    }

    if (sub->width <= 0 || sub->height <= 0) {
        return;
    }

    // draw
    lt_surface *surf = lt_getsurface(lt_window());
    RenColor *s = image->pixels;
    RenColor *d = (RenColor *)surf->pixels;
    s += sub->x + sub->y * image->width;
    d += x + y * surf->w;
    int sr = image->width - sub->width;
    int dr = surf->w - sub->width;

    for (int j = 0; j < sub->height; j++) {
        for (int i = 0; i < sub->width; i++) {
            *d = blend_pixel2(*d, *s, color);
            d++;
            s++;
        }
        d += dr;
        s += sr;
    }
}

int ren_draw_text(RenFont *font, const char *text, int x, int y, RenColor color) {
    RenRect rect;
    const char *p = text;
    unsigned codepoint;
    while (*p) {
        p = utf8_to_codepoint_(p, &codepoint);
        GlyphSet *set = get_glyphset(font, codepoint);
        stbtt_bakedchar *g = &set->glyphs[codepoint & 0xff];
        rect.x = g->x0;
        rect.y = g->y0;
        rect.width = g->x1 - g->x0;
        rect.height = g->y1 - g->y0;
        ren_draw_image(set->image, &rect, x + g->xoff, y + g->yoff, color);
        x += g->xadvance;
    }
    return x;
}

// ----------------------------------------------------------------------------
// lite/renderer_font.c

static int f_load(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    float size = luaL_checknumber(L, 2);
    RenFont **self = (RenFont **)lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, API_TYPE_FONT);
    *self = ren_load_font(filename, size);
    if (!*self) {
        luaL_error(L, "failed to load font");
    }
    return 1;
}

static int f_set_tab_width(lua_State *L) {
    RenFont **self = (RenFont **)luaL_checkudata(L, 1, API_TYPE_FONT);
    int n = luaL_checknumber(L, 2);
    ren_set_font_tab_width(*self, n);
    return 0;
}

static int f_GC(lua_State *L) {
    RenFont **self = (RenFont **)luaL_checkudata(L, 1, API_TYPE_FONT);
    if (*self) {
        rencache_free_font(*self);
    }
    return 0;
}

static int f_get_width(lua_State *L) {
    RenFont **self = (RenFont **)luaL_checkudata(L, 1, API_TYPE_FONT);
    const char *text = luaL_checkstring(L, 2);
    lua_pushnumber(L, ren_get_font_width(*self, text));
    return 1;
}

static int f_get_height(lua_State *L) {
    RenFont **self = (RenFont **)luaL_checkudata(L, 1, API_TYPE_FONT);
    lua_pushnumber(L, ren_get_font_height(*self));
    return 1;
}

int luaopen_renderer_font(lua_State *L) {
    static const luaL_Reg lib[] = {{"__gc", f_GC}, {"load", f_load}, {"set_tab_width", f_set_tab_width}, {"get_width", f_get_width}, {"get_height", f_get_height}, {NULL, NULL}};
    luaL_newmetatable(L, API_TYPE_FONT);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

// ----------------------------------------------------------------------------
// lite/renderer_api.c

static RenColor checkcolor(lua_State *L, int idx, int def) {
    RenColor color;
    if (lua_isnoneornil(L, idx)) {
        return RenColor{(u8)def, (u8)def, (u8)def, 255};
    }
    lua_rawgeti(L, idx, 1);
    lua_rawgeti(L, idx, 2);
    lua_rawgeti(L, idx, 3);
    lua_rawgeti(L, idx, 4);
    color.r = luaL_checknumber(L, -4);
    color.g = luaL_checknumber(L, -3);
    color.b = luaL_checknumber(L, -2);
    color.a = luaL_optnumber(L, -1, 255);
    lua_pop(L, 4);
    return color;
}

static int f_show_debug(lua_State *L) {
    luaL_checkany(L, 1);
    rencache_show_debug(lua_toboolean(L, 1));
    return 0;
}

static int f_get_size(lua_State *L) {
    int w, h;
    ren_get_size(&w, &h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}

static int f_begin_frame(lua_State *L) {
    rencache_begin_frame();
    return 0;
}

static int f_end_frame(lua_State *L) {
    rencache_end_frame();
    return 0;
}

static int f_set_clip_rect(lua_State *L) {
    RenRect rect;
    rect.x = luaL_checknumber(L, 1);
    rect.y = luaL_checknumber(L, 2);
    rect.width = luaL_checknumber(L, 3);
    rect.height = luaL_checknumber(L, 4);
    rencache_set_clip_rect(rect);
    return 0;
}

static int f_draw_rect(lua_State *L) {
    RenRect rect;
    rect.x = luaL_checknumber(L, 1);
    rect.y = luaL_checknumber(L, 2);
    rect.width = luaL_checknumber(L, 3);
    rect.height = luaL_checknumber(L, 4);
    RenColor color = checkcolor(L, 5, 255);
    rencache_draw_rect(rect, color);
    return 0;
}

static int f_draw_text(lua_State *L) {
    RenFont **font = (RenFont **)luaL_checkudata(L, 1, API_TYPE_FONT);
    const char *text = luaL_checkstring(L, 2);
    int x = luaL_checknumber(L, 3);
    int y = luaL_checknumber(L, 4);
    RenColor color = checkcolor(L, 5, 255);
    x = rencache_draw_text(*font, text, x, y, color);
    lua_pushnumber(L, x);
    return 1;
}

int luaopen_renderer(lua_State *L) {
    static const luaL_Reg lib[] = {{"show_debug", f_show_debug},       {"get_size", f_get_size},   {"begin_frame", f_begin_frame}, {"end_frame", f_end_frame},
                                   {"set_clip_rect", f_set_clip_rect}, {"draw_rect", f_draw_rect}, {"draw_text", f_draw_text},     {NULL, NULL}};
    luaL_newlib(L, lib);
    luaopen_renderer_font(L);
    lua_setfield(L, -2, "font");
    return 1;
}

// ----------------------------------------------------------------------------
// lite/rencache.c

/* a cache over the software renderer -- all drawing operations are stored as
** commands when issued. At the end of the frame we write the commands to a grid
** of hash values, take the cells that have changed since the previous frame,
** merge them into dirty rectangles and redraw only those regions */

#define CELLS_X 80
#define CELLS_Y 50
#define CELL_SIZE 96
#define COMMAND_BUF_SIZE (1024 * 512)

enum { FREE_FONT, SET_CLIP, DRAW_TEXT, DRAW_RECT };

typedef struct {
    int type, size;
    RenRect rect;
    RenColor color;
    RenFont *font;
    int tab_width;
    char text[0];
} Command;

static unsigned cells_buf1[CELLS_X * CELLS_Y];
static unsigned cells_buf2[CELLS_X * CELLS_Y];
static unsigned *cells_prev = cells_buf1;
static unsigned *cells = cells_buf2;
static RenRect rect_buf[CELLS_X * CELLS_Y / 2];
static char command_buf[COMMAND_BUF_SIZE];
static int command_buf_idx;
static RenRect screen_rect;
static bool show_debug;

// 32bit fnv-1a hash
#define HASH_INITIAL 2166136261

static void hash(unsigned *h, const void *data, int size) {
    const unsigned char *p = (unsigned char *)data;
    while (size--) {
        *h = (*h ^ *p++) * 16777619;
    }
}

static inline int cell_idx(int x, int y) { return x + y * CELLS_X; }

static inline bool rects_overlap(RenRect a, RenRect b) { return b.x + b.width >= a.x && b.x <= a.x + a.width && b.y + b.height >= a.y && b.y <= a.y + a.height; }

static RenRect intersect_rects(RenRect a, RenRect b) {
    int x1 = NEKO_MAX(a.x, b.x);
    int y1 = NEKO_MAX(a.y, b.y);
    int x2 = NEKO_MIN(a.x + a.width, b.x + b.width);
    int y2 = NEKO_MIN(a.y + a.height, b.y + b.height);
    return RenRect{x1, y1, NEKO_MAX(0, x2 - x1), NEKO_MAX(0, y2 - y1)};
}

static RenRect merge_rects(RenRect a, RenRect b) {
    int x1 = NEKO_MIN(a.x, b.x);
    int y1 = NEKO_MIN(a.y, b.y);
    int x2 = NEKO_MAX(a.x + a.width, b.x + b.width);
    int y2 = NEKO_MAX(a.y + a.height, b.y + b.height);
    return RenRect{x1, y1, x2 - x1, y2 - y1};
}

static Command *push_command(int type, int size) {
    size_t alignment = 7;                    // alignof(max_align_t) - 1; //< C11 https://github.com/rxi/lite/pull/292/commits/ad1bdf56e3f212446e1c61fd45de8b94de5e2bc3
    size = (size + alignment) & ~alignment;  //< https://github.com/rxi/lite/pull/292/commits/ad1bdf56e3f212446e1c61fd45de8b94de5e2bc3
    Command *cmd = (Command *)(command_buf + command_buf_idx);
    int n = command_buf_idx + size;
    if (n > COMMAND_BUF_SIZE) {
        fprintf(stderr, "Warning: (" __FILE__ "): exhausted command buffer\n");
        return NULL;
    }
    command_buf_idx = n;
    lt_memset(cmd, 0, sizeof(Command));
    cmd->type = type;
    cmd->size = size;
    return cmd;
}

static bool next_command(Command **prev) {
    if (*prev == NULL) {
        *prev = (Command *)command_buf;
    } else {
        *prev = (Command *)(((char *)*prev) + (*prev)->size);
    }
    return *prev != ((Command *)(command_buf + command_buf_idx));
}

void rencache_show_debug(bool enable) { show_debug = enable; }

void rencache_free_font(RenFont *font) {
    Command *cmd = push_command(FREE_FONT, sizeof(Command));
    if (cmd) {
        cmd->font = font;
    }
}

void rencache_set_clip_rect(RenRect rect) {
    Command *cmd = push_command(SET_CLIP, sizeof(Command));
    if (cmd) {
        cmd->rect = intersect_rects(rect, screen_rect);
    }
}

void rencache_draw_rect(RenRect rect, RenColor color) {
    if (!rects_overlap(screen_rect, rect)) {
        return;
    }
    Command *cmd = push_command(DRAW_RECT, sizeof(Command));
    if (cmd) {
        cmd->rect = rect;
        cmd->color = color;
    }
}

int rencache_draw_text(RenFont *font, const char *text, int x, int y, RenColor color) {
    RenRect rect;
    rect.x = x;
    rect.y = y;
    rect.width = ren_get_font_width(font, text);
    rect.height = ren_get_font_height(font);

    if (rects_overlap(screen_rect, rect)) {
        int sz = strlen(text) + 1;
        Command *cmd = push_command(DRAW_TEXT, sizeof(Command) + sz);
        if (cmd) {
            memcpy(cmd->text, text, sz);
            cmd->color = color;
            cmd->font = font;
            cmd->rect = rect;
            cmd->tab_width = ren_get_font_tab_width(font);
        }
    }

    return x + rect.width;
}

void rencache_invalidate(void) { lt_memset(cells_prev, 0xff, sizeof(cells_buf1)); }

void rencache_begin_frame(void) {
    // reset all cells if the screen width/height has changed
    int w, h;
    ren_get_size(&w, &h);
    if (screen_rect.width != w || h != screen_rect.height) {
        screen_rect.width = w;
        screen_rect.height = h;
        rencache_invalidate();
    }
}

static void update_overlapping_cells(RenRect r, unsigned h) {
    int x1 = r.x / CELL_SIZE;
    int y1 = r.y / CELL_SIZE;
    int x2 = (r.x + r.width) / CELL_SIZE;
    int y2 = (r.y + r.height) / CELL_SIZE;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int idx = cell_idx(x, y);
            hash(&cells[idx], &h, sizeof(h));
        }
    }
}

static void push_rect(RenRect r, int *count) {
    // try to merge with existing rectangle
    for (int i = *count - 1; i >= 0; i--) {
        RenRect *rp = &rect_buf[i];
        if (rects_overlap(*rp, r)) {
            *rp = merge_rects(*rp, r);
            return;
        }
    }
    // couldn't merge with previous rectangle: push
    rect_buf[(*count)++] = r;
}

void rencache_end_frame(void) {
    // update cells from commands
    Command *cmd = NULL;
    RenRect cr = screen_rect;
    while (next_command(&cmd)) {
        if (cmd->type == SET_CLIP) {
            cr = cmd->rect;
        }
        RenRect r = intersect_rects(cmd->rect, cr);
        if (r.width == 0 || r.height == 0) {
            continue;
        }
        unsigned h = HASH_INITIAL;
        hash(&h, cmd, cmd->size);
        update_overlapping_cells(r, h);
    }

    // push rects for all cells changed from last frame, reset cells
    int rect_count = 0;
    int max_x = screen_rect.width / CELL_SIZE + 1;
    int max_y = screen_rect.height / CELL_SIZE + 1;
    for (int y = 0; y < max_y; y++) {
        for (int x = 0; x < max_x; x++) {
            // compare previous and current cell for change
            int idx = cell_idx(x, y);
            if (cells[idx] != cells_prev[idx]) {
                push_rect(RenRect{x, y, 1, 1}, &rect_count);
            }
            cells_prev[idx] = HASH_INITIAL;
        }
    }

    // expand rects from cells to pixels
    for (int i = 0; i < rect_count; i++) {
        RenRect *r = &rect_buf[i];
        r->x *= CELL_SIZE;
        r->y *= CELL_SIZE;
        r->width *= CELL_SIZE;
        r->height *= CELL_SIZE;
        *r = intersect_rects(*r, screen_rect);
    }

    // redraw updated regions
    bool has_free_commands = false;
    for (int i = 0; i < rect_count; i++) {
        // draw
        RenRect r = rect_buf[i];
        ren_set_clip_rect(r);

        cmd = NULL;
        while (next_command(&cmd)) {
            switch (cmd->type) {
                case FREE_FONT:
                    has_free_commands = true;
                    break;
                case SET_CLIP:
                    ren_set_clip_rect(intersect_rects(cmd->rect, r));
                    break;
                case DRAW_RECT:
                    ren_draw_rect(cmd->rect, cmd->color);
                    break;
                case DRAW_TEXT:
                    ren_set_font_tab_width(cmd->font, cmd->tab_width);
                    ren_draw_text(cmd->font, cmd->text, cmd->rect.x, cmd->rect.y, cmd->color);
                    break;
            }
        }

        if (show_debug) {
            RenColor color = {(u8)rand(), (u8)rand(), (u8)rand(), 50};
            ren_draw_rect(r, color);
        }
    }

    // update dirty rects
    if (rect_count > 0) {
        ren_update_rects(rect_buf, rect_count);
    }

    // free fonts
    if (has_free_commands) {
        cmd = NULL;
        while (next_command(&cmd)) {
            if (cmd->type == FREE_FONT) {
                ren_free_font(cmd->font);
            }
        }
    }

    // swap cell buffer and reset
    unsigned *tmp = cells;
    cells = cells_prev;
    cells_prev = tmp;
    command_buf_idx = 0;
}

// ----------------------------------------------------------------------------
// lite/system.c

static int f_set_cursor(lua_State *L) {
    static const char *cursor_opts[] = {"arrow", "ibeam", "sizeh", "sizev", "hand", NULL};
    int n = luaL_checkoption(L, 1, "arrow", cursor_opts);
    lt_setcursor(n);
    return 0;
}

static int f_set_window_title(lua_State *L) {
    const char *title = luaL_checkstring(L, 1);
    lt_setwindowtitle(title);
    return 0;
}
static int f_set_window_mode(lua_State *L) {
    static const char *window_opts[] = {"normal", "maximized", "fullscreen", 0};
    enum { WIN_NORMAL, WIN_MAXIMIZED, WIN_FULLSCREEN };
    int n = luaL_checkoption(L, 1, "normal", window_opts);
    lt_setwindowmode(n);
    return 0;
}
static int f_window_has_focus(lua_State *L) {
    unsigned flags = lt_haswindowfocus();
    lua_pushboolean(L, flags);
    return 1;
}

static int f_show_confirm_dialog(lua_State *L) {
    const char *title = luaL_checkstring(L, 1);
    const char *msg = luaL_checkstring(L, 2);
    int id = lt_prompt(msg, title);  // 0:no, 1:yes
    lua_pushboolean(L, !!id);
    return 1;
}

static int f_chdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int err = neko_os_chdir(path);
    if (err) {
        luaL_error(L, "chdir() failed");
    }
    return 0;
}
static int f_list_dir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    lua_newtable(L);
    lt_globpath(L, path);
    return 1;
}
static int f_absolute_path(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    char *res = lt_realpath(path, NULL);
    if (!res) {
        return 0;
    }
    lua_pushstring(L, res);
    lt_realpath_free(res);
    return 1;
}
static int f_get_file_info(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);

    struct stat s;
    int err = stat(path, &s);
    if (err < 0) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }

    lua_newtable(L);
    lua_pushnumber(L, s.st_mtime);
    lua_setfield(L, -2, "modified");

    lua_pushnumber(L, s.st_size);
    lua_setfield(L, -2, "size");

    if (S_ISREG(s.st_mode)) {
        lua_pushstring(L, "file");
    } else if (S_ISDIR(s.st_mode)) {
        lua_pushstring(L, "dir");
    } else {
        lua_pushnil(L);
    }
    lua_setfield(L, -2, "type");

    return 1;
}

static int f_get_clipboard(lua_State *L) {
    const char *text = lt_getclipboard(lt_window());
    if (!text) {
        return 0;
    }
    lua_pushstring(L, text);
    return 1;
}
static int f_set_clipboard(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    lt_setclipboard(lt_window(), text);
    return 0;
}

static int f_get_time(lua_State *L) {
    double ss = lt_time_ms() / 1000.0;
    lua_pushnumber(L, ss);
    return 1;
}
static int f_sleep(lua_State *L) {
    double ss = luaL_checknumber(L, 1);
    lt_sleep_ms(ss * 1000);
    return 0;
}

static int f_exec(lua_State *L) {
    size_t len;
    const char *cmd = luaL_checklstring(L, 1, &len);
    char *buf = (char *)lt_malloc(len + 32);
    if (!buf) {
        luaL_error(L, "buffer allocation failed");
    }
#if _WIN32
    sprintf(buf, "cmd /c \"%s\"", cmd);
    WinExec(buf, SW_HIDE);
#else
    sprintf(buf, "%s &", cmd);
    int res = system(buf);
#endif
    lt_free(buf);
    return 0;
}

static int f_fuzzy_match(lua_State *L) {
    const char *str = luaL_checkstring(L, 1);
    const char *ptn = luaL_checkstring(L, 2);
    int score = 0;
    int run = 0;

    while (*str && *ptn) {
        while (*str == ' ') {
            str++;
        }
        while (*ptn == ' ') {
            ptn++;
        }
        if (tolower(*str) == tolower(*ptn)) {
            score += run * 10 - (*str != *ptn);
            run++;
            ptn++;
        } else {
            score -= 10;
            run = 0;
        }
        str++;
    }
    if (*ptn) {
        return 0;
    }

    lua_pushnumber(L, score - (int)strlen(str));
    return 1;
}

static int f_poll_event(lua_State *L) {  // init.lua > core.step() wakes on mousemoved || inputtext
    int rc = lt_poll_event(L);
    return rc;
}

int luaopen_system(lua_State *L) {
    static const luaL_Reg lib[] = {{"poll_event", f_poll_event},
                                   {"set_cursor", f_set_cursor},
                                   {"set_window_title", f_set_window_title},
                                   {"set_window_mode", f_set_window_mode},
                                   {"window_has_focus", f_window_has_focus},
                                   {"show_confirm_dialog", f_show_confirm_dialog},
                                   {"chdir", f_chdir},
                                   {"list_dir", f_list_dir},
                                   {"absolute_path", f_absolute_path},
                                   {"get_file_info", f_get_file_info},
                                   {"get_clipboard", f_get_clipboard},
                                   {"set_clipboard", f_set_clipboard},
                                   {"get_time", f_get_time},
                                   {"sleep", f_sleep},
                                   {"exec", f_exec},
                                   {"fuzzy_match", f_fuzzy_match},
                                   {NULL, NULL}};
    luaL_newlib(L, lib);
    return 1;
}

// ----------------------------------------------------------------------------
// lite/api/api.c

void api_load_libs(lua_State *L) {
    static const luaL_Reg libs[] = {{"system", luaopen_system}, {"renderer", luaopen_renderer}, {NULL, NULL}};
    for (int i = 0; libs[i].name; i++) {
        luaL_requiref(L, libs[i].name, libs[i].func, 1);
    }
}

// ----------------------------------------------------------------------------
// lite/main.c

void lt_init(lua_State *L, void *handle, const char *pathdata, int argc, char **argv, float scale, const char *platform) {
    // setup renderer
    ren_init(handle);

    // setup lua context
    api_load_libs(L);

    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "ARGS");

    lua_pushstring(L, "1.11");
    lua_setglobal(L, "VERSION");

    lua_pushstring(L, platform);
    lua_setglobal(L, "PLATFORM");

    lua_pushnumber(L, scale);
    lua_setglobal(L, "SCALE");

    lua_pushstring(L, pathdata);
    lua_setglobal(L, "DATADIR");

    lua_pushstring(L, g_app->lite_init_path.cstr());
    lua_setglobal(L, "NEKO_LITE_DIR");

    lua_pushstring(L, os_program_dir().cstr());
    lua_setglobal(L, "EXEDIR");

    // init lite
    luaL_dostring(L, "core = {}");
    luaL_dostring(L,
                  "xpcall(function()\n"
                  "  SCALE = tonumber(os.getenv(\"LITE_SCALE\")) or SCALE\n"
                  "  PATHSEP = package.config:sub(1, 1)\n"
                  "  USERDIR = NEKO_LITE_DIR .. '/data/user/'\n"
                  "  package.path = NEKO_LITE_DIR .. '/data/?.lua;' .. package.path\n"
                  "  package.path = NEKO_LITE_DIR .. '/data/?/init.lua;' .. package.path\n"
                  "  core = require('core')\n"
                  "  core.init()\n"
                  "end, function(err)\n"
                  "  print('Error: ' .. tostring(err))\n"
                  "  print(debug.traceback(nil, 2))\n"
                  "  if core and core.on_error then\n"
                  "    pcall(core.on_error, err)\n"
                  "  end\n"
                  "  os.exit(1)\n"
                  "end)");

#if 1
    input_add_key_down_callback(lt_key_down);
    input_add_key_up_callback(lt_key_up);
    input_add_char_down_callback(lt_char_down);
    input_add_mouse_down_callback(lt_mouse_down);
    input_add_mouse_up_callback(lt_mouse_up);
    input_add_mouse_move_callback(lt_mouse_move);
    input_add_scroll_callback(lt_scroll);
#endif
}

void lt_tick(struct lua_State *L) {
    luaL_dostring(L,
                  "xpcall(function()\n"
                  "  core.run1()\n"
                  "end, function(err)\n"
                  "  print('Error: ' .. tostring(err))\n"
                  "  print(debug.traceback(nil, 2))\n"
                  "  if core and core.on_error then\n"
                  "    pcall(core.on_error, err)\n"
                  "  end\n"
                  "  os.exit(1)\n"
                  "end)");
}

void lt_fini() {
    auto s = lt_getsurface(lt_window());
    mem_free(s->pixels);
}

#endif