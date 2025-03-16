#include "base/common/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/entitybase.hpp"

static bool enabled;

void edit_set_enabled(bool e) { enabled = e; }
bool edit_get_enabled() { return enabled; }

// 无法选择不可编辑的实体
void edit_set_editable(NativeEntity ent, bool editable) {
    if (editable)
        uneditable::pool->Remove(ent);
    else
        uneditable::pool->Remove(ent);
}
bool edit_get_editable(NativeEntity ent) { return !uneditable::pool->Get(ent); }

// --- bboxes --------------------------------------------------------------

void edit_bboxes_update(NativeEntity ent, BBox bbox) {
    BBoxPoolElem* elem;

    // editable?
    if (!edit_get_editable(ent)) return;

    elem = BBoxPoolElem::pool->Get(ent);

    // merge if already exists, else set
    if (elem)
        elem->bbox = bbox_merge(elem->bbox, bbox);
    else {
        elem = (BBoxPoolElem*)BBoxPoolElem::pool->Add(ent);
        elem->bbox = bbox;
    }
}

bool edit_bboxes_has(NativeEntity ent) { return BBoxPoolElem::pool->Get(ent) != NULL; }
BBox edit_bboxes_get(NativeEntity ent) {
    BBoxPoolElem* elem = (BBoxPoolElem*)BBoxPoolElem::pool->Get(ent);
    error_assert(elem);
    return elem->bbox;
}

int edit_bboxes_get_num() { return entitypool_size(BBoxPoolElem::pool); }

NativeEntity edit_bboxes_get_nth_ent(int n) {
    error_assert(n < entitypool_size(BBoxPoolElem::pool));
    BBoxPoolElem* elem = (BBoxPoolElem*)entitypool_nth(BBoxPoolElem::pool, n);
    return elem->pool_elem.ent;
}

BBox edit_bboxes_get_nth_bbox(int n) {
    error_assert(n < entitypool_size(BBoxPoolElem::pool));
    BBoxPoolElem* elem = entitypool_nth(BBoxPoolElem::pool, n);
    return elem->bbox;
}

void edit_bboxes_set_selected(NativeEntity ent, bool selected) {
    BBoxPoolElem* elem = (BBoxPoolElem*)BBoxPoolElem::pool->Get(ent);
    error_assert(elem);
    elem->selected = selected;
}

static Asset bboxes_shader = {};
static GLuint bboxes_vao;
static GLuint bboxes_vbo;

static void _bboxes_init() {
    BBoxPoolElem::pool = entitypool_new<BBoxPoolElem>();

    // 创建着色器程序 加载图集 绑定参数
    bool ok = asset_load_kind(AssetKind_Shader, "shader/bbox.glsl", &bboxes_shader);
    error_assert(ok);

    GLuint sid = bboxes_shader.shader.id;

    glUseProgram(sid);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &bboxes_vao);
    glBindVertexArray(bboxes_vao);
    glGenBuffers(1, &bboxes_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat1", BBoxPoolElem, wmat.v[0]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat2", BBoxPoolElem, wmat.v[3]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat3", BBoxPoolElem, wmat.v[6]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "bbmin", BBoxPoolElem, bbox.min);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "bbmax", BBoxPoolElem, bbox.max);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 1, "selected", BBoxPoolElem, selected);
}
static void _bboxes_fini() {
    // clean up GL stuff

    glDeleteBuffers(1, &bboxes_vbo);
    glDeleteVertexArrays(1, &bboxes_vao);

    entitypool_free(BBoxPoolElem::pool);
}

static void _bboxes_update_all() {
    NativeEntity ent;
    BBoxPoolElem* elem;
    static BBox defaultbb = {{-0.25, -0.25}, {0.25, 0.25}};

    if (!enabled) return;

    entitypool_foreach(elem, BBoxPoolElem::pool) {
        ent = elem->pool_elem.ent;
        if (!transform_has(ent)) continue;

        // update world matrix
        elem->wmat = transform_get_world_matrix(ent);

        // if no bbox, make default
        if (elem->bbox.max.x - elem->bbox.min.x <= NEKO_EPSILON || elem->bbox.max.y - elem->bbox.min.y <= NEKO_EPSILON) elem->bbox = defaultbb;
    }
}

// needs bbox shader program to be bound
static void _bboxes_draw_all() {
    vec2 win;
    unsigned int nbboxes;

    GLuint sid = bboxes_shader.shader.id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat*)camera_get_inverse_view_matrix_ptr());
    win = Neko::the<Game>().get_window_size();
    glUniform1f(glGetUniformLocation(sid, "aspect"), win.x / win.y);
    glUniform1f(glGetUniformLocation(sid, "is_grid"), 0);

    glBindVertexArray(bboxes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    nbboxes = entitypool_size(BBoxPoolElem::pool);
    glBufferData(GL_ARRAY_BUFFER, nbboxes * sizeof(BBoxPoolElem), entitypool_begin(BBoxPoolElem::pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nbboxes);
}

// --- grid ----------------------------------------------------------------

static vec2 grid_size = {1.0, 1.0};

// 每个维度上都是非负的 零意味着没有网格
void edit_set_grid_size(vec2 size) {
    if (size.x < 0.0) size.x = 0.0;
    if (size.y < 0.0) size.y = 0.0;

    grid_size = size;
}
vec2 edit_get_grid_size() { return grid_size; }

static Array<BBoxPoolElem> grid_cells;  // 用于绘制网格的bbox

static void _grid_create_cells() {
    BBox cbox, cellbox;
    NativeEntity camera;
    vec2 cur, csize;

    // find camera bounds in world space
    camera = camera_get_current_camera();
    if (native_entity_eq(camera, entity_nil))
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
    while (csize.x / cellbox.max.x > 70 || csize.y / cellbox.max.y > 70) cellbox.max = vec2_float_mul(cellbox.max, 2);

    // find lower grid snap for min
    if (grid_size.x > 0)
        cbox.min.x = cellbox.max.x * float_floor(cbox.min.x / cellbox.max.x);
    else
        cbox.min.x -= 0.5;
    if (grid_size.y > 0)
        cbox.min.y = cellbox.max.y * float_floor(cbox.min.y / cellbox.max.y);
    else
        cbox.min.y -= 0.5;

    // fill in with grid cells
    for (cur.x = cbox.min.x; cur.x < cbox.max.x; cur.x += cellbox.max.x)
        for (cur.y = cbox.min.y; cur.y < cbox.max.y; cur.y += cellbox.max.y) {
            grid_cells.push(BBoxPoolElem{.wmat = mat3_scaling_rotation_translation(luavec2(1, 1), 0, cur), .bbox = cellbox, .selected = 0});
        }
}

static void _grid_init() {}
static void _grid_fini() { grid_cells.trash(); }

static void _grid_draw() {
    vec2 win;
    unsigned int ncells;

    GLuint sid = bboxes_shader.shader.id;

    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat*)camera_get_inverse_view_matrix_ptr());
    win = Neko::the<Game>().get_window_size();
    glUniform1f(glGetUniformLocation(sid, "aspect"), win.x / win.y);
    glUniform1f(glGetUniformLocation(sid, "is_grid"), 1);

    _grid_create_cells();
    glBindVertexArray(bboxes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, bboxes_vbo);
    ncells = grid_cells.len;
    glBufferData(GL_ARRAY_BUFFER, ncells * sizeof(BBoxPoolElem), grid_cells.data, GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, ncells);
    grid_cells.resize(0);
}

// --- line ----------------------------------------------------------------

static Asset line_shader = {};
static GLuint line_vao;
static GLuint line_vbo;

typedef struct LinePoint LinePoint;
struct LinePoint {
    vec2 position;
    f32 point_size;
    Color color;
};

static Array<LinePoint> line_points;  // 每个连续的对都是一条线

void edit_line_add_xy(vec2 p, f32 point_size, Color color) { line_points.push(LinePoint{.position = p, .point_size = point_size, .color = color}); }

// 在两个世界空间坐标之间画一条线
void edit_line_add(vec2 a, vec2 b, f32 point_size, Color color) {
    edit_line_add_xy(a, point_size, color);
    edit_line_add_xy(b, point_size, color);
}

static void _line_init() {

    // init draw stuff
    bool ok = asset_load_kind(AssetKind_Shader, "shader/edit_line.glsl", &line_shader);
    error_assert(ok);

    GLuint sid = line_shader.shader.id;

    glUseProgram(sid);
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);
    glGenBuffers(1, &line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "position", LinePoint, position);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 1, "point_size", LinePoint, point_size);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 4, "color", LinePoint, color);
}
static void _line_fini() {
    // clean up draw stuff
    glDeleteBuffers(1, &line_vbo);
    glDeleteVertexArrays(1, &line_vao);

    line_points.trash();
}

static void _line_draw_all() {
    unsigned int npoints;

    const mat3* mat = camera_get_inverse_view_matrix_ptr();

    GLuint sid = line_shader.shader.id;

    // bind program, update uniforms
    glUseProgram(sid);
    GLuint inverse_view_matrix_id = glGetUniformLocation(sid, "inverse_view_matrix");
    glUniformMatrix3fv(inverse_view_matrix_id, 1, GL_FALSE, (const GLfloat*)mat);

    // draw!
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

    npoints = line_points.len;
    glBufferData(GL_ARRAY_BUFFER, npoints * sizeof(LinePoint), line_points.data, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, npoints);
    glDrawArrays(GL_POINTS, 0, npoints);
}

// -------------------------------------------------------------------------

int edit_clear(App* app, event_t evt) {
    entitypool_clear(BBoxPoolElem::pool);
    line_points.resize(0);
    return 0;
}

void edit_init() {
    PROFILE_FUNC();

    uneditable::pool = entitypool_new<uneditable>();

    _bboxes_init();
    _grid_init();
    _line_init();
}

void edit_fini() {
    _line_fini();
    _grid_fini();
    _bboxes_fini();

    entitypool_free(uneditable::pool);
}

static void _uneditable_remove(NativeEntity ent) { uneditable::pool->Remove(ent); }

int edit_update_all(App* app, event_t evt) {
    entitypool_remove_destroyed(uneditable::pool, _uneditable_remove);

    _bboxes_update_all();

    return 0;
}

void edit_draw_all() {
    if (!enabled) return;

    _bboxes_draw_all();
    _grid_draw();
    _line_draw_all();
}

void edit_save_all(App* app) {
    // Store *t, *elem_s;
    // EntityPoolElem *elem;

    // if (store_child_save(&t, "edit", s)) {
    //     vec2_save(&grid_size, "grid_size", t);
    //     entitypool_save_foreach(elem, elem_s, uneditable::pool, "uneditable_pool", t);
    // }
}
void edit_load_all(App* app) {
    // Store *t, *elem_s;
    // EntityPoolElem *elem;

    // if (store_child_load(&t, "edit", s)) {
    //     vec2_load(&grid_size, "grid_size", grid_size, t);
    //     entitypool_load_foreach(elem, elem_s, uneditable::pool, "uneditable_pool", t);
    // }
}
