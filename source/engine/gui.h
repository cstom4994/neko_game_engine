#pragma once

#include "engine/base.h"
#include "engine/entity.h"
#include "engine/input.h"

NEKO_SCRIPT(
        gui,

        /*
         * get root entity of which all gui entites are descendants
         *
         * this entity's transform is set up so that all its children
         * have screen pixel coordinates and stay in the camera's view
         */
        NEKO_EXPORT Entity gui_get_root();

        // gui

        NEKO_EXPORT void gui_add(Entity ent);

        NEKO_EXPORT void gui_remove(Entity ent);

        NEKO_EXPORT bool gui_has(Entity ent);

        NEKO_EXPORT void gui_set_color(Entity ent, Color color);

        NEKO_EXPORT Color gui_get_color(Entity ent);

        NEKO_EXPORT void gui_set_visible(Entity ent, bool visible);

        NEKO_EXPORT bool gui_get_visible(Entity ent);

        NEKO_EXPORT void gui_set_focusable(Entity ent, bool focusable);

        NEKO_EXPORT bool gui_get_focusable(Entity ent);

        NEKO_EXPORT void gui_set_captures_events(Entity ent, bool captures_events);

        NEKO_EXPORT bool gui_get_captures_events(Entity ent);

        typedef enum GuiAlign GuiAlign; enum GuiAlign{
                GA_MIN = 0,    // h: left, v: bottom
                GA_MID = 1,    // h: center, v: center
                GA_MAX = 2,    // h: right, v: top
                GA_TABLE = 3,  // h: left-right table, v: top-down table
                GA_NONE = 4,   // manual position
        };

        NEKO_EXPORT void gui_set_halign(Entity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_halign(Entity ent);

        NEKO_EXPORT void gui_set_valign(Entity ent, GuiAlign align);

        NEKO_EXPORT GuiAlign gui_get_valign(Entity ent);

        NEKO_EXPORT void gui_set_padding(Entity ent, LuaVec2 padding);  // h, v

        NEKO_EXPORT LuaVec2 gui_get_padding(Entity ent);  // h, v

        // entity_nil for no focus
        NEKO_EXPORT void gui_set_focused_entity(Entity ent);

        NEKO_EXPORT Entity gui_get_focused_entity();

        NEKO_EXPORT void gui_set_focus(Entity ent, bool focus);

        NEKO_EXPORT bool gui_get_focus(Entity ent);

        NEKO_EXPORT bool gui_has_focus();  // whether any gui is focused

        NEKO_EXPORT void gui_fire_event_changed(Entity ent);

        NEKO_EXPORT bool gui_event_focus_enter(Entity ent);

        NEKO_EXPORT bool gui_event_focus_exit(Entity ent);

        NEKO_EXPORT bool gui_event_changed(Entity ent);  // input value changed

        NEKO_EXPORT MouseCode gui_event_mouse_down(Entity ent);

        NEKO_EXPORT MouseCode gui_event_mouse_up(Entity ent);

        NEKO_EXPORT KeyCode gui_event_key_down(Entity ent);

        NEKO_EXPORT KeyCode gui_event_key_up(Entity ent);

        // whether some gui element captured the current event
        NEKO_EXPORT bool gui_captured_event();

        // gui_rect

        NEKO_EXPORT void gui_rect_add(Entity ent); NEKO_EXPORT void gui_rect_remove(Entity ent); NEKO_EXPORT bool gui_rect_has(Entity ent);

        NEKO_EXPORT void gui_rect_set_size(Entity ent, LuaVec2 size); NEKO_EXPORT LuaVec2 gui_rect_get_size(Entity ent);

        NEKO_EXPORT void gui_rect_set_hfit(Entity ent, bool fit); NEKO_EXPORT bool gui_rect_get_hfit(Entity ent); NEKO_EXPORT void gui_rect_set_vfit(Entity ent, bool fit);
        NEKO_EXPORT bool gui_rect_get_vfit(Entity ent);

        NEKO_EXPORT void gui_rect_set_hfill(Entity ent, bool fill); NEKO_EXPORT bool gui_rect_get_hfill(Entity ent); NEKO_EXPORT void gui_rect_set_vfill(Entity ent, bool fill);
        NEKO_EXPORT bool gui_rect_get_vfill(Entity ent);

        // gui_text

        NEKO_EXPORT void gui_text_add(Entity ent); 
        
        NEKO_EXPORT void gui_text_remove(Entity ent); 
        
        NEKO_EXPORT bool gui_text_has(Entity ent);

        NEKO_EXPORT void gui_text_set_str(Entity ent, const char* str); 
        
        NEKO_EXPORT const char* gui_text_get_str(Entity ent); 
        
        NEKO_EXPORT void gui_text_set_cursor(Entity ent, int cursor);

        // gui_textedit

        NEKO_EXPORT void gui_textedit_add(Entity ent); NEKO_EXPORT void gui_textedit_remove(Entity ent); NEKO_EXPORT bool gui_textedit_has(Entity ent);

        NEKO_EXPORT void gui_textedit_set_cursor(Entity ent, unsigned int cursor); NEKO_EXPORT unsigned int gui_textedit_get_cursor(Entity ent);

        NEKO_EXPORT void gui_textedit_set_numerical(Entity ent, bool numerical); NEKO_EXPORT bool gui_textedit_get_numerical(Entity ent);
        NEKO_EXPORT Scalar gui_textedit_get_num(Entity ent);  // 0 if not numerical

)

void gui_event_clear();

void gui_init();
void gui_fini();
void gui_update_all();
void gui_draw_all();
void gui_key_down(KeyCode key);
void gui_key_up(KeyCode key);
void gui_char_down(unsigned int c);
void gui_mouse_down(MouseCode mouse);
void gui_mouse_up(MouseCode mouse);
void gui_save_all(Store* s);
void gui_load_all(Store* s);
