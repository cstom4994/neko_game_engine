#include "engine/ui.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit.h"
#include "engine/api.hpp"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/ecs.h"
#include "engine/game.h"
#include "engine/luax.h"
#include "engine/os.h"
#include "engine/prelude.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "gfx.h"

// imgui
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static Entity gui_root;  // 所有 gui 都应该是它的子节点 以便随屏幕移动

static Entity focused;  // 当前聚焦的实体 如果没有则为entity_nil

static bool captured_event = false;

// --- common --------------------------------------------------------------

// 所有 GUI 系统共有的一般功能/数据

typedef struct Gui Gui;
struct Gui {
    EntityPoolElem pool_elem;

    bool setvisible;       // 外部设置可见性
    bool visible;          // 内部递归计算可见性
    bool updated_visible;  // 用于递归可见性计算
    bool focusable;        // can be focused
    bool captures_events;

    Color color;

    BBox bbox;  // 在实体空间中
    GuiAlign halign;
    GuiAlign valign;
    CVec2 padding;
};

static EntityPool *gui_pool;

static EntityMap *focus_enter_map;
static EntityMap *focus_exit_map;
static EntityMap *changed_map;
static EntityMap *mouse_down_map;
static EntityMap *mouse_up_map;
static EntityMap *key_down_map;
static EntityMap *key_up_map;

Entity gui_get_root() { return gui_root; }

void gui_add(Entity ent) {
    Gui *gui;

    if (entitypool_get(gui_pool, ent)) return;  // 已经有gui

    transform_add(ent);

    gui = (Gui *)entitypool_add(gui_pool, ent);
    gui->visible = true;
    gui->setvisible = true;
    gui->focusable = false;
    gui->captures_events = true;
    gui->color = color_gray;
    gui->bbox = bbox(vec2_zero, vec2(32, 32));
    gui->halign = GA_NONE;
    gui->valign = GA_NONE;
    gui->padding = vec2(5, 5);
}

void gui_remove(Entity ent) { entitypool_remove(gui_pool, ent); }

bool gui_has(Entity ent) { return entitypool_get(gui_pool, ent) != NULL; }

void gui_set_color(Entity ent, Color color) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->color = color;
}
Color gui_get_color(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->color;
}

void gui_set_visible(Entity ent, bool visible) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->setvisible = visible;
}
bool gui_get_visible(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->visible;
}

void gui_set_focusable(Entity ent, bool focusable) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->focusable = focusable;
}
bool gui_get_focusable(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->focusable;
}

void gui_set_captures_events(Entity ent, bool captures_events) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->captures_events = captures_events;
}
bool gui_get_captures_events(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->captures_events;
}

void gui_set_halign(Entity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->halign = align;
}
GuiAlign gui_get_halign(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->halign;
}
void gui_set_valign(Entity ent, GuiAlign align) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->valign = align;
}
GuiAlign gui_get_valign(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->valign;
}
void gui_set_padding(Entity ent, CVec2 padding) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->padding = padding;
}
CVec2 gui_get_padding(Entity ent) {
    Gui *gui = (Gui *)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->padding;
}

void gui_set_focused_entity(Entity ent) {
    if (entity_eq(focused, ent)) return;

    if (entity_eq(ent, entity_nil)) entitymap_set(focus_exit_map, focused, true);
    focused = ent;
    if (!entity_eq(focused, entity_nil)) entitymap_set(focus_enter_map, focused, true);
}
Entity gui_get_focused_entity() { return focused; }
void gui_set_focus(Entity ent, bool focus) {
    if (focus)
        gui_set_focused_entity(ent);
    else if (entity_eq(focused, ent))
        gui_set_focused_entity(entity_nil);
}
bool gui_get_focus(Entity ent) { return entity_eq(focused, ent); }
bool gui_has_focus() { return !entity_eq(focused, entity_nil); }

void gui_fire_event_changed(Entity ent) { entitymap_set(changed_map, ent, true); }

bool gui_event_focus_enter(Entity ent) { return entitymap_get(focus_enter_map, ent); }
bool gui_event_focus_exit(Entity ent) { return entitymap_get(focus_exit_map, ent); }
bool gui_event_changed(Entity ent) { return entitymap_get(changed_map, ent); }
MouseCode gui_event_mouse_down(Entity ent) { return (MouseCode)entitymap_get(mouse_down_map, ent); }
MouseCode gui_event_mouse_up(Entity ent) { return (MouseCode)entitymap_get(mouse_up_map, ent); }
KeyCode gui_event_key_down(Entity ent) { return (KeyCode)entitymap_get(key_down_map, ent); }
KeyCode gui_event_key_up(Entity ent) { return (KeyCode)entitymap_get(key_up_map, ent); }

bool gui_captured_event() { return captured_event; }

static void _common_init() {
    gui_pool = entitypool_new(Gui);
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
    entitypool_free(gui_pool);
}

static void _common_update_destroyed() {
    if (entity_destroyed(focused)) focused = entity_nil;
    entitypool_remove_destroyed(gui_pool, gui_remove);
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
    pgui = (Gui *)entitypool_get(gui_pool, transform_get_parent(gui->pool_elem.ent));
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

    entitypool_foreach(gui, gui_pool) gui->updated_visible = false;
    entitypool_foreach(gui, gui_pool) _common_update_visible_rec(gui);
}

static void _common_align(Gui *gui, GuiAlign halign, GuiAlign valign) {
    Gui *pgui;
    BBox b, pb;
    CVec2 pos;
    Entity ent;
    Scalar mid, pmid;

    if (halign == GA_NONE && valign == GA_NONE) return;

    ent = gui->pool_elem.ent;

    // get parent-space bounding box and position
    b = bbox_transform(transform_get_matrix(ent), gui->bbox);
    pos = transform_get_position(ent);

    // get parent gui and its bounding box
    pgui = (Gui *)entitypool_get(gui_pool, transform_get_parent(ent));
    if (!pgui) return;
    pb = pgui->bbox;

    // macro to avoid repetition -- 'z' is CVec2 axis member (x or y)
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
    entitypool_foreach(gui, gui_pool) _common_align(gui, gui->halign == GA_NONE ? GA_NONE : GA_MIN, gui->valign == GA_NONE ? GA_NONE : GA_MAX);
}

static void _common_update_align() {
    Gui *gui;
    entitypool_foreach(gui, gui_pool) _common_align(gui, gui->halign, gui->valign);
}

// attach root GUI entities to gui_root
static void _common_attach_root() {
    Gui *gui;
    Entity ent;

    entitypool_foreach(gui, gui_pool) {
        ent = gui->pool_elem.ent;
        if (!entity_eq(ent, gui_root) && entity_eq(transform_get_parent(ent), entity_nil)) transform_set_parent(ent, gui_root);
    }
}

static void _common_update_all() {
    Gui *gui;

    _common_attach_root();

    // update edit bboxes
    if (edit_get_enabled()) entitypool_foreach(gui, gui_pool) edit_bboxes_update(gui->pool_elem.ent, gui->bbox);
}

// 'focus_clear' is whether to clear focus if click outside
static void _common_mouse_event(EntityMap *emap, MouseCode mouse, bool focus_clear) {
    Gui *gui;
    CVec2 m;
    CMat3 t;
    Entity ent;
    bool some_focused = false;

    m = camera_unit_to_world(input_get_mouse_pos_unit());
    entitypool_foreach(gui, gui_pool) if (gui->visible && !(edit_get_enabled() && edit_get_editable(gui->pool_elem.ent))) {
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
    if (!entity_eq(focused, entity_nil)) {
        entitymap_set(key_down_map, focused, key);
        captured_event = true;
    }
}
static void _common_key_up(KeyCode key) {
    if (!entity_eq(focused, entity_nil)) {
        entitymap_set(key_up_map, focused, key);
        captured_event = true;
    }
}
static void _common_char_down(unsigned int c) {
    if (!entity_eq(focused, entity_nil)) captured_event = true;
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

    if (store_child_save(&t, "gui", s)) entitypool_save_foreach(gui, gui_s, gui_pool, "pool", t) {
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

    if (store_child_load(&t, "gui", s)) entitypool_load_foreach(gui, gui_s, gui_pool, "pool", t) {
            color_load(&gui->color, "color", color_gray, gui_s);
            bool_load(&gui->visible, "visible", true, gui_s);
            bool_load(&gui->setvisible, "setvisible", true, gui_s);
            bool_load(&gui->focusable, "focusable", false, gui_s);
            bool_load(&gui->captures_events, "captures_events", true, gui_s);
            enum_load(&gui->halign, "halign", GA_NONE, gui_s);
            enum_load(&gui->valign, "valign", GA_NONE, gui_s);
            vec2_load(&gui->padding, "padding", vec2(5, 5), gui_s);
        }

    _common_attach_root();
}

// --- rect ----------------------------------------------------------------

typedef struct Rect Rect;
struct Rect {
    EntityPoolElem pool_elem;

    CMat3 wmat;

    CVec2 size;
    bool visible;
    Color color;

    bool hfit;
    bool vfit;
    bool hfill;
    bool vfill;

    bool updated;
    int depth;  // for draw order -- child depth > parent depth
};

static EntityPool *rect_pool;

void gui_rect_add(Entity ent) {
    Rect *rect;

    if (entitypool_get(rect_pool, ent)) return;

    gui_add(ent);

    rect = (Rect *)entitypool_add(rect_pool, ent);
    rect->size = vec2(64, 64);
    rect->hfit = true;
    rect->vfit = true;
    rect->hfill = false;
    rect->vfill = false;
}
void gui_rect_remove(Entity ent) { entitypool_remove(rect_pool, ent); }
bool gui_rect_has(Entity ent) { return entitypool_get(rect_pool, ent) != NULL; }

void gui_rect_set_size(Entity ent, CVec2 size) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->size = size;
}
CVec2 gui_rect_get_size(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->size;
}

void gui_rect_set_hfit(Entity ent, bool fit) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfit = fit;
}
bool gui_rect_get_hfit(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfit;
}
void gui_rect_set_vfit(Entity ent, bool fit) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfit = fit;
}
bool gui_rect_get_vfit(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfit;
}

void gui_rect_set_hfill(Entity ent, bool fill) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfill = fill;
}
bool gui_rect_get_hfill(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfill;
}
void gui_rect_set_vfill(Entity ent, bool fill) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfill = fill;
}
bool gui_rect_get_vfill(Entity ent) {
    Rect *rect = (Rect *)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfill;
}

static GLuint rect_program;
static GLuint rect_vao;
static GLuint rect_vbo;

static void _rect_init() {
    // init pool
    rect_pool = entitypool_new(Rect);

    // create shader program, load texture, bind parameters
    rect_program = gfx_create_program("rect_program", "shader/rect.vert", "shader/rect.geom", "shader/rect.frag");
    glUseProgram(rect_program);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &rect_vao);
    glBindVertexArray(rect_vao);
    glGenBuffers(1, &rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat1", Rect, wmat.m[0]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat2", Rect, wmat.m[1]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 3, "wmat3", Rect, wmat.m[2]);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 2, "size", Rect, size);
    gfx_bind_vertex_attrib(rect_program, GL_INT, 1, "visible", Rect, visible);
    gfx_bind_vertex_attrib(rect_program, GL_FLOAT, 4, "color", Rect, color);
}
static void _rect_fini() {
    // fini gl stuff
    glDeleteProgram(rect_program);
    glDeleteBuffers(1, &rect_vbo);
    glDeleteVertexArrays(1, &rect_vao);

    // fini pool
    entitypool_free(rect_pool);
}

static void _rect_update_child_first(Entity ent);

static void _rect_update_table_align(Rect *rect) {
    Entity rect_ent, *children;
    Gui *child;
    unsigned int nchildren, i;
    Scalar delta;
    BBox b;
    CVec2 pos, curr;

    rect_ent = rect->pool_elem.ent;

    curr = vec2_zero;
    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui *)entitypool_get(gui_pool, children[i]);
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

static void _rect_update_fit(Rect *rect) {
    Entity rect_ent, *children;
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
        child = (Gui *)entitypool_get(gui_pool, children[i]);
        if (!child || !child->visible) continue;
        _rect_update_child_first(children[i]);

        b = bbox_transform(transform_get_matrix(children[i]), child->bbox);
        if (rect->hfit) maxx = scalar_max(maxx, b.max.x + child->padding.x);
        if (rect->vfit) miny = scalar_min(miny, b.min.y - child->padding.y);
    }

    if (rect->hfit) rect->size.x = maxx;
    if (rect->vfit) rect->size.y = -miny;
}

static void _rect_update_child_first(Entity ent) {
    Rect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(gui_pool, ent);
    if (!gui) return;

    rect = (Rect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_table_align(rect);
    _rect_update_fit(rect);

    gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));
}

static void _rect_update_parent_first(Entity ent);

static void _rect_update_fill(Rect *rect) {
    Entity ent;
    Gui *pgui, *gui;
    BBox b;
    Entity parent;

    ent = rect->pool_elem.ent;
    gui = (Gui *)entitypool_get(gui_pool, ent);
    if (!gui) return;

    if (!rect || !rect->visible || rect->updated || !(rect->hfill || rect->vfill)) return;

    parent = transform_get_parent(ent);
    pgui = (Gui *)entitypool_get(gui_pool, parent);
    if (!pgui) return;  // no parent to fill to

    _rect_update_parent_first(parent);
    b = bbox_transform(mat3_inverse(transform_get_matrix(ent)), pgui->bbox);

    if (rect->hfill) rect->size.x = b.max.x - gui->padding.x;
    if (rect->vfill) rect->size.y = -b.min.y + gui->padding.y;
}

static void _rect_update_depth(Rect *rect) {
    Rect *prect;

    prect = (Rect *)entitypool_get(rect_pool, transform_get_parent(rect->pool_elem.ent));
    if (prect) {
        _rect_update_parent_first(prect->pool_elem.ent);
        rect->depth = prect->depth + 1;
    } else
        rect->depth = 0;
}

static void _rect_update_parent_first(Entity ent) {
    Rect *rect;
    Gui *gui;

    gui = (Gui *)entitypool_get(gui_pool, ent);
    if (!gui) return;

    rect = (Rect *)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_fill(rect);
    _rect_update_depth(rect);

    gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));
}

static void _rect_update_all() {
    Rect *rect;
    Gui *gui;

    entitypool_remove_destroyed(rect_pool, gui_rect_remove);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_child_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_parent_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) {
        gui = (Gui *)entitypool_get(gui_pool, rect->pool_elem.ent);
        error_assert(gui);

        // write gui bbox
        gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));

        // read gui properties
        rect->visible = gui->visible;
        rect->color = gui->color;
    }
}

static void _rect_update_wmat() {
    Rect *rect;
    entitypool_foreach(rect, rect_pool) rect->wmat = transform_get_world_matrix(rect->pool_elem.ent);
}

static int _rect_depth_compare(const void *a, const void *b) {
    const Rect *ra = (Rect *)a, *rb = (Rect *)b;
    if (ra->depth == rb->depth) return ((int)ra->pool_elem.ent.id) - ((int)rb->pool_elem.ent.id);
    return ra->depth - rb->depth;
}

static void _rect_draw_all() {
    unsigned int nrects;

    // depth sort
    entitypool_sort(rect_pool, _rect_depth_compare);

    // bind shader program
    glUseProgram(rect_program);
    glUniformMatrix3fv(glGetUniformLocation(rect_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // draw!
    glBindVertexArray(rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    nrects = entitypool_size(rect_pool);
    glBufferData(GL_ARRAY_BUFFER, nrects * sizeof(Rect), entitypool_begin(rect_pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nrects);
}

static void _rect_save_all(Store *s) {
    Store *t, *rect_s;
    Rect *rect;

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
    Rect *rect;

    if (store_child_load(&t, "gui_rect", s)) entitypool_load_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_load(&rect->size, "size", vec2(64, 64), rect_s);
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
    CVec2 pos;        // position in space of text entity in size-less units
    CVec2 cell;       // cell in font image
    float is_cursor;  // > 0 iff. this char is cursor
};

// info per text entity
typedef struct Text Text;
struct Text {
    EntityPoolElem pool_elem;

    char *str;
    CArray *chars;  // per-character info buffered to shader
    CVec2 bounds;   // max x, min y in size-less units

    int cursor;
};

static EntityPool *text_pool;

static Scalar cursor_blink_time = 0;

static void _text_add_cursor(Text *text, CVec2 pos) {
    TextChar *tc;

    // compute position in font grid
    tc = (TextChar *)array_add(text->chars);
    tc->pos = pos;
    tc->cell = vec2(' ' % TEXT_GRID_W, TEXT_GRID_H - 1 - (' ' / TEXT_GRID_W));
    tc->is_cursor = 1;
}

// just update with existing string if str is NULL
static void _text_set_str(Text *text, const char *str) {
    char c;
    TextChar *tc;
    CVec2 pos;
    int i = 0;

    // copy to struct?
    if (str) {
        mem_free(text->str);
        text->str = (char *)mem_alloc(strlen(str) + 1);
        strcpy(text->str, str);
    } else
        str = text->str;

    // create TextChar array and update bounds
    pos = vec2(0, -1);
    text->bounds = vec2(1, -1);
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
        tc->cell = vec2(c % TEXT_GRID_W, TEXT_GRID_H - 1 - (c / TEXT_GRID_W));
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

void gui_text_add(Entity ent) {
    Text *text;

    if (entitypool_get(text_pool, ent)) return;  // already has text

    gui_add(ent);

    text = (Text *)entitypool_add(text_pool, ent);
    text->chars = array_new(TextChar);
    text->str = NULL;  // _text_set_str(...) calls mem_free(text->str)
    text->cursor = -1;
    _text_set_str(text, "");
}
void gui_text_remove(Entity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    if (text) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_remove(text_pool, ent);
}
bool gui_text_has(Entity ent) { return entitypool_get(text_pool, ent) != NULL; }

void gui_text_set_str(Entity ent, const char *str) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    _text_set_str(text, str);
}
const char *gui_text_get_str(Entity ent) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    return text->str;
}

static void _text_set_cursor(Entity ent, int cursor) {
    Text *text = (Text *)entitypool_get(text_pool, ent);
    error_assert(text);
    text->cursor = cursor;
    _text_set_str(text, NULL);
}

static GLuint text_program;
static GLuint text_vao;
static GLuint text_vbo;

static void _text_init() {
    // init pool
    text_pool = entitypool_new(Text);

    // create shader program, load texture, bind parameters
    text_program = gfx_create_program("text_program", "shader/text.vert", "shader/text.geom", "shader/text.frag");
    glUseProgram(text_program);

    asset_load(AssetLoadData{AssetKind_Image, true}, "assets/data/font1.png", NULL);

    glUniform1i(glGetUniformLocation(text_program, "tex0"), 0);
    glUniform2f(glGetUniformLocation(text_program, "inv_grid_size"), 1.0 / TEXT_GRID_W, 1.0 / TEXT_GRID_H);
    glUniform2f(glGetUniformLocation(text_program, "size"), TEXT_FONT_W, TEXT_FONT_H);

    // make vao, vbo, bind attributes
    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);
    glGenBuffers(1, &text_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);
}
static void _text_fini() {
    Text *text;

    // fini gl stuff
    glDeleteProgram(text_program);
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
    static CVec2 size = {TEXT_FONT_W, TEXT_FONT_H};

    cursor_blink_time += 2 * timing_instance.true_dt;

    entitypool_remove_destroyed(text_pool, gui_text_remove);

    entitypool_foreach(text, text_pool) {
        // blink on when focus entered
        if (gui_event_focus_enter(text->pool_elem.ent)) cursor_blink_time = 1;

        // gui bbox
        gui = (Gui *)entitypool_get(gui_pool, text->pool_elem.ent);
        error_assert(gui);
        gui->bbox = bbox_bound(vec2_zero, vec2_mul(size, text->bounds));
    }
}

void ME_draw_text(String text, Color256 col, int x, int y, bool outline, Color256 outline_col) {

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *draw_list = ImGui::GetBackgroundDrawList(viewport);

    if (outline) {

        auto outline_col_im = ImColor(outline_col.r, outline_col.g, outline_col.b, col.a);

        draw_list->AddText(ImVec2(x + 0, y - 1), outline_col_im, text.cstr());  // up
        draw_list->AddText(ImVec2(x + 0, y + 1), outline_col_im, text.cstr());  // down
        draw_list->AddText(ImVec2(x + 1, y + 0), outline_col_im, text.cstr());  // right
        draw_list->AddText(ImVec2(x - 1, y + 0), outline_col_im, text.cstr());  // left

        draw_list->AddText(ImVec2(x + 1, y + 1), outline_col_im, text.cstr());  // down-right
        draw_list->AddText(ImVec2(x - 1, y + 1), outline_col_im, text.cstr());  // down-left

        draw_list->AddText(ImVec2(x + 1, y - 1), outline_col_im, text.cstr());  // up-right
        draw_list->AddText(ImVec2(x - 1, y - 1), outline_col_im, text.cstr());  // up-left
    }

    draw_list->AddText(ImVec2(x, y), ImColor(col.r, col.g, col.b, col.a), text.cstr());  // base
}

static void _text_draw_all() {
    CVec2 hwin;
    Text *text;
    Gui *gui;
    CMat3 wmat;
    unsigned int nchars;

    hwin = vec2_scalar_mul(game_get_window_size(), 0.5);

    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);

    // TODO: 我不知道为什么引入neko_render后会有错误 在这里重新设置顶点属性指针可以修复
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "pos", TextChar, pos);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 2, "cell", TextChar, cell);
    gfx_bind_vertex_attrib(text_program, GL_FLOAT, 1, "is_cursor", TextChar, is_cursor);

    // bind shader program
    glUseProgram(text_program);
    // glUniform1i(glGetUniformLocation(text_program, "tex0"), 1);
    glUniform1f(glGetUniformLocation(text_program, "cursor_blink"), ((int)cursor_blink_time) & 1);
    glUniformMatrix3fv(glGetUniformLocation(text_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat *)camera_get_inverse_view_matrix_ptr());

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    texture_bind("assets/data/font1.png");

    // draw!
    entitypool_foreach(text, text_pool) {
        gui = (Gui *)entitypool_get(gui_pool, text->pool_elem.ent);
        error_assert(gui);
        if (!gui->visible) continue;
        glUniform4fv(glGetUniformLocation(text_program, "base_color"), 1, (const GLfloat *)&gui->color);

        wmat = transform_get_world_matrix(text->pool_elem.ent);
        glUniformMatrix3fv(glGetUniformLocation(text_program, "wmat"), 1, GL_FALSE, (const GLfloat *)&wmat);

        nchars = array_length(text->chars);
        glBufferData(GL_ARRAY_BUFFER, nchars * sizeof(TextChar), array_begin(text->chars), GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, nchars);

        CVec2 text_pos = transform_get_world_position(text->pool_elem.ent);
        ME_draw_text(text->str, NEKO_COLOR_WHITE, text_pos.x, text_pos.y, false, NEKO_COLOR_WHITE);
    }
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

EntityPool *textedit_pool;

void gui_textedit_add(Entity ent) {
    TextEdit *textedit;

    if (entitypool_get(textedit_pool, ent)) return;

    gui_text_add(ent);
    gui_set_focusable(ent, true);

    textedit = (TextEdit *)entitypool_add(textedit_pool, ent);
    textedit->cursor = 0;
    textedit->numerical = false;
}
void gui_textedit_remove(Entity ent) { entitypool_remove(textedit_pool, ent); }
bool gui_textedit_has(Entity ent) { return entitypool_get(textedit_pool, ent) != NULL; }

void gui_textedit_set_numerical(Entity ent, bool numerical) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->numerical = numerical;
}
bool gui_textedit_get_numerical(Entity ent) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->numerical;
}
Scalar gui_textedit_get_num(Entity ent) { return strtof(gui_text_get_str(ent), NULL); }

static void _textedit_fix_cursor(TextEdit *textedit) {
    unsigned int len = strlen(gui_text_get_str(textedit->pool_elem.ent));
    if (textedit->cursor > len) textedit->cursor = len;
}

void gui_textedit_set_cursor(Entity ent, unsigned int cursor) {
    TextEdit *textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    textedit = (TextEdit *)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->cursor = cursor;
    _textedit_fix_cursor(textedit);
}
unsigned int gui_textedit_get_cursor(Entity ent) {
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
    Entity ent;
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
    Entity ent;
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

void gui_event_clear() { _common_event_clear(); }

static void _create_root() {
    gui_root = entity_create();
    transform_add(gui_root);
    transform_set_position(gui_root, vec2(-1, 1));  // origin at top-left
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
    CVec2 win_size;

    win_size = game_get_window_size();

    edit_set_editable(gui_root, false);

    // child of camera so GUI stays on screen
    transform_set_parent(gui_root, camera_get_current_camera());

    // use pixel coordinates
    transform_set_scale(gui_root, scalar_vec2_div(2, win_size));
    gui_rect_set_size(gui_root, win_size);
}

void gui_update_all() {
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

// namespace neko::imgui::wrap_ImGuiInputTextCallbackData {
// void pointer(lua_State *L, ImGuiInputTextCallbackData &v);
// }

namespace neko::imgui::util {

static lua_CFunction str_format = NULL;

lua_Integer field_tointeger(lua_State *L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Number field_tonumber(lua_State *L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return v;
}

bool field_toboolean(lua_State *L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    bool v = !!lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

ImTextureID get_texture_id(lua_State *L, int idx) {
    // int lua_handle = (int)luaL_checkinteger(L, idx);
    // if (auto id = ImGui_ImplBgfx_GetTextureID(lua_handle)) {
    //     return *id;
    // }
    // luaL_error(L, "Invalid handle type TEXTURE");
    // std::unreachable();
    neko_assert(0);
    return 0;
}

const char *format(lua_State *L, int idx) {
    lua_pushcfunction(L, str_format);
    lua_insert(L, idx);
    lua_call(L, lua_gettop(L) - idx, 1);
    return lua_tostring(L, -1);
}

static void *strbuf_realloc(lua_State *L, void *ptr, size_t osize, size_t nsize) {
    void *ud;
    lua_Alloc allocator = lua_getallocf(L, &ud);
    return allocator(ud, ptr, osize, nsize);
}

static int strbuf_assgin(lua_State *L) {
    auto sbuf = (strbuf *)lua_touserdata(L, 1);
    size_t newsize = 0;
    const char *newbuf = luaL_checklstring(L, 2, &newsize);
    newsize++;
    if (newsize > sbuf->size) {
        sbuf->data = (char *)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
        sbuf->size = newsize;
    }
    memcpy(sbuf->data, newbuf, newsize);
    return 0;
}

static int strbuf_resize(lua_State *L) {
    auto sbuf = (strbuf *)lua_touserdata(L, 1);
    size_t newsize = (size_t)luaL_checkinteger(L, 2);
    sbuf->data = (char *)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
    sbuf->size = newsize;
    return 0;
}

static int strbuf_tostring(lua_State *L) {
    auto sbuf = (strbuf *)lua_touserdata(L, 1);
    lua_pushstring(L, sbuf->data);
    return 1;
}

static int strbuf_release(lua_State *L) {
    auto sbuf = (strbuf *)lua_touserdata(L, 1);
    strbuf_realloc(L, sbuf->data, sbuf->size, 0);
    sbuf->data = NULL;
    sbuf->size = 0;
    return 0;
}

static constexpr size_t kStrBufMinSize = 256;

strbuf *strbuf_create(lua_State *L, int idx) {
    size_t sz;
    const char *text = lua_tolstring(L, idx, &sz);
    auto sbuf = (strbuf *)lua_newuserdatauv(L, sizeof(strbuf), 0);
    if (text == NULL) {
        sbuf->size = kStrBufMinSize;
        sbuf->data = (char *)strbuf_realloc(L, NULL, 0, sbuf->size);
        sbuf->data[0] = '\0';
    } else {
        sbuf->size = (std::max)(sz + 1, kStrBufMinSize);
        sbuf->data = (char *)strbuf_realloc(L, NULL, 0, sbuf->size);
        memcpy(sbuf->data, text, sz + 1);
    }
    if (luaL_newmetatable(L, "ImGui::StringBuf")) {
        lua_pushcfunction(L, strbuf_tostring);
        lua_setfield(L, -2, "__tostring");
        lua_pushcfunction(L, strbuf_release);
        lua_setfield(L, -2, "__gc");
        static luaL_Reg l[] = {
                {"Assgin", strbuf_assgin},
                {"Resize", strbuf_resize},
                {NULL, NULL},
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
    return sbuf;
}

strbuf *strbuf_get(lua_State *L, int idx) {
    if (lua_type(L, idx) == LUA_TUSERDATA) {
        auto sbuf = (strbuf *)luaL_checkudata(L, idx, "ImGui::StringBuf");
        return sbuf;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    int t = lua_geti(L, idx, 1);
    if (t != LUA_TSTRING && t != LUA_TNIL) {
        auto sbuf = (strbuf *)luaL_checkudata(L, -1, "ImGui::StringBuf");
        lua_pop(L, 1);
        return sbuf;
    }
    auto sbuf = strbuf_create(L, -1);
    lua_replace(L, -2);
    lua_seti(L, idx, 1);
    return sbuf;
}

int input_callback(ImGuiInputTextCallbackData *data) {
    auto ctx = (input_context *)data->UserData;
    lua_State *L = ctx->L;
    lua_pushvalue(L, ctx->callback);
    // wrap_ImGuiInputTextCallbackData::pointer(L, *data);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        return 1;
    }
    lua_Integer retval = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return (int)retval;
}

void create_table(lua_State *L, std::span<TableInteger> l) {
    lua_createtable(L, 0, (int)l.size());
    for (auto const &e : l) {
        lua_pushinteger(L, e.value);
        lua_setfield(L, -2, e.name);
    }
}

void set_table(lua_State *L, std::span<TableAny> l) {
    for (auto const &e : l) {
        e.value(L);
        lua_setfield(L, -2, e.name);
    }
}

static void set_table(lua_State *L, std::span<luaL_Reg> l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (auto const &e : l) {
        for (int i = 0; i < nup; i++) {
            lua_pushvalue(L, -nup);
        }
        lua_pushcclosure(L, e.func, nup);
        lua_setfield(L, -(nup + 2), e.name);
    }
    lua_pop(L, nup);
}

static int make_flags(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int i, t;
    lua_Integer r = 0;
    for (i = 1; (t = lua_geti(L, 1, i)) != LUA_TNIL; i++) {
        if (t != LUA_TSTRING) luaL_error(L, "Flag name should be string, it's %s", lua_typename(L, t));
        if (lua_gettable(L, lua_upvalueindex(1)) != LUA_TNUMBER) {
            lua_geti(L, 1, i);
            luaL_error(L, "Invalid flag %s.%s", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, -1));
        }
        lua_Integer v = lua_tointeger(L, -1);
        lua_pop(L, 1);
        r |= v;
    }
    lua_pushinteger(L, r);
    return 1;
}

void struct_gen(lua_State *L, const char *name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters) {
    lua_newuserdatauv(L, sizeof(uintptr_t), 0);
    int ud = lua_gettop(L);
    lua_newtable(L);
    if (!setters.empty()) {
        static lua_CFunction setter_func = +[](lua_State *L) {
            lua_pushvalue(L, 2);
            if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
                return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
            }
            lua_pushvalue(L, 3);
            lua_call(L, 1, 0);
            return 0;
        };
        lua_createtable(L, 0, (int)setters.size());
        lua_pushvalue(L, ud);
        set_table(L, setters, 1);
        lua_pushstring(L, name);
        lua_pushcclosure(L, setter_func, 2);
        lua_setfield(L, -2, "__newindex");
    }
    if (!funcs.empty()) {
        lua_createtable(L, 0, (int)funcs.size());
        lua_pushvalue(L, ud);
        set_table(L, funcs, 1);
        lua_newtable(L);
    }
    static lua_CFunction getter_func = +[](lua_State *L) {
        lua_pushvalue(L, 2);
        if (LUA_TNIL == lua_gettable(L, lua_upvalueindex(1))) {
            return luaL_error(L, "%s.%s is invalid.", lua_tostring(L, lua_upvalueindex(2)), lua_tostring(L, 2));
        }
        lua_call(L, 0, 1);
        return 1;
    };
    lua_createtable(L, 0, (int)getters.size());
    lua_pushvalue(L, ud);
    set_table(L, getters, 1);
    lua_pushstring(L, name);
    lua_pushcclosure(L, getter_func, 2);
    lua_setfield(L, -2, "__index");
    if (!funcs.empty()) {
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);
}

void flags_gen(lua_State *L, const char *name) {
    lua_pushstring(L, name);
    lua_pushcclosure(L, make_flags, 2);
}

void init(lua_State *L) {
    luaopen_string(L);
    lua_getfield(L, -1, "format");
    str_format = lua_tocfunction(L, -1);
    lua_pop(L, 2);
}

}  // namespace neko::imgui::util

void imgui_init() {
    PROFILE_FUNC();

    ImGui::SetAllocatorFunctions(+[](size_t sz, void *user_data) { return mem_alloc(sz); }, +[](void *ptr, void *user_data) { return mem_free(ptr); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_app->game_window, true);
    ImGui_ImplOpenGL3_Init();

    CVAR_REF(conf_imgui_font, String);

    if (neko_strlen(conf_imgui_font.data.str) > 0) {
        auto &io = ImGui::GetIO();

        ImFontConfig config;
        // config.PixelSnapH = 1;

        String ttf_file;
        vfs_read_entire_file(&ttf_file, conf_imgui_font.data.str);
        // neko_defer(mem_free(ttf_file.data));
        // void *ttf_data = ::mem_alloc(ttf_file.len);  // TODO:: imgui 内存方法接管
        // memcpy(ttf_data, ttf_file.data, ttf_file.len);
        io.Fonts->AddFontFromMemoryTTF(ttf_file.data, ttf_file.len, 12.0f, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
}

void imgui_fini() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_draw_pre() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_draw_post() {
    ImGui::Render();
    // int displayX, displayY;
    // glfwGetFramebufferSize(window, &displayX, &displayY);
    // glViewport(0, 0, displayX, displayY);
    // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}