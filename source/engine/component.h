#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "engine/ecs/entity.h"
#include "engine/graphics.h"
#include "engine/input.h"

// can set scale, rotation, position -- 按该顺序应用的转换

NEKO_SCRIPT(transform,

            NEKO_EXPORT void transform_add(NativeEntity ent);

            NEKO_EXPORT void transform_remove(NativeEntity ent);

            NEKO_EXPORT bool transform_has(NativeEntity ent);

            // 根转换具有父级 = entity_nil
            NEKO_EXPORT void transform_set_parent(NativeEntity ent, NativeEntity parent);

            NEKO_EXPORT NativeEntity transform_get_parent(NativeEntity ent);

            NEKO_EXPORT ecs_id_t transform_get_num_children(NativeEntity ent);

            NEKO_EXPORT NativeEntity * transform_get_children(NativeEntity ent);
            // 脱离父项和所有子项
            NEKO_EXPORT void transform_detach_all(NativeEntity ent);
            // destroy ent and all children
            NEKO_EXPORT void transform_destroy_rec(NativeEntity ent);

            NEKO_EXPORT void transform_set_position(NativeEntity ent, vec2 pos);

            NEKO_EXPORT vec2 transform_get_position(NativeEntity ent);

            NEKO_EXPORT void transform_translate(NativeEntity ent, vec2 trans);

            NEKO_EXPORT void transform_set_rotation(NativeEntity ent, Scalar rot);

            NEKO_EXPORT Scalar transform_get_rotation(NativeEntity ent);

            NEKO_EXPORT void transform_rotate(NativeEntity ent, Scalar rot);

            NEKO_EXPORT void transform_set_scale(NativeEntity ent, vec2 scale);

            NEKO_EXPORT vec2 transform_get_scale(NativeEntity ent);

            NEKO_EXPORT vec2 transform_get_world_position(NativeEntity ent);

            NEKO_EXPORT Scalar transform_get_world_rotation(NativeEntity ent);

            NEKO_EXPORT vec2 transform_get_world_scale(NativeEntity ent);

            NEKO_EXPORT mat3 transform_get_world_matrix(NativeEntity ent);  // world-space

            NEKO_EXPORT mat3 transform_get_matrix(NativeEntity ent);  // parent-space

            NEKO_EXPORT vec2 transform_local_to_world(NativeEntity ent, vec2 v);

            NEKO_EXPORT vec2 transform_world_to_local(NativeEntity ent, vec2 v);

            NEKO_EXPORT ecs_id_t transform_get_dirty_count(NativeEntity ent);

            NEKO_EXPORT void transform_set_save_filter_rec(NativeEntity ent, bool filter);

)

void transform_init();
void transform_fini();
int transform_update_all(App* app, event_t evt);
void transform_save_all(Store* s);
void transform_load_all(Store* s);

#include "engine/components/tiledmap.hpp"

/*
 * if no current camera, the (inverse) view matrix is identity, which means
 * the view is a 2x2 unit box at the center of the world
 */

NEKO_SCRIPT(camera,

            NEKO_EXPORT void camera_add(NativeEntity ent);

            NEKO_EXPORT void camera_remove(NativeEntity ent);

            NEKO_EXPORT bool camera_has(NativeEntity ent);

            // set camera to use in edit mode -- not saved/loaded
            NEKO_EXPORT void camera_set_edit_camera(NativeEntity ent);

            // set/get currently active camera -- entity_nil if none
            NEKO_EXPORT void camera_set_current(NativeEntity ent, bool current);

            NEKO_EXPORT bool camera_get_current(NativeEntity ent);

            NEKO_EXPORT void camera_set_current_camera(NativeEntity ent);

            NEKO_EXPORT NativeEntity camera_get_current_camera();

            // number of world units to fit vertically on screen
            NEKO_EXPORT void camera_set_viewport_height(NativeEntity ent, Scalar height);

            NEKO_EXPORT Scalar camera_get_viewport_height(NativeEntity ent);

            NEKO_EXPORT mat3 camera_get_inverse_view_matrix();

            // screen-space coordinates <-> world coordinates transformations
            NEKO_EXPORT vec2 camera_world_to_pixels(vec2 p);

            NEKO_EXPORT vec2 camera_world_to_unit(vec2 p);

            NEKO_EXPORT vec2 camera_pixels_to_world(vec2 p);

            NEKO_EXPORT vec2 camera_unit_to_world(vec2 p);

)

const mat3* camera_get_inverse_view_matrix_ptr();  // for quick GLSL binding

void camera_init();
void camera_fini();
int camera_update_all(App* app, event_t evt);
void camera_save_all(Store* s);
void camera_load_all(Store* s);

NEKO_SCRIPT(sprite,

            NEKO_EXPORT void sprite_set_atlas(const char* filename);

            NEKO_EXPORT const char* sprite_get_atlas();

            NEKO_EXPORT void sprite_add(NativeEntity ent);

            NEKO_EXPORT void sprite_remove(NativeEntity ent);

            NEKO_EXPORT bool sprite_has(NativeEntity ent);

            // size to draw in world units, centered at transform position
            NEKO_EXPORT void sprite_set_size(NativeEntity ent, vec2 size);

            NEKO_EXPORT vec2 sprite_get_size(NativeEntity ent);

            // bottom left corner of atlas region in pixels
            NEKO_EXPORT void sprite_set_texcell(NativeEntity ent, vec2 texcell);

            NEKO_EXPORT vec2 sprite_get_texcell(NativeEntity ent);

            // size of atlas region in pixels
            NEKO_EXPORT void sprite_set_texsize(NativeEntity ent, vec2 texsize);

            NEKO_EXPORT vec2 sprite_get_texsize(NativeEntity ent);

            // lower depth drawn on top
            NEKO_EXPORT void sprite_set_depth(NativeEntity ent, int depth);

            NEKO_EXPORT int sprite_get_depth(NativeEntity ent);

)

void sprite_init();
void sprite_fini();
int sprite_update_all(App* app, event_t evt);
void sprite_draw_all();
void sprite_save_all(Store* s);
void sprite_load_all(Store* s);

NEKO_SCRIPT(
        edit,

        NEKO_EXPORT void edit_set_enabled(bool e);

        NEKO_EXPORT bool edit_get_enabled();

        // 无法选择不可编辑的实体
        NEKO_EXPORT void edit_set_editable(NativeEntity ent, bool editable);

        NEKO_EXPORT bool edit_get_editable(NativeEntity ent);

        // 每个维度上都是非负的 零意味着没有网格
        NEKO_EXPORT void edit_set_grid_size(vec2 size);

        NEKO_EXPORT vec2 edit_get_grid_size();

        // 用于点击选择等
        NEKO_EXPORT void edit_bboxes_update(NativeEntity ent, BBox bbox);  // 合并bbox

        NEKO_EXPORT bool edit_bboxes_has(NativeEntity ent);

        NEKO_EXPORT BBox edit_bboxes_get(NativeEntity ent);

        NEKO_EXPORT unsigned int edit_bboxes_get_num();

        struct EntityBBoxPair {
            NativeEntity ent;
            BBox bbox;
        };

        NEKO_EXPORT NativeEntity edit_bboxes_get_nth_ent(unsigned int n); NEKO_EXPORT BBox edit_bboxes_get_nth_bbox(unsigned int n);

        NEKO_EXPORT void edit_bboxes_set_selected(NativeEntity ent, bool selected);

        // 在两个世界空间坐标之间画一条线
        NEKO_EXPORT void edit_line_add(vec2 a, vec2 b, Scalar point_size, Color color);

)

void edit_line_add_xy(vec2 p, Scalar point_size, Color color);

struct App;

int edit_clear(App* app, event_t evt);

void edit_init();
void edit_fini();
int edit_update_all(App* app, event_t evt);
void edit_draw_all();
void edit_save_all(Store* s);
void edit_load_all(Store* s);

NEKO_SCRIPT(
        gui,

        /*
         * get root entity of which all gui entites are descendants
         *
         * this entity's transform is set up so that all its children
         * have screen pixel coordinates and stay in the camera's view
         */
        NEKO_EXPORT NativeEntity gui_get_root();

        // gui

        NEKO_EXPORT void gui_add(NativeEntity ent);

        NEKO_EXPORT void gui_remove(NativeEntity ent);

        NEKO_EXPORT bool gui_has(NativeEntity ent);

        NEKO_EXPORT void gui_set_color(NativeEntity ent, Color color);

        NEKO_EXPORT Color gui_get_color(NativeEntity ent);

        NEKO_EXPORT void gui_set_visible(NativeEntity ent, bool visible);

        NEKO_EXPORT bool gui_get_visible(NativeEntity ent);

        NEKO_EXPORT void gui_set_focusable(NativeEntity ent, bool focusable);

        NEKO_EXPORT bool gui_get_focusable(NativeEntity ent);

        NEKO_EXPORT void gui_set_captures_events(NativeEntity ent, bool captures_events);

        NEKO_EXPORT bool gui_get_captures_events(NativeEntity ent);

        typedef enum GuiAlign{
                GA_MIN = 0,    // h: left, v: bottom
                GA_MID = 1,    // h: center, v: center
                GA_MAX = 2,    // h: right, v: top
                GA_TABLE = 3,  // h: left-right table, v: top-down table
                GA_NONE = 4,   // manual position
        } GuiAlign;

        NEKO_EXPORT void gui_set_halign(NativeEntity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_halign(NativeEntity ent);

        NEKO_EXPORT void gui_set_valign(NativeEntity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_valign(NativeEntity ent);

        NEKO_EXPORT void gui_set_padding(NativeEntity ent, vec2 padding);  // h, v

        NEKO_EXPORT vec2 gui_get_padding(NativeEntity ent);  // h, v

        // entity_nil for no focus
        NEKO_EXPORT void gui_set_focused_entity(NativeEntity ent);

        NEKO_EXPORT NativeEntity gui_get_focused_entity();

        NEKO_EXPORT void gui_set_focus(NativeEntity ent, bool focus);

        NEKO_EXPORT bool gui_get_focus(NativeEntity ent);

        NEKO_EXPORT bool gui_has_focus();  // whether any gui is focused

        NEKO_EXPORT void gui_fire_event_changed(NativeEntity ent);

        NEKO_EXPORT bool gui_event_focus_enter(NativeEntity ent);

        NEKO_EXPORT bool gui_event_focus_exit(NativeEntity ent);

        NEKO_EXPORT bool gui_event_changed(NativeEntity ent);  // input value changed

        NEKO_EXPORT MouseCode gui_event_mouse_down(NativeEntity ent);

        NEKO_EXPORT MouseCode gui_event_mouse_up(NativeEntity ent);

        NEKO_EXPORT KeyCode gui_event_key_down(NativeEntity ent);

        NEKO_EXPORT KeyCode gui_event_key_up(NativeEntity ent);

        // whether some gui element captured the current event
        NEKO_EXPORT bool gui_captured_event();

        // gui_rect

        NEKO_EXPORT void gui_rect_add(NativeEntity ent);

        NEKO_EXPORT void gui_rect_remove(NativeEntity ent);

        NEKO_EXPORT bool gui_rect_has(NativeEntity ent);

        NEKO_EXPORT void gui_rect_set_size(NativeEntity ent, vec2 size);

        NEKO_EXPORT vec2 gui_rect_get_size(NativeEntity ent);

        NEKO_EXPORT void gui_rect_set_hfit(NativeEntity ent, bool fit);

        NEKO_EXPORT bool gui_rect_get_hfit(NativeEntity ent);

        NEKO_EXPORT void gui_rect_set_vfit(NativeEntity ent, bool fit);

        NEKO_EXPORT bool gui_rect_get_vfit(NativeEntity ent);

        NEKO_EXPORT void gui_rect_set_hfill(NativeEntity ent, bool fill);

        NEKO_EXPORT bool gui_rect_get_hfill(NativeEntity ent);

        NEKO_EXPORT void gui_rect_set_vfill(NativeEntity ent, bool fill);

        NEKO_EXPORT bool gui_rect_get_vfill(NativeEntity ent);

        // gui_text

        NEKO_EXPORT void gui_text_add(NativeEntity ent);

        NEKO_EXPORT void gui_text_remove(NativeEntity ent);

        NEKO_EXPORT bool gui_text_has(NativeEntity ent);

        NEKO_EXPORT void gui_text_set_str(NativeEntity ent, const char* str);

        NEKO_EXPORT const char* gui_text_get_str(NativeEntity ent);

        NEKO_EXPORT void gui_text_set_cursor(NativeEntity ent, int cursor);

        // gui_textedit

        NEKO_EXPORT void gui_textedit_add(NativeEntity ent);

        NEKO_EXPORT void gui_textedit_remove(NativeEntity ent);

        NEKO_EXPORT bool gui_textedit_has(NativeEntity ent);

        NEKO_EXPORT void gui_textedit_set_cursor(NativeEntity ent, unsigned int cursor);

        NEKO_EXPORT unsigned int gui_textedit_get_cursor(NativeEntity ent);

        NEKO_EXPORT void gui_textedit_set_numerical(NativeEntity ent, bool numerical);

        NEKO_EXPORT bool gui_textedit_get_numerical(NativeEntity ent);

        NEKO_EXPORT Scalar gui_textedit_get_num(NativeEntity ent);  // 0 if not numerical

)

int gui_event_clear(App* app, event_t evt);

void gui_init();
void gui_fini();
int gui_update_all(App* app, event_t evt);
void gui_draw_all();
void gui_key_down(KeyCode key);
void gui_key_up(KeyCode key);
void gui_char_down(unsigned int c);
void gui_mouse_down(MouseCode mouse);
void gui_mouse_up(MouseCode mouse);
void gui_save_all(Store* s);
void gui_load_all(Store* s);

int open_db(lua_State* L);

#endif
