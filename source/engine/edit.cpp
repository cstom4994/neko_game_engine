#include "engine/edit.h"

#include <inttypes.h>
#include <stdio.h>

#include "engine/base.h"
#include "engine/camera.h"
#include "engine/entity.h"
#include "engine/game.h"
#include "engine/input.h"
#include "engine/prelude.h"
#include "engine/transform.h"
#include "gfx.h"

static bool enabled;

// editability data
static EntityPool *uneditable_pool; /* Entites are in this pool
                                       iff. not editable */

void edit_set_enabled(bool e) { enabled = e; }
bool edit_get_enabled() { return enabled; }

void edit_set_editable(Entity ent, bool editable) {
    if (editable)
        entitypool_remove(uneditable_pool, ent);
    else
        entitypool_add(uneditable_pool, ent);
}
bool edit_get_editable(Entity ent) { return !entitypool_get(uneditable_pool, ent); }

// --- bboxes --------------------------------------------------------------

// bbox pool
typedef struct BBoxPoolElem BBoxPoolElem;
struct BBoxPoolElem {
    EntityPoolElem pool_elem;

    LuaMat3 wmat;
    BBox bbox;
    Scalar selected;  // > 0.5 if and only if selected
};
static EntityPool *bbox_pool;

void edit_bboxes_update(Entity ent, BBox bbox) {
    BBoxPoolElem *elem;

    // editable?
    if (!edit_get_editable(ent)) return;

    elem = (BBoxPoolElem *)entitypool_get(bbox_pool, ent);

    // merge if already exists, else set
    if (elem)
        elem->bbox = bbox_merge(elem->bbox, bbox);
    else {
        elem = (BBoxPoolElem *)entitypool_add(bbox_pool, ent);
        elem->bbox = bbox;
    }
}

bool edit_bboxes_has(Entity ent) { return entitypool_get(bbox_pool, ent) != NULL; }
BBox edit_bboxes_get(Entity ent) {
    BBoxPoolElem *elem = (BBoxPoolElem *)entitypool_get(bbox_pool, ent);
    error_assert(elem);
    return elem->bbox;
}

unsigned int edit_bboxes_get_num() { return entitypool_size(bbox_pool); }

Entity edit_bboxes_get_nth_ent(unsigned int n) {
    BBoxPoolElem *elem;
    // struct EntityBBoxPair bbpair {};

    error_assert(n < entitypool_size(bbox_pool));
    elem = (BBoxPoolElem *)entitypool_nth(bbox_pool, n);

    // bbpair.ent = elem->pool_elem.ent;
    // bbpair.bbox = elem->bbox;
    return elem->pool_elem.ent;
}

BBox edit_bboxes_get_nth_bbox(unsigned int n) {
    BBoxPoolElem *elem;
    // struct EntityBBoxPair bbpair {};

    error_assert(n < entitypool_size(bbox_pool));
    elem = (BBoxPoolElem *)entitypool_nth(bbox_pool, n);

    // bbpair.ent = elem->pool_elem.ent;
    // bbpair.bbox = elem->bbox;
    return elem->bbox;
}

void edit_bboxes_set_selected(Entity ent, bool selected) {
    BBoxPoolElem *elem = (BBoxPoolElem *)entitypool_get(bbox_pool, ent);
    error_assert(elem);
    elem->selected = selected;
}

static GLuint bboxes_program;
static GLuint bboxes_vao;
static GLuint bboxes_vbo;

static void _bboxes_init() {
    bbox_pool = entitypool_new(BBoxPoolElem);

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

    entitypool_free(bbox_pool);
}

static void _bboxes_update_all() {
    Entity ent;
    BBoxPoolElem *elem;
    static BBox defaultbb = {{-0.25, -0.25}, {0.25, 0.25}};

    if (!enabled) return;

    entitypool_foreach(elem, bbox_pool) {
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
    nbboxes = entitypool_size(bbox_pool);
    glBufferData(GL_ARRAY_BUFFER, nbboxes * sizeof(BBoxPoolElem), entitypool_begin(bbox_pool), GL_STREAM_DRAW);
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
    entitypool_clear(bbox_pool);
    array_clear(line_points);
}

void edit_init() {
    PROFILE_FUNC();

    uneditable_pool = entitypool_new(EntityPoolElem);

    _bboxes_init();
    _grid_init();
    _line_init();
}

void edit_fini() {
    _line_fini();
    _grid_fini();
    _bboxes_fini();

    entitypool_free(uneditable_pool);
}

static void _uneditable_remove(Entity ent) { entitypool_remove(uneditable_pool, ent); }

void edit_update_all() {
    entitypool_remove_destroyed(uneditable_pool, _uneditable_remove);

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
        entitypool_save_foreach(elem, elem_s, uneditable_pool, "uneditable_pool", t);
    }
}
void edit_load_all(Store *s) {
    Store *t, *elem_s;
    EntityPoolElem *elem;

    if (store_child_load(&t, "edit", s)) {
        vec2_load(&grid_size, "grid_size", grid_size, t);
        entitypool_load_foreach(elem, elem_s, uneditable_pool, "uneditable_pool", t);
    }
}

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
