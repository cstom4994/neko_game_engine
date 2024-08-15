#include "engine/ui.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/api.hpp"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/ecs.h"
#include "engine/edit.h"
#include "engine/game.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/luax.h"
#include "engine/os.h"
#include "engine/prelude.h"
#include "engine/texture.h"
#include "engine/transform.h"

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

static EntityPool* gui_pool;

static EntityMap* focus_enter_map;
static EntityMap* focus_exit_map;
static EntityMap* changed_map;
static EntityMap* mouse_down_map;
static EntityMap* mouse_up_map;
static EntityMap* key_down_map;
static EntityMap* key_up_map;

Entity gui_get_root() { return gui_root; }

void gui_add(Entity ent) {
    Gui* gui;

    if (entitypool_get(gui_pool, ent)) return;  // 已经有gui

    transform_add(ent);

    gui = (Gui*)entitypool_add(gui_pool, ent);
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
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->color = color;
}
Color gui_get_color(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->color;
}

void gui_set_visible(Entity ent, bool visible) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->setvisible = visible;
}
bool gui_get_visible(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->visible;
}

void gui_set_focusable(Entity ent, bool focusable) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->focusable = focusable;
}
bool gui_get_focusable(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->focusable;
}

void gui_set_captures_events(Entity ent, bool captures_events) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->captures_events = captures_events;
}
bool gui_get_captures_events(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->captures_events;
}

void gui_set_halign(Entity ent, GuiAlign align) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->halign = align;
}
GuiAlign gui_get_halign(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->halign;
}
void gui_set_valign(Entity ent, GuiAlign align) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->valign = align;
}
GuiAlign gui_get_valign(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    return gui->valign;
}
void gui_set_padding(Entity ent, CVec2 padding) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
    error_assert(gui);
    gui->padding = padding;
}
CVec2 gui_get_padding(Entity ent) {
    Gui* gui = (Gui*)entitypool_get(gui_pool, ent);
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

static void _common_update_visible_rec(Gui* gui) {
    Gui* pgui;

    if (gui->updated_visible) return;

    // false visibility takes priority
    if (!gui->setvisible) {
        gui->visible = false;
        gui->updated_visible = true;
        return;
    }

    // if has parent, inherit
    pgui = (Gui*)entitypool_get(gui_pool, transform_get_parent(gui->pool_elem.ent));
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
    Gui* gui;

    entitypool_foreach(gui, gui_pool) gui->updated_visible = false;
    entitypool_foreach(gui, gui_pool) _common_update_visible_rec(gui);
}

static void _common_align(Gui* gui, GuiAlign halign, GuiAlign valign) {
    Gui* pgui;
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
    pgui = (Gui*)entitypool_get(gui_pool, transform_get_parent(ent));
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
    Gui* gui;
    entitypool_foreach(gui, gui_pool) _common_align(gui, gui->halign == GA_NONE ? GA_NONE : GA_MIN, gui->valign == GA_NONE ? GA_NONE : GA_MAX);
}

static void _common_update_align() {
    Gui* gui;
    entitypool_foreach(gui, gui_pool) _common_align(gui, gui->halign, gui->valign);
}

// attach root GUI entities to gui_root
static void _common_attach_root() {
    Gui* gui;
    Entity ent;

    entitypool_foreach(gui, gui_pool) {
        ent = gui->pool_elem.ent;
        if (!entity_eq(ent, gui_root) && entity_eq(transform_get_parent(ent), entity_nil)) transform_set_parent(ent, gui_root);
    }
}

static void _common_update_all() {
    Gui* gui;

    _common_attach_root();

    // update edit bboxes
    if (edit_get_enabled()) entitypool_foreach(gui, gui_pool) edit_bboxes_update(gui->pool_elem.ent, gui->bbox);
}

// 'focus_clear' is whether to clear focus if click outside
static void _common_mouse_event(EntityMap* emap, MouseCode mouse, bool focus_clear) {
    Gui* gui;
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

static void _common_save_all(Store* s) {
    Store *t, *gui_s;
    Gui* gui;

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
static void _common_load_all(Store* s) {
    Store *t, *gui_s;
    Gui* gui;

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

static EntityPool* rect_pool;

void gui_rect_add(Entity ent) {
    Rect* rect;

    if (entitypool_get(rect_pool, ent)) return;

    gui_add(ent);

    rect = (Rect*)entitypool_add(rect_pool, ent);
    rect->size = vec2(64, 64);
    rect->hfit = true;
    rect->vfit = true;
    rect->hfill = false;
    rect->vfill = false;
}
void gui_rect_remove(Entity ent) { entitypool_remove(rect_pool, ent); }
bool gui_rect_has(Entity ent) { return entitypool_get(rect_pool, ent) != NULL; }

void gui_rect_set_size(Entity ent, CVec2 size) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->size = size;
}
CVec2 gui_rect_get_size(Entity ent) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->size;
}

void gui_rect_set_hfit(Entity ent, bool fit) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfit = fit;
}
bool gui_rect_get_hfit(Entity ent) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfit;
}
void gui_rect_set_vfit(Entity ent, bool fit) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfit = fit;
}
bool gui_rect_get_vfit(Entity ent) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->vfit;
}

void gui_rect_set_hfill(Entity ent, bool fill) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->hfill = fill;
}
bool gui_rect_get_hfill(Entity ent) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    return rect->hfill;
}
void gui_rect_set_vfill(Entity ent, bool fill) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
    error_assert(rect);
    rect->vfill = fill;
}
bool gui_rect_get_vfill(Entity ent) {
    Rect* rect = (Rect*)entitypool_get(rect_pool, ent);
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

static void _rect_update_table_align(Rect* rect) {
    Entity rect_ent, *children;
    Gui* child;
    unsigned int nchildren, i;
    Scalar delta;
    BBox b;
    CVec2 pos, curr;

    rect_ent = rect->pool_elem.ent;

    curr = vec2_zero;
    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui*)entitypool_get(gui_pool, children[i]);
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

static void _rect_update_fit(Rect* rect) {
    Entity rect_ent, *children;
    Gui* child;
    unsigned int nchildren, i;
    Scalar miny, maxx;
    BBox b;

    rect_ent = rect->pool_elem.ent;

    miny = 0;
    maxx = 0;

    children = transform_get_children(rect_ent);
    nchildren = transform_get_num_children(rect_ent);
    for (i = 0; i < nchildren; ++i) {
        child = (Gui*)entitypool_get(gui_pool, children[i]);
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
    Rect* rect;
    Gui* gui;

    gui = (Gui*)entitypool_get(gui_pool, ent);
    if (!gui) return;

    rect = (Rect*)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_table_align(rect);
    _rect_update_fit(rect);

    gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));
}

static void _rect_update_parent_first(Entity ent);

static void _rect_update_fill(Rect* rect) {
    Entity ent;
    Gui *pgui, *gui;
    BBox b;
    Entity parent;

    ent = rect->pool_elem.ent;
    gui = (Gui*)entitypool_get(gui_pool, ent);
    if (!gui) return;

    if (!rect || !rect->visible || rect->updated || !(rect->hfill || rect->vfill)) return;

    parent = transform_get_parent(ent);
    pgui = (Gui*)entitypool_get(gui_pool, parent);
    if (!pgui) return;  // no parent to fill to

    _rect_update_parent_first(parent);
    b = bbox_transform(mat3_inverse(transform_get_matrix(ent)), pgui->bbox);

    if (rect->hfill) rect->size.x = b.max.x - gui->padding.x;
    if (rect->vfill) rect->size.y = -b.min.y + gui->padding.y;
}

static void _rect_update_depth(Rect* rect) {
    Rect* prect;

    prect = (Rect*)entitypool_get(rect_pool, transform_get_parent(rect->pool_elem.ent));
    if (prect) {
        _rect_update_parent_first(prect->pool_elem.ent);
        rect->depth = prect->depth + 1;
    } else
        rect->depth = 0;
}

static void _rect_update_parent_first(Entity ent) {
    Rect* rect;
    Gui* gui;

    gui = (Gui*)entitypool_get(gui_pool, ent);
    if (!gui) return;

    rect = (Rect*)entitypool_get(rect_pool, ent);
    if (!rect || rect->updated) return;
    _rect_update_fill(rect);
    _rect_update_depth(rect);

    gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));
}

static void _rect_update_all() {
    Rect* rect;
    Gui* gui;

    entitypool_remove_destroyed(rect_pool, gui_rect_remove);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_child_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) rect->updated = false;
    entitypool_foreach(rect, rect_pool) _rect_update_parent_first(rect->pool_elem.ent);

    entitypool_foreach(rect, rect_pool) {
        gui = (Gui*)entitypool_get(gui_pool, rect->pool_elem.ent);
        error_assert(gui);

        // write gui bbox
        gui->bbox = bbox_bound(vec2_zero, vec2(rect->size.x, -rect->size.y));

        // read gui properties
        rect->visible = gui->visible;
        rect->color = gui->color;
    }
}

static void _rect_update_wmat() {
    Rect* rect;
    entitypool_foreach(rect, rect_pool) rect->wmat = transform_get_world_matrix(rect->pool_elem.ent);
}

static int _rect_depth_compare(const void* a, const void* b) {
    const Rect *ra = (Rect*)a, *rb = (Rect*)b;
    if (ra->depth == rb->depth) return ((int)ra->pool_elem.ent.id) - ((int)rb->pool_elem.ent.id);
    return ra->depth - rb->depth;
}

static void _rect_draw_all() {
    unsigned int nrects;

    // depth sort
    entitypool_sort(rect_pool, _rect_depth_compare);

    // bind shader program
    glUseProgram(rect_program);
    glUniformMatrix3fv(glGetUniformLocation(rect_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat*)camera_get_inverse_view_matrix_ptr());

    // draw!
    glBindVertexArray(rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
    nrects = entitypool_size(rect_pool);
    glBufferData(GL_ARRAY_BUFFER, nrects * sizeof(Rect), entitypool_begin(rect_pool), GL_STREAM_DRAW);
    glDrawArrays(GL_POINTS, 0, nrects);
}

static void _rect_save_all(Store* s) {
    Store *t, *rect_s;
    Rect* rect;

    if (store_child_save(&t, "gui_rect", s)) entitypool_save_foreach(rect, rect_s, rect_pool, "pool", t) {
            vec2_save(&rect->size, "size", rect_s);
            color_save(&rect->color, "color", rect_s);
            bool_save(&rect->hfit, "hfit", rect_s);
            bool_save(&rect->vfit, "vfit", rect_s);
            bool_save(&rect->hfill, "hfill", rect_s);
            bool_save(&rect->vfill, "vfill", rect_s);
        }
}
static void _rect_load_all(Store* s) {
    Store *t, *rect_s;
    Rect* rect;

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

    char* str;
    CArray* chars;  // per-character info buffered to shader
    CVec2 bounds;   // max x, min y in size-less units

    int cursor;
};

static EntityPool* text_pool;

static Scalar cursor_blink_time = 0;

static void _text_add_cursor(Text* text, CVec2 pos) {
    TextChar* tc;

    // compute position in font grid
    tc = (TextChar*)array_add(text->chars);
    tc->pos = pos;
    tc->cell = vec2(' ' % TEXT_GRID_W, TEXT_GRID_H - 1 - (' ' / TEXT_GRID_W));
    tc->is_cursor = 1;
}

// just update with existing string if str is NULL
static void _text_set_str(Text* text, const char* str) {
    char c;
    TextChar* tc;
    CVec2 pos;
    int i = 0;

    // copy to struct?
    if (str) {
        mem_free(text->str);
        text->str = (char*)mem_alloc(strlen(str) + 1);
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
        tc = (TextChar*)array_add(text->chars);
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
    Text* text;

    if (entitypool_get(text_pool, ent)) return;  // already has text

    gui_add(ent);

    text = (Text*)entitypool_add(text_pool, ent);
    text->chars = array_new(TextChar);
    text->str = NULL;  // _text_set_str(...) calls mem_free(text->str)
    text->cursor = -1;
    _text_set_str(text, "");
}
void gui_text_remove(Entity ent) {
    Text* text = (Text*)entitypool_get(text_pool, ent);
    if (text) {
        mem_free(text->str);
        array_free(text->chars);
    }
    entitypool_remove(text_pool, ent);
}
bool gui_text_has(Entity ent) { return entitypool_get(text_pool, ent) != NULL; }

void gui_text_set_str(Entity ent, const char* str) {
    Text* text = (Text*)entitypool_get(text_pool, ent);
    error_assert(text);
    _text_set_str(text, str);
}
const char* gui_text_get_str(Entity ent) {
    Text* text = (Text*)entitypool_get(text_pool, ent);
    error_assert(text);
    return text->str;
}

static void _text_set_cursor(Entity ent, int cursor) {
    Text* text = (Text*)entitypool_get(text_pool, ent);
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
    Text* text;

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
    Text* text;
    Gui* gui;
    static CVec2 size = {TEXT_FONT_W, TEXT_FONT_H};

    cursor_blink_time += 2 * timing_instance.true_dt;

    entitypool_remove_destroyed(text_pool, gui_text_remove);

    entitypool_foreach(text, text_pool) {
        // blink on when focus entered
        if (gui_event_focus_enter(text->pool_elem.ent)) cursor_blink_time = 1;

        // gui bbox
        gui = (Gui*)entitypool_get(gui_pool, text->pool_elem.ent);
        error_assert(gui);
        gui->bbox = bbox_bound(vec2_zero, vec2_mul(size, text->bounds));
    }
}

void ME_draw_text(String text, Color256 col, int x, int y, bool outline, Color256 outline_col) {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList(viewport);

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
    Text* text;
    Gui* gui;
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
    glUniformMatrix3fv(glGetUniformLocation(text_program, "inverse_view_matrix"), 1, GL_FALSE, (const GLfloat*)camera_get_inverse_view_matrix_ptr());

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    texture_bind("assets/data/font1.png");

    // draw!
    entitypool_foreach(text, text_pool) {
        gui = (Gui*)entitypool_get(gui_pool, text->pool_elem.ent);
        error_assert(gui);
        if (!gui->visible) continue;
        glUniform4fv(glGetUniformLocation(text_program, "base_color"), 1, (const GLfloat*)&gui->color);

        wmat = transform_get_world_matrix(text->pool_elem.ent);
        glUniformMatrix3fv(glGetUniformLocation(text_program, "wmat"), 1, GL_FALSE, (const GLfloat*)&wmat);

        nchars = array_length(text->chars);
        glBufferData(GL_ARRAY_BUFFER, nchars * sizeof(TextChar), array_begin(text->chars), GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, nchars);

        CVec2 text_pos = transform_get_world_position(text->pool_elem.ent);
        ME_draw_text(text->str, NEKO_COLOR_WHITE, text_pos.x, text_pos.y, false, NEKO_COLOR_WHITE);
    }
}

static void _text_save_all(Store* s) {
    Store *t, *text_s;
    Text* text;

    if (store_child_save(&t, "gui_text", s)) entitypool_save_foreach(text, text_s, text_pool, "pool", t) {
            string_save((const char**)&text->str, "str", text_s);
            int_save(&text->cursor, "cursor", text_s);
        }
}
static void _text_load_all(Store* s) {
    Store *t, *text_s;
    Text* text;

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

EntityPool* textedit_pool;

void gui_textedit_add(Entity ent) {
    TextEdit* textedit;

    if (entitypool_get(textedit_pool, ent)) return;

    gui_text_add(ent);
    gui_set_focusable(ent, true);

    textedit = (TextEdit*)entitypool_add(textedit_pool, ent);
    textedit->cursor = 0;
    textedit->numerical = false;
}
void gui_textedit_remove(Entity ent) { entitypool_remove(textedit_pool, ent); }
bool gui_textedit_has(Entity ent) { return entitypool_get(textedit_pool, ent) != NULL; }

void gui_textedit_set_numerical(Entity ent, bool numerical) {
    TextEdit* textedit = (TextEdit*)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->numerical = numerical;
}
bool gui_textedit_get_numerical(Entity ent) {
    TextEdit* textedit = (TextEdit*)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->numerical;
}
Scalar gui_textedit_get_num(Entity ent) { return strtof(gui_text_get_str(ent), NULL); }

static void _textedit_fix_cursor(TextEdit* textedit) {
    unsigned int len = strlen(gui_text_get_str(textedit->pool_elem.ent));
    if (textedit->cursor > len) textedit->cursor = len;
}

void gui_textedit_set_cursor(Entity ent, unsigned int cursor) {
    TextEdit* textedit = (TextEdit*)entitypool_get(textedit_pool, ent);
    textedit = (TextEdit*)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    textedit->cursor = cursor;
    _textedit_fix_cursor(textedit);
}
unsigned int gui_textedit_get_cursor(Entity ent) {
    TextEdit* textedit = (TextEdit*)entitypool_get(textedit_pool, ent);
    error_assert(textedit);
    return textedit->cursor;
}

static void _textedit_init() { textedit_pool = entitypool_new(TextEdit); }
static void _textedit_fini() { entitypool_free(textedit_pool); }

static bool _textedit_set_str(TextEdit* textedit, const char* str) {
    gui_text_set_str(textedit->pool_elem.ent, str);
    entitymap_set(changed_map, textedit->pool_elem.ent, true);
    return true;
}

// common function for key/char events
static void _textedit_key_event(KeyCode key, unsigned int c) {
    Entity ent;
    TextEdit* textedit;
    const char* old;
    char* new_ptr = NULL;

    textedit = (TextEdit*)entitypool_get(textedit_pool, focused);
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

        new_ptr = (char*)mem_alloc(strlen(old));  // 1 less, but 1 more for null
        strncpy(new_ptr, old, textedit->cursor);
        strcpy(&new_ptr[textedit->cursor], &old[textedit->cursor + 1]);
        _textedit_set_str(textedit, new_ptr);
    }

    // insert char
    else if (isprint(c)) {
        new_ptr = (char*)mem_alloc(strlen(old) + 2);  // 1 for new_ptr char, 1 for null
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
    TextEdit* textedit;

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

static void _textedit_save_all(Store* s) {
    Store *t, *textedit_s;
    TextEdit* textedit;

    if (store_child_save(&t, "gui_textedit", s)) entitypool_save_foreach(textedit, textedit_s, textedit_pool, "pool", t) {
            uint_save(&textedit->cursor, "cursor", textedit_s);
            bool_save(&textedit->numerical, "numerical", textedit_s);
        }
}
static void _textedit_load_all(Store* s) {
    Store *t, *textedit_s;
    TextEdit* textedit;

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

void gui_save_all(Store* s) {
    _common_save_all(s);
    _rect_save_all(s);
    _text_save_all(s);
    _textedit_save_all(s);
}
void gui_load_all(Store* s) {
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

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return v;
}

lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    auto v = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return v;
}

bool field_toboolean(lua_State* L, int idx, lua_Integer i) {
    lua_geti(L, idx, i);
    bool v = !!lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

ImTextureID get_texture_id(lua_State* L, int idx) {
    // int lua_handle = (int)luaL_checkinteger(L, idx);
    // if (auto id = ImGui_ImplBgfx_GetTextureID(lua_handle)) {
    //     return *id;
    // }
    // luaL_error(L, "Invalid handle type TEXTURE");
    // std::unreachable();
    neko_assert(0);
    return 0;
}

const char* format(lua_State* L, int idx) {
    lua_pushcfunction(L, str_format);
    lua_insert(L, idx);
    lua_call(L, lua_gettop(L) - idx, 1);
    return lua_tostring(L, -1);
}

static void* strbuf_realloc(lua_State* L, void* ptr, size_t osize, size_t nsize) {
    void* ud;
    lua_Alloc allocator = lua_getallocf(L, &ud);
    return allocator(ud, ptr, osize, nsize);
}

static int strbuf_assgin(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = 0;
    const char* newbuf = luaL_checklstring(L, 2, &newsize);
    newsize++;
    if (newsize > sbuf->size) {
        sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
        sbuf->size = newsize;
    }
    memcpy(sbuf->data, newbuf, newsize);
    return 0;
}

static int strbuf_resize(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    size_t newsize = (size_t)luaL_checkinteger(L, 2);
    sbuf->data = (char*)strbuf_realloc(L, sbuf->data, sbuf->size, newsize);
    sbuf->size = newsize;
    return 0;
}

static int strbuf_tostring(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    lua_pushstring(L, sbuf->data);
    return 1;
}

static int strbuf_release(lua_State* L) {
    auto sbuf = (strbuf*)lua_touserdata(L, 1);
    strbuf_realloc(L, sbuf->data, sbuf->size, 0);
    sbuf->data = NULL;
    sbuf->size = 0;
    return 0;
}

static constexpr size_t kStrBufMinSize = 256;

strbuf* strbuf_create(lua_State* L, int idx) {
    size_t sz;
    const char* text = lua_tolstring(L, idx, &sz);
    auto sbuf = (strbuf*)lua_newuserdatauv(L, sizeof(strbuf), 0);
    if (text == NULL) {
        sbuf->size = kStrBufMinSize;
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
        sbuf->data[0] = '\0';
    } else {
        sbuf->size = (std::max)(sz + 1, kStrBufMinSize);
        sbuf->data = (char*)strbuf_realloc(L, NULL, 0, sbuf->size);
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

strbuf* strbuf_get(lua_State* L, int idx) {
    if (lua_type(L, idx) == LUA_TUSERDATA) {
        auto sbuf = (strbuf*)luaL_checkudata(L, idx, "ImGui::StringBuf");
        return sbuf;
    }
    luaL_checktype(L, idx, LUA_TTABLE);
    int t = lua_geti(L, idx, 1);
    if (t != LUA_TSTRING && t != LUA_TNIL) {
        auto sbuf = (strbuf*)luaL_checkudata(L, -1, "ImGui::StringBuf");
        lua_pop(L, 1);
        return sbuf;
    }
    auto sbuf = strbuf_create(L, -1);
    lua_replace(L, -2);
    lua_seti(L, idx, 1);
    return sbuf;
}

int input_callback(ImGuiInputTextCallbackData* data) {
    auto ctx = (input_context*)data->UserData;
    lua_State* L = ctx->L;
    lua_pushvalue(L, ctx->callback);
    // wrap_ImGuiInputTextCallbackData::pointer(L, *data);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        return 1;
    }
    lua_Integer retval = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return (int)retval;
}

void create_table(lua_State* L, std::span<TableInteger> l) {
    lua_createtable(L, 0, (int)l.size());
    for (auto const& e : l) {
        lua_pushinteger(L, e.value);
        lua_setfield(L, -2, e.name);
    }
}

void set_table(lua_State* L, std::span<TableAny> l) {
    for (auto const& e : l) {
        e.value(L);
        lua_setfield(L, -2, e.name);
    }
}

static void set_table(lua_State* L, std::span<luaL_Reg> l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (auto const& e : l) {
        for (int i = 0; i < nup; i++) {
            lua_pushvalue(L, -nup);
        }
        lua_pushcclosure(L, e.func, nup);
        lua_setfield(L, -(nup + 2), e.name);
    }
    lua_pop(L, nup);
}

static int make_flags(lua_State* L) {
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

void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters) {
    lua_newuserdatauv(L, sizeof(uintptr_t), 0);
    int ud = lua_gettop(L);
    lua_newtable(L);
    if (!setters.empty()) {
        static lua_CFunction setter_func = +[](lua_State* L) {
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
    static lua_CFunction getter_func = +[](lua_State* L) {
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

void flags_gen(lua_State* L, const char* name) {
    lua_pushstring(L, name);
    lua_pushcclosure(L, make_flags, 2);
}

void init(lua_State* L) {
    luaopen_string(L);
    lua_getfield(L, -1, "format");
    str_format = lua_tocfunction(L, -1);
    lua_pop(L, 2);
}

}  // namespace neko::imgui::util

void imgui_init() {
    PROFILE_FUNC();

    ImGui::SetAllocatorFunctions(+[](size_t sz, void* user_data) { return mem_alloc(sz); }, +[](void* ptr, void* user_data) { return mem_free(ptr); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_app->game_window, true);
    ImGui_ImplOpenGL3_Init();

    CVAR_REF(conf_imgui_font, String);

    if (neko_strlen(conf_imgui_font.data.str) > 0) {
        auto& io = ImGui::GetIO();

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

#if 1

#define ui_unused(x) ((void)(x))

#define ui_stack_push(stk, val)                                                     \
    do {                                                                            \
        NEKO_EXPECT((stk).idx < (i32)(sizeof((stk).items) / sizeof(*(stk).items))); \
        (stk).items[(stk).idx] = (val);                                             \
        (stk).idx++; /* incremented after incase `val` uses this value */           \
    } while (0)

#define ui_stack_pop(stk)           \
    do {                            \
        NEKO_EXPECT((stk).idx > 0); \
        (stk).idx--;                \
    } while (0)

/* 32bit fnv-1a hash */
#define NEKO_UI_HASH_INITIAL 2166136261

static void ui_hash(ui_id* hash, const void* data, i32 size) {
    const unsigned char* p = (const unsigned char*)data;
    while (size--) {
        *hash = (*hash ^ *p++) * 16777619;
    }
}

static ui_rect_t ui_unclipped_rect = {0, 0, 0x1000000, 0x1000000};

// Default styles
static ui_style_t ui_default_container_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_button_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_text_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_label_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_panel_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_input_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_scroll_style[3] = NEKO_DEFAULT_VAL();
static ui_style_t ui_default_image_style[3] = NEKO_DEFAULT_VAL();

static ui_style_t ui_default_style = {
        // font | size | spacing | indent | title_height | scroll_width | thumb_width
        NULL,
        {68, 18},
        2,
        10,
        20,
        5,
        5,

        // colors
        {
                {25, 25, 25, 255},     // NEKO_UI_COLOR_BACKGROUND
                {255, 255, 255, 255},  // NEKO_UI_COLOR_CONTENT
                {29, 29, 29, 76},      // NEKO_UI_COLOR_BORDER
                {0, 0, 0, 31},         // NEKO_UI_COLOR_SHADOW
                {0, 0, 0, 0},          // NEKO_UI_COLOR_CONTENT_BACKGROUND
                {0, 0, 0, 0},          // NEKO_UI_COLOR_CONTENT_SHADOW
                {0, 0, 0, 0}           // NEKO_UI_COLOR_CONTENT_BORDER
        },

        // padding (left, right, top, bottom)
        {2, 2, 2, 2},

        // margin (left, right, top, bottom)
        {2, 2, 2, 2},

        // border width (left, right, top, bottom)
        {1, 1, 1, 1},

        // border radius (left, right, top, bottom)
        {0, 0, 0, 0},

        // flex direction / justification / alignment / shrink / grow
        NEKO_UI_DIRECTION_COLUMN,
        NEKO_UI_JUSTIFY_START,
        NEKO_UI_ALIGN_CENTER,

        // shadow x, y
        1,
        1};

static ui_style_sheet_t ui_default_style_sheet = NEKO_DEFAULT_VAL();

static ui_style_t ui_get_current_element_style(ui_context_t* ctx, const ui_selector_desc_t* desc, i32 elementid, i32 state) {

#define NEKO_UI_APPLY_STYLE(SE)                                               \
    do {                                                                      \
        switch ((SE)->type) {                                                 \
            case NEKO_UI_STYLE_WIDTH:                                         \
                style.size[0] = (float)(SE)->value;                           \
                break;                                                        \
            case NEKO_UI_STYLE_HEIGHT:                                        \
                style.size[1] = (float)(SE)->value;                           \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_PADDING: {                                     \
                style.padding[NEKO_UI_PADDING_LEFT] = (i32)(SE)->value;       \
                style.padding[NEKO_UI_PADDING_TOP] = (i32)(SE)->value;        \
                style.padding[NEKO_UI_PADDING_RIGHT] = (i32)(SE)->value;      \
                style.padding[NEKO_UI_PADDING_BOTTOM] = (i32)(SE)->value;     \
            }                                                                 \
                                                                              \
            case NEKO_UI_STYLE_PADDING_LEFT:                                  \
                style.padding[NEKO_UI_PADDING_LEFT] = (i32)(SE)->value;       \
                break;                                                        \
            case NEKO_UI_STYLE_PADDING_TOP:                                   \
                style.padding[NEKO_UI_PADDING_TOP] = (i32)(SE)->value;        \
                break;                                                        \
            case NEKO_UI_STYLE_PADDING_RIGHT:                                 \
                style.padding[NEKO_UI_PADDING_RIGHT] = (i32)(SE)->value;      \
                break;                                                        \
            case NEKO_UI_STYLE_PADDING_BOTTOM:                                \
                style.padding[NEKO_UI_PADDING_BOTTOM] = (i32)(SE)->value;     \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_MARGIN: {                                      \
                style.margin[NEKO_UI_MARGIN_LEFT] = (i32)(SE)->value;         \
                style.margin[NEKO_UI_MARGIN_TOP] = (i32)(SE)->value;          \
                style.margin[NEKO_UI_MARGIN_RIGHT] = (i32)(SE)->value;        \
                style.margin[NEKO_UI_MARGIN_BOTTOM] = (i32)(SE)->value;       \
            } break;                                                          \
                                                                              \
            case NEKO_UI_STYLE_MARGIN_LEFT:                                   \
                style.margin[NEKO_UI_MARGIN_LEFT] = (i32)(SE)->value;         \
                break;                                                        \
            case NEKO_UI_STYLE_MARGIN_TOP:                                    \
                style.margin[NEKO_UI_MARGIN_TOP] = (i32)(SE)->value;          \
                break;                                                        \
            case NEKO_UI_STYLE_MARGIN_RIGHT:                                  \
                style.margin[NEKO_UI_MARGIN_RIGHT] = (i32)(SE)->value;        \
                break;                                                        \
            case NEKO_UI_STYLE_MARGIN_BOTTOM:                                 \
                style.margin[NEKO_UI_MARGIN_BOTTOM] = (i32)(SE)->value;       \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_BORDER_RADIUS: {                               \
                style.border_radius[0] = (SE)->value;                         \
                style.border_radius[1] = (SE)->value;                         \
                style.border_radius[2] = (SE)->value;                         \
                style.border_radius[3] = (SE)->value;                         \
            } break;                                                          \
                                                                              \
            case NEKO_UI_STYLE_BORDER_RADIUS_LEFT:                            \
                style.border_radius[0] = (SE)->value;                         \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_RADIUS_RIGHT:                           \
                style.border_radius[1] = (SE)->value;                         \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_RADIUS_TOP:                             \
                style.border_radius[2] = (SE)->value;                         \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_RADIUS_BOTTOM:                          \
                style.border_radius[3] = (SE)->value;                         \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_BORDER_WIDTH: {                                \
                style.border_width[0] = (SE)->value;                          \
                style.border_width[1] = (SE)->value;                          \
                style.border_width[2] = (SE)->value;                          \
                style.border_width[3] = (SE)->value;                          \
            } break;                                                          \
                                                                              \
            case NEKO_UI_STYLE_BORDER_WIDTH_LEFT:                             \
                style.border_width[0] = (SE)->value;                          \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_WIDTH_RIGHT:                            \
                style.border_width[1] = (SE)->value;                          \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_WIDTH_TOP:                              \
                style.border_width[2] = (SE)->value;                          \
                break;                                                        \
            case NEKO_UI_STYLE_BORDER_WIDTH_BOTTOM:                           \
                style.border_width[3] = (SE)->value;                          \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_DIRECTION:                                     \
                style.direction = (i32)(SE)->value;                           \
                break;                                                        \
            case NEKO_UI_STYLE_ALIGN_CONTENT:                                 \
                style.align_content = (i32)(SE)->value;                       \
                break;                                                        \
            case NEKO_UI_STYLE_JUSTIFY_CONTENT:                               \
                style.justify_content = (i32)(SE)->value;                     \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_SHADOW_X:                                      \
                style.shadow_x = (i32)(SE)->value;                            \
                break;                                                        \
            case NEKO_UI_STYLE_SHADOW_Y:                                      \
                style.shadow_y = (i32)(SE)->value;                            \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_COLOR_BACKGROUND:                              \
                style.colors[NEKO_UI_COLOR_BACKGROUND] = (SE)->color;         \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_BORDER:                                  \
                style.colors[NEKO_UI_COLOR_BORDER] = (SE)->color;             \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_SHADOW:                                  \
                style.colors[NEKO_UI_COLOR_SHADOW] = (SE)->color;             \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_CONTENT:                                 \
                style.colors[NEKO_UI_COLOR_CONTENT] = (SE)->color;            \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_CONTENT_BACKGROUND:                      \
                style.colors[NEKO_UI_COLOR_CONTENT_BACKGROUND] = (SE)->color; \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_CONTENT_BORDER:                          \
                style.colors[NEKO_UI_COLOR_CONTENT_BORDER] = (SE)->color;     \
                break;                                                        \
            case NEKO_UI_STYLE_COLOR_CONTENT_SHADOW:                          \
                style.colors[NEKO_UI_COLOR_CONTENT_SHADOW] = (SE)->color;     \
                break;                                                        \
                                                                              \
            case NEKO_UI_STYLE_FONT:                                          \
                style.font = (SE)->font;                                      \
                break;                                                        \
        }                                                                     \
    } while (0)

    ui_style_t style = ctx->style_sheet->styles[elementid][state];

    // Look for id tag style
    ui_style_list_t* id_styles = NULL;
    ui_style_list_t* cls_styles[NEKO_UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();

    if (desc) {
        char TMP[256] = NEKO_DEFAULT_VAL();

        // ID selector
        neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const u64 id_hash = neko_hash_str64(TMP);
        id_styles = neko_hash_table_exists(ctx->style_sheet->cid_styles, id_hash) ? neko_hash_table_getp(ctx->style_sheet->cid_styles, id_hash) : NULL;

        // Class selectors
        for (u32 i = 0; i < NEKO_UI_CLS_SELECTOR_MAX; ++i) {
            if (desc->classes[i]) {
                neko_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
                const u64 cls_hash = neko_hash_str64(TMP);
                cls_styles[i] = neko_hash_table_exists(ctx->style_sheet->cid_styles, cls_hash) ? neko_hash_table_getp(ctx->style_sheet->cid_styles, cls_hash) : NULL;
            } else
                break;
        }
    }

    // Override with class styles
    if (*cls_styles) {
        for (u32 i = 0; i < NEKO_UI_CLS_SELECTOR_MAX; ++i) {
            if (!cls_styles[i]) break;
            for (u32 s = 0; s < neko_dyn_array_size(cls_styles[i]->styles[state]); ++s) {
                ui_style_element_t* se = &cls_styles[i]->styles[state][s];
                NEKO_UI_APPLY_STYLE(se);
            }
        }
    }

    // Override with id styles
    if (id_styles) {
        for (u32 i = 0; i < neko_dyn_array_size(id_styles->styles[state]); ++i) {
            ui_style_element_t* se = &id_styles->styles[state][i];
            NEKO_UI_APPLY_STYLE(se);
        }
    }

    if (neko_hash_table_exists(ctx->inline_styles, (ui_element_type)elementid)) {
        ui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, (ui_element_type)elementid);
        if (neko_dyn_array_size(iss->styles[state])) {
            // Get last size to apply for styles for this state
            const u32 scz = neko_dyn_array_size(iss->style_counts);
            const u32 ct = state == 0x00 ? iss->style_counts[scz - 3] : state == 0x01 ? iss->style_counts[scz - 2] : iss->style_counts[scz - 1];
            const u32 ssz = neko_dyn_array_size(iss->styles[state]);

            for (u32 i = 0; i < ct; ++i) {
                u32 idx = (ssz - ct + i);
                ui_style_element_t* se = &iss->styles[state][idx];
                NEKO_UI_APPLY_STYLE(se);
            }
        }
    }

    return style;
}

ui_style_t ui_animation_get_blend_style(ui_context_t* ctx, ui_animation_t* anim, const ui_selector_desc_t* desc, i32 elementid) {
    ui_style_t ret = NEKO_DEFAULT_VAL();

    i32 focus_state = anim->focus_state;
    i32 hover_state = anim->hover_state;

    ui_style_t s0 = ui_get_current_element_style(ctx, desc, elementid, anim->start_state);
    ui_style_t s1 = ui_get_current_element_style(ctx, desc, elementid, anim->end_state);

    ui_inline_style_stack_t* iss = NULL;
    if (neko_hash_table_exists(ctx->inline_styles, (ui_element_type)elementid)) {
        iss = neko_hash_table_getp(ctx->inline_styles, (ui_element_type)elementid);
    }

    if (anim->direction == NEKO_UI_ANIMATION_DIRECTION_FORWARD) {
        ret = s1;
    } else {
        ret = s0;
    }

    const ui_animation_property_list_t* list = NULL;
    if (neko_hash_table_exists(ctx->style_sheet->animations, (ui_element_type)elementid)) {
        list = neko_hash_table_getp(ctx->style_sheet->animations, (ui_element_type)elementid);
    }

    const ui_animation_property_list_t* id_list = NULL;
    const ui_animation_property_list_t* cls_list[NEKO_UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();
    bool has_class_animations = false;

    if (desc) {
        char TMP[256] = NEKO_DEFAULT_VAL();

        // ID animations
        if (desc->id) {
            neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
            const u64 id_hash = neko_hash_str64(TMP);
            if (neko_hash_table_exists(ctx->style_sheet->cid_animations, id_hash)) {
                id_list = neko_hash_table_getp(ctx->style_sheet->cid_animations, id_hash);
            }
        }

        // Class animations
        if (*desc->classes) {
            for (u32 i = 0; i < NEKO_UI_CLS_SELECTOR_MAX; ++i) {
                if (!desc->classes[i]) break;
                neko_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
                const u64 cls_hash = neko_hash_str64(TMP);
                if (cls_hash && neko_hash_table_exists(ctx->style_sheet->cid_animations, cls_hash)) {
                    cls_list[i] = neko_hash_table_getp(ctx->style_sheet->cid_animations, cls_hash);
                    has_class_animations = true;
                }
            }
        }
    }

#define NEKO_UI_BLEND_COLOR(TYPE)                                                                \
    do {                                                                                         \
        Color256* c0 = &s0.colors[TYPE];                                                         \
        Color256* c1 = &s1.colors[TYPE];                                                         \
        float r = 255.f * neko_interp_smoothstep((float)c0->r / 255.f, (float)c1->r / 255.f, t); \
        float g = 255.f * neko_interp_smoothstep((float)c0->g / 255.f, (float)c1->g / 255.f, t); \
        float b = 255.f * neko_interp_smoothstep((float)c0->b / 255.f, (float)c1->b / 255.f, t); \
        float a = 255.f * neko_interp_smoothstep((float)c0->a / 255.f, (float)c1->a / 255.f, t); \
        ret.colors[TYPE] = color256((u8)r, (u8)g, (u8)b, (u8)a);                                 \
    } while (0)

#define NEKO_UI_BLEND_VALUE(FIELD, TYPE)                     \
    do {                                                     \
        float v0 = (float)s0.FIELD;                          \
        float v1 = (float)s1.FIELD;                          \
        ret.FIELD = (TYPE)neko_interp_smoothstep(v0, v1, t); \
    } while (0)

#define NEKO_UI_BLEND_PROPERTIES(LIST)                                                                                                                 \
    do {                                                                                                                                               \
        for (u32 i = 0; i < neko_dyn_array_size(LIST); ++i) {                                                                                          \
            const ui_animation_property_t* prop = &LIST[i];                                                                                            \
            float t = 0.f;                                                                                                                             \
            switch (anim->direction) {                                                                                                                 \
                default:                                                                                                                               \
                case NEKO_UI_ANIMATION_DIRECTION_FORWARD: {                                                                                            \
                    t = NEKO_CLAMP(neko_map_range((float)prop->delay, (float)prop->time + (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f); \
                } break;                                                                                                                               \
                case NEKO_UI_ANIMATION_DIRECTION_BACKWARD: {                                                                                           \
                    if (prop->time <= 0.f)                                                                                                             \
                        t = 1.f;                                                                                                                       \
                    else                                                                                                                               \
                        t = NEKO_CLAMP(neko_map_range((float)0.f, (float)anim->max - (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f);      \
                } break;                                                                                                                               \
            }                                                                                                                                          \
                                                                                                                                                       \
            switch (prop->type) {                                                                                                                      \
                case NEKO_UI_STYLE_COLOR_BACKGROUND: {                                                                                                 \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_BACKGROUND);                                                                                     \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_SHADOW: {                                                                                                     \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_SHADOW);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_BORDER: {                                                                                                     \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_BORDER);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_CONTENT: {                                                                                                    \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_CONTENT);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_CONTENT_BACKGROUND: {                                                                                         \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_CONTENT_BACKGROUND);                                                                             \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_CONTENT_SHADOW: {                                                                                             \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_CONTENT_SHADOW);                                                                                 \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_COLOR_CONTENT_BORDER: {                                                                                             \
                    NEKO_UI_BLEND_COLOR(NEKO_UI_COLOR_CONTENT_BORDER);                                                                                 \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_WIDTH: {                                                                                                            \
                    NEKO_UI_BLEND_VALUE(size[0], float);                                                                                               \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_HEIGHT: {                                                                                                           \
                    NEKO_UI_BLEND_VALUE(size[1], float);                                                                                               \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_WIDTH: {                                                                                                     \
                    NEKO_UI_BLEND_VALUE(border_width[0], i16);                                                                                         \
                    NEKO_UI_BLEND_VALUE(border_width[1], i16);                                                                                         \
                    NEKO_UI_BLEND_VALUE(border_width[2], i16);                                                                                         \
                    NEKO_UI_BLEND_VALUE(border_width[3], i16);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_WIDTH_LEFT: {                                                                                                \
                    NEKO_UI_BLEND_VALUE(border_width[0], i16);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_WIDTH_RIGHT: {                                                                                               \
                    NEKO_UI_BLEND_VALUE(border_width[1], i16);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_WIDTH_TOP: {                                                                                                 \
                    NEKO_UI_BLEND_VALUE(border_width[2], i16);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_WIDTH_BOTTOM: {                                                                                              \
                    NEKO_UI_BLEND_VALUE(border_width[3], i16);                                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_RADIUS: {                                                                                                    \
                    NEKO_UI_BLEND_VALUE(border_radius[0], i16);                                                                                        \
                    NEKO_UI_BLEND_VALUE(border_radius[1], i16);                                                                                        \
                    NEKO_UI_BLEND_VALUE(border_radius[2], i16);                                                                                        \
                    NEKO_UI_BLEND_VALUE(border_radius[3], i16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_RADIUS_LEFT: {                                                                                               \
                    NEKO_UI_BLEND_VALUE(border_radius[0], i16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_RADIUS_RIGHT: {                                                                                              \
                    NEKO_UI_BLEND_VALUE(border_radius[1], i16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_RADIUS_TOP: {                                                                                                \
                    NEKO_UI_BLEND_VALUE(border_radius[2], i16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_BORDER_RADIUS_BOTTOM: {                                                                                             \
                    NEKO_UI_BLEND_VALUE(border_radius[3], i16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_MARGIN_BOTTOM: {                                                                                                    \
                    NEKO_UI_BLEND_VALUE(margin[NEKO_UI_MARGIN_BOTTOM], i16);                                                                           \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_MARGIN_TOP: {                                                                                                       \
                    NEKO_UI_BLEND_VALUE(margin[NEKO_UI_MARGIN_TOP], i16);                                                                              \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_MARGIN_LEFT: {                                                                                                      \
                    NEKO_UI_BLEND_VALUE(margin[NEKO_UI_MARGIN_LEFT], i16);                                                                             \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_MARGIN_RIGHT: {                                                                                                     \
                    NEKO_UI_BLEND_VALUE(margin[NEKO_UI_MARGIN_RIGHT], i16);                                                                            \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_MARGIN: {                                                                                                           \
                    NEKO_UI_BLEND_VALUE(margin[0], i16);                                                                                               \
                    NEKO_UI_BLEND_VALUE(margin[1], i16);                                                                                               \
                    NEKO_UI_BLEND_VALUE(margin[2], i16);                                                                                               \
                    NEKO_UI_BLEND_VALUE(margin[3], i16);                                                                                               \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_PADDING_BOTTOM: {                                                                                                   \
                    NEKO_UI_BLEND_VALUE(padding[NEKO_UI_PADDING_BOTTOM], i32);                                                                         \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_PADDING_TOP: {                                                                                                      \
                    NEKO_UI_BLEND_VALUE(padding[NEKO_UI_PADDING_TOP], i32);                                                                            \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_PADDING_LEFT: {                                                                                                     \
                    NEKO_UI_BLEND_VALUE(padding[NEKO_UI_PADDING_LEFT], i32);                                                                           \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_PADDING_RIGHT: {                                                                                                    \
                    NEKO_UI_BLEND_VALUE(padding[NEKO_UI_PADDING_RIGHT], i32);                                                                          \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_PADDING: {                                                                                                          \
                    NEKO_UI_BLEND_VALUE(padding[0], i32);                                                                                              \
                    NEKO_UI_BLEND_VALUE(padding[1], i32);                                                                                              \
                    NEKO_UI_BLEND_VALUE(padding[2], i32);                                                                                              \
                    NEKO_UI_BLEND_VALUE(padding[3], i32);                                                                                              \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_SHADOW_X: {                                                                                                         \
                    NEKO_UI_BLEND_VALUE(shadow_x, i16);                                                                                                \
                } break;                                                                                                                               \
                case NEKO_UI_STYLE_SHADOW_Y: {                                                                                                         \
                    NEKO_UI_BLEND_VALUE(shadow_y, i16);                                                                                                \
                } break;                                                                                                                               \
            }                                                                                                                                          \
        }                                                                                                                                              \
    } while (0)

    // Get final blends
    if (list && !neko_dyn_array_empty(list->properties[anim->end_state])) {
        NEKO_UI_BLEND_PROPERTIES(list->properties[anim->end_state]);
    }

    // Class list
    if (has_class_animations) {
        for (u32 c = 0; c < NEKO_UI_CLS_SELECTOR_MAX; ++c) {
            if (!cls_list[c]) continue;
            if (!neko_dyn_array_empty(cls_list[c]->properties[anim->end_state])) {
                NEKO_UI_BLEND_PROPERTIES(cls_list[c]->properties[anim->end_state]);
            }
        }
    }

    // Id list
    if (id_list && !neko_dyn_array_empty(id_list->properties[anim->end_state])) {
        NEKO_UI_BLEND_PROPERTIES(id_list->properties[anim->end_state]);
    }

    if (iss) {
        NEKO_UI_BLEND_PROPERTIES(iss->animations[anim->end_state]);
    }

    return ret;
}

static void __ui_animation_get_time(ui_context_t* ctx, ui_id id, i32 elementid, const ui_selector_desc_t* desc, ui_inline_style_stack_t* iss, i32 state, ui_animation_t* anim) {
    u32 act = 0, ssz = 0;
    if (iss && neko_dyn_array_size(iss->animations[state])) {
        const u32 scz = neko_dyn_array_size(iss->animation_counts);
        act = state == 0x00 ? iss->animation_counts[scz - 3] : state == 0x01 ? iss->animation_counts[scz - 2] : iss->animation_counts[scz - 1];
        ssz = neko_dyn_array_size(iss->animations[state]);
    }
    ui_animation_property_list_t* cls_list[NEKO_UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();
    const ui_animation_property_list_t* id_list = NULL;
    const ui_animation_property_list_t* list = NULL;
    bool has_class_animations = false;

    if (desc) {
        char TMP[256] = NEKO_DEFAULT_VAL();

        // Id animations
        neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const u64 id_hash = neko_hash_str64(TMP);
        if (neko_hash_table_exists(ctx->style_sheet->cid_animations, id_hash)) {
            id_list = neko_hash_table_getp(ctx->style_sheet->cid_animations, id_hash);
        }

        // Class animations
        for (u32 i = 0; i < NEKO_UI_CLS_SELECTOR_MAX; ++i) {
            if (!desc->classes[i]) break;
            neko_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
            const u64 cls_hash = neko_hash_str64(TMP);
            if (neko_hash_table_exists(ctx->style_sheet->cid_animations, cls_hash)) {
                cls_list[i] = neko_hash_table_getp(ctx->style_sheet->cid_animations, cls_hash);
                has_class_animations = true;
            }
        }
    }

    // Element type animations
    if (neko_hash_table_exists(ctx->style_sheet->animations, (ui_element_type)elementid)) {
        list = neko_hash_table_getp(ctx->style_sheet->animations, (ui_element_type)elementid);
    }

    // Fill properties in order of specificity
    ui_animation_property_t properties[NEKO_UI_STYLE_COUNT] = NEKO_DEFAULT_VAL();
    for (u32 i = 0; i < NEKO_UI_STYLE_COUNT; ++i) {
        properties[i].type = (ui_style_element_type)i;
    }

#define GUI_SET_PROPERTY_TIMES(PROP_LIST)                            \
    do {                                                             \
        for (u32 p = 0; p < neko_dyn_array_size((PROP_LIST)); ++p) { \
            ui_animation_property_t* prop = &(PROP_LIST)[p];         \
            properties[prop->type].time = prop->time;                \
            properties[prop->type].delay = prop->delay;              \
        }                                                            \
    } while (0)

    // Element type list
    if (list) {
        neko_dyn_array(ui_animation_property_t) props = list->properties[state];
        GUI_SET_PROPERTY_TIMES(props);
    }

    // Class list
    if (has_class_animations) {
        for (u32 c = 0; c < NEKO_UI_CLS_SELECTOR_MAX; ++c) {
            if (!cls_list[c]) continue;
            neko_dyn_array(ui_animation_property_t) props = cls_list[c]->properties[state];
            GUI_SET_PROPERTY_TIMES(props);
        }
    }

    // Id list
    if (id_list) {
        neko_dyn_array(ui_animation_property_t) props = id_list->properties[state];
        GUI_SET_PROPERTY_TIMES(props);
    }

    // Inline style list
    if (act && iss) {
        for (u32 a = 0; a < act; ++a) {
            u32 idx = ssz - act + a;
            ui_animation_property_t* ap = &iss->animations[state][idx];
            properties[ap->type].time = ap->time;
            properties[ap->type].delay = ap->delay;
        }
    }

    // Set max times
    for (u32 i = 0; i < NEKO_UI_STYLE_COUNT; ++i) {
        if (properties[i].time > anim->max) anim->max = properties[i].time;
        if (properties[i].delay > anim->delay) anim->delay = properties[i].delay;
    }

    // Finalize time
    anim->max += anim->delay;
    anim->max = NEKO_MAX(anim->max, 5);
}

ui_animation_t* ui_get_animation(ui_context_t* ctx, ui_id id, const ui_selector_desc_t* desc, i32 elementid) {
    ui_animation_t* anim = NULL;

    const bool valid_eid = (elementid >= 0 && elementid < NEKO_UI_ELEMENT_COUNT);

    // Construct new animation if necessary to insert
    if (ctx->state_switch_id == id) {
        if (!neko_hash_table_exists(ctx->animations, id)) {
            ui_animation_t val = NEKO_DEFAULT_VAL();
            neko_hash_table_insert(ctx->animations, id, val);
        }

        ui_inline_style_stack_t* iss = NULL;
        if (neko_hash_table_exists(ctx->inline_styles, (ui_element_type)elementid)) {
            iss = neko_hash_table_getp(ctx->inline_styles, (ui_element_type)elementid);
        }

#define ANIM_GET_TIME(STATE)

        anim = neko_hash_table_getp(ctx->animations, id);
        anim->playing = true;

        i16 focus_state = 0x00;
        i16 hover_state = 0x00;
        i16 direction = 0x00;
        i16 start_state = 0x00;
        i16 end_state = 0x00;
        i16 time_state = 0x00;

        switch (ctx->switch_state) {
            case NEKO_UI_ELEMENT_STATE_OFF_FOCUS: {
                if (ctx->hover == id) {
                    anim->direction = NEKO_UI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = NEKO_UI_ELEMENT_STATE_HOVER;
                    anim->end_state = NEKO_UI_ELEMENT_STATE_FOCUS;
                    time_state = NEKO_UI_ELEMENT_STATE_HOVER;
                    if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                } else {
                    anim->direction = NEKO_UI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
                    anim->end_state = NEKO_UI_ELEMENT_STATE_FOCUS;
                    time_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
                    if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                }
            } break;

            case NEKO_UI_ELEMENT_STATE_ON_FOCUS: {
                anim->direction = NEKO_UI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = NEKO_UI_ELEMENT_STATE_HOVER;
                anim->end_state = NEKO_UI_ELEMENT_STATE_FOCUS;
                time_state = NEKO_UI_ELEMENT_STATE_FOCUS;
                if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;

            case NEKO_UI_ELEMENT_STATE_OFF_HOVER: {
                anim->direction = NEKO_UI_ANIMATION_DIRECTION_BACKWARD;
                anim->start_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
                anim->end_state = NEKO_UI_ELEMENT_STATE_HOVER;
                time_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
                if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = anim->max;
            } break;

            case NEKO_UI_ELEMENT_STATE_ON_HOVER: {
                anim->direction = NEKO_UI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
                anim->end_state = NEKO_UI_ELEMENT_STATE_HOVER;
                time_state = NEKO_UI_ELEMENT_STATE_HOVER;
                if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;
        }

        // Reset state switches and id
        ctx->state_switch_id = 0;
        ctx->switch_state = 0;

        return anim;
    }

    // Return if found
    if (neko_hash_table_exists(ctx->animations, id)) {
        anim = neko_hash_table_getp(ctx->animations, id);
    }

    if (anim && !anim->playing) {
        // This is causing a crash...
        neko_hash_table_erase(ctx->animations, id);
        anim = NULL;
    }

    return anim;
}

void ui_animation_update(ui_context_t* ctx, ui_animation_t* anim) {
    if (ctx->frame == anim->frame) return;

    const i16 dt = (i16)(timing_instance.delta * 1000.f);

    if (anim->playing) {
        // Forward
        switch (anim->direction) {
            default:
            case (NEKO_UI_ANIMATION_DIRECTION_FORWARD): {
                anim->time += dt;
                if (anim->time >= anim->max) {
                    anim->time = anim->max;
                    anim->playing = false;
                }
            } break;

            case (NEKO_UI_ANIMATION_DIRECTION_BACKWARD): {
                anim->time -= dt;
                if (anim->time <= 0) {
                    anim->time = 0;
                    anim->playing = false;
                }
            } break;
        }
    }

    anim->frame = ctx->frame;
}

ui_rect_t ui_rect(float x, float y, float w, float h) {
    ui_rect_t res;
    res.x = x;
    res.y = y;
    res.w = w;
    res.h = h;
    return res;
}

static ui_rect_t ui_expand_rect(ui_rect_t rect, i16 v[4]) { return ui_rect(rect.x - v[0], rect.y - v[2], rect.w + v[0] + v[1], rect.h + v[2] + v[3]); }

static ui_rect_t ui_intersect_rects(ui_rect_t r1, ui_rect_t r2) {
    i32 x1 = (i32)NEKO_MAX(r1.x, r2.x);
    i32 y1 = (i32)NEKO_MAX(r1.y, r2.y);
    i32 x2 = (i32)NEKO_MIN(r1.x + r1.w, r2.x + r2.w);
    i32 y2 = (i32)NEKO_MIN(r1.y + r1.h, r2.y + r2.h);
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    return ui_rect((float)x1, (float)y1, (float)x2 - (float)x1, (float)y2 - (float)y1);
}

static i32 ui_rect_overlaps_vec2(ui_rect_t r, neko_vec2 p) { return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h; }

ui_container_t* ui_get_top_most_container(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return NULL;
    if (split->children[0].type == NEKO_UI_SPLIT_NODE_CONTAINER) return split->children[0].container;
    if (split->children[1].type == NEKO_UI_SPLIT_NODE_CONTAINER) return split->children[1].container;
    ui_container_t* c0 = ui_get_top_most_container(ctx, neko_slot_array_getp(ctx->splits, split->children[0].split));
    ui_container_t* c1 = ui_get_top_most_container(ctx, neko_slot_array_getp(ctx->splits, split->children[1].split));
    if (c0->zindex > c1->zindex) return c0;
    return c1;
}

void ui_bring_split_to_front(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return;

    if (!split->parent) {
        neko_snprintfc(TMP, 256, "!editor_dockspace%zu", (size_t)split);
        ui_id id = ui_get_id(ctx, TMP, 256);
        ui_container_t* cnt = ui_get_container(ctx, TMP);
        // if (cnt) ui_bring_to_front(ctx, cnt);
        // cnt->zindex = 0;
    }

    ui_split_node_t* c0 = &split->children[0];
    ui_split_node_t* c1 = &split->children[1];

    if (c0->type == NEKO_UI_SPLIT_NODE_CONTAINER) {
        ui_bring_to_front(ctx, c0->container);
        // ctx->hover = c0;
    } else {
        ui_split_t* s = neko_slot_array_getp(ctx->splits, c0->split);
        ui_bring_split_to_front(ctx, s);
    }

    if (c1->type == NEKO_UI_SPLIT_NODE_CONTAINER) {
        ui_bring_to_front(ctx, c1->container);
        // ctx->hover = c1;
    } else {
        ui_split_t* s = neko_slot_array_getp(ctx->splits, c1->split);
        ui_bring_split_to_front(ctx, s);
    }
}

static void ui_update_split(ui_context_t* ctx, ui_split_t* split) {
    // Iterate through children, resize them based on size/position
    const ui_rect_t* sr = &split->rect;
    const float ratio = split->ratio;
    switch (split->type) {
        case NEKO_UI_SPLIT_LEFT: {
            if (split->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x + sr->w * ratio, sr->y, sr->w * (1.f - ratio), sr->h);
                ui_update_split(ctx, s);
            }

            if (split->children[NEKO_UI_SPLIT_NODE_CHILD].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w * (ratio), sr->h);
                ui_update_split(ctx, s);
            }
        } break;

        case NEKO_UI_SPLIT_RIGHT: {
            if (split->children[NEKO_UI_SPLIT_NODE_CHILD].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x + sr->w * (1.f - ratio), sr->y, sr->w * (ratio), sr->h);
                ui_update_split(ctx, s);
            }

            if (split->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w * (1.f - ratio), sr->h);
                ui_update_split(ctx, s);
            }

        } break;

        case NEKO_UI_SPLIT_TOP: {
            if (split->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y + sr->h * (ratio), sr->w, sr->h * (1.f - ratio));
                ui_update_split(ctx, s);
            }

            if (split->children[NEKO_UI_SPLIT_NODE_CHILD].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w, sr->h * (ratio));
                ui_update_split(ctx, s);
            }
        } break;

        case NEKO_UI_SPLIT_BOTTOM: {
            if (split->children[NEKO_UI_SPLIT_NODE_CHILD].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y + sr->h * (1.f - ratio), sr->w, sr->h * (ratio));
                ui_update_split(ctx, s);
            }

            if (split->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w, sr->h * (1.f - ratio));
                ui_update_split(ctx, s);
            }
        } break;
    }
}

static ui_split_t* ui_get_root_split_from_split(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return NULL;

    // Cache top root level split
    ui_split_t* root_split = split && split->parent ? neko_slot_array_getp(ctx->splits, split->parent) : split ? split : NULL;
    while (root_split && root_split->parent) {
        root_split = neko_slot_array_getp(ctx->splits, root_split->parent);
    }

    return root_split;
}

static ui_split_t* ui_get_root_split(ui_context_t* ctx, ui_container_t* cnt) {
    ui_split_t* split = ui_get_split(ctx, cnt);
    if (split)
        return ui_get_root_split_from_split(ctx, split);
    else
        return NULL;
}

ui_container_t* ui_get_root_container_from_split(ui_context_t* ctx, ui_split_t* split) {
    ui_split_t* root = ui_get_root_split_from_split(ctx, split);
    ui_split_t* s = root;
    ui_container_t* c = NULL;
    while (s && !c) {
        if (s->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
            s = neko_slot_array_getp(ctx->splits, s->children[NEKO_UI_SPLIT_NODE_PARENT].split);
        } else if (s->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_CONTAINER) {
            c = s->children[NEKO_UI_SPLIT_NODE_PARENT].container;
        } else {
            s = NULL;
        }
    }
    return c;
}

ui_container_t* ui_get_root_container(ui_context_t* ctx, ui_container_t* cnt) {
    ui_container_t* parent = ui_get_parent(ctx, cnt);
    if (parent->split) {
        ui_split_t* root = ui_get_root_split(ctx, parent);
        ui_split_t* s = root;
        ui_container_t* c = NULL;
        while (s && !c) {
            if (s->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_SPLIT) {
                s = neko_slot_array_getp(ctx->splits, s->children[NEKO_UI_SPLIT_NODE_PARENT].split);
            } else if (s->children[NEKO_UI_SPLIT_NODE_PARENT].type == NEKO_UI_SPLIT_NODE_CONTAINER) {
                c = s->children[NEKO_UI_SPLIT_NODE_PARENT].container;
            } else {
                s = NULL;
            }
        }
        return c;
    }

    return parent;
}

ui_tab_bar_t* ui_get_tab_bar(ui_context_t* ctx, ui_container_t* cnt) {
    return ((cnt->tab_bar && cnt->tab_bar < neko_slot_array_size(ctx->tab_bars)) ? neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL);
}

ui_split_t* ui_get_split(ui_context_t* ctx, ui_container_t* cnt) {
    ui_tab_bar_t* tab_bar = ui_get_tab_bar(ctx, cnt);
    ui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;
    ui_split_t* split = cnt->split ? neko_slot_array_getp(ctx->splits, cnt->split) : NULL;

    // Look at split if in tab group
    if (!split && tab_bar) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->split) {
                split = neko_slot_array_getp(ctx->splits, ((ui_container_t*)tab_bar->items[i].data)->split);
            }
        }
    }

    return split;
}

static ui_command_t* ui_push_jump(ui_context_t* ctx, ui_command_t* dst) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_JUMP, sizeof(ui_jumpcommand_t));
    cmd->jump.dst = dst;
    return cmd;
}

static void ui_draw_frame(ui_context_t* ctx, ui_rect_t rect, ui_style_t* style) {
    ui_draw_rect(ctx, rect, style->colors[NEKO_UI_COLOR_BACKGROUND]);

    // draw border
    if (style->colors[NEKO_UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(rect, (i16*)style->border_width), (i16*)style->border_width, style->colors[NEKO_UI_COLOR_BORDER]);
    }
}

static i32 ui_compare_zindex(const void* a, const void* b) { return (*(ui_container_t**)a)->zindex - (*(ui_container_t**)b)->zindex; }

ui_style_t* ui_push_style(ui_context_t* ctx, ui_style_t* style) {
    ui_style_t* save = ctx->style;
    ctx->style = style;
    return save;
}

void ui_push_inline_style(ui_context_t* ctx, ui_element_type elementid, ui_inline_style_desc_t* desc) {
    if (elementid >= NEKO_UI_ELEMENT_COUNT || !desc) {
        return;
    }

    if (!neko_hash_table_exists(ctx->inline_styles, elementid)) {
        ui_inline_style_stack_t v = NEKO_DEFAULT_VAL();
        neko_hash_table_insert(ctx->inline_styles, elementid, v);
    }

    ui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, elementid);
    neko_assert(iss);

    // Counts to keep for popping off
    u32 style_ct[3] = NEKO_DEFAULT_VAL(), anim_ct[3] = NEKO_DEFAULT_VAL();

    if (desc->all.style.data && desc->all.style.size) {
        // Total amount to write for each section
        u32 ct = desc->all.style.size / sizeof(ui_style_element_t);
        style_ct[0] += ct;
        style_ct[1] += ct;
        style_ct[2] += ct;

        // Iterate through all properties, then just push them back into style element list
        for (u32 i = 0; i < ct; ++i) {
            neko_dyn_array_push(iss->styles[0], desc->all.style.data[i]);
            neko_dyn_array_push(iss->styles[1], desc->all.style.data[i]);
            neko_dyn_array_push(iss->styles[2], desc->all.style.data[i]);
        }
    }
    if (desc->all.animation.data && desc->all.animation.size) {
        // Total amount to write for each section
        u32 ct = desc->all.animation.size / sizeof(ui_animation_property_t);
        anim_ct[0] += ct;
        anim_ct[1] += ct;
        anim_ct[2] += ct;

        for (u32 i = 0; i < ct; ++i) {
            neko_dyn_array_push(iss->animations[0], desc->all.animation.data[i]);
            neko_dyn_array_push(iss->animations[1], desc->all.animation.data[i]);
            neko_dyn_array_push(iss->animations[2], desc->all.animation.data[i]);
        }
    }

#define NEKO_UI_COPY_INLINE_STYLE(TYPE, INDEX)                                             \
    do {                                                                                   \
        if (desc->TYPE.style.data && desc->TYPE.style.size) {                              \
            u32 ct = desc->TYPE.style.size / sizeof(ui_style_element_t);                   \
            style_ct[INDEX] += ct;                                                         \
            for (u32 i = 0; i < ct; ++i) {                                                 \
                neko_dyn_array_push(iss->styles[INDEX], desc->TYPE.style.data[i]);         \
            }                                                                              \
        }                                                                                  \
        if (desc->TYPE.animation.data && desc->TYPE.animation.size) {                      \
            u32 ct = desc->TYPE.animation.size / sizeof(ui_animation_property_t);          \
            anim_ct[INDEX] += ct;                                                          \
                                                                                           \
            for (u32 i = 0; i < ct; ++i) {                                                 \
                neko_dyn_array_push(iss->animations[INDEX], desc->TYPE.animation.data[i]); \
            }                                                                              \
        }                                                                                  \
    } while (0)

    // Copy remaining individual styles
    NEKO_UI_COPY_INLINE_STYLE(def, 0);
    NEKO_UI_COPY_INLINE_STYLE(hover, 1);
    NEKO_UI_COPY_INLINE_STYLE(focus, 2);

    // Add final counts
    neko_dyn_array_push(iss->style_counts, style_ct[0]);
    neko_dyn_array_push(iss->style_counts, style_ct[1]);
    neko_dyn_array_push(iss->style_counts, style_ct[2]);

    neko_dyn_array_push(iss->animation_counts, anim_ct[0]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[1]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[2]);
}

void ui_pop_inline_style(ui_context_t* ctx, ui_element_type elementid) {
    if (elementid >= NEKO_UI_ELEMENT_COUNT) {
        return;
    }

    if (!neko_hash_table_exists(ctx->inline_styles, elementid)) {
        return;
    }

    ui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, elementid);
    neko_assert(iss);

    if (neko_dyn_array_size(iss->style_counts) >= 3) {
        const u32 sz = neko_dyn_array_size(iss->style_counts);
        u32 c0 = iss->style_counts[sz - 3];  // default
        u32 c1 = iss->style_counts[sz - 2];  // hover
        u32 c2 = iss->style_counts[sz - 1];  // focus

        // Pop off elements
        if (iss->styles[0]) neko_dyn_array_head(iss->styles[0])->size -= c0;
        if (iss->styles[1]) neko_dyn_array_head(iss->styles[1])->size -= c1;
        if (iss->styles[2]) neko_dyn_array_head(iss->styles[2])->size -= c2;
    }

    if (neko_dyn_array_size(iss->animation_counts) >= 3) {
        const u32 sz = neko_dyn_array_size(iss->animation_counts);
        u32 c0 = iss->animation_counts[sz - 3];  // default
        u32 c1 = iss->animation_counts[sz - 2];  // hover
        u32 c2 = iss->animation_counts[sz - 1];  // focus

        // Pop off elements
        if (iss->animations[0]) neko_dyn_array_head(iss->animations[0])->size -= c0;
        if (iss->animations[1]) neko_dyn_array_head(iss->animations[1])->size -= c1;
        if (iss->animations[2]) neko_dyn_array_head(iss->animations[2])->size -= c2;
    }
}

void ui_pop_style(ui_context_t* ctx, ui_style_t* style) { ctx->style = style; }

static void ui_push_layout(ui_context_t* ctx, ui_rect_t body, neko_vec2 scroll) {
    ui_layout_t layout;
    i32 width = 0;
    memset(&layout, 0, sizeof(layout));
    layout.body = ui_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    layout.max = neko_v2(-0x1000000, -0x1000000);
    layout.direction = ctx->style->direction;
    layout.justify_content = ctx->style->justify_content;
    layout.align_content = ctx->style->align_content;
    memcpy(layout.padding, ctx->style->padding, sizeof(i32) * 4);
    ui_stack_push(ctx->layout_stack, layout);
    ui_layout_row(ctx, 1, &width, 0);
}

static void ui_pop_layout(ui_context_t* ctx) { ui_stack_pop(ctx->layout_stack); }

ui_layout_t* ui_get_layout(ui_context_t* ctx) { return &ctx->layout_stack.items[ctx->layout_stack.idx - 1]; }

static void ui_pop_container(ui_context_t* ctx) {
    ui_container_t* cnt = ui_get_current_container(ctx);
    ui_layout_t* layout = ui_get_layout(ctx);
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;

    /* pop container, layout and id */
    ui_stack_pop(ctx->container_stack);
    ui_stack_pop(ctx->layout_stack);
    ui_pop_id(ctx);
}

#define ui_scrollbar(ctx, cnt, b, cs, x, y, w, h)                                                                                                \
    do {                                                                                                                                         \
        /* only add scrollbar if content size is larger than body */                                                                             \
        i32 maxscroll = (i32)(cs.y - b->h);                                                                                                      \
                                                                                                                                                 \
        if (maxscroll > 0 && b->h > 0) {                                                                                                         \
            ui_rect_t base, thumb;                                                                                                               \
            ui_id id = ui_get_id(ctx, "!scrollbar" #y, 11);                                                                                      \
            const i32 elementid = NEKO_UI_ELEMENT_SCROLL;                                                                                        \
            ui_style_t style = NEKO_DEFAULT_VAL();                                                                                               \
            ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);                                                                   \
                                                                                                                                                 \
            /* Update anim (keep states locally within animation, only way to do this)*/                                                         \
            if (anim) {                                                                                                                          \
                ui_animation_update(ctx, anim);                                                                                                  \
                                                                                                                                                 \
                /* Get blended style based on animation*/                                                                                        \
                style = ui_animation_get_blend_style(ctx, anim, desc, elementid);                                                                \
            } else {                                                                                                                             \
                style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)                                            \
                        : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)                                            \
                                           : ui_get_current_element_style(ctx, desc, elementid, 0x00);                                           \
            }                                                                                                                                    \
                                                                                                                                                 \
            i32 sz = (i32)style.size[0];                                                                                                         \
            if (cs.y > cnt->body.h) {                                                                                                            \
                body->w -= sz;                                                                                                                   \
            }                                                                                                                                    \
            if (cs.x > cnt->body.w) {                                                                                                            \
                body->h -= sz;                                                                                                                   \
            }                                                                                                                                    \
                                                                                                                                                 \
            /* get sizing / positioning */                                                                                                       \
            base = *b;                                                                                                                           \
            base.x = b->x + b->w;                                                                                                                \
            base.w = style.size[0];                                                                                                              \
                                                                                                                                                 \
            /* handle input */                                                                                                                   \
            ui_update_control(ctx, id, base, 0);                                                                                                 \
            if (ctx->focus == id && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {                                                                     \
                cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;                                                                             \
            }                                                                                                                                    \
            /* clamp scroll to limits */                                                                                                         \
            cnt->scroll.y = NEKO_CLAMP(cnt->scroll.y, 0, maxscroll);                                                                             \
            i32 state = ctx->focus == id ? NEKO_UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_UI_ELEMENT_STATE_HOVER : 0x00;                  \
                                                                                                                                                 \
            /* draw base and thumb */                                                                                                            \
            ui_draw_rect(ctx, base, style.colors[NEKO_UI_COLOR_BACKGROUND]);                                                                     \
            /* draw border*/                                                                                                                     \
            if (style.colors[NEKO_UI_COLOR_BORDER].a) {                                                                                          \
                ui_draw_box(ctx, ui_expand_rect(base, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]);  \
            }                                                                                                                                    \
            float pl = ((float)style.padding[NEKO_UI_PADDING_LEFT]);                                                                             \
            float pr = ((float)style.padding[NEKO_UI_PADDING_RIGHT]);                                                                            \
            float pt = ((float)style.padding[NEKO_UI_PADDING_TOP]);                                                                              \
            float pb = ((float)style.padding[NEKO_UI_PADDING_BOTTOM]);                                                                           \
            float w = ((float)base.w - pr);                                                                                                      \
            float x = (float)(base.x + pl);                                                                                                      \
            thumb = base;                                                                                                                        \
            thumb.x = x;                                                                                                                         \
            thumb.w = w;                                                                                                                         \
            thumb.h = NEKO_MAX(style.thumb_size, base.h * b->h / cs.y) - pb;                                                                     \
            thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll + pt;                                                                      \
            ui_draw_rect(ctx, thumb, style.colors[NEKO_UI_COLOR_CONTENT]);                                                                       \
            /* draw border*/                                                                                                                     \
            if (style.colors[NEKO_UI_COLOR_BORDER].a) {                                                                                          \
                ui_draw_box(ctx, ui_expand_rect(thumb, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]); \
            }                                                                                                                                    \
                                                                                                                                                 \
            /* set this as the scroll_target (will get scrolled on mousewheel) */                                                                \
            /* if the mouse is over it */                                                                                                        \
            if (ui_mouse_over(ctx, *b) || ui_mouse_over(ctx, base) || ui_mouse_over(ctx, thumb)) {                                               \
                ctx->scroll_target = cnt;                                                                                                        \
            }                                                                                                                                    \
        }                                                                                                                                        \
    } while (0)

static void ui_scrollbars(ui_context_t* ctx, ui_container_t* cnt, ui_rect_t* body, const ui_selector_desc_t* desc, u64 opt) {
    i32 sz = (i32)ctx->style_sheet->styles[NEKO_UI_ELEMENT_SCROLL][0x00].size[0];
    neko_vec2 cs = cnt->content_size;
    cs.x += ctx->style->padding[NEKO_UI_PADDING_LEFT] * 2;
    cs.y += ctx->style->padding[NEKO_UI_PADDING_TOP] * 2;
    ui_push_clip_rect(ctx, *body);

    /* resize body to make room for scrollbars */
    if (cs.y > cnt->body.h) {
        body->w -= sz;
    }
    if (cs.x > cnt->body.w) {
        body->h -= sz;
    }

    /* to create a horizontal or vertical scrollbar almost-identical code is
    ** used; only the references to `x|y` `w|h` need to be switched */
    ui_scrollbar(ctx, cnt, body, cs, x, y, w, h);

    if (~opt & NEKO_UI_OPT_NOSCROLLHORIZONTAL) {
        ui_scrollbar(ctx, cnt, body, cs, y, x, h, w);
    }

    if (cs.y <= cnt->body.h) {
        cnt->scroll.y = 0;
    }
    if (cs.x <= cnt->body.w) {
        cnt->scroll.x = 0;
    }

    ui_pop_clip_rect(ctx);
}

static void ui_push_container_body(ui_context_t* ctx, ui_container_t* cnt, ui_rect_t body, const ui_selector_desc_t* desc, u64 opt) {
    if (~opt & NEKO_UI_OPT_NOSCROLL) {
        ui_scrollbars(ctx, cnt, &body, desc, opt);
    }
    i32* padding = ctx->style->padding;
    float l = body.x + padding[NEKO_UI_PADDING_LEFT];
    float t = body.y + padding[NEKO_UI_PADDING_TOP];
    float r = body.x + body.w - padding[NEKO_UI_PADDING_RIGHT];
    float b = body.y + body.h - padding[NEKO_UI_PADDING_BOTTOM];

    ui_rect_t rect = ui_rect(l, t, r - l, b - t);
    ui_push_layout(ctx, rect, cnt->scroll);
    cnt->body = body;
}

static void ui_begin_root_container(ui_context_t* ctx, ui_container_t* cnt, u64 opt) {
    ui_stack_push(ctx->container_stack, cnt);

    /* push container to roots list and push head command */
    ui_stack_push(ctx->root_list, cnt);
    cnt->head = ui_push_jump(ctx, NULL);

    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (ui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex) && ~opt & NEKO_UI_OPT_NOHOVER &&
        cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        ctx->next_hover_root = cnt;
    }

    /* clipping is reset here in case a root-container is made within
    ** another root-containers's begin/end block; this prevents the inner
    ** root-container being clipped to the outer */
    ui_stack_push(ctx->clip_stack, ui_unclipped_rect);
}

static void ui_root_container_end(ui_context_t* ctx) {
    /* push tail 'goto' jump command and set head 'skip' command. the final steps
    ** on initing these are done in ui_end() */
    ui_container_t* cnt = ui_get_current_container(ctx);
    cnt->tail = ui_push_jump(ctx, NULL);
    cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;

    /* pop base clip rect and container */
    ui_pop_clip_rect(ctx);
    ui_pop_container(ctx);
}

#define NEKO_UI_COPY_STYLES(DST, SRC, ELEM) \
    do {                                    \
        DST[ELEM][0x00] = SRC[ELEM][0x00];  \
        DST[ELEM][0x01] = SRC[ELEM][0x01];  \
        DST[ELEM][0x02] = SRC[ELEM][0x02];  \
    } while (0)

ui_style_sheet_t ui_style_sheet_create(ui_context_t* ctx, ui_style_sheet_desc_t* desc) {
    // Generate new style sheet based on default element styles
    ui_style_sheet_t style_sheet = NEKO_DEFAULT_VAL();

    // Copy all default styles
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_CONTAINER);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_LABEL);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_TEXT);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_PANEL);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_INPUT);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_BUTTON);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_SCROLL);
    NEKO_UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, NEKO_UI_ELEMENT_IMAGE);

    //  void ui_style_sheet_set_element_styles(ui_style_sheet_t* style_sheet, ui_element_type element, ui_element_state state,
    // ui_style_element_t* styles, size_t size);

#define NEKO_UI_APPLY_STYLE_ELEMENT(ELEMENT, TYPE)                                                                                                      \
    do {                                                                                                                                                \
        if ((ELEMENT).all.style.data) {                                                                                                                 \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_UI_ELEMENT_STATE_NEG, (ELEMENT).all.style.data, (ELEMENT).all.style.size);       \
        } else if ((ELEMENT).def.style.data) {                                                                                                          \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_UI_ELEMENT_STATE_DEFAULT, (ELEMENT).def.style.data, (ELEMENT).def.style.size);   \
        }                                                                                                                                               \
        if ((ELEMENT).hover.style.data) {                                                                                                               \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_UI_ELEMENT_STATE_HOVER, (ELEMENT).hover.style.data, (ELEMENT).hover.style.size); \
        }                                                                                                                                               \
        if ((ELEMENT).focus.style.data) {                                                                                                               \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_UI_ELEMENT_STATE_FOCUS, (ELEMENT).focus.style.data, (ELEMENT).focus.style.size); \
        }                                                                                                                                               \
    } while (0)

    // Iterate through descriptor
    if (desc) {
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->button, NEKO_UI_ELEMENT_BUTTON);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->container, NEKO_UI_ELEMENT_CONTAINER);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->panel, NEKO_UI_ELEMENT_PANEL);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->scroll, NEKO_UI_ELEMENT_SCROLL);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->image, NEKO_UI_ELEMENT_IMAGE);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->label, NEKO_UI_ELEMENT_LABEL);
        NEKO_UI_APPLY_STYLE_ELEMENT(desc->text, NEKO_UI_ELEMENT_TEXT);
    }

#define COPY_ANIM_DATA(TYPE, ELEMENT)                                                                   \
    do {                                                                                                \
        /* Apply animations */                                                                          \
        if (desc->TYPE.all.animation.data) {                                                            \
            i32 cnt = desc->TYPE.all.animation.size / sizeof(ui_animation_property_t);                  \
            if (!neko_hash_table_exists(style_sheet.animations, ELEMENT)) {                             \
                ui_animation_property_list_t v = NEKO_DEFAULT_VAL();                                    \
                neko_hash_table_insert(style_sheet.animations, ELEMENT, v);                             \
            }                                                                                           \
            ui_animation_property_list_t* list = neko_hash_table_getp(style_sheet.animations, ELEMENT); \
            neko_assert(list);                                                                          \
            /* Register animation properties for all */                                                 \
            for (u32 i = 0; i < 3; ++i) {                                                               \
                for (u32 c = 0; c < cnt; ++c) {                                                         \
                    neko_dyn_array_push(list->properties[i], desc->TYPE.all.animation.data[c]);         \
                }                                                                                       \
            }                                                                                           \
        }                                                                                               \
    } while (0)

    // Copy animations
    COPY_ANIM_DATA(button, NEKO_UI_ELEMENT_BUTTON);
    COPY_ANIM_DATA(label, NEKO_UI_ELEMENT_LABEL);
    COPY_ANIM_DATA(scroll, NEKO_UI_ELEMENT_SCROLL);
    COPY_ANIM_DATA(image, NEKO_UI_ELEMENT_IMAGE);
    COPY_ANIM_DATA(panel, NEKO_UI_ELEMENT_PANEL);
    COPY_ANIM_DATA(text, NEKO_UI_ELEMENT_TEXT);
    COPY_ANIM_DATA(container, NEKO_UI_ELEMENT_CONTAINER);

    return style_sheet;
}

void ui_style_sheet_fini(ui_style_sheet_t* ss) {
    // Need to free all animations
    if (!ss || !ss->animations) {
        console_log("Trying to destroy invalid style sheet");
        return;
    }

    for (neko_hash_table_iter it = neko_hash_table_iter_new(ss->animations); neko_hash_table_iter_valid(ss->animations, it); neko_hash_table_iter_advance(ss->animations, it)) {
        ui_animation_property_list_t* list = neko_hash_table_iter_getp(ss->animations, it);
        for (u32 i = 0; i < 3; ++i) {
            neko_dyn_array_free(list->properties[i]);
        }
    }
    neko_hash_table_free(ss->animations);
}

void ui_set_style_sheet(ui_context_t* ctx, ui_style_sheet_t* style_sheet) { ctx->style_sheet = style_sheet ? style_sheet : &ui_default_style_sheet; }

void ui_style_sheet_set_element_styles(ui_style_sheet_t* ss, ui_element_type element, ui_element_state state, ui_style_element_t* styles, size_t size) {
    const u32 count = size / sizeof(ui_style_element_t);
    u32 idx_cnt = 1;
    u32 idx = 0;

    // Switch on state
    switch (state) {
        // Do all
        default:
            idx_cnt = 3;
            break;
        case NEKO_UI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case NEKO_UI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case NEKO_UI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        ui_style_t* cs = &ss->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            ui_style_element_t* se = &styles[i];

            switch (se->type) {
                // Width/Height
                case NEKO_UI_STYLE_WIDTH:
                    cs->size[0] = (float)se->value;
                    break;
                case NEKO_UI_STYLE_HEIGHT:
                    cs->size[1] = (float)se->value;
                    break;

                // Padding
                case NEKO_UI_STYLE_PADDING: {
                    cs->padding[NEKO_UI_PADDING_LEFT] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_TOP] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_RIGHT] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_BOTTOM] = (i32)se->value;
                }
                case NEKO_UI_STYLE_PADDING_LEFT:
                    cs->padding[NEKO_UI_PADDING_LEFT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_TOP:
                    cs->padding[NEKO_UI_PADDING_TOP] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_RIGHT:
                    cs->padding[NEKO_UI_PADDING_RIGHT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_BOTTOM:
                    cs->padding[NEKO_UI_PADDING_BOTTOM] = (i32)se->value;
                    break;

                case NEKO_UI_STYLE_MARGIN: {
                    cs->margin[NEKO_UI_MARGIN_LEFT] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_TOP] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_RIGHT] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_BOTTOM] = (i32)se->value;
                } break;

                case NEKO_UI_STYLE_MARGIN_LEFT:
                    cs->margin[NEKO_UI_MARGIN_LEFT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_TOP:
                    cs->margin[NEKO_UI_MARGIN_TOP] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_RIGHT:
                    cs->margin[NEKO_UI_MARGIN_RIGHT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_BOTTOM:
                    cs->margin[NEKO_UI_MARGIN_BOTTOM] = (i32)se->value;
                    break;

                // Border
                case NEKO_UI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case NEKO_UI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case NEKO_UI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case NEKO_UI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case NEKO_UI_STYLE_DIRECTION:
                    cs->direction = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (i32)se->value;
                    break;

                // Shadow
                case NEKO_UI_STYLE_SHADOW_X:
                    cs->shadow_x = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_SHADOW_Y:
                    cs->shadow_y = (i32)se->value;
                    break;

                // Colors
                case NEKO_UI_STYLE_COLOR_BACKGROUND:
                    cs->colors[NEKO_UI_COLOR_BACKGROUND] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_BORDER:
                    cs->colors[NEKO_UI_COLOR_BORDER] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_SHADOW:
                    cs->colors[NEKO_UI_COLOR_SHADOW] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT:
                    cs->colors[NEKO_UI_COLOR_CONTENT] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[NEKO_UI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[NEKO_UI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[NEKO_UI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case NEKO_UI_STYLE_FONT:
                    cs->font = se->font;
                    break;
            }
        }
    }
}

void ui_set_element_style(ui_context_t* ctx, ui_element_type element, ui_element_state state, ui_style_element_t* style, size_t size) {
    const u32 count = size / sizeof(ui_style_element_t);
    u32 idx_cnt = 1;
    u32 idx = 0;

    // Switch on state
    switch (state) {
        // Do all
        default:
            idx_cnt = 3;
            break;
        case NEKO_UI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case NEKO_UI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case NEKO_UI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        ui_style_t* cs = &ctx->style_sheet->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            ui_style_element_t* se = &style[i];

            switch (se->type) {
                // Width/Height
                case NEKO_UI_STYLE_WIDTH:
                    cs->size[0] = (float)se->value;
                    break;
                case NEKO_UI_STYLE_HEIGHT:
                    cs->size[1] = (float)se->value;
                    break;

                // Padding
                case NEKO_UI_STYLE_PADDING: {
                    cs->padding[NEKO_UI_PADDING_LEFT] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_TOP] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_RIGHT] = (i32)se->value;
                    cs->padding[NEKO_UI_PADDING_BOTTOM] = (i32)se->value;
                }
                case NEKO_UI_STYLE_PADDING_LEFT:
                    cs->padding[NEKO_UI_PADDING_LEFT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_TOP:
                    cs->padding[NEKO_UI_PADDING_TOP] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_RIGHT:
                    cs->padding[NEKO_UI_PADDING_RIGHT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_PADDING_BOTTOM:
                    cs->padding[NEKO_UI_PADDING_BOTTOM] = (i32)se->value;
                    break;

                case NEKO_UI_STYLE_MARGIN: {
                    cs->margin[NEKO_UI_MARGIN_LEFT] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_TOP] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_RIGHT] = (i32)se->value;
                    cs->margin[NEKO_UI_MARGIN_BOTTOM] = (i32)se->value;
                } break;

                case NEKO_UI_STYLE_MARGIN_LEFT:
                    cs->margin[NEKO_UI_MARGIN_LEFT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_TOP:
                    cs->margin[NEKO_UI_MARGIN_TOP] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_RIGHT:
                    cs->margin[NEKO_UI_MARGIN_RIGHT] = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_MARGIN_BOTTOM:
                    cs->margin[NEKO_UI_MARGIN_BOTTOM] = (i32)se->value;
                    break;

                // Border
                case NEKO_UI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case NEKO_UI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case NEKO_UI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case NEKO_UI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case NEKO_UI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case NEKO_UI_STYLE_DIRECTION:
                    cs->direction = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (i32)se->value;
                    break;

                // Shadow
                case NEKO_UI_STYLE_SHADOW_X:
                    cs->shadow_x = (i32)se->value;
                    break;
                case NEKO_UI_STYLE_SHADOW_Y:
                    cs->shadow_y = (i32)se->value;
                    break;

                // Colors
                case NEKO_UI_STYLE_COLOR_BACKGROUND:
                    cs->colors[NEKO_UI_COLOR_BACKGROUND] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_BORDER:
                    cs->colors[NEKO_UI_COLOR_BORDER] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_SHADOW:
                    cs->colors[NEKO_UI_COLOR_SHADOW] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT:
                    cs->colors[NEKO_UI_COLOR_CONTENT] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[NEKO_UI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[NEKO_UI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case NEKO_UI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[NEKO_UI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case NEKO_UI_STYLE_FONT:
                    cs->font = se->font;
                    break;
            }
        }
    }
}

ui_container_t* ui_get_container_ex(ui_context_t* ctx, ui_id id, u64 opt) {
    ui_container_t* cnt;

    /* try to get existing container from pool */
    i32 idx = ui_pool_get(ctx, ctx->container_pool, NEKO_UI_CONTAINERPOOL_SIZE, id);

    if (idx >= 0) {
        if (ctx->containers[idx].open || ~opt & NEKO_UI_OPT_CLOSED) {
            ui_pool_update(ctx, ctx->container_pool, idx);
        }
        return &ctx->containers[idx];
    }

    if (opt & NEKO_UI_OPT_CLOSED) {
        return NULL;
    }

    /* container not found in pool: init new container */
    idx = ui_pool_init(ctx, ctx->container_pool, NEKO_UI_CONTAINERPOOL_SIZE, id);
    cnt = &ctx->containers[idx];
    memset(cnt, 0, sizeof(*cnt));
    cnt->open = 1;
    cnt->id = id;
    cnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE | NEKO_UI_WINDOW_FLAGS_FIRST_INIT;
    ui_bring_to_front(ctx, cnt);

    // neko_println("CONSTRUCTING: %zu", id);

    return cnt;
}

static i32 ui_text_width(neko_asset_font_t* font, const char* text, i32 len) {
    neko_vec2 td = neko_asset_font_text_dimensions(font, text, len);
    return (i32)td.x;
}

static i32 ui_text_height(neko_asset_font_t* font, const char* text, i32 len) {
    return (i32)neko_asset_font_max_height(font);
    neko_vec2 td = neko_asset_font_text_dimensions(font, text, len);
    return (i32)td.y;
}

// Grabs max height for a given font
static i32 ui_font_height(neko_asset_font_t* font) { return (i32)neko_asset_font_max_height(font); }

static neko_vec2 ui_text_dimensions(neko_asset_font_t* font, const char* text, i32 len) {
    neko_vec2 td = neko_asset_font_text_dimensions(font, text, len);
    return td;
}

// =========================== //
// ======== Docking ========== //
// =========================== //

void ui_dock_ex(ui_context_t* ctx, const char* dst, const char* src, i32 split_type, float ratio) {
    ui_hints_t hints = NEKO_DEFAULT_VAL();
    // hints.framebuffer_size = neko_os_framebuffer_sizev(ctx->window_hndl);
    CVec2 fb_size = game_get_window_size();
    hints.framebuffer_size = {fb_size.x, fb_size.y};
    hints.viewport = ui_rect(0.f, 0.f, 0.f, 0.f);
    u32 f = ctx->frame;
    if (!f) ui_begin(ctx, &hints);
    ui_container_t* dst_cnt = ui_get_container(ctx, dst);
    ui_container_t* src_cnt = ui_get_container(ctx, src);
    ui_dock_ex_cnt(ctx, dst_cnt, src_cnt, split_type, ratio);
    if (f != ctx->frame) ui_end(ctx, true);
}

void ui_undock_ex(ui_context_t* ctx, const char* name) {
    ui_container_t* cnt = ui_get_container(ctx, name);
    ui_undock_ex_cnt(ctx, cnt);
}

void ui_set_split(ui_context_t* ctx, ui_container_t* cnt, u32 id) {
    cnt->split = id;
    ui_tab_bar_t* tb = ui_get_tab_bar(ctx, cnt);
    if (tb) {
        for (u32 i = 0; i < tb->size; ++i) {
            ((ui_container_t*)tb->items[i].data)->split = id;
        }
    }
}

ui_container_t* ui_get_parent(ui_context_t* ctx, ui_container_t* cnt) { return (cnt->parent ? cnt->parent : cnt); }

void ui_dock_ex_cnt(ui_context_t* ctx, ui_container_t* child, ui_container_t* parent, i32 split_type, float ratio) {
    // Get top-level parent windows
    parent = ui_get_parent(ctx, parent);
    child = ui_get_parent(ctx, child);

    if (!child || !parent) {
        return;
    }

    if (split_type == NEKO_UI_SPLIT_TAB) {
        // If the parent window has a tab bar, then need to get that tab bar item and add it
        if (parent->tab_bar) {
            neko_println("add to tab bar");

            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, parent->tab_bar);
            neko_assert(tab_bar);

            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar) {
                u32 tbid = child->tab_bar;
                ui_tab_bar_t* ctb = neko_slot_array_getp(ctx->tab_bars, child->tab_bar);
                for (u32 i = 0; i < ctb->size; ++i) {
                    ui_tab_item_t* cti = &tab_bar->items[tab_bar->size];
                    ui_container_t* c = (ui_container_t*)ctb->items[i].data;
                    cti->tab_bar = parent->tab_bar;
                    cti->zindex = tab_bar->size++;
                    cti->idx = cti->zindex;
                    cti->data = c;
                    c->tab_item = cti->idx;
                    c->parent = parent;
                }

                // Free other tab bar
                // neko_slot_array_erase(ctx->tab_bars, tbid);
            } else {
                ui_tab_item_t* tab_item = &tab_bar->items[tab_bar->size];
                tab_item->tab_bar = parent->tab_bar;
                tab_item->zindex = tab_bar->size++;
                tab_item->idx = tab_item->zindex;
                tab_bar->focus = tab_bar->size - 1;
                child->tab_item = tab_item->idx;
            }

            tab_bar->items[child->tab_item].data = child;
            child->rect = parent->rect;
            child->parent = parent;
            child->tab_bar = parent->tab_bar;
        } else {
            // Otherwise, create new tab bar
            console_log("create tab bar");

            // Create tab bar
            ui_tab_bar_t tb = NEKO_DEFAULT_VAL();
            u32 hndl = neko_slot_array_insert(ctx->tab_bars, tb);
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, hndl);
            neko_assert(tab_bar);

            // Create tab items
            ui_tab_item_t* parent_tab_item = &tab_bar->items[tab_bar->size];
            parent_tab_item->zindex = tab_bar->size++;
            parent_tab_item->tab_bar = hndl;

            // Set parent tab item
            parent->tab_item = 0;
            parent_tab_item->data = parent;

            u32 tbid = child->tab_bar;

            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar) {
                ui_tab_bar_t* ctb = neko_slot_array_getp(ctx->tab_bars, child->tab_bar);
                for (u32 i = 0; i < ctb->size; ++i) {
                    ui_tab_item_t* cti = &tab_bar->items[tab_bar->size];
                    ui_container_t* c = (ui_container_t*)ctb->items[i].data;
                    cti->tab_bar = hndl;
                    cti->zindex = tab_bar->size++;
                    cti->idx = cti->zindex;
                    cti->data = c;
                    c->tab_item = cti->idx;
                    c->parent = parent;
                    c->tab_bar = hndl;
                }

                // TODO: This erase is causing a crash.
                // neko_slot_array_erase(ctx->tab_bars, tbid);
            } else {
                ui_tab_item_t* child_tab_item = &tab_bar->items[tab_bar->size];
                child_tab_item->zindex = tab_bar->size++;
                child_tab_item->idx = child_tab_item->zindex;
                child_tab_item->tab_bar = hndl;

                // Set child tab item
                child->tab_item = child_tab_item->idx;
                child_tab_item->data = child;
            }

            tab_bar->focus = 1;
            child->rect = parent->rect;
            child->parent = parent;
            tab_bar->rect = parent->rect;

            parent->tab_bar = hndl;
            child->tab_bar = hndl;

            // Bring everything to front...right?
        }

        ui_split_t* root_split = ui_get_root_split(ctx, parent);
        if (root_split) {
            ui_update_split(ctx, root_split);
            ui_bring_split_to_front(ctx, root_split);
        }
    } else {
        // Cache previous root splits
        ui_split_t* ps = ui_get_root_split(ctx, parent);
        ui_split_t* cs = ui_get_root_split(ctx, child);

        ui_tab_bar_t* tab_bar = ui_get_tab_bar(ctx, parent);

        ui_split_t split = NEKO_DEFAULT_VAL();
        split.type = (ui_split_type)split_type;
        split.ratio = ratio;
        ui_split_node_t c0 = NEKO_DEFAULT_VAL();
        c0.type = NEKO_UI_SPLIT_NODE_CONTAINER;
        c0.container = child;
        ui_split_node_t c1 = NEKO_DEFAULT_VAL();
        c1.type = NEKO_UI_SPLIT_NODE_CONTAINER;
        c1.container = parent;
        split.children[NEKO_UI_SPLIT_NODE_CHILD] = c0;
        split.children[NEKO_UI_SPLIT_NODE_PARENT] = c1;
        split.rect = ps ? ps->rect : parent->rect;
        split.prev_rect = split.rect;

        // Add new split to array
        u32 hndl = neko_slot_array_insert(ctx->splits, split);

        // Get newly inserted split pointer
        ui_split_t* sp = neko_slot_array_getp(ctx->splits, hndl);
        sp->id = hndl;

        // If both parents are null, creating a new split, new nodes, assigning to children's parents
        if (!cs && !ps) {
            neko_println("0");
            parent->split = hndl;
            child->split = hndl;
        }

        // Child has split
        else if (cs && !ps) {
            neko_println("1");
            // If child has split, then the split is different...
            sp->children[NEKO_UI_SPLIT_NODE_CHILD].type = NEKO_UI_SPLIT_NODE_SPLIT;
            sp->children[NEKO_UI_SPLIT_NODE_CHILD].split = cs->id;

            // Child split needs to be set to this parent
            cs->parent = hndl;

            parent->split = hndl;
        }

        // Parent has split
        else if (ps && !cs) {
            neko_println("2");

            // No child to tree to assign, so we can get the raw parent split here
            ps = neko_slot_array_getp(ctx->splits, parent->split);

            // Assign parent split to previous
            sp->parent = ps->id;

            // Fix up references
            if (ps->children[NEKO_UI_SPLIT_NODE_PARENT].container == parent) {
                ps->children[NEKO_UI_SPLIT_NODE_PARENT].type = NEKO_UI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_UI_SPLIT_NODE_PARENT].split = hndl;
            } else {
                ps->children[NEKO_UI_SPLIT_NODE_CHILD].type = NEKO_UI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_UI_SPLIT_NODE_CHILD].split = hndl;
            }

            parent->split = hndl;
            child->split = hndl;
        }

        // Both have splits
        else {
            neko_println("3");

            // Get parent split
            ps = neko_slot_array_getp(ctx->splits, parent->split);

            // Set parent id for new split to parent previous split
            sp->parent = ps->id;

            // Fix up references
            sp->children[NEKO_UI_SPLIT_NODE_CHILD].type = NEKO_UI_SPLIT_NODE_SPLIT;
            sp->children[NEKO_UI_SPLIT_NODE_CHILD].split = cs->id;

            // Need to check which node to replace
            if (ps->children[NEKO_UI_SPLIT_NODE_CHILD].container == parent) {
                ps->children[NEKO_UI_SPLIT_NODE_CHILD].type = NEKO_UI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_UI_SPLIT_NODE_CHILD].split = hndl;
            } else {
                ps->children[NEKO_UI_SPLIT_NODE_PARENT].type = NEKO_UI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_UI_SPLIT_NODE_PARENT].split = hndl;
            }

            cs->parent = hndl;
            parent->split = hndl;
        }

        ui_split_t* root_split = ui_get_root_split(ctx, parent);
        ui_update_split(ctx, root_split);
        ui_bring_split_to_front(ctx, root_split);
    }
}

void ui_undock_ex_cnt(ui_context_t* ctx, ui_container_t* cnt) {
    // If has a tab item idx, need to grab that
    ui_split_t* split = ui_get_split(ctx, cnt);

    // Get root split for container
    ui_split_t* root_split = NULL;

    // Get parent split of this owning split
    ui_split_t* ps = split && split->parent ? neko_slot_array_getp(ctx->splits, split->parent) : NULL;

    if (cnt->tab_bar) {
        // Get parent container for this container
        ui_container_t* parent = ui_get_parent(ctx, cnt);

        // Check if split
        if (parent->split) {
            // No split, so just do stuff normally...
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            ui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

            if (tab_bar->size > 2) {
                // Get index
                u32 idx = 0;
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (tab_bar->items[i].data == cnt) {
                        idx = i;
                        break;
                    }
                }

                // Swap all the way down the chain
                for (u32 i = idx; i < tab_bar->size; ++i) {
                    ui_tab_item_swap(ctx, (ui_container_t*)tab_bar->items[i].data, +1);
                }

                // Swap windows as well
                ((ui_container_t*)(tab_item->data))->tab_item = tab_item->idx;

                // Set focus to first window
                tab_bar->focus = idx ? idx - 1 : idx;
                neko_assert(tab_bar->items[tab_bar->focus].data != cnt);

                // Set split for focus
                if (parent == cnt) {
                    // Set parent for other containers (assuming parent was removed)
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        ui_container_t* c = (ui_container_t*)tab_bar->items[i].data;
                        c->parent = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                        tab_bar->items[i].idx = i;
                        tab_bar->items[i].zindex = i;
                    }

                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;
                    fcnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[0].container == cnt) {
                        split->children[0].container = fcnt;
                    } else {
                        split->children[1].container = fcnt;
                    }
                }

                // Set size
                tab_bar->size--;
            }
            // Destroy tab
            else {
                u32 tbid = tab_item->tab_bar;

                // Get index
                u32 idx = 0;
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (tab_bar->items[i].data == cnt) {
                        idx = i;
                        break;
                    }
                }

                // Swap all the way down the chain
                for (u32 i = idx; i < tab_bar->size; ++i) {
                    ui_tab_item_swap(ctx, (ui_container_t*)tab_bar->items[i].data, +1);
                }

                for (u32 i = 0; i < tab_bar->size; ++i) {
                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect;
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE;
                }

                // Fix up split reference
                if (parent == cnt) {
                    tab_bar->focus = idx ? idx - 1 : idx;

                    neko_assert((ui_container_t*)tab_bar->items[tab_bar->focus].data != cnt);

                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[0].container == parent) {
                        split->children[0].container = fcnt;
                    } else {
                        split->children[1].container = fcnt;
                    }
                }

                // neko_slot_array_erase(ctx->tab_bars, tbid);
            }

            // Remove tab index from container
            cnt->tab_item = 0x00;
            cnt->tab_bar = 0x00;
            // Remove parent
            cnt->parent = NULL;
            // Set split to 0
            cnt->split = 0x00;

            ui_bring_to_front(ctx, cnt);

        } else {
            // No split, so just do stuff normally...
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            ui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

            // Set next available window to visible/focused and rearrange all tab item zindices
            if (tab_bar->size > 2) {
                u32 idx = 0;
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (tab_bar->items[i].data == cnt) {
                        idx = i;
                        break;
                    }
                }

                // Swap all the way down the chain
                for (u32 i = idx; i < tab_bar->size; ++i) {
                    ui_tab_item_swap(ctx, (ui_container_t*)tab_bar->items[i].data, +1);
                }

                // Set size
                tab_bar->size--;

                // Set focus to first window
                tab_bar->focus = idx ? idx - 1 : idx;

                // Set parent for other containers
                if (parent == cnt) {
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        ui_container_t* c = (ui_container_t*)tab_bar->items[i].data;
                        c->parent = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                    }
                }
            }
            // Only 2 windows left in tab bar
            else {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect;
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE;
                }

                tab_bar->size = 0;

                // Destroy tab bar, reset windows
                // neko_slot_array_erase(ctx->tab_bars, tab_item->tab_bar);
            }

            // Remove tab index from container
            cnt->tab_item = 0x00;
            cnt->tab_bar = 0x00;
            // Remove parent
            cnt->parent = NULL;

            ui_bring_to_front(ctx, cnt);
        }
    } else {
        // Rmove split reference from container
        cnt->split = 0x00;

        ui_split_node_t* remain_node = split->children[NEKO_UI_SPLIT_NODE_CHILD].container == cnt    ? &split->children[NEKO_UI_SPLIT_NODE_PARENT]
                                       : split->children[NEKO_UI_SPLIT_NODE_PARENT].container == cnt ? &split->children[NEKO_UI_SPLIT_NODE_CHILD]
                                                                                                     : NULL;

        neko_assert(remain_node);

        // Set child split in prev container split to split container parent
        if (ps) {
            u32 node_id = ps->children[NEKO_UI_SPLIT_NODE_CHILD].split == split->id ? NEKO_UI_SPLIT_NODE_CHILD : NEKO_UI_SPLIT_NODE_PARENT;
            ui_split_node_t* fix_node = &ps->children[node_id];
            *fix_node = *remain_node;
            switch (fix_node->type) {
                case NEKO_UI_SPLIT_NODE_CONTAINER: {
                    ui_container_t* remcnt = fix_node->container;
                    remcnt->split = ps->id;
                } break;

                case NEKO_UI_SPLIT_NODE_SPLIT: {
                    ui_split_t* remsplit = neko_slot_array_getp(ctx->splits, fix_node->split);
                    remsplit->parent = ps->id;
                } break;
            }

            root_split = ui_get_root_split_from_split(ctx, ps);
        }
        // Otherwise, we were root dock and have to treat that case for remaining nodes
        else {
            switch (remain_node->type) {
                case NEKO_UI_SPLIT_NODE_CONTAINER: {
                    ui_container_t* remcnt = remain_node->container;
                    remcnt->rect = split->rect;
                    remcnt->split = 0x00;
                    root_split = ui_get_root_split(ctx, remcnt);
                } break;

                case NEKO_UI_SPLIT_NODE_SPLIT: {
                    ui_split_t* remsplit = neko_slot_array_getp(ctx->splits, remain_node->split);
                    remsplit->rect = split->rect;
                    remsplit->parent = 0x00;
                    root_split = ui_get_root_split_from_split(ctx, remsplit);
                } break;
            }
        }

        // Erase split
        neko_slot_array_erase(ctx->splits, split->id);

        // Update
        if (root_split) ui_update_split(ctx, root_split);
        if (root_split) ui_bring_split_to_front(ctx, root_split);
        ui_bring_to_front(ctx, cnt);
    }
}

// ============================= //
// ========= Main API ========== //
// ============================= //

#define NEKO_UI_COPY_STYLE(DST, SRC)              \
    do {                                          \
        DST[NEKO_UI_ELEMENT_STATE_DEFAULT] = SRC; \
        DST[NEKO_UI_ELEMENT_STATE_HOVER] = SRC;   \
        DST[NEKO_UI_ELEMENT_STATE_FOCUS] = SRC;   \
    } while (0)

static void ui_init_default_styles(ui_context_t* ctx) {
    // Set up main default style
    ui_default_style.font = neko_idraw_default_font();
    ui_default_style.size[0] = 68.f;
    ui_default_style.size[1] = 18.f;
    ui_default_style.spacing = 2;
    ui_default_style.indent = 10;
    ui_default_style.title_height = 20;
    ui_default_style.scrollbar_size = 5;
    ui_default_style.thumb_size = 5;

    // Initialize all default styles
    NEKO_UI_COPY_STYLE(ui_default_container_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_button_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_text_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_label_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_panel_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_input_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_scroll_style, ui_default_style);
    NEKO_UI_COPY_STYLE(ui_default_image_style, ui_default_style);

    ui_style_t* style = NULL;

    // button
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_button_style[i];
        style->justify_content = NEKO_UI_JUSTIFY_CENTER;
    }
    ui_default_button_style[NEKO_UI_ELEMENT_STATE_DEFAULT].colors[NEKO_UI_COLOR_BACKGROUND] = color256(35, 35, 35, 255);
    ui_default_button_style[NEKO_UI_ELEMENT_STATE_HOVER].colors[NEKO_UI_COLOR_BACKGROUND] = color256(40, 40, 40, 255);
    ui_default_button_style[NEKO_UI_ELEMENT_STATE_FOCUS].colors[NEKO_UI_COLOR_BACKGROUND] = color256(0, 214, 121, 255);

    // panel
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_panel_style[i];
        style->colors[NEKO_UI_COLOR_BACKGROUND] = color256(30, 30, 30, 160);
        style->size[1] = 19;
    }

    // input
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_input_style[i];
        style->colors[NEKO_UI_COLOR_BACKGROUND] = color256(20, 20, 20, 255);
        style->size[1] = 19;
    }

    // text
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_text_style[i];
        style->colors[NEKO_UI_COLOR_BACKGROUND] = color256(0, 0, 0, 0);
        style->colors[NEKO_UI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
    }

    // label
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_label_style[i];
        style->colors[NEKO_UI_COLOR_BACKGROUND] = color256(0, 0, 0, 0);
        style->colors[NEKO_UI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
        style->size[1] = 19;
    }

    // scroll
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_scroll_style[i];
        style->size[0] = 10;
        style->padding[NEKO_UI_PADDING_RIGHT] = 4;
    }

    style = &ui_default_scroll_style[NEKO_UI_ELEMENT_STATE_DEFAULT];
    style->colors[NEKO_UI_COLOR_BACKGROUND] = color256(22, 22, 22, 255);
    style->colors[NEKO_UI_COLOR_CONTENT] = color256(255, 255, 255, 100);

#define NEKO_UI_COPY(DST, SRC) \
    do {                       \
        DST[0x00] = SRC[0x00]; \
        DST[0x01] = SRC[0x01]; \
        DST[0x02] = SRC[0x02]; \
    } while (0)

    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_CONTAINER], ui_default_container_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_LABEL], ui_default_label_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_TEXT], ui_default_text_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_PANEL], ui_default_panel_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_INPUT], ui_default_input_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_BUTTON], ui_default_button_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_SCROLL], ui_default_scroll_style);
    NEKO_UI_COPY(ui_default_style_sheet.styles[NEKO_UI_ELEMENT_IMAGE], ui_default_image_style);

    ctx->style_sheet = &ui_default_style_sheet;
    ctx->style = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][0x00];
}

// static char button_map[256] = NEKO_DEFAULT_VAL();
// static char key_map[512] = NEKO_DEFAULT_VAL();

INPUT_WRAP_DEFINE(ui);

ui_context_t ui_new(u32 window_hndl) {
    ui_context_t ctx = NEKO_DEFAULT_VAL();
    ui_init(&ctx, window_hndl);
    return ctx;
}

void ui_init(ui_context_t* ctx, u32 window_hndl) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->gui_idraw = neko_immediate_draw_new();
    ctx->overlay_draw_list = neko_immediate_draw_new();
    ui_init_default_styles(ctx);
    ctx->window_hndl = window_hndl;
    ctx->last_zindex = 1000;
    neko_slot_array_reserve(ctx->splits, NEKO_UI_NEKO_UI_SPLIT_SIZE);
    ui_split_t split = NEKO_DEFAULT_VAL();
    neko_slot_array_insert(ctx->splits, split);  // First item is set for 0x00 invalid
    neko_slot_array_reserve(ctx->tab_bars, NEKO_UI_NEKO_UI_TAB_SIZE);
    ui_tab_bar_t tb = NEKO_DEFAULT_VAL();
    neko_slot_array_insert(ctx->tab_bars, tb);

    // button_map[NEKO_MOUSE_LBUTTON & 0xff] = NEKO_UI_MOUSE_LEFT;
    // button_map[NEKO_MOUSE_RBUTTON & 0xff] = NEKO_UI_MOUSE_RIGHT;
    // button_map[NEKO_MOUSE_MBUTTON & 0xff] = NEKO_UI_MOUSE_MIDDLE;

    // key_map[NEKO_KEYCODE_LEFT_SHIFT & 0xff] = NEKO_UI_KEY_SHIFT;
    // key_map[NEKO_KEYCODE_RIGHT_SHIFT & 0xff] = NEKO_UI_KEY_SHIFT;
    // key_map[NEKO_KEYCODE_LEFT_CONTROL & 0xff] = NEKO_UI_KEY_CTRL;
    // key_map[NEKO_KEYCODE_RIGHT_CONTROL & 0xff] = NEKO_UI_KEY_CTRL;
    // key_map[NEKO_KEYCODE_LEFT_ALT & 0xff] = NEKO_UI_KEY_ALT;
    // key_map[NEKO_KEYCODE_RIGHT_ALT & 0xff] = NEKO_UI_KEY_ALT;
    // key_map[NEKO_KEYCODE_ENTER & 0xff] = NEKO_UI_KEY_RETURN;
    // key_map[NEKO_KEYCODE_BACKSPACE & 0xff] = NEKO_UI_KEY_BACKSPACE;

#if 1
    input_add_key_down_callback(ui_key_down);
    input_add_key_up_callback(ui_key_up);
    input_add_char_down_callback(ui_char_down);
    input_add_mouse_down_callback(ui_mouse_down);
    input_add_mouse_up_callback(ui_mouse_up);
    input_add_mouse_move_callback(ui_mouse_move);
    input_add_scroll_callback(ui_scroll);
#endif
}

void ui_init_font_stash(ui_context_t* ctx, ui_font_stash_desc_t* desc) {
    neko_hash_table_clear(ctx->font_stash);
    u32 ct = sizeof(ui_font_desc_t) / desc->size;
    for (u32 i = 0; i < ct; ++i) {
        neko_hash_table_insert(ctx->font_stash, neko_hash_str64(desc->fonts[i].key), desc->fonts[i].font);
    }
}

ui_context_t ui_context_new(u32 window_hndl) {
    ui_context_t gui = NEKO_DEFAULT_VAL();
    ui_init(&gui, window_hndl);
    return gui;
}

void ui_free(ui_context_t* ctx) {
    /*
typedef struct ui_context_t
{
    // Core state
    ui_style_t* style;              // Active style
    ui_style_sheet_t* style_sheet;  // Active style sheet
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    ui_id state_switch_id;          // Id that had a state switch
    i32 switch_state;
    ui_id lock_focus;
    i32 last_hover_state;
    i32 last_focus_state;
    ui_id prev_hover;
    ui_id prev_focus;
    ui_rect_t last_rect;
    i32 last_zindex;
    i32 updated_focus;
    i32 frame;
    neko_vec2 framebuffer_size;
    ui_rect_t viewport;
    ui_container_t* active_root;
    ui_container_t* hover_root;
    ui_container_t* next_hover_root;
    ui_container_t* scroll_target;
    ui_container_t* focus_root;
    ui_container_t* next_focus_root;
    ui_container_t* dockable_root;
    ui_container_t* prev_dockable_root;
    ui_container_t* docked_root;
    ui_container_t* undock_root;
    ui_split_t*     focus_split;
    ui_split_t*     next_hover_split;
    ui_split_t*     hover_split;
    ui_id           next_lock_hover_id;
    ui_id           lock_hover_id;
    char number_edit_buf[NEKO_UI_MAX_FMT];
    ui_id number_edit;
    ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(ui_request_t) requests;

    // Stacks
    ui_stack(u8, NEKO_UI_COMMANDLIST_SIZE) command_list;
    ui_stack(ui_container_t*, NEKO_UI_ROOTLIST_SIZE) root_list;
    ui_stack(ui_container_t*, NEKO_UI_CONTAINERSTACK_SIZE) container_stack;
    ui_stack(ui_rect_t, NEKO_UI_CLIPSTACK_SIZE) clip_stack;
    ui_stack(ui_id, NEKO_UI_IDSTACK_SIZE) id_stack;
    ui_stack(ui_layout_t, NEKO_UI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(ui_element_type, ui_inline_style_stack_t) inline_styles;

    // Retained state pools
    ui_pool_item_t container_pool[NEKO_UI_CONTAINERPOOL_SIZE];
    ui_container_t containers[NEKO_UI_CONTAINERPOOL_SIZE];
    ui_pool_item_t treenode_pool[NEKO_UI_TREENODEPOOL_SIZE];

    neko_slot_array(ui_split_t) splits;
    neko_slot_array(ui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    i32 mouse_down;
    i32 mouse_pressed;
    i32 key_down;
    i32 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gsi;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(ui_id, ui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, neko_asset_font_t*) font_stash;

    // Callbacks
    struct {
        ui_on_draw_button_callback button;
    } callbacks;

} ui_context_t;
*/
    neko_hash_table_free(ctx->font_stash);
    neko_immediate_draw_free(&ctx->gui_idraw);
    neko_immediate_draw_free(&ctx->overlay_draw_list);
    neko_hash_table_free(ctx->animations);
    neko_slot_array_free(ctx->splits);
    neko_slot_array_free(ctx->tab_bars);
    neko_hash_table_free(ctx->inline_styles);

    // Inline style stacks
    for (neko_hash_table_iter it = neko_hash_table_iter_new(ctx->inline_styles); neko_hash_table_iter_valid(ctx->inline_styles, it); neko_hash_table_iter_advance(ctx->inline_styles, it)) {
        ui_inline_style_stack_t* stack = neko_hash_table_iter_getp(ctx->inline_styles, it);
        for (u32 i = 0; i < 3; ++i) {
            neko_dyn_array_free(stack->styles[i]);
            neko_dyn_array_free(stack->animations[i]);
        }
        neko_dyn_array_free(stack->style_counts);      // amount of styles to pop off at "top of stack" for each state
        neko_dyn_array_free(stack->animation_counts);  // amount of animations to pop off at "top of stack" for each state
    }

    neko_dyn_array_free(ctx->requests);
}

static void ui_draw_splits(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return;

    ui_split_t* root_split = ui_get_root_split_from_split(ctx, split);

    // Draw split
    const ui_rect_t* sr = &split->rect;
    neko_vec2 hd = neko_v2(sr->w * 0.5f, sr->h * 0.5f);
    ui_rect_t r = NEKO_DEFAULT_VAL();
    Color256 c = color256(0, 0, 0, 0);
    const float ratio = split->ratio;
    ui_container_t* top = ui_get_top_most_container(ctx, root_split);
    ui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root : ctx->next_hover_root ? ctx->next_hover_root : NULL;
    bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;
    valid_hover = false;
    bool valid = true;

    split->frame = ctx->frame;
    root_split->frame = ctx->frame;

    bool can_draw = true;
    for (u32 i = 0; i < 2; ++i) {
        if (split->children[i].type == NEKO_UI_SPLIT_NODE_CONTAINER && can_draw) {
            ui_container_t* cnt = split->children[i].container;

            // Don't draw split if this container belongs to a editor_dockspace
            if (cnt->opt & NEKO_UI_OPT_DOCKSPACE) {
                can_draw = false;
                continue;
            }

            switch (split->type) {
                case NEKO_UI_SPLIT_LEFT: {
                    r = ui_rect(sr->x + sr->w * ratio - NEKO_UI_SPLIT_SIZE * 0.5f, sr->y + NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE, sr->h - 2.f * NEKO_UI_SPLIT_SIZE);
                } break;

                case NEKO_UI_SPLIT_RIGHT: {
                    r = ui_rect(sr->x + sr->w * (1.f - ratio) - NEKO_UI_SPLIT_SIZE * 0.5f, sr->y + NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE, sr->h - 2.f * NEKO_UI_SPLIT_SIZE);
                } break;

                case NEKO_UI_SPLIT_TOP: {
                    r = ui_rect(sr->x + NEKO_UI_SPLIT_SIZE, sr->y + sr->h * (ratio)-NEKO_UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE);
                } break;

                case NEKO_UI_SPLIT_BOTTOM: {
                    r = ui_rect(sr->x + NEKO_UI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - NEKO_UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE);
                } break;
            }

            i16 exp[4] = {1, 1, 1, 1};
            ui_rect_t expand = ui_expand_rect(r, exp);
            bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && ui_rect_overlaps_vec2(expand, ctx->mouse_pos) && !ctx->lock_hover_id;
            if (hover) ctx->next_hover_split = split;
            if (hover && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                if (!ctx->focus_split) ctx->focus_split = split;
            }
            bool active = ctx->focus_split == split;
            if (active && valid) {
                ctx->next_hover_root = top;
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_SPLIT_RATIO;
                req.split = split;
                neko_dyn_array_push(ctx->requests, req);
            }

            if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                Color256 bc = ctx->focus_split == split ? ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_FOCUS].colors[NEKO_UI_COLOR_BACKGROUND]
                                                        : ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_HOVER].colors[NEKO_UI_COLOR_BACKGROUND];
                ui_draw_rect(ctx, r, bc);
                can_draw = false;
            }
        } else if (split->children[i].type == NEKO_UI_SPLIT_NODE_SPLIT) {
            if (can_draw) {
                switch (split->type) {
                    case NEKO_UI_SPLIT_LEFT: {
                        r = ui_rect(sr->x + sr->w * ratio - NEKO_UI_SPLIT_SIZE * 0.5f, sr->y + NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE, sr->h - 2.f * NEKO_UI_SPLIT_SIZE);
                    } break;

                    case NEKO_UI_SPLIT_RIGHT: {
                        r = ui_rect(sr->x + sr->w * (1.f - ratio) - NEKO_UI_SPLIT_SIZE * 0.5f, sr->y + NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE, sr->h - 2.f * NEKO_UI_SPLIT_SIZE);
                    } break;

                    case NEKO_UI_SPLIT_TOP: {
                        r = ui_rect(sr->x + NEKO_UI_SPLIT_SIZE, sr->y + sr->h * (ratio)-NEKO_UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE);
                    } break;

                    case NEKO_UI_SPLIT_BOTTOM: {
                        r = ui_rect(sr->x + NEKO_UI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - NEKO_UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_UI_SPLIT_SIZE, NEKO_UI_SPLIT_SIZE);
                    } break;
                }

                i16 exp[] = {1, 1, 1, 1};
                ui_rect_t expand = ui_expand_rect(r, exp);
                bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && ui_rect_overlaps_vec2(expand, ctx->mouse_pos);
                if (hover) ctx->next_hover_split = split;
                if (hover && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                    if (!ctx->focus_split) ctx->focus_split = split;
                }
                bool active = ctx->focus_split == split;
                if (active) {
                    ctx->next_hover_root = top;
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = NEKO_UI_SPLIT_RATIO;
                    req.split = split;
                    neko_dyn_array_push(ctx->requests, req);
                }

                if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                    Color256 bc = ctx->focus_split == split ? ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_FOCUS].colors[NEKO_UI_COLOR_BACKGROUND]
                                                            : ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_HOVER].colors[NEKO_UI_COLOR_BACKGROUND];
                    ui_draw_rect(ctx, r, bc);
                    can_draw = false;
                }
            }

            ui_split_t* child = neko_slot_array_getp(ctx->splits, split->children[i].split);
            ui_draw_splits(ctx, child);
        }
    }
    if (ctx->focus_split == split && ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
        ctx->focus_split = NULL;
    }
}

static void ui_get_split_lowest_zindex(ui_context_t* ctx, ui_split_t* split, i32* index) {
    if (!split) return;

    if (split->children[0].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[0].container->zindex < *index) {
        *index = split->children[0].container->zindex;
    }
    if (split->children[0].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[0].container->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[0].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((ui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[1].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[1].container->zindex < *index) {
        *index = split->children[1].container->zindex;
    }
    if (split->children[1].type == NEKO_UI_SPLIT_NODE_CONTAINER && split->children[1].container->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[1].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((ui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[0].type == NEKO_UI_SPLIT_NODE_SPLIT) {
        ui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[0].split), index);
    }

    if (split->children[1].type == NEKO_UI_SPLIT_NODE_SPLIT) {
        ui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[1].split), index);
    }
}

void ui_begin(ui_context_t* ctx, const ui_hints_t* hints) {
    ui_hints_t default_hints = NEKO_DEFAULT_VAL();
    // default_hints.framebuffer_size = neko_os_framebuffer_sizev(ctx->window_hndl);
    CVec2 fb_size = game_get_window_size();
    default_hints.framebuffer_size = {fb_size.x, fb_size.y};
    default_hints.viewport = ui_rect(0.f, 0.f, default_hints.framebuffer_size.x, default_hints.framebuffer_size.y);
    ui_hints_t hint = hints ? *hints : default_hints;

    // Set up viewport
    ui_rect_t vp = hint.viewport;
    if (~hint.flags & NEKO_UI_HINT_FLAG_NO_INVERT_Y) {
        vp.y = hint.viewport.y;
    } else {
        vp.y = hint.framebuffer_size.y - hint.viewport.h - hint.viewport.y;
    }

    // 捕获事件信息
    auto mouse_pos_tmp = input_get_mouse_pos_pixels_fix();
    neko_vec2 mouse_pos = {mouse_pos_tmp.x, mouse_pos_tmp.y};

    // Check for scale and bias
    if (hint.flags & NEKO_UI_HINT_FLAG_NO_SCALE_BIAS_MOUSE) {
        float px = (mouse_pos.x - vp.x) / vp.w;
        float py = (mouse_pos.y - vp.y) / vp.h;
        float xv = vp.w * px;
        float yv = vp.h * py;
        mouse_pos = neko_v2(xv, yv);
    } else {
        neko_vec2 fb_vp_ratio = neko_v2(hint.framebuffer_size.x / vp.w, hint.framebuffer_size.y / vp.h);
        float px = mouse_pos.x - (vp.x * fb_vp_ratio.x);
        float py = mouse_pos.y + (vp.y * fb_vp_ratio.y);
        float xv = px / fb_vp_ratio.x;
        float yv = py / fb_vp_ratio.y;
        mouse_pos = neko_v2(xv, yv);
    }

#if 0
    neko_os_event_t evt = NEKO_DEFAULT_VAL();
    while (neko_os_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PF_EVENT_MOUSE: {
                switch (evt.mouse.action) {
                    case NEKO_PF_MOUSE_MOVE: {
                        // ctx->mouse_pos = evt.mouse.move;
                    } break;

                    case NEKO_PF_MOUSE_WHEEL: {
                        ui_input_scroll(ctx, (i32)(-evt.mouse.wheel.x * 30.f), (i32)(-evt.mouse.wheel.y * 30.f));
                    } break;

                    case NEKO_PF_MOUSE_BUTTON_DOWN:
                    case NEKO_PF_MOUSE_BUTTON_PRESSED: {
                        i32 code = 1 << evt.mouse.button;
                        ui_input_mousedown(ctx, (i32)mouse_pos.x, (i32)mouse_pos.y, code);
                    } break;

                    case NEKO_PF_MOUSE_BUTTON_RELEASED: {
                        i32 code = 1 << evt.mouse.button;
                        ui_input_mouseup(ctx, (i32)mouse_pos.x, (i32)mouse_pos.y, code);
                    } break;

                    case NEKO_PF_MOUSE_ENTER: {
                        // If there are user callbacks, could trigger them here
                    } break;

                    case NEKO_PF_MOUSE_LEAVE: {
                        // If there are user callbacks, could trigger them here
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PF_EVENT_TEXT: {
                // Input text
                char txt[2] = {(char)(evt.text.codepoint & 255), 0};
                ui_input_text(ctx, txt);
            } break;

            case NEKO_PF_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PF_KEY_DOWN:
                    case NEKO_PF_KEY_PRESSED: {
                        ui_input_keydown(ctx, key_map[evt.key.keycode & 511]);
                    } break;

                    case NEKO_PF_KEY_RELEASED: {
                        ui_input_keyup(ctx, key_map[evt.key.keycode & 511]);
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PF_EVENT_WINDOW: {
                switch (evt.window.action) {
                    default:
                        break;
                }

            } break;

            default:
                break;
        }
    }
#endif

    for (INPUT_WRAP_event e; input_wrap_next_e(&ui_input_queue, &e); input_wrap_free_e(&e))
        if (e.type) switch (e.type) {
                default:
                    break;
                // case INPUT_WRAP_FILE_DROPPED:
                //     rc += lt_emit_event(L, "filedropped", "sdd", e.file.paths[0], lt_mx, lt_my);
                //     break;
                case INPUT_WRAP_KEY_PRESSED:
                case INPUT_WRAP_KEY_REPEATED:
                    ui_input_keydown(ctx, e.keyboard.key);
                    break;
                case INPUT_WRAP_KEY_RELEASED:
                    ui_input_keyup(ctx, e.keyboard.key);
                    break;
                case INPUT_WRAP_CODEPOINT_INPUT:
                    // rc += lt_emit_event(L, "textinput", "s", codepoint_to_utf8_(e.codepoint));
                    break;
                case INPUT_WRAP_BUTTON_PRESSED: {
                    // rc += lt_emit_event(L, "mousepressed", "sddd", lt_button_name(e.mouse.button), lt_mx, lt_my, printi(1 + clicks));

                    i32 code = 1 << e.mouse.button;
                    ui_input_mousedown(ctx, (i32)mouse_pos.x, (i32)(-mouse_pos.y), code);

                } break;
                case INPUT_WRAP_BUTTON_RELEASED: {
                    i32 code = 1 << e.mouse.button;
                    ui_input_mouseup(ctx, (i32)mouse_pos.x, (i32)(-mouse_pos.y), code);

                } break;
                case INPUT_WRAP_SCROLLED:
                    ui_input_scroll(ctx, (i32)(-e.scroll.x * 30.f), (i32)(-e.scroll.y * 30.f));
            }

    ctx->framebuffer_size = hint.framebuffer_size;
    ctx->viewport = ui_rect(hint.viewport.x, hint.viewport.y, hint.viewport.w, hint.viewport.h);
    ctx->mouse_pos = {mouse_pos.x, mouse_pos.y};
    ctx->command_list.idx = 0;
    ctx->root_list.idx = 0;
    ctx->scroll_target = NULL;
    ctx->hover_root = ctx->next_hover_root;
    ctx->next_hover_root = NULL;
    ctx->focus_root = ctx->next_focus_root;
    ctx->next_focus_root = NULL;
    ctx->prev_dockable_root = ctx->dockable_root;
    ctx->dockable_root = NULL;
    ctx->hover_split = ctx->next_hover_split;
    ctx->next_hover_split = NULL;
    ctx->lock_hover_id = ctx->next_lock_hover_id;
    ctx->next_lock_hover_id = 0x00;
    ctx->mouse_delta.x = ctx->mouse_pos.x - ctx->last_mouse_pos.x;
    ctx->mouse_delta.y = ctx->mouse_pos.y - ctx->last_mouse_pos.y;
    ctx->frame++;

    // Set up overlay draw list
    neko_vec2 fbs = ctx->framebuffer_size;
    neko_idraw_defaults(&ctx->overlay_draw_list);
    neko_idraw_camera2d(&ctx->overlay_draw_list, (u32)ctx->viewport.w, (u32)ctx->viewport.h);  // Need to pass in a viewport for this instead

    for (neko_slot_array_iter it = neko_slot_array_iter_new(ctx->splits); neko_slot_array_iter_valid(ctx->splits, it); neko_slot_array_iter_advance(ctx->splits, it)) {
        if (!it) continue;

        ui_split_t* split = neko_slot_array_iter_getp(ctx->splits, it);

        // Root split
        if (!split->parent) {
            ui_container_t* root_cnt = ui_get_root_container_from_split(ctx, split);
            ui_rect_t r = split->rect;
            r.x -= 10.f;
            r.w += 20.f;
            r.y -= 10.f;
            r.h += 20.f;
            neko_snprintfc(TMP, 256, "!dockspace%zu", (size_t)split);
            u64 opt = NEKO_UI_OPT_NOFRAME | NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOMOVE | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NOSCROLL | NEKO_UI_OPT_NOCLIP | NEKO_UI_OPT_NODOCK |
                      NEKO_UI_OPT_DOCKSPACE | NEKO_UI_OPT_NOBORDER;
            ui_window_begin_ex(ctx, TMP, r, NULL, NULL, opt);
            {
                // Set zindex for sorting (always below the bottom most window in this split tree)
                ui_container_t* ds = ui_get_current_container(ctx);
                i32 zindex = INT32_MAX - 1;
                ui_get_split_lowest_zindex(ctx, split, &zindex);
                if (root_cnt->opt & NEKO_UI_OPT_DOCKSPACE)
                    ds->zindex = 0;
                else
                    ds->zindex = NEKO_CLAMP((i32)zindex - 1, 0, INT32_MAX);

                ui_rect_t fr = split->rect;
                fr.x += NEKO_UI_SPLIT_SIZE;
                fr.y += NEKO_UI_SPLIT_SIZE;
                fr.w -= 2.f * NEKO_UI_SPLIT_SIZE;
                fr.h -= 2.f * NEKO_UI_SPLIT_SIZE;
                // ui_draw_frame(ctx, fr, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][0x00]);

                // Draw splits
                ui_draw_splits(ctx, split);

                // Do resize controls for editor_dockspace
                ui_container_t* top = ui_get_top_most_container(ctx, split);
                const ui_rect_t* sr = &split->rect;
                ui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root : ctx->next_hover_root ? ctx->next_hover_root : NULL;
                bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;

                // W
                {
                    // Cache rect
                    ui_rect_t lr = ui_rect(fr.x - 2.f * NEKO_UI_SPLIT_SIZE, fr.y, NEKO_UI_SPLIT_SIZE, fr.h);
                    ui_rect_t ex = lr;
                    ex.x -= 10.f;
                    ex.w += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_l", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, lr, NEKO_UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = NEKO_UI_SPLIT_RESIZE_W;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // E
                {
                    // Cache rect
                    ui_rect_t rr = ui_rect(fr.x + fr.w + NEKO_UI_SPLIT_SIZE, fr.y, NEKO_UI_SPLIT_SIZE, fr.h);
                    ui_rect_t ex = rr;
                    ex.w += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_r", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, rr, NEKO_UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = NEKO_UI_SPLIT_RESIZE_E;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // N
                {
                    // Cache rect
                    ui_rect_t tr = ui_rect(fr.x, fr.y - 2.f * NEKO_UI_SPLIT_SIZE, fr.w, NEKO_UI_SPLIT_SIZE);
                    ui_rect_t ex = tr;
                    ex.y -= 10.f;
                    ex.h += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_t", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, tr, NEKO_UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = NEKO_UI_SPLIT_RESIZE_N;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // S
                {
                    // Cache rect
                    ui_rect_t br = ui_rect(fr.x, fr.y + fr.h + NEKO_UI_SPLIT_SIZE, fr.w, NEKO_UI_SPLIT_SIZE);
                    ui_rect_t ex = br;
                    ex.h += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_b", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, br, NEKO_UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = NEKO_UI_SPLIT_RESIZE_S;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }
            }
            ui_window_end(ctx);
        }
    }

    if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
        ctx->lock_focus = 0x00;
    }
}

static void ui_docking(ui_context_t* ctx) {
    if (ctx->undock_root) {
        ui_undock_ex_cnt(ctx, ctx->undock_root);
    }

    if (!ctx->focus_root || ctx->focus_root->opt & NEKO_UI_OPT_NODOCK) return;

    if (ctx->dockable_root || ctx->prev_dockable_root) {
        ui_container_t* cnt = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;

        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
            i32 b = 0;
        }

        // Cache hoverable tile information
        neko_vec2 c = neko_v2(cnt->rect.x + cnt->rect.w / 2.f, cnt->rect.y + cnt->rect.h / 2.f);

        const float sz = NEKO_CLAMP(NEKO_MIN(cnt->rect.w * 0.1f, cnt->rect.h * 0.1f), 15.f, 25.f);
        const float off = sz + sz * 0.2f;
        Color256 def_col = neko_color_alpha(ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_FOCUS].colors[NEKO_UI_COLOR_BACKGROUND], 100);
        Color256 hov_col = neko_color_alpha(ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_FOCUS].colors[NEKO_UI_COLOR_BACKGROUND], 150);

        ui_rect_t center = ui_rect(c.x, c.y, sz, sz);
        ui_rect_t left = ui_rect(c.x - off, c.y, sz, sz);
        ui_rect_t right = ui_rect(c.x + off, c.y, sz, sz);
        ui_rect_t top = ui_rect(c.x, c.y - off, sz, sz);
        ui_rect_t bottom = ui_rect(c.x, c.y + off, sz, sz);

        i32 hov_c = (ui_rect_overlaps_vec2(center, ctx->mouse_pos));
        i32 hov_l = ui_rect_overlaps_vec2(left, ctx->mouse_pos);
        i32 hov_r = ui_rect_overlaps_vec2(right, ctx->mouse_pos);
        i32 hov_t = ui_rect_overlaps_vec2(top, ctx->mouse_pos);
        i32 hov_b = ui_rect_overlaps_vec2(bottom, ctx->mouse_pos);
        i32 hov_w = ui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos);

        bool can_dock = true;

        // Can't dock one editor_dockspace into another
        if (ctx->focus_root->opt & NEKO_UI_OPT_DOCKSPACE) {
            can_dock = false;
        }

        if (ctx->focus_root->tab_bar) {
            ui_container_t* tcmp = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, ctx->focus_root->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                ui_container_t* tcnt = (ui_container_t*)tab_bar->items[i].data;
                if (tcnt == tcmp) {
                    can_dock = false;
                }
            }
        }

        // Need to make sure you CAN dock here first
        if (ctx->dockable_root && can_dock) {
            // Need to now grab overlay draw list, then draw rects into it
            neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

            bool is_dockspace = ctx->dockable_root->opt & NEKO_UI_OPT_DOCKSPACE;

            // Draw center rect
            neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_c ? hov_col : def_col, R_PRIMITIVE_TRIANGLES);
            // neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w + 1, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);

            if (!is_dockspace) {
                neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_l ? hov_col : def_col, R_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_r ? hov_col : def_col, R_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_t ? hov_col : def_col, R_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_b ? hov_col : def_col, R_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
            }

            const float d = 0.5f;
            const float hs = sz * 0.5f;

            if (is_dockspace) {
                if (hov_c) {
                    center = ui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                }
            } else {
                if (hov_c && !ctx->focus_root->split) {
                    center = ui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                } else if (hov_l) {
                    left = ui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w * d + hs, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                } else if (hov_r) {
                    right = ui_rect(cnt->rect.x + cnt->rect.w * d + hs, cnt->rect.y, cnt->rect.w * (1.f - d) - hs, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                } else if (hov_b) {
                    bottom = ui_rect(cnt->rect.x, cnt->rect.y + cnt->rect.h * d + hs, cnt->rect.w, cnt->rect.h * (1.f - d) - hs);
                    neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                } else if (hov_t) {
                    top = ui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h * d + hs);
                    neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, R_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), def_col, R_PRIMITIVE_TRIANGLES);
                }
            }
        }

        // Handle docking
        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
            ui_container_t* parent = ctx->prev_dockable_root;
            ui_container_t* child = ctx->focus_root;

            bool is_dockspace = ctx->prev_dockable_root->opt & NEKO_UI_OPT_DOCKSPACE;

            if (is_dockspace) {
                if (hov_c) ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_TOP, 1.0f);
            } else {
                if (hov_c && !ctx->focus_root->split)
                    ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_TAB, 0.5f);
                else if (hov_l)
                    ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_LEFT, 0.5f);
                else if (hov_r)
                    ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_RIGHT, 0.5f);
                else if (hov_t)
                    ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_TOP, 0.5f);
                else if (hov_b)
                    ui_dock_ex_cnt(ctx, child, parent, NEKO_UI_SPLIT_BOTTOM, 0.5f);
            }
        }
    }
}

void ui_end(ui_context_t* ctx, bool update) {
    i32 i, n;

    // Check for docking, draw overlays
    ui_docking(ctx);

    for (u32 r = 0; r < neko_dyn_array_size(ctx->requests); ++r) {
        const ui_request_t* req = &ctx->requests[r];

        // If split moved, update position for next frame
        switch (req->type) {
            case NEKO_UI_CNT_MOVE: {
                if (!update) break;
                if (req->cnt) {
                    req->cnt->rect.x += ctx->mouse_delta.x;
                    req->cnt->rect.y += ctx->mouse_delta.y;

                    if (req->cnt->tab_bar) {
                        ui_tab_bar_t* tb = neko_slot_array_getp(ctx->tab_bars, req->cnt->tab_bar);
                        neko_assert(tb);
                        tb->rect.x += ctx->mouse_delta.x;
                        tb->rect.y += ctx->mouse_delta.y;
                    }
                }
            } break;

            case NEKO_UI_CNT_FOCUS: {
                if (!update) break;
                if (!req->cnt) break;

                ui_container_t* cnt = (ui_container_t*)req->cnt;
                neko_assert(cnt);

                ui_split_t* rs = ui_get_root_split(ctx, cnt);

                if (cnt->tab_bar) {
                    if (rs) {
                        ui_bring_split_to_front(ctx, rs);
                    }

                    ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
                    ui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];
                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->opt |= NEKO_UI_OPT_NOHOVER;
                    fcnt->opt |= NEKO_UI_OPT_NOINTERACT;
                    fcnt->flags &= ~NEKO_UI_WINDOW_FLAGS_VISIBLE;
                    tab_bar->focus = tab_item->idx;
                    cnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE;

                    // Bring all tab items to front
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        ui_bring_to_front(ctx, (ui_container_t*)tab_bar->items[i].data);
                    }
                    ui_bring_to_front(ctx, cnt);
                }

            } break;

            case NEKO_UI_SPLIT_MOVE: {
                if (req->split) {
                    req->split->rect.x += ctx->mouse_delta.x;
                    req->split->rect.y += ctx->mouse_delta.y;
                    ui_update_split(ctx, req->split);
                }

            } break;

            case NEKO_UI_SPLIT_RESIZE_SE: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_W: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                    }
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_E: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_N: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                    }
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_NE: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                    }
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_NW: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                    }

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                    }
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_S: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RESIZE_SW: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                    }
                    ui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_UI_SPLIT_RATIO: {
                const float smin = 0.05f;
                const float smax = 1.f - smin;
                ui_split_t* split = req->split;

                switch (split->type) {
                    case NEKO_UI_SPLIT_LEFT: {
                        split->ratio = NEKO_CLAMP(split->ratio + ctx->mouse_delta.x / split->rect.w, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case NEKO_UI_SPLIT_RIGHT: {
                        split->ratio = NEKO_CLAMP(split->ratio - ctx->mouse_delta.x / split->rect.w, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case NEKO_UI_SPLIT_TOP: {
                        split->ratio = NEKO_CLAMP(split->ratio + ctx->mouse_delta.y / split->rect.h, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case NEKO_UI_SPLIT_BOTTOM: {
                        split->ratio = NEKO_CLAMP(split->ratio - ctx->mouse_delta.y / split->rect.h, smin, smax);
                        ui_update_split(ctx, split);
                    } break;
                }

                // Bring to font
                ui_bring_split_to_front(ctx, ui_get_root_split_from_split(ctx, split));

            } break;

            case NEKO_UI_TAB_SWAP_LEFT: {
                ui_tab_item_swap(ctx, req->cnt, -1);
            } break;

            case NEKO_UI_TAB_SWAP_RIGHT: {
                ui_tab_item_swap(ctx, req->cnt, +1);
            } break;
        }
    }

    // Clear reqests
    neko_dyn_array_clear(ctx->requests);

    // Check stacks
    NEKO_EXPECT(ctx->container_stack.idx == 0);
    NEKO_EXPECT(ctx->clip_stack.idx == 0);
    NEKO_EXPECT(ctx->id_stack.idx == 0);
    NEKO_EXPECT(ctx->layout_stack.idx == 0);

    // Have to clear style stacks

    // Set previous frame ids
    // ctx->prev_hover = 0;
    // ctx->prev_focus = ctx->focus;

    // Handle scroll input
    if (ctx->scroll_target) {
        ctx->scroll_target->scroll.x += ctx->scroll_delta.x;
        ctx->scroll_target->scroll.y += ctx->scroll_delta.y;
    }

    // Unset focus if focus id was not touched this frame
    if (!ctx->updated_focus) {
        ctx->focus = 0;
    }
    ctx->updated_focus = 0;

    // Bring hover root to front if mouse was pressed
    if (update && ctx->mouse_pressed && ctx->next_hover_root && ctx->next_hover_root->zindex < ctx->last_zindex && ctx->next_hover_root->zindex >= 0) {
        // Root split
        ui_split_t* split = ui_get_root_split(ctx, ctx->next_hover_root);

        // Need to bring entire editor_dockspace to front
        if (split) {
            ui_bring_split_to_front(ctx, split);
        } else if (~ctx->next_hover_root->opt & NEKO_UI_OPT_NOFOCUS) {
            ui_bring_to_front(ctx, ctx->next_hover_root);
        }
    }

    if (ctx->mouse_pressed && (!ctx->next_hover_root || ctx->next_hover_root->opt & NEKO_UI_OPT_NOFOCUS)) {
        ctx->active_root = NULL;
    }

    // Reset state
    ctx->key_pressed = 0;
    ctx->input_text[0] = '\0';
    ctx->mouse_pressed = 0;
    ctx->scroll_delta = neko_v2(0, 0);
    ctx->last_mouse_pos = ctx->mouse_pos;
    ctx->undock_root = NULL;

    // TODO neko_os_set_cursor
    // if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
    //     neko_os_set_cursor(ctx->window_hndl, NEKO_PF_CURSOR_ARROW);
    // }

    // Sort root containers by zindex
    n = ctx->root_list.idx;
    qsort(ctx->root_list.items, n, sizeof(ui_container_t*), ui_compare_zindex);

    // Set root container jump commands
    for (i = 0; i < n; i++) {
        ui_container_t* cnt = ctx->root_list.items[i];

        // If this is the first container then make the first command jump to it.
        // otherwise set the previous container's tail to jump to this one
        if (i == 0) {
            ui_command_t* cmd = (ui_command_t*)ctx->command_list.items;
            cmd->jump.dst = (char*)cnt->head + sizeof(ui_jumpcommand_t);
        } else {
            ui_container_t* prev = ctx->root_list.items[i - 1];
            prev->tail->jump.dst = (char*)cnt->head + sizeof(ui_jumpcommand_t);
        }

        // Make the last container's tail jump to the end of command list
        if (i == n - 1) {
            cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
        }
    }
}

void ui_render(ui_context_t* ctx, neko_command_buffer_t* cb) {
    const neko_vec2 fb = ctx->framebuffer_size;
    const ui_rect_t* viewport = &ctx->viewport;
    neko_immediate_draw_t* gui_idraw = &ctx->gui_idraw;

    neko_idraw_defaults(&ctx->gui_idraw);
    // neko_idraw_camera2D(&ctx->gsi, (u32)fb.x, (u32)fb.y);
    neko_idraw_camera2d(&ctx->gui_idraw, (u32)viewport->w, (u32)viewport->h);
    neko_idraw_blend_enabled(&ctx->gui_idraw, true);

    ui_rect_t clip = ui_unclipped_rect;

    ui_command_t* cmd = NULL;
    while (ui_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case NEKO_UI_COMMAND_CUSTOM: {
                neko_idraw_defaults(&ctx->gui_idraw);
                neko_idraw_set_view_scissor(&ctx->gui_idraw, (i32)(cmd->custom.clip.x), (i32)(fb.y - cmd->custom.clip.h - cmd->custom.clip.y), (i32)(cmd->custom.clip.w), (i32)(cmd->custom.clip.h));

                if (cmd->custom.cb) {
                    cmd->custom.cb(ctx, &cmd->custom);
                }

                neko_idraw_defaults(&ctx->gui_idraw);
                // neko_idraw_camera2D(&ctx->gsi, (u32)fb.x, (u32)fb.y);
                neko_idraw_camera2d(&ctx->gui_idraw, (u32)viewport->w, (u32)viewport->h);
                neko_idraw_blend_enabled(&ctx->gui_idraw, true);
                // gfx_set_viewport(&ctx->gsi.commands, 0, 0, (u32)fb.x, (u32)fb.y);
                gfx_set_viewport(&ctx->gui_idraw.commands, (u32)viewport->x, (u32)viewport->y, (u32)viewport->w, (u32)viewport->h);

                neko_idraw_set_view_scissor(&ctx->gui_idraw, (i32)(clip.x), (i32)(fb.y - clip.h - clip.y), (i32)(clip.w), (i32)(clip.h));

            } break;

            case NEKO_UI_COMMAND_PIPELINE: {
                neko_idraw_pipeline_set(&ctx->gui_idraw, cmd->pipeline.pipeline);

                // Set layout if valid
                if (cmd->pipeline.layout_sz) {
                    switch (cmd->pipeline.layout_type) {
                        case NEKO_IDRAW_LAYOUT_VATTR: {
                            neko_idraw_vattr_list(&ctx->gui_idraw, (neko_idraw_vattr_type*)cmd->pipeline.layout, cmd->pipeline.layout_sz);
                        } break;

                        case NEKO_IDRAW_LAYOUT_MESH: {
                            neko_idraw_vattr_list_mesh(&ctx->gui_idraw, (neko_asset_mesh_layout_t*)cmd->pipeline.layout, cmd->pipeline.layout_sz);
                        } break;
                    }
                }

                // If not a valid pipeline, then set back to default gui pipeline
                if (!cmd->pipeline.pipeline.id) {
                    neko_idraw_blend_enabled(&ctx->gui_idraw, true);
                }

            } break;

            case NEKO_UI_COMMAND_UNIFORMS: {
                gfx_bind_desc_t bind = NEKO_DEFAULT_VAL();

                // Set uniform bind
                gfx_bind_uniform_desc_t uniforms[1] = NEKO_DEFAULT_VAL();
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms);

                // Treat as byte buffer, read data
                neko_byte_buffer_t buffer = NEKO_DEFAULT_VAL();
                buffer.capacity = NEKO_UI_COMMANDLIST_SIZE;
                buffer.data = (u8*)cmd->uniforms.data;

                // Write count
                neko_byte_buffer_readc(&buffer, u16, ct);

                // Iterate through all uniforms, memcpy data as needed for each uniform in list
                for (u32 i = 0; i < ct; ++i) {
                    neko_byte_buffer_readc(&buffer, neko_handle(gfx_uniform_t), hndl);
                    neko_byte_buffer_readc(&buffer, size_t, sz);
                    neko_byte_buffer_readc(&buffer, u16, binding);
                    void* udata = (buffer.data + buffer.position);
                    neko_byte_buffer_advance_position(&buffer, sz);

                    uniforms[0].uniform = hndl;
                    uniforms[0].binding = binding;
                    uniforms[0].data = udata;
                    gfx_apply_bindings(&ctx->gui_idraw.commands, &bind);
                }
            } break;

            case NEKO_UI_COMMAND_TEXT: {
                const neko_vec2* tp = &cmd->text.pos;
                const char* ts = cmd->text.str;
                const Color256* tc = &cmd->text.color;
                const neko_asset_font_t* tf = cmd->text.font;
                neko_idraw_text(&ctx->gui_idraw, tp->x, tp->y, ts, tf, false, *tc);
            } break;

            case NEKO_UI_COMMAND_SHAPE: {
                neko_idraw_texture(&ctx->gui_idraw, neko_handle_invalid(gfx_texture_t));
                Color256* c = &cmd->shape.color;

                switch (cmd->shape.type) {
                    case NEKO_UI_SHAPE_RECT: {
                        ui_rect_t* r = &cmd->shape.rect;
                        neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2s(0.f), neko_v2s(1.f), *c, R_PRIMITIVE_TRIANGLES);
                    } break;

                    case NEKO_UI_SHAPE_CIRCLE: {
                        neko_vec2* cp = &cmd->shape.circle.center;
                        float* r = &cmd->shape.circle.radius;
                        neko_idraw_circle(&ctx->gui_idraw, cp->x, cp->y, *r, 16, c->r, c->g, c->b, c->a, R_PRIMITIVE_TRIANGLES);
                    } break;

                    case NEKO_UI_SHAPE_TRIANGLE: {
                        neko_vec2* pa = &cmd->shape.triangle.points[0];
                        neko_vec2* pb = &cmd->shape.triangle.points[1];
                        neko_vec2* pc = &cmd->shape.triangle.points[2];
                        neko_idraw_trianglev(&ctx->gui_idraw, *pa, *pb, *pc, *c, R_PRIMITIVE_TRIANGLES);

                    } break;

                    case NEKO_UI_SHAPE_LINE: {
                        neko_vec2* s = &cmd->shape.line.start;
                        neko_vec2* e = &cmd->shape.line.end;
                        neko_idraw_linev(&ctx->gui_idraw, *s, *e, *c);
                    } break;
                }

            } break;

            case NEKO_UI_COMMAND_IMAGE: {
                neko_idraw_texture(&ctx->gui_idraw, cmd->image.hndl);
                Color256* c = &cmd->image.color;
                ui_rect_t* r = &cmd->image.rect;
                neko_vec4* uvs = &cmd->image.uvs;
                neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2(uvs->x, uvs->y), neko_v2(uvs->z, uvs->w), *c, R_PRIMITIVE_TRIANGLES);
            } break;

            case NEKO_UI_COMMAND_CLIP: {
                // Will project scissor/clipping rectangles into framebuffer space
                neko_vec2 clip_off = neko_v2s(0.f);    // (0,0) unless using multi-viewports
                neko_vec2 clip_scale = neko_v2s(1.f);  // (1,1) unless using retina display which are often (2,2)

                ui_rect_t clip_rect;
                clip_rect.x = (cmd->clip.rect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (cmd->clip.rect.y - clip_off.y) * clip_scale.y;
                clip_rect.w = (cmd->clip.rect.w - clip_off.x) * clip_scale.x;
                clip_rect.h = (cmd->clip.rect.h - clip_off.y) * clip_scale.y;

                clip_rect.x = NEKO_MAX(clip_rect.x, 0.f);
                clip_rect.y = NEKO_MAX(clip_rect.y, 0.f);
                clip_rect.w = NEKO_MAX(clip_rect.w, 0.f);
                clip_rect.h = NEKO_MAX(clip_rect.h, 0.f);

                clip = clip_rect;

                neko_idraw_set_view_scissor(&ctx->gui_idraw, (i32)(clip_rect.x), (i32)(fb.y - clip_rect.h - clip_rect.y), (i32)(clip_rect.w), (i32)(clip_rect.h));

            } break;
        }
    }

    // Draw main list
    neko_idraw_draw(&ctx->gui_idraw, cb);

    // Draw overlay list
    neko_idraw_draw(&ctx->overlay_draw_list, cb);
}

void ui_renderpass_submit(ui_context_t* ctx, neko_command_buffer_t* cb, Color256 c) {
    neko_vec2 fbs = ctx->framebuffer_size;
    ui_rect_t* vp = &ctx->viewport;
    gfx_clear_action_t action = NEKO_DEFAULT_VAL();
    action.color[0] = (float)c.r / 255.f;
    action.color[1] = (float)c.g / 255.f;
    action.color[2] = (float)c.b / 255.f;
    action.color[3] = (float)c.a / 255.f;
    gfx_renderpass_begin(cb, R_RENDER_PASS_DEFAULT);
    {
        gfx_clear(cb, action);
        gfx_set_viewport(cb, (u32)vp->x, (u32)vp->y, (u32)vp->w, (u32)vp->h);
        ui_render(ctx, cb);
    }
    gfx_renderpass_end(cb);
}

void ui_renderpass_submit_ex(ui_context_t* ctx, neko_command_buffer_t* cb, gfx_clear_action_t action) {
    neko_vec2 fbs = ctx->framebuffer_size;
    ui_rect_t* vp = &ctx->viewport;
    neko_renderpass_t pass = NEKO_DEFAULT_VAL();
    gfx_renderpass_begin(cb, pass);
    gfx_set_viewport(cb, (u32)vp->x, (u32)vp->y, (u32)vp->w, (u32)vp->h);
    gfx_clear(cb, action);
    ui_render(ctx, cb);
    gfx_renderpass_end(cb);
}

void ui_set_hover(ui_context_t* ctx, ui_id id) {
    ctx->prev_hover = ctx->hover;
    ctx->hover = id;
}

void ui_set_focus(ui_context_t* ctx, ui_id id) {
    ctx->prev_focus = ctx->focus;
    ctx->focus = id;
    ctx->updated_focus = 1;
}

ui_id ui_get_id(ui_context_t* ctx, const void* data, i32 size) {
    i32 idx = ctx->id_stack.idx;
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : NEKO_UI_HASH_INITIAL;
    ui_hash(&res, data, size);
    ctx->last_id = res;

    return res;
}

ui_id ui_get_id_hash(ui_context_t* ctx, const void* data, i32 size, ui_id hash) {
    ui_id res = hash;
    ui_hash(&res, data, size);
    ctx->last_id = res;
    return res;
}

void ui_push_id(ui_context_t* ctx, const void* data, i32 size) { ui_stack_push(ctx->id_stack, ui_get_id(ctx, data, size)); }

void ui_pop_id(ui_context_t* ctx) { ui_stack_pop(ctx->id_stack); }

void ui_push_clip_rect(ui_context_t* ctx, ui_rect_t rect) {
    ui_rect_t last = ui_get_clip_rect(ctx);
    ui_stack_push(ctx->clip_stack, ui_intersect_rects(rect, last));
}

void ui_pop_clip_rect(ui_context_t* ctx) { ui_stack_pop(ctx->clip_stack); }

ui_rect_t ui_get_clip_rect(ui_context_t* ctx) {
    NEKO_EXPECT(ctx->clip_stack.idx > 0);
    return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
}

i32 ui_check_clip(ui_context_t* ctx, ui_rect_t r) {
    ui_rect_t cr = ui_get_clip_rect(ctx);

    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h || r.y + r.h < cr.y) {
        return NEKO_UI_CLIP_ALL;
    }

    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y && r.y + r.h <= cr.y + cr.h) {
        return 0;
    }

    return NEKO_UI_CLIP_PART;
}

ui_container_t* ui_get_current_container(ui_context_t* ctx) {
    NEKO_EXPECT(ctx->container_stack.idx > 0);
    return ctx->container_stack.items[ctx->container_stack.idx - 1];
}

void ui_current_container_close(ui_context_t* ctx) {
    ui_container_t* cnt = ui_get_current_container(ctx);
    cnt->open = false;
}

ui_container_t* ui_get_container(ui_context_t* ctx, const char* name) {
    ui_id id = ui_get_id(ctx, name, strlen(name));
    return ui_get_container_ex(ctx, id, 0);
}

void ui_bring_to_front(ui_context_t* ctx, ui_container_t* cnt) {
    ui_container_t* root = ui_get_root_container(ctx, cnt);
    if (root->opt & NEKO_UI_OPT_NOBRINGTOFRONT) {
        if (cnt->opt & NEKO_UI_OPT_DOCKSPACE)
            cnt->zindex = 0;
        else
            cnt->zindex = 2;
        if (cnt->tab_bar) {
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                ((ui_container_t*)tab_bar->items[i].data)->zindex = cnt->zindex + i;
            }
        }
    } else {
        cnt->zindex = ++ctx->last_zindex;

        // If container is part of a tab item, then iterate and bring to front as well
        if (cnt->tab_bar) {
            ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                ((ui_container_t*)tab_bar->items[i].data)->zindex = ++ctx->last_zindex;
            }
        }
    }
}

/*============================================================================
** Pool
**============================================================================*/

i32 ui_pool_init(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id) {
    i32 i, n = -1, f = ctx->frame;
    for (i = 0; i < len; i++) {
        if (items[i].last_update < f) {
            f = items[i].last_update;
            n = i;
        }
    }

    NEKO_EXPECT(n > -1);
    items[n].id = id;
    ui_pool_update(ctx, items, n);

    return n;
}

i32 ui_pool_get(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id) {
    // Note: This is a linear hash lookup. Could speed this up with a quadratic lookup.
    i32 i;
    ui_unused(ctx);
    for (i = 0; i < len; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

void ui_pool_update(ui_context_t* ctx, ui_pool_item_t* items, i32 idx) { items[idx].last_update = ctx->frame; }

/*============================================================================
** input handlers
**============================================================================*/

void ui_input_mousemove(ui_context_t* ctx, i32 x, i32 y) { ctx->mouse_pos = neko_v2((f32)x, (f32)y); }

void ui_input_mousedown(ui_context_t* ctx, i32 x, i32 y, i32 btn) {
    ui_input_mousemove(ctx, x, y);
    ctx->mouse_down |= btn;
    ctx->mouse_pressed |= btn;
}

void ui_input_mouseup(ui_context_t* ctx, i32 x, i32 y, i32 btn) {
    ui_input_mousemove(ctx, x, y);
    ctx->mouse_down &= ~btn;
}

void ui_input_scroll(ui_context_t* ctx, i32 x, i32 y) {
    ctx->scroll_delta.x += x;
    ctx->scroll_delta.y += y;
}

void ui_input_keydown(ui_context_t* ctx, i32 key) {
    ctx->key_pressed |= key;
    ctx->key_down |= key;
}

void ui_input_keyup(ui_context_t* ctx, i32 key) { ctx->key_down &= ~key; }

void ui_input_text(ui_context_t* ctx, const char* text) {
    i32 len = strlen(ctx->input_text);
    i32 size = strlen(text) + 1;
    if (len + size > (i32)sizeof(ctx->input_text)) return;
    memcpy(ctx->input_text + len, text, size);
}

/*============================================================================
** commandlist
**============================================================================*/

ui_command_t* ui_push_command(ui_context_t* ctx, i32 type, i32 size) {
    ui_command_t* cmd = (ui_command_t*)(ctx->command_list.items + ctx->command_list.idx);
    NEKO_EXPECT(ctx->command_list.idx + size < NEKO_UI_COMMANDLIST_SIZE);
    cmd->base.type = type;
    cmd->base.size = size;
    ctx->command_list.idx += size;
    return cmd;
}

i32 ui_next_command(ui_context_t* ctx, ui_command_t** cmd) {
    if (*cmd) {
        *cmd = (ui_command_t*)(((char*)*cmd) + (*cmd)->base.size);
    } else {
        *cmd = (ui_command_t*)ctx->command_list.items;
    }

    while ((u8*)*cmd != (u8*)(ctx->command_list.items + ctx->command_list.idx)) {
        if ((*cmd)->type != NEKO_UI_COMMAND_JUMP) {
            return 1;
        }
        *cmd = (ui_command_t*)((*cmd)->jump.dst);
    }
    return 0;
}

void ui_set_clip(ui_context_t* ctx, ui_rect_t rect) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_CLIP, sizeof(ui_clipcommand_t));
    cmd->clip.rect = rect;
}

void ui_set_pipeline(ui_context_t* ctx, neko_handle(gfx_pipeline_t) pip, void* layout, size_t sz, neko_idraw_layout_type type) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_PIPELINE, sizeof(ui_pipelinecommand_t));
    cmd->pipeline.pipeline = pip;
    cmd->pipeline.layout_type = type;
    cmd->pipeline.layout = ctx->command_list.items + ctx->command_list.idx;
    cmd->pipeline.layout_sz = sz;
    cmd->base.size += sz;

    // Copy data and move list forward
    memcpy(ctx->command_list.items + ctx->command_list.idx, layout, sz);
    ctx->command_list.idx += sz;
}

void ui_bind_uniforms(ui_context_t* ctx, gfx_bind_uniform_desc_t* uniforms, size_t uniforms_sz) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_UNIFORMS, sizeof(ui_binduniformscommand_t));
    cmd->uniforms.data = ctx->command_list.items + ctx->command_list.idx;

    // Treat as byte buffer, write into data then set size
    neko_byte_buffer_t buffer = NEKO_DEFAULT_VAL();
    buffer.capacity = NEKO_UI_COMMANDLIST_SIZE;
    buffer.data = (u8*)cmd->uniforms.data;

    const u16 ct = uniforms_sz / sizeof(gfx_bind_uniform_desc_t);

    // Write count
    neko_byte_buffer_write(&buffer, u16, ct);

    // Iterate through all uniforms, memcpy data as needed for each uniform in list
    for (u32 i = 0; i < ct; ++i) {
        gfx_bind_uniform_desc_t* decl = &uniforms[i];
        neko_handle(gfx_uniform_t) hndl = decl->uniform;
        const size_t sz = gfx_uniform_size_query(hndl);
        neko_byte_buffer_write(&buffer, neko_handle(gfx_uniform_t), hndl);
        neko_byte_buffer_write(&buffer, size_t, sz);
        neko_byte_buffer_write(&buffer, u16, (u16)decl->binding);
        neko_byte_buffer_write_bulk(&buffer, decl->data, sz);
    }

    // Record final sizes
    const size_t sz = buffer.size;
    cmd->base.size += sz;
    ctx->command_list.idx += sz;
}

void ui_draw_line(ui_context_t* ctx, neko_vec2 start, neko_vec2 end, Color256 color) {
    ui_command_t* cmd;
    ui_rect_t rect = NEKO_DEFAULT_VAL();
    neko_vec2 s = start.x < end.x ? start : end;
    neko_vec2 e = start.x < end.x ? end : start;
    ui_rect(s.x, s.y, e.x - s.x, e.y - s.y);
    rect = ui_intersect_rects(rect, ui_get_clip_rect(ctx));

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == NEKO_UI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = NEKO_UI_SHAPE_LINE;
    cmd->shape.line.start = s;
    cmd->shape.line.end = e;
    cmd->shape.color = color;

    // reset clipping if it was set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_rect(ui_context_t* ctx, ui_rect_t rect, Color256 color) {
    ui_command_t* cmd;
    rect = ui_intersect_rects(rect, ui_get_clip_rect(ctx));
    if (rect.w > 0 && rect.h > 0) {
        cmd = ui_push_command(ctx, NEKO_UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
        cmd->shape.type = NEKO_UI_SHAPE_RECT;
        cmd->shape.rect = rect;
        cmd->shape.color = color;
    }
}

void ui_draw_circle(ui_context_t* ctx, neko_vec2 position, float radius, Color256 color) {
    ui_command_t* cmd;
    ui_rect_t rect = ui_rect(position.x - radius, position.y - radius, 2.f * radius, 2.f * radius);

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == NEKO_UI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    // do shape command
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = NEKO_UI_SHAPE_CIRCLE;
    cmd->shape.circle.center = position;
    cmd->shape.circle.radius = radius;
    cmd->shape.color = color;

    // reset clipping if it was set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_triangle(ui_context_t* ctx, neko_vec2 a, neko_vec2 b, neko_vec2 c, Color256 color) {
    ui_command_t* cmd;

    // Check each point against rect (if partially clipped, then good
    i32 clipped = 0x00;
    ui_rect_t clip = ui_get_clip_rect(ctx);
    i32 ca = ui_rect_overlaps_vec2(clip, a);
    i32 cb = ui_rect_overlaps_vec2(clip, b);
    i32 cc = ui_rect_overlaps_vec2(clip, c);

    if (ca && cb && cc)
        clipped = 0x00;  // No clip
    else if (!ca && !cb && !cc)
        clipped = NEKO_UI_CLIP_ALL;
    else if (ca || cb || cc)
        clipped = NEKO_UI_CLIP_PART;

    if (clipped == NEKO_UI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_UI_CLIP_PART) {
        ui_set_clip(ctx, clip);
    }

    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = NEKO_UI_SHAPE_TRIANGLE;
    cmd->shape.triangle.points[0] = a;
    cmd->shape.triangle.points[1] = b;
    cmd->shape.triangle.points[2] = c;
    cmd->shape.color = color;

    // Reset clipping if set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_box(ui_context_t* ctx, ui_rect_t rect, i16* w, Color256 color) {
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
    // neko_idraw_rectvd(dl, neko_v2(rect.x, rect.y), neko_v2(rect.w, rect.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_RED, R_PRIMITIVE_LINES);

    const float l = (float)w[0], r = (float)w[1], t = (float)w[2], b = (float)w[3];
    ui_draw_rect(ctx, ui_rect(rect.x + l, rect.y, rect.w - r - l, t), color);               // top
    ui_draw_rect(ctx, ui_rect(rect.x + l, rect.y + rect.h - b, rect.w - r - l, b), color);  // bottom
    ui_draw_rect(ctx, ui_rect(rect.x, rect.y, l, rect.h), color);                           // left
    ui_draw_rect(ctx, ui_rect(rect.x + rect.w - r, rect.y, r, rect.h), color);              // right
}

void ui_draw_text(ui_context_t* ctx, neko_asset_font_t* font, const char* str, i32 len, neko_vec2 pos, Color256 color, i32 shadow_x, i32 shadow_y, Color256 shadow_color) {
    // Set to default font
    if (!font) {
        font = neko_idraw_default_font();
    }

#define DRAW_TEXT(TEXT, RECT, COLOR)                                                      \
    do {                                                                                  \
        ui_command_t* cmd;                                                                \
        neko_vec2 td = ui_text_dimensions(font, TEXT, -1);                                \
        ui_rect_t rect = (RECT);                                                          \
        i32 clipped = ui_check_clip(ctx, rect);                                           \
                                                                                          \
        if (clipped == NEKO_UI_CLIP_ALL) {                                                \
            return;                                                                       \
        }                                                                                 \
                                                                                          \
        if (clipped == NEKO_UI_CLIP_PART) {                                               \
            ui_rect_t crect = ui_get_clip_rect(ctx);                                      \
            ui_set_clip(ctx, crect);                                                      \
        }                                                                                 \
                                                                                          \
        /* add command */                                                                 \
        if (len < 0) {                                                                    \
            len = strlen(TEXT);                                                           \
        }                                                                                 \
                                                                                          \
        cmd = ui_push_command(ctx, NEKO_UI_COMMAND_TEXT, sizeof(ui_textcommand_t) + len); \
        memcpy(cmd->text.str, TEXT, len);                                                 \
        cmd->text.str[len] = '\0';                                                        \
        cmd->text.pos = neko_v2(rect.x, rect.y);                                          \
        cmd->text.color = COLOR;                                                          \
        cmd->text.font = font;                                                            \
                                                                                          \
        if (clipped) {                                                                    \
            ui_set_clip(ctx, ui_unclipped_rect);                                          \
        }                                                                                 \
    } while (0)

    // Draw shadow
    if (shadow_x || shadow_y && shadow_color.a) {
        DRAW_TEXT(str, ui_rect(pos.x + (float)shadow_x, pos.y + (float)shadow_y, td.x, td.y), shadow_color);
    }

    // Draw text
    { DRAW_TEXT(str, ui_rect(pos.x, pos.y, td.x, td.y), color); }
}

void ui_draw_image(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, Color256 color) {
    ui_command_t* cmd;

    /* do clip command if the rect isn't fully contained within the cliprect */
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == NEKO_UI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    /* do image command */
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_IMAGE, sizeof(ui_imagecommand_t));
    cmd->image.hndl = hndl;
    cmd->image.rect = rect;
    cmd->image.uvs = neko_v4(uv0.x, uv0.y, uv1.x, uv1.y);
    cmd->image.color = color;

    /* reset clipping if it was set */
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_custom(ui_context_t* ctx, ui_rect_t rect, ui_draw_callback_t cb, void* data, size_t sz) {
    ui_command_t* cmd;

    ui_rect_t viewport = rect;

    rect = ui_intersect_rects(rect, ui_get_clip_rect(ctx));

    /* do clip command if the rect isn't fully contained within the cliprect */
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == NEKO_UI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    i32 idx = ctx->id_stack.idx;
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : NEKO_UI_HASH_INITIAL;

    /* do custom command */
    cmd = ui_push_command(ctx, NEKO_UI_COMMAND_CUSTOM, sizeof(ui_customcommand_t));
    cmd->custom.clip = rect;
    cmd->custom.viewport = viewport;
    cmd->custom.cb = cb;
    cmd->custom.hover = ctx->hover;
    cmd->custom.focus = ctx->focus;
    cmd->custom.hash = res;
    cmd->custom.data = ctx->command_list.items + ctx->command_list.idx;
    cmd->custom.sz = sz;
    cmd->base.size += sz;

    // Copy data and move list forward
    memcpy(ctx->command_list.items + ctx->command_list.idx, data, sz);
    ctx->command_list.idx += sz;

    /* reset clipping if it was set */
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_nine_rect(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, u32 left, u32 right, u32 top, u32 bottom, Color256 color) {
    // Draw images based on rect, slice image based on uvs (uv0, uv1), original texture dimensions (width, height) and control margins (left, right, top, bottom)
    gfx_texture_desc_t desc = NEKO_DEFAULT_VAL();
    gfx_texture_desc_query(hndl, &desc);
    u32 width = desc.width;
    u32 height = desc.height;

    // tl
    {
        u32 w = left;
        u32 h = top;
        ui_rect_t r = ui_rect(rect.x, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv0.y);
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // tr
    {
        u32 w = right;
        u32 h = top;
        ui_rect_t r = ui_rect(rect.x + rect.w - w, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y);
        neko_vec2 st1 = neko_v2(uv1.x, uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // br
    {
        u32 w = right;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x + rect.w - (f32)w, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x, uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bl
    {
        u32 w = left;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // top
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = top;
        ui_rect_t r = ui_rect(rect.x + left, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y);
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bottom
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x + left, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // left
    {
        u32 w = left;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // right
    {
        u32 w = right;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x + rect.w - (f32)w, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x, uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // center
    {
        u32 w = (u32)rect.w - right - left;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x + left, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }
}

/*============================================================================
** layout
**============================================================================*/
enum { NEKO_UI_RELATIVE = 1, NEKO_UI_ABSOLUTE = 2 };

ui_rect_t ui_layout_anchor(const ui_rect_t* p, i32 width, i32 height, i32 xoff, i32 yoff, ui_layout_anchor_type type) {
    float w = (float)width;
    float h = (float)height;
    ui_rect_t r = ui_rect(p->x, p->y, w, h);

    switch (type) {
        default:
        case NEKO_UI_LAYOUT_ANCHOR_TOPLEFT: {
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_TOPCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_TOPRIGHT: {
            r.x = p->x + (p->w - w);
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_LEFT: {
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_CENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_RIGHT: {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_BOTTOMLEFT: {
            r.y = p->y + (p->h - h);
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_BOTTOMCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h);
        } break;

        case NEKO_UI_LAYOUT_ANCHOR_BOTTOMRIGHT: {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h);
        } break;
    }

    // Apply offset
    r.x += xoff;
    r.y += yoff;

    return r;
}

void ui_layout_column_begin(ui_context_t* ctx) { ui_push_layout(ctx, ui_layout_next(ctx), neko_v2(0, 0)); }

void ui_layout_column_end(ui_context_t* ctx) {
    ui_layout_t *a, *b;
    b = ui_get_layout(ctx);
    ui_stack_pop(ctx->layout_stack);

    /* inherit position/next_row/max from child layout if they are greater */
    a = ui_get_layout(ctx);
    a->position.x = NEKO_MAX(a->position.x, b->position.x + b->body.x - a->body.x);
    a->next_row = (i32)NEKO_MAX((f32)a->next_row, (f32)b->next_row + (f32)b->body.y - (f32)a->body.y);
    a->max.x = NEKO_MAX(a->max.x, b->max.x);
    a->max.y = NEKO_MAX(a->max.y, b->max.y);
}

void ui_layout_row(ui_context_t* ctx, i32 items, const i32* widths, i32 height) {
    ui_style_t* style = ctx->style;
    ui_layout_t* layout = ui_get_layout(ctx);

    if (widths) {
        NEKO_EXPECT(items <= NEKO_UI_MAX_WIDTHS);
        memcpy(layout->widths, widths, items * sizeof(widths[0]));
    }
    layout->items = items;
    layout->position = neko_v2((f32)layout->indent, (f32)layout->next_row);
    layout->size.y = (f32)height;
    layout->item_index = 0;
}

void ui_layout_row_ex(ui_context_t* ctx, i32 items, const i32* widths, i32 height, i32 justification) {
    ui_layout_row(ctx, items, widths, height);
    ui_layout_t* layout = ui_get_layout(ctx);

    switch (justification) {
        default:
            break;

        case NEKO_UI_JUSTIFY_CENTER: {
            // Iterate through all widths, calculate total
            // X is center - tw/2
            float w = 0.f;
            for (u32 i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f ? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w) * 0.5f + layout->indent;
        } break;

        case NEKO_UI_JUSTIFY_END: {
            float w = 0.f;
            for (u32 i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f ? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w);
        } break;
    }
}

void ui_layout_width(ui_context_t* ctx, i32 width) { ui_get_layout(ctx)->size.x = (f32)width; }

void ui_layout_height(ui_context_t* ctx, i32 height) { ui_get_layout(ctx)->size.y = (f32)height; }

void ui_layout_set_next(ui_context_t* ctx, ui_rect_t r, i32 relative) {
    ui_layout_t* layout = ui_get_layout(ctx);
    layout->next = r;
    layout->next_type = relative ? NEKO_UI_RELATIVE : NEKO_UI_ABSOLUTE;
}

ui_rect_t ui_layout_peek_next(ui_context_t* ctx) {
    ui_layout_t layout = *ui_get_layout(ctx);
    ui_style_t* style = ctx->style;
    ui_rect_t res;

    if (layout.next_type) {
        /* handle rect set by `ui_layout_set_next` */
        i32 type = layout.next_type;
        res = layout.next;
        if (type == NEKO_UI_ABSOLUTE) {
            return res;
        }

    } else {
        // handle next row
        if (layout.item_index == layout.items) {
            ui_layout_row(ctx, layout.items, NULL, (i32)layout.size.y);
        }

        const i32 items = layout.items;
        const i32 idx = layout.item_index;

        i32 ml = style->margin[NEKO_UI_MARGIN_LEFT];
        i32 mr = style->margin[NEKO_UI_MARGIN_RIGHT];
        i32 mt = style->margin[NEKO_UI_MARGIN_TOP];
        i32 mb = style->margin[NEKO_UI_MARGIN_BOTTOM];

        // position
        res.x = layout.position.x + ml;
        res.y = layout.position.y + mt;

        // size
        res.w = layout.items > 0 ? layout.widths[layout.item_index] : layout.size.x;
        res.h = layout.size.y;

        // default fallbacks
        if (res.w == 0) {
            res.w = style->size[0];
        }
        if (res.h == 0) {
            res.h = style->size[1];
        }

        if (res.w < 0) {
            res.w += layout.body.w - res.x + 1;
        }
        if (res.h < 0) {
            res.h += layout.body.h - res.y + 1;
        }

        layout.item_index++;
    }

    /* update position */
    layout.position.x += res.w + style->margin[NEKO_UI_MARGIN_RIGHT];
    layout.next_row = (i32)NEKO_MAX(layout.next_row, res.y + res.h + style->margin[NEKO_UI_MARGIN_BOTTOM]);

    /* apply body offset */
    res.x += layout.body.x;
    res.y += layout.body.y;

    /* update max position */
    layout.max.x = NEKO_MAX(layout.max.x, res.x + res.w);
    layout.max.y = NEKO_MAX(layout.max.y, res.y + res.h);

    return res;
}

ui_rect_t ui_layout_next(ui_context_t* ctx) {
    ui_layout_t* layout = ui_get_layout(ctx);
    ui_style_t* style = ctx->style;
    ui_rect_t res;

    if (layout->next_type) {
        /* handle rect set by `ui_layout_set_next` */
        i32 type = layout->next_type;
        layout->next_type = 0;
        res = layout->next;
        if (type == NEKO_UI_ABSOLUTE) {
            return (ctx->last_rect = res);
        }

    } else {
        // handle next row
        if (layout->item_index == layout->items) {
            ui_layout_row(ctx, layout->items, NULL, (i32)layout->size.y);
        }

        const i32 items = layout->items;
        const i32 idx = layout->item_index;

        i32 ml = style->margin[NEKO_UI_MARGIN_LEFT];
        i32 mr = style->margin[NEKO_UI_MARGIN_RIGHT];
        i32 mt = style->margin[NEKO_UI_MARGIN_TOP];
        i32 mb = style->margin[NEKO_UI_MARGIN_BOTTOM];

        // position
        res.x = layout->position.x + ml;
        res.y = layout->position.y + mt;

        // size
        res.w = layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
        res.h = layout->size.y;

        // default fallbacks
        if (res.w == 0) {
            res.w = style->size[0];
        }
        if (res.h == 0) {
            res.h = style->size[1];
        }

        // Not sure about this... should probably iterate through the rest, figure out what's left, then
        // determine how much to divide up
        if (res.w < 0) {
            res.w += layout->body.w - res.x + 1;
        }
        if (res.h < 0) {
            res.h += layout->body.h - res.y + 1;
        }

        layout->item_index++;
    }

    /* update position */
    layout->position.x += res.w + style->margin[NEKO_UI_MARGIN_RIGHT];
    layout->next_row = (i32)NEKO_MAX(layout->next_row, res.y + res.h + style->margin[NEKO_UI_MARGIN_BOTTOM]);  //  + style->margin[NEKO_UI_MARGIN_TOP] * 0.5f);

    /* apply body offset */
    res.x += layout->body.x;
    res.y += layout->body.y;

    /* update max position */
    layout->max.x = NEKO_MAX(layout->max.x, res.x + res.w);
    layout->max.y = NEKO_MAX(layout->max.y, res.y + res.h);

    return (ctx->last_rect = res);
}

/*============================================================================
** controls
**============================================================================*/

static i32 ui_in_hover_root(ui_context_t* ctx) {
    i32 i = ctx->container_stack.idx;
    while (i--) {
        if (ctx->container_stack.items[i] == ctx->hover_root) {
            return 1;
        }

        /* only root containers have their `head` field set; stop searching if we've
        ** reached the current root container */
        if (ctx->container_stack.items[i]->head) {
            break;
        }
    }
    return 0;
}

void ui_draw_control_frame(ui_context_t* ctx, ui_id id, ui_rect_t rect, i32 elementid, u64 opt) {
    if (opt & NEKO_UI_OPT_NOFRAME) {
        return;
    }
    i32 state = ctx->focus == id ? NEKO_UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_UI_ELEMENT_STATE_HOVER : 0x00;
    ui_draw_frame(ctx, rect, &ctx->style_sheet->styles[elementid][state]);
}

void ui_draw_control_text(ui_context_t* ctx, const char* str, ui_rect_t rect, const ui_style_t* style, u64 opt) {
    neko_vec2 pos = neko_v2(rect.x, rect.y);
    neko_asset_font_t* font = style->font;
    neko_vec2 td = ui_text_dimensions(font, str, -1);
    i32 tw = (i32)td.x;
    i32 th = (i32)td.y;

    ui_push_clip_rect(ctx, rect);

    // Grab stylings
    const i32 padding_left = style->padding[NEKO_UI_PADDING_LEFT];
    const i32 padding_top = style->padding[NEKO_UI_PADDING_TOP];
    const i32 padding_right = style->padding[NEKO_UI_PADDING_RIGHT];
    const i32 padding_bottom = style->padding[NEKO_UI_PADDING_BOTTOM];
    const i32 align = style->align_content;
    const i32 justify = style->justify_content;

    // Determine x placement based on justification
    switch (justify) {
        default:
        case NEKO_UI_JUSTIFY_START: {
            pos.x = rect.x + padding_left;
        } break;

        case NEKO_UI_JUSTIFY_CENTER: {
            pos.x = rect.x + (rect.w - tw) * 0.5f;
        } break;

        case NEKO_UI_JUSTIFY_END: {
            pos.x = rect.x + (rect.w - tw) - padding_right;
        } break;
    }

    // Determine y placement based on alignment within rect
    switch (align) {
        default:
        case NEKO_UI_ALIGN_START: {
            pos.y = rect.y + padding_top;
        } break;

        case NEKO_UI_ALIGN_CENTER: {
            pos.y = rect.y + (rect.h - th) * 0.5f;
        } break;

        case NEKO_UI_ALIGN_END: {
            pos.y = rect.y + (rect.h - th) - padding_bottom;
        } break;
    }

    bool is_content = (opt & NEKO_UI_OPT_ISCONTENT);
    i32 bg_color = is_content ? NEKO_UI_COLOR_CONTENT_BACKGROUND : NEKO_UI_COLOR_BACKGROUND;
    i32 sh_color = is_content ? NEKO_UI_COLOR_CONTENT_SHADOW : NEKO_UI_COLOR_SHADOW;
    i32 bd_color = is_content ? NEKO_UI_COLOR_CONTENT_BORDER : NEKO_UI_COLOR_BORDER;

    i32 sx = style->shadow_x;
    i32 sy = style->shadow_y;
    const Color256* sc = &style->colors[sh_color];

    // Border
    const Color256* bc = &style->colors[bd_color];
    if (bc->a && ~opt & NEKO_UI_OPT_NOSTYLEBORDER) {
        ui_pop_clip_rect(ctx);
        ui_rect_t border_rect = ui_expand_rect(rect, (i16*)style->border_width);
        ui_push_clip_rect(ctx, border_rect);
        ui_draw_box(ctx, border_rect, (i16*)style->border_width, *bc);
    }

    // Background
    if (~opt & NEKO_UI_OPT_NOSTYLEBACKGROUND) {
        ui_draw_rect(ctx, rect, style->colors[bg_color]);
    }

    // Text
    ui_draw_text(ctx, font, str, -1, pos, style->colors[NEKO_UI_COLOR_CONTENT], sx, sy, *sc);

    ui_pop_clip_rect(ctx);
}

i32 ui_mouse_over(ui_context_t* ctx, ui_rect_t rect) {
    return ui_rect_overlaps_vec2(rect, ctx->mouse_pos) && !ctx->hover_split && !ctx->lock_hover_id && ui_rect_overlaps_vec2(ui_get_clip_rect(ctx), ctx->mouse_pos) && ui_in_hover_root(ctx);
}

void ui_update_control(ui_context_t* ctx, ui_id id, ui_rect_t rect, u64 opt) {
    i32 mouseover = 0;
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

    ui_id prev_hov = ctx->prev_hover;
    ui_id prev_foc = ctx->prev_focus;

    // I should do updates in here

    if (opt & NEKO_UI_OPT_FORCEFOCUS) {
        mouseover = ui_rect_overlaps_vec2(ui_get_clip_rect(ctx), ctx->mouse_pos);
    } else {
        mouseover = ui_mouse_over(ctx, rect);
    }

    // Check for 'mouse-over' with id selection here

    if (ctx->focus == id) {
        ctx->updated_focus = 1;
    }
    if (opt & NEKO_UI_OPT_NOINTERACT) {
        return;
    }

    // Check for hold focus here
    if (mouseover && !ctx->mouse_down) {
        ui_set_hover(ctx, id);
    }

    if (ctx->focus == id) {
        ui_set_focus(ctx, id);
        if (ctx->mouse_pressed && !mouseover) {
            ui_set_focus(ctx, 0);
        }
        if (!ctx->mouse_down && ~opt & NEKO_UI_OPT_HOLDFOCUS) {
            ui_set_focus(ctx, 0);
        }
    }

    if (ctx->prev_hover == id && !mouseover) {
        ctx->prev_hover = ctx->hover;
    }

    if (ctx->hover == id) {
        if (ctx->mouse_pressed) {
            if ((opt & NEKO_UI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == NEKO_UI_MOUSE_LEFT) || (~opt & NEKO_UI_OPT_LEFTCLICKONLY)) {
                ui_set_focus(ctx, id);
            }
        } else if (!mouseover) {
            ui_set_hover(ctx, 0);
        }
    }

    // Do state check
    if (~opt & NEKO_UI_OPT_NOSWITCHSTATE) {
        if (ctx->focus == id) {
            if (ctx->prev_focus != id)
                ctx->last_focus_state = NEKO_UI_ELEMENT_STATE_ON_FOCUS;
            else
                ctx->last_focus_state = NEKO_UI_ELEMENT_STATE_FOCUS;
        } else {
            if (ctx->prev_focus == id)
                ctx->last_focus_state = NEKO_UI_ELEMENT_STATE_OFF_FOCUS;
            else
                ctx->last_focus_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->hover == id) {
            if (ctx->prev_hover != id)
                ctx->last_hover_state = NEKO_UI_ELEMENT_STATE_ON_HOVER;
            else
                ctx->last_hover_state = NEKO_UI_ELEMENT_STATE_HOVER;
        } else {
            if (ctx->prev_hover == id)
                ctx->last_hover_state = NEKO_UI_ELEMENT_STATE_OFF_HOVER;
            else
                ctx->last_hover_state = NEKO_UI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->prev_focus == id && !ctx->mouse_down && ~opt & NEKO_UI_OPT_HOLDFOCUS) {
            ctx->prev_focus = ctx->focus;
        }

        if (ctx->last_hover_state == NEKO_UI_ELEMENT_STATE_ON_HOVER || ctx->last_hover_state == NEKO_UI_ELEMENT_STATE_OFF_HOVER || ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_ON_FOCUS ||
            ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_OFF_FOCUS) {
            // Don't have a hover state switch if focused
            ctx->switch_state = ctx->last_focus_state ? ctx->last_focus_state : ctx->focus != id ? ctx->last_hover_state : NEKO_UI_ELEMENT_STATE_DEFAULT;
            switch (ctx->switch_state) {
                case NEKO_UI_ELEMENT_STATE_OFF_HOVER:
                case NEKO_UI_ELEMENT_STATE_ON_HOVER: {
                    if (ctx->focus == id || ctx->prev_focus == id) {
                        ctx->switch_state = 0x00;
                    }
                } break;
            }
            if (ctx->switch_state && ctx->prev_focus != id) ctx->state_switch_id = id;
        }
    } else {
        ctx->prev_focus = prev_foc;
        ctx->prev_hover = prev_hov;
    }
}

i32 ui_text_ex(ui_context_t* ctx, const char* text, i32 wrap, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0, elementid = NEKO_UI_ELEMENT_TEXT;
    ui_id id = ui_get_id(ctx, text, strlen(text));
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    const char *start, *end, *p = text;
    i32 width = -1;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    ui_style_t* save = ui_push_style(ctx, &style);
    neko_asset_font_t* font = ctx->style->font;
    Color256* color = &ctx->style->colors[NEKO_UI_COLOR_CONTENT];
    i32 sx = ctx->style->shadow_x;
    i32 sy = ctx->style->shadow_y;
    if (opt & NEKO_UI_OPT_NOSTYLESHADOW) {
        sx = 0;
        sy = 0;
    }
    Color256* sc = &ctx->style->colors[NEKO_UI_COLOR_SHADOW];
    i32 th = ui_font_height(font);
    ui_layout_column_begin(ctx);
    ui_layout_row(ctx, 1, &width, th);

    ui_rect_t tr = ui_layout_next(ctx);
    ui_layout_set_next(ctx, tr, 0);
    ui_rect_t r = ui_layout_next(ctx);
    ui_rect_t bg = r;
    do {
        i32 w = 0;
        start = end = p;
        do {
            const char* word = p;
            while (*p && *p != ' ' && *p != '\n') {
                p++;
            }

            if (wrap) w += ui_text_width(font, word, p - word);
            if (w > r.w && end != start) {
                break;
            }

            if (wrap) w += ui_text_width(font, p, 1);
            end = p++;

        } while (*end && *end != '\n');

        if (r.w > tr.w) tr.w = r.w;
        tr.h = (r.y + r.h) - tr.y;

        ui_rect_t txtrct = r;
        bg = r;
        if (*end) {
            r = ui_layout_next(ctx);
            bg.h = r.y - bg.y;
        } else {
            i32 th = ui_text_height(font, start, end - start);
            bg.h = r.h + (float)th / 2.f;
        }

        // Draw frame here for background if applicable (need to do this to account for space between wrap)
        if (ctx->style->colors[NEKO_UI_COLOR_BACKGROUND].a && ~opt & NEKO_UI_OPT_NOSTYLEBACKGROUND) {
            ui_draw_rect(ctx, bg, style.colors[NEKO_UI_COLOR_BACKGROUND]);
        }

        // Draw text
        ui_draw_text(ctx, font, start, end - start, neko_v2(txtrct.x, txtrct.y), *color, sx, sy, *sc);
        p = end + 1;

    } while (*end);

    // draw border
    if (style.colors[NEKO_UI_COLOR_BORDER].a && ~opt & NEKO_UI_OPT_NOSTYLEBORDER) {
        ui_draw_box(ctx, ui_expand_rect(tr, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]);
    }

    ui_update_control(ctx, id, tr, 0x00);

    // handle click
    if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_UI_RES_SUBMIT;
    }

    ui_layout_column_end(ctx);
    ui_pop_style(ctx, save);

    return res;
}

//  i32 ui_text_fc_ex(ui_context_t* ctx, const char* text, neko_font_index fontindex) {
//     i32 width = -1;
//     i32 th = 20;
//     ui_layout_column_begin(ctx);
//     ui_layout_row(ctx, 1, &width, th);
//     ui_layout_t* layout = ui_get_layout(ctx);
//     if (fontindex == -1) fontindex = ctx->gui_idraw.data->font_fc_default;
//     gfx_fc_text(text, fontindex, layout->body.x, layout->body.y + layout->body.h / 2);
//     ui_layout_column_end(ctx);
//     return 0;
// }

i32 ui_label_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    // Want to push animations here for styles
    i32 res = 0;
    i32 elementid = NEKO_UI_ELEMENT_LABEL;
    ui_id id = ui_get_id(ctx, label, neko_strlen(label));

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, 0x00);
    ui_draw_control_text(ctx, label_tag, r, &style, 0x00);
    ui_pop_style(ctx, save);
    if (id_tag) ui_pop_id(ctx);

    /* handle click */
    if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_UI_RES_SUBMIT;
    }

    return res;
}

i32 ui_image_ex(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, neko_vec2 uv0, neko_vec2 uv1, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &hndl, sizeof(hndl));
    const i32 elementid = NEKO_UI_ELEMENT_IMAGE;

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt);

    /* handle click */
    if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_UI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[NEKO_UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(r, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]);
    }

    ui_draw_image(ctx, hndl, r, uv0, uv1, style.colors[NEKO_UI_COLOR_CONTENT]);

    ui_pop_style(ctx, save);

    return res;
}

i32 ui_combo_begin_ex(ui_context_t* ctx, const char* id, const char* current_item, i32 max_items, ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    opt = NEKO_UI_OPT_NOMOVE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_FORCESETRECT;

    if (ui_button(ctx, current_item)) {
        ui_popup_open(ctx, id);
    }

    i32 ct = max_items > 0 ? max_items : 0;
    ui_rect_t rect = ctx->last_rect;
    rect.y += rect.h;
    rect.h = ct ? (ct + 1) * ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x00].size[1] : rect.h;
    return ui_popup_begin_ex(ctx, id, rect, NULL, opt);
}

i32 ui_combo_item_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = ui_button_ex(ctx, label, desc, opt);
    if (res) {
        ui_current_container_close(ctx);
    }
    return res;
}

void ui_combo_end(ui_context_t* ctx) { ui_popup_end(ctx); }

void ui_parse_label_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz) {
    // neko_lexer_t lex = neko_lexer_c_ctor(str);
    // while (neko_lexer_can_lex(&lex)) {
    //     neko_token_t token = neko_lexer_next_token(&lex);
    //     switch (token.type) {
    //         case NEKO_TOKEN_HASH: {
    //             if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
    //                 neko_token_t end = neko_lexer_current_token(&lex);

    //                 // Determine len
    //                 size_t len = NEKO_MIN(end.text - str, sz);

    //                 memcpy(buffer, str, len);
    //                 return;
    //             }
    //         } break;
    //     }
    // }

    // Reached end, so just memcpy
    memcpy(buffer, str, NEKO_MIN(sz, strlen(str) + 1));
}

void ui_parse_id_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz, u64 opt) {
    if (opt & NEKO_UI_OPT_PARSEIDTAGONLY) {
        // neko_lexer_t lex = neko_lexer_c_ctor(str);
        // while (neko_lexer_can_lex(&lex)) {
        //     neko_token_t token = neko_lexer_next_token(&lex);
        //     switch (token.type) {
        //         case NEKO_TOKEN_HASH: {
        //             if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
        //                 neko_token_t end = neko_lexer_next_token(&lex);
        //                 end = neko_lexer_next_token(&lex);
        //                 // 确定长度
        //                 size_t len = NEKO_MIN((str + strlen(str)) - end.text, sz);
        //                 memcpy(buffer, end.text, len);
        //                 return;
        //             }
        //         } break;
        //     }
        // }
    } else {
        size_t str_sz = strlen(str);
        size_t actual_sz = NEKO_MIN(str_sz, sz - 1);
        memcpy(buffer, str, actual_sz);
        buffer[actual_sz] = 0;
    }
}

i32 ui_button_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    // Note: clip out early here for performance

    i32 res = 0;
    ui_id id = ui_get_id(ctx, label, strlen(label));
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, NEKO_UI_ELEMENT_BUTTON);

    // Push id if tag available
    if (id_tag) {
        ui_push_id(ctx, id_tag, strlen(id_tag));
    }

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, NEKO_UI_ELEMENT_BUTTON);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, NEKO_UI_ELEMENT_BUTTON, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, NEKO_UI_ELEMENT_BUTTON, 0x01)
                                   : ui_get_current_element_style(ctx, desc, NEKO_UI_ELEMENT_BUTTON, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt);

    /* handle click or button press for submission */
    if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_UI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[NEKO_UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(r, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]);
    }

    opt |= NEKO_UI_OPT_ISCONTENT;
    ui_draw_rect(ctx, r, style.colors[NEKO_UI_COLOR_BACKGROUND]);
    if (label) {
        ui_draw_control_text(ctx, label_tag, r, &style, opt);
    }

    ui_pop_style(ctx, save);

    if (id_tag) ui_pop_id(ctx);

    return res;
}

i32 ui_checkbox_ex(ui_context_t* ctx, const char* label, i32* state, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &state, sizeof(state));
    ui_rect_t r = ui_layout_next(ctx);
    ui_rect_t box = ui_rect(r.x, r.y, r.h, r.h);
    i32 ox = (i32)(box.w * 0.2f), oy = (i32)(box.h * 0.2f);
    ui_rect_t inner_box = ui_rect(box.x + ox, box.y + oy, box.w - 2 * ox, box.h - 2 * oy);
    ui_update_control(ctx, id, r, 0);

    i32 elementid = NEKO_UI_ELEMENT_BUTTON;
    ui_style_t style = NEKO_DEFAULT_VAL();
    style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
            : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                               : ui_get_current_element_style(ctx, desc, elementid, 0x00);

    /* handle click */
    if ((ctx->mouse_pressed == NEKO_UI_MOUSE_LEFT || (ctx->mouse_pressed && ~opt & NEKO_UI_OPT_LEFTCLICKONLY)) && ctx->focus == id) {
        res |= NEKO_UI_RES_CHANGE;
        *state = !*state;
    }

    /* draw */
    ui_draw_control_frame(ctx, id, box, NEKO_UI_ELEMENT_INPUT, 0);
    if (*state) {
        // Draw in a filled rect
        ui_draw_rect(ctx, inner_box, style.colors[NEKO_UI_COLOR_BACKGROUND]);
    }

    r = ui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    ui_draw_control_text(ctx, label, r, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][0], 0);
    return res;
}

i32 ui_textbox_raw(ui_context_t* ctx, char* buf, i32 bufsz, ui_id id, ui_rect_t rect, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = 0;

    i32 elementid = NEKO_UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    ui_style_t* save = ui_push_style(ctx, &style);

    ui_update_control(ctx, id, rect, opt | NEKO_UI_OPT_HOLDFOCUS);

    if (ctx->focus == id) {
        /* handle text input */
        i32 len = strlen(buf);
        i32 n = NEKO_MIN(bufsz - len - 1, (i32)strlen(ctx->input_text));
        if (n > 0) {
            memcpy(buf + len, ctx->input_text, n);
            len += n;
            buf[len] = '\0';
            res |= NEKO_UI_RES_CHANGE;
        }

        // handle backspace
        if (ctx->key_pressed & NEKO_UI_KEY_BACKSPACE && len > 0) {
            if (ctx->key_down & NEKO_UI_KEY_CTRL) {
                for (--len; len > 0; len--) {
                    // 跳过 utf-8 连续字节
                    if ((buf[len - 1] & 0xc0) == 0x80) continue;
                    // 查找直到分隔符
                    if (strchr(" ()[]{},.-+*=/\\^~|\"'&%#@!<>;:", buf[len - 1])) break;
                }
            } else {
                // 跳过 utf-8 连续字节
                while ((buf[--len] & 0xc0) == 0x80 && len > 0);
            }
            buf[len] = '\0';
            res |= NEKO_UI_RES_CHANGE;
        }

        // TODO: 处理粘贴
        // if (neko_os_key_pressed(NEKO_KEYCODE_V) && ctx->key_down & NEKO_UI_KEY_CTRL) {
        //     const_str clipboard = neko_pf_window_get_clipboard(ctx->window_hndl);
        //     printf("%s --\n", clipboard);
        //     i32 n = NEKO_MIN(bufsz - len - 1, (i32)strlen(clipboard));
        //     if (n > 0) {
        //         memcpy(buf + len, clipboard, n);
        //         len += n;
        //         buf[len] = '\0';
        //         res |= NEKO_UI_RES_CHANGE;
        //     }
        // }

        // handle return
        if (ctx->key_pressed & NEKO_UI_KEY_RETURN) {
            ui_set_focus(ctx, 0);
            res |= NEKO_UI_RES_SUBMIT;
        }
    }

    /* draw */

    // Textbox border
    ui_draw_box(ctx, ui_expand_rect(rect, (i16*)style.border_width), (i16*)style.border_width, style.colors[NEKO_UI_COLOR_BORDER]);

    // Textbox bg
    ui_draw_control_frame(ctx, id, rect, NEKO_UI_ELEMENT_INPUT, opt);

    // Text and carret
    if (ctx->focus == id) {
        ui_style_t* sp = &style;
        Color256* color = &sp->colors[NEKO_UI_COLOR_CONTENT];
        i32 sx = sp->shadow_x;
        i32 sy = sp->shadow_y;
        Color256* sc = &sp->colors[NEKO_UI_COLOR_SHADOW];
        neko_asset_font_t* font = sp->font;
        i32 textw = ui_text_width(font, buf, -1);
        i32 texth = ui_font_height(font);
        i32 ofx = (i32)(rect.w - sp->padding[NEKO_UI_PADDING_RIGHT] - textw - 1);
        i32 textx = (i32)(rect.x + NEKO_MIN(ofx, sp->padding[NEKO_UI_PADDING_LEFT]));
        i32 texty = (i32)(rect.y + (rect.h - texth) / 2);
        i32 cary = (i32)(rect.y + 1);
        ui_push_clip_rect(ctx, rect);

        // Draw text
        ui_draw_control_text(ctx, buf, rect, &style, opt);

        // Draw caret (control alpha based on frame)
        static bool on = true;
        static float ct = 0.f;
        if (~opt & NEKO_UI_OPT_NOCARET) {
            neko_vec2 pos = neko_v2(rect.x, rect.y);

            // Grab stylings
            const i32 padding_left = sp->padding[NEKO_UI_PADDING_LEFT];
            const i32 padding_top = sp->padding[NEKO_UI_PADDING_TOP];
            const i32 padding_right = sp->padding[NEKO_UI_PADDING_RIGHT];
            const i32 padding_bottom = sp->padding[NEKO_UI_PADDING_BOTTOM];
            const i32 align = sp->align_content;
            const i32 justify = sp->justify_content;

            // Determine x placement based on justification
            switch (justify) {
                default:
                case NEKO_UI_JUSTIFY_START: {
                    pos.x = rect.x + padding_left;
                } break;

                case NEKO_UI_JUSTIFY_CENTER: {
                    pos.x = rect.x + (rect.w - textw) * 0.5f;
                } break;

                case NEKO_UI_JUSTIFY_END: {
                    pos.x = rect.x + (rect.w - textw) - padding_right;
                } break;
            }

            // Determine caret position based on style justification
            ui_rect_t cr = ui_rect(pos.x + textw + padding_right, (f32)rect.y + 5.f, 1.f, (f32)rect.h - 10.f);

            if (ctx->last_focus_state == NEKO_UI_ELEMENT_STATE_ON_FOCUS) {
                on = true;
                ct = 0.f;
            }
            ct += 0.1f;
            if (ct >= 3.f) {
                on = !on;
                ct = 0.f;
            }
            Color256 col = *color;
            col.a = on ? col.a : 0;
            ui_draw_rect(ctx, cr, col);
        }

        ui_pop_clip_rect(ctx);
    } else {
        ui_style_t* sp = &style;
        Color256* color = &sp->colors[NEKO_UI_COLOR_CONTENT];
        neko_asset_font_t* font = sp->font;
        i32 sx = sp->shadow_x;
        i32 sy = sp->shadow_y;
        Color256* sc = &sp->colors[NEKO_UI_COLOR_SHADOW];
        i32 textw = ui_text_width(font, buf, -1);
        i32 texth = ui_text_height(font, buf, -1);
        i32 textx = (i32)(rect.x + sp->padding[NEKO_UI_PADDING_LEFT]);
        i32 texty = (i32)(rect.y + (rect.h - texth) / 2);
        ui_push_clip_rect(ctx, rect);
        ui_draw_control_text(ctx, buf, rect, &style, opt);
        ui_pop_clip_rect(ctx);
    }

    ui_pop_style(ctx, save);

    return res;
}

static i32 ui_number_textbox(ui_context_t* ctx, ui_real* value, ui_rect_t r, ui_id id, const ui_selector_desc_t* desc) {
    if (ctx->mouse_pressed == NEKO_UI_MOUSE_LEFT && ctx->key_down & NEKO_UI_KEY_SHIFT && ctx->hover == id) {
        ctx->number_edit = id;
        neko_snprintf(ctx->number_edit_buf, NEKO_UI_MAX_FMT, NEKO_UI_REAL_FMT, *value);
    }
    if (ctx->number_edit == id) {
        // This is broken for some reason...
        i32 res = ui_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, desc, 0);

        if (res & NEKO_UI_RES_SUBMIT || ctx->focus != id) {
            *value = strtod(ctx->number_edit_buf, NULL);
            ctx->number_edit = 0;
        } else {
            return 1;
        }
    }
    return 0;
}

i32 ui_textbox_ex(ui_context_t* ctx, char* buf, i32 bufsz, const ui_selector_desc_t* desc, u64 opt) {
    // Handle animation here...
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &buf, sizeof(buf));
    i32 elementid = NEKO_UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, opt | NEKO_UI_OPT_HOLDFOCUS);
    res |= ui_textbox_raw(ctx, buf, bufsz, id, r, desc, opt);
    ui_pop_style(ctx, save);

    return res;
}

i32 ui_slider_ex(ui_context_t* ctx, ui_real* value, ui_real low, ui_real high, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt) {
    char buf[NEKO_UI_MAX_FMT + 1];
    ui_rect_t thumb;
    i32 x, w, res = 0;
    ui_real last = *value, v = last;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    i32 elementid = NEKO_UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);
    i32 state = ctx->focus == id ? NEKO_UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_UI_ELEMENT_STATE_HOVER : NEKO_UI_ELEMENT_STATE_DEFAULT;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ui_get_current_element_style(ctx, desc, elementid, state);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t base = ui_layout_next(ctx);

    /* handle text input mode */
    if (ui_number_textbox(ctx, &v, base, id, desc)) {
        return res;
    }

    /* handle normal mode */
    ui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && (ctx->mouse_down | ctx->mouse_pressed) == NEKO_UI_MOUSE_LEFT) {
        v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
        if (step) {
            v = (((v + step / 2) / step)) * step;
        }
    }

    /* clamp and store value, update res */
    *value = v = NEKO_CLAMP(v, low, high);
    if (last != v) {
        res |= NEKO_UI_RES_CHANGE;
    }

    /* draw base */
    ui_draw_control_frame(ctx, id, base, NEKO_UI_ELEMENT_INPUT, opt);

    /* draw control */
    w = style.thumb_size;  // Don't like this...
    x = (i32)((v - low) * (base.w - w) / (high - low));
    thumb = ui_rect((f32)base.x + (f32)x, base.y, (f32)w, base.h);
    ui_draw_control_frame(ctx, id, thumb, NEKO_UI_ELEMENT_BUTTON, opt);

    /* draw text    */
    style.colors[NEKO_UI_COLOR_BACKGROUND] = ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][state].colors[NEKO_UI_COLOR_BACKGROUND];
    neko_snprintf(buf, NEKO_UI_MAX_FMT, fmt, v);
    ui_draw_control_text(ctx, buf, base, &style, opt);  // oh...bg

    // Pop style
    ui_pop_style(ctx, save);

    return res;
}

i32 ui_number_ex(ui_context_t* ctx, ui_real* value, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt) {
    char buf[NEKO_UI_MAX_FMT + 1];
    i32 res = 0;
    ui_id id = ui_get_id(ctx, &value, sizeof(value));
    i32 elementid = NEKO_UI_ELEMENT_INPUT;
    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    ui_style_t* save = ui_push_style(ctx, &style);
    ui_rect_t base = ui_layout_next(ctx);
    ui_real last = *value;

    /* handle text input mode */
    if (ui_number_textbox(ctx, value, base, id, desc)) {
        ui_pop_style(ctx, save);
        return res;
    }

    /* handle normal mode */
    ui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
        *value += ctx->mouse_delta.x * step;
    }

    /* set flag if value changed */
    if (*value != last) {
        res |= NEKO_UI_RES_CHANGE;
    }

    /* draw base */
    ui_draw_control_frame(ctx, id, base, NEKO_UI_ELEMENT_INPUT, opt);

    /* draw text    */
    neko_snprintf(buf, NEKO_UI_MAX_FMT, fmt, *value);
    ui_draw_control_text(ctx, buf, base, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][0], opt);

    ui_pop_style(ctx, save);

    return res;
}

static i32 __ui_header(ui_context_t* ctx, const char* label, i32 istreenode, const ui_selector_desc_t* desc, u64 opt) {
    ui_rect_t r;
    i32 active, expanded;
    i32 width = -1;
    ui_layout_row(ctx, 1, &width, 0);

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    ui_id id = ui_get_id(ctx, id_tag, strlen(id_tag));
    i32 idx = ui_pool_get(ctx, ctx->treenode_pool, NEKO_UI_TREENODEPOOL_SIZE, id);

    if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));

    active = (idx >= 0);
    expanded = (opt & NEKO_UI_OPT_EXPANDED) ? !active : active;
    r = ui_layout_next(ctx);
    ui_update_control(ctx, id, r, 0);

    /* handle click */
    active ^= (ctx->mouse_pressed == NEKO_UI_MOUSE_LEFT && ctx->focus == id);

    /* update pool ref */
    if (idx >= 0) {
        if (active) {
            ui_pool_update(ctx, ctx->treenode_pool, idx);
        } else {
            memset(&ctx->treenode_pool[idx], 0, sizeof(ui_pool_item_t));
        }

    } else if (active) {
        ui_pool_init(ctx, ctx->treenode_pool, NEKO_UI_TREENODEPOOL_SIZE, id);
    }

    /* draw */
    if (istreenode) {
        if (ctx->hover == id) {
            ui_draw_frame(ctx, r, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][NEKO_UI_ELEMENT_STATE_HOVER]);
        }
    } else {
        ui_draw_control_frame(ctx, id, r, NEKO_UI_ELEMENT_BUTTON, 0);
    }

    const float sz = 6.f;
    if (expanded) {
        neko_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        neko_vec2 b = neko_vec2_add(a, neko_v2(sz, 0.f));
        neko_vec2 c = neko_vec2_add(a, neko_v2(sz / 2.f, sz));
        ui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][0x00].colors[NEKO_UI_COLOR_CONTENT]);
    } else {
        neko_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        neko_vec2 b = neko_vec2_add(a, neko_v2(sz, sz / 2.f));
        neko_vec2 c = neko_vec2_add(a, neko_v2(0.f, sz));
        ui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][0x00].colors[NEKO_UI_COLOR_CONTENT]);
    }

    // Draw text for treenode
    r.x += r.h - ctx->style->padding[NEKO_UI_PADDING_TOP];
    r.w -= r.h - ctx->style->padding[NEKO_UI_PADDING_BOTTOM];
    ui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_TEXT][0x00], 0);

    if (id_tag) ui_pop_id(ctx);

    return expanded ? NEKO_UI_RES_ACTIVE : 0;
}

i32 ui_header_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) { return __ui_header(ctx, label, 0, desc, opt); }

i32 ui_treenode_begin_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt) {
    i32 res = __ui_header(ctx, label, 1, desc, opt);
    if (res & NEKO_UI_RES_ACTIVE) {
        ui_get_layout(ctx)->indent += ctx->style->indent;
        ui_stack_push(ctx->id_stack, ctx->last_id);
    }

    return res;
}

void ui_treenode_end(ui_context_t* ctx) {
    ui_get_layout(ctx)->indent -= ctx->style->indent;
    ui_pop_id(ctx);
}

// -1 for left, + 1 for right
void ui_tab_item_swap(ui_context_t* ctx, ui_container_t* cnt, i32 direction) {
    ui_tab_bar_t* tab_bar = ui_get_tab_bar(ctx, cnt);
    if (!tab_bar) return;

    i32 item = (i32)cnt->tab_item;
    i32 idx = NEKO_CLAMP(item + direction, 0, (i32)tab_bar->size - 1);

    ui_container_t* scnt = (ui_container_t*)tab_bar->items[idx].data;

    ui_tab_item_t* cti = &tab_bar->items[cnt->tab_item];
    ui_tab_item_t* sti = &tab_bar->items[idx];
    ui_tab_item_t tmp = *cti;

    // Swap cti
    sti->data = cnt;
    cnt->tab_item = sti->idx;

    // Swap sti
    cti->data = scnt;
    scnt->tab_item = cti->idx;

    tab_bar->focus = sti->idx;
}

i32 ui_window_begin_ex(ui_context_t* ctx, const char* title, ui_rect_t rect, bool* open, const ui_selector_desc_t* desc, u64 opt) {
    ui_rect_t body;
    ui_id id = ui_get_id(ctx, title, strlen(title));
    ui_container_t* cnt = ui_get_container_ex(ctx, id, opt);

    char id_tag[256] = NEKO_DEFAULT_VAL();
    char label_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, title, id_tag, sizeof(id_tag), opt);
    ui_parse_label_tag(ctx, title, label_tag, sizeof(label_tag));

    if (cnt && open) {
        cnt->open = *open;
    }

    if (!cnt || !cnt->open) {
        return 0;
    }

    memcpy(cnt->name, label_tag, 256);

    const i32 title_max_size = 100;

    bool new_frame = cnt->frame != ctx->frame;

    i32 state = ctx->active_root == cnt ? NEKO_UI_ELEMENT_STATE_FOCUS : ctx->hover_root == cnt ? NEKO_UI_ELEMENT_STATE_HOVER : NEKO_UI_ELEMENT_STATE_DEFAULT;

    const float split_size = NEKO_UI_SPLIT_SIZE;

    ui_stack_push(ctx->id_stack, id);

    // Get splits
    ui_split_t* split = ui_get_split(ctx, cnt);
    ui_split_t* root_split = ui_get_root_split(ctx, cnt);

    // Get root container
    ui_container_t* root_cnt = ui_get_root_container(ctx, cnt);

    // Cache rect
    if ((cnt->rect.w == 0.f || opt & NEKO_UI_OPT_FORCESETRECT || opt & NEKO_UI_OPT_FULLSCREEN || cnt->flags & NEKO_UI_WINDOW_FLAGS_FIRST_INIT) && new_frame) {
        if (opt & NEKO_UI_OPT_FULLSCREEN) {
            neko_vec2 fb = ctx->framebuffer_size;
            cnt->rect = ui_rect(0, 0, fb.x, fb.y);

            // Set root split rect size
            if (root_split) {
                root_split->rect = cnt->rect;
                ui_update_split(ctx, root_split);
            }
        } else {
            // Set root split rect size
            if (root_split && root_cnt == cnt) {
                root_split->rect = rect;
                ui_update_split(ctx, root_split);
            } else {
                cnt->rect = rect;
            }
        }
        cnt->flags = cnt->flags & ~NEKO_UI_WINDOW_FLAGS_FIRST_INIT;
    }
    ui_begin_root_container(ctx, cnt, opt);
    rect = body = cnt->rect;
    cnt->opt = opt;

    if (opt & NEKO_UI_OPT_DOCKSPACE) {
        cnt->zindex = 0;
    }

    // If parent cannot move/resize, set to this opt as well
    if (root_cnt->opt & NEKO_UI_OPT_NOMOVE) {
        cnt->opt |= NEKO_UI_OPT_NOMOVE;
    }

    if (root_cnt->opt & NEKO_UI_OPT_NORESIZE) {
        cnt->opt |= NEKO_UI_OPT_NORESIZE;
    }

    // If in a tab view, then title has to be handled differently...
    ui_tab_bar_t* tab_bar = cnt->tab_bar ? neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL;
    ui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;

    if (tab_item && tab_item) {
        if (tab_bar->focus == tab_item->idx) {
            cnt->flags |= NEKO_UI_WINDOW_FLAGS_VISIBLE;
            cnt->opt &= !NEKO_UI_OPT_NOINTERACT;
            cnt->opt &= !NEKO_UI_OPT_NOHOVER;
        } else {
            cnt->flags &= ~NEKO_UI_WINDOW_FLAGS_VISIBLE;
            cnt->opt |= NEKO_UI_OPT_NOINTERACT;
            cnt->opt |= NEKO_UI_OPT_NOHOVER;
        }
    }

    bool in_root = false;

    // If hovered root is in the tab group and moused over, then is hovered
    if (tab_bar) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (ctx->hover_root == (ui_container_t*)tab_bar->items[i].data) {
                in_root = true;
                break;
            }
        }
    }

    ui_container_t* s_cnt = cnt;
    if (tab_bar && split) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->split) {
                s_cnt = (ui_container_t*)tab_bar->items[i].data;
            }
        }
    }

    // Do split size/position
    if (split) {
        const ui_style_t* cstyle = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][state];
        const ui_rect_t* sr = &split->rect;
        const float ratio = split->ratio;
        float shsz = split_size;
        const float omr = (1.f - ratio);

        switch (split->type) {
            case NEKO_UI_SPLIT_LEFT: {
                if (split->children[NEKO_UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + sr->w * ratio + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                }

            } break;

            case NEKO_UI_SPLIT_RIGHT: {
                if (split->children[NEKO_UI_SPLIT_NODE_PARENT].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + sr->w * (1.f - ratio) + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                }
            } break;

            case NEKO_UI_SPLIT_TOP: {
                if (split->children[NEKO_UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * ratio - 2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + sr->h * ratio + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;

            case NEKO_UI_SPLIT_BOTTOM: {
                if (split->children[NEKO_UI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + sr->h * (1.f - ratio) + shsz, sr->w - 2.f * shsz, sr->h * (ratio)-2.f * shsz);
                } else {
                    cnt->rect = ui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;
        }
    }

    // Calculate movement
    if (~cnt->opt & NEKO_UI_OPT_NOTITLE && new_frame) {
        ui_rect_t* rp = root_split ? &root_split->rect : &cnt->rect;

        // Cache rect
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        tr.x += split_size;
        ui_id id = ui_get_id(ctx, "!title", 6);
        ui_update_control(ctx, id, tr, opt);

        // Need to move the entire thing
        if ((id == ctx->focus || id == ctx->hover) && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
            // This log_lock id is what I need...

            ctx->active_root = cnt;

            if (tab_bar) {
                ctx->next_focus_root = (ui_container_t*)(tab_bar->items[tab_bar->focus].data);
                ui_bring_to_front(ctx, (ui_container_t*)tab_bar->items[tab_bar->focus].data);
                if (id == ctx->focus && tab_bar->focus != tab_item->idx) ctx->lock_focus = id;
            } else {
                ctx->next_focus_root = cnt;
            }

            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }

        // Tab view
        i32 tw = title_max_size;
        id = ui_get_id(ctx, "!split_tab", 10);
        const float hp = 0.8f;
        tr.x += split_size;
        float h = tr.h * hp;
        float y = tr.y + tr.h * (1.f - hp);

        // Will update tab bar rect size with parent window rect
        if (tab_item) {
            // Get tab bar
            ui_rect_t* r = &tab_bar->rect;

            // Determine width
            i32 tab_width = (i32)NEKO_MIN(r->w / (float)tab_bar->size, title_max_size);
            tw = tab_item->zindex ? (i32)tab_width : (i32)(tab_width + 1.f);

            // Determine position (based on zindex and total width)
            float xoff = 0.f;  // tab_item->zindex ? 2.f : 0.f;
            tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
        }

        ui_rect_t r = ui_rect(tr.x + split_size, y, (f32)tw, h);

        ui_update_control(ctx, id, r, opt);

        // Need to move the entire thing
        if ((id == ctx->hover || id == ctx->focus) && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
            ui_set_focus(ctx, id);
            ctx->next_focus_root = cnt;
            ctx->active_root = cnt;

            // Don't move from tab bar
            if (tab_item) {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h) {
                    ctx->undock_root = cnt;
                }

                if (tab_bar->focus != tab_item->idx) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = NEKO_UI_CNT_FOCUS;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            else if (root_split) {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h) {
                    ctx->undock_root = cnt;
                }
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }
    }

    // Control frame for body movement
    if (~root_cnt->opt & NEKO_UI_OPT_NOMOVE && ~cnt->opt & NEKO_UI_OPT_NOMOVE && ~cnt->opt & NEKO_UI_OPT_NOINTERACT && ~cnt->opt & NEKO_UI_OPT_NOHOVER && new_frame &&
        cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        // Cache rect
        ui_rect_t br = cnt->rect;

        if (~cnt->opt & NEKO_UI_OPT_NOTITLE) {
            br.y += ctx->style->title_height;
            br.h -= ctx->style->title_height;
        }
        ui_id id = ui_get_id(ctx, "!body", 5);
        // ui_update_control(ctx, id, br, (opt | NEKO_UI_OPT_NOSWITCHSTATE));

        // Need to move the entire thing
        if (ctx->hover_root == cnt && !ctx->focus_split && !ctx->focus && !ctx->lock_focus && !ctx->hover && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            ctx->next_focus_root = cnt;
            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else if (tab_bar) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_CNT_FOCUS;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);

                req.type = NEKO_UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }
    }

    // Get parent window if in tab view, then set rect to it (will be a frame off though...)
    if (tab_item && tab_bar) {
        if (tab_bar->focus == tab_item->idx || split) {
            tab_bar->rect = cnt->rect;
        } else {
            cnt->rect = tab_bar->rect;
        }
    }

    // Cache body
    body = cnt->rect;

    if (split) {
        const float sh = split_size * 0.5f;
    }

    if (~opt & NEKO_UI_OPT_NOTITLE) {
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }
        body.y += tr.h;
        body.h -= tr.h;
    }

    i32 zindex = INT32_MAX - 1;
    if (root_split) {
        ui_get_split_lowest_zindex(ctx, root_split, &zindex);
        if (zindex == cnt->zindex) {
            ui_style_t* style = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][state];
            ui_draw_rect(ctx, root_split->rect, style->colors[NEKO_UI_COLOR_BACKGROUND]);
            ui_draw_splits(ctx, root_split);
        }
    }

    // draw body frame
    if (~opt & NEKO_UI_OPT_NOFRAME && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        ui_style_t* style = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][state];

        if (ctx->active_root == root_cnt) {
            i32 f = 0;
        }

        ui_draw_rect(ctx, body, style->colors[NEKO_UI_COLOR_BACKGROUND]);

        // draw border (get root cnt and check state of that)
        if (split) {
            i32 root_state = ctx->active_root == root_cnt ? NEKO_UI_ELEMENT_STATE_FOCUS : ctx->hover_root == root_cnt ? NEKO_UI_ELEMENT_STATE_HOVER : NEKO_UI_ELEMENT_STATE_DEFAULT;

            bool share_split = ctx->active_root && ui_get_root_container(ctx, ctx->active_root) == root_cnt ? true : false;

            // Have to look and see if hovered root shares split...
            ui_style_t* root_style = style;
            if (share_split) {
                root_style = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][NEKO_UI_ELEMENT_STATE_FOCUS];
            } else {
                root_style = state == NEKO_UI_ELEMENT_STATE_FOCUS        ? style
                             : root_state == NEKO_UI_ELEMENT_STATE_FOCUS ? &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][root_state]
                             : root_state == NEKO_UI_ELEMENT_STATE_HOVER ? &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][root_state]
                                                                         : style;
            }
            if (~opt & NEKO_UI_OPT_NOBORDER && root_style->colors[NEKO_UI_COLOR_BORDER].a) {
                ui_draw_box(ctx, ui_expand_rect(split->rect, (i16*)root_style->border_width), (i16*)root_style->border_width, root_style->colors[NEKO_UI_COLOR_BORDER]);
            }
        } else {
            if (~opt & NEKO_UI_OPT_NOBORDER && style->colors[NEKO_UI_COLOR_BORDER].a) {
                ui_draw_box(ctx, ui_expand_rect(cnt->rect, (i16*)style->border_width), (i16*)style->border_width, style->colors[NEKO_UI_COLOR_BORDER]);
            }
        }
    }

    if (split && ~opt & NEKO_UI_OPT_NOCLIP && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        i16 exp[] = {1, 1, 1, 1};
        ui_push_clip_rect(ctx, ui_expand_rect(cnt->rect, exp));
    }

    if (split) {
        const float sh = split_size * 0.5f;
        body.x += sh;
        body.w -= split_size;
    }

    // do title bar
    if (~opt & NEKO_UI_OPT_NOTITLE) {
        ui_style_t* cstyle = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][state];
        ui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }

        // Don't draw this unless you're the bottom window or first frame in a tab group (if in editor_dockspace)
        if (tab_bar) {
            bool lowest = true;
            {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (cnt->zindex > ((ui_container_t*)(tab_bar->items[i].data))->zindex) {
                        lowest = false;
                        break;
                    }
                }
                if (lowest) {
                    ui_draw_frame(ctx, tr, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_PANEL][0x00]);
                    // ui_draw_box(ctx, ui_expand_rect(tr, (i16*)cstyle->border_width), (i16*)cstyle->border_width, cstyle->colors[NEKO_UI_COLOR_BORDER]);
                }
            }
        }

        else {
            ui_draw_frame(ctx, tr, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_PANEL][0x00]);
            // ui_draw_box(ctx, ui_expand_rect(tr, (i16*)cstyle->border_width), cstyle->border_width, cstyle->colors[NEKO_UI_COLOR_BORDER]);
        }

        // Draw tab control
        {

            // Tab view
            i32 tw = title_max_size;
            id = ui_get_id(ctx, "!split_tab", 10);
            const float hp = 0.8f;
            tr.x += split_size;
            float h = tr.h * hp;
            float y = tr.y + tr.h * (1.f - hp);

            // Will update tab bar rect size with parent window rect
            if (tab_item) {
                // Get tab bar
                ui_rect_t* r = &tab_bar->rect;

                // Determine width
                i32 tab_width = (i32)NEKO_MIN(r->w / (float)tab_bar->size, title_max_size);
                tw = (i32)(tab_width - 2.f);

                // Determine position (based on zindex and total width)
                float xoff = !tab_item->zindex ? split_size : 2.f;  // tab_item->zindex ? 2.f : 0.f;
                tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
            }

            ui_rect_t r = ui_rect(tr.x + split_size, y, (f32)tw, h);

            bool hovered = false;

            if (in_root && ui_rect_overlaps_vec2(r, ctx->mouse_pos)) {
                neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
                // neko_idraw_rectvd(dl, neko_v2(r.x, r.y), neko_v2(r.w, r.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_WHITE, R_PRIMITIVE_LINES);
                hovered = true;
            }

            bool other_root_active = ctx->focus_root != cnt;
            if (tab_bar) {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (tab_bar->items[i].data == ctx->focus_root) {
                        other_root_active = false;
                    }
                }
            }

            if (!other_root_active && hovered && ctx->mouse_down == NEKO_UI_MOUSE_LEFT && !ctx->lock_focus) {
                // This is an issue...
                ui_set_focus(ctx, id);
                ctx->lock_focus = id;

                if (tab_item && tab_bar->focus != tab_item->idx) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = NEKO_UI_CNT_FOCUS;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            if (!other_root_active && ctx->mouse_down == NEKO_UI_MOUSE_LEFT && ctx->focus == id) {
                if (ctx->mouse_pos.x < r.x) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = NEKO_UI_TAB_SWAP_LEFT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
                if (ctx->mouse_pos.x > r.x + r.w) {
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = NEKO_UI_TAB_SWAP_RIGHT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            bool tab_focus = (!tab_bar || (tab_bar && tab_item && tab_bar->focus == tab_item->idx));

            Color256 def = ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x00].colors[NEKO_UI_COLOR_BACKGROUND];
            Color256 hov = ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x01].colors[NEKO_UI_COLOR_BACKGROUND];
            Color256 act = ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x02].colors[NEKO_UI_COLOR_BACKGROUND];
            Color256 inactive = color256(10, 10, 10, 50);

            i16 exp[] = {1, 1, 1, 1};
            ui_push_clip_rect(ctx, ui_expand_rect(cnt->rect, exp));

            ui_push_clip_rect(ctx, r);

            ui_draw_rect(ctx, r, id == ctx->focus ? act : hovered ? hov : tab_focus ? def : inactive);
            ui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][state], opt);

            ui_pop_clip_rect(ctx);
            ui_pop_clip_rect(ctx);
        }

        // do `close` button
        /*
        if (~opt & NEKO_UI_OPT_NOCLOSE && false)
        {
            ui_id id = ui_get_id(ctx, "!close", 6);
            ui_rect_t r = ui_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
            tr.w -= r.w;
            ui_draw_icon(ctx, NEKO_UI_ICON_CLOSE, r, ctx->style->colors[NEKO_UI_COLOR_TITLETEXT]);
            ui_update_control(ctx, id, r, opt);
            if (ctx->mouse_pressed == NEKO_UI_MOUSE_LEFT && id == ctx->focus)
            {
                cnt->open = 0;
            }
        }
        */
    }

    // resize to content size
    if (opt & NEKO_UI_OPT_AUTOSIZE && !split) {
        /*
        ui_rect_t r = ui_get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
        cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
        */
    }

    if (split && ~opt & NEKO_UI_OPT_NOCLIP && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        ui_pop_clip_rect(ctx);
    }

    // Draw border
    if (~opt & NEKO_UI_OPT_NOFRAME && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        const int* w = (int*)ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][0x00].border_width;
        const Color256* bc = &ctx->style_sheet->styles[NEKO_UI_ELEMENT_CONTAINER][0x00].colors[NEKO_UI_COLOR_BORDER];
        // ui_draw_box(ctx, ui_expand_rect(cnt->rect, w), w, *bc);
    }

    ui_push_container_body(ctx, cnt, body, desc, opt);

    /* close if this is a popup window and elsewhere was clicked */
    if (opt & NEKO_UI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) {
        cnt->open = 0;
    }

    if (~opt & NEKO_UI_OPT_NOCLIP) {
        if (cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
            ui_push_clip_rect(ctx, cnt->body);
        } else {
            ui_push_clip_rect(ctx, ui_rect(0, 0, 0, 0));
        }
    }

    return NEKO_UI_RES_ACTIVE;
}

void ui_window_end(ui_context_t* ctx) {
    ui_container_t* cnt = ui_get_current_container(ctx);

    // Get root container
    ui_container_t* root_cnt = ui_get_root_container(ctx, cnt);

    // Get splits
    ui_split_t* split = ui_get_split(ctx, cnt);
    ui_split_t* root_split = ui_get_root_split(ctx, cnt);

    const bool new_frame = cnt->frame != ctx->frame;

    // Cache opt
    const u64 opt = cnt->opt;

    // Pop clip for rect
    if (~cnt->opt & NEKO_UI_OPT_NOCLIP) {
        ui_pop_clip_rect(ctx);
    }

    if (~cnt->opt & NEKO_UI_OPT_NOCLIP) {
        ui_push_clip_rect(ctx, cnt->rect);
    }

    // do `resize` handle
    if (~cnt->opt & NEKO_UI_OPT_NORESIZE && ~root_cnt->opt & NEKO_UI_OPT_NORESIZE && new_frame && ~cnt->opt & NEKO_UI_OPT_DOCKSPACE) {
        i32 sz = ctx->style->title_height;
        ui_id id = ui_get_id(ctx, "!resize", 7);
        ui_rect_t r = ui_rect(cnt->rect.x + cnt->rect.w - (f32)sz, cnt->rect.y + cnt->rect.h - (f32)sz, (f32)sz, (f32)sz);
        ui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == NEKO_UI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            if (root_split) {
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = NEKO_UI_SPLIT_RESIZE_SE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                cnt->rect.w = NEKO_MAX(96, cnt->rect.w + ctx->mouse_delta.x);
                cnt->rect.h = NEKO_MAX(64, cnt->rect.h + ctx->mouse_delta.y);
            }
        }

        // Draw resize icon (this will also be a callback)
        const u32 grid = 5;
        const float w = r.w / (float)grid;
        const float h = r.h / (float)grid;
        const float m = 2.f;
        const float o = 5.f;

        Color256 col = ctx->focus == id   ? ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x02].colors[NEKO_UI_COLOR_BACKGROUND]
                       : ctx->hover == id ? ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x01].colors[NEKO_UI_COLOR_BACKGROUND]
                                          : ctx->style_sheet->styles[NEKO_UI_ELEMENT_BUTTON][0x00].colors[NEKO_UI_COLOR_BACKGROUND];

        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * (grid - 2) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 1) - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * grid - o, r.y + h * grid - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 1) - o, r.y + h * grid - o, w - m, h - m), col);
        ui_draw_rect(ctx, ui_rect(r.x + w * (grid - 2) - o, r.y + h * grid - o, w - m, h - m), col);
    }

    if (~cnt->opt & NEKO_UI_OPT_NOCLIP) {
        ui_pop_clip_rect(ctx);
    }

    // draw shadow
    if (~opt & NEKO_UI_OPT_NOFRAME && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        ui_rect_t* r = &cnt->rect;
        u32 ssz = (u32)(split ? NEKO_UI_SPLIT_SIZE : 5);

        ui_draw_rect(ctx, ui_rect(r->x, r->y + r->h, r->w + 1, 1), ctx->style->colors[NEKO_UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x, r->y + r->h, r->w + (f32)ssz, (f32)ssz), ctx->style->colors[NEKO_UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x + r->w, r->y, 1, r->h), ctx->style->colors[NEKO_UI_COLOR_SHADOW]);
        ui_draw_rect(ctx, ui_rect(r->x + r->w, r->y, (f32)ssz, r->h), ctx->style->colors[NEKO_UI_COLOR_SHADOW]);
    }

#define _gui_window_resize_ctrl(ID, RECT, MOUSE, SPLIT_TYPE, MOD_KEY, ...) \
    do {                                                                   \
        if (ctx->key_down == (MOD_KEY)) {                                  \
            ui_id _ID = (ID);                                              \
            ui_rect_t _R = (RECT);                                         \
            ui_update_control(ctx, (ID), _R, opt);                         \
                                                                           \
            if (_ID == ctx->hover || _ID == ctx->focus) {                  \
                ui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);                   \
            }                                                              \
                                                                           \
            if (_ID == ctx->focus && ctx->mouse_down == (MOUSE)) {         \
                ui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);                   \
                if (root_split) {                                          \
                    ui_request_t req = NEKO_DEFAULT_VAL();                 \
                    req.type = (SPLIT_TYPE);                               \
                    req.split = root_split;                                \
                    neko_dyn_array_push(ctx->requests, req);               \
                } else if (new_frame) {                                    \
                    __VA_ARGS__                                            \
                }                                                          \
            }                                                              \
        }                                                                  \
    } while (0)

    // Control frame for body resize
    if (~opt & NEKO_UI_OPT_NORESIZE && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        // Cache main rect
        ui_rect_t* r = root_split ? &root_split->rect : &cnt->rect;
        ui_rect_t* cr = &cnt->rect;

        const float border_ratio = 0.333f;

        if (split) {
            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_w", 8), ui_rect(cr->x, cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), NEKO_UI_MOUSE_RIGHT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_e", 8),
                                    ui_rect(cr->x + cr->w * (1.f - border_ratio), cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), NEKO_UI_MOUSE_LEFT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_n", 8), ui_rect(cr->x + cr->w * border_ratio, cr->y, cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_s", 8),
                                    ui_rect(cr->x + cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_se", 9), ui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * border_ratio, cr->h * border_ratio),
                                    NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_ne", 9), ui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y, cr->w * border_ratio, cr->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_nw", 9), ui_rect(cr->x, cr->y, cr->w * border_ratio, cr->h * border_ratio), NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_INVALID,
                                    NEKO_UI_KEY_CTRL, {});

            _gui_window_resize_ctrl(ui_get_id(ctx, "!split_sw", 9), ui_rect(cr->x, cr->y + cr->h - cr->h * border_ratio, cr->w * border_ratio, cr->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                    NEKO_UI_SPLIT_RESIZE_INVALID, NEKO_UI_KEY_CTRL, {});
        }

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_w", 6), ui_rect(r->x, r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), NEKO_UI_MOUSE_LEFT,
                                NEKO_UI_SPLIT_RESIZE_W, NEKO_UI_KEY_ALT, {
                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_e", 6), ui_rect(r->x + r->w * (1.f - border_ratio), r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)),
                                NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_E, NEKO_UI_KEY_ALT, { r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40); });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_n", 6), ui_rect(r->x + r->w * border_ratio, r->y, r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                NEKO_UI_SPLIT_RESIZE_N, NEKO_UI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_s", 6), ui_rect(r->x + r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * (1.f - 2.f * border_ratio), r->h * border_ratio),
                                NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_S, NEKO_UI_KEY_ALT, { r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40); });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_se", 7), ui_rect(r->x + r->w - r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * border_ratio, r->h * border_ratio),
                                NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_SE, NEKO_UI_KEY_ALT, {
                                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_ne", 7), ui_rect(r->x + r->w - r->w * border_ratio, r->y, r->w * border_ratio, r->h * border_ratio), NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_NE,
                                NEKO_UI_KEY_ALT, {
                                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_nw", 7), ui_rect(r->x, r->y, r->w * border_ratio, r->h * border_ratio), NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_NW, NEKO_UI_KEY_ALT, {
            float h = r->h;
            float max_y = h + r->y;
            r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
            if (fabsf(r->h - h) > 0.f) {
                r->y = NEKO_MIN(r->y + ctx->mouse_delta.y, max_y);
            }

            float w = r->w;
            float max_x = r->x + r->w;
            r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
            if (fabsf(r->w - w) > 0.f) {
                r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
            }
        });

        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_sw", 7), ui_rect(r->x, r->y + r->h - r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), NEKO_UI_MOUSE_LEFT, NEKO_UI_SPLIT_RESIZE_SW,
                                NEKO_UI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);

                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = NEKO_MIN(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        // move instead of resize?
        _gui_window_resize_ctrl(ui_get_id(ctx, "!res_c", 6), ui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), NEKO_UI_MOUSE_LEFT,
                                NEKO_UI_SPLIT_MOVE, NEKO_UI_KEY_ALT, {
                                    ctx->next_focus_root = cnt;
                                    r->x += ctx->mouse_delta.x;
                                    r->y += ctx->mouse_delta.y;
                                });

        static bool capture = false;
        static neko_vec2 mp = {0};
        static ui_rect_t _rect = {0};

        /*
        _gui_window_resize_ctrl(
            ui_get_id(ctx, "!res_c", 5),
            ui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio),
            NEKO_UI_SPLIT_RESIZE_CENTER,
            {
                if (!capture)
                {
                    capture = true;
                    mp = ctx->mouse_pos;
                    _rect = *r;
                }

                // Grow based on dist from center
                neko_vec2 c = neko_v2(r->x + r->w * 0.5f, r->y + r->h * 0.5f);
                neko_vec2 a = neko_vec2_sub(c, mp);
                neko_vec2 b = neko_vec2_sub(c, ctx->mouse_pos);
                neko_vec2 na = neko_vec2_norm(a);
                neko_vec2 nb = neko_vec2_norm(b);
                float dist = neko_vec2_len(neko_vec2_sub(b, a));
                float dot = neko_vec2_dot(na, nb);
                neko_println("len: %.2f, dot: %.2f", dist, dot);

                // Grow rect by dot product (scale dimensions)
                float sign = dot >= 0.f ? 1.f : -1.f;
                float factor = 1.f - dist / 1000.f;
                r->w = _rect.w * factor * sign;
                r->h = _rect.h * factor * sign;

                // Equidistant resize from middle (grow rect based on delta)
                float h = r->h;
                float max_y = h + r->y;
                r->h = NEKO_MAX(r->h - ctx->mouse_delta.y, 40);
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = NEKO_MIN(r->y - ctx->mouse_delta.y, max_y);
                }

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = NEKO_MAX(r->w - ctx->mouse_delta.x, 40);
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = NEKO_MIN(r->x - ctx->mouse_delta.x, max_x);
                }
            });
        */

        if (ctx->mouse_down != NEKO_UI_MOUSE_LEFT) {
            capture = false;
            mp = neko_v2s(0.f);
        }
    }

    // Determine if focus root in same tab group as current window for docking
    bool can_dock = true;
    if (cnt->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
        for (u32 t = 0; t < tab_bar->size; ++t) {
            if (tab_bar->items[t].data == ctx->focus_root) {
                can_dock = false;
            }
        }
    }

    // Do docking overlay (if enabled)
    if (can_dock && ~cnt->opt & NEKO_UI_OPT_NODOCK && ctx->focus_root && ctx->focus_root != cnt &&
        ui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) &&  // This is the incorrect part - need to check if this container isn't being overlapped by another
        ctx->mouse_down == NEKO_UI_MOUSE_LEFT && ~cnt->opt & NEKO_UI_OPT_NOHOVER && cnt->flags & NEKO_UI_WINDOW_FLAGS_VISIBLE) {
        ui_split_t* focus_split = ui_get_root_split(ctx, ctx->focus_root);
        ui_split_t* cnt_split = ui_get_root_split(ctx, cnt);

        // NOTE: this is incorrect...
        if ((!focus_split && !cnt_split) || ((focus_split || cnt_split) && (focus_split != cnt_split))) {
            // Set dockable root container
            ctx->dockable_root = ctx->dockable_root && cnt->zindex > ctx->dockable_root->zindex ? cnt : ctx->dockable_root ? ctx->dockable_root : cnt;
        }
    }

    // Set current frame
    cnt->frame = ctx->frame;

    // Pop root container
    ui_root_container_end(ctx);
}

void ui_popup_open(ui_context_t* ctx, const char* name) {
    ui_container_t* cnt = ui_get_container(ctx, name);

    // Set as hover root so popup isn't closed in window_begin_ex()
    ctx->hover_root = ctx->next_hover_root = cnt;

    // position at mouse cursor, open and bring-to-front
    cnt->rect = ui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100);
    cnt->open = 1;
    ui_bring_to_front(ctx, cnt);
}

i32 ui_popup_begin_ex(ui_context_t* ctx, const char* name, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt) {
    opt |= (NEKO_UI_OPT_POPUP | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_CLOSED);
    return ui_window_begin_ex(ctx, name, r, NULL, NULL, opt);
}

void ui_popup_end(ui_context_t* ctx) { ui_window_end(ctx); }

void ui_panel_begin_ex(ui_context_t* ctx, const char* name, const ui_selector_desc_t* desc, u64 opt) {
    ui_container_t* cnt;
    const i32 elementid = NEKO_UI_ELEMENT_PANEL;
    char id_tag[256] = NEKO_DEFAULT_VAL();
    ui_parse_id_tag(ctx, name, id_tag, sizeof(id_tag), opt);

    // if (id_tag) ui_push_id(ctx, id_tag, strlen(id_tag));
    // else ui_push_id(ctx, name, strlen(name));
    ui_push_id(ctx, name, strlen(name));
    cnt = ui_get_container_ex(ctx, ctx->last_id, opt);
    cnt->rect = ui_layout_next(ctx);

    const ui_id id = ui_get_id(ctx, name, strlen(name));

    ui_style_t style = NEKO_DEFAULT_VAL();
    ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        ui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = ui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : ui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    if (~opt & NEKO_UI_OPT_NOFRAME) {
        ui_draw_frame(ctx, cnt->rect, &style);
    }

    // Need a way to push/pop syles temp styles
    ui_stack_push(ctx->container_stack, cnt);
    ui_push_container_body(ctx, cnt, cnt->rect, desc, opt);
    ui_push_clip_rect(ctx, cnt->body);
}

void ui_panel_end(ui_context_t* ctx) {
    ui_pop_clip_rect(ctx);
    ui_pop_container(ctx);
}

static u8 uint8_slider(ui_context_t* ctx, unsigned char* value, int low, int high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (u8)tmp;
    ui_pop_id(ctx);
    return res;
}

static i32 int32_slider(ui_context_t* ctx, i32* value, i32 low, i32 high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (i32)tmp;
    ui_pop_id(ctx);
    return res;
}

static i16 int16_slider(ui_context_t* ctx, i16* value, i32 low, i32 high, const ui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    ui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = ui_slider_ex(ctx, &tmp, (ui_real)low, (ui_real)high, 0, "%.0f", desc, opt);
    *value = (i16)tmp;
    ui_pop_id(ctx);
    return res;
}

//=== Demos ===//

i32 ui_style_editor(ui_context_t* ctx, ui_style_sheet_t* style_sheet, ui_rect_t rect, bool* open) {
    if (!style_sheet) {
        style_sheet = &ui_default_style_sheet;
    }

    static struct {
        const char* label;
        i32 idx;
    } elements[] = {{"container", NEKO_UI_ELEMENT_CONTAINER}, {"button", NEKO_UI_ELEMENT_BUTTON}, {"panel", NEKO_UI_ELEMENT_PANEL},
                    {"input", NEKO_UI_ELEMENT_INPUT},         {"label", NEKO_UI_ELEMENT_LABEL},   {"text", NEKO_UI_ELEMENT_TEXT},
                    {"scroll", NEKO_UI_ELEMENT_SCROLL},       {"image", NEKO_UI_ELEMENT_IMAGE},   {NULL}};

    static const char* states[] = {"default", "hover", "focus"};

    static struct {
        const char* label;
        i32 idx;
    } colors[] = {{"background", NEKO_UI_COLOR_BACKGROUND},
                  {"border", NEKO_UI_COLOR_BORDER},
                  {"shadow", NEKO_UI_COLOR_SHADOW},
                  {"content", NEKO_UI_COLOR_CONTENT},
                  {"content_shadow", NEKO_UI_COLOR_CONTENT_SHADOW},
                  {"content_background", NEKO_UI_COLOR_CONTENT_BACKGROUND},
                  {"content_border", NEKO_UI_COLOR_CONTENT_BORDER},
                  {NULL}};

    if (ui_window_begin_ex(ctx, "Style_Editor", rect, open, NULL, 0x00)) {
        for (u32 i = 0; elements[i].label; ++i) {
            i32 idx = elements[i].idx;

            if (ui_treenode_begin_ex(ctx, elements[i].label, NULL, 0x00)) {
                for (u32 j = 0; j < NEKO_UI_ELEMENT_STATE_COUNT; ++j) {
                    ui_push_id(ctx, &j, sizeof(j));
                    ui_style_t* s = &style_sheet->styles[idx][j];
                    if (ui_treenode_begin_ex(ctx, states[j], NULL, 0x00)) {
                        ui_style_t* save = ui_push_style(ctx, &ctx->style_sheet->styles[NEKO_UI_ELEMENT_PANEL][0x00]);
                        i32 row[] = {-1};
                        ui_layout_row(ctx, 1, row, 300);
                        ui_panel_begin(ctx, states[j]);
                        {
                            ui_layout_t* l = ui_get_layout(ctx);
                            ui_rect_t* r = &l->body;

                            const i32 ls = 80;

                            // size
                            i32 w = (i32)((l->body.w - ls) * 0.35f);
                            {
                                i32 row[] = {ls, w, w};
                                ui_layout_row(ctx, 3, row, 0);
                            }

                            ui_label(ctx, "size:");
                            ui_slider(ctx, &s->size[0], 0.f, 500.f);
                            ui_slider(ctx, &s->size[1], 0.f, 500.f);

                            w = (i32)((l->body.w - ls) * 0.2f);

                            {
                                i32 row[] = {ls, w, w, w, w};
                                ui_layout_row(ctx, 5, row, 0);
                            }

                            ui_label(ctx, "border_width:");
                            int16_slider(ctx, &s->border_width[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[3], 0, 100, NULL, 0x00);

                            ui_label(ctx, "border_radius:");
                            int16_slider(ctx, &s->border_radius[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[3], 0, 100, NULL, 0x00);

                            // padding/margin
                            ui_label(ctx, "padding:");
                            int32_slider(ctx, &s->padding[0], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[1], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[2], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[3], 0, 100, NULL, 0x00);

                            ui_label(ctx, "margin:");
                            int16_slider(ctx, &s->margin[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[3], 0, 100, NULL, 0x00);

                            // Colors
                            int sw = (i32)(l->body.w * 0.14);
                            {
                                i32 row[] = {80, sw, sw, sw, sw, -1};
                                ui_layout_row(ctx, 6, row, 0);
                            }

                            for (u32 c = 0; colors[c].label; ++c) {
                                ui_label(ctx, colors[c].label);
                                uint8_slider(ctx, &s->colors[c].r, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].g, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].b, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].a, 0, 255, NULL, 0x00);
                                ui_draw_rect(ctx, ui_layout_next(ctx), s->colors[c]);
                            }
                        }
                        ui_panel_end(ctx);
                        ui_pop_style(ctx, save);

                        ui_treenode_end(ctx);
                    }
                    ui_pop_id(ctx);
                }
                ui_treenode_end(ctx);
            }
        }
        ui_window_end(ctx);
    }

    return 0x01;
}

i32 ui_demo_window(ui_context_t* ctx, ui_rect_t rect, bool* open) {

    if (ui_window_begin_ex(ctx, "Demo_Window", rect, open, NULL, 0x00)) {
        ui_container_t* win = ui_get_current_container(ctx);

        if (ui_treenode_begin(ctx, "Help")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }

            ui_panel_begin(ctx, "#!window_info");
            {
                {
                    i32 row[] = {-1};
                    ui_layout_row(ctx, 1, row, 0);
                }
                ui_label(ctx, "ABOUT THIS DEMO:");
                ui_text(ctx, "  - Sections below are demonstrating many aspects of the util.");
                // ui_text(ctx, " 测试中文，你好世界");
            }
            ui_panel_end(ctx);
            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Window Info")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!window_info");
            {
                char buf[64];
                {
                    i32 row[] = {65, -1};
                    ui_layout_row(ctx, 2, row, 0);
                }

                ui_label(ctx, "Position:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.x, win->rect.y);
                ui_label(ctx, buf);

                ui_label(ctx, "Size:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.w, win->rect.h);
                ui_label(ctx, buf);

                ui_label(ctx, "Title:");
                ui_label(ctx, win->name);

                ui_label(ctx, "ID:");
                neko_snprintf(buf, 64, "%zu", win->id);
                ui_label(ctx, buf);

                ui_label(ctx, "Open:");
                neko_snprintf(buf, 64, "%s", win->open ? "true" : "close");
                ui_label(ctx, buf);
            }
            ui_panel_end(ctx);

            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Context State")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!context_state");
            {
                char buf[64];
                {
                    i32 row[] = {80, -1};
                    ui_layout_row(ctx, 2, row, 0);
                }

                ui_label(ctx, "Hovered:");
                neko_snprintf(buf, 64, "%s", ctx->hover_root ? ctx->hover_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Focused:");
                neko_snprintf(buf, 64, "%s", ctx->focus_root ? ctx->focus_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Active:");
                neko_snprintf(buf, 64, "%s", ctx->active_root ? ctx->active_root->name : "NULL");
                ui_label(ctx, buf);

                ui_label(ctx, "Lock Focus:");
                neko_snprintf(buf, 64, "%zu", ctx->lock_focus);
                ui_label(ctx, buf);
            }
            ui_panel_end(ctx);

            ui_treenode_end(ctx);
        }

        if (ui_treenode_begin(ctx, "Widgets")) {
            {
                i32 row[] = {-10};
                ui_layout_row(ctx, 1, row, 170);
            }
            ui_panel_begin(ctx, "#!widgets");
            {
                {
                    i32 row[] = {150, 50};
                    ui_layout_row(ctx, 2, row, 0);
                }
                ui_layout_column_begin(ctx);
                {
                    {
                        i32 row[] = {0};
                        ui_layout_row(ctx, 1, row, 0);
                    }
                    ui_button(ctx, "Button");

                    // Label
                    ui_label(ctx, "Label");

                    // Text
                    {
                        i32 row[] = {150};
                        ui_layout_row(ctx, 1, row, 0);
                    }
                    ui_text(ctx, "This is some text");

                    static char buf[64] = {0};
                    ui_textbox(ctx, buf, 64);
                }
                ui_layout_column_end(ctx);

                ui_layout_column_begin(ctx);
                {
                    ui_label(ctx, "(?)");
                    if (ctx->hover == ctx->last_id) neko_println("HOVERED");
                }
                ui_layout_column_end(ctx);
            }
            ui_panel_end(ctx);
            ui_treenode_end(ctx);
        }

        ui_window_end(ctx);
    }
    return 0x01;
}

#endif