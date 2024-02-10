

#define NEKO_GUI_INCLUDE_FIXED_TYPES
#define NEKO_GUI_INCLUDE_STANDARD_IO
#define NEKO_GUI_INCLUDE_STANDARD_VARARGS
#define NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
#define NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NEKO_GUI_INCLUDE_FONT_BAKING
#define NEKO_GUI_INCLUDE_DEFAULT_FONT
#define NEKO_GUI_UINT_DRAW_INDEX
#define NEKO_GUI_INCLUDE_STANDARD_BOOL

#ifndef NEKO_GUI_IMPL_H
#define NEKO_GUI_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#define NEKO_GUI_UNDEFINED (-1.0f)
#define NEKO_GUI_UTF_INVALID 0xFFFD
#define NEKO_GUI_UTF_SIZE 4
#ifndef NEKO_GUI_INPUT_MAX
#define NEKO_GUI_INPUT_MAX 16
#endif
#ifndef NEKO_GUI_MAX_NUMBER_BUFFER
#define NEKO_GUI_MAX_NUMBER_BUFFER 64
#endif
#ifndef NEKO_GUI_SCROLLBAR_HIDING_TIMEOUT
#define NEKO_GUI_SCROLLBAR_HIDING_TIMEOUT 4.0f
#endif

#define NEKO_GUI_API extern
#define NEKO_GUI_LIB static

#define NEKO_GUI_INTERN static
#define NEKO_GUI_STORAGE static
#define NEKO_GUI_GLOBAL static

#define NEKO_GUI_FLAG(x) (1 << (x))
#define NEKO_GUI_STRINGIFY(x) #x
#define NEKO_GUI_MACRO_STRINGIFY(x) NEKO_GUI_STRINGIFY(x)
#define NEKO_GUI_STRING_JOIN_IMMEDIATE(arg1, arg2) arg1##arg2
#define NEKO_GUI_STRING_JOIN_DELAY(arg1, arg2) NEKO_GUI_STRING_JOIN_IMMEDIATE(arg1, arg2)
#define NEKO_GUI_STRING_JOIN(arg1, arg2) NEKO_GUI_STRING_JOIN_DELAY(arg1, arg2)

#ifdef _MSC_VER
#define NEKO_GUI_UNIQUE_NAME(name) NEKO_GUI_STRING_JOIN(name, __COUNTER__)
#else
#define NEKO_GUI_UNIQUE_NAME(name) NEKO_GUI_STRING_JOIN(name, __LINE__)
#endif

#ifndef NEKO_GUI_STATIC_ASSERT
#define NEKO_GUI_STATIC_ASSERT(exp) typedef char NEKO_GUI_UNIQUE_NAME(_dummy_array)[(exp) ? 1 : -1]
#endif

#ifndef NEKO_GUI_FILE_LINE
#ifdef _MSC_VER
#define NEKO_GUI_FILE_LINE __FILE__ ":" NEKO_GUI_MACRO_STRINGIFY(__COUNTER__)
#else
#define NEKO_GUI_FILE_LINE __FILE__ ":" NEKO_GUI_MACRO_STRINGIFY(__LINE__)
#endif
#endif

#define NEKO_GUI_MIN(a, b) ((a) < (b) ? (a) : (b))
#define NEKO_GUI_MAX(a, b) ((a) < (b) ? (b) : (a))
#define NEKO_GUI_CLAMP(i, v, x) (NEKO_GUI_MAX(NEKO_GUI_MIN(v, x), i))

#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
#include <stdarg.h>
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#include <sal.h>
#define NEKO_GUI_PRINTF_FORMAT_STRING _Printf_format_string_
#else
#define NEKO_GUI_PRINTF_FORMAT_STRING
#endif
#if defined(__GNUC__)
#define NEKO_GUI_PRINTF_VARARG_FUNC(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))
#define NEKO_GUI_PRINTF_VALIST_FUNC(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, 0)))
#else
#define NEKO_GUI_PRINTF_VARARG_FUNC(fmtargnumber)
#define NEKO_GUI_PRINTF_VALIST_FUNC(fmtargnumber)
#endif
#endif

#ifdef NEKO_GUI_INCLUDE_FIXED_TYPES
#include <stdint.h>
#define NEKO_GUI_INT8 int8_t
#define NEKO_GUI_UINT8 uint8_t
#define NEKO_GUI_INT16 int16_t
#define NEKO_GUI_UINT16 uint16_t
#define NEKO_GUI_INT32 int32_t
#define NEKO_GUI_UINT32 uint32_t
#define NEKO_GUI_SIZE_TYPE uintptr_t
#define NEKO_GUI_POINTER_TYPE uintptr_t
#else
#ifndef NEKO_GUI_INT8
#define NEKO_GUI_INT8 signed char
#endif
#ifndef NEKO_GUI_UINT8
#define NEKO_GUI_UINT8 unsigned char
#endif
#ifndef NEKO_GUI_INT16
#define NEKO_GUI_INT16 signed short
#endif
#ifndef NEKO_GUI_UINT16
#define NEKO_GUI_UINT16 unsigned short
#endif
#ifndef NEKO_GUI_INT32
#if defined(_MSC_VER)
#define NEKO_GUI_INT32 __int32
#else
#define NEKO_GUI_INT32 signed int
#endif
#endif
#ifndef NEKO_GUI_UINT32
#if defined(_MSC_VER)
#define NEKO_GUI_UINT32 unsigned __int32
#else
#define NEKO_GUI_UINT32 unsigned int
#endif
#endif
#ifndef NEKO_GUI_SIZE_TYPE
#if defined(_WIN64) && defined(_MSC_VER)
#define NEKO_GUI_SIZE_TYPE unsigned __int64
#elif (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
#define NEKO_GUI_SIZE_TYPE unsigned __int32
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#define NEKO_GUI_SIZE_TYPE unsigned long
#else
#define NEKO_GUI_SIZE_TYPE unsigned int
#endif
#else
#define NEKO_GUI_SIZE_TYPE unsigned long
#endif
#endif
#ifndef NEKO_GUI_POINTER_TYPE
#if defined(_WIN64) && defined(_MSC_VER)
#define NEKO_GUI_POINTER_TYPE unsigned __int64
#elif (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
#define NEKO_GUI_POINTER_TYPE unsigned __int32
#elif defined(__GNUC__) || defined(__clang__)
#if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#define NEKO_GUI_POINTER_TYPE unsigned long
#else
#define NEKO_GUI_POINTER_TYPE unsigned int
#endif
#else
#define NEKO_GUI_POINTER_TYPE unsigned long
#endif
#endif
#endif

//#include <stdbool.h>
#define NEKO_GUI_BOOL bool

typedef NEKO_GUI_INT8 neko_gui_char;
typedef NEKO_GUI_UINT8 neko_gui_uchar;
typedef NEKO_GUI_UINT8 neko_gui_byte;
typedef NEKO_GUI_INT16 neko_gui_short;
typedef NEKO_GUI_UINT16 neko_gui_ushort;
typedef NEKO_GUI_INT32 neko_gui_int;
typedef NEKO_GUI_UINT32 neko_gui_uint;
typedef NEKO_GUI_SIZE_TYPE neko_gui_size;
typedef NEKO_GUI_POINTER_TYPE neko_gui_ptr;
typedef NEKO_GUI_BOOL neko_gui_bool;

typedef neko_gui_uint neko_gui_hash;
typedef neko_gui_uint neko_gui_flags;
typedef neko_gui_uint neko_gui_rune;

NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_short) == 2);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_ushort) == 2);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_uint) == 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_int) == 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_byte) == 1);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_flags) >= 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_rune) >= 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_size) >= sizeof(void *));
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_ptr) >= sizeof(void *));
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_bool) == 1);

struct neko_gui_buffer;
struct neko_gui_allocator;
struct neko_gui_command_buffer;
struct neko_gui_draw_command;
struct neko_gui_convert_config;
struct neko_gui_style_item;
struct neko_gui_text_edit;
struct neko_gui_draw_list;
struct neko_gui_user_font;
struct neko_gui_panel;
struct neko_gui_context;
struct neko_gui_draw_vertex_layout_element;
struct neko_gui_style_button;
struct neko_gui_style_toggle;
struct neko_gui_style_selectable;
struct neko_gui_style_slide;
struct neko_gui_style_progress;
struct neko_gui_style_scrollbar;
struct neko_gui_style_edit;
struct neko_gui_style_property;
struct neko_gui_style_chart;
struct neko_gui_style_combo;
struct neko_gui_style_tab;
struct neko_gui_style_window_header;
struct neko_gui_style_window;

enum { neko_gui_false, neko_gui_true };
struct neko_gui_color {
    neko_gui_byte r, g, b, a;
};
struct neko_gui_colorf {
    float r, g, b, a;
};
struct neko_gui_vec2 {
    float x, y;
};
struct neko_gui_vec2i {
    short x, y;
};
struct neko_gui_rect {
    float x, y, w, h;
};
struct neko_gui_recti {
    short x, y, w, h;
};
typedef char neko_gui_glyph[NEKO_GUI_UTF_SIZE];
typedef union {
    void *ptr;
    int id;
} neko_gui_handle;
struct neko_gui_image {
    neko_gui_handle handle;
    neko_gui_ushort w, h;
    neko_gui_ushort region[4];
};
struct neko_gui_nine_slice {
    struct neko_gui_image img;
    neko_gui_ushort l, t, r, b;
};
struct neko_gui_cursor {
    struct neko_gui_image img;
    struct neko_gui_vec2 size, offset;
};
struct neko_gui_scroll {
    neko_gui_uint x, y;
};

enum neko_gui_heading { NEKO_GUI_UP, NEKO_GUI_RIGHT, NEKO_GUI_DOWN, NEKO_GUI_LEFT };
enum neko_gui_button_behavior { NEKO_GUI_BUTTON_DEFAULT, NEKO_GUI_BUTTON_REPEATER };
enum neko_gui_modify { NEKO_GUI_FIXED = neko_gui_false, NEKO_GUI_MODIFIABLE = neko_gui_true };
enum neko_gui_orientation { NEKO_GUI_VERTICAL, NEKO_GUI_HORIZONTAL };
enum neko_gui_collapse_states { NEKO_GUI_MINIMIZED = neko_gui_false, NEKO_GUI_MAXIMIZED = neko_gui_true };
enum neko_gui_show_states { NEKO_GUI_HIDDEN = neko_gui_false, NEKO_GUI_SHOWN = neko_gui_true };
enum neko_gui_chart_type { NEKO_GUI_CHART_LINES, NEKO_GUI_CHART_COLUMN, NEKO_GUI_CHART_MAX };
enum neko_gui_chart_event { NEKO_GUI_CHART_HOVERING = 0x01, NEKO_GUI_CHART_CLICKED = 0x02 };
enum neko_gui_color_format { NEKO_GUI_RGB, NEKO_GUI_RGBA };
enum neko_gui_popup_type { NEKO_GUI_POPUP_STATIC, NEKO_GUI_POPUP_DYNAMIC };
enum neko_gui_layout_format { NEKO_GUI_DYNAMIC, NEKO_GUI_STATIC };
enum neko_gui_tree_type { NEKO_GUI_TREE_NODE, NEKO_GUI_TREE_TAB };

typedef void *(*neko_gui_plugin_alloc)(neko_gui_handle, void *old, neko_gui_size);
typedef void (*neko_gui_plugin_free)(neko_gui_handle, void *old);
typedef neko_gui_bool (*neko_gui_plugin_filter)(const struct neko_gui_text_edit *, neko_gui_rune unicode);
typedef void (*neko_gui_plugin_paste)(neko_gui_handle, struct neko_gui_text_edit *);
typedef void (*neko_gui_plugin_copy)(neko_gui_handle, const char *, int len);

struct neko_gui_allocator {
    neko_gui_handle userdata;
    neko_gui_plugin_alloc alloc;
    neko_gui_plugin_free free;
};
enum neko_gui_symbol_type {
    NEKO_GUI_SYMBOL_NONE,
    NEKO_GUI_SYMBOL_X,
    NEKO_GUI_SYMBOL_UNDERSCORE,
    NEKO_GUI_SYMBOL_CIRCLE_SOLID,
    NEKO_GUI_SYMBOL_CIRCLE_OUTLINE,
    NEKO_GUI_SYMBOL_RECT_SOLID,
    NEKO_GUI_SYMBOL_RECT_OUTLINE,
    NEKO_GUI_SYMBOL_TRIANGLE_UP,
    NEKO_GUI_SYMBOL_TRIANGLE_DOWN,
    NEKO_GUI_SYMBOL_TRIANGLE_LEFT,
    NEKO_GUI_SYMBOL_TRIANGLE_RIGHT,
    NEKO_GUI_SYMBOL_PLUS,
    NEKO_GUI_SYMBOL_MINUS,
    NEKO_GUI_SYMBOL_MAX
};

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR

NEKO_GUI_API neko_gui_bool neko_gui_init_default(struct neko_gui_context *, const struct neko_gui_user_font *);
#endif

NEKO_GUI_API neko_gui_bool neko_gui_init_fixed(struct neko_gui_context *, void *memory, neko_gui_size size, const struct neko_gui_user_font *);

NEKO_GUI_API neko_gui_bool neko_gui_init_impl(struct neko_gui_context *, struct neko_gui_allocator *, const struct neko_gui_user_font *);

NEKO_GUI_API neko_gui_bool neko_gui_init_custom(struct neko_gui_context *, struct neko_gui_buffer *cmds, struct neko_gui_buffer *pool, const struct neko_gui_user_font *);

NEKO_GUI_API void neko_gui_clear(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_free(struct neko_gui_context *);
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA

NEKO_GUI_API void neko_gui_set_user_data(struct neko_gui_context *, neko_gui_handle handle);
#endif

enum neko_gui_keys {
    NEKO_GUI_KEY_NONE,
    NEKO_GUI_KEY_SHIFT,
    NEKO_GUI_KEY_CTRL,
    NEKO_GUI_KEY_DEL,
    NEKO_GUI_KEY_ENTER,
    NEKO_GUI_KEY_TAB,
    NEKO_GUI_KEY_BACKSPACE,
    NEKO_GUI_KEY_COPY,
    NEKO_GUI_KEY_CUT,
    NEKO_GUI_KEY_PASTE,
    NEKO_GUI_KEY_UP,
    NEKO_GUI_KEY_DOWN,
    NEKO_GUI_KEY_LEFT,
    NEKO_GUI_KEY_RIGHT,

    NEKO_GUI_KEY_TEXT_INSERT_MODE,
    NEKO_GUI_KEY_TEXT_REPLACE_MODE,
    NEKO_GUI_KEY_TEXT_RESET_MODE,
    NEKO_GUI_KEY_TEXT_LINE_START,
    NEKO_GUI_KEY_TEXT_LINE_END,
    NEKO_GUI_KEY_TEXT_START,
    NEKO_GUI_KEY_TEXT_END,
    NEKO_GUI_KEY_TEXT_UNDO,
    NEKO_GUI_KEY_TEXT_REDO,
    NEKO_GUI_KEY_TEXT_SELECT_ALL,
    NEKO_GUI_KEY_TEXT_WORD_LEFT,
    NEKO_GUI_KEY_TEXT_WORD_RIGHT,

    NEKO_GUI_KEY_SCROLL_START,
    NEKO_GUI_KEY_SCROLL_END,
    NEKO_GUI_KEY_SCROLL_DOWN,
    NEKO_GUI_KEY_SCROLL_UP,
    NEKO_GUI_KEY_MAX
};
enum neko_gui_buttons { NEKO_GUI_BUTTON_LEFT, NEKO_GUI_BUTTON_MIDDLE, NEKO_GUI_BUTTON_RIGHT, NEKO_GUI_BUTTON_DOUBLE, NEKO_GUI_BUTTON_MAX };

NEKO_GUI_API void neko_gui_input_begin(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_input_motion(struct neko_gui_context *, int x, int y);

NEKO_GUI_API void neko_gui_input_key(struct neko_gui_context *, enum neko_gui_keys, neko_gui_bool down);

NEKO_GUI_API void neko_gui_input_button(struct neko_gui_context *, enum neko_gui_buttons, int x, int y, neko_gui_bool down);

NEKO_GUI_API void neko_gui_input_scroll(struct neko_gui_context *, struct neko_gui_vec2 val);

NEKO_GUI_API void neko_gui_input_char(struct neko_gui_context *, char);

NEKO_GUI_API void neko_gui_input_glyph(struct neko_gui_context *, const neko_gui_glyph);

NEKO_GUI_API void neko_gui_input_unicode(struct neko_gui_context *, neko_gui_rune);

NEKO_GUI_API void neko_gui_input_end(struct neko_gui_context *);

enum neko_gui_anti_aliasing { NEKO_GUI_ANTI_ALIASING_OFF, NEKO_GUI_ANTI_ALIASING_ON };
enum neko_gui_convert_result {
    NEKO_GUI_CONVERT_SUCCESS = 0,
    NEKO_GUI_CONVERT_INVALID_PARAM = 1,
    NEKO_GUI_CONVERT_COMMAND_BUFFER_FULL = NEKO_GUI_FLAG(1),
    NEKO_GUI_CONVERT_VERTEX_BUFFER_FULL = NEKO_GUI_FLAG(2),
    NEKO_GUI_CONVERT_ELEMENT_BUFFER_FULL = NEKO_GUI_FLAG(3)
};
struct neko_gui_draw_null_texture {
    neko_gui_handle texture;
    struct neko_gui_vec2 uv;
};
struct neko_gui_convert_config {
    float global_alpha;
    enum neko_gui_anti_aliasing line_AA;
    enum neko_gui_anti_aliasing shape_AA;
    unsigned circle_segment_count;
    unsigned arc_segment_count;
    unsigned curve_segment_count;
    struct neko_gui_draw_null_texture tex_null;
    const struct neko_gui_draw_vertex_layout_element *vertex_layout;
    neko_gui_size vertex_size;
    neko_gui_size vertex_alignment;
};

NEKO_GUI_API const struct neko_gui_command *neko_gui__begin(struct neko_gui_context *);

NEKO_GUI_API const struct neko_gui_command *neko_gui__next(struct neko_gui_context *, const struct neko_gui_command *);

#define neko_gui_foreach(c, ctx) for ((c) = neko_gui__begin(ctx); (c) != 0; (c) = neko_gui__next(ctx, c))
#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT

NEKO_GUI_API neko_gui_flags neko_gui_convert(struct neko_gui_context *, struct neko_gui_buffer *cmds, struct neko_gui_buffer *vertices, struct neko_gui_buffer *elements,
                                             const struct neko_gui_convert_config *);

NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_begin(const struct neko_gui_context *, const struct neko_gui_buffer *);

NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_end(const struct neko_gui_context *, const struct neko_gui_buffer *);

NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_next(const struct neko_gui_draw_command *, const struct neko_gui_buffer *, const struct neko_gui_context *);

#define neko_gui_draw_foreach(cmd, ctx, b) for ((cmd) = neko_gui__draw_begin(ctx, b); (cmd) != 0; (cmd) = neko_gui__draw_next(cmd, b, ctx))
#endif

enum neko_gui_panel_flags {
    NEKO_GUI_WINDOW_BORDER = NEKO_GUI_FLAG(0),
    NEKO_GUI_WINDOW_MOVABLE = NEKO_GUI_FLAG(1),
    NEKO_GUI_WINDOW_SCALABLE = NEKO_GUI_FLAG(2),
    NEKO_GUI_WINDOW_CLOSABLE = NEKO_GUI_FLAG(3),
    NEKO_GUI_WINDOW_MINIMIZABLE = NEKO_GUI_FLAG(4),
    NEKO_GUI_WINDOW_NO_SCROLLBAR = NEKO_GUI_FLAG(5),
    NEKO_GUI_WINDOW_TITLE = NEKO_GUI_FLAG(6),
    NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE = NEKO_GUI_FLAG(7),
    NEKO_GUI_WINDOW_BACKGROUND = NEKO_GUI_FLAG(8),
    NEKO_GUI_WINDOW_SCALE_LEFT = NEKO_GUI_FLAG(9),
    NEKO_GUI_WINDOW_NO_INPUT = NEKO_GUI_FLAG(10)
};

NEKO_GUI_API neko_gui_bool neko_gui_begin(struct neko_gui_context *ctx, const char *title, struct neko_gui_rect bounds, neko_gui_flags flags);

NEKO_GUI_API neko_gui_bool neko_gui_begin_titled(struct neko_gui_context *ctx, const char *name, const char *title, struct neko_gui_rect bounds, neko_gui_flags flags);

NEKO_GUI_API void neko_gui_end(struct neko_gui_context *ctx);

NEKO_GUI_API struct neko_gui_window *neko_gui_window_find(struct neko_gui_context *ctx, const char *name);

NEKO_GUI_API struct neko_gui_rect neko_gui_window_get_bounds(const struct neko_gui_context *ctx);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_position(const struct neko_gui_context *ctx);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_size(const struct neko_gui_context *);

NEKO_GUI_API float neko_gui_window_get_width(const struct neko_gui_context *);

NEKO_GUI_API float neko_gui_window_get_height(const struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_panel *neko_gui_window_get_panel(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_rect neko_gui_window_get_content_region(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_min(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_max(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_size(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_command_buffer *neko_gui_window_get_canvas(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_window_get_scroll(struct neko_gui_context *, neko_gui_uint *offset_x, neko_gui_uint *offset_y);

NEKO_GUI_API neko_gui_bool neko_gui_window_has_focus(const struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_hovered(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_collapsed(struct neko_gui_context *ctx, const char *name);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_closed(struct neko_gui_context *, const char *);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_hidden(struct neko_gui_context *, const char *);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_active(struct neko_gui_context *, const char *);

NEKO_GUI_API neko_gui_bool neko_gui_window_is_any_hovered(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_item_is_any_active(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_window_set_bounds(struct neko_gui_context *, const char *name, struct neko_gui_rect bounds);

NEKO_GUI_API void neko_gui_window_set_position(struct neko_gui_context *, const char *name, struct neko_gui_vec2 pos);

NEKO_GUI_API void neko_gui_window_set_size(struct neko_gui_context *, const char *name, struct neko_gui_vec2);

NEKO_GUI_API void neko_gui_window_set_focus(struct neko_gui_context *, const char *name);

NEKO_GUI_API void neko_gui_window_set_scroll(struct neko_gui_context *, neko_gui_uint offset_x, neko_gui_uint offset_y);

NEKO_GUI_API void neko_gui_window_close(struct neko_gui_context *ctx, const char *name);

NEKO_GUI_API void neko_gui_window_collapse(struct neko_gui_context *, const char *name, enum neko_gui_collapse_states state);

NEKO_GUI_API void neko_gui_window_collapse_if(struct neko_gui_context *, const char *name, enum neko_gui_collapse_states, int cond);

NEKO_GUI_API void neko_gui_window_show(struct neko_gui_context *, const char *name, enum neko_gui_show_states);

NEKO_GUI_API void neko_gui_window_show_if(struct neko_gui_context *, const char *name, enum neko_gui_show_states, int cond);

NEKO_GUI_API void neko_gui_layout_set_min_row_height(struct neko_gui_context *, float height);

NEKO_GUI_API void neko_gui_layout_reset_min_row_height(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_rect neko_gui_layout_widget_bounds(struct neko_gui_context *);

NEKO_GUI_API float neko_gui_layout_ratio_from_pixel(struct neko_gui_context *, float pixel_width);

NEKO_GUI_API void neko_gui_layout_row_dynamic(struct neko_gui_context *ctx, float height, int cols);

NEKO_GUI_API void neko_gui_layout_row_static(struct neko_gui_context *ctx, float height, int item_width, int cols);

NEKO_GUI_API void neko_gui_layout_row_begin(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float row_height, int cols);

NEKO_GUI_API void neko_gui_layout_row_push(struct neko_gui_context *, float value);

NEKO_GUI_API void neko_gui_layout_row_end(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_layout_row(struct neko_gui_context *, enum neko_gui_layout_format, float height, int cols, const float *ratio);

NEKO_GUI_API void neko_gui_layout_row_template_begin(struct neko_gui_context *, float row_height);

NEKO_GUI_API void neko_gui_layout_row_template_push_dynamic(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_layout_row_template_push_variable(struct neko_gui_context *, float min_width);

NEKO_GUI_API void neko_gui_layout_row_template_push_static(struct neko_gui_context *, float width);

NEKO_GUI_API void neko_gui_layout_row_template_end(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_layout_space_begin(struct neko_gui_context *, enum neko_gui_layout_format, float height, int widget_count);

NEKO_GUI_API void neko_gui_layout_space_push(struct neko_gui_context *, struct neko_gui_rect bounds);

NEKO_GUI_API void neko_gui_layout_space_end(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_bounds(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_layout_space_to_screen(struct neko_gui_context *, struct neko_gui_vec2);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_layout_space_to_local(struct neko_gui_context *, struct neko_gui_vec2);

NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_rect_to_screen(struct neko_gui_context *, struct neko_gui_rect);

NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_rect_to_local(struct neko_gui_context *, struct neko_gui_rect);

NEKO_GUI_API void neko_gui_spacer(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_group_begin(struct neko_gui_context *, const char *title, neko_gui_flags);

NEKO_GUI_API neko_gui_bool neko_gui_group_begin_titled(struct neko_gui_context *, const char *name, const char *title, neko_gui_flags);

NEKO_GUI_API void neko_gui_group_end(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_group_scrolled_offset_begin(struct neko_gui_context *, neko_gui_uint *x_offset, neko_gui_uint *y_offset, const char *title, neko_gui_flags flags);

NEKO_GUI_API neko_gui_bool neko_gui_group_scrolled_begin(struct neko_gui_context *, struct neko_gui_scroll *off, const char *title, neko_gui_flags);

NEKO_GUI_API void neko_gui_group_scrolled_end(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_group_get_scroll(struct neko_gui_context *, const char *id, neko_gui_uint *x_offset, neko_gui_uint *y_offset);

NEKO_GUI_API void neko_gui_group_set_scroll(struct neko_gui_context *, const char *id, neko_gui_uint x_offset, neko_gui_uint y_offset);

#define neko_gui_tree_push(ctx, type, title, state) neko_gui_tree_push_hashed(ctx, type, title, state, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), __LINE__)

#define neko_gui_tree_push_id(ctx, type, title, state, id) neko_gui_tree_push_hashed(ctx, type, title, state, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), id)

NEKO_GUI_API neko_gui_bool neko_gui_tree_push_hashed(struct neko_gui_context *, enum neko_gui_tree_type, const char *title, enum neko_gui_collapse_states initial_state, const char *hash, int len,
                                                     int seed);

#define neko_gui_tree_image_push(ctx, type, img, title, state) neko_gui_tree_image_push_hashed(ctx, type, img, title, state, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), __LINE__)

#define neko_gui_tree_image_push_id(ctx, type, img, title, state, id) neko_gui_tree_image_push_hashed(ctx, type, img, title, state, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), id)

NEKO_GUI_API neko_gui_bool neko_gui_tree_image_push_hashed(struct neko_gui_context *, enum neko_gui_tree_type, struct neko_gui_image, const char *title, enum neko_gui_collapse_states initial_state,
                                                           const char *hash, int len, int seed);

NEKO_GUI_API void neko_gui_tree_pop(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_tree_state_push(struct neko_gui_context *, enum neko_gui_tree_type, const char *title, enum neko_gui_collapse_states *state);

NEKO_GUI_API neko_gui_bool neko_gui_tree_state_image_push(struct neko_gui_context *, enum neko_gui_tree_type, struct neko_gui_image, const char *title, enum neko_gui_collapse_states *state);

NEKO_GUI_API void neko_gui_tree_state_pop(struct neko_gui_context *);

#define neko_gui_tree_element_push(ctx, type, title, state, sel) neko_gui_tree_element_push_hashed(ctx, type, title, state, sel, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), __LINE__)
#define neko_gui_tree_element_push_id(ctx, type, title, state, sel, id) neko_gui_tree_element_push_hashed(ctx, type, title, state, sel, NEKO_GUI_FILE_LINE, neko_gui_strlen(NEKO_GUI_FILE_LINE), id)
NEKO_GUI_API neko_gui_bool neko_gui_tree_element_push_hashed(struct neko_gui_context *, enum neko_gui_tree_type, const char *title, enum neko_gui_collapse_states initial_state,
                                                             neko_gui_bool *selected, const char *hash, int len, int seed);
NEKO_GUI_API neko_gui_bool neko_gui_tree_element_image_push_hashed(struct neko_gui_context *, enum neko_gui_tree_type, struct neko_gui_image, const char *title,
                                                                   enum neko_gui_collapse_states initial_state, neko_gui_bool *selected, const char *hash, int len, int seed);
NEKO_GUI_API void neko_gui_tree_element_pop(struct neko_gui_context *);

struct neko_gui_list_view {

    int begin, end, count;

    int total_height;
    struct neko_gui_context *ctx;
    neko_gui_uint *scroll_pointer;
    neko_gui_uint scroll_value;
};
NEKO_GUI_API neko_gui_bool neko_gui_list_view_begin(struct neko_gui_context *, struct neko_gui_list_view *out, const char *id, neko_gui_flags, int row_height, int row_count);
NEKO_GUI_API void neko_gui_list_view_end(struct neko_gui_list_view *);

enum neko_gui_widget_layout_states { NEKO_GUI_WIDGET_INVALID, NEKO_GUI_WIDGET_VALID, NEKO_GUI_WIDGET_ROM };
enum neko_gui_widget_states {
    NEKO_GUI_WIDGET_STATE_MODIFIED = NEKO_GUI_FLAG(1),
    NEKO_GUI_WIDGET_STATE_INACTIVE = NEKO_GUI_FLAG(2),
    NEKO_GUI_WIDGET_STATE_ENTERED = NEKO_GUI_FLAG(3),
    NEKO_GUI_WIDGET_STATE_HOVER = NEKO_GUI_FLAG(4),
    NEKO_GUI_WIDGET_STATE_ACTIVED = NEKO_GUI_FLAG(5),
    NEKO_GUI_WIDGET_STATE_LEFT = NEKO_GUI_FLAG(6),
    NEKO_GUI_WIDGET_STATE_HOVERED = NEKO_GUI_WIDGET_STATE_HOVER | NEKO_GUI_WIDGET_STATE_MODIFIED,
    NEKO_GUI_WIDGET_STATE_ACTIVE = NEKO_GUI_WIDGET_STATE_ACTIVED | NEKO_GUI_WIDGET_STATE_MODIFIED
};
NEKO_GUI_API enum neko_gui_widget_layout_states neko_gui_widget(struct neko_gui_rect *, const struct neko_gui_context *);
NEKO_GUI_API enum neko_gui_widget_layout_states neko_gui_widget_fitting(struct neko_gui_rect *, struct neko_gui_context *, struct neko_gui_vec2);
NEKO_GUI_API struct neko_gui_rect neko_gui_widget_bounds(struct neko_gui_context *);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_widget_position(struct neko_gui_context *);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_widget_size(struct neko_gui_context *);
NEKO_GUI_API float neko_gui_widget_width(struct neko_gui_context *);
NEKO_GUI_API float neko_gui_widget_height(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_widget_is_hovered(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_widget_is_mouse_clicked(struct neko_gui_context *, enum neko_gui_buttons);
NEKO_GUI_API neko_gui_bool neko_gui_widget_has_mouse_click_down(struct neko_gui_context *, enum neko_gui_buttons, neko_gui_bool down);
NEKO_GUI_API void neko_gui_spacing(struct neko_gui_context *, int cols);

enum neko_gui_text_align {
    NEKO_GUI_TEXT_ALIGN_LEFT = 0x01,
    NEKO_GUI_TEXT_ALIGN_CENTERED = 0x02,
    NEKO_GUI_TEXT_ALIGN_RIGHT = 0x04,
    NEKO_GUI_TEXT_ALIGN_TOP = 0x08,
    NEKO_GUI_TEXT_ALIGN_MIDDLE = 0x10,
    NEKO_GUI_TEXT_ALIGN_BOTTOM = 0x20
};
enum neko_gui_text_alignment {
    NEKO_GUI_TEXT_LEFT = NEKO_GUI_TEXT_ALIGN_MIDDLE | NEKO_GUI_TEXT_ALIGN_LEFT,
    NEKO_GUI_TEXT_CENTERED = NEKO_GUI_TEXT_ALIGN_MIDDLE | NEKO_GUI_TEXT_ALIGN_CENTERED,
    NEKO_GUI_TEXT_RIGHT = NEKO_GUI_TEXT_ALIGN_MIDDLE | NEKO_GUI_TEXT_ALIGN_RIGHT
};
NEKO_GUI_API void neko_gui_text(struct neko_gui_context *, const char *, int, neko_gui_flags);
NEKO_GUI_API void neko_gui_text_colored(struct neko_gui_context *, const char *, int, neko_gui_flags, struct neko_gui_color);
NEKO_GUI_API void neko_gui_text_wrap(struct neko_gui_context *, const char *, int);
NEKO_GUI_API void neko_gui_text_wrap_colored(struct neko_gui_context *, const char *, int, struct neko_gui_color);
NEKO_GUI_API void neko_gui_label(struct neko_gui_context *, const char *, neko_gui_flags align);
NEKO_GUI_API void neko_gui_label_colored(struct neko_gui_context *, const char *, neko_gui_flags align, struct neko_gui_color);
NEKO_GUI_API void neko_gui_label_wrap(struct neko_gui_context *, const char *);
NEKO_GUI_API void neko_gui_label_colored_wrap(struct neko_gui_context *, const char *, struct neko_gui_color);
NEKO_GUI_API void neko_gui_image(struct neko_gui_context *, struct neko_gui_image);
NEKO_GUI_API void neko_gui_image_color(struct neko_gui_context *, struct neko_gui_image, struct neko_gui_color);
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
NEKO_GUI_API void neko_gui_labelf(struct neko_gui_context *, neko_gui_flags, NEKO_GUI_PRINTF_FORMAT_STRING const char *, ...) NEKO_GUI_PRINTF_VARARG_FUNC(3);
NEKO_GUI_API void neko_gui_labelf_colored(struct neko_gui_context *, neko_gui_flags, struct neko_gui_color, NEKO_GUI_PRINTF_FORMAT_STRING const char *, ...) NEKO_GUI_PRINTF_VARARG_FUNC(4);
NEKO_GUI_API void neko_gui_labelf_wrap(struct neko_gui_context *, NEKO_GUI_PRINTF_FORMAT_STRING const char *, ...) NEKO_GUI_PRINTF_VARARG_FUNC(2);
NEKO_GUI_API void neko_gui_labelf_colored_wrap(struct neko_gui_context *, struct neko_gui_color, NEKO_GUI_PRINTF_FORMAT_STRING const char *, ...) NEKO_GUI_PRINTF_VARARG_FUNC(3);
NEKO_GUI_API void neko_gui_labelfv(struct neko_gui_context *, neko_gui_flags, NEKO_GUI_PRINTF_FORMAT_STRING const char *, va_list) NEKO_GUI_PRINTF_VALIST_FUNC(3);
NEKO_GUI_API void neko_gui_labelfv_colored(struct neko_gui_context *, neko_gui_flags, struct neko_gui_color, NEKO_GUI_PRINTF_FORMAT_STRING const char *, va_list) NEKO_GUI_PRINTF_VALIST_FUNC(4);
NEKO_GUI_API void neko_gui_labelfv_wrap(struct neko_gui_context *, NEKO_GUI_PRINTF_FORMAT_STRING const char *, va_list) NEKO_GUI_PRINTF_VALIST_FUNC(2);
NEKO_GUI_API void neko_gui_labelfv_colored_wrap(struct neko_gui_context *, struct neko_gui_color, NEKO_GUI_PRINTF_FORMAT_STRING const char *, va_list) NEKO_GUI_PRINTF_VALIST_FUNC(3);
NEKO_GUI_API void neko_gui_value_bool(struct neko_gui_context *, const char *prefix, int);
NEKO_GUI_API void neko_gui_value_int(struct neko_gui_context *, const char *prefix, int);
NEKO_GUI_API void neko_gui_value_uint(struct neko_gui_context *, const char *prefix, unsigned int);
NEKO_GUI_API void neko_gui_value_float(struct neko_gui_context *, const char *prefix, float);
NEKO_GUI_API void neko_gui_value_color_byte(struct neko_gui_context *, const char *prefix, struct neko_gui_color);
NEKO_GUI_API void neko_gui_value_color_float(struct neko_gui_context *, const char *prefix, struct neko_gui_color);
NEKO_GUI_API void neko_gui_value_color_hex(struct neko_gui_context *, const char *prefix, struct neko_gui_color);
#endif

NEKO_GUI_API neko_gui_bool neko_gui_button_text(struct neko_gui_context *, const char *title, int len);
NEKO_GUI_API neko_gui_bool neko_gui_button_label(struct neko_gui_context *, const char *title);
NEKO_GUI_API neko_gui_bool neko_gui_button_color(struct neko_gui_context *, struct neko_gui_color);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol(struct neko_gui_context *, enum neko_gui_symbol_type);
NEKO_GUI_API neko_gui_bool neko_gui_button_image(struct neko_gui_context *, struct neko_gui_image img);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags text_alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_image_label(struct neko_gui_context *, struct neko_gui_image img, const char *, neko_gui_flags text_alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_image_text(struct neko_gui_context *, struct neko_gui_image img, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_text_styled(struct neko_gui_context *, const struct neko_gui_style_button *, const char *title, int len);
NEKO_GUI_API neko_gui_bool neko_gui_button_label_styled(struct neko_gui_context *, const struct neko_gui_style_button *, const char *title);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_styled(struct neko_gui_context *, const struct neko_gui_style_button *, enum neko_gui_symbol_type);
NEKO_GUI_API neko_gui_bool neko_gui_button_image_styled(struct neko_gui_context *, const struct neko_gui_style_button *, struct neko_gui_image img);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_text_styled(struct neko_gui_context *, const struct neko_gui_style_button *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_label_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, enum neko_gui_symbol_type symbol, const char *title,
                                                               neko_gui_flags align);
NEKO_GUI_API neko_gui_bool neko_gui_button_image_label_styled(struct neko_gui_context *, const struct neko_gui_style_button *, struct neko_gui_image img, const char *, neko_gui_flags text_alignment);
NEKO_GUI_API neko_gui_bool neko_gui_button_image_text_styled(struct neko_gui_context *, const struct neko_gui_style_button *, struct neko_gui_image img, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API void neko_gui_button_set_behavior(struct neko_gui_context *, enum neko_gui_button_behavior);
NEKO_GUI_API neko_gui_bool neko_gui_button_push_behavior(struct neko_gui_context *, enum neko_gui_button_behavior);
NEKO_GUI_API neko_gui_bool neko_gui_button_pop_behavior(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_check_label(struct neko_gui_context *, const char *, neko_gui_bool active);
NEKO_GUI_API neko_gui_bool neko_gui_check_text(struct neko_gui_context *, const char *, int, neko_gui_bool active);
NEKO_GUI_API unsigned neko_gui_check_flags_label(struct neko_gui_context *, const char *, unsigned int flags, unsigned int value);
NEKO_GUI_API unsigned neko_gui_check_flags_text(struct neko_gui_context *, const char *, int, unsigned int flags, unsigned int value);
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_label(struct neko_gui_context *, const char *, neko_gui_bool *active);
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_text(struct neko_gui_context *, const char *, int, neko_gui_bool *active);
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_flags_label(struct neko_gui_context *, const char *, unsigned int *flags, unsigned int value);
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_flags_text(struct neko_gui_context *, const char *, int, unsigned int *flags, unsigned int value);

NEKO_GUI_API neko_gui_bool neko_gui_radio_label(struct neko_gui_context *, const char *, neko_gui_bool *active);
NEKO_GUI_API neko_gui_bool neko_gui_radio_text(struct neko_gui_context *, const char *, int, neko_gui_bool *active);
NEKO_GUI_API neko_gui_bool neko_gui_option_label(struct neko_gui_context *, const char *, neko_gui_bool active);
NEKO_GUI_API neko_gui_bool neko_gui_option_text(struct neko_gui_context *, const char *, int, neko_gui_bool active);

NEKO_GUI_API neko_gui_bool neko_gui_selectable_label(struct neko_gui_context *, const char *, neko_gui_flags align, neko_gui_bool *value);
NEKO_GUI_API neko_gui_bool neko_gui_selectable_text(struct neko_gui_context *, const char *, int, neko_gui_flags align, neko_gui_bool *value);
NEKO_GUI_API neko_gui_bool neko_gui_selectable_image_label(struct neko_gui_context *, struct neko_gui_image, const char *, neko_gui_flags align, neko_gui_bool *value);
NEKO_GUI_API neko_gui_bool neko_gui_selectable_image_text(struct neko_gui_context *, struct neko_gui_image, const char *, int, neko_gui_flags align, neko_gui_bool *value);
NEKO_GUI_API neko_gui_bool neko_gui_selectable_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags align, neko_gui_bool *value);
NEKO_GUI_API neko_gui_bool neko_gui_selectable_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags align, neko_gui_bool *value);

NEKO_GUI_API neko_gui_bool neko_gui_select_label(struct neko_gui_context *, const char *, neko_gui_flags align, neko_gui_bool value);
NEKO_GUI_API neko_gui_bool neko_gui_select_text(struct neko_gui_context *, const char *, int, neko_gui_flags align, neko_gui_bool value);
NEKO_GUI_API neko_gui_bool neko_gui_select_image_label(struct neko_gui_context *, struct neko_gui_image, const char *, neko_gui_flags align, neko_gui_bool value);
NEKO_GUI_API neko_gui_bool neko_gui_select_image_text(struct neko_gui_context *, struct neko_gui_image, const char *, int, neko_gui_flags align, neko_gui_bool value);
NEKO_GUI_API neko_gui_bool neko_gui_select_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags align, neko_gui_bool value);
NEKO_GUI_API neko_gui_bool neko_gui_select_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags align, neko_gui_bool value);

NEKO_GUI_API float neko_gui_slide_float(struct neko_gui_context *, float min, float val, float max, float step);
NEKO_GUI_API int neko_gui_slide_int(struct neko_gui_context *, int min, int val, int max, int step);
NEKO_GUI_API neko_gui_bool neko_gui_slider_float(struct neko_gui_context *, float min, float *val, float max, float step);
NEKO_GUI_API neko_gui_bool neko_gui_slider_int(struct neko_gui_context *, int min, int *val, int max, int step);

NEKO_GUI_API neko_gui_bool neko_gui_progress(struct neko_gui_context *, neko_gui_size *cur, neko_gui_size max, neko_gui_bool modifyable);
NEKO_GUI_API neko_gui_size neko_gui_prog(struct neko_gui_context *, neko_gui_size cur, neko_gui_size max, neko_gui_bool modifyable);

NEKO_GUI_API struct neko_gui_colorf neko_gui_color_picker(struct neko_gui_context *, struct neko_gui_colorf, enum neko_gui_color_format);
NEKO_GUI_API neko_gui_bool neko_gui_color_pick(struct neko_gui_context *, struct neko_gui_colorf *, enum neko_gui_color_format);

NEKO_GUI_API void neko_gui_property_int(struct neko_gui_context *, const char *name, int min, int *val, int max, int step, float inc_per_pixel);

NEKO_GUI_API void neko_gui_property_float(struct neko_gui_context *, const char *name, float min, float *val, float max, float step, float inc_per_pixel);

NEKO_GUI_API void neko_gui_property_double(struct neko_gui_context *, const char *name, double min, double *val, double max, double step, float inc_per_pixel);

NEKO_GUI_API int neko_gui_propertyi(struct neko_gui_context *, const char *name, int min, int val, int max, int step, float inc_per_pixel);

NEKO_GUI_API float neko_gui_propertyf(struct neko_gui_context *, const char *name, float min, float val, float max, float step, float inc_per_pixel);

NEKO_GUI_API double neko_gui_propertyd(struct neko_gui_context *, const char *name, double min, double val, double max, double step, float inc_per_pixel);

enum neko_gui_edit_flags {
    NEKO_GUI_EDIT_DEFAULT = 0,
    NEKO_GUI_EDIT_READ_ONLY = NEKO_GUI_FLAG(0),
    NEKO_GUI_EDIT_AUTO_SELECT = NEKO_GUI_FLAG(1),
    NEKO_GUI_EDIT_SIG_ENTER = NEKO_GUI_FLAG(2),
    NEKO_GUI_EDIT_ALLOW_TAB = NEKO_GUI_FLAG(3),
    NEKO_GUI_EDIT_NO_CURSOR = NEKO_GUI_FLAG(4),
    NEKO_GUI_EDIT_SELECTABLE = NEKO_GUI_FLAG(5),
    NEKO_GUI_EDIT_CLIPBOARD = NEKO_GUI_FLAG(6),
    NEKO_GUI_EDIT_CTRL_ENTER_NEWLINE = NEKO_GUI_FLAG(7),
    NEKO_GUI_EDIT_NO_HORIZONTAL_SCROLL = NEKO_GUI_FLAG(8),
    NEKO_GUI_EDIT_ALWAYS_INSERT_MODE = NEKO_GUI_FLAG(9),
    NEKO_GUI_EDIT_MULTILINE = NEKO_GUI_FLAG(10),
    NEKO_GUI_EDIT_GOTO_END_ON_ACTIVATE = NEKO_GUI_FLAG(11)
};
enum neko_gui_edit_types {
    NEKO_GUI_EDIT_SIMPLE = NEKO_GUI_EDIT_ALWAYS_INSERT_MODE,
    NEKO_GUI_EDIT_FIELD = NEKO_GUI_EDIT_SIMPLE | NEKO_GUI_EDIT_SELECTABLE | NEKO_GUI_EDIT_CLIPBOARD,
    NEKO_GUI_EDIT_BOX = NEKO_GUI_EDIT_ALWAYS_INSERT_MODE | NEKO_GUI_EDIT_SELECTABLE | NEKO_GUI_EDIT_MULTILINE | NEKO_GUI_EDIT_ALLOW_TAB | NEKO_GUI_EDIT_CLIPBOARD,
    NEKO_GUI_EDIT_EDITOR = NEKO_GUI_EDIT_SELECTABLE | NEKO_GUI_EDIT_MULTILINE | NEKO_GUI_EDIT_ALLOW_TAB | NEKO_GUI_EDIT_CLIPBOARD
};
enum neko_gui_edit_events {
    NEKO_GUI_EDIT_ACTIVE = NEKO_GUI_FLAG(0),
    NEKO_GUI_EDIT_INACTIVE = NEKO_GUI_FLAG(1),
    NEKO_GUI_EDIT_ACTIVATED = NEKO_GUI_FLAG(2),
    NEKO_GUI_EDIT_DEACTIVATED = NEKO_GUI_FLAG(3),
    NEKO_GUI_EDIT_COMMITED = NEKO_GUI_FLAG(4)
};
NEKO_GUI_API neko_gui_flags neko_gui_edit_string(struct neko_gui_context *, neko_gui_flags, char *buffer, int *len, int max, neko_gui_plugin_filter);
NEKO_GUI_API neko_gui_flags neko_gui_edit_string_zero_terminated(struct neko_gui_context *, neko_gui_flags, char *buffer, int max, neko_gui_plugin_filter);
NEKO_GUI_API neko_gui_flags neko_gui_edit_buffer(struct neko_gui_context *, neko_gui_flags, struct neko_gui_text_edit *, neko_gui_plugin_filter);
NEKO_GUI_API void neko_gui_edit_focus(struct neko_gui_context *, neko_gui_flags flags);
NEKO_GUI_API void neko_gui_edit_unfocus(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_chart_begin(struct neko_gui_context *, enum neko_gui_chart_type, int num, float min, float max);
NEKO_GUI_API neko_gui_bool neko_gui_chart_begin_colored(struct neko_gui_context *, enum neko_gui_chart_type, struct neko_gui_color, struct neko_gui_color active, int num, float min, float max);
NEKO_GUI_API void neko_gui_chart_add_slot(struct neko_gui_context *ctx, const enum neko_gui_chart_type, int count, float min_value, float max_value);
NEKO_GUI_API void neko_gui_chart_add_slot_colored(struct neko_gui_context *ctx, const enum neko_gui_chart_type, struct neko_gui_color, struct neko_gui_color active, int count, float min_value,
                                                  float max_value);
NEKO_GUI_API neko_gui_flags neko_gui_chart_push(struct neko_gui_context *, float);
NEKO_GUI_API neko_gui_flags neko_gui_chart_push_slot(struct neko_gui_context *, float, int);
NEKO_GUI_API void neko_gui_chart_end(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_plot(struct neko_gui_context *, enum neko_gui_chart_type, const float *values, int count, int offset);
NEKO_GUI_API void neko_gui_plot_function(struct neko_gui_context *, enum neko_gui_chart_type, void *userdata, float (*value_getter)(void *user, int index), int count, int offset);

NEKO_GUI_API neko_gui_bool neko_gui_popup_begin(struct neko_gui_context *, enum neko_gui_popup_type, const char *, neko_gui_flags, struct neko_gui_rect bounds);
NEKO_GUI_API void neko_gui_popup_close(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_popup_end(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_popup_get_scroll(struct neko_gui_context *, neko_gui_uint *offset_x, neko_gui_uint *offset_y);
NEKO_GUI_API void neko_gui_popup_set_scroll(struct neko_gui_context *, neko_gui_uint offset_x, neko_gui_uint offset_y);

NEKO_GUI_API int neko_gui_combo(struct neko_gui_context *, const char **items, int count, int selected, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API int neko_gui_combo_separator(struct neko_gui_context *, const char *items_separated_by_separator, int separator, int selected, int count, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API int neko_gui_combo_string(struct neko_gui_context *, const char *items_separated_by_zeros, int selected, int count, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API int neko_gui_combo_callback(struct neko_gui_context *, void (*item_getter)(void *, int, const char **), void *userdata, int selected, int count, int item_height,
                                         struct neko_gui_vec2 size);
NEKO_GUI_API void neko_gui_combobox(struct neko_gui_context *, const char **items, int count, int *selected, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API void neko_gui_combobox_string(struct neko_gui_context *, const char *items_separated_by_zeros, int *selected, int count, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API void neko_gui_combobox_separator(struct neko_gui_context *, const char *items_separated_by_separator, int separator, int *selected, int count, int item_height, struct neko_gui_vec2 size);
NEKO_GUI_API void neko_gui_combobox_callback(struct neko_gui_context *, void (*item_getter)(void *, int, const char **), void *, int *selected, int count, int item_height, struct neko_gui_vec2 size);

NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_text(struct neko_gui_context *, const char *selected, int, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_label(struct neko_gui_context *, const char *selected, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_color(struct neko_gui_context *, struct neko_gui_color color, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol(struct neko_gui_context *, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol_label(struct neko_gui_context *, const char *selected, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol_text(struct neko_gui_context *, const char *selected, int, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image(struct neko_gui_context *, struct neko_gui_image img, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image_label(struct neko_gui_context *, const char *selected, struct neko_gui_image, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image_text(struct neko_gui_context *, const char *selected, int, struct neko_gui_image, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_label(struct neko_gui_context *, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_text(struct neko_gui_context *, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_image_label(struct neko_gui_context *, struct neko_gui_image, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_image_text(struct neko_gui_context *, struct neko_gui_image, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API void neko_gui_combo_close(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_combo_end(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_contextual_begin(struct neko_gui_context *, neko_gui_flags, struct neko_gui_vec2, struct neko_gui_rect trigger_bounds);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_text(struct neko_gui_context *, const char *, int, neko_gui_flags align);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_label(struct neko_gui_context *, const char *, neko_gui_flags align);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_image_label(struct neko_gui_context *, struct neko_gui_image, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_image_text(struct neko_gui_context *, struct neko_gui_image, const char *, int len, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API void neko_gui_contextual_close(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_contextual_end(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_tooltip(struct neko_gui_context *, const char *);
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
NEKO_GUI_API void neko_gui_tooltipf(struct neko_gui_context *, NEKO_GUI_PRINTF_FORMAT_STRING const char *, ...) NEKO_GUI_PRINTF_VARARG_FUNC(2);
NEKO_GUI_API void neko_gui_tooltipfv(struct neko_gui_context *, NEKO_GUI_PRINTF_FORMAT_STRING const char *, va_list) NEKO_GUI_PRINTF_VALIST_FUNC(2);
#endif
NEKO_GUI_API neko_gui_bool neko_gui_tooltip_begin(struct neko_gui_context *, float width);
NEKO_GUI_API void neko_gui_tooltip_end(struct neko_gui_context *);

NEKO_GUI_API void neko_gui_menubar_begin(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_menubar_end(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_text(struct neko_gui_context *, const char *title, int title_len, neko_gui_flags align, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_label(struct neko_gui_context *, const char *, neko_gui_flags align, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image(struct neko_gui_context *, const char *, struct neko_gui_image, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image_text(struct neko_gui_context *, const char *, int, neko_gui_flags align, struct neko_gui_image, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image_label(struct neko_gui_context *, const char *, neko_gui_flags align, struct neko_gui_image, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol(struct neko_gui_context *, const char *, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol_text(struct neko_gui_context *, const char *, int, neko_gui_flags align, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol_label(struct neko_gui_context *, const char *, neko_gui_flags align, enum neko_gui_symbol_type, struct neko_gui_vec2 size);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_text(struct neko_gui_context *, const char *, int, neko_gui_flags align);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_label(struct neko_gui_context *, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_image_label(struct neko_gui_context *, struct neko_gui_image, const char *, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_image_text(struct neko_gui_context *, struct neko_gui_image, const char *, int len, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_symbol_text(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, int, neko_gui_flags alignment);
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_symbol_label(struct neko_gui_context *, enum neko_gui_symbol_type, const char *, neko_gui_flags alignment);
NEKO_GUI_API void neko_gui_menu_close(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_menu_end(struct neko_gui_context *);

enum neko_gui_style_colors {
    NEKO_GUI_COLOR_TEXT,
    NEKO_GUI_COLOR_WINDOW,
    NEKO_GUI_COLOR_HEADER,
    NEKO_GUI_COLOR_BORDER,
    NEKO_GUI_COLOR_BUTTON,
    NEKO_GUI_COLOR_BUTTON_HOVER,
    NEKO_GUI_COLOR_BUTTON_ACTIVE,
    NEKO_GUI_COLOR_TOGGLE,
    NEKO_GUI_COLOR_TOGGLE_HOVER,
    NEKO_GUI_COLOR_TOGGLE_CURSOR,
    NEKO_GUI_COLOR_SELECT,
    NEKO_GUI_COLOR_SELECT_ACTIVE,
    NEKO_GUI_COLOR_SLIDER,
    NEKO_GUI_COLOR_SLIDER_CURSOR,
    NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER,
    NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE,
    NEKO_GUI_COLOR_PROPERTY,
    NEKO_GUI_COLOR_EDIT,
    NEKO_GUI_COLOR_EDIT_CURSOR,
    NEKO_GUI_COLOR_COMBO,
    NEKO_GUI_COLOR_CHART,
    NEKO_GUI_COLOR_CHART_COLOR,
    NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT,
    NEKO_GUI_COLOR_SCROLLBAR,
    NEKO_GUI_COLOR_SCROLLBAR_CURSOR,
    NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER,
    NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE,
    NEKO_GUI_COLOR_TAB_HEADER,
    NEKO_GUI_COLOR_COUNT
};
enum neko_gui_style_cursor {
    NEKO_GUI_CURSOR_ARROW,
    NEKO_GUI_CURSOR_TEXT,
    NEKO_GUI_CURSOR_MOVE,
    NEKO_GUI_CURSOR_RESIZE_VERTICAL,
    NEKO_GUI_CURSOR_RESIZE_HORIZONTAL,
    NEKO_GUI_CURSOR_RESIZE_TOP_LEFT_DOWN_RIGHT,
    NEKO_GUI_CURSOR_RESIZE_TOP_RIGHT_DOWN_LEFT,
    NEKO_GUI_CURSOR_COUNT
};
NEKO_GUI_API void neko_gui_style_default(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_style_from_table(struct neko_gui_context *, const struct neko_gui_color *);
NEKO_GUI_API void neko_gui_style_load_cursor(struct neko_gui_context *, enum neko_gui_style_cursor, const struct neko_gui_cursor *);
NEKO_GUI_API void neko_gui_style_load_all_cursors(struct neko_gui_context *, struct neko_gui_cursor *);
NEKO_GUI_API const char *neko_gui_style_get_color_by_name(enum neko_gui_style_colors);
NEKO_GUI_API void neko_gui_style_set_font(struct neko_gui_context *, const struct neko_gui_user_font *);
NEKO_GUI_API neko_gui_bool neko_gui_style_set_cursor(struct neko_gui_context *, enum neko_gui_style_cursor);
NEKO_GUI_API void neko_gui_style_show_cursor(struct neko_gui_context *);
NEKO_GUI_API void neko_gui_style_hide_cursor(struct neko_gui_context *);

NEKO_GUI_API neko_gui_bool neko_gui_style_push_font(struct neko_gui_context *, const struct neko_gui_user_font *);
NEKO_GUI_API neko_gui_bool neko_gui_style_push_float(struct neko_gui_context *, float *, float);
NEKO_GUI_API neko_gui_bool neko_gui_style_push_vec2(struct neko_gui_context *, struct neko_gui_vec2 *, struct neko_gui_vec2);
NEKO_GUI_API neko_gui_bool neko_gui_style_push_style_item(struct neko_gui_context *, struct neko_gui_style_item *, struct neko_gui_style_item);
NEKO_GUI_API neko_gui_bool neko_gui_style_push_flags(struct neko_gui_context *, neko_gui_flags *, neko_gui_flags);
NEKO_GUI_API neko_gui_bool neko_gui_style_push_color(struct neko_gui_context *, struct neko_gui_color *, struct neko_gui_color);

NEKO_GUI_API neko_gui_bool neko_gui_style_pop_font(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_float(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_vec2(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_style_item(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_flags(struct neko_gui_context *);
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_color(struct neko_gui_context *);

NEKO_GUI_API struct neko_gui_color neko_gui_rgb(int r, int g, int b);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_iv(const int *rgb);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_bv(const neko_gui_byte *rgb);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_f(float r, float g, float b);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_fv(const float *rgb);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_cf(struct neko_gui_colorf c);
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_hex(const char *rgb);

NEKO_GUI_API struct neko_gui_color neko_gui_rgba(int r, int g, int b, int a);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_u32(neko_gui_uint);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_iv(const int *rgba);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_bv(const neko_gui_byte *rgba);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_f(float r, float g, float b, float a);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_fv(const float *rgba);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_cf(struct neko_gui_colorf c);
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_hex(const char *rgb);

NEKO_GUI_API struct neko_gui_colorf neko_gui_hsva_colorf(float h, float s, float v, float a);
NEKO_GUI_API struct neko_gui_colorf neko_gui_hsva_colorfv(float *c);
NEKO_GUI_API void neko_gui_colorf_hsva_f(float *out_h, float *out_s, float *out_v, float *out_a, struct neko_gui_colorf in);
NEKO_GUI_API void neko_gui_colorf_hsva_fv(float *hsva, struct neko_gui_colorf in);

NEKO_GUI_API struct neko_gui_color neko_gui_hsv(int h, int s, int v);
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_iv(const int *hsv);
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_bv(const neko_gui_byte *hsv);
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_f(float h, float s, float v);
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_fv(const float *hsv);

NEKO_GUI_API struct neko_gui_color neko_gui_hsva(int h, int s, int v, int a);
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_iv(const int *hsva);
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_bv(const neko_gui_byte *hsva);
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_f(float h, float s, float v, float a);
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_fv(const float *hsva);

NEKO_GUI_API void neko_gui_color_f(float *r, float *g, float *b, float *a, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_fv(float *rgba_out, struct neko_gui_color);
NEKO_GUI_API struct neko_gui_colorf neko_gui_color_cf(struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_d(double *r, double *g, double *b, double *a, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_dv(double *rgba_out, struct neko_gui_color);

NEKO_GUI_API neko_gui_uint neko_gui_color_u32(struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hex_rgba(char *output, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hex_rgb(char *output, struct neko_gui_color);

NEKO_GUI_API void neko_gui_color_hsv_i(int *out_h, int *out_s, int *out_v, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsv_b(neko_gui_byte *out_h, neko_gui_byte *out_s, neko_gui_byte *out_v, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsv_iv(int *hsv_out, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsv_bv(neko_gui_byte *hsv_out, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsv_f(float *out_h, float *out_s, float *out_v, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsv_fv(float *hsv_out, struct neko_gui_color);

NEKO_GUI_API void neko_gui_color_hsva_i(int *h, int *s, int *v, int *a, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsva_b(neko_gui_byte *h, neko_gui_byte *s, neko_gui_byte *v, neko_gui_byte *a, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsva_iv(int *hsva_out, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsva_bv(neko_gui_byte *hsva_out, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsva_f(float *out_h, float *out_s, float *out_v, float *out_a, struct neko_gui_color);
NEKO_GUI_API void neko_gui_color_hsva_fv(float *hsva_out, struct neko_gui_color);

NEKO_GUI_API neko_gui_handle neko_gui_handle_ptr(void *);
NEKO_GUI_API neko_gui_handle neko_gui_handle_id(int);
NEKO_GUI_API struct neko_gui_image neko_gui_image_handle(neko_gui_handle);
NEKO_GUI_API struct neko_gui_image neko_gui_image_ptr(void *);
NEKO_GUI_API struct neko_gui_image neko_gui_image_id(int);
NEKO_GUI_API neko_gui_bool neko_gui_image_is_subimage(const struct neko_gui_image *img);
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_ptr(void *, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region);
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_id(int, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region);
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_handle(neko_gui_handle, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region);

NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_handle(neko_gui_handle, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b);
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_ptr(void *, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b);
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_id(int, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b);
NEKO_GUI_API int neko_gui_nine_slice_is_sub9slice(const struct neko_gui_nine_slice *img);
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_ptr(void *, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r,
                                                               neko_gui_ushort b);
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_id(int, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r,
                                                              neko_gui_ushort b);
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_handle(neko_gui_handle, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect sub_region, neko_gui_ushort l, neko_gui_ushort t,
                                                                  neko_gui_ushort r, neko_gui_ushort b);

NEKO_GUI_API neko_gui_hash neko_gui_murmur_hash(const void *key, int len, neko_gui_hash seed);
NEKO_GUI_API void neko_gui_triangle_from_direction(struct neko_gui_vec2 *result, struct neko_gui_rect r, float pad_x, float pad_y, enum neko_gui_heading);

NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2(float x, float y);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2i(int x, int y);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2v(const float *xy);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2iv(const int *xy);

NEKO_GUI_API struct neko_gui_rect neko_gui_get_null_rect(void);
NEKO_GUI_API struct neko_gui_rect neko_gui_rect(float x, float y, float w, float h);
NEKO_GUI_API struct neko_gui_rect neko_gui_recti(int x, int y, int w, int h);
NEKO_GUI_API struct neko_gui_rect neko_gui_recta(struct neko_gui_vec2 pos, struct neko_gui_vec2 size);
NEKO_GUI_API struct neko_gui_rect neko_gui_rectv(const float *xywh);
NEKO_GUI_API struct neko_gui_rect neko_gui_rectiv(const int *xywh);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_rect_pos(struct neko_gui_rect);
NEKO_GUI_API struct neko_gui_vec2 neko_gui_rect_size(struct neko_gui_rect);

NEKO_GUI_API int neko_gui_strlen(const char *str);
NEKO_GUI_API int neko_gui_stricmp(const char *s1, const char *s2);
NEKO_GUI_API int neko_gui_stricmpn(const char *s1, const char *s2, int n);
NEKO_GUI_API int neko_gui_strtoi(const char *str, const char **endptr);
NEKO_GUI_API float neko_gui_strtof(const char *str, const char **endptr);
#ifndef NEKO_GUI_STRTOD
#define NEKO_GUI_STRTOD neko_gui_strtod
NEKO_GUI_API double neko_gui_strtod(const char *str, const char **endptr);
#endif
NEKO_GUI_API int neko_gui_strfilter(const char *text, const char *regexp);
NEKO_GUI_API int neko_gui_strmatch_fuzzy_string(char const *str, char const *pattern, int *out_score);
NEKO_GUI_API int neko_gui_strmatch_fuzzy_text(const char *txt, int txt_len, const char *pattern, int *out_score);

NEKO_GUI_API int neko_gui_utf_decode(const char *, neko_gui_rune *, int);
NEKO_GUI_API int neko_gui_utf_encode(neko_gui_rune, char *, int);
NEKO_GUI_API int neko_gui_utf_len(const char *, int byte_len);
NEKO_GUI_API const char *neko_gui_utf_at(const char *buffer, int length, int index, neko_gui_rune *unicode, int *len);

struct neko_gui_user_font_glyph;
typedef float (*neko_gui_text_width_f)(neko_gui_handle, float h, const char *, int len);
typedef void (*neko_gui_query_font_glyph_f)(neko_gui_handle handle, float font_height, struct neko_gui_user_font_glyph *glyph, neko_gui_rune codepoint, neko_gui_rune next_codepoint);

#if defined(NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT) || defined(NEKO_GUI_INCLUDE_SOFTWARE_FONT)
struct neko_gui_user_font_glyph {
    struct neko_gui_vec2 uv[2];

    struct neko_gui_vec2 offset;

    float width, height;

    float xadvance;
};
#endif

struct neko_gui_user_font {
    neko_gui_handle userdata;

    float height;

    neko_gui_text_width_f width;

#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
    neko_gui_query_font_glyph_f query;

    neko_gui_handle texture;

#endif
};

#ifdef NEKO_GUI_INCLUDE_FONT_BAKING
enum neko_gui_font_coord_type { NEKO_GUI_COORD_UV, NEKO_GUI_COORD_PIXEL };

struct neko_gui_font;
struct neko_gui_baked_font {
    float height;

    float ascent, descent;

    neko_gui_rune glyph_offset;

    neko_gui_rune glyph_count;

    const neko_gui_rune *ranges;
};

struct neko_gui_font_config {
    struct neko_gui_font_config *next;

    void *ttf_blob;

    neko_gui_size ttf_size;

    unsigned char ttf_data_owned_by_atlas;

    unsigned char merge_mode;

    unsigned char pixel_snap;

    unsigned char oversample_v, oversample_h;

    unsigned char padding[3];

    float size;

    enum neko_gui_font_coord_type coord_type;

    struct neko_gui_vec2 spacing;

    const neko_gui_rune *range;

    struct neko_gui_baked_font *font;

    neko_gui_rune fallback_glyph;

    struct neko_gui_font_config *n;
    struct neko_gui_font_config *p;
};

struct neko_gui_font_glyph {
    neko_gui_rune codepoint;
    float xadvance;
    float x0, y0, x1, y1, w, h;
    float u0, v0, u1, v1;
};

struct neko_gui_font {
    struct neko_gui_font *next;
    struct neko_gui_user_font handle;
    struct neko_gui_baked_font info;
    float scale;
    struct neko_gui_font_glyph *glyphs;
    const struct neko_gui_font_glyph *fallback;
    neko_gui_rune fallback_codepoint;
    neko_gui_handle texture;
    struct neko_gui_font_config *config;
};

enum neko_gui_font_atlas_format { NEKO_GUI_FONT_ATLAS_ALPHA8, NEKO_GUI_FONT_ATLAS_RGBA32 };

struct neko_gui_font_atlas {
    void *pixel;
    int tex_width;
    int tex_height;

    struct neko_gui_allocator permanent;
    struct neko_gui_allocator temporary;

    struct neko_gui_recti custom;
    struct neko_gui_cursor cursors[NEKO_GUI_CURSOR_COUNT];

    int glyph_count;
    struct neko_gui_font_glyph *glyphs;
    struct neko_gui_font *default_font;
    struct neko_gui_font *fonts;
    struct neko_gui_font_config *config;
    int font_num;
};

NEKO_GUI_API const neko_gui_rune *neko_gui_font_default_glyph_ranges(void);
NEKO_GUI_API const neko_gui_rune *neko_gui_font_chinese_glyph_ranges(void);
NEKO_GUI_API const neko_gui_rune *neko_gui_font_cyrillic_glyph_ranges(void);
NEKO_GUI_API const neko_gui_rune *neko_gui_font_korean_glyph_ranges(void);

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_font_atlas_init_default(struct neko_gui_font_atlas *);
#endif
NEKO_GUI_API void neko_gui_font_atlas_init(struct neko_gui_font_atlas *, struct neko_gui_allocator *);
NEKO_GUI_API void neko_gui_font_atlas_init_custom(struct neko_gui_font_atlas *, struct neko_gui_allocator *persistent, struct neko_gui_allocator *transient);
NEKO_GUI_API void neko_gui_font_atlas_begin(struct neko_gui_font_atlas *);
NEKO_GUI_API struct neko_gui_font_config neko_gui_font_config(float pixel_height);
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add(struct neko_gui_font_atlas *, const struct neko_gui_font_config *);
#ifdef NEKO_GUI_INCLUDE_DEFAULT_FONT
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_default(struct neko_gui_font_atlas *, float height, const struct neko_gui_font_config *);
#endif
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_from_memory(struct neko_gui_font_atlas *atlas, void *memory, neko_gui_size size, float height, const struct neko_gui_font_config *config);
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_from_file(struct neko_gui_font_atlas *atlas, const char *file_path, float height, const struct neko_gui_font_config *);
#endif
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_compressed(struct neko_gui_font_atlas *, void *memory, neko_gui_size size, float height, const struct neko_gui_font_config *);
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_compressed_base85(struct neko_gui_font_atlas *, const char *data, float height, const struct neko_gui_font_config *config);
NEKO_GUI_API const void *neko_gui_font_atlas_bake(struct neko_gui_font_atlas *, int *width, int *height, enum neko_gui_font_atlas_format);
NEKO_GUI_API void neko_gui_font_atlas_end(struct neko_gui_font_atlas *, neko_gui_handle tex, struct neko_gui_draw_null_texture *);
NEKO_GUI_API const struct neko_gui_font_glyph *neko_gui_font_find_glyph(struct neko_gui_font *, neko_gui_rune unicode);
NEKO_GUI_API void neko_gui_font_atlas_cleanup(struct neko_gui_font_atlas *atlas);
NEKO_GUI_API void neko_gui_font_atlas_clear(struct neko_gui_font_atlas *);

#endif

struct neko_gui_memory_status {
    void *memory;
    unsigned int type;
    neko_gui_size size;
    neko_gui_size allocated;
    neko_gui_size needed;
    neko_gui_size calls;
};

enum neko_gui_allocation_type { NEKO_GUI_BUFFER_FIXED, NEKO_GUI_BUFFER_DYNAMIC };

enum neko_gui_buffer_allocation_type { NEKO_GUI_BUFFER_FRONT, NEKO_GUI_BUFFER_BACK, NEKO_GUI_BUFFER_MAX };

struct neko_gui_buffer_marker {
    neko_gui_bool active;
    neko_gui_size offset;
};

struct neko_gui_memory {
    void *ptr;
    neko_gui_size size;
};
struct neko_gui_buffer {
    struct neko_gui_buffer_marker marker[NEKO_GUI_BUFFER_MAX];

    struct neko_gui_allocator pool;

    enum neko_gui_allocation_type type;

    struct neko_gui_memory memory;

    float grow_factor;

    neko_gui_size allocated;

    neko_gui_size needed;

    neko_gui_size calls;

    neko_gui_size size;
};

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_buffer_init_default(struct neko_gui_buffer *);
#endif
NEKO_GUI_API void neko_gui_buffer_init(struct neko_gui_buffer *, const struct neko_gui_allocator *, neko_gui_size size);
NEKO_GUI_API void neko_gui_buffer_init_fixed(struct neko_gui_buffer *, void *memory, neko_gui_size size);
NEKO_GUI_API void neko_gui_buffer_info(struct neko_gui_memory_status *, struct neko_gui_buffer *);
NEKO_GUI_API void neko_gui_buffer_push(struct neko_gui_buffer *, enum neko_gui_buffer_allocation_type type, const void *memory, neko_gui_size size, neko_gui_size align);
NEKO_GUI_API void neko_gui_buffer_mark(struct neko_gui_buffer *, enum neko_gui_buffer_allocation_type type);
NEKO_GUI_API void neko_gui_buffer_reset(struct neko_gui_buffer *, enum neko_gui_buffer_allocation_type type);
NEKO_GUI_API void neko_gui_buffer_clear(struct neko_gui_buffer *);
NEKO_GUI_API void neko_gui_buffer_free(struct neko_gui_buffer *);
NEKO_GUI_API void *neko_gui_buffer_memory(struct neko_gui_buffer *);
NEKO_GUI_API const void *neko_gui_buffer_memory_const(const struct neko_gui_buffer *);
NEKO_GUI_API neko_gui_size neko_gui_buffer_total(struct neko_gui_buffer *);

struct neko_gui_str {
    struct neko_gui_buffer buffer;
    int len;
};

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_str_init_default(struct neko_gui_str *);
#endif
NEKO_GUI_API void neko_gui_str_init(struct neko_gui_str *, const struct neko_gui_allocator *, neko_gui_size size);
NEKO_GUI_API void neko_gui_str_init_fixed(struct neko_gui_str *, void *memory, neko_gui_size size);
NEKO_GUI_API void neko_gui_str_clear(struct neko_gui_str *);
NEKO_GUI_API void neko_gui_str_free(struct neko_gui_str *);

NEKO_GUI_API int neko_gui_str_append_text_char(struct neko_gui_str *, const char *, int);
NEKO_GUI_API int neko_gui_str_append_str_char(struct neko_gui_str *, const char *);
NEKO_GUI_API int neko_gui_str_append_text_utf8(struct neko_gui_str *, const char *, int);
NEKO_GUI_API int neko_gui_str_append_str_utf8(struct neko_gui_str *, const char *);
NEKO_GUI_API int neko_gui_str_append_text_runes(struct neko_gui_str *, const neko_gui_rune *, int);
NEKO_GUI_API int neko_gui_str_append_str_runes(struct neko_gui_str *, const neko_gui_rune *);

NEKO_GUI_API int neko_gui_str_insert_at_char(struct neko_gui_str *, int pos, const char *, int);
NEKO_GUI_API int neko_gui_str_insert_at_rune(struct neko_gui_str *, int pos, const char *, int);

NEKO_GUI_API int neko_gui_str_insert_text_char(struct neko_gui_str *, int pos, const char *, int);
NEKO_GUI_API int neko_gui_str_insert_str_char(struct neko_gui_str *, int pos, const char *);
NEKO_GUI_API int neko_gui_str_insert_text_utf8(struct neko_gui_str *, int pos, const char *, int);
NEKO_GUI_API int neko_gui_str_insert_str_utf8(struct neko_gui_str *, int pos, const char *);
NEKO_GUI_API int neko_gui_str_insert_text_runes(struct neko_gui_str *, int pos, const neko_gui_rune *, int);
NEKO_GUI_API int neko_gui_str_insert_str_runes(struct neko_gui_str *, int pos, const neko_gui_rune *);

NEKO_GUI_API void neko_gui_str_remove_chars(struct neko_gui_str *, int len);
NEKO_GUI_API void neko_gui_str_remove_runes(struct neko_gui_str *str, int len);
NEKO_GUI_API void neko_gui_str_delete_chars(struct neko_gui_str *, int pos, int len);
NEKO_GUI_API void neko_gui_str_delete_runes(struct neko_gui_str *, int pos, int len);

NEKO_GUI_API char *neko_gui_str_at_char(struct neko_gui_str *, int pos);
NEKO_GUI_API char *neko_gui_str_at_rune(struct neko_gui_str *, int pos, neko_gui_rune *unicode, int *len);
NEKO_GUI_API neko_gui_rune neko_gui_str_rune_at(const struct neko_gui_str *, int pos);
NEKO_GUI_API const char *neko_gui_str_at_char_const(const struct neko_gui_str *, int pos);
NEKO_GUI_API const char *neko_gui_str_at_const(const struct neko_gui_str *, int pos, neko_gui_rune *unicode, int *len);

NEKO_GUI_API char *neko_gui_str_get(struct neko_gui_str *);
NEKO_GUI_API const char *neko_gui_str_get_const(const struct neko_gui_str *);
NEKO_GUI_API int neko_gui_str_len(struct neko_gui_str *);
NEKO_GUI_API int neko_gui_str_len_char(struct neko_gui_str *);

#ifndef NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT
#define NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT 99
#endif

#ifndef NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT
#define NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT 999
#endif

struct neko_gui_text_edit;
struct neko_gui_clipboard {
    neko_gui_handle userdata;
    neko_gui_plugin_paste paste;
    neko_gui_plugin_copy copy;
};

struct neko_gui_text_undo_record {
    int where;
    short insert_length;
    short delete_length;
    short char_storage;
};

struct neko_gui_text_undo_state {
    struct neko_gui_text_undo_record undo_rec[NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT];
    neko_gui_rune undo_char[NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT];
    short undo_point;
    short redo_point;
    short undo_char_point;
    short redo_char_point;
};

enum neko_gui_text_edit_type { NEKO_GUI_TEXT_EDIT_SINGLE_LINE, NEKO_GUI_TEXT_EDIT_MULTI_LINE };

enum neko_gui_text_edit_mode { NEKO_GUI_TEXT_EDIT_MODE_VIEW, NEKO_GUI_TEXT_EDIT_MODE_INSERT, NEKO_GUI_TEXT_EDIT_MODE_REPLACE };

struct neko_gui_text_edit {
    struct neko_gui_clipboard clip;
    struct neko_gui_str string;
    neko_gui_plugin_filter filter;
    struct neko_gui_vec2 scrollbar;

    int cursor;
    int select_start;
    int select_end;
    unsigned char mode;
    unsigned char cursor_at_end_of_line;
    unsigned char initialized;
    unsigned char has_preferred_x;
    unsigned char single_line;
    unsigned char active;
    unsigned char padding1;
    float preferred_x;
    struct neko_gui_text_undo_state undo;
};

NEKO_GUI_API neko_gui_bool neko_gui_filter_default(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_ascii(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_float(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_decimal(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_hex(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_oct(const struct neko_gui_text_edit *, neko_gui_rune unicode);
NEKO_GUI_API neko_gui_bool neko_gui_filter_binary(const struct neko_gui_text_edit *, neko_gui_rune unicode);

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_textedit_init_default(struct neko_gui_text_edit *);
#endif
NEKO_GUI_API void neko_gui_textedit_init(struct neko_gui_text_edit *, struct neko_gui_allocator *, neko_gui_size size);
NEKO_GUI_API void neko_gui_textedit_init_fixed(struct neko_gui_text_edit *, void *memory, neko_gui_size size);
NEKO_GUI_API void neko_gui_textedit_free(struct neko_gui_text_edit *);
NEKO_GUI_API void neko_gui_textedit_text(struct neko_gui_text_edit *, const char *, int total_len);
NEKO_GUI_API void neko_gui_textedit_delete(struct neko_gui_text_edit *, int where, int len);
NEKO_GUI_API void neko_gui_textedit_delete_selection(struct neko_gui_text_edit *);
NEKO_GUI_API void neko_gui_textedit_select_all(struct neko_gui_text_edit *);
NEKO_GUI_API neko_gui_bool neko_gui_textedit_cut(struct neko_gui_text_edit *);
NEKO_GUI_API neko_gui_bool neko_gui_textedit_paste(struct neko_gui_text_edit *, char const *, int len);
NEKO_GUI_API void neko_gui_textedit_undo(struct neko_gui_text_edit *);
NEKO_GUI_API void neko_gui_textedit_redo(struct neko_gui_text_edit *);

enum neko_gui_command_type {
    NEKO_GUI_COMMAND_NOP,
    NEKO_GUI_COMMAND_SCISSOR,
    NEKO_GUI_COMMAND_LINE,
    NEKO_GUI_COMMAND_CURVE,
    NEKO_GUI_COMMAND_RECT,
    NEKO_GUI_COMMAND_RECT_FILLED,
    NEKO_GUI_COMMAND_RECT_MULTI_COLOR,
    NEKO_GUI_COMMAND_CIRCLE,
    NEKO_GUI_COMMAND_CIRCLE_FILLED,
    NEKO_GUI_COMMAND_ARC,
    NEKO_GUI_COMMAND_ARC_FILLED,
    NEKO_GUI_COMMAND_TRIANGLE,
    NEKO_GUI_COMMAND_TRIANGLE_FILLED,
    NEKO_GUI_COMMAND_POLYGON,
    NEKO_GUI_COMMAND_POLYGON_FILLED,
    NEKO_GUI_COMMAND_POLYLINE,
    NEKO_GUI_COMMAND_TEXT,
    NEKO_GUI_COMMAND_IMAGE,
    NEKO_GUI_COMMAND_CUSTOM
};

struct neko_gui_command {
    enum neko_gui_command_type type;
    neko_gui_size next;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_handle userdata;
#endif
};

struct neko_gui_command_scissor {
    struct neko_gui_command header;
    short x, y;
    unsigned short w, h;
};

struct neko_gui_command_line {
    struct neko_gui_command header;
    unsigned short line_thickness;
    struct neko_gui_vec2i begin;
    struct neko_gui_vec2i end;
    struct neko_gui_color color;
};

struct neko_gui_command_curve {
    struct neko_gui_command header;
    unsigned short line_thickness;
    struct neko_gui_vec2i begin;
    struct neko_gui_vec2i end;
    struct neko_gui_vec2i ctrl[2];
    struct neko_gui_color color;
};

struct neko_gui_command_rect {
    struct neko_gui_command header;
    unsigned short rounding;
    unsigned short line_thickness;
    short x, y;
    unsigned short w, h;
    struct neko_gui_color color;
};

struct neko_gui_command_rect_filled {
    struct neko_gui_command header;
    unsigned short rounding;
    short x, y;
    unsigned short w, h;
    struct neko_gui_color color;
};

struct neko_gui_command_rect_multi_color {
    struct neko_gui_command header;
    short x, y;
    unsigned short w, h;
    struct neko_gui_color left;
    struct neko_gui_color top;
    struct neko_gui_color bottom;
    struct neko_gui_color right;
};

struct neko_gui_command_triangle {
    struct neko_gui_command header;
    unsigned short line_thickness;
    struct neko_gui_vec2i a;
    struct neko_gui_vec2i b;
    struct neko_gui_vec2i c;
    struct neko_gui_color color;
};

struct neko_gui_command_triangle_filled {
    struct neko_gui_command header;
    struct neko_gui_vec2i a;
    struct neko_gui_vec2i b;
    struct neko_gui_vec2i c;
    struct neko_gui_color color;
};

struct neko_gui_command_circle {
    struct neko_gui_command header;
    short x, y;
    unsigned short line_thickness;
    unsigned short w, h;
    struct neko_gui_color color;
};

struct neko_gui_command_circle_filled {
    struct neko_gui_command header;
    short x, y;
    unsigned short w, h;
    struct neko_gui_color color;
};

struct neko_gui_command_arc {
    struct neko_gui_command header;
    short cx, cy;
    unsigned short r;
    unsigned short line_thickness;
    float a[2];
    struct neko_gui_color color;
};

struct neko_gui_command_arc_filled {
    struct neko_gui_command header;
    short cx, cy;
    unsigned short r;
    float a[2];
    struct neko_gui_color color;
};

struct neko_gui_command_polygon {
    struct neko_gui_command header;
    struct neko_gui_color color;
    unsigned short line_thickness;
    unsigned short point_count;
    struct neko_gui_vec2i points[1];
};

struct neko_gui_command_polygon_filled {
    struct neko_gui_command header;
    struct neko_gui_color color;
    unsigned short point_count;
    struct neko_gui_vec2i points[1];
};

struct neko_gui_command_polyline {
    struct neko_gui_command header;
    struct neko_gui_color color;
    unsigned short line_thickness;
    unsigned short point_count;
    struct neko_gui_vec2i points[1];
};

struct neko_gui_command_image {
    struct neko_gui_command header;
    short x, y;
    unsigned short w, h;
    struct neko_gui_image img;
    struct neko_gui_color col;
};

typedef void (*neko_gui_command_custom_callback)(void *canvas, short x, short y, unsigned short w, unsigned short h, neko_gui_handle callback_data);
struct neko_gui_command_custom {
    struct neko_gui_command header;
    short x, y;
    unsigned short w, h;
    neko_gui_handle callback_data;
    neko_gui_command_custom_callback callback;
};

struct neko_gui_command_text {
    struct neko_gui_command header;
    const struct neko_gui_user_font *font;
    struct neko_gui_color background;
    struct neko_gui_color foreground;
    short x, y;
    unsigned short w, h;
    float height;
    int length;
    char string[1];
};

enum neko_gui_command_clipping { NEKO_GUI_CLIPPING_OFF = neko_gui_false, NEKO_GUI_CLIPPING_ON = neko_gui_true };

struct neko_gui_command_buffer {
    struct neko_gui_buffer *base;
    struct neko_gui_rect clip;
    int use_clipping;
    neko_gui_handle userdata;
    neko_gui_size begin, end, last;
};

NEKO_GUI_API void neko_gui_stroke_line(struct neko_gui_command_buffer *b, float x0, float y0, float x1, float y1, float line_thickness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_curve(struct neko_gui_command_buffer *, float, float, float, float, float, float, float, float, float line_thickness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_rect(struct neko_gui_command_buffer *, struct neko_gui_rect, float rounding, float line_thickness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_circle(struct neko_gui_command_buffer *, struct neko_gui_rect, float line_thickness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_arc(struct neko_gui_command_buffer *, float cx, float cy, float radius, float a_min, float a_max, float line_thickness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_triangle(struct neko_gui_command_buffer *, float, float, float, float, float, float, float line_thichness, struct neko_gui_color);
NEKO_GUI_API void neko_gui_stroke_polyline(struct neko_gui_command_buffer *, float *points, int point_count, float line_thickness, struct neko_gui_color col);
NEKO_GUI_API void neko_gui_stroke_polygon(struct neko_gui_command_buffer *, float *, int point_count, float line_thickness, struct neko_gui_color);

NEKO_GUI_API void neko_gui_fill_rect(struct neko_gui_command_buffer *, struct neko_gui_rect, float rounding, struct neko_gui_color);
NEKO_GUI_API void neko_gui_fill_rect_multi_color(struct neko_gui_command_buffer *, struct neko_gui_rect, struct neko_gui_color left, struct neko_gui_color top, struct neko_gui_color right,
                                                 struct neko_gui_color bottom);
NEKO_GUI_API void neko_gui_fill_circle(struct neko_gui_command_buffer *, struct neko_gui_rect, struct neko_gui_color);
NEKO_GUI_API void neko_gui_fill_arc(struct neko_gui_command_buffer *, float cx, float cy, float radius, float a_min, float a_max, struct neko_gui_color);
NEKO_GUI_API void neko_gui_fill_triangle(struct neko_gui_command_buffer *, float x0, float y0, float x1, float y1, float x2, float y2, struct neko_gui_color);
NEKO_GUI_API void neko_gui_fill_polygon(struct neko_gui_command_buffer *, float *, int point_count, struct neko_gui_color);

NEKO_GUI_API void neko_gui_draw_image(struct neko_gui_command_buffer *, struct neko_gui_rect, const struct neko_gui_image *, struct neko_gui_color);
NEKO_GUI_API void neko_gui_draw_nine_slice(struct neko_gui_command_buffer *, struct neko_gui_rect, const struct neko_gui_nine_slice *, struct neko_gui_color);
NEKO_GUI_API void neko_gui_draw_text(struct neko_gui_command_buffer *, struct neko_gui_rect, const char *text, int len, const struct neko_gui_user_font *, struct neko_gui_color,
                                     struct neko_gui_color);
NEKO_GUI_API void neko_gui_push_scissor(struct neko_gui_command_buffer *, struct neko_gui_rect);
NEKO_GUI_API void neko_gui_push_custom(struct neko_gui_command_buffer *, struct neko_gui_rect, neko_gui_command_custom_callback, neko_gui_handle usr);

struct neko_gui_mouse_button {
    neko_gui_bool down;
    unsigned int clicked;
    struct neko_gui_vec2 clicked_pos;
};
struct neko_gui_mouse {
    struct neko_gui_mouse_button buttons[NEKO_GUI_BUTTON_MAX];
    struct neko_gui_vec2 pos;
#ifdef NEKO_GUI_BUTTON_TRIGGER_ON_RELEASE
    struct neko_gui_vec2 down_pos;
#endif
    struct neko_gui_vec2 prev;
    struct neko_gui_vec2 delta;
    struct neko_gui_vec2 scroll_delta;
    unsigned char grab;
    unsigned char grabbed;
    unsigned char ungrab;
};

struct neko_gui_key {
    neko_gui_bool down;
    unsigned int clicked;
};
struct neko_gui_keyboard {
    struct neko_gui_key keys[NEKO_GUI_KEY_MAX];
    char text[NEKO_GUI_INPUT_MAX];
    int text_len;
};

struct neko_gui_input {
    struct neko_gui_keyboard keyboard;
    struct neko_gui_mouse mouse;
};

NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click(const struct neko_gui_input *, enum neko_gui_buttons);
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_in_rect(const struct neko_gui_input *, enum neko_gui_buttons, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_in_button_rect(const struct neko_gui_input *, enum neko_gui_buttons, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_down_in_rect(const struct neko_gui_input *, enum neko_gui_buttons, struct neko_gui_rect, neko_gui_bool down);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_click_in_rect(const struct neko_gui_input *, enum neko_gui_buttons, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_click_down_in_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b, neko_gui_bool down);
NEKO_GUI_API neko_gui_bool neko_gui_input_any_mouse_click_in_rect(const struct neko_gui_input *, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_prev_hovering_rect(const struct neko_gui_input *, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_hovering_rect(const struct neko_gui_input *, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_mouse_clicked(const struct neko_gui_input *, enum neko_gui_buttons, struct neko_gui_rect);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_down(const struct neko_gui_input *, enum neko_gui_buttons);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_pressed(const struct neko_gui_input *, enum neko_gui_buttons);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_released(const struct neko_gui_input *, enum neko_gui_buttons);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_pressed(const struct neko_gui_input *, enum neko_gui_keys);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_released(const struct neko_gui_input *, enum neko_gui_keys);
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_down(const struct neko_gui_input *, enum neko_gui_keys);

#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT

#ifdef NEKO_GUI_UINT_DRAW_INDEX
typedef neko_gui_uint neko_gui_draw_index;
#else
typedef neko_gui_ushort neko_gui_draw_index;
#endif
enum neko_gui_draw_list_stroke {
    NEKO_GUI_STROKE_OPEN = neko_gui_false,

    NEKO_GUI_STROKE_CLOSED = neko_gui_true

};

enum neko_gui_draw_vertex_layout_attribute { NEKO_GUI_VERTEX_POSITION, NEKO_GUI_VERTEX_COLOR, NEKO_GUI_VERTEX_TEXCOORD, NEKO_GUI_VERTEX_ATTRIBUTE_COUNT };

enum neko_gui_draw_vertex_layout_format {
    NEKO_GUI_FORMAT_SCHAR,
    NEKO_GUI_FORMAT_SSHORT,
    NEKO_GUI_FORMAT_SINT,
    NEKO_GUI_FORMAT_UCHAR,
    NEKO_GUI_FORMAT_USHORT,
    NEKO_GUI_FORMAT_UINT,
    NEKO_GUI_FORMAT_FLOAT,
    NEKO_GUI_FORMAT_DOUBLE,

    NEKO_GUI_FORMAT_COLOR_BEGIN,
    NEKO_GUI_FORMAT_R8G8B8 = NEKO_GUI_FORMAT_COLOR_BEGIN,
    NEKO_GUI_FORMAT_R16G15B16,
    NEKO_GUI_FORMAT_R32G32B32,

    NEKO_GUI_FORMAT_R8G8B8A8,
    NEKO_GUI_FORMAT_B8G8R8A8,
    NEKO_GUI_FORMAT_R16G15B16A16,
    NEKO_GUI_FORMAT_R32G32B32A32,
    NEKO_GUI_FORMAT_R32G32B32A32_FLOAT,
    NEKO_GUI_FORMAT_R32G32B32A32_DOUBLE,

    NEKO_GUI_FORMAT_RGB32,
    NEKO_GUI_FORMAT_RGBA32,
    NEKO_GUI_FORMAT_COLOR_END = NEKO_GUI_FORMAT_RGBA32,
    NEKO_GUI_FORMAT_COUNT
};

#define NEKO_GUI_VERTEX_LAYOUT_END NEKO_GUI_VERTEX_ATTRIBUTE_COUNT, NEKO_GUI_FORMAT_COUNT, 0
struct neko_gui_draw_vertex_layout_element {
    enum neko_gui_draw_vertex_layout_attribute attribute;
    enum neko_gui_draw_vertex_layout_format format;
    neko_gui_size offset;
};

struct neko_gui_draw_command {
    unsigned int elem_count;

    struct neko_gui_rect clip_rect;

    neko_gui_handle texture;

#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_handle userdata;
#endif
};

struct neko_gui_draw_list {
    struct neko_gui_rect clip_rect;
    struct neko_gui_vec2 circle_vtx[12];
    struct neko_gui_convert_config config;

    struct neko_gui_buffer *buffer;
    struct neko_gui_buffer *vertices;
    struct neko_gui_buffer *elements;

    unsigned int element_count;
    unsigned int vertex_count;
    unsigned int cmd_count;
    neko_gui_size cmd_offset;

    unsigned int path_count;
    unsigned int path_offset;

    enum neko_gui_anti_aliasing line_AA;
    enum neko_gui_anti_aliasing shape_AA;

#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_handle userdata;
#endif
};

NEKO_GUI_API void neko_gui_draw_list_init(struct neko_gui_draw_list *);
NEKO_GUI_API void neko_gui_draw_list_setup(struct neko_gui_draw_list *, const struct neko_gui_convert_config *, struct neko_gui_buffer *cmds, struct neko_gui_buffer *vertices,
                                           struct neko_gui_buffer *elements, enum neko_gui_anti_aliasing line_aa, enum neko_gui_anti_aliasing shape_aa);

#define neko_gui_draw_list_foreach(cmd, can, b) for ((cmd) = neko_gui__draw_list_begin(can, b); (cmd) != 0; (cmd) = neko_gui__draw_list_next(cmd, b, can))
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_begin(const struct neko_gui_draw_list *, const struct neko_gui_buffer *);
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_next(const struct neko_gui_draw_command *, const struct neko_gui_buffer *, const struct neko_gui_draw_list *);
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_end(const struct neko_gui_draw_list *, const struct neko_gui_buffer *);

NEKO_GUI_API void neko_gui_draw_list_path_clear(struct neko_gui_draw_list *);
NEKO_GUI_API void neko_gui_draw_list_path_line_to(struct neko_gui_draw_list *, struct neko_gui_vec2 pos);
NEKO_GUI_API void neko_gui_draw_list_path_arc_to_fast(struct neko_gui_draw_list *, struct neko_gui_vec2 center, float radius, int a_min, int a_max);
NEKO_GUI_API void neko_gui_draw_list_path_arc_to(struct neko_gui_draw_list *, struct neko_gui_vec2 center, float radius, float a_min, float a_max, unsigned int segments);
NEKO_GUI_API void neko_gui_draw_list_path_rect_to(struct neko_gui_draw_list *, struct neko_gui_vec2 a, struct neko_gui_vec2 b, float rounding);
NEKO_GUI_API void neko_gui_draw_list_path_curve_to(struct neko_gui_draw_list *, struct neko_gui_vec2 p2, struct neko_gui_vec2 p3, struct neko_gui_vec2 p4, unsigned int num_segments);
NEKO_GUI_API void neko_gui_draw_list_path_fill(struct neko_gui_draw_list *, struct neko_gui_color);
NEKO_GUI_API void neko_gui_draw_list_path_stroke(struct neko_gui_draw_list *, struct neko_gui_color, enum neko_gui_draw_list_stroke closed, float thickness);

NEKO_GUI_API void neko_gui_draw_list_stroke_line(struct neko_gui_draw_list *, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_color, float thickness);
NEKO_GUI_API void neko_gui_draw_list_stroke_rect(struct neko_gui_draw_list *, struct neko_gui_rect rect, struct neko_gui_color, float rounding, float thickness);
NEKO_GUI_API void neko_gui_draw_list_stroke_triangle(struct neko_gui_draw_list *, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_vec2 c, struct neko_gui_color, float thickness);
NEKO_GUI_API void neko_gui_draw_list_stroke_circle(struct neko_gui_draw_list *, struct neko_gui_vec2 center, float radius, struct neko_gui_color, unsigned int segs, float thickness);
NEKO_GUI_API void neko_gui_draw_list_stroke_curve(struct neko_gui_draw_list *, struct neko_gui_vec2 p0, struct neko_gui_vec2 cp0, struct neko_gui_vec2 cp1, struct neko_gui_vec2 p1,
                                                  struct neko_gui_color, unsigned int segments, float thickness);
NEKO_GUI_API void neko_gui_draw_list_stroke_poly_line(struct neko_gui_draw_list *, const struct neko_gui_vec2 *pnts, const unsigned int cnt, struct neko_gui_color, enum neko_gui_draw_list_stroke,
                                                      float thickness, enum neko_gui_anti_aliasing);

NEKO_GUI_API void neko_gui_draw_list_fill_rect(struct neko_gui_draw_list *, struct neko_gui_rect rect, struct neko_gui_color, float rounding);
NEKO_GUI_API void neko_gui_draw_list_fill_rect_multi_color(struct neko_gui_draw_list *, struct neko_gui_rect rect, struct neko_gui_color left, struct neko_gui_color top, struct neko_gui_color right,
                                                           struct neko_gui_color bottom);
NEKO_GUI_API void neko_gui_draw_list_fill_triangle(struct neko_gui_draw_list *, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_vec2 c, struct neko_gui_color);
NEKO_GUI_API void neko_gui_draw_list_fill_circle(struct neko_gui_draw_list *, struct neko_gui_vec2 center, float radius, struct neko_gui_color col, unsigned int segs);
NEKO_GUI_API void neko_gui_draw_list_fill_poly_convex(struct neko_gui_draw_list *, const struct neko_gui_vec2 *points, const unsigned int count, struct neko_gui_color, enum neko_gui_anti_aliasing);

NEKO_GUI_API void neko_gui_draw_list_add_image(struct neko_gui_draw_list *, struct neko_gui_image texture, struct neko_gui_rect rect, struct neko_gui_color);
NEKO_GUI_API void neko_gui_draw_list_add_text(struct neko_gui_draw_list *, const struct neko_gui_user_font *, struct neko_gui_rect, const char *text, int len, float font_height,
                                              struct neko_gui_color);
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
NEKO_GUI_API void neko_gui_draw_list_push_userdata(struct neko_gui_draw_list *, neko_gui_handle userdata);
#endif

#endif

enum neko_gui_style_item_type { NEKO_GUI_STYLE_ITEM_COLOR, NEKO_GUI_STYLE_ITEM_IMAGE, NEKO_GUI_STYLE_ITEM_NINE_SLICE };

union neko_gui_style_item_data {
    struct neko_gui_color color;
    struct neko_gui_image image;
    struct neko_gui_nine_slice slice;
};

struct neko_gui_style_item {
    enum neko_gui_style_item_type type;
    union neko_gui_style_item_data data;
};

struct neko_gui_style_text {
    struct neko_gui_color color;
    struct neko_gui_vec2 padding;
};

struct neko_gui_style_button {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_color text_background;
    struct neko_gui_color text_normal;
    struct neko_gui_color text_hover;
    struct neko_gui_color text_active;
    neko_gui_flags text_alignment;

    float border;
    float rounding;
    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 image_padding;
    struct neko_gui_vec2 touch_padding;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle userdata);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle userdata);
};

struct neko_gui_style_toggle {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_style_item cursor_normal;
    struct neko_gui_style_item cursor_hover;

    struct neko_gui_color text_normal;
    struct neko_gui_color text_hover;
    struct neko_gui_color text_active;
    struct neko_gui_color text_background;
    neko_gui_flags text_alignment;

    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 touch_padding;
    float spacing;
    float border;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_selectable {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item pressed;

    struct neko_gui_style_item normal_active;
    struct neko_gui_style_item hover_active;
    struct neko_gui_style_item pressed_active;

    struct neko_gui_color text_normal;
    struct neko_gui_color text_hover;
    struct neko_gui_color text_pressed;

    struct neko_gui_color text_normal_active;
    struct neko_gui_color text_hover_active;
    struct neko_gui_color text_pressed_active;
    struct neko_gui_color text_background;
    neko_gui_flags text_alignment;

    float rounding;
    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 touch_padding;
    struct neko_gui_vec2 image_padding;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_slider {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_color bar_normal;
    struct neko_gui_color bar_hover;
    struct neko_gui_color bar_active;
    struct neko_gui_color bar_filled;

    struct neko_gui_style_item cursor_normal;
    struct neko_gui_style_item cursor_hover;
    struct neko_gui_style_item cursor_active;

    float border;
    float rounding;
    float bar_height;
    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 spacing;
    struct neko_gui_vec2 cursor_size;

    int show_buttons;
    struct neko_gui_style_button inc_button;
    struct neko_gui_style_button dec_button;
    enum neko_gui_symbol_type inc_symbol;
    enum neko_gui_symbol_type dec_symbol;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_progress {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_style_item cursor_normal;
    struct neko_gui_style_item cursor_hover;
    struct neko_gui_style_item cursor_active;
    struct neko_gui_color cursor_border_color;

    float rounding;
    float border;
    float cursor_border;
    float cursor_rounding;
    struct neko_gui_vec2 padding;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_scrollbar {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_style_item cursor_normal;
    struct neko_gui_style_item cursor_hover;
    struct neko_gui_style_item cursor_active;
    struct neko_gui_color cursor_border_color;

    float border;
    float rounding;
    float border_cursor;
    float rounding_cursor;
    struct neko_gui_vec2 padding;

    int show_buttons;
    struct neko_gui_style_button inc_button;
    struct neko_gui_style_button dec_button;
    enum neko_gui_symbol_type inc_symbol;
    enum neko_gui_symbol_type dec_symbol;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_edit {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;
    struct neko_gui_style_scrollbar scrollbar;

    struct neko_gui_color cursor_normal;
    struct neko_gui_color cursor_hover;
    struct neko_gui_color cursor_text_normal;
    struct neko_gui_color cursor_text_hover;

    struct neko_gui_color text_normal;
    struct neko_gui_color text_hover;
    struct neko_gui_color text_active;

    struct neko_gui_color selected_normal;
    struct neko_gui_color selected_hover;
    struct neko_gui_color selected_text_normal;
    struct neko_gui_color selected_text_hover;

    float border;
    float rounding;
    float cursor_size;
    struct neko_gui_vec2 scrollbar_size;
    struct neko_gui_vec2 padding;
    float row_padding;
};

struct neko_gui_style_property {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_color label_normal;
    struct neko_gui_color label_hover;
    struct neko_gui_color label_active;

    enum neko_gui_symbol_type sym_left;
    enum neko_gui_symbol_type sym_right;

    float border;
    float rounding;
    struct neko_gui_vec2 padding;

    struct neko_gui_style_edit edit;
    struct neko_gui_style_button inc_button;
    struct neko_gui_style_button dec_button;

    neko_gui_handle userdata;
    void (*draw_begin)(struct neko_gui_command_buffer *, neko_gui_handle);
    void (*draw_end)(struct neko_gui_command_buffer *, neko_gui_handle);
};

struct neko_gui_style_chart {

    struct neko_gui_style_item background;
    struct neko_gui_color border_color;
    struct neko_gui_color selected_color;
    struct neko_gui_color color;

    float border;
    float rounding;
    struct neko_gui_vec2 padding;
};

struct neko_gui_style_combo {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;
    struct neko_gui_color border_color;

    struct neko_gui_color label_normal;
    struct neko_gui_color label_hover;
    struct neko_gui_color label_active;

    struct neko_gui_color symbol_normal;
    struct neko_gui_color symbol_hover;
    struct neko_gui_color symbol_active;

    struct neko_gui_style_button button;
    enum neko_gui_symbol_type sym_normal;
    enum neko_gui_symbol_type sym_hover;
    enum neko_gui_symbol_type sym_active;

    float border;
    float rounding;
    struct neko_gui_vec2 content_padding;
    struct neko_gui_vec2 button_padding;
    struct neko_gui_vec2 spacing;
};

struct neko_gui_style_tab {

    struct neko_gui_style_item background;
    struct neko_gui_color border_color;
    struct neko_gui_color text;

    struct neko_gui_style_button tab_maximize_button;
    struct neko_gui_style_button tab_minimize_button;
    struct neko_gui_style_button node_maximize_button;
    struct neko_gui_style_button node_minimize_button;
    enum neko_gui_symbol_type sym_minimize;
    enum neko_gui_symbol_type sym_maximize;

    float border;
    float rounding;
    float indent;
    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 spacing;
};

enum neko_gui_style_header_align { NEKO_GUI_HEADER_LEFT, NEKO_GUI_HEADER_RIGHT };
struct neko_gui_style_window_header {

    struct neko_gui_style_item normal;
    struct neko_gui_style_item hover;
    struct neko_gui_style_item active;

    struct neko_gui_style_button close_button;
    struct neko_gui_style_button minimize_button;
    enum neko_gui_symbol_type close_symbol;
    enum neko_gui_symbol_type minimize_symbol;
    enum neko_gui_symbol_type maximize_symbol;

    struct neko_gui_color label_normal;
    struct neko_gui_color label_hover;
    struct neko_gui_color label_active;

    enum neko_gui_style_header_align align;
    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 label_padding;
    struct neko_gui_vec2 spacing;
};

struct neko_gui_style_window {
    struct neko_gui_style_window_header header;
    struct neko_gui_style_item fixed_background;
    struct neko_gui_color background;

    struct neko_gui_color border_color;
    struct neko_gui_color popup_border_color;
    struct neko_gui_color combo_border_color;
    struct neko_gui_color contextual_border_color;
    struct neko_gui_color menu_border_color;
    struct neko_gui_color group_border_color;
    struct neko_gui_color tooltip_border_color;
    struct neko_gui_style_item scaler;

    float border;
    float combo_border;
    float contextual_border;
    float menu_border;
    float group_border;
    float tooltip_border;
    float popup_border;
    float min_row_height_padding;

    float rounding;
    struct neko_gui_vec2 spacing;
    struct neko_gui_vec2 scrollbar_size;
    struct neko_gui_vec2 min_size;

    struct neko_gui_vec2 padding;
    struct neko_gui_vec2 group_padding;
    struct neko_gui_vec2 popup_padding;
    struct neko_gui_vec2 combo_padding;
    struct neko_gui_vec2 contextual_padding;
    struct neko_gui_vec2 menu_padding;
    struct neko_gui_vec2 tooltip_padding;
};

struct neko_gui_style {
    const struct neko_gui_user_font *font;
    const struct neko_gui_cursor *cursors[NEKO_GUI_CURSOR_COUNT];
    const struct neko_gui_cursor *cursor_active;
    struct neko_gui_cursor *cursor_last;
    int cursor_visible;

    struct neko_gui_style_text text;
    struct neko_gui_style_button button;
    struct neko_gui_style_button contextual_button;
    struct neko_gui_style_button menu_button;
    struct neko_gui_style_toggle option;
    struct neko_gui_style_toggle checkbox;
    struct neko_gui_style_selectable selectable;
    struct neko_gui_style_slider slider;
    struct neko_gui_style_progress progress;
    struct neko_gui_style_property property;
    struct neko_gui_style_edit edit;
    struct neko_gui_style_chart chart;
    struct neko_gui_style_scrollbar scrollh;
    struct neko_gui_style_scrollbar scrollv;
    struct neko_gui_style_tab tab;
    struct neko_gui_style_combo combo;
    struct neko_gui_style_window window;
};

NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_color(struct neko_gui_color);
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_image(struct neko_gui_image img);
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_nine_slice(struct neko_gui_nine_slice slice);
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_hide(void);

#ifndef NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS
#define NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS 16
#endif
#ifndef NEKO_GUI_CHART_MAX_SLOT
#define NEKO_GUI_CHART_MAX_SLOT 4
#endif

enum neko_gui_panel_type {
    NEKO_GUI_PANEL_NONE = 0,
    NEKO_GUI_PANEL_WINDOW = NEKO_GUI_FLAG(0),
    NEKO_GUI_PANEL_GROUP = NEKO_GUI_FLAG(1),
    NEKO_GUI_PANEL_POPUP = NEKO_GUI_FLAG(2),
    NEKO_GUI_PANEL_CONTEXTUAL = NEKO_GUI_FLAG(4),
    NEKO_GUI_PANEL_COMBO = NEKO_GUI_FLAG(5),
    NEKO_GUI_PANEL_MENU = NEKO_GUI_FLAG(6),
    NEKO_GUI_PANEL_TOOLTIP = NEKO_GUI_FLAG(7)
};
enum neko_gui_panel_set {
    NEKO_GUI_PANEL_SET_NONBLOCK = NEKO_GUI_PANEL_CONTEXTUAL | NEKO_GUI_PANEL_COMBO | NEKO_GUI_PANEL_MENU | NEKO_GUI_PANEL_TOOLTIP,
    NEKO_GUI_PANEL_SET_POPUP = NEKO_GUI_PANEL_SET_NONBLOCK | NEKO_GUI_PANEL_POPUP,
    NEKO_GUI_PANEL_SET_SUB = NEKO_GUI_PANEL_SET_POPUP | NEKO_GUI_PANEL_GROUP
};

struct neko_gui_chart_slot {
    enum neko_gui_chart_type type;
    struct neko_gui_color color;
    struct neko_gui_color highlight;
    float min, max, range;
    int count;
    struct neko_gui_vec2 last;
    int index;
};

struct neko_gui_chart {
    int slot;
    float x, y, w, h;
    struct neko_gui_chart_slot slots[NEKO_GUI_CHART_MAX_SLOT];
};

enum neko_gui_panel_row_layout_type {
    NEKO_GUI_LAYOUT_DYNAMIC_FIXED = 0,
    NEKO_GUI_LAYOUT_DYNAMIC_ROW,
    NEKO_GUI_LAYOUT_DYNAMIC_FREE,
    NEKO_GUI_LAYOUT_DYNAMIC,
    NEKO_GUI_LAYOUT_STATIC_FIXED,
    NEKO_GUI_LAYOUT_STATIC_ROW,
    NEKO_GUI_LAYOUT_STATIC_FREE,
    NEKO_GUI_LAYOUT_STATIC,
    NEKO_GUI_LAYOUT_TEMPLATE,
    NEKO_GUI_LAYOUT_COUNT
};
struct neko_gui_row_layout {
    enum neko_gui_panel_row_layout_type type;
    int index;
    float height;
    float min_height;
    int columns;
    const float *ratio;
    float item_width;
    float item_height;
    float item_offset;
    float filled;
    struct neko_gui_rect item;
    int tree_depth;
    float templates[NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS];
};

struct neko_gui_popup_buffer {
    neko_gui_size begin;
    neko_gui_size parent;
    neko_gui_size last;
    neko_gui_size end;
    neko_gui_bool active;
};

struct neko_gui_menu_state {
    float x, y, w, h;
    struct neko_gui_scroll offset;
};

struct neko_gui_panel {
    enum neko_gui_panel_type type;
    neko_gui_flags flags;
    struct neko_gui_rect bounds;
    neko_gui_uint *offset_x;
    neko_gui_uint *offset_y;
    float at_x, at_y, max_x;
    float footer_height;
    float header_height;
    float border;
    unsigned int has_scrolling;
    struct neko_gui_rect clip;
    struct neko_gui_menu_state menu;
    struct neko_gui_row_layout row;
    struct neko_gui_chart chart;
    struct neko_gui_command_buffer *buffer;
    struct neko_gui_panel *parent;
};

#ifndef NEKO_GUI_WINDOW_MAX_NAME
#define NEKO_GUI_WINDOW_MAX_NAME 64
#endif

struct neko_gui_table;
enum neko_gui_window_flags {
    NEKO_GUI_WINDOW_PRIVATE = NEKO_GUI_FLAG(11),
    NEKO_GUI_WINDOW_DYNAMIC = NEKO_GUI_WINDOW_PRIVATE,

    NEKO_GUI_WINDOW_ROM = NEKO_GUI_FLAG(12),

    NEKO_GUI_WINDOW_NOT_INTERACTIVE = NEKO_GUI_WINDOW_ROM | NEKO_GUI_WINDOW_NO_INPUT,

    NEKO_GUI_WINDOW_HIDDEN = NEKO_GUI_FLAG(13),

    NEKO_GUI_WINDOW_CLOSED = NEKO_GUI_FLAG(14),

    NEKO_GUI_WINDOW_MINIMIZED = NEKO_GUI_FLAG(15),

    NEKO_GUI_WINDOW_REMOVE_ROM = NEKO_GUI_FLAG(16),

    NEKO_GUI_WINDOW_NO_SAVE = NEKO_GUI_FLAG(17)

};

struct neko_gui_popup_state {
    struct neko_gui_window *win;
    enum neko_gui_panel_type type;
    struct neko_gui_popup_buffer buf;
    neko_gui_hash name;
    neko_gui_bool active;
    unsigned combo_count;
    unsigned con_count, con_old;
    unsigned active_con;
    struct neko_gui_rect header;
};

struct neko_gui_edit_state {
    neko_gui_hash name;
    unsigned int seq;
    unsigned int old;
    int active, prev;
    int cursor;
    int sel_start;
    int sel_end;
    struct neko_gui_scroll scrollbar;
    unsigned char mode;
    unsigned char single_line;
};

struct neko_gui_property_state {
    int active, prev;
    char buffer[NEKO_GUI_MAX_NUMBER_BUFFER];
    int length;
    int cursor;
    int select_start;
    int select_end;
    neko_gui_hash name;
    unsigned int seq;
    unsigned int old;
    int state;
};

struct neko_gui_window {
    unsigned int seq;
    neko_gui_hash name;
    char name_string[NEKO_GUI_WINDOW_MAX_NAME];
    neko_gui_flags flags;

    struct neko_gui_rect bounds;
    struct neko_gui_scroll scrollbar;
    struct neko_gui_command_buffer buffer;
    struct neko_gui_panel *layout;
    float scrollbar_hiding_timer;

    struct neko_gui_property_state property;
    struct neko_gui_popup_state popup;
    struct neko_gui_edit_state edit;
    unsigned int scrolled;

    struct neko_gui_table *tables;
    unsigned int table_count;

    struct neko_gui_window *next;
    struct neko_gui_window *prev;
    struct neko_gui_window *parent;
};

#ifndef NEKO_GUI_BUTTON_BEHAVIOR_STACK_SIZE
#define NEKO_GUI_BUTTON_BEHAVIOR_STACK_SIZE 8
#endif

#ifndef NEKO_GUI_FONT_STACK_SIZE
#define NEKO_GUI_FONT_STACK_SIZE 8
#endif

#ifndef NEKO_GUI_STYLE_ITEM_STACK_SIZE
#define NEKO_GUI_STYLE_ITEM_STACK_SIZE 16
#endif

#ifndef NEKO_GUI_FLOAT_STACK_SIZE
#define NEKO_GUI_FLOAT_STACK_SIZE 32
#endif

#ifndef NEKO_GUI_VECTOR_STACK_SIZE
#define NEKO_GUI_VECTOR_STACK_SIZE 16
#endif

#ifndef NEKO_GUI_FLAGS_STACK_SIZE
#define NEKO_GUI_FLAGS_STACK_SIZE 32
#endif

#ifndef NEKO_GUI_COLOR_STACK_SIZE
#define NEKO_GUI_COLOR_STACK_SIZE 32
#endif

#define NEKO_GUI_CONFIGURATION_STACK_TYPE(prefix, name, type) \
    struct neko_gui_config_stack_##name##_element {           \
        prefix##_##type *address;                             \
        prefix##_##type old_value;                            \
    }
#define NEKO_GUI_CONFIG_STACK(type, size)                             \
    struct neko_gui_config_stack_##type {                             \
        int head;                                                     \
        struct neko_gui_config_stack_##type##_element elements[size]; \
    }

#define neko_gui_float float
NEKO_GUI_CONFIGURATION_STACK_TYPE(struct neko_gui, style_item, style_item);
NEKO_GUI_CONFIGURATION_STACK_TYPE(neko_gui, float, float);
NEKO_GUI_CONFIGURATION_STACK_TYPE(struct neko_gui, vec2, vec2);
NEKO_GUI_CONFIGURATION_STACK_TYPE(neko_gui, flags, flags);
NEKO_GUI_CONFIGURATION_STACK_TYPE(struct neko_gui, color, color);
NEKO_GUI_CONFIGURATION_STACK_TYPE(const struct neko_gui, user_font, user_font *);
NEKO_GUI_CONFIGURATION_STACK_TYPE(enum neko_gui, button_behavior, button_behavior);

NEKO_GUI_CONFIG_STACK(style_item, NEKO_GUI_STYLE_ITEM_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(float, NEKO_GUI_FLOAT_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(vec2, NEKO_GUI_VECTOR_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(flags, NEKO_GUI_FLAGS_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(color, NEKO_GUI_COLOR_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(user_font, NEKO_GUI_FONT_STACK_SIZE);
NEKO_GUI_CONFIG_STACK(button_behavior, NEKO_GUI_BUTTON_BEHAVIOR_STACK_SIZE);

struct neko_gui_configuration_stacks {
    struct neko_gui_config_stack_style_item style_items;
    struct neko_gui_config_stack_float floats;
    struct neko_gui_config_stack_vec2 vectors;
    struct neko_gui_config_stack_flags flags;
    struct neko_gui_config_stack_color colors;
    struct neko_gui_config_stack_user_font fonts;
    struct neko_gui_config_stack_button_behavior button_behaviors;
};

#define NEKO_GUI_VALUE_PAGE_CAPACITY (((NEKO_GUI_MAX(sizeof(struct neko_gui_window), sizeof(struct neko_gui_panel)) / sizeof(neko_gui_uint))) / 2)

struct neko_gui_table {
    unsigned int seq;
    unsigned int size;
    neko_gui_hash keys[NEKO_GUI_VALUE_PAGE_CAPACITY];
    neko_gui_uint values[NEKO_GUI_VALUE_PAGE_CAPACITY];
    struct neko_gui_table *next, *prev;
};

union neko_gui_page_data {
    struct neko_gui_table tbl;
    struct neko_gui_panel pan;
    struct neko_gui_window win;
};

struct neko_gui_page_element {
    union neko_gui_page_data data;
    struct neko_gui_page_element *next;
    struct neko_gui_page_element *prev;
};

struct neko_gui_page {
    unsigned int size;
    struct neko_gui_page *next;
    struct neko_gui_page_element win[1];
};

struct neko_gui_pool {
    struct neko_gui_allocator alloc;
    enum neko_gui_allocation_type type;
    unsigned int page_count;
    struct neko_gui_page *pages;
    struct neko_gui_page_element *freelist;
    unsigned capacity;
    neko_gui_size size;
    neko_gui_size cap;
};

struct neko_gui_context {

    struct neko_gui_input input;
    struct neko_gui_style style;
    struct neko_gui_buffer memory;
    struct neko_gui_clipboard clip;
    neko_gui_flags last_widget_state;
    enum neko_gui_button_behavior button_behavior;
    struct neko_gui_configuration_stacks stacks;
    float delta_time_seconds;

#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
    struct neko_gui_draw_list draw_list;
#endif
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_handle userdata;
#endif

    struct neko_gui_text_edit text_edit;

    struct neko_gui_command_buffer overlay;

    int build;
    int use_pool;
    struct neko_gui_pool pool;
    struct neko_gui_window *begin;
    struct neko_gui_window *end;
    struct neko_gui_window *active;
    struct neko_gui_window *current;
    struct neko_gui_page_element *freelist;
    unsigned int count;
    unsigned int seq;
};

#define NEKO_GUI_PI 3.141592654f
#define NEKO_GUI_UTF_INVALID 0xFFFD
#define NEKO_GUI_MAX_FLOAT_PRECISION 2

#define NEKO_GUI_UNUSED(x) ((void)(x))
#define NEKO_GUI_SATURATE(x) (NEKO_GUI_MAX(0, NEKO_GUI_MIN(1.0f, x)))
#define NEKO_GUI_LEN(a) (sizeof(a) / sizeof(a)[0])
#define NEKO_GUI_ABS(a) (((a) < 0) ? -(a) : (a))
#define NEKO_GUI_BETWEEN(x, a, b) ((a) <= (x) && (x) < (b))
#define NEKO_GUI_INBOX(px, py, x, y, w, h) (NEKO_GUI_BETWEEN(px, x, x + w) && NEKO_GUI_BETWEEN(py, y, y + h))
#define NEKO_GUI_INTERSECT(x0, y0, w0, h0, x1, y1, w1, h1) ((x1 < (x0 + w0)) && (x0 < (x1 + w1)) && (y1 < (y0 + h0)) && (y0 < (y1 + h1)))
#define NEKO_GUI_CONTAINS(x, y, w, h, bx, by, bw, bh) (NEKO_GUI_INBOX(x, y, bx, by, bw, bh) && NEKO_GUI_INBOX(x + w, y + h, bx, by, bw, bh))

#define neko_gui_vec2_sub(a, b) neko_gui_vec2((a).x - (b).x, (a).y - (b).y)
#define neko_gui_vec2_add(a, b) neko_gui_vec2((a).x + (b).x, (a).y + (b).y)
#define neko_gui_vec2_len_sqr(a) ((a).x * (a).x + (a).y * (a).y)
#define neko_gui_vec2_muls(a, t) neko_gui_vec2((a).x *(t), (a).y *(t))

#define neko_gui_ptr_add(t, p, i) ((t *)((void *)((neko_gui_byte *)(p) + (i))))
#define neko_gui_ptr_add_const(t, p, i) ((const t *)((const void *)((const neko_gui_byte *)(p) + (i))))
#define neko_gui_zero_struct(s) neko_gui_zero(&s, sizeof(s))

#if defined(__PTRDIFF_TYPE__)
#define NEKO_GUI_UINT_TO_PTR(x) ((void *)(__PTRDIFF_TYPE__)(x))
#define NEKO_GUI_PTR_TO_UINT(x) ((neko_gui_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__)
#define NEKO_GUI_UINT_TO_PTR(x) ((void *)&((char *)0)[x])
#define NEKO_GUI_PTR_TO_UINT(x) ((neko_gui_size)(((char *)x) - (char *)0))
#elif defined(NEKO_GUI_USE_FIXED_TYPES)
#define NEKO_GUI_UINT_TO_PTR(x) ((void *)(uintptr_t)(x))
#define NEKO_GUI_PTR_TO_UINT(x) ((uintptr_t)(x))
#else
#define NEKO_GUI_UINT_TO_PTR(x) ((void *)(x))
#define NEKO_GUI_PTR_TO_UINT(x) ((neko_gui_size)(x))
#endif

#define NEKO_GUI_ALIGN_PTR(x, mask) (NEKO_GUI_UINT_TO_PTR((NEKO_GUI_PTR_TO_UINT((neko_gui_byte *)(x) + (mask - 1)) & ~(mask - 1))))
#define NEKO_GUI_ALIGN_PTR_BACK(x, mask) (NEKO_GUI_UINT_TO_PTR((NEKO_GUI_PTR_TO_UINT((neko_gui_byte *)(x)) & ~(mask - 1))))

#if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#define NEKO_GUI_OFFSETOF(st, m) (__builtin_offsetof(st, m))
#else
#define NEKO_GUI_OFFSETOF(st, m) ((neko_gui_ptr) & (((st *)0)->m))
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
template <typename T>
struct neko_gui_alignof;
template <typename T, int size_diff>
struct neko_gui_helper {
    enum { value = size_diff };
};
template <typename T>
struct neko_gui_helper<T, 0> {
    enum { value = neko_gui_alignof<T>::value };
};
template <typename T>
struct neko_gui_alignof {
    struct Big {
        T x;
        char c;
    };
    enum { diff = sizeof(Big) - sizeof(T), value = neko_gui_helper<Big, diff>::value };
};
#define NEKO_GUI_ALIGNOF(t) (neko_gui_alignof<t>::value)
#else
#define NEKO_GUI_ALIGNOF(t) \
    NEKO_GUI_OFFSETOF(      \
            struct {        \
                char c;     \
                t _h;       \
            },              \
            _h)
#endif

#define NEKO_GUI_CONTAINER_OF(ptr, type, member) (type *)((void *)((char *)(1 ? (ptr) : &((type *)0)->member) - NEKO_GUI_OFFSETOF(type, member)))

#ifndef NEKO_GUI_MEMSET
#define NEKO_GUI_MEMSET neko_gui_memset
NEKO_GUI_LIB void neko_gui_memset(void *ptr, int c0, neko_gui_size size) {
#define neko_gui_word unsigned
#define neko_gui_wsize sizeof(neko_gui_word)
#define neko_gui_wmask (neko_gui_wsize - 1)
    neko_gui_byte *dst = (neko_gui_byte *)ptr;
    unsigned c = 0;
    neko_gui_size t = 0;

    if ((c = (neko_gui_byte)c0) != 0) {
        c = (c << 8) | c;
        if (sizeof(unsigned int) > 2) c = (c << 16) | c;
    }

    dst = (neko_gui_byte *)ptr;
    if (size < 3 * neko_gui_wsize) {
        while (size--) *dst++ = (neko_gui_byte)c0;
        return;
    }

    if ((t = NEKO_GUI_PTR_TO_UINT(dst) & neko_gui_wmask) != 0) {
        t = neko_gui_wsize - t;
        size -= t;
        do {
            *dst++ = (neko_gui_byte)c0;
        } while (--t != 0);
    }

    t = size / neko_gui_wsize;
    do {
        *(neko_gui_word *)((void *)dst) = c;
        dst += neko_gui_wsize;
    } while (--t != 0);

    t = (size & neko_gui_wmask);
    if (t != 0) {
        do {
            *dst++ = (neko_gui_byte)c0;
        } while (--t != 0);
    }

#undef neko_gui_word
#undef neko_gui_wsize
#undef neko_gui_wmask
}
#endif

#endif