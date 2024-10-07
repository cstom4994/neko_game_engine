
#include "engine/base/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"

static NativeEntity gui_root;  // 所有 gui 都应该是它的子节点 以便随屏幕移动

static NativeEntity focused;  // 当前聚焦的实体 如果没有则为entity_nil

static bool captured_event = false;

// --- common --------------------------------------------------------------

// 所有 GUI 系统共有的一般功能/数据

DECL_ENT(Gui, bool setvisible;  // 外部设置可见性
         bool visible;          // 内部递归计算可见性
         bool updated_visible;  // 用于递归可见性计算
         bool focusable;        // can be focused
         bool captures_events;

         Color color;

         BBox bbox;  // 在实体空间中
         GuiAlign halign; GuiAlign valign; vec2 padding;);

static NativeEntityPool *pool_gui;

static NativeEntityMap *focus_enter_map;
static NativeEntityMap *focus_exit_map;
static NativeEntityMap *changed_map;
static NativeEntityMap *mouse_down_map;
static NativeEntityMap *mouse_up_map;
static NativeEntityMap *key_down_map;
static NativeEntityMap *key_up_map;

NativeEntity gui_get_root() { return gui_root; }

void gui_add(NativeEntity ent) {
    Gui *gui;

    if (entitypool_get(pool_gui, ent)) return;  // 已经有gui

    transform_add(ent);

    gui = (Gui *)entitypool_add(pool_gui, ent);
    gui->visible = true;
    gui->setvisible = true;
    gui->focusable = false;
    gui->captures_events = true;
    gui->color = color_gray;
    gui->bbox = bbox(vec2_zero, luavec2(32, 32));
    gui->halign = GA_NONE;
    gui->valign = GA_NONE;
    gui->padding = luavec2(5, 5);
}

void gui_remove(NativeEntity ent) { entitypool_remove(pool_gui, ent); }

bool gui_has(NativeEntity ent) { return entitypool_get(pool_gui, ent) != NULL; }

void gui_set_color(NativeEntity ent, Color color) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->color = color;
}
Color gui_get_color(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->color;
}

void gui_set_visible(NativeEntity ent, bool visible) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->setvisible = visible;
}
bool gui_get_visible(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->visible;
}

void gui_set_focusable(NativeEntity ent, bool focusable) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->focusable = focusable;
}
bool gui_get_focusable(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->focusable;
}

void gui_set_captures_events(NativeEntity ent, bool captures_events) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->captures_events = captures_events;
}
bool gui_get_captures_events(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->captures_events;
}

void gui_set_halign(NativeEntity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->halign = align;
}
GuiAlign gui_get_halign(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->halign;
}
void gui_set_valign(NativeEntity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->valign = align;
}
GuiAlign gui_get_valign(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->valign;
}
void gui_set_padding(NativeEntity ent, vec2 padding) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    gui->padding = padding;
}
vec2 gui_get_padding(NativeEntity ent) {
    Gui *gui = (Gui *)entitypool_get(pool_gui, ent);
    error_assert(gui);
    return gui->padding;
}

void gui_set_focused_entity(NativeEntity ent) {
    if (native_entity_eq(focused, ent)) return;

    if (native_entity_eq(ent, entity_nil)) entitymap_set(focus_exit_map, focused, true);
    focused = ent;
    if (!native_entity_eq(focused, entity_nil)) entitymap_set(focus_enter_map, focused, true);
}
NativeEntity gui_get_focused_entity() { return focused; }
void gui_set_focus(NativeEntity ent, bool focus) {
    if (focus)
        gui_set_focused_entity(ent);
    else if (native_entity_eq(focused, ent))
        gui_set_focused_entity(entity_nil);
}
bool gui_get_focus(NativeEntity ent) { return native_entity_eq(focused, ent); }
bool gui_has_focus() { return !native_entity_eq(focused, entity_nil); }

void gui_fire_event_changed(NativeEntity ent) { entitymap_set(changed_map, ent, true); }

bool gui_event_focus_enter(NativeEntity ent) { return entitymap_get(focus_enter_map, ent); }
bool gui_event_focus_exit(NativeEntity ent) { return entitymap_get(focus_exit_map, ent); }
bool gui_event_changed(NativeEntity ent) { return entitymap_get(changed_map, ent); }
MouseCode gui_event_mouse_down(NativeEntity ent) { return (MouseCode)entitymap_get(mouse_down_map, ent); }
MouseCode gui_event_mouse_up(NativeEntity ent) { return (MouseCode)entitymap_get(mouse_up_map, ent); }
KeyCode gui_event_key_down(NativeEntity ent) { return (KeyCode)entitymap_get(key_down_map, ent); }
KeyCode gui_event_key_up(NativeEntity ent) { return (KeyCode)entitymap_get(key_up_map, ent); }

bool gui_captured_event() { return captured_event; }

static void _common_init() {
    pool_gui = entitypool_new(Gui);
    focus_enter_map = entitymap_new(false);
    focus_exit_map = entitymap_new(false);
    changed_map = entitymap_new(false);
    mouse_down_map = entitymap_new(MC_NONE);
    mouse_up_map = entitymap_new(MC_NONE);
    key_down_map = entitymap_new(KC_NONE);
    key_up_map = entitymap_new(KC_NONE);
}
static void _common_fini() {
    entitymap_free(key_up_map);
    entitymap_free(key_down_map);
    entitymap_free(mouse_up_map);
    entitymap_free(mouse_down_map);
    entitymap_free(changed_map);
    entitymap_free(focus_enter_map);
    entitymap_free(focus_exit_map);
    entitypool_free(pool_gui);
}

static void _common_update_destroyed() {
    if (entity_destroyed(focused)) focused = entity_nil;
    entitypool_remove_destroyed(pool_gui, gui_remove);
}

static void _common_update_visible_rec(Gui *gui) {
    Gui *pgui;

    if (gui->updated_visible) return;

    // false visibility takes priority
    if (!gui->setvisible) {
        gui->visible = false;
        gui->updated_visible = true;
        return;
    }

    // if has parent, inherit
    pgui = (Gui *)entitypool_get(pool_gui, transform_get_parent(gui->pool_elem.ent));
    if (pgui) {
        _common_update_visible_rec(pgui);
        gui->visible = pgui->visible;
        gui->updated_visible = true;
        return;
    }

    // else just set
    gui->visible = true;
    gui->updated_visible = true;
}
static void _common_update_visible() {
    Gui *gui;

    entitypool_foreach(gui, pool_gui) gui->updated_visible = false;
    entitypool_foreach(gui, pool_gui) _common_update_visible_rec(gui);
}

static void _common_align(Gui *gui, GuiAlign halign, GuiAlign valign) {
    Gui *pgui;
    BBox b, pb;
    vec2 pos;
    NativeEntity ent;
    Scalar mid, pmid;

    if (halign == GA_NONE && valign == GA_NONE) return;

    ent = gui->pool_elem.ent;

    // get parent-space bounding box and position
    b = bbox_transform(transform_get_matrix(ent), gui->bbox);
    pos = transform_get_position(ent);

    // get parent gui and its bounding box
    pgui = (Gui *)entitypool_get(pool_gui, transform_get_parent(ent));
    if (!pgui) return;
    pb = pgui->bbox;

    // macro to avoid repetition -- 'z' is vec2 axis member (x or y)
#define axis_align(align, z)                                       \
    switch (align) {                                               \
        case GA_MIN:                                               \
            pos.z = pb.min.z + gui->padding.z + pos.z - b.min.z;   \
            break;                                                 \
        case GA_MAX:                                               \
            pos.z = pb.max.z - gui->padding.z - (b.max.z - pos.z); \
            break;                                                 \
        case GA_MID:                                               \
            mid = 0.5 * (b.min.z + b.max.z);                       \
            pmid = 0.5 * (pb.min.z + pb.max.z);                    \
            pos.z = pmid - (mid - pos.z);                          \
            break;                                                 \
        default:                                                   \
            break;                                                 \
    }

    axis_align(halign, x);
    axis_align(valign, y);
    transform_set_position(ent, pos);
}

// move everything to top-left -- for fit calculations
static void _common_reset_align() {
    Gui *gui;
    entitypool_foreach(gui, pool_gui) _common_align(gui, gui->halign == GA_NONE ? GA_NONE : GA_MIN, gui->valign == GA_NONE ? GA_NONE : GA_MAX);
}

static void _common_update_align() {
    Gui *gui;
    entitypool_foreach(gui, pool_gui) _common_align(gui, gui->halign, gui->valign);
}

// attach root GUI entities to gui_root
static void _common_attach_root() {
    Gui *gui;
    NativeEntity ent;

    entitypool_foreach(gui, pool_gui) {
        ent = gui->pool_elem.ent;
        if (!native_entity_eq(ent, gui_root) && native_entity_eq(transform_get_parent(ent), entity_nil)) transform_set_parent(ent, gui_root);
    }
}

static void _common_update_all() {
    Gui *gui;

    _common_attach_root();

    // update edit bboxes
    if (edit_get_enabled()) entitypool_foreach(gui, pool_gui) edit_bboxes_update(gui->pool_elem.ent, gui->bbox);
}

// 'focus_clear' is whether to clear focus if click outside
static void _common_mouse_event(NativeEntityMap *emap, MouseCode mouse, bool focus_clear) {
    Gui *gui;
    vec2 m;
    mat3 t;
    NativeEntity ent;
    bool some_focused = false;

    m = camera_unit_to_world(input_get_mouse_pos_unit());
    entitypool_foreach(gui, pool_gui) if (gui->visible && !(edit_get_enabled() && edit_get_editable(gui->pool_elem.ent))) {
        ent = gui->pool_elem.ent;

        t = mat3_inverse(transform_get_world_matrix(ent));
        if (bbox_contains(gui->bbox, mat3_transform(t, m))) {
            entitymap_set(emap, ent, mouse);

            if (gui->captures_events) captured_event = true;

            // focus?
            if (gui->focusable && mouse == MC_LEFT) {
                some_focused = true;
                gui_set_focused_entity(ent);
            }
        }
    }

    // none focused? clear
    if (focus_clear && !some_focused) gui_set_focused_entity(entity_nil);
}
static void _common_mouse_down(MouseCode mouse) { _common_mouse_event(mouse_down_map, mouse, true); }
static void _common_mouse_up(MouseCode mouse) { _common_mouse_event(mouse_up_map, mouse, false); }

static void _common_key_down(KeyCode key) {
    if (!native_entity_eq(focused, entity_nil)) {
        entitymap_set(key_down_map, focused, key);
        captured_event = true;
    }
}
static void _common_key_up(KeyCode key) {
    if (!native_entity_eq(focused, entity_nil)) {
        entitymap_set(key_up_map, focused, key);
        captured_event = true;
    }
}
static void _common_char_down(unsigned int c) {
    if (!native_entity_eq(focused, entity_nil)) captured_event = true;
}

static void _common_event_clear() {
    entitymap_clear(focus_enter_map);
    entitymap_clear(focus_exit_map);
    entitymap_clear(changed_map);
    entitymap_clear(mouse_down_map);
    entitymap_clear(mouse_up_map);
    entitymap_clear(key_down_map);
    entitymap_clear(key_up_map);
    captured_event = false;
}

static void _common_save_all(Store *s) {
    Store *t, *gui_s;
    Gui *gui;

    if (store_child_save(&t, "gui", s)) entitypool_save_foreach(gui, gui_s, pool_gui, "pool", t) {
            color_save(&gui->color, "color", gui_s);
            bool_save(&gui->visible, "visible", gui_s);
            bool_save(&gui->setvisible, "setvisible", gui_s);
            bool_save(&gui->focusable, "focusable", gui_s);
            bool_save(&gui->captures_events, "captures_events", gui_s);
            enum_save(&gui->halign, "halign", gui_s);
            enum_save(&gui->valign, "valign", gui_s);
            vec2_save(&gui->padding, "padding", gui_s);
        }
}
static void _common_load_all(Store *s) {
    Store *t, *gui_s;
    Gui *gui;

    if (store_child_load(&t, "gui", s)) entitypool_load_foreach(gui, gui_s, pool_gui, "pool", t) {
            color_load(&gui->color, "color", color_gray, gui_s);
            bool_load(&gui->visible, "visible", true, gui_s);
            bool_load(&gui->setvisible, "setvisible", true, gui_s);
            bool_load(&gui->focusable, "focusable", false, gui_s);
            bool_load(&gui->captures_events, "captures_events", true, gui_s);
            enum_load(&gui->halign, "halign", GA_NONE, gui_s);
            enum_load(&gui->valign, "valign", GA_NONE, gui_s);
            vec2_load(&gui->padding, "padding", luavec2(5, 5), gui_s);
        }

    _common_attach_root();
}

// --- rect ----------------------------------------------------------------

typedef struct GuiRect GuiRect;
struct GuiRect {
    EntityPoolElem pool_elem;

    mat3 wmat;

    vec2 size;
    bool visible;
    Color color;

    bool hfit;
    bool vfit;
    bool hfill;
    bool vfill;

    bool updated;
    int depth;  // for draw order -- child depth > parent depth
};

static NativeEntityPool *rect_pool;

void gui_rect_add(NativeEntity ent) {
    GuiRect *rect;

    if (entitypool_get(rect_pool, ent)) return;

    gui_add(ent);

    rect = (GuiRect *)entitypool_add(rect_pool, ent);
    rect->size = luavec2(64, 64);
    rect->hfit = true;
    rect->vfit = true;
    rect->hfill = false;
    rect->vfill = false;
}
void gui_rect_remove(NativeEntity ent) { entitypool_remove(rect_pool, ent); }
bool gui_rect_has(NativeEntity ent) { return entitypool_get(rect_pool, ent) != NULL; }

void gui_rect_set_size(NativeEntity ent, vec2 size) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->size = size;
}
vec2 gui_rect_get_size(NativeEntity ent) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->size;
}

void gui_rect_set_hfit(NativeEntity ent, bool fit) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfit = fit;
}
bool gui_rect_get_hfit(NativeEntity ent) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfit;
}
void gui_rect_set_vfit(NativeEntity ent, bool fit) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfit = fit;
}
bool gui_rect_get_vfit(NativeEntity ent) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfit;
}

void gui_rect_set_hfill(NativeEntity ent, bool fill) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfill = fill;
}
bool gui_rect_get_hfill(NativeEntity ent) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfill;
}
void gui_rect_set_vfill(NativeEntity ent, bool fill) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfill = fill;
}
bool gui_rect_get_vfill(NativeEntity ent) {
    GuiRect *rect = (GuiRect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfill;
}

static Asset rect_shader = {};
static GLuint rect_vao;
static GLuint rect_vbo;

static void _rect_init() {
    // init pool
    rect_pool = entitypool_new(GuiRect);

    // create shader program, load texture, bind parameters
    bool ok = asset_load_kind(AssetKind_Shader, "shader/rect.glsl", &rect_shader);
    error_assert(ok);

    GLuint sid = rect_shader.shader.id;

    glUseProgram(sid);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat1", GuiRect, wmat.m[0]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat2", GuiRect, wmat.m[1]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 3, "wmat3", GuiRect, wmat.m[2]);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "size", GuiRect, size);
    gfx_bind_vertex_attrib(sid, GL_INT, 1, "visible", GuiRect, visible);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 4, "color", GuiRect, color);
}
static void _rect_fini() {
    // fini gl stuff
    glDeleteBuffers(1, &rect_vbo);
    glDeleteVertexArrays(1, &rect_vao);

    // fini pool
    entitypool_free(rect_pool);
}

static void _rect_update_child_first(NativeEntity ent);

static void _rect_update_table_align(GuiRect *rect) {
    NativeEntity rect_ent, *children;
    Gui *child;
    unsigned int nchildren, i;
    Scalar delta;
    BBox b;
    vec2 pos, curr;

    rect_ent = rect->pool_elem.ent;

    curr = vec2_zero;
    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui *)entitypool_get(pool_gui, children[i]);
        if (!(child && child->visible && (child->halign == GA_TABLE || child->valign == GA_TABLE))) continue;
        _rect_update_child_first(children[i]);

        b = bbox_transform(transform_get_matrix(children[i]), child->bbox);
        pos = transform_get_position(children[i]);

        if (child->halign == GA_TABLE) {
            delta = curr.x + child->padding.x - b.min.x;
            pos.x += delta;
            curr.x = b.max.x + delta;
        }
        if (child->valign == GA_TABLE) {
            delta = curr.y - child->padding.y - b.max.y;
            pos.y += delta;
            curr.y = b.min.y + delta;
        }

        transform_set_position(children[i], pos);
    }
}

static void _rect_update_fit(GuiRect *rect) {
    NativeEntity rect_ent, *children;
    Gui *child;
    unsigned int nchildren, i;
    Scalar miny, maxx;
    BBox b;

    rect_ent = rect->pool_elem.ent;

    miny = 0;
    maxx = 0;

    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui *)entitypool_get(pool_gui, children[i]);
        if (!child || !child->visible) continue;
        _rect_update_child_first(children[i]);

        b = bbox_transform(transform_get_matrix(children[i]), child->bbox);
        if (rect->hfit) maxx = scalar_max(maxx, b.max.x + child->padding.x);
        if (rect->vfit) miny = scalar_min(miny, b.min.y - child->padding.y);
    }

    if (rect->hfit) rect->size.x = maxx;
    if (rect->vfit) rect->size.y = -miny;
}

static void _rect_update_child_first(NativeEntity ent) {
    GuiRect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    rect = (GuiRect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_table_align(rect);
    _rect_update_fit(rect);

    gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));
}

static void _rect_update_parent_first(NativeEntity ent);

static void _rect_update_fill(GuiRect *rect) {
    NativeEntity ent;
    Gui *pgui, *gui;
    BBox b;
    NativeEntity parent;

    ent = rect->pool_elem.ent;
    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    if (!rect || !rect->visible || rect->updated || !(rect->hfill || rect->vfill)) return;

    parent = transform_get_parent(ent);
    pgui = (Gui *)entitypool_get(pool_gui, parent);
    if (!pgui) return;  // no parent to fill to

    _rect_update_parent_first(parent);
    b = bbox_transform(mat3_inverse(transform_get_matrix(ent)), pgui->bbox);

    if (rect->hfill) rect->size.x = b.max.x - gui->padding.x;
    if (rect->vfill) rect->size.y = -b.min.y + gui->padding.y;
}

static void _rect_update_depth(GuiRect *rect) {
    GuiRect *prect;

    prect = (GuiRect *)entitypool_get(rect_pool, transform_get_parent(rect->pool_elem.ent));
    if (prect) {
        _rect_update_parent_first(prect->pool_elem.ent);
        rect->depth = prect->depth + 1;
    } else
        rect->depth = 0;
}

static void _rect_update_parent_first(NativeEntity ent) {
    GuiRect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(pool_gui, ent);
    if (!gui) return;

    rect = (GuiRect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_fill(rect);
    _rect_update_depth(rect);

    gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));
}

static void _rect_update_all() {
    GuiRect *rect;
    Gui *gui;

    entitypool_remove_destroyed(rect_pool, gui_rect_remove);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_child_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_parent_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) {
        gui = (Gui *)entitypool_get(pool_gui, rect->pool_elem.ent);
        error_assert(gui);

        // write gui bbox
        gui->bbox = bbox_bound(vec2_zero, luavec2(rect->size.x, -rect->size.y));

        // read gui properties
        rect->visible = gui->visible;
        rect->color = gui->color;
    }
}

static void _rect_update_wmat() {
    GuiRect *rect;
    entitypool_foreach(rect, rect_pool) rect->wmat = transform_get_world_matrix(rect->pool_elem.ent);
}

static int _rect_depth_compare(const void *a, const void *b) {
    const GuiRect *ra = (GuiRect *)a, *rb = (GuiRect *)b;
    if (ra->depth == rb->depth) return ((int)ra->pool_elem.ent.id) - ((int)rb->pool_elem.ent.id);
    return ra->depth - rb->depth;
}

static void _rect_draw_all() {
    unsigned int nrects;

    // depth sort
    entitypool_sort(rect_pool, _rect_depth_compare);

    GLuint sid = rect_shader.shader.id;

    // bind shader program
    glUseProgram(sid);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // draw!
    glBindVertexArray(rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    nrects = entitypool_size(rect_pool);
    glBufferData(GL_ARRAY_BUFFER, nrects * sizeof(GuiRect), entitypool_begin(rect_pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nrects);
}

static void _rect_save_all(Store *s) {
    Store *t, *rect_s;
    GuiRect *rect;

    if (store_child_save(&t, "gui_rect", s)) entitypool_save_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_save(&rect->size, "size", rect_s);
            color_save(&rect->color, "color", rect_s);
            bool_save(&rect->hfit, "hfit", rect_s);
            bool_save(&rect->vfit, "vfit", rect_s);
            bool_save(&rect->hfill, "hfill", rect_s);
            bool_save(&rect->vfill, "vfill", rect_s);
        }
}
static void _rect_load_all(Store *s) {
    Store *t, *rect_s;
    GuiRect *rect;

    if (store_child_load(&t, "gui_rect", s)) entitypool_load_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_load(&rect->size, "size", luavec2(64, 64), rect_s);
            color_load(&rect->color, "color", color_gray, rect_s);
            bool_load(&rect->hfit, "hfit", true, rect_s);
            bool_load(&rect->vfit, "vfit", true, rect_s);
            bool_load(&rect->hfill, "hfill", false, rect_s);
            bool_load(&rect->vfill, "vfill", false, rect_s);
        }
}

// --- text ----------------------------------------------------------------

#define TEXT_GRID_W 16
#define TEXT_GRID_H 16

#define TEXT_FONT_W 10
#define TEXT_FONT_H 12

// info to send to shader program for each character
typedef struct TextChar TextChar;
struct TextChar {
    vec2 pos;         // position in space of text entity in size-less units
    vec2 cell;        // cell in font image
    float is_cursor;  // > 0 iff. this char is cursor
};

// info per text entity
typedef struct Text Text;
struct Text {
    EntityPoolElem pool_elem;

    char *str;
    CArray *chars;  // per-character info buffered to shader
    vec2 bounds;    // max x, min y in size-less units

    int cursor;
};

static NativeEntityPool *text_pool;

static Scalar cursor_blink_time = 0;

static void _text_add_cursor(Text *text, vec2 pos) {
    TextChar *tc;

    // compute position in font grid
    tc = (TextChar *)array_add(text->chars);
    tc->pos = pos;
    tc->cell = luavec2(' ' % TEXT_GRID_W, TEXT_GRID_H - 1 - (' ' / TEXT_GRID_W));
    tc->is_cursor = 1;
}

// just update with existing string if str is NULL
static void _text_set_str(Text *text, const char *str) {
    char c;
    TextChar *tc;
    vec2 pos;
    int i = 0;

    // copy to struct?
    if (str) {
        mem_free(text->str);
        text->str = (char *)mem_alloc(strlen(str) + 1);
        strcpy(text->str, str);
    } else
        str = text->str;

    // create TextChar array and update bounds
    pos = luavec2(0, -1);
    text->bounds = luavec2(1, -1);
    array_clear(text->chars);
    while (*str) {
        if (i++ == text->cursor) _text_add_cursor(text, pos);

        c = *str++;
        switch (c) {
            case '\n':
                // next line
                pos.x = 0;
                pos.y -= 1;
                continue;
        }

        // compute position in font grid
        tc = (TextChar *)array_add(text->chars);
        tc->pos = pos;
        tc->cell = luavec2(c % TEXT_GRID_W, TEXT_GRID_H - 1 - (c / TEXT_GRID_W));
        tc->is_cursor = -1;

        // move ahead
        pos.x += 1;
        text->bounds.x = scalar_max(text->bounds.x, pos.x);
    }

    // cursor at end?
    if (i == text->cursor) {
        _text_add_cursor(text, pos);
        pos.x += 1;
        text->bounds.x = scalar_max(text->bounds.x, pos.x);
    }

    text->bounds.y = pos.y;
}

void gui_text_add(NativeEntity ent) {
    Text *text;

    if (entitypool_get(text_pool, ent)) return;  // already has text

    gui_add(ent);

    text = (Text *)entitypool_add(text_pool, ent);
    text->chars = array_new(TextChar);
    text->str = NULL;  // _text_set_str(...) calls mem_free(text->str)
    text->cursor = -1;
    _text_set_str(text, "");
}
void gui_text_remove(NativeEntity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    if (text) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_remove(text_pool, ent);
}
bool gui_text_has(NativeEntity ent) { return entitypool_get(text_pool, ent) != NULL; }

void gui_text_set_str(NativeEntity ent, const char *str) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    _text_set_str(text, str);
}
const char *gui_text_get_str(NativeEntity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    return text->str;
}

static void _text_set_cursor(NativeEntity ent, int cursor) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    text->cursor = cursor;
    _text_set_str(text, NULL);
}

static Asset text_shader = {};
static GLuint text_vao;
static GLuint text_vbo;

static void _text_init() {
    // init pool
    text_pool = entitypool_new(Text);

    // create shader program, load texture, bind parameters
    bool ok = asset_load_kind(AssetKind_Shader, "shader/text.glsl", &text_shader);
    error_assert(ok);

    GLuint sid = text_shader.shader.id;

    glUseProgram(sid);

    asset_load(AssetLoadData{AssetKind_Image, true}, "assets/data/font1.png", NULL);

    glUniform1i(glGetUniformLocation(sid, "tex0"), 0);
    glUniform2f(glGetUniformLocation(sid, "inv_grid_size"), 1.0 / TEXT_GRID_W, 1.0 / TEXT_GRID_H);
    glUniform2f(glGetUniformLocation(sid, "size"), TEXT_FONT_W, TEXT_FONT_H);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);
    glGenBuffers(1, &text_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);
}
static void _text_fini() {
    Text *text;

    // fini gl stuff
    glDeleteBuffers(1, &text_vbo);
    glDeleteVertexArrays(1, &text_vao);

    // fini pool
    entitypool_foreach(text, text_pool) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_free(text_pool);
}

static void _text_update_all() {
    Text *text;
    Gui *gui;
    static vec2 size = {TEXT_FONT_W, TEXT_FONT_H};

    cursor_blink_time += 2 * get_timing_instance()->true_dt;

    entitypool_remove_destroyed(text_pool, gui_text_remove);

    entitypool_foreach(text, text_pool) {
        // blink on when focus entered
        if (gui_event_focus_enter(text->pool_elem.ent)) cursor_blink_time = 1;

        // gui bbox
        gui = (Gui *)entitypool_get(pool_gui, text->pool_elem.ent);
        error_assert(gui);
        gui->bbox = bbox_bound(vec2_zero, vec2_mul(size, text->bounds));
    }
}

static void _text_draw_all() {
    vec2 hwin;
    Text *text;
    Gui *gui;
    mat3 wmat;
    vec2 pos;
    unsigned int nchars;

    hwin = vec2_scalar_mul(game_get_window_size(), 0.5);

    GLuint sid = text_shader.shader.id;

    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);

    // TODO: 我不知道为什么引入neko_render后会有错误 在这里重新设置顶点属性指针可以修复
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(sid, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);

    // bind shader program
    glUseProgram(sid);
    // glUniform1i(glGetUniformLocation(sid, "tex0"), 1);
    glUniform1f(glGetUniformLocation(sid, "cursor_blink"), ((int)cursor_blink_time) & 1);
    glUniformMatrix3fv(glGetUniformLocation(sid, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    texture_bind("assets/data/font1.png");

    // draw!
    entitypool_foreach(text, text_pool) {
        gui = (Gui *)entitypool_get(pool_gui, text->pool_elem.ent);
        error_assert(gui);
        if (!gui->visible) continue;
        glUniform4fv(glGetUniformLocation(sid, "base_color"), 1, (const GLfloat *)&gui->color);

        wmat = transform_get_world_matrix(text->pool_elem.ent);
        glUniformMatrix3fv(glGetUniformLocation(sid, "wmat"), 1, GL_FALSE, (const GLfloat *)&wmat);

        nchars = array_length(text->chars);
        glBufferData(GL_ARRAY_BUFFER, nchars * sizeof(TextChar), array_begin(text->chars), GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, nchars);
    }

    // entitypool_foreach(text, text_pool) {
    //     gui = (Gui*)entitypool_get(gui_pool, text->pool_elem.ent);
    //     error_assert(gui);
    //     if (!gui->visible) continue;
    //     pos = transform_get_position(text->pool_elem.ent);
    //     const_str str = text->str;
    //     draw_font(&g_app->idraw, g_app->default_font, 16.f, pos.x, pos.y, str, NEKO_COLOR_WHITE);
    // }
}

static void _text_save_all(Store *s) {
    Store *t, *text_s;
    Text *text;

    if (store_child_save(&t, "gui_text", s)) entitypool_save_foreach(text, text_s, text_pool, "pool", t) {
            string_save((const char **)&text->str, "str", text_s);
            int_save(&text->cursor, "cursor", text_s);
        }
}
static void _text_load_all(Store *s) {
    Store *t, *text_s;
    Text *text;

    if (store_child_load(&t, "gui_text", s)) entitypool_load_foreach(text, text_s, text_pool, "pool", t) {
            text->chars = array_new(TextChar);
            string_load(&text->str, "str", "", text_s);
            int_load(&text->cursor, "cursor", -1, text_s);
            _text_set_str(text, NULL);
        }
}

// --- textedit ------------------------------------------------------------

typedef struct TextEdit TextEdit;
struct TextEdit {
    EntityPoolElem pool_elem;

    unsigned int cursor;  // 0 at beginning of string
    bool numerical;
};

NativeEntityPool *textedit_pool;

void gui_textedit_add(NativeEntity ent) {
    TextEdit *textedit;

    if (entitypool_get(textedit_pool, ent)) return;

    gui_text_add(ent);
    gui_set_focusable(ent, true);

    textedit = (TextEdit *)entitypool_add(textedit_pool, ent);
    textedit->cursor = 0;
    textedit->numerical = false;
}
void gui_textedit_remove(NativeEntity ent) { entitypool_remove(textedit_pool, ent); }
bool gui_textedit_has(NativeEntity ent) { return entitypool_get(textedit_pool, ent) != NULL; }

void gui_textedit_set_numerical(NativeEntity ent, bool numerical) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->numerical = numerical;
}
bool gui_textedit_get_numerical(NativeEntity ent) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->numerical;
}
Scalar gui_textedit_get_num(NativeEntity ent) { return strtof(gui_text_get_str(ent), NULL); }

static void _textedit_fix_cursor(TextEdit *textedit) {
    unsigned int len = strlen(gui_text_get_str(textedit->pool_elem.ent));
    if (textedit->cursor > len) textedit->cursor = len;
}

void gui_textedit_set_cursor(NativeEntity ent, unsigned int cursor) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->cursor = cursor;
    _textedit_fix_cursor(textedit);
}
unsigned int gui_textedit_get_cursor(NativeEntity ent) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->cursor;
}

static void _textedit_init() { textedit_pool = entitypool_new(TextEdit); }
static void _textedit_fini() { entitypool_free(textedit_pool); }

static bool _textedit_set_str(TextEdit *textedit, const char *str) {
    gui_text_set_str(textedit->pool_elem.ent, str);
    entitymap_set(changed_map, textedit->pool_elem.ent, true);
    return true;
}

// common function for key/char events
static void _textedit_key_event(KeyCode key, unsigned int c) {
    NativeEntity ent;
    TextEdit *textedit;
    const char *old;
    char *new_ptr = NULL;

    textedit = (TextEdit *)entitypool_get(textedit_pool, focused);
    if (!textedit) return;
    ent = textedit->pool_elem.ent;
    _textedit_fix_cursor(textedit);

    // blink on for feedback
    cursor_blink_time = 1;

    old = gui_text_get_str(ent);

    // confirm?
    if (key == KC_ENTER || key == KC_ESCAPE) {
        gui_set_focused_entity(entity_nil);
    }

    // left/right
    else if (key == KC_LEFT) {
        if (textedit->cursor > 0) --textedit->cursor;
    } else if (key == KC_RIGHT) {
        if (textedit->cursor < strlen(old)) ++textedit->cursor;
    }

    // remove char
    else if (key == KC_BACKSPACE || key == KC_DELETE) {
        if (key == KC_BACKSPACE)
            if (textedit->cursor > 0) --textedit->cursor;

        new_ptr = (char *)mem_alloc(strlen(old));  // 1 less, but 1 more for null
        strncpy(new_ptr, old, textedit->cursor);
        strcpy(&new_ptr[textedit->cursor], &old[textedit->cursor + 1]);
        _textedit_set_str(textedit, new_ptr);
    }

    // insert char
    else if (isprint(c)) {
        new_ptr = (char *)mem_alloc(strlen(old) + 2);  // 1 for new_ptr char, 1 for null
        strncpy(new_ptr, old, textedit->cursor);
        new_ptr[textedit->cursor] = (char)c;
        strcpy(&new_ptr[textedit->cursor + 1], &old[textedit->cursor]);
        if (_textedit_set_str(textedit, new_ptr)) ++textedit->cursor;
    }

    mem_free(new_ptr);
}

static void _textedit_char_down(unsigned int c) { _textedit_key_event(KC_NONE, c); }

static void _textedit_key_down(KeyCode key) { _textedit_key_event(key, 0); }

static void _textedit_update_all() {
    NativeEntity ent;
    TextEdit *textedit;

    entitypool_remove_destroyed(textedit_pool, gui_textedit_remove);

    entitypool_foreach(textedit, textedit_pool) {
        ent = textedit->pool_elem.ent;
        _textedit_fix_cursor(textedit);

        // focus stuff
        if (gui_get_focus(ent))
            _text_set_cursor(ent, textedit->cursor);
        else
            _text_set_cursor(ent, -1);
    }
}

static void _textedit_save_all(Store *s) {
    Store *t, *textedit_s;
    TextEdit *textedit;

    if (store_child_save(&t, "gui_textedit", s)) entitypool_save_foreach(textedit, textedit_s, textedit_pool, "pool", t) {
            uint_save(&textedit->cursor, "cursor", textedit_s);
            bool_save(&textedit->numerical, "numerical", textedit_s);
        }
}
static void _textedit_load_all(Store *s) {
    Store *t, *textedit_s;
    TextEdit *textedit;

    if (store_child_load(&t, "gui_textedit", s)) entitypool_load_foreach(textedit, textedit_s, textedit_pool, "pool", t) {
            uint_load(&textedit->cursor, "cursor", 0, textedit_s);
            bool_load(&textedit->numerical, "numerical", false, textedit_s);
        }
}

// -------------------------------------------------------------------------

int gui_event_clear(App *app, event_t evt) {
    _common_event_clear();
    return 0;
}

static void _create_root() {
    gui_root = entity_create();
    transform_add(gui_root);
    transform_set_position(gui_root, luavec2(-1, 1));  // origin at top-left
    gui_rect_add(gui_root);
    gui_rect_set_hfit(gui_root, false);
    gui_rect_set_vfit(gui_root, false);
    gui_set_color(gui_root, color_clear);
    gui_set_captures_events(gui_root, false);
}

void gui_init() {
    PROFILE_FUNC();

    focused = entity_nil;
    _common_init();
    _rect_init();
    _text_init();
    _textedit_init();
    _create_root();
}

void gui_fini() {
    _textedit_fini();
    _text_fini();
    _rect_fini();
    _common_fini();
}

static void _update_root() {
    vec2 win_size;

    win_size = game_get_window_size();

    edit_set_editable(gui_root, false);

    // child of camera so GUI stays on screen
    transform_set_parent(gui_root, camera_get_current_camera());

    // use pixel coordinates
    transform_set_scale(gui_root, scalar_vec2_div(2, win_size));
    gui_rect_set_size(gui_root, win_size);
}

int gui_update_all(App *app, event_t evt) {
    _update_root();
    _common_update_destroyed();
    _common_update_visible();
    _common_reset_align();
    _textedit_update_all();
    _text_update_all();
    _rect_update_all();
    _common_update_align();
    _rect_update_wmat();
    _common_update_all();

    return 0;
}

void gui_draw_all() {
    _rect_draw_all();
    _text_draw_all();
}

void gui_key_down(KeyCode key) {
    _common_key_down(key);
    _textedit_key_down(key);
}
void gui_char_down(unsigned int c) {
    _common_char_down(c);
    _textedit_char_down(c);
}
void gui_key_up(KeyCode key) { _common_key_up(key); }
void gui_mouse_down(MouseCode mouse) { _common_mouse_down(mouse); }
void gui_mouse_up(MouseCode mouse) { _common_mouse_up(mouse); }

void gui_save_all(Store *s) {
    _common_save_all(s);
    _rect_save_all(s);
    _text_save_all(s);
    _textedit_save_all(s);
}
void gui_load_all(Store *s) {
    _common_load_all(s);
    _rect_load_all(s);
    _text_load_all(s);
    _textedit_load_all(s);
}
