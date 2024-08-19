#include "engine/game.h"
#include "engine/luax.h"
#include "engine/ui.h"

ui_rect_t lua_ui_check_rect(lua_State *L, i32 arg) {
    ui_rect_t rect = {};
    rect.x = luax_number_field(L, arg, "x");
    rect.y = luax_number_field(L, arg, "y");
    rect.w = luax_number_field(L, arg, "w");
    rect.h = luax_number_field(L, arg, "h");
    return rect;
}

void lua_ui_rect_push(lua_State *L, ui_rect_t rect) {
    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "x", rect.x);
    luax_set_number_field(L, "y", rect.y);
    luax_set_number_field(L, "w", rect.w);
    luax_set_number_field(L, "h", rect.h);
}

Color256 lua_ui_check_color(lua_State *L, i32 arg) {
    Color256 color = {};
    color.r = luax_number_field(L, arg, "r");
    color.g = luax_number_field(L, arg, "g");
    color.b = luax_number_field(L, arg, "b");
    color.a = luax_number_field(L, arg, "a");
    return color;
}

// mt_ui_container

static int mt_ui_container_rect(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_ui_rect_push(L, container->rect);
    return 1;
}

static int mt_ui_container_set_rect(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    container->rect = lua_ui_check_rect(L, 2);
    return 0;
}

static int mt_ui_container_body(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_ui_rect_push(L, container->body);
    return 1;
}

static int mt_ui_container_content_size(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->content_size.x);
    lua_pushinteger(L, container->content_size.y);
    return 2;
}

static int mt_ui_container_scroll(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->scroll.x);
    lua_pushinteger(L, container->scroll.y);
    return 2;
}

static int mt_ui_container_set_scroll(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    container->scroll.x = luaL_checknumber(L, 2);
    container->scroll.y = luaL_checknumber(L, 3);
    return 0;
}

static int mt_ui_container_zindex(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushinteger(L, container->zindex);
    return 1;
}

static int mt_ui_container_open(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    lua_pushboolean(L, container->open);
    return 1;
}

int open_mt_ui_container(lua_State *L) {
    luaL_Reg reg[] = {
            {"rect", mt_ui_container_rect},
            {"set_rect", mt_ui_container_set_rect},
            {"body", mt_ui_container_body},
            {"content_size", mt_ui_container_content_size},
            {"scroll", mt_ui_container_scroll},
            {"set_scroll", mt_ui_container_set_scroll},
            {"zindex", mt_ui_container_zindex},
            {"open", mt_ui_container_open},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_ui_container", reg);
    return 0;
}

static int mt_ui_style_size(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_pushinteger(L, style->size.x);
    lua_pushinteger(L, style->size.y);
    return 2;
}

static int mt_ui_style_set_size(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    style->size.x = luaL_checknumber(L, 2);
    style->size.y = luaL_checknumber(L, 3);
    return 0;
}

#define MT_UI_STYLE_GETSET(name)                                                  \
    static int mt_ui_style_##name(lua_State *L) {                                 \
        ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style"); \
        lua_pushinteger(L, style->name);                                          \
        return 1;                                                                 \
    }                                                                             \
    static int mt_ui_style_set_##name(lua_State *L) {                             \
        ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style"); \
        style->name = luaL_checknumber(L, 2);                                     \
        return 0;                                                                 \
    }

// MT_UI_STYLE_GETSET(padding);
MT_UI_STYLE_GETSET(spacing);
MT_UI_STYLE_GETSET(indent);
MT_UI_STYLE_GETSET(title_height);
MT_UI_STYLE_GETSET(scrollbar_size);
MT_UI_STYLE_GETSET(thumb_size);

static int mt_ui_style_color(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    if (colorid < 0 || colorid >= UI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    lua_createtable(L, 0, 4);
    luax_set_number_field(L, "r", style->colors[colorid].r);
    luax_set_number_field(L, "g", style->colors[colorid].g);
    luax_set_number_field(L, "b", style->colors[colorid].b);
    luax_set_number_field(L, "a", style->colors[colorid].a);
    return 1;
}

static int mt_ui_style_set_color(lua_State *L) {
    ui_style_t *style = *(ui_style_t **)luaL_checkudata(L, 1, "mt_ui_style");
    lua_Integer colorid = luaL_checkinteger(L, 2);
    Color256 color = lua_ui_check_color(L, 3);

    if (colorid < 0 || colorid >= UI_COLOR_MAX) {
        return luaL_error(L, "color id out of range");
    }

    style->colors[colorid] = color;
    return 0;
}

int open_mt_ui_style(lua_State *L) {
    luaL_Reg reg[] = {
            {"size", mt_ui_style_size},
            {"set_size", mt_ui_style_set_size},
            // {"padding", mt_ui_style_padding},
            // {"set_padding", mt_ui_style_set_padding},
            {"spacing", mt_ui_style_spacing},
            {"set_spacing", mt_ui_style_set_spacing},
            {"indent", mt_ui_style_indent},
            {"set_indent", mt_ui_style_set_indent},
            {"title_height", mt_ui_style_title_height},
            {"set_title_height", mt_ui_style_set_title_height},
            {"scrollbar_size", mt_ui_style_scrollbar_size},
            {"set_scrollbar_size", mt_ui_style_set_scrollbar_size},
            {"thumb_size", mt_ui_style_thumb_size},
            {"set_thumb_size", mt_ui_style_set_thumb_size},
            {"color", mt_ui_style_color},
            {"set_color", mt_ui_style_set_color},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_ui_style", reg);
    return 0;
}

// mt_ui_ref

static int mt_ui_ref_gc(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");
    mem_free(ref);
    return 0;
}

static int mt_ui_ref_get(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");

    switch (ref->kind) {
        case MUIRefKind_Boolean:
            lua_pushboolean(L, ref->boolean);
            return 1;
        case MUIRefKind_Real:
            lua_pushnumber(L, ref->real);
            return 1;
        case MUIRefKind_String:
            lua_pushstring(L, ref->string);
            return 1;
        case MUIRefKind_Nil:
        default:
            return 0;
    }
}

static int mt_ui_ref_set(lua_State *L) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, 1, "mt_ui_ref");
    lua_ui_set_ref(L, ref, 2);
    return 0;
}

int open_mt_ui_ref(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_ui_ref_gc},
            {"get", mt_ui_ref_get},
            {"set", mt_ui_ref_set},
            {nullptr, nullptr},
    };
    luax_new_class(L, "mt_ui_ref", reg);
    return 0;
}

// ui api

void lua_ui_set_ref(lua_State *L, MUIRef *ref, i32 arg) {
    i32 type = lua_type(L, arg);
    switch (type) {
        case LUA_TBOOLEAN:
            ref->kind = MUIRefKind_Boolean;
            ref->boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            ref->kind = MUIRefKind_Real;
            ref->real = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            ref->kind = MUIRefKind_String;
            String s = luax_check_string(L, arg);
            if (s.len > array_size(ref->string) - 1) {
                s.len = array_size(ref->string) - 1;
            }
            memcpy(ref->string, s.data, s.len);
            ref->string[s.len] = '\0';
            break;
        }
        default:
            ref->kind = MUIRefKind_Nil;
    }
}

MUIRef *lua_ui_check_ref(lua_State *L, i32 arg, MUIRefKind kind) {
    MUIRef *ref = *(MUIRef **)luaL_checkudata(L, arg, "mt_ui_ref");

    if (ref->kind != kind) {
        memset(ref, 0, sizeof(MUIRef));
        ref->kind = kind;
    }

    return ref;
}

static ui_context_t *ui_ctx() { return &g_app->ui; }

static int l_ui_set_focus(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    ui_set_focus(ui_ctx(), (ui_id)id);
    return 0;
}

static int l_ui_get_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_id id = ui_get_id(ui_ctx(), name.data, name.len);
    lua_pushinteger(L, (lua_Integer)id);
    return 1;
}

static int l_ui_push_id(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_push_id(ui_ctx(), name.data, name.len);
    return 0;
}

static int l_ui_pop_id(lua_State *L) {
    ui_pop_id(ui_ctx());
    return 0;
}

static int l_ui_push_clip_rect(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    ui_push_clip_rect(ui_ctx(), rect);
    return 0;
}

static int l_ui_pop_clip_rect(lua_State *L) {
    ui_pop_clip_rect(ui_ctx());
    return 0;
}

static int l_ui_get_clip_rect(lua_State *L) {
    ui_rect_t rect = ui_get_clip_rect(ui_ctx());
    lua_ui_rect_push(L, rect);
    return 1;
}

static int l_ui_check_clip(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);

    i32 clip = ui_check_clip(ui_ctx(), rect);
    lua_pushinteger(L, clip);
    return 1;
}

static int l_ui_get_current_container(lua_State *L) {
    ui_container_t *container = ui_get_current_container(ui_ctx());
    luax_ptr_userdata(L, container, "mt_ui_container");
    return 1;
}

static int l_ui_get_container(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_container_t *container = ui_get_container(ui_ctx(), name.data);
    luax_ptr_userdata(L, container, "mt_ui_container");
    return 1;
}

static int l_ui_bring_to_front(lua_State *L) {
    ui_container_t *container = *(ui_container_t **)luaL_checkudata(L, 1, "mt_ui_container");
    ui_bring_to_front(ui_ctx(), container);
    return 0;
}

static int l_ui_set_clip(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    ui_set_clip(ui_ctx(), rect);
    return 0;
}

static int l_ui_draw_rect(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    Color256 color = lua_ui_check_color(L, 2);
    ui_draw_rect(ui_ctx(), rect, color);
    return 0;
}

static int l_ui_draw_box(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    Color256 color = lua_ui_check_color(L, 2);
    // i16 border_width = 1;
    ui_style_t *style = ui_ctx()->style;
    ui_draw_box(ui_ctx(), rect, style->border_width, color);
    return 0;
}

static int l_ui_draw_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    lua_Number x = luaL_checknumber(L, 2);
    lua_Number y = luaL_checknumber(L, 3);
    Color256 color = lua_ui_check_color(L, 4);

    vec2 pos = {(f32)x, (f32)y};
    ui_draw_text(ui_ctx(), nullptr, str.data, str.len, pos, color, 0, 0, color);
    return 0;
}

// static int l_ui_draw_icon(lua_State *L) {
//     lua_Integer id = luaL_checkinteger(L, 1);
//     ui_rect_t rect = lua_ui_check_rect(L, 2);
//     Color256 color = lua_ui_check_color(L, 3);
//     ui_draw_icon(ui_ctx(), id, rect, color);
//     return 0;
// }

static int l_ui_layout_row(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 2);

    i32 widths[UI_MAX_WIDTHS] = {};

    lua_Integer n = luax_len(L, 1);
    if (n > UI_MAX_WIDTHS) {
        n = UI_MAX_WIDTHS;
    }

    for (i32 i = 0; i < n; i++) {
        luax_geti(L, 1, i + 1);
        widths[i] = luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    ui_layout_row(ui_ctx(), n, widths, height);
    return 0;
}

static int l_ui_layout_width(lua_State *L) {
    lua_Number width = luaL_checknumber(L, 1);
    ui_layout_width(ui_ctx(), width);
    return 0;
}

static int l_ui_layout_height(lua_State *L) {
    lua_Number height = luaL_checknumber(L, 1);
    ui_layout_height(ui_ctx(), height);
    return 0;
}

static int l_ui_layout_begin_column(lua_State *L) {
    ui_layout_column_begin(ui_ctx());
    return 0;
}

static int l_ui_layout_end_column(lua_State *L) {
    ui_layout_column_end(ui_ctx());
    return 0;
}

static int l_ui_layout_set_next(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    bool relative = lua_toboolean(L, 2);
    ui_layout_set_next(ui_ctx(), rect, relative);
    return 0;
}

static int l_ui_layout_next(lua_State *L) {
    ui_rect_t rect = ui_layout_next(ui_ctx());
    lua_ui_rect_push(L, rect);
    return 1;
}

static int l_ui_draw_control_frame(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    ui_rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    ui_draw_control_frame(ui_ctx(), id, rect, colorid, opt);
    return 0;
}

static int l_ui_draw_control_text(lua_State *L) {
    String str = luax_check_string(L, 1);
    ui_rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer colorid = luaL_checkinteger(L, 3);
    lua_Integer opt = luaL_checkinteger(L, 4);
    ui_draw_control_text(ui_ctx(), str.data, rect, ui_ctx()->style, opt);
    return 0;
}

static int l_ui_mouse_over(lua_State *L) {
    ui_rect_t rect = lua_ui_check_rect(L, 1);
    int res = ui_mouse_over(ui_ctx(), rect);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_update_control(lua_State *L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    ui_rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer opt = luaL_checkinteger(L, 3);
    ui_update_control(ui_ctx(), id, rect, opt);
    return 0;
}

static int l_ui_text(lua_State *L) {
    String text = luax_check_string(L, 1);
    ui_text(ui_ctx(), text.data);
    return 0;
}

static int l_ui_label(lua_State *L) {
    String text = luax_check_string(L, 1);
    ui_label(ui_ctx(), text.data);
    return 0;
}

static int l_ui_button(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer icon = luaL_optinteger(L, 2, 0);
    lua_Integer opt = luaL_optinteger(L, 3, UI_ALIGN_CENTER);
    i32 res = ui_button_ex(ui_ctx(), text.data, NULL, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_checkbox(lua_State *L) {
    String text = luax_check_string(L, 1);
    MUIRef *ref = lua_ui_check_ref(L, 2, MUIRefKind_Boolean);

    i32 res = ui_checkbox(ui_ctx(), text.data, &ref->boolean);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_textbox_raw(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_String);
    lua_Integer id = luaL_checkinteger(L, 2);
    ui_rect_t rect = lua_ui_check_rect(L, 3);
    i32 opt = luaL_optinteger(L, 4, 0);

    i32 res = ui_textbox_raw(ui_ctx(), ref->string, array_size(ref->string), id, rect, NULL, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_textbox(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_String);
    i32 opt = luaL_optinteger(L, 2, 0);

    i32 res = ui_textbox_ex(ui_ctx(), ref->string, array_size(ref->string), NULL, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_slider(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_Real);
    ui_real low = (ui_real)luaL_checknumber(L, 2);
    ui_real high = (ui_real)luaL_checknumber(L, 3);
    ui_real step = (ui_real)luaL_optnumber(L, 4, 0);
    String fmt = luax_opt_string(L, 5, UI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 6, UI_ALIGN_CENTER);

    i32 res = ui_slider_ex(ui_ctx(), &ref->real, low, high, step, fmt.data, NULL, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_number(lua_State *L) {
    MUIRef *ref = lua_ui_check_ref(L, 1, MUIRefKind_Real);
    ui_real step = (ui_real)luaL_checknumber(L, 2);
    String fmt = luax_opt_string(L, 3, UI_SLIDER_FMT);
    i32 opt = luaL_optinteger(L, 4, UI_ALIGN_CENTER);

    i32 res = ui_number_ex(ui_ctx(), &ref->real, step, fmt.data, NULL, opt);
    lua_pushinteger(L, res);
    return 1;
}

static int l_ui_header(lua_State *L) {
    String text = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    i32 res = ui_header_ex(ui_ctx(), text.data, NULL, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_begin_treenode(lua_State *L) {
    String label = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);

    i32 res = ui_treenode_begin_ex(ui_ctx(), label.data, NULL, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_treenode(lua_State *L) {
    ui_treenode_end(ui_ctx());
    return 0;
}

static int l_ui_begin_window(lua_State *L) {
    String title = luax_check_string(L, 1);
    ui_rect_t rect = lua_ui_check_rect(L, 2);
    lua_Integer opt = luaL_optinteger(L, 3, 0x00);

    i32 res = ui_window_begin_ex(ui_ctx(), title.data, rect, 0, NULL, opt);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_window(lua_State *L) {
    ui_window_end(ui_ctx());
    return 0;
}

static int l_ui_open_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_popup_open(ui_ctx(), name.data);
    return 0;
}

static int l_ui_begin_popup(lua_State *L) {
    String name = luax_check_string(L, 1);
    ui_rect_t rect = lua_ui_check_rect(L, 2);
    i32 res = ui_popup_begin(ui_ctx(), name.data, rect);
    lua_pushboolean(L, res);
    return 1;
}

static int l_ui_end_popup(lua_State *L) {
    ui_popup_end(ui_ctx());
    return 0;
}

static int l_ui_begin_panel(lua_State *L) {
    String name = luax_check_string(L, 1);
    lua_Integer opt = luaL_optinteger(L, 2, 0);
    ui_panel_begin_ex(ui_ctx(), name.data, NULL, opt);
    return 0;
}

static int l_ui_end_panel(lua_State *L) {
    ui_panel_end(ui_ctx());
    return 0;
}

static int l_ui_get_hover(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->hover);
    return 1;
}

static int l_ui_get_focus(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->focus);
    return 1;
}

static int l_ui_get_last_id(lua_State *L) {
    lua_pushinteger(L, ui_ctx()->last_id);
    return 1;
}

static int l_ui_get_style(lua_State *L) {
    luax_ptr_userdata(L, ui_ctx()->style, "mt_ui_style");
    return 1;
}

static int l_ui_rect(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "x", luaL_checknumber(L, 1));
    luax_set_int_field(L, "y", luaL_checknumber(L, 2));
    luax_set_int_field(L, "w", luaL_checknumber(L, 3));
    luax_set_int_field(L, "h", luaL_checknumber(L, 4));
    return 1;
}

static int l_ui_color(lua_State *L) {
    lua_createtable(L, 0, 4);
    luax_set_int_field(L, "r", luaL_checknumber(L, 1));
    luax_set_int_field(L, "g", luaL_checknumber(L, 2));
    luax_set_int_field(L, "b", luaL_checknumber(L, 3));
    luax_set_int_field(L, "a", luaL_checknumber(L, 4));
    return 1;
}

static int l_ui_ref(lua_State *L) {
    MUIRef *ref = (MUIRef *)mem_alloc(sizeof(MUIRef));
    lua_ui_set_ref(L, ref, 1);
    luax_ptr_userdata(L, ref, "mt_ui_ref");
    return 1;
}

static int l_ui_begin(lua_State *L) {
    ui_begin(ui_ctx(), NULL);
    return 0;
}

static int l_ui_end(lua_State *L) {
    ui_end(ui_ctx(), true);
    return 0;
}

int open_ui(lua_State *L) {
    luaL_Reg reg[] = {
            {"Begin", l_ui_begin},
            {"End", l_ui_end},

            {"set_focus", l_ui_set_focus},
            {"get_id", l_ui_get_id},
            {"push_id", l_ui_push_id},
            {"pop_id", l_ui_pop_id},
            {"push_clip_rect", l_ui_push_clip_rect},
            {"pop_clip_rect", l_ui_pop_clip_rect},
            {"get_clip_rect", l_ui_get_clip_rect},
            {"check_clip", l_ui_check_clip},
            {"get_current_container", l_ui_get_current_container},
            {"get_container", l_ui_get_container},
            {"bring_to_front", l_ui_bring_to_front},

            {"set_clip", l_ui_set_clip},
            {"draw_rect", l_ui_draw_rect},
            {"draw_box", l_ui_draw_box},
            {"draw_text", l_ui_draw_text},
            // {"draw_icon", l_ui_draw_icon},

            {"layout_row", l_ui_layout_row},
            {"layout_width", l_ui_layout_width},
            {"layout_height", l_ui_layout_height},
            {"layout_begin_column", l_ui_layout_begin_column},
            {"layout_end_column", l_ui_layout_end_column},
            {"layout_set_next", l_ui_layout_set_next},
            {"layout_next", l_ui_layout_next},

            {"draw_control_frame", l_ui_draw_control_frame},
            {"draw_control_text", l_ui_draw_control_text},
            {"mouse_over", l_ui_mouse_over},
            {"update_control", l_ui_update_control},

            {"text", l_ui_text},
            {"label", l_ui_label},
            {"button", l_ui_button},
            {"checkbox", l_ui_checkbox},
            {"textbox_raw", l_ui_textbox_raw},
            {"textbox", l_ui_textbox},
            {"slider", l_ui_slider},
            {"number", l_ui_number},
            {"header", l_ui_header},
            {"begin_treenode", l_ui_begin_treenode},
            {"end_treenode", l_ui_end_treenode},
            {"begin_window", l_ui_begin_window},
            {"end_window", l_ui_end_window},
            {"open_popup", l_ui_open_popup},
            {"begin_popup", l_ui_begin_popup},
            {"end_popup", l_ui_end_popup},
            {"begin_panel", l_ui_begin_panel},
            {"end_panel", l_ui_end_panel},

            // access
            {"get_hover", l_ui_get_hover},
            {"get_focus", l_ui_get_focus},
            {"get_last_id", l_ui_get_last_id},
            {"get_style", l_ui_get_style},

            // utility
            {"rect", l_ui_rect},
            {"color", l_ui_color},
            {"ref", l_ui_ref},
            {nullptr, nullptr},
    };

    luaL_newlib(L, reg);

    // luax_set_string_field(L, "VERSION", UI_VERSION);

    luax_set_int_field(L, "COMMANDLIST_SIZE", UI_COMMANDLIST_SIZE);
    luax_set_int_field(L, "ROOTLIST_SIZE", UI_ROOTLIST_SIZE);
    luax_set_int_field(L, "CONTAINERSTACK_SIZE", UI_CONTAINERSTACK_SIZE);
    luax_set_int_field(L, "CLIPSTACK_SIZE", UI_CLIPSTACK_SIZE);
    luax_set_int_field(L, "IDSTACK_SIZE", UI_IDSTACK_SIZE);
    luax_set_int_field(L, "LAYOUTSTACK_SIZE", UI_LAYOUTSTACK_SIZE);
    luax_set_int_field(L, "CONTAINERPOOL_SIZE", UI_CONTAINERPOOL_SIZE);
    luax_set_int_field(L, "TREENODEPOOL_SIZE", UI_TREENODEPOOL_SIZE);
    luax_set_int_field(L, "MAX_WIDTHS", UI_MAX_WIDTHS);
    luax_set_string_field(L, "REAL_FMT", UI_REAL_FMT);
    luax_set_string_field(L, "SLIDER_FMT", UI_SLIDER_FMT);
    luax_set_int_field(L, "MAX_FMT", UI_MAX_FMT);

    luax_set_int_field(L, "CLIP_PART", UI_CLIP_PART);
    luax_set_int_field(L, "CLIP_ALL", UI_CLIP_ALL);

    luax_set_int_field(L, "COMMAND_JUMP", UI_COMMAND_JUMP);
    luax_set_int_field(L, "COMMAND_CLIP", UI_COMMAND_CLIP);
    luax_set_int_field(L, "COMMAND_SHAPE", UI_COMMAND_SHAPE);
    luax_set_int_field(L, "COMMAND_TEXT", UI_COMMAND_TEXT);
    luax_set_int_field(L, "COMMAND_ICON", UI_COMMAND_ICON);

    // luax_set_int_field(L, "COLOR_TEXT", UI_COLOR_TEXT);
    // luax_set_int_field(L, "COLOR_BORDER", UI_COLOR_BORDER);
    // luax_set_int_field(L, "COLOR_WINDOWBG", UI_COLOR_WINDOWBG);
    // luax_set_int_field(L, "COLOR_TITLEBG", UI_COLOR_TITLEBG);
    // luax_set_int_field(L, "COLOR_TITLETEXT", UI_COLOR_TITLETEXT);
    // luax_set_int_field(L, "COLOR_PANELBG", UI_COLOR_PANELBG);
    // luax_set_int_field(L, "COLOR_BUTTON", UI_COLOR_BUTTON);
    // luax_set_int_field(L, "COLOR_BUTTONHOVER", UI_COLOR_BUTTONHOVER);
    // luax_set_int_field(L, "COLOR_BUTTONFOCUS", UI_COLOR_BUTTONFOCUS);
    // luax_set_int_field(L, "COLOR_BASE", UI_COLOR_BASE);
    // luax_set_int_field(L, "COLOR_BASEHOVER", UI_COLOR_BASEHOVER);
    // luax_set_int_field(L, "COLOR_BASEFOCUS", UI_COLOR_BASEFOCUS);
    // luax_set_int_field(L, "COLOR_SCROLLBASE", UI_COLOR_SCROLLBASE);
    // luax_set_int_field(L, "COLOR_SCROLLTHUMB", UI_COLOR_SCROLLTHUMB);

    luax_set_int_field(L, "ICON_CLOSE", UI_ICON_CLOSE);
    luax_set_int_field(L, "ICON_CHECK", UI_ICON_CHECK);
    luax_set_int_field(L, "ICON_COLLAPSED", UI_ICON_COLLAPSED);
    luax_set_int_field(L, "ICON_EXPANDED", UI_ICON_EXPANDED);

    luax_set_int_field(L, "RES_ACTIVE", UI_RES_ACTIVE);
    luax_set_int_field(L, "RES_SUBMIT", UI_RES_SUBMIT);
    luax_set_int_field(L, "RES_CHANGE", UI_RES_CHANGE);

    luax_set_int_field(L, "OPT_ALIGNCENTER", UI_ALIGN_CENTER);
    // luax_set_int_field(L, "OPT_ALIGNRIGHT", UI_OPT_ALIGN_RIGHT);
    luax_set_int_field(L, "OPT_NOINTERACT", UI_OPT_NOINTERACT);
    luax_set_int_field(L, "OPT_NOFRAME", UI_OPT_NOFRAME);
    luax_set_int_field(L, "OPT_NORESIZE", UI_OPT_NORESIZE);
    luax_set_int_field(L, "OPT_NOSCROLL", UI_OPT_NOSCROLL);
    luax_set_int_field(L, "OPT_NOCLOSE", UI_OPT_NOCLOSE);
    luax_set_int_field(L, "OPT_NOTITLE", UI_OPT_NOTITLE);
    luax_set_int_field(L, "OPT_HOLDFOCUS", UI_OPT_HOLDFOCUS);
    luax_set_int_field(L, "OPT_AUTOSIZE", UI_OPT_AUTOSIZE);
    luax_set_int_field(L, "OPT_POPUP", UI_OPT_POPUP);
    luax_set_int_field(L, "OPT_CLOSED", UI_OPT_CLOSED);
    luax_set_int_field(L, "OPT_EXPANDED", UI_OPT_EXPANDED);

    return 1;
}

#if 0


namespace neko::imgui::util {

struct TableInteger {
    const char* name;
    lua_Integer value;
};

using GenerateAny = void (*)(lua_State* L);
struct TableAny {
    const char* name;
    GenerateAny value;
};

struct strbuf {
    char* data;
    size_t size;
};

struct input_context {
    lua_State* L;
    int callback;
};

lua_Integer field_tointeger(lua_State* L, int idx, lua_Integer i);
lua_Number field_tonumber(lua_State* L, int idx, lua_Integer i);
bool field_toboolean(lua_State* L, int idx, lua_Integer i);
ImTextureID get_texture_id(lua_State* L, int idx);
const char* format(lua_State* L, int idx);
strbuf* strbuf_create(lua_State* L, int idx);
strbuf* strbuf_get(lua_State* L, int idx);
int input_callback(ImGuiInputTextCallbackData* data);
void create_table(lua_State* L, std::span<TableInteger> l);
void set_table(lua_State* L, std::span<TableAny> l);
void struct_gen(lua_State* L, const char* name, std::span<luaL_Reg> funcs, std::span<luaL_Reg> setters, std::span<luaL_Reg> getters);
void flags_gen(lua_State* L, const char* name);
void init(lua_State* L);

}  // namespace neko::imgui::util


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
    if (luax_pcall(L, 1, 1) != LUA_OK) {
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

#endif