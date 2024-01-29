
#include "neko_gui_impl.h"

#ifndef NEKO_GUI_INTERNAL_H
#define NEKO_GUI_INTERNAL_H

#ifndef NEKO_GUI_POOL_DEFAULT_CAPACITY
#define NEKO_GUI_POOL_DEFAULT_CAPACITY 16
#endif

#ifndef NEKO_GUI_DEFAULT_COMMAND_BUFFER_SIZE
#define NEKO_GUI_DEFAULT_COMMAND_BUFFER_SIZE (4 * 1024)
#endif

#ifndef NEKO_GUI_BUFFER_DEFAULT_INITIAL_SIZE
#define NEKO_GUI_BUFFER_DEFAULT_INITIAL_SIZE (4 * 1024)
#endif

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
#include <stdlib.h>
#endif
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
#include <stdio.h>
#endif
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
#include <stdarg.h>
#endif
#ifndef NEKO_GUI_ASSERT
#include <assert.h>
#define NEKO_GUI_ASSERT(expr) assert(expr)
#endif

#define NEKO_GUI_DEFAULT (-1)

#ifndef NEKO_GUI_VSNPRINTF

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)) || \
        (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)) || defined(_ISOC99_SOURCE) || defined(_BSD_SOURCE)
#define NEKO_GUI_VSNPRINTF(s, n, f, a) vsnprintf(s, n, f, a)
#else
#define NEKO_GUI_VSNPRINTF(s, n, f, a) vsprintf(s, f, a)
#endif
#endif

#define NEKO_GUI_SCHAR_MIN (-127)
#define NEKO_GUI_SCHAR_MAX 127
#define NEKO_GUI_UCHAR_MIN 0
#define NEKO_GUI_UCHAR_MAX 256
#define NEKO_GUI_SSHORT_MIN (-32767)
#define NEKO_GUI_SSHORT_MAX 32767
#define NEKO_GUI_USHORT_MIN 0
#define NEKO_GUI_USHORT_MAX 65535
#define NEKO_GUI_SINT_MIN (-2147483647)
#define NEKO_GUI_SINT_MAX 2147483647
#define NEKO_GUI_UINT_MIN 0
#define NEKO_GUI_UINT_MAX 4294967295u

NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_size) >= sizeof(void *));
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_ptr) == sizeof(void *));
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_flags) >= 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_rune) >= 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_ushort) == 2);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_short) == 2);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_uint) == 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_int) == 4);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_byte) == 1);
NEKO_GUI_STATIC_ASSERT(sizeof(neko_gui_bool) == 1);

NEKO_GUI_GLOBAL const struct neko_gui_rect neko_gui_null_rect = {-8192.0f, -8192.0f, 16384, 16384};
#define NEKO_GUI_FLOAT_PRECISION 0.00000000000001

NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_red = {255, 0, 0, 255};
NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_green = {0, 255, 0, 255};
NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_blue = {0, 0, 255, 255};
NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_white = {255, 255, 255, 255};
NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_black = {0, 0, 0, 255};
NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_yellow = {255, 255, 0, 255};

#define neko_gui_widget_state_reset(s)                                            \
    if ((*(s)) & NEKO_GUI_WIDGET_STATE_MODIFIED)                                  \
        (*(s)) = NEKO_GUI_WIDGET_STATE_INACTIVE | NEKO_GUI_WIDGET_STATE_MODIFIED; \
    else                                                                          \
        (*(s)) = NEKO_GUI_WIDGET_STATE_INACTIVE;

#ifndef NEKO_GUI_INV_SQRT
NEKO_GUI_LIB float neko_gui_inv_sqrt(float n);
#endif
#ifndef NEKO_GUI_SIN
NEKO_GUI_LIB float neko_gui_sin(float x);
#endif
#ifndef NEKO_GUI_COS
NEKO_GUI_LIB float neko_gui_cos(float x);
#endif
NEKO_GUI_LIB neko_gui_uint neko_gui_round_up_pow2(neko_gui_uint v);
NEKO_GUI_LIB struct neko_gui_rect neko_gui_shrineko_gui_rect(struct neko_gui_rect r, float amount);
NEKO_GUI_LIB struct neko_gui_rect neko_gui_pad_rect(struct neko_gui_rect r, struct neko_gui_vec2 pad);
NEKO_GUI_LIB void neko_gui_unify(struct neko_gui_rect *clip, const struct neko_gui_rect *a, float x0, float y0, float x1, float y1);
NEKO_GUI_LIB double neko_gui_pow(double x, int n);
NEKO_GUI_LIB int neko_gui_ifloord(double x);
NEKO_GUI_LIB int neko_gui_ifloorf(float x);
NEKO_GUI_LIB int neko_gui_iceilf(float x);
NEKO_GUI_LIB int neko_gui_log10(double n);

enum { NEKO_GUI_DO_NOT_STOP_ON_NEW_LINE, NEKO_GUI_STOP_ON_NEW_LINE };
NEKO_GUI_LIB neko_gui_bool neko_gui_is_lower(int c);
NEKO_GUI_LIB neko_gui_bool neko_gui_is_upper(int c);
NEKO_GUI_LIB int neko_gui_to_upper(int c);
NEKO_GUI_LIB int neko_gui_to_lower(int c);

#ifndef NEKO_GUI_MEMCPY
NEKO_GUI_LIB void *neko_gui_memcopy(void *dst, const void *src, neko_gui_size n);
#endif
#ifndef NEKO_GUI_MEMSET
NEKO_GUI_LIB void neko_gui_memset(void *ptr, int c0, neko_gui_size size);
#endif
NEKO_GUI_LIB void neko_gui_zero(void *ptr, neko_gui_size size);
NEKO_GUI_LIB char *neko_gui_itoa(char *s, long n);
NEKO_GUI_LIB int neko_gui_string_float_limit(char *string, int prec);
#ifndef NEKO_GUI_DTOA
NEKO_GUI_LIB char *neko_gui_dtoa(char *s, double n);
#endif
NEKO_GUI_LIB int neko_gui_text_clamp(const struct neko_gui_user_font *font, const char *text, int text_len, float space, int *glyphs, float *text_width, neko_gui_rune *sep_list, int sep_count);
NEKO_GUI_LIB struct neko_gui_vec2 neko_gui_text_calculate_text_bounds(const struct neko_gui_user_font *font, const char *begin, int byte_len, float row_height, const char **remaining,
                                                                      struct neko_gui_vec2 *out_offset, int *glyphs, int op);
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
NEKO_GUI_LIB int neko_gui_strfmt(char *buf, int buf_size, const char *fmt, va_list args);
#endif
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
NEKO_GUI_LIB char *neko_gui_file_load(const char *path, neko_gui_size *siz, struct neko_gui_allocator *alloc);
#endif

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_LIB void *neko_gui_malloc(neko_gui_handle unused, void *old, neko_gui_size size);
NEKO_GUI_LIB void neko_gui_mfree(neko_gui_handle unused, void *ptr);
#endif
NEKO_GUI_LIB void *neko_gui_buffer_align(void *unaligned, neko_gui_size align, neko_gui_size *alignment, enum neko_gui_buffer_allocation_type type);
NEKO_GUI_LIB void *neko_gui_buffer_alloc(struct neko_gui_buffer *b, enum neko_gui_buffer_allocation_type type, neko_gui_size size, neko_gui_size align);
NEKO_GUI_LIB void *neko_gui_buffer_realloc(struct neko_gui_buffer *b, neko_gui_size capacity, neko_gui_size *size);

NEKO_GUI_LIB void neko_gui_command_buffer_init(struct neko_gui_command_buffer *cb, struct neko_gui_buffer *b, enum neko_gui_command_clipping clip);
NEKO_GUI_LIB void neko_gui_command_buffer_reset(struct neko_gui_command_buffer *b);
NEKO_GUI_LIB void *neko_gui_command_buffer_push(struct neko_gui_command_buffer *b, enum neko_gui_command_type t, neko_gui_size size);
NEKO_GUI_LIB void neko_gui_draw_symbol(struct neko_gui_command_buffer *out, enum neko_gui_symbol_type type, struct neko_gui_rect content, struct neko_gui_color background,
                                       struct neko_gui_color foreground, float border_width, const struct neko_gui_user_font *font);

NEKO_GUI_LIB void neko_gui_start_buffer(struct neko_gui_context *ctx, struct neko_gui_command_buffer *b);
NEKO_GUI_LIB void neko_gui_start(struct neko_gui_context *ctx, struct neko_gui_window *win);
NEKO_GUI_LIB void neko_gui_start_popup(struct neko_gui_context *ctx, struct neko_gui_window *win);
NEKO_GUI_LIB void neko_gui_finish_popup(struct neko_gui_context *ctx, struct neko_gui_window *);
NEKO_GUI_LIB void neko_gui_finish_buffer(struct neko_gui_context *ctx, struct neko_gui_command_buffer *b);
NEKO_GUI_LIB void neko_gui_finish(struct neko_gui_context *ctx, struct neko_gui_window *w);
NEKO_GUI_LIB void neko_gui_build(struct neko_gui_context *ctx);

NEKO_GUI_LIB void neko_gui_textedit_clear_state(struct neko_gui_text_edit *state, enum neko_gui_text_edit_type type, neko_gui_plugin_filter filter);
NEKO_GUI_LIB void neko_gui_textedit_click(struct neko_gui_text_edit *state, float x, float y, const struct neko_gui_user_font *font, float row_height);
NEKO_GUI_LIB void neko_gui_textedit_drag(struct neko_gui_text_edit *state, float x, float y, const struct neko_gui_user_font *font, float row_height);
NEKO_GUI_LIB void neko_gui_textedit_key(struct neko_gui_text_edit *state, enum neko_gui_keys key, int shift_mod, const struct neko_gui_user_font *font, float row_height);

enum neko_gui_window_insert_location { NEKO_GUI_INSERT_BACK, NEKO_GUI_INSERT_FRONT };
NEKO_GUI_LIB void *neko_gui_create_window(struct neko_gui_context *ctx);
NEKO_GUI_LIB void neko_gui_remove_window(struct neko_gui_context *, struct neko_gui_window *);
NEKO_GUI_LIB void neko_gui_free_window(struct neko_gui_context *ctx, struct neko_gui_window *win);
NEKO_GUI_LIB struct neko_gui_window *neko_gui_find_window(struct neko_gui_context *ctx, neko_gui_hash hash, const char *name);
NEKO_GUI_LIB void neko_gui_insert_window(struct neko_gui_context *ctx, struct neko_gui_window *win, enum neko_gui_window_insert_location loc);

NEKO_GUI_LIB void neko_gui_pool_init(struct neko_gui_pool *pool, struct neko_gui_allocator *alloc, unsigned int capacity);
NEKO_GUI_LIB void neko_gui_pool_free(struct neko_gui_pool *pool);
NEKO_GUI_LIB void neko_gui_pool_init_fixed(struct neko_gui_pool *pool, void *memory, neko_gui_size size);
NEKO_GUI_LIB struct neko_gui_page_element *neko_gui_pool_alloc(struct neko_gui_pool *pool);

NEKO_GUI_LIB struct neko_gui_page_element *neko_gui_create_page_element(struct neko_gui_context *ctx);
NEKO_GUI_LIB void neko_gui_lineko_gui_page_element_into_freelist(struct neko_gui_context *ctx, struct neko_gui_page_element *elem);
NEKO_GUI_LIB void neko_gui_free_page_element(struct neko_gui_context *ctx, struct neko_gui_page_element *elem);

NEKO_GUI_LIB struct neko_gui_table *neko_gui_create_table(struct neko_gui_context *ctx);
NEKO_GUI_LIB void neko_gui_remove_table(struct neko_gui_window *win, struct neko_gui_table *tbl);
NEKO_GUI_LIB void neko_gui_free_table(struct neko_gui_context *ctx, struct neko_gui_table *tbl);
NEKO_GUI_LIB void neko_gui_push_table(struct neko_gui_window *win, struct neko_gui_table *tbl);
NEKO_GUI_LIB neko_gui_uint *neko_gui_add_value(struct neko_gui_context *ctx, struct neko_gui_window *win, neko_gui_hash name, neko_gui_uint value);
NEKO_GUI_LIB neko_gui_uint *neko_gui_find_value(struct neko_gui_window *win, neko_gui_hash name);

NEKO_GUI_LIB void *neko_gui_create_panel(struct neko_gui_context *ctx);
NEKO_GUI_LIB void neko_gui_free_panel(struct neko_gui_context *, struct neko_gui_panel *pan);
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_has_header(neko_gui_flags flags, const char *title);
NEKO_GUI_LIB struct neko_gui_vec2 neko_gui_panel_get_padding(const struct neko_gui_style *style, enum neko_gui_panel_type type);
NEKO_GUI_LIB float neko_gui_panel_get_border(const struct neko_gui_style *style, neko_gui_flags flags, enum neko_gui_panel_type type);
NEKO_GUI_LIB struct neko_gui_color neko_gui_panel_get_border_color(const struct neko_gui_style *style, enum neko_gui_panel_type type);
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_is_sub(enum neko_gui_panel_type type);
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_is_nonblock(enum neko_gui_panel_type type);
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_begin(struct neko_gui_context *ctx, const char *title, enum neko_gui_panel_type panel_type);
NEKO_GUI_LIB void neko_gui_panel_end(struct neko_gui_context *ctx);

NEKO_GUI_LIB float neko_gui_layout_row_calculate_usable_space(const struct neko_gui_style *style, enum neko_gui_panel_type type, float total_space, int columns);
NEKO_GUI_LIB void neko_gui_panel_layout(const struct neko_gui_context *ctx, struct neko_gui_window *win, float height, int cols);
NEKO_GUI_LIB void neko_gui_row_layout(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float height, int cols, int width);
NEKO_GUI_LIB void neko_gui_panel_alloc_row(const struct neko_gui_context *ctx, struct neko_gui_window *win);
NEKO_GUI_LIB void neko_gui_layout_widget_space(struct neko_gui_rect *bounds, const struct neko_gui_context *ctx, struct neko_gui_window *win, int modify);
NEKO_GUI_LIB void neko_gui_panel_alloc_space(struct neko_gui_rect *bounds, const struct neko_gui_context *ctx);
NEKO_GUI_LIB void neko_gui_layout_peek(struct neko_gui_rect *bounds, struct neko_gui_context *ctx);

NEKO_GUI_LIB neko_gui_bool neko_gui_nonblock_begin(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_rect body, struct neko_gui_rect header, enum neko_gui_panel_type panel_type);

struct neko_gui_text {
    struct neko_gui_vec2 padding;
    struct neko_gui_color background;
    struct neko_gui_color text;
};
NEKO_GUI_LIB void neko_gui_widget_text(struct neko_gui_command_buffer *o, struct neko_gui_rect b, const char *string, int len, const struct neko_gui_text *t, neko_gui_flags a,
                                       const struct neko_gui_user_font *f);
NEKO_GUI_LIB void neko_gui_widget_text_wrap(struct neko_gui_command_buffer *o, struct neko_gui_rect b, const char *string, int len, const struct neko_gui_text *t, const struct neko_gui_user_font *f);

NEKO_GUI_LIB neko_gui_bool neko_gui_button_behavior(neko_gui_flags *state, struct neko_gui_rect r, const struct neko_gui_input *i, enum neko_gui_button_behavior behavior);
NEKO_GUI_LIB const struct neko_gui_style_item *neko_gui_draw_button(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, neko_gui_flags state,
                                                                    const struct neko_gui_style_button *style);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect r, const struct neko_gui_style_button *style,
                                              const struct neko_gui_input *in, enum neko_gui_button_behavior behavior, struct neko_gui_rect *content);
NEKO_GUI_LIB void neko_gui_draw_button_text(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                            const struct neko_gui_style_button *style, const char *txt, int len, neko_gui_flags text_alignment, const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *string, int len, neko_gui_flags align,
                                                   enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_input *in,
                                                   const struct neko_gui_user_font *font);
NEKO_GUI_LIB void neko_gui_draw_button_symbol(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                              const struct neko_gui_style_button *style, enum neko_gui_symbol_type type, const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_symbol(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, enum neko_gui_symbol_type symbol,
                                                     enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_input *in,
                                                     const struct neko_gui_user_font *font);
NEKO_GUI_LIB void neko_gui_draw_button_image(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                             const struct neko_gui_style_button *style, const struct neko_gui_image *img);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, struct neko_gui_image img, enum neko_gui_button_behavior b,
                                                    const struct neko_gui_style_button *style, const struct neko_gui_input *in);
NEKO_GUI_LIB void neko_gui_draw_button_text_symbol(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label, const struct neko_gui_rect *symbol,
                                                   neko_gui_flags state, const struct neko_gui_style_button *style, const char *str, int len, enum neko_gui_symbol_type type,
                                                   const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text_symbol(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, enum neko_gui_symbol_type symbol, const char *str,
                                                          int len, neko_gui_flags align, enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style,
                                                          const struct neko_gui_user_font *font, const struct neko_gui_input *in);
NEKO_GUI_LIB void neko_gui_draw_button_text_image(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label, const struct neko_gui_rect *image,
                                                  neko_gui_flags state, const struct neko_gui_style_button *style, const char *str, int len, const struct neko_gui_user_font *font,
                                                  const struct neko_gui_image *img);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, struct neko_gui_image img, const char *str, int len,
                                                         neko_gui_flags align, enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_user_font *font,
                                                         const struct neko_gui_input *in);

enum neko_gui_toggle_type { NEKO_GUI_TOGGLE_CHECK, NEKO_GUI_TOGGLE_OPTION };
NEKO_GUI_LIB neko_gui_bool neko_gui_toggle_behavior(const struct neko_gui_input *in, struct neko_gui_rect select, neko_gui_flags *state, neko_gui_bool active);
NEKO_GUI_LIB void neko_gui_draw_checkbox(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_toggle *style, neko_gui_bool active, const struct neko_gui_rect *label,
                                         const struct neko_gui_rect *selector, const struct neko_gui_rect *cursors, const char *string, int len, const struct neko_gui_user_font *font);
NEKO_GUI_LIB void neko_gui_draw_option(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_toggle *style, neko_gui_bool active, const struct neko_gui_rect *label,
                                       const struct neko_gui_rect *selector, const struct neko_gui_rect *cursors, const char *string, int len, const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_toggle(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect r, neko_gui_bool *active, const char *str, int len,
                                              enum neko_gui_toggle_type type, const struct neko_gui_style_toggle *style, const struct neko_gui_input *in, const struct neko_gui_user_font *font);

NEKO_GUI_LIB neko_gui_size neko_gui_progress_behavior(neko_gui_flags *state, struct neko_gui_input *in, struct neko_gui_rect r, struct neko_gui_rect cursor, neko_gui_size max, neko_gui_size value,
                                                      neko_gui_bool modifiable);
NEKO_GUI_LIB void neko_gui_draw_progress(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_progress *style, const struct neko_gui_rect *bounds,
                                         const struct neko_gui_rect *scursor, neko_gui_size value, neko_gui_size max);
NEKO_GUI_LIB neko_gui_size neko_gui_do_progress(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, neko_gui_size value, neko_gui_size max,
                                                neko_gui_bool modifiable, const struct neko_gui_style_progress *style, struct neko_gui_input *in);

NEKO_GUI_LIB float neko_gui_slider_behavior(neko_gui_flags *state, struct neko_gui_rect *logical_cursor, struct neko_gui_rect *visual_cursor, struct neko_gui_input *in, struct neko_gui_rect bounds,
                                            float slider_min, float slider_max, float slider_value, float slider_step, float slider_steps);
NEKO_GUI_LIB void neko_gui_draw_slider(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_slider *style, const struct neko_gui_rect *bounds,
                                       const struct neko_gui_rect *visual_cursor, float min, float value, float max);
NEKO_GUI_LIB float neko_gui_do_slider(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, float min, float val, float max, float step,
                                      const struct neko_gui_style_slider *style, struct neko_gui_input *in, const struct neko_gui_user_font *font);

NEKO_GUI_LIB float neko_gui_scrollbar_behavior(neko_gui_flags *state, struct neko_gui_input *in, int has_scrolling, const struct neko_gui_rect *scroll, const struct neko_gui_rect *cursor,
                                               const struct neko_gui_rect *empty0, const struct neko_gui_rect *empty1, float scroll_offset, float target, float scroll_step,
                                               enum neko_gui_orientation o);
NEKO_GUI_LIB void neko_gui_draw_scrollbar(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_scrollbar *style, const struct neko_gui_rect *bounds,
                                          const struct neko_gui_rect *scroll);
NEKO_GUI_LIB float neko_gui_do_scrollbarv(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect scroll, int has_scrolling, float offset, float target, float step,
                                          float button_pixel_inc, const struct neko_gui_style_scrollbar *style, struct neko_gui_input *in, const struct neko_gui_user_font *font);
NEKO_GUI_LIB float neko_gui_do_scrollbarh(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect scroll, int has_scrolling, float offset, float target, float step,
                                          float button_pixel_inc, const struct neko_gui_style_scrollbar *style, struct neko_gui_input *in, const struct neko_gui_user_font *font);

NEKO_GUI_LIB void neko_gui_draw_selectable(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_selectable *style, neko_gui_bool active,
                                           const struct neko_gui_rect *bounds, const struct neko_gui_rect *icon, const struct neko_gui_image *img, enum neko_gui_symbol_type sym, const char *string,
                                           int len, neko_gui_flags align, const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_selectable(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *str, int len, neko_gui_flags align,
                                                  neko_gui_bool *value, const struct neko_gui_style_selectable *style, const struct neko_gui_input *in, const struct neko_gui_user_font *font);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_selectable_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *str, int len, neko_gui_flags align,
                                                        neko_gui_bool *value, const struct neko_gui_image *img, const struct neko_gui_style_selectable *style, const struct neko_gui_input *in,
                                                        const struct neko_gui_user_font *font);

NEKO_GUI_LIB void neko_gui_edit_draw_text(struct neko_gui_command_buffer *out, const struct neko_gui_style_edit *style, float pos_x, float pos_y, float x_offset, const char *text, int byte_len,
                                          float row_height, const struct neko_gui_user_font *font, struct neko_gui_color background, struct neko_gui_color foreground, neko_gui_bool is_selected);
NEKO_GUI_LIB neko_gui_flags neko_gui_do_edit(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, neko_gui_flags flags, neko_gui_plugin_filter filter,
                                             struct neko_gui_text_edit *edit, const struct neko_gui_style_edit *style, struct neko_gui_input *in, const struct neko_gui_user_font *font);

NEKO_GUI_LIB neko_gui_bool neko_gui_color_picker_behavior(neko_gui_flags *state, const struct neko_gui_rect *bounds, const struct neko_gui_rect *matrix, const struct neko_gui_rect *hue_bar,
                                                          const struct neko_gui_rect *alpha_bar, struct neko_gui_colorf *color, const struct neko_gui_input *in);
NEKO_GUI_LIB void neko_gui_draw_color_picker(struct neko_gui_command_buffer *o, const struct neko_gui_rect *matrix, const struct neko_gui_rect *hue_bar, const struct neko_gui_rect *alpha_bar,
                                             struct neko_gui_colorf col);
NEKO_GUI_LIB neko_gui_bool neko_gui_do_color_picker(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_colorf *col, enum neko_gui_color_format fmt,
                                                    struct neko_gui_rect bounds, struct neko_gui_vec2 padding, const struct neko_gui_input *in, const struct neko_gui_user_font *font);

enum neko_gui_property_status { NEKO_GUI_PROPERTY_DEFAULT, NEKO_GUI_PROPERTY_EDIT, NEKO_GUI_PROPERTY_DRAG };
enum neko_gui_property_filter { NEKO_GUI_FILTER_INT, NEKO_GUI_FILTER_FLOAT };
enum neko_gui_property_kind { NEKO_GUI_PROPERTY_INT, NEKO_GUI_PROPERTY_FLOAT, NEKO_GUI_PROPERTY_DOUBLE };
union neko_gui_property {
    int i;
    float f;
    double d;
};
struct neko_gui_property_variant {
    enum neko_gui_property_kind kind;
    union neko_gui_property value;
    union neko_gui_property min_value;
    union neko_gui_property max_value;
    union neko_gui_property step;
};
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_int(int value, int min_value, int max_value, int step);
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_float(float value, float min_value, float max_value, float step);
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_double(double value, double min_value, double max_value, double step);

NEKO_GUI_LIB void neko_gui_drag_behavior(neko_gui_flags *state, const struct neko_gui_input *in, struct neko_gui_rect drag, struct neko_gui_property_variant *variant, float inc_per_pixel);
NEKO_GUI_LIB void neko_gui_property_behavior(neko_gui_flags *ws, const struct neko_gui_input *in, struct neko_gui_rect property, struct neko_gui_rect label, struct neko_gui_rect edit,
                                             struct neko_gui_rect empty, int *state, struct neko_gui_property_variant *variant, float inc_per_pixel);
NEKO_GUI_LIB void neko_gui_draw_property(struct neko_gui_command_buffer *out, const struct neko_gui_style_property *style, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label,
                                         neko_gui_flags state, const char *name, int len, const struct neko_gui_user_font *font);
NEKO_GUI_LIB void neko_gui_do_property(neko_gui_flags *ws, struct neko_gui_command_buffer *out, struct neko_gui_rect property, const char *name, struct neko_gui_property_variant *variant,
                                       float inc_per_pixel, char *buffer, int *len, int *state, int *cursor, int *select_begin, int *select_end, const struct neko_gui_style_property *style,
                                       enum neko_gui_property_filter filter, struct neko_gui_input *in, const struct neko_gui_user_font *font, struct neko_gui_text_edit *text_edit,
                                       enum neko_gui_button_behavior behavior);
NEKO_GUI_LIB void neko_gui_property(struct neko_gui_context *ctx, const char *name, struct neko_gui_property_variant *variant, float inc_per_pixel, const enum neko_gui_property_filter filter);

#ifdef NEKO_GUI_INCLUDE_FONT_BAKING

#ifndef STBTT_malloc
static void *neko_gui_stbtt_malloc(neko_gui_size size, void *user_data) {
    struct neko_gui_allocator *alloc = (struct neko_gui_allocator *)user_data;
    return alloc->alloc(alloc->userdata, 0, size);
}

static void neko_gui_stbtt_free(void *ptr, void *user_data) {
    struct neko_gui_allocator *alloc = (struct neko_gui_allocator *)user_data;
    alloc->free(alloc->userdata, ptr);
}

// #define STBTT_malloc(x, u) neko_gui_stbtt_malloc(x, u)
// #define STBTT_free(x, u) neko_gui_stbtt_free(x, u)

#endif

#endif

#endif

#ifndef NEKO_GUI_INV_SQRT
#define NEKO_GUI_INV_SQRT neko_gui_inv_sqrt
NEKO_GUI_LIB float neko_gui_inv_sqrt(float n) {
    float x2;
    const float threehalfs = 1.5f;
    union {
        neko_gui_uint i;
        float f;
    } conv = {0};
    conv.f = n;
    x2 = n * 0.5f;
    conv.i = 0x5f375A84 - (conv.i >> 1);
    conv.f = conv.f * (threehalfs - (x2 * conv.f * conv.f));
    return conv.f;
}
#endif
#ifndef NEKO_GUI_SIN
#define NEKO_GUI_SIN neko_gui_sin
NEKO_GUI_LIB float neko_gui_sin(float x) {
    NEKO_GUI_STORAGE const float a0 = +1.91059300966915117e-31f;
    NEKO_GUI_STORAGE const float a1 = +1.00086760103908896f;
    NEKO_GUI_STORAGE const float a2 = -1.21276126894734565e-2f;
    NEKO_GUI_STORAGE const float a3 = -1.38078780785773762e-1f;
    NEKO_GUI_STORAGE const float a4 = -2.67353392911981221e-2f;
    NEKO_GUI_STORAGE const float a5 = +2.08026600266304389e-2f;
    NEKO_GUI_STORAGE const float a6 = -3.03996055049204407e-3f;
    NEKO_GUI_STORAGE const float a7 = +1.38235642404333740e-4f;
    return a0 + x * (a1 + x * (a2 + x * (a3 + x * (a4 + x * (a5 + x * (a6 + x * a7))))));
}
#endif
#ifndef NEKO_GUI_COS
#define NEKO_GUI_COS neko_gui_cos
NEKO_GUI_LIB float neko_gui_cos(float x) {

    NEKO_GUI_STORAGE const float a0 = 9.9995999154986614e-1f;
    NEKO_GUI_STORAGE const float a1 = 1.2548995793001028e-3f;
    NEKO_GUI_STORAGE const float a2 = -5.0648546280678015e-1f;
    NEKO_GUI_STORAGE const float a3 = 1.2942246466519995e-2f;
    NEKO_GUI_STORAGE const float a4 = 2.8668384702547972e-2f;
    NEKO_GUI_STORAGE const float a5 = 7.3726485210586547e-3f;
    NEKO_GUI_STORAGE const float a6 = -3.8510875386947414e-3f;
    NEKO_GUI_STORAGE const float a7 = 4.7196604604366623e-4f;
    NEKO_GUI_STORAGE const float a8 = -1.8776444013090451e-5f;
    return a0 + x * (a1 + x * (a2 + x * (a3 + x * (a4 + x * (a5 + x * (a6 + x * (a7 + x * a8)))))));
}
#endif
NEKO_GUI_LIB neko_gui_uint neko_gui_round_up_pow2(neko_gui_uint v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
NEKO_GUI_LIB double neko_gui_pow(double x, int n) {

    double r = 1;
    int plus = n >= 0;
    n = (plus) ? n : -n;
    while (n > 0) {
        if ((n & 1) == 1) r *= x;
        n /= 2;
        x *= x;
    }
    return plus ? r : 1.0 / r;
}
NEKO_GUI_LIB int neko_gui_ifloord(double x) {
    x = (double)((int)x - ((x < 0.0) ? 1 : 0));
    return (int)x;
}
NEKO_GUI_LIB int neko_gui_ifloorf(float x) {
    x = (float)((int)x - ((x < 0.0f) ? 1 : 0));
    return (int)x;
}
NEKO_GUI_LIB int neko_gui_iceilf(float x) {
    if (x >= 0) {
        int i = (int)x;
        return (x > i) ? i + 1 : i;
    } else {
        int t = (int)x;
        float r = x - (float)t;
        return (r > 0.0f) ? t + 1 : t;
    }
}
NEKO_GUI_LIB int neko_gui_log10(double n) {
    int neg;
    int ret;
    int exp = 0;

    neg = (n < 0) ? 1 : 0;
    ret = (neg) ? (int)-n : (int)n;
    while ((ret / 10) > 0) {
        ret /= 10;
        exp++;
    }
    if (neg) exp = -exp;
    return exp;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_get_null_rect(void) { return neko_gui_null_rect; }
NEKO_GUI_API struct neko_gui_rect neko_gui_rect(float x, float y, float w, float h) {
    struct neko_gui_rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_recti(int x, int y, int w, int h) {
    struct neko_gui_rect r;
    r.x = (float)x;
    r.y = (float)y;
    r.w = (float)w;
    r.h = (float)h;
    return r;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_recta(struct neko_gui_vec2 pos, struct neko_gui_vec2 size) { return neko_gui_rect(pos.x, pos.y, size.x, size.y); }
NEKO_GUI_API struct neko_gui_rect neko_gui_rectv(const float *r) { return neko_gui_rect(r[0], r[1], r[2], r[3]); }
NEKO_GUI_API struct neko_gui_rect neko_gui_rectiv(const int *r) { return neko_gui_recti(r[0], r[1], r[2], r[3]); }
NEKO_GUI_API struct neko_gui_vec2 neko_gui_rect_pos(struct neko_gui_rect r) {
    struct neko_gui_vec2 ret;
    ret.x = r.x;
    ret.y = r.y;
    return ret;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_rect_size(struct neko_gui_rect r) {
    struct neko_gui_vec2 ret;
    ret.x = r.w;
    ret.y = r.h;
    return ret;
}
NEKO_GUI_LIB struct neko_gui_rect neko_gui_shrineko_gui_rect(struct neko_gui_rect r, float amount) {
    struct neko_gui_rect res;
    r.w = NEKO_GUI_MAX(r.w, 2 * amount);
    r.h = NEKO_GUI_MAX(r.h, 2 * amount);
    res.x = r.x + amount;
    res.y = r.y + amount;
    res.w = r.w - 2 * amount;
    res.h = r.h - 2 * amount;
    return res;
}
NEKO_GUI_LIB struct neko_gui_rect neko_gui_pad_rect(struct neko_gui_rect r, struct neko_gui_vec2 pad) {
    r.w = NEKO_GUI_MAX(r.w, 2 * pad.x);
    r.h = NEKO_GUI_MAX(r.h, 2 * pad.y);
    r.x += pad.x;
    r.y += pad.y;
    r.w -= 2 * pad.x;
    r.h -= 2 * pad.y;
    return r;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2(float x, float y) {
    struct neko_gui_vec2 ret;
    ret.x = x;
    ret.y = y;
    return ret;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2i(int x, int y) {
    struct neko_gui_vec2 ret;
    ret.x = (float)x;
    ret.y = (float)y;
    return ret;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2v(const float *v) { return neko_gui_vec2(v[0], v[1]); }
NEKO_GUI_API struct neko_gui_vec2 neko_gui_vec2iv(const int *v) { return neko_gui_vec2i(v[0], v[1]); }
NEKO_GUI_LIB void neko_gui_unify(struct neko_gui_rect *clip, const struct neko_gui_rect *a, float x0, float y0, float x1, float y1) {
    NEKO_GUI_ASSERT(a);
    NEKO_GUI_ASSERT(clip);
    clip->x = NEKO_GUI_MAX(a->x, x0);
    clip->y = NEKO_GUI_MAX(a->y, y0);
    clip->w = NEKO_GUI_MIN(a->x + a->w, x1) - clip->x;
    clip->h = NEKO_GUI_MIN(a->y + a->h, y1) - clip->y;
    clip->w = NEKO_GUI_MAX(0, clip->w);
    clip->h = NEKO_GUI_MAX(0, clip->h);
}

NEKO_GUI_API void neko_gui_triangle_from_direction(struct neko_gui_vec2 *result, struct neko_gui_rect r, float pad_x, float pad_y, enum neko_gui_heading direction) {
    float w_half, h_half;
    NEKO_GUI_ASSERT(result);

    r.w = NEKO_GUI_MAX(2 * pad_x, r.w);
    r.h = NEKO_GUI_MAX(2 * pad_y, r.h);
    r.w = r.w - 2 * pad_x;
    r.h = r.h - 2 * pad_y;

    r.x = r.x + pad_x;
    r.y = r.y + pad_y;

    w_half = r.w / 2.0f;
    h_half = r.h / 2.0f;

    if (direction == NEKO_GUI_UP) {
        result[0] = neko_gui_vec2(r.x + w_half, r.y);
        result[1] = neko_gui_vec2(r.x + r.w, r.y + r.h);
        result[2] = neko_gui_vec2(r.x, r.y + r.h);
    } else if (direction == NEKO_GUI_RIGHT) {
        result[0] = neko_gui_vec2(r.x, r.y);
        result[1] = neko_gui_vec2(r.x + r.w, r.y + h_half);
        result[2] = neko_gui_vec2(r.x, r.y + r.h);
    } else if (direction == NEKO_GUI_DOWN) {
        result[0] = neko_gui_vec2(r.x, r.y);
        result[1] = neko_gui_vec2(r.x + r.w, r.y);
        result[2] = neko_gui_vec2(r.x + w_half, r.y + r.h);
    } else {
        result[0] = neko_gui_vec2(r.x, r.y + h_half);
        result[1] = neko_gui_vec2(r.x + r.w, r.y);
        result[2] = neko_gui_vec2(r.x + r.w, r.y + r.h);
    }
}

NEKO_GUI_INTERN int neko_gui_str_match_here(const char *regexp, const char *text);
NEKO_GUI_INTERN int neko_gui_str_match_star(int c, const char *regexp, const char *text);
NEKO_GUI_LIB neko_gui_bool neko_gui_is_lower(int c) { return (c >= 'a' && c <= 'z') || (c >= 0xE0 && c <= 0xFF); }
NEKO_GUI_LIB neko_gui_bool neko_gui_is_upper(int c) { return (c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDF); }
NEKO_GUI_LIB int neko_gui_to_upper(int c) { return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c; }
NEKO_GUI_LIB int neko_gui_to_lower(int c) { return (c >= 'A' && c <= 'Z') ? (c - ('a' + 'A')) : c; }

#ifndef NEKO_GUI_MEMCPY
#define NEKO_GUI_MEMCPY neko_gui_memcopy
NEKO_GUI_LIB void *neko_gui_memcopy(void *dst0, const void *src0, neko_gui_size length) {
    neko_gui_ptr t;
    char *dst = (char *)dst0;
    const char *src = (const char *)src0;
    if (length == 0 || dst == src) goto done;

#define neko_gui_word int
#define neko_gui_wsize sizeof(neko_gui_word)
#define neko_gui_wmask (neko_gui_wsize - 1)
#define NEKO_GUI_TLOOP(s) \
    if (t) NEKO_GUI_TLOOP1(s)
#define NEKO_GUI_TLOOP1(s) \
    do {                   \
        s;                 \
    } while (--t)

    if (dst < src) {
        t = (neko_gui_ptr)src;
        if ((t | (neko_gui_ptr)dst) & neko_gui_wmask) {
            if ((t ^ (neko_gui_ptr)dst) & neko_gui_wmask || length < neko_gui_wsize)
                t = length;
            else
                t = neko_gui_wsize - (t & neko_gui_wmask);
            length -= t;
            NEKO_GUI_TLOOP1(*dst++ = *src++);
        }
        t = length / neko_gui_wsize;
        NEKO_GUI_TLOOP(*(neko_gui_word *)(void *)dst = *(const neko_gui_word *)(const void *)src; src += neko_gui_wsize; dst += neko_gui_wsize);
        t = length & neko_gui_wmask;
        NEKO_GUI_TLOOP(*dst++ = *src++);
    } else {
        src += length;
        dst += length;
        t = (neko_gui_ptr)src;
        if ((t | (neko_gui_ptr)dst) & neko_gui_wmask) {
            if ((t ^ (neko_gui_ptr)dst) & neko_gui_wmask || length <= neko_gui_wsize)
                t = length;
            else
                t &= neko_gui_wmask;
            length -= t;
            NEKO_GUI_TLOOP1(*--dst = *--src);
        }
        t = length / neko_gui_wsize;
        NEKO_GUI_TLOOP(src -= neko_gui_wsize; dst -= neko_gui_wsize; *(neko_gui_word *)(void *)dst = *(const neko_gui_word *)(const void *)src);
        t = length & neko_gui_wmask;
        NEKO_GUI_TLOOP(*--dst = *--src);
    }
#undef neko_gui_word
#undef neko_gui_wsize
#undef neko_gui_wmask
#undef NEKO_GUI_TLOOP
#undef NEKO_GUI_TLOOP1
done:
    return (dst0);
}
#endif

NEKO_GUI_LIB void neko_gui_zero(void *ptr, neko_gui_size size) {
    NEKO_GUI_ASSERT(ptr);
    NEKO_GUI_MEMSET(ptr, 0, size);
}
NEKO_GUI_API int neko_gui_strlen(const char *str) {
    int siz = 0;
    NEKO_GUI_ASSERT(str);
    while (str && *str++ != '\0') siz++;
    return siz;
}
NEKO_GUI_API int neko_gui_strtoi(const char *str, const char **endptr) {
    int neg = 1;
    const char *p = str;
    int value = 0;

    NEKO_GUI_ASSERT(str);
    if (!str) return 0;

    while (*p == ' ') p++;
    if (*p == '-') {
        neg = -1;
        p++;
    }
    while (*p && *p >= '0' && *p <= '9') {
        value = value * 10 + (int)(*p - '0');
        p++;
    }
    if (endptr) *endptr = p;
    return neg * value;
}
NEKO_GUI_API double neko_gui_strtod(const char *str, const char **endptr) {
    double m;
    double neg = 1.0;
    const char *p = str;
    double value = 0;
    double number = 0;

    NEKO_GUI_ASSERT(str);
    if (!str) return 0;

    while (*p == ' ') p++;
    if (*p == '-') {
        neg = -1.0;
        p++;
    }

    while (*p && *p != '.' && *p != 'e') {
        value = value * 10.0 + (double)(*p - '0');
        p++;
    }

    if (*p == '.') {
        p++;
        for (m = 0.1; *p && *p != 'e'; p++) {
            value = value + (double)(*p - '0') * m;
            m *= 0.1;
        }
    }
    if (*p == 'e') {
        int i, pow, div;
        p++;
        if (*p == '-') {
            div = neko_gui_true;
            p++;
        } else if (*p == '+') {
            div = neko_gui_false;
            p++;
        } else
            div = neko_gui_false;

        for (pow = 0; *p; p++) pow = pow * 10 + (int)(*p - '0');

        for (m = 1.0, i = 0; i < pow; i++) m *= 10.0;

        if (div)
            value /= m;
        else
            value *= m;
    }
    number = value * neg;
    if (endptr) *endptr = p;
    return number;
}
NEKO_GUI_API float neko_gui_strtof(const char *str, const char **endptr) {
    float float_value;
    double double_value;
    double_value = NEKO_GUI_STRTOD(str, endptr);
    float_value = (float)double_value;
    return float_value;
}
NEKO_GUI_API int neko_gui_stricmp(const char *s1, const char *s2) {
    neko_gui_int c1, c2, d;
    do {
        c1 = *s1++;
        c2 = *s2++;
        d = c1 - c2;
        while (d) {
            if (c1 <= 'Z' && c1 >= 'A') {
                d += ('a' - 'A');
                if (!d) break;
            }
            if (c2 <= 'Z' && c2 >= 'A') {
                d -= ('a' - 'A');
                if (!d) break;
            }
            return ((d >= 0) << 1) - 1;
        }
    } while (c1);
    return 0;
}
NEKO_GUI_API int neko_gui_stricmpn(const char *s1, const char *s2, int n) {
    int c1, c2, d;
    NEKO_GUI_ASSERT(n >= 0);
    do {
        c1 = *s1++;
        c2 = *s2++;
        if (!n--) return 0;

        d = c1 - c2;
        while (d) {
            if (c1 <= 'Z' && c1 >= 'A') {
                d += ('a' - 'A');
                if (!d) break;
            }
            if (c2 <= 'Z' && c2 >= 'A') {
                d -= ('a' - 'A');
                if (!d) break;
            }
            return ((d >= 0) << 1) - 1;
        }
    } while (c1);
    return 0;
}
NEKO_GUI_INTERN int neko_gui_str_match_here(const char *regexp, const char *text) {
    if (regexp[0] == '\0') return 1;
    if (regexp[1] == '*') return neko_gui_str_match_star(regexp[0], regexp + 2, text);
    if (regexp[0] == '$' && regexp[1] == '\0') return *text == '\0';
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)) return neko_gui_str_match_here(regexp + 1, text + 1);
    return 0;
}
NEKO_GUI_INTERN int neko_gui_str_match_star(int c, const char *regexp, const char *text) {
    do {
        if (neko_gui_str_match_here(regexp, text)) return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}
NEKO_GUI_API int neko_gui_strfilter(const char *text, const char *regexp) {

    if (regexp[0] == '^') return neko_gui_str_match_here(regexp + 1, text);
    do {
        if (neko_gui_str_match_here(regexp, text)) return 1;
    } while (*text++ != '\0');
    return 0;
}
NEKO_GUI_API int neko_gui_strmatch_fuzzy_text(const char *str, int str_len, const char *pattern, int *out_score) {

#define NEKO_GUI_ADJACENCY_BONUS 5

#define NEKO_GUI_SEPARATOR_BONUS 10

#define NEKO_GUI_CAMEL_BONUS 10

#define NEKO_GUI_LEADING_LETTER_PENALTY (-3)

#define NEKO_GUI_MAX_LEADING_LETTER_PENALTY (-9)

#define NEKO_GUI_UNMATCHED_LETTER_PENALTY (-1)

    int score = 0;
    char const *pattern_iter = pattern;
    int str_iter = 0;
    int prev_matched = neko_gui_false;
    int prev_lower = neko_gui_false;

    int prev_separator = neko_gui_true;

    char const *best_letter = 0;
    int best_letter_score = 0;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(pattern);
    if (!str || !str_len || !pattern) return 0;
    while (str_iter < str_len) {
        const char pattern_letter = *pattern_iter;
        const char str_letter = str[str_iter];

        int next_match = *pattern_iter != '\0' && neko_gui_to_lower(pattern_letter) == neko_gui_to_lower(str_letter);
        int rematch = best_letter && neko_gui_to_upper(*best_letter) == neko_gui_to_upper(str_letter);

        int advanced = next_match && best_letter;
        int pattern_repeat = best_letter && *pattern_iter != '\0';
        pattern_repeat = pattern_repeat && neko_gui_to_lower(*best_letter) == neko_gui_to_lower(pattern_letter);

        if (advanced || pattern_repeat) {
            score += best_letter_score;
            best_letter = 0;
            best_letter_score = 0;
        }

        if (next_match || rematch) {
            int new_score = 0;

            if (pattern_iter == pattern) {
                int count = (int)(&str[str_iter] - str);
                int penalty = NEKO_GUI_LEADING_LETTER_PENALTY * count;
                if (penalty < NEKO_GUI_MAX_LEADING_LETTER_PENALTY) penalty = NEKO_GUI_MAX_LEADING_LETTER_PENALTY;

                score += penalty;
            }

            if (prev_matched) new_score += NEKO_GUI_ADJACENCY_BONUS;

            if (prev_separator) new_score += NEKO_GUI_SEPARATOR_BONUS;

            if (prev_lower && neko_gui_is_upper(str_letter)) new_score += NEKO_GUI_CAMEL_BONUS;

            if (next_match) ++pattern_iter;

            if (new_score >= best_letter_score) {

                if (best_letter != 0) score += NEKO_GUI_UNMATCHED_LETTER_PENALTY;

                best_letter = &str[str_iter];
                best_letter_score = new_score;
            }
            prev_matched = neko_gui_true;
        } else {
            score += NEKO_GUI_UNMATCHED_LETTER_PENALTY;
            prev_matched = neko_gui_false;
        }

        prev_lower = neko_gui_is_lower(str_letter) != 0;
        prev_separator = str_letter == '_' || str_letter == ' ';

        ++str_iter;
    }

    if (best_letter) score += best_letter_score;

    if (*pattern_iter != '\0') return neko_gui_false;

    if (out_score) *out_score = score;
    return neko_gui_true;
}
NEKO_GUI_API int neko_gui_strmatch_fuzzy_string(char const *str, char const *pattern, int *out_score) { return neko_gui_strmatch_fuzzy_text(str, neko_gui_strlen(str), pattern, out_score); }
NEKO_GUI_LIB int neko_gui_string_float_limit(char *string, int prec) {
    int dot = 0;
    char *c = string;
    while (*c) {
        if (*c == '.') {
            dot = 1;
            c++;
            continue;
        }
        if (dot == (prec + 1)) {
            *c = 0;
            break;
        }
        if (dot > 0) dot++;
        c++;
    }
    return (int)(c - string);
}
NEKO_GUI_INTERN void neko_gui_strrev_ascii(char *s) {
    int len = neko_gui_strlen(s);
    int end = len / 2;
    int i = 0;
    char t;
    for (; i < end; ++i) {
        t = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = t;
    }
}
NEKO_GUI_LIB char *neko_gui_itoa(char *s, long n) {
    long i = 0;
    if (n == 0) {
        s[i++] = '0';
        s[i] = 0;
        return s;
    }
    if (n < 0) {
        s[i++] = '-';
        n = -n;
    }
    while (n > 0) {
        s[i++] = (char)('0' + (n % 10));
        n /= 10;
    }
    s[i] = 0;
    if (s[0] == '-') ++s;

    neko_gui_strrev_ascii(s);
    return s;
}
#ifndef NEKO_GUI_DTOA
#define NEKO_GUI_DTOA neko_gui_dtoa
NEKO_GUI_LIB char *neko_gui_dtoa(char *s, double n) {
    int useExp = 0;
    int digit = 0, m = 0, m1 = 0;
    char *c = s;
    int neg = 0;

    NEKO_GUI_ASSERT(s);
    if (!s) return 0;

    if (n == 0.0) {
        s[0] = '0';
        s[1] = '\0';
        return s;
    }

    neg = (n < 0);
    if (neg) n = -n;

    m = neko_gui_log10(n);
    useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
    if (neg) *(c++) = '-';

    if (useExp) {
        if (m < 0) m -= 1;
        n = n / (double)neko_gui_pow(10.0, m);
        m1 = m;
        m = 0;
    }
    if (m < 1.0) {
        m = 0;
    }

    while (n > NEKO_GUI_FLOAT_PRECISION || m >= 0) {
        double weight = neko_gui_pow(10.0, m);
        if (weight > 0) {
            double t = (double)n / weight;
            digit = neko_gui_ifloord(t);
            n -= ((double)digit * weight);
            *(c++) = (char)('0' + (char)digit);
        }
        if (m == 0 && n > 0) *(c++) = '.';
        m--;
    }

    if (useExp) {

        int i, j;
        *(c++) = 'e';
        if (m1 > 0) {
            *(c++) = '+';
        } else {
            *(c++) = '-';
            m1 = -m1;
        }
        m = 0;
        while (m1 > 0) {
            *(c++) = (char)('0' + (char)(m1 % 10));
            m1 /= 10;
            m++;
        }
        c -= m;
        for (i = 0, j = m - 1; i < j; i++, j--) {

            c[i] ^= c[j];
            c[j] ^= c[i];
            c[i] ^= c[j];
        }
        c += m;
    }
    *(c) = '\0';
    return s;
}
#endif
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
#ifndef NEKO_GUI_INCLUDE_STANDARD_IO
NEKO_GUI_INTERN int neko_gui_vsnprintf(char *buf, int buf_size, const char *fmt, va_list args) {
    enum neko_gui_arg_type { NEKO_GUI_ARG_TYPE_CHAR, NEKO_GUI_ARG_TYPE_SHORT, NEKO_GUI_ARG_TYPE_DEFAULT, NEKO_GUI_ARG_TYPE_LONG };
    enum neko_gui_arg_flags { NEKO_GUI_ARG_FLAG_LEFT = 0x01, NEKO_GUI_ARG_FLAG_PLUS = 0x02, NEKO_GUI_ARG_FLAG_SPACE = 0x04, NEKO_GUI_ARG_FLAG_NUM = 0x10, NEKO_GUI_ARG_FLAG_ZERO = 0x20 };

    char number_buffer[NEKO_GUI_MAX_NUMBER_BUFFER];
    enum neko_gui_arg_type arg_type = NEKO_GUI_ARG_TYPE_DEFAULT;
    int precision = NEKO_GUI_DEFAULT;
    int width = NEKO_GUI_DEFAULT;
    neko_gui_flags flag = 0;

    int len = 0;
    int result = -1;
    const char *iter = fmt;

    NEKO_GUI_ASSERT(buf);
    NEKO_GUI_ASSERT(buf_size);
    if (!buf || !buf_size || !fmt) return 0;
    for (iter = fmt; *iter && len < buf_size; iter++) {

        while (*iter && (*iter != '%') && (len < buf_size)) buf[len++] = *iter++;
        if (!(*iter) || len >= buf_size) break;
        iter++;

        while (*iter) {
            if (*iter == '-')
                flag |= NEKO_GUI_ARG_FLAG_LEFT;
            else if (*iter == '+')
                flag |= NEKO_GUI_ARG_FLAG_PLUS;
            else if (*iter == ' ')
                flag |= NEKO_GUI_ARG_FLAG_SPACE;
            else if (*iter == '#')
                flag |= NEKO_GUI_ARG_FLAG_NUM;
            else if (*iter == '0')
                flag |= NEKO_GUI_ARG_FLAG_ZERO;
            else
                break;
            iter++;
        }

        width = NEKO_GUI_DEFAULT;
        if (*iter >= '1' && *iter <= '9') {
            const char *end;
            width = neko_gui_strtoi(iter, &end);
            if (end == iter)
                width = -1;
            else
                iter = end;
        } else if (*iter == '*') {
            width = va_arg(args, int);
            iter++;
        }

        precision = NEKO_GUI_DEFAULT;
        if (*iter == '.') {
            iter++;
            if (*iter == '*') {
                precision = va_arg(args, int);
                iter++;
            } else {
                const char *end;
                precision = neko_gui_strtoi(iter, &end);
                if (end == iter)
                    precision = -1;
                else
                    iter = end;
            }
        }

        if (*iter == 'h') {
            if (*(iter + 1) == 'h') {
                arg_type = NEKO_GUI_ARG_TYPE_CHAR;
                iter++;
            } else
                arg_type = NEKO_GUI_ARG_TYPE_SHORT;
            iter++;
        } else if (*iter == 'l') {
            arg_type = NEKO_GUI_ARG_TYPE_LONG;
            iter++;
        } else
            arg_type = NEKO_GUI_ARG_TYPE_DEFAULT;

        if (*iter == '%') {
            NEKO_GUI_ASSERT(arg_type == NEKO_GUI_ARG_TYPE_DEFAULT);
            NEKO_GUI_ASSERT(precision == NEKO_GUI_DEFAULT);
            NEKO_GUI_ASSERT(width == NEKO_GUI_DEFAULT);
            if (len < buf_size) buf[len++] = '%';
        } else if (*iter == 's') {

            const char *str = va_arg(args, const char *);
            NEKO_GUI_ASSERT(str != buf && "buffer and argument are not allowed to overlap!");
            NEKO_GUI_ASSERT(arg_type == NEKO_GUI_ARG_TYPE_DEFAULT);
            NEKO_GUI_ASSERT(precision == NEKO_GUI_DEFAULT);
            NEKO_GUI_ASSERT(width == NEKO_GUI_DEFAULT);
            if (str == buf) return -1;
            while (str && *str && len < buf_size) buf[len++] = *str++;
        } else if (*iter == 'n') {

            signed int *n = va_arg(args, int *);
            NEKO_GUI_ASSERT(arg_type == NEKO_GUI_ARG_TYPE_DEFAULT);
            NEKO_GUI_ASSERT(precision == NEKO_GUI_DEFAULT);
            NEKO_GUI_ASSERT(width == NEKO_GUI_DEFAULT);
            if (n) *n = len;
        } else if (*iter == 'c' || *iter == 'i' || *iter == 'd') {

            long value = 0;
            const char *num_iter;
            int num_len, num_print, padding;
            int cur_precision = NEKO_GUI_MAX(precision, 1);
            int cur_width = NEKO_GUI_MAX(width, 0);

            if (arg_type == NEKO_GUI_ARG_TYPE_CHAR)
                value = (signed char)va_arg(args, int);
            else if (arg_type == NEKO_GUI_ARG_TYPE_SHORT)
                value = (signed short)va_arg(args, int);
            else if (arg_type == NEKO_GUI_ARG_TYPE_LONG)
                value = va_arg(args, signed long);
            else if (*iter == 'c')
                value = (unsigned char)va_arg(args, int);
            else
                value = va_arg(args, signed int);

            neko_gui_itoa(number_buffer, value);
            num_len = neko_gui_strlen(number_buffer);
            padding = NEKO_GUI_MAX(cur_width - NEKO_GUI_MAX(cur_precision, num_len), 0);
            if ((flag & NEKO_GUI_ARG_FLAG_PLUS) || (flag & NEKO_GUI_ARG_FLAG_SPACE)) padding = NEKO_GUI_MAX(padding - 1, 0);

            if (!(flag & NEKO_GUI_ARG_FLAG_LEFT)) {
                while (padding-- > 0 && (len < buf_size)) {
                    if ((flag & NEKO_GUI_ARG_FLAG_ZERO) && (precision == NEKO_GUI_DEFAULT))
                        buf[len++] = '0';
                    else
                        buf[len++] = ' ';
                }
            }

            if ((flag & NEKO_GUI_ARG_FLAG_PLUS) && value >= 0 && len < buf_size)
                buf[len++] = '+';
            else if ((flag & NEKO_GUI_ARG_FLAG_SPACE) && value >= 0 && len < buf_size)
                buf[len++] = ' ';

            num_print = NEKO_GUI_MAX(cur_precision, num_len);
            while (precision && (num_print > num_len) && (len < buf_size)) {
                buf[len++] = '0';
                num_print--;
            }

            num_iter = number_buffer;
            while (precision && *num_iter && len < buf_size) buf[len++] = *num_iter++;

            if (flag & NEKO_GUI_ARG_FLAG_LEFT) {
                while ((padding-- > 0) && (len < buf_size)) buf[len++] = ' ';
            }
        } else if (*iter == 'o' || *iter == 'x' || *iter == 'X' || *iter == 'u') {

            unsigned long value = 0;
            int num_len = 0, num_print, padding = 0;
            int cur_precision = NEKO_GUI_MAX(precision, 1);
            int cur_width = NEKO_GUI_MAX(width, 0);
            unsigned int base = (*iter == 'o') ? 8 : (*iter == 'u') ? 10 : 16;

            const char *upper_output_format = "0123456789ABCDEF";
            const char *lower_output_format = "0123456789abcdef";
            const char *output_format = (*iter == 'x') ? lower_output_format : upper_output_format;

            if (arg_type == NEKO_GUI_ARG_TYPE_CHAR)
                value = (unsigned char)va_arg(args, int);
            else if (arg_type == NEKO_GUI_ARG_TYPE_SHORT)
                value = (unsigned short)va_arg(args, int);
            else if (arg_type == NEKO_GUI_ARG_TYPE_LONG)
                value = va_arg(args, unsigned long);
            else
                value = va_arg(args, unsigned int);

            do {

                int digit = output_format[value % base];
                if (num_len < NEKO_GUI_MAX_NUMBER_BUFFER) number_buffer[num_len++] = (char)digit;
                value /= base;
            } while (value > 0);

            num_print = NEKO_GUI_MAX(cur_precision, num_len);
            padding = NEKO_GUI_MAX(cur_width - NEKO_GUI_MAX(cur_precision, num_len), 0);
            if (flag & NEKO_GUI_ARG_FLAG_NUM) padding = NEKO_GUI_MAX(padding - 1, 0);

            if (!(flag & NEKO_GUI_ARG_FLAG_LEFT)) {
                while ((padding-- > 0) && (len < buf_size)) {
                    if ((flag & NEKO_GUI_ARG_FLAG_ZERO) && (precision == NEKO_GUI_DEFAULT))
                        buf[len++] = '0';
                    else
                        buf[len++] = ' ';
                }
            }

            if (num_print && (flag & NEKO_GUI_ARG_FLAG_NUM)) {
                if ((*iter == 'o') && (len < buf_size)) {
                    buf[len++] = '0';
                } else if ((*iter == 'x') && ((len + 1) < buf_size)) {
                    buf[len++] = '0';
                    buf[len++] = 'x';
                } else if ((*iter == 'X') && ((len + 1) < buf_size)) {
                    buf[len++] = '0';
                    buf[len++] = 'X';
                }
            }
            while (precision && (num_print > num_len) && (len < buf_size)) {
                buf[len++] = '0';
                num_print--;
            }

            while (num_len > 0) {
                if (precision && (len < buf_size)) buf[len++] = number_buffer[num_len - 1];
                num_len--;
            }

            if (flag & NEKO_GUI_ARG_FLAG_LEFT) {
                while ((padding-- > 0) && (len < buf_size)) buf[len++] = ' ';
            }
        } else if (*iter == 'f') {

            const char *num_iter;
            int cur_precision = (precision < 0) ? 6 : precision;
            int prefix, cur_width = NEKO_GUI_MAX(width, 0);
            double value = va_arg(args, double);
            int num_len = 0, frac_len = 0, dot = 0;
            int padding = 0;

            NEKO_GUI_ASSERT(arg_type == NEKO_GUI_ARG_TYPE_DEFAULT);
            NEKO_GUI_DTOA(number_buffer, value);
            num_len = neko_gui_strlen(number_buffer);

            num_iter = number_buffer;
            while (*num_iter && *num_iter != '.') num_iter++;

            prefix = (*num_iter == '.') ? (int)(num_iter - number_buffer) + 1 : 0;
            padding = NEKO_GUI_MAX(cur_width - (prefix + NEKO_GUI_MIN(cur_precision, num_len - prefix)), 0);
            if ((flag & NEKO_GUI_ARG_FLAG_PLUS) || (flag & NEKO_GUI_ARG_FLAG_SPACE)) padding = NEKO_GUI_MAX(padding - 1, 0);

            if (!(flag & NEKO_GUI_ARG_FLAG_LEFT)) {
                while (padding-- > 0 && (len < buf_size)) {
                    if (flag & NEKO_GUI_ARG_FLAG_ZERO)
                        buf[len++] = '0';
                    else
                        buf[len++] = ' ';
                }
            }

            num_iter = number_buffer;
            if ((flag & NEKO_GUI_ARG_FLAG_PLUS) && (value >= 0) && (len < buf_size))
                buf[len++] = '+';
            else if ((flag & NEKO_GUI_ARG_FLAG_SPACE) && (value >= 0) && (len < buf_size))
                buf[len++] = ' ';
            while (*num_iter) {
                if (dot) frac_len++;
                if (len < buf_size) buf[len++] = *num_iter;
                if (*num_iter == '.') dot = 1;
                if (frac_len >= cur_precision) break;
                num_iter++;
            }

            while (frac_len < cur_precision) {
                if (!dot && len < buf_size) {
                    buf[len++] = '.';
                    dot = 1;
                }
                if (len < buf_size) buf[len++] = '0';
                frac_len++;
            }

            if (flag & NEKO_GUI_ARG_FLAG_LEFT) {
                while ((padding-- > 0) && (len < buf_size)) buf[len++] = ' ';
            }
        } else {

            NEKO_GUI_ASSERT(0 && "specifier is not supported!");
            return result;
        }
    }
    buf[(len >= buf_size) ? (buf_size - 1) : len] = 0;
    result = (len >= buf_size) ? -1 : len;
    return result;
}
#endif
NEKO_GUI_LIB int neko_gui_strfmt(char *buf, int buf_size, const char *fmt, va_list args) {
    int result = -1;
    NEKO_GUI_ASSERT(buf);
    NEKO_GUI_ASSERT(buf_size);
    if (!buf || !buf_size || !fmt) return 0;
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
    result = NEKO_GUI_VSNPRINTF(buf, (neko_gui_size)buf_size, fmt, args);
    result = (result >= buf_size) ? -1 : result;
    buf[buf_size - 1] = 0;
#else
    result = neko_gui_vsnprintf(buf, buf_size, fmt, args);
#endif
    return result;
}
#endif
NEKO_GUI_API neko_gui_hash neko_gui_murmur_hash(const void *key, int len, neko_gui_hash seed) {

#define NEKO_GUI_ROTL(x, r) ((x) << (r) | ((x) >> (32 - r)))

    neko_gui_uint h1 = seed;
    neko_gui_uint k1;
    const neko_gui_byte *data = (const neko_gui_byte *)key;
    const neko_gui_byte *keyptr = data;
    neko_gui_byte *k1ptr;
    const int bsize = sizeof(k1);
    const int nblocks = len / 4;

    const neko_gui_uint c1 = 0xcc9e2d51;
    const neko_gui_uint c2 = 0x1b873593;
    const neko_gui_byte *tail;
    int i;

    if (!key) return 0;
    for (i = 0; i < nblocks; ++i, keyptr += bsize) {
        k1ptr = (neko_gui_byte *)&k1;
        k1ptr[0] = keyptr[0];
        k1ptr[1] = keyptr[1];
        k1ptr[2] = keyptr[2];
        k1ptr[3] = keyptr[3];

        k1 *= c1;
        k1 = NEKO_GUI_ROTL(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = NEKO_GUI_ROTL(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    tail = (const neko_gui_byte *)(data + nblocks * 4);
    k1 = 0;
    switch (len & 3) {
        case 3:
            k1 ^= (neko_gui_uint)(tail[2] << 16);
        case 2:
            k1 ^= (neko_gui_uint)(tail[1] << 8u);
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = NEKO_GUI_ROTL(k1, 15);
            k1 *= c2;
            h1 ^= k1;
            break;
        default:
            break;
    }

    h1 ^= (neko_gui_uint)len;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

#undef NEKO_GUI_ROTL
    return h1;
}
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
NEKO_GUI_LIB char *neko_gui_file_load(const char *path, neko_gui_size *siz, struct neko_gui_allocator *alloc) {
    char *buf;
    FILE *fd;
    long ret;

    NEKO_GUI_ASSERT(path);
    NEKO_GUI_ASSERT(siz);
    NEKO_GUI_ASSERT(alloc);
    if (!path || !siz || !alloc) return 0;

    fd = fopen(path, "rb");
    if (!fd) return 0;
    fseek(fd, 0, SEEK_END);
    ret = ftell(fd);
    if (ret < 0) {
        fclose(fd);
        return 0;
    }
    *siz = (neko_gui_size)ret;
    fseek(fd, 0, SEEK_SET);
    buf = (char *)alloc->alloc(alloc->userdata, 0, *siz);
    NEKO_GUI_ASSERT(buf);
    if (!buf) {
        fclose(fd);
        return 0;
    }
    *siz = (neko_gui_size)fread(buf, 1, *siz, fd);
    fclose(fd);
    return buf;
}
#endif
NEKO_GUI_LIB int neko_gui_text_clamp(const struct neko_gui_user_font *font, const char *text, int text_len, float space, int *glyphs, float *text_width, neko_gui_rune *sep_list, int sep_count) {
    int i = 0;
    int glyph_len = 0;
    float last_width = 0;
    neko_gui_rune unicode = 0;
    float width = 0;
    int len = 0;
    int g = 0;
    float s;

    int sep_len = 0;
    int sep_g = 0;
    float sep_width = 0;
    sep_count = NEKO_GUI_MAX(sep_count, 0);

    glyph_len = neko_gui_utf_decode(text, &unicode, text_len);
    while (glyph_len && (width < space) && (len < text_len)) {
        len += glyph_len;
        s = font->width(font->userdata, font->height, text, len);
        for (i = 0; i < sep_count; ++i) {
            if (unicode != sep_list[i]) continue;
            sep_width = last_width = width;
            sep_g = g + 1;
            sep_len = len;
            break;
        }
        if (i == sep_count) {
            last_width = sep_width = width;
            sep_g = g + 1;
        }
        width = s;
        glyph_len = neko_gui_utf_decode(&text[len], &unicode, text_len - len);
        g++;
    }
    if (len >= text_len) {
        *glyphs = g;
        *text_width = last_width;
        return len;
    } else {
        *glyphs = sep_g;
        *text_width = sep_width;
        return (!sep_len) ? len : sep_len;
    }
}
NEKO_GUI_LIB struct neko_gui_vec2 neko_gui_text_calculate_text_bounds(const struct neko_gui_user_font *font, const char *begin, int byte_len, float row_height, const char **remaining,
                                                                      struct neko_gui_vec2 *out_offset, int *glyphs, int op) {
    float line_height = row_height;
    struct neko_gui_vec2 text_size = neko_gui_vec2(0, 0);
    float line_width = 0.0f;

    float glyph_width;
    int glyph_len = 0;
    neko_gui_rune unicode = 0;
    int text_len = 0;
    if (!begin || byte_len <= 0 || !font) return neko_gui_vec2(0, row_height);

    glyph_len = neko_gui_utf_decode(begin, &unicode, byte_len);
    if (!glyph_len) return text_size;
    glyph_width = font->width(font->userdata, font->height, begin, glyph_len);

    *glyphs = 0;
    while ((text_len < byte_len) && glyph_len) {
        if (unicode == '\n') {
            text_size.x = NEKO_GUI_MAX(text_size.x, line_width);
            text_size.y += line_height;
            line_width = 0;
            *glyphs += 1;
            if (op == NEKO_GUI_STOP_ON_NEW_LINE) break;

            text_len++;
            glyph_len = neko_gui_utf_decode(begin + text_len, &unicode, byte_len - text_len);
            continue;
        }

        if (unicode == '\r') {
            text_len++;
            *glyphs += 1;
            glyph_len = neko_gui_utf_decode(begin + text_len, &unicode, byte_len - text_len);
            continue;
        }

        *glyphs = *glyphs + 1;
        text_len += glyph_len;
        line_width += (float)glyph_width;
        glyph_len = neko_gui_utf_decode(begin + text_len, &unicode, byte_len - text_len);
        glyph_width = font->width(font->userdata, font->height, begin + text_len, glyph_len);
        continue;
    }

    if (text_size.x < line_width) text_size.x = line_width;
    if (out_offset) *out_offset = neko_gui_vec2(line_width, text_size.y + line_height);
    if (line_width > 0 || text_size.y == 0.0f) text_size.y += line_height;
    if (remaining) *remaining = begin + text_len;
    return text_size;
}

NEKO_GUI_INTERN int neko_gui_parse_hex(const char *p, int length) {
    int i = 0;
    int len = 0;
    while (len < length) {
        i <<= 4;
        if (p[len] >= 'a' && p[len] <= 'f')
            i += ((p[len] - 'a') + 10);
        else if (p[len] >= 'A' && p[len] <= 'F')
            i += ((p[len] - 'A') + 10);
        else
            i += (p[len] - '0');
        len++;
    }
    return i;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgba(int r, int g, int b, int a) {
    struct neko_gui_color ret;
    ret.r = (neko_gui_byte)NEKO_GUI_CLAMP(0, r, 255);
    ret.g = (neko_gui_byte)NEKO_GUI_CLAMP(0, g, 255);
    ret.b = (neko_gui_byte)NEKO_GUI_CLAMP(0, b, 255);
    ret.a = (neko_gui_byte)NEKO_GUI_CLAMP(0, a, 255);
    return ret;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_hex(const char *rgb) {
    struct neko_gui_color col;
    const char *c = rgb;
    if (*c == '#') c++;
    col.r = (neko_gui_byte)neko_gui_parse_hex(c, 2);
    col.g = (neko_gui_byte)neko_gui_parse_hex(c + 2, 2);
    col.b = (neko_gui_byte)neko_gui_parse_hex(c + 4, 2);
    col.a = 255;
    return col;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_hex(const char *rgb) {
    struct neko_gui_color col;
    const char *c = rgb;
    if (*c == '#') c++;
    col.r = (neko_gui_byte)neko_gui_parse_hex(c, 2);
    col.g = (neko_gui_byte)neko_gui_parse_hex(c + 2, 2);
    col.b = (neko_gui_byte)neko_gui_parse_hex(c + 4, 2);
    col.a = (neko_gui_byte)neko_gui_parse_hex(c + 6, 2);
    return col;
}
NEKO_GUI_API void neko_gui_color_hex_rgba(char *output, struct neko_gui_color col) {
#define NEKO_GUI_TO_HEX(i) ((i) <= 9 ? '0' + (i) : 'A' - 10 + (i))
    output[0] = (char)NEKO_GUI_TO_HEX((col.r & 0xF0) >> 4);
    output[1] = (char)NEKO_GUI_TO_HEX((col.r & 0x0F));
    output[2] = (char)NEKO_GUI_TO_HEX((col.g & 0xF0) >> 4);
    output[3] = (char)NEKO_GUI_TO_HEX((col.g & 0x0F));
    output[4] = (char)NEKO_GUI_TO_HEX((col.b & 0xF0) >> 4);
    output[5] = (char)NEKO_GUI_TO_HEX((col.b & 0x0F));
    output[6] = (char)NEKO_GUI_TO_HEX((col.a & 0xF0) >> 4);
    output[7] = (char)NEKO_GUI_TO_HEX((col.a & 0x0F));
    output[8] = '\0';
#undef NEKO_GUI_TO_HEX
}
NEKO_GUI_API void neko_gui_color_hex_rgb(char *output, struct neko_gui_color col) {
#define NEKO_GUI_TO_HEX(i) ((i) <= 9 ? '0' + (i) : 'A' - 10 + (i))
    output[0] = (char)NEKO_GUI_TO_HEX((col.r & 0xF0) >> 4);
    output[1] = (char)NEKO_GUI_TO_HEX((col.r & 0x0F));
    output[2] = (char)NEKO_GUI_TO_HEX((col.g & 0xF0) >> 4);
    output[3] = (char)NEKO_GUI_TO_HEX((col.g & 0x0F));
    output[4] = (char)NEKO_GUI_TO_HEX((col.b & 0xF0) >> 4);
    output[5] = (char)NEKO_GUI_TO_HEX((col.b & 0x0F));
    output[6] = '\0';
#undef NEKO_GUI_TO_HEX
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_iv(const int *c) { return neko_gui_rgba(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_bv(const neko_gui_byte *c) { return neko_gui_rgba(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgb(int r, int g, int b) {
    struct neko_gui_color ret;
    ret.r = (neko_gui_byte)NEKO_GUI_CLAMP(0, r, 255);
    ret.g = (neko_gui_byte)NEKO_GUI_CLAMP(0, g, 255);
    ret.b = (neko_gui_byte)NEKO_GUI_CLAMP(0, b, 255);
    ret.a = (neko_gui_byte)255;
    return ret;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_iv(const int *c) { return neko_gui_rgb(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_bv(const neko_gui_byte *c) { return neko_gui_rgb(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_u32(neko_gui_uint in) {
    struct neko_gui_color ret;
    ret.r = (in & 0xFF);
    ret.g = ((in >> 8) & 0xFF);
    ret.b = ((in >> 16) & 0xFF);
    ret.a = (neko_gui_byte)((in >> 24) & 0xFF);
    return ret;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_f(float r, float g, float b, float a) {
    struct neko_gui_color ret;
    ret.r = (neko_gui_byte)(NEKO_GUI_SATURATE(r) * 255.0f);
    ret.g = (neko_gui_byte)(NEKO_GUI_SATURATE(g) * 255.0f);
    ret.b = (neko_gui_byte)(NEKO_GUI_SATURATE(b) * 255.0f);
    ret.a = (neko_gui_byte)(NEKO_GUI_SATURATE(a) * 255.0f);
    return ret;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_fv(const float *c) { return neko_gui_rgba_f(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgba_cf(struct neko_gui_colorf c) { return neko_gui_rgba_f(c.r, c.g, c.b, c.a); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_f(float r, float g, float b) {
    struct neko_gui_color ret;
    ret.r = (neko_gui_byte)(NEKO_GUI_SATURATE(r) * 255.0f);
    ret.g = (neko_gui_byte)(NEKO_GUI_SATURATE(g) * 255.0f);
    ret.b = (neko_gui_byte)(NEKO_GUI_SATURATE(b) * 255.0f);
    ret.a = 255;
    return ret;
}
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_fv(const float *c) { return neko_gui_rgb_f(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_rgb_cf(struct neko_gui_colorf c) { return neko_gui_rgb_f(c.r, c.g, c.b); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsv(int h, int s, int v) { return neko_gui_hsva(h, s, v, 255); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_iv(const int *c) { return neko_gui_hsv(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_bv(const neko_gui_byte *c) { return neko_gui_hsv(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_f(float h, float s, float v) { return neko_gui_hsva_f(h, s, v, 1.0f); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsv_fv(const float *c) { return neko_gui_hsv_f(c[0], c[1], c[2]); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsva(int h, int s, int v, int a) {
    float hf = ((float)NEKO_GUI_CLAMP(0, h, 255)) / 255.0f;
    float sf = ((float)NEKO_GUI_CLAMP(0, s, 255)) / 255.0f;
    float vf = ((float)NEKO_GUI_CLAMP(0, v, 255)) / 255.0f;
    float af = ((float)NEKO_GUI_CLAMP(0, a, 255)) / 255.0f;
    return neko_gui_hsva_f(hf, sf, vf, af);
}
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_iv(const int *c) { return neko_gui_hsva(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_bv(const neko_gui_byte *c) { return neko_gui_hsva(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_colorf neko_gui_hsva_colorf(float h, float s, float v, float a) {
    int i;
    float p, q, t, f;
    struct neko_gui_colorf out = {0, 0, 0, 0};
    if (s <= 0.0f) {
        out.r = v;
        out.g = v;
        out.b = v;
        out.a = a;
        return out;
    }
    h = h / (60.0f / 360.0f);
    i = (int)h;
    f = h - (float)i;
    p = v * (1.0f - s);
    q = v * (1.0f - (s * f));
    t = v * (1.0f - s * (1.0f - f));

    switch (i) {
        case 0:
        default:
            out.r = v;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = v;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = v;
            out.b = t;
            break;
        case 3:
            out.r = p;
            out.g = q;
            out.b = v;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = v;
            break;
        case 5:
            out.r = v;
            out.g = p;
            out.b = q;
            break;
    }
    out.a = a;
    return out;
}
NEKO_GUI_API struct neko_gui_colorf neko_gui_hsva_colorfv(float *c) { return neko_gui_hsva_colorf(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_f(float h, float s, float v, float a) {
    struct neko_gui_colorf c = neko_gui_hsva_colorf(h, s, v, a);
    return neko_gui_rgba_f(c.r, c.g, c.b, c.a);
}
NEKO_GUI_API struct neko_gui_color neko_gui_hsva_fv(const float *c) { return neko_gui_hsva_f(c[0], c[1], c[2], c[3]); }
NEKO_GUI_API neko_gui_uint neko_gui_color_u32(struct neko_gui_color in) {
    neko_gui_uint out = (neko_gui_uint)in.r;
    out |= ((neko_gui_uint)in.g << 8);
    out |= ((neko_gui_uint)in.b << 16);
    out |= ((neko_gui_uint)in.a << 24);
    return out;
}
NEKO_GUI_API void neko_gui_color_f(float *r, float *g, float *b, float *a, struct neko_gui_color in) {
    NEKO_GUI_STORAGE const float s = 1.0f / 255.0f;
    *r = (float)in.r * s;
    *g = (float)in.g * s;
    *b = (float)in.b * s;
    *a = (float)in.a * s;
}
NEKO_GUI_API void neko_gui_color_fv(float *c, struct neko_gui_color in) { neko_gui_color_f(&c[0], &c[1], &c[2], &c[3], in); }
NEKO_GUI_API struct neko_gui_colorf neko_gui_color_cf(struct neko_gui_color in) {
    struct neko_gui_colorf o;
    neko_gui_color_f(&o.r, &o.g, &o.b, &o.a, in);
    return o;
}
NEKO_GUI_API void neko_gui_color_d(double *r, double *g, double *b, double *a, struct neko_gui_color in) {
    NEKO_GUI_STORAGE const double s = 1.0 / 255.0;
    *r = (double)in.r * s;
    *g = (double)in.g * s;
    *b = (double)in.b * s;
    *a = (double)in.a * s;
}
NEKO_GUI_API void neko_gui_color_dv(double *c, struct neko_gui_color in) { neko_gui_color_d(&c[0], &c[1], &c[2], &c[3], in); }
NEKO_GUI_API void neko_gui_color_hsv_f(float *out_h, float *out_s, float *out_v, struct neko_gui_color in) {
    float a;
    neko_gui_color_hsva_f(out_h, out_s, out_v, &a, in);
}
NEKO_GUI_API void neko_gui_color_hsv_fv(float *out, struct neko_gui_color in) {
    float a;
    neko_gui_color_hsva_f(&out[0], &out[1], &out[2], &a, in);
}
NEKO_GUI_API void neko_gui_colorf_hsva_f(float *out_h, float *out_s, float *out_v, float *out_a, struct neko_gui_colorf in) {
    float chroma;
    float K = 0.0f;
    if (in.g < in.b) {
        const float t = in.g;
        in.g = in.b;
        in.b = t;
        K = -1.f;
    }
    if (in.r < in.g) {
        const float t = in.r;
        in.r = in.g;
        in.g = t;
        K = -2.f / 6.0f - K;
    }
    chroma = in.r - ((in.g < in.b) ? in.g : in.b);
    *out_h = NEKO_GUI_ABS(K + (in.g - in.b) / (6.0f * chroma + 1e-20f));
    *out_s = chroma / (in.r + 1e-20f);
    *out_v = in.r;
    *out_a = in.a;
}
NEKO_GUI_API void neko_gui_colorf_hsva_fv(float *hsva, struct neko_gui_colorf in) { neko_gui_colorf_hsva_f(&hsva[0], &hsva[1], &hsva[2], &hsva[3], in); }
NEKO_GUI_API void neko_gui_color_hsva_f(float *out_h, float *out_s, float *out_v, float *out_a, struct neko_gui_color in) {
    struct neko_gui_colorf col;
    neko_gui_color_f(&col.r, &col.g, &col.b, &col.a, in);
    neko_gui_colorf_hsva_f(out_h, out_s, out_v, out_a, col);
}
NEKO_GUI_API void neko_gui_color_hsva_fv(float *out, struct neko_gui_color in) { neko_gui_color_hsva_f(&out[0], &out[1], &out[2], &out[3], in); }
NEKO_GUI_API void neko_gui_color_hsva_i(int *out_h, int *out_s, int *out_v, int *out_a, struct neko_gui_color in) {
    float h, s, v, a;
    neko_gui_color_hsva_f(&h, &s, &v, &a, in);
    *out_h = (neko_gui_byte)(h * 255.0f);
    *out_s = (neko_gui_byte)(s * 255.0f);
    *out_v = (neko_gui_byte)(v * 255.0f);
    *out_a = (neko_gui_byte)(a * 255.0f);
}
NEKO_GUI_API void neko_gui_color_hsva_iv(int *out, struct neko_gui_color in) { neko_gui_color_hsva_i(&out[0], &out[1], &out[2], &out[3], in); }
NEKO_GUI_API void neko_gui_color_hsva_bv(neko_gui_byte *out, struct neko_gui_color in) {
    int tmp[4];
    neko_gui_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    out[0] = (neko_gui_byte)tmp[0];
    out[1] = (neko_gui_byte)tmp[1];
    out[2] = (neko_gui_byte)tmp[2];
    out[3] = (neko_gui_byte)tmp[3];
}
NEKO_GUI_API void neko_gui_color_hsva_b(neko_gui_byte *h, neko_gui_byte *s, neko_gui_byte *v, neko_gui_byte *a, struct neko_gui_color in) {
    int tmp[4];
    neko_gui_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    *h = (neko_gui_byte)tmp[0];
    *s = (neko_gui_byte)tmp[1];
    *v = (neko_gui_byte)tmp[2];
    *a = (neko_gui_byte)tmp[3];
}
NEKO_GUI_API void neko_gui_color_hsv_i(int *out_h, int *out_s, int *out_v, struct neko_gui_color in) {
    int a;
    neko_gui_color_hsva_i(out_h, out_s, out_v, &a, in);
}
NEKO_GUI_API void neko_gui_color_hsv_b(neko_gui_byte *out_h, neko_gui_byte *out_s, neko_gui_byte *out_v, struct neko_gui_color in) {
    int tmp[4];
    neko_gui_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    *out_h = (neko_gui_byte)tmp[0];
    *out_s = (neko_gui_byte)tmp[1];
    *out_v = (neko_gui_byte)tmp[2];
}
NEKO_GUI_API void neko_gui_color_hsv_iv(int *out, struct neko_gui_color in) { neko_gui_color_hsv_i(&out[0], &out[1], &out[2], in); }
NEKO_GUI_API void neko_gui_color_hsv_bv(neko_gui_byte *out, struct neko_gui_color in) {
    int tmp[4];
    neko_gui_color_hsv_i(&tmp[0], &tmp[1], &tmp[2], in);
    out[0] = (neko_gui_byte)tmp[0];
    out[1] = (neko_gui_byte)tmp[1];
    out[2] = (neko_gui_byte)tmp[2];
}

NEKO_GUI_GLOBAL const neko_gui_byte neko_gui_utfbyte[NEKO_GUI_UTF_SIZE + 1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
NEKO_GUI_GLOBAL const neko_gui_byte neko_gui_utfmask[NEKO_GUI_UTF_SIZE + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
NEKO_GUI_GLOBAL const neko_gui_uint neko_gui_utfmin[NEKO_GUI_UTF_SIZE + 1] = {0, 0, 0x80, 0x800, 0x10000};
NEKO_GUI_GLOBAL const neko_gui_uint neko_gui_utfmax[NEKO_GUI_UTF_SIZE + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

NEKO_GUI_INTERN int neko_gui_utf_validate(neko_gui_rune *u, int i) {
    NEKO_GUI_ASSERT(u);
    if (!u) return 0;
    if (!NEKO_GUI_BETWEEN(*u, neko_gui_utfmin[i], neko_gui_utfmax[i]) || NEKO_GUI_BETWEEN(*u, 0xD800, 0xDFFF)) *u = NEKO_GUI_UTF_INVALID;
    for (i = 1; *u > neko_gui_utfmax[i]; ++i)
        ;
    return i;
}
NEKO_GUI_INTERN neko_gui_rune neko_gui_utf_decode_byte(char c, int *i) {
    NEKO_GUI_ASSERT(i);
    if (!i) return 0;
    for (*i = 0; *i < (int)NEKO_GUI_LEN(neko_gui_utfmask); ++(*i)) {
        if (((neko_gui_byte)c & neko_gui_utfmask[*i]) == neko_gui_utfbyte[*i]) return (neko_gui_byte)(c & ~neko_gui_utfmask[*i]);
    }
    return 0;
}
NEKO_GUI_API int neko_gui_utf_decode(const char *c, neko_gui_rune *u, int clen) {
    int i, j, len, type = 0;
    neko_gui_rune udecoded;

    NEKO_GUI_ASSERT(c);
    NEKO_GUI_ASSERT(u);

    if (!c || !u) return 0;
    if (!clen) return 0;
    *u = NEKO_GUI_UTF_INVALID;

    udecoded = neko_gui_utf_decode_byte(c[0], &len);
    if (!NEKO_GUI_BETWEEN(len, 1, NEKO_GUI_UTF_SIZE)) return 1;

    for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
        udecoded = (udecoded << 6) | neko_gui_utf_decode_byte(c[i], &type);
        if (type != 0) return j;
    }
    if (j < len) return 0;
    *u = udecoded;
    neko_gui_utf_validate(u, len);
    return len;
}
NEKO_GUI_INTERN char neko_gui_utf_encode_byte(neko_gui_rune u, int i) { return (char)((neko_gui_utfbyte[i]) | ((neko_gui_byte)u & ~neko_gui_utfmask[i])); }
NEKO_GUI_API int neko_gui_utf_encode(neko_gui_rune u, char *c, int clen) {
    int len, i;
    len = neko_gui_utf_validate(&u, 0);
    if (clen < len || !len || len > NEKO_GUI_UTF_SIZE) return 0;

    for (i = len - 1; i != 0; --i) {
        c[i] = neko_gui_utf_encode_byte(u, 0);
        u >>= 6;
    }
    c[0] = neko_gui_utf_encode_byte(u, len);
    return len;
}
NEKO_GUI_API int neko_gui_utf_len(const char *str, int len) {
    const char *text;
    int glyphs = 0;
    int text_len;
    int glyph_len;
    int src_len = 0;
    neko_gui_rune unicode;

    NEKO_GUI_ASSERT(str);
    if (!str || !len) return 0;

    text = str;
    text_len = len;
    glyph_len = neko_gui_utf_decode(text, &unicode, text_len);
    while (glyph_len && src_len < len) {
        glyphs++;
        src_len = src_len + glyph_len;
        glyph_len = neko_gui_utf_decode(text + src_len, &unicode, text_len - src_len);
    }
    return glyphs;
}
NEKO_GUI_API const char *neko_gui_utf_at(const char *buffer, int length, int index, neko_gui_rune *unicode, int *len) {
    int i = 0;
    int src_len = 0;
    int glyph_len = 0;
    const char *text;
    int text_len;

    NEKO_GUI_ASSERT(buffer);
    NEKO_GUI_ASSERT(unicode);
    NEKO_GUI_ASSERT(len);

    if (!buffer || !unicode || !len) return 0;
    if (index < 0) {
        *unicode = NEKO_GUI_UTF_INVALID;
        *len = 0;
        return 0;
    }

    text = buffer;
    text_len = length;
    glyph_len = neko_gui_utf_decode(text, unicode, text_len);
    while (glyph_len) {
        if (i == index) {
            *len = glyph_len;
            break;
        }

        i++;
        src_len = src_len + glyph_len;
        glyph_len = neko_gui_utf_decode(text + src_len, unicode, text_len - src_len);
    }
    if (i != index) return 0;
    return buffer + src_len;
}

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_LIB void *neko_gui_malloc(neko_gui_handle unused, void *old, neko_gui_size size) {
    NEKO_GUI_UNUSED(unused);
    NEKO_GUI_UNUSED(old);
    return malloc(size);
}
NEKO_GUI_LIB void neko_gui_mfree(neko_gui_handle unused, void *ptr) {
    NEKO_GUI_UNUSED(unused);
    free(ptr);
}
NEKO_GUI_API void neko_gui_buffer_init_default(struct neko_gui_buffer *buffer) {
    struct neko_gui_allocator alloc;
    alloc.userdata.ptr = 0;
    alloc.alloc = neko_gui_malloc;
    alloc.free = neko_gui_mfree;
    neko_gui_buffer_init(buffer, &alloc, NEKO_GUI_BUFFER_DEFAULT_INITIAL_SIZE);
}
#endif

NEKO_GUI_API void neko_gui_buffer_init(struct neko_gui_buffer *b, const struct neko_gui_allocator *a, neko_gui_size initial_size) {
    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(a);
    NEKO_GUI_ASSERT(initial_size);
    if (!b || !a || !initial_size) return;

    neko_gui_zero(b, sizeof(*b));
    b->type = NEKO_GUI_BUFFER_DYNAMIC;
    b->memory.ptr = a->alloc(a->userdata, 0, initial_size);
    b->memory.size = initial_size;
    b->size = initial_size;
    b->grow_factor = 2.0f;
    b->pool = *a;
}
NEKO_GUI_API void neko_gui_buffer_init_fixed(struct neko_gui_buffer *b, void *m, neko_gui_size size) {
    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(m);
    NEKO_GUI_ASSERT(size);
    if (!b || !m || !size) return;

    neko_gui_zero(b, sizeof(*b));
    b->type = NEKO_GUI_BUFFER_FIXED;
    b->memory.ptr = m;
    b->memory.size = size;
    b->size = size;
}
NEKO_GUI_LIB void *neko_gui_buffer_align(void *unaligned, neko_gui_size align, neko_gui_size *alignment, enum neko_gui_buffer_allocation_type type) {
    void *memory = 0;
    switch (type) {
        default:
        case NEKO_GUI_BUFFER_MAX:
        case NEKO_GUI_BUFFER_FRONT:
            if (align) {
                memory = NEKO_GUI_ALIGN_PTR(unaligned, align);
                *alignment = (neko_gui_size)((neko_gui_byte *)memory - (neko_gui_byte *)unaligned);
            } else {
                memory = unaligned;
                *alignment = 0;
            }
            break;
        case NEKO_GUI_BUFFER_BACK:
            if (align) {
                memory = NEKO_GUI_ALIGN_PTR_BACK(unaligned, align);
                *alignment = (neko_gui_size)((neko_gui_byte *)unaligned - (neko_gui_byte *)memory);
            } else {
                memory = unaligned;
                *alignment = 0;
            }
            break;
    }
    return memory;
}
NEKO_GUI_LIB void *neko_gui_buffer_realloc(struct neko_gui_buffer *b, neko_gui_size capacity, neko_gui_size *size) {
    void *temp;
    neko_gui_size buffer_size;

    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(size);
    if (!b || !size || !b->pool.alloc || !b->pool.free) return 0;

    buffer_size = b->memory.size;
    temp = b->pool.alloc(b->pool.userdata, b->memory.ptr, capacity);
    NEKO_GUI_ASSERT(temp);
    if (!temp) return 0;

    *size = capacity;
    if (temp != b->memory.ptr) {
        NEKO_GUI_MEMCPY(temp, b->memory.ptr, buffer_size);
        b->pool.free(b->pool.userdata, b->memory.ptr);
    }

    if (b->size == buffer_size) {

        b->size = capacity;
        return temp;
    } else {

        void *dst, *src;
        neko_gui_size back_size;
        back_size = buffer_size - b->size;
        dst = neko_gui_ptr_add(void, temp, capacity - back_size);
        src = neko_gui_ptr_add(void, temp, b->size);
        NEKO_GUI_MEMCPY(dst, src, back_size);
        b->size = capacity - back_size;
    }
    return temp;
}
NEKO_GUI_LIB void *neko_gui_buffer_alloc(struct neko_gui_buffer *b, enum neko_gui_buffer_allocation_type type, neko_gui_size size, neko_gui_size align) {
    int full;
    neko_gui_size alignment;
    void *unaligned;
    void *memory;

    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(size);
    if (!b || !size) return 0;
    b->needed += size;

    if (type == NEKO_GUI_BUFFER_FRONT)
        unaligned = neko_gui_ptr_add(void, b->memory.ptr, b->allocated);
    else
        unaligned = neko_gui_ptr_add(void, b->memory.ptr, b->size - size);
    memory = neko_gui_buffer_align(unaligned, align, &alignment, type);

    if (type == NEKO_GUI_BUFFER_FRONT)
        full = ((b->allocated + size + alignment) > b->size);
    else
        full = ((b->size - NEKO_GUI_MIN(b->size, (size + alignment))) <= b->allocated);

    if (full) {
        neko_gui_size capacity;
        if (b->type != NEKO_GUI_BUFFER_DYNAMIC) return 0;
        NEKO_GUI_ASSERT(b->pool.alloc && b->pool.free);
        if (b->type != NEKO_GUI_BUFFER_DYNAMIC || !b->pool.alloc || !b->pool.free) return 0;

        capacity = (neko_gui_size)((float)b->memory.size * b->grow_factor);
        capacity = NEKO_GUI_MAX(capacity, neko_gui_round_up_pow2((neko_gui_uint)(b->allocated + size)));
        b->memory.ptr = neko_gui_buffer_realloc(b, capacity, &b->memory.size);
        if (!b->memory.ptr) return 0;

        if (type == NEKO_GUI_BUFFER_FRONT)
            unaligned = neko_gui_ptr_add(void, b->memory.ptr, b->allocated);
        else
            unaligned = neko_gui_ptr_add(void, b->memory.ptr, b->size - size);
        memory = neko_gui_buffer_align(unaligned, align, &alignment, type);
    }
    if (type == NEKO_GUI_BUFFER_FRONT)
        b->allocated += size + alignment;
    else
        b->size -= (size + alignment);
    b->needed += alignment;
    b->calls++;
    return memory;
}
NEKO_GUI_API void neko_gui_buffer_push(struct neko_gui_buffer *b, enum neko_gui_buffer_allocation_type type, const void *memory, neko_gui_size size, neko_gui_size align) {
    void *mem = neko_gui_buffer_alloc(b, type, size, align);
    if (!mem) return;
    NEKO_GUI_MEMCPY(mem, memory, size);
}
NEKO_GUI_API void neko_gui_buffer_mark(struct neko_gui_buffer *buffer, enum neko_gui_buffer_allocation_type type) {
    NEKO_GUI_ASSERT(buffer);
    if (!buffer) return;
    buffer->marker[type].active = neko_gui_true;
    if (type == NEKO_GUI_BUFFER_BACK)
        buffer->marker[type].offset = buffer->size;
    else
        buffer->marker[type].offset = buffer->allocated;
}
NEKO_GUI_API void neko_gui_buffer_reset(struct neko_gui_buffer *buffer, enum neko_gui_buffer_allocation_type type) {
    NEKO_GUI_ASSERT(buffer);
    if (!buffer) return;
    if (type == NEKO_GUI_BUFFER_BACK) {

        buffer->needed -= (buffer->memory.size - buffer->marker[type].offset);
        if (buffer->marker[type].active)
            buffer->size = buffer->marker[type].offset;
        else
            buffer->size = buffer->memory.size;
        buffer->marker[type].active = neko_gui_false;
    } else {

        buffer->needed -= (buffer->allocated - buffer->marker[type].offset);
        if (buffer->marker[type].active)
            buffer->allocated = buffer->marker[type].offset;
        else
            buffer->allocated = 0;
        buffer->marker[type].active = neko_gui_false;
    }
}
NEKO_GUI_API void neko_gui_buffer_clear(struct neko_gui_buffer *b) {
    NEKO_GUI_ASSERT(b);
    if (!b) return;
    b->allocated = 0;
    b->size = b->memory.size;
    b->calls = 0;
    b->needed = 0;
}
NEKO_GUI_API void neko_gui_buffer_free(struct neko_gui_buffer *b) {
    NEKO_GUI_ASSERT(b);
    if (!b || !b->memory.ptr) return;
    if (b->type == NEKO_GUI_BUFFER_FIXED) return;
    if (!b->pool.free) return;
    NEKO_GUI_ASSERT(b->pool.free);
    b->pool.free(b->pool.userdata, b->memory.ptr);
}
NEKO_GUI_API void neko_gui_buffer_info(struct neko_gui_memory_status *s, struct neko_gui_buffer *b) {
    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(s);
    if (!s || !b) return;
    s->allocated = b->allocated;
    s->size = b->memory.size;
    s->needed = b->needed;
    s->memory = b->memory.ptr;
    s->calls = b->calls;
}
NEKO_GUI_API void *neko_gui_buffer_memory(struct neko_gui_buffer *buffer) {
    NEKO_GUI_ASSERT(buffer);
    if (!buffer) return 0;
    return buffer->memory.ptr;
}
NEKO_GUI_API const void *neko_gui_buffer_memory_const(const struct neko_gui_buffer *buffer) {
    NEKO_GUI_ASSERT(buffer);
    if (!buffer) return 0;
    return buffer->memory.ptr;
}
NEKO_GUI_API neko_gui_size neko_gui_buffer_total(struct neko_gui_buffer *buffer) {
    NEKO_GUI_ASSERT(buffer);
    if (!buffer) return 0;
    return buffer->memory.size;
}

#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_str_init_default(struct neko_gui_str *str) {
    struct neko_gui_allocator alloc;
    alloc.userdata.ptr = 0;
    alloc.alloc = neko_gui_malloc;
    alloc.free = neko_gui_mfree;
    neko_gui_buffer_init(&str->buffer, &alloc, 32);
    str->len = 0;
}
#endif

NEKO_GUI_API void neko_gui_str_init(struct neko_gui_str *str, const struct neko_gui_allocator *alloc, neko_gui_size size) {
    neko_gui_buffer_init(&str->buffer, alloc, size);
    str->len = 0;
}
NEKO_GUI_API void neko_gui_str_init_fixed(struct neko_gui_str *str, void *memory, neko_gui_size size) {
    neko_gui_buffer_init_fixed(&str->buffer, memory, size);
    str->len = 0;
}
NEKO_GUI_API int neko_gui_str_append_text_char(struct neko_gui_str *s, const char *str, int len) {
    char *mem;
    NEKO_GUI_ASSERT(s);
    NEKO_GUI_ASSERT(str);
    if (!s || !str || !len) return 0;
    mem = (char *)neko_gui_buffer_alloc(&s->buffer, NEKO_GUI_BUFFER_FRONT, (neko_gui_size)len * sizeof(char), 0);
    if (!mem) return 0;
    NEKO_GUI_MEMCPY(mem, str, (neko_gui_size)len * sizeof(char));
    s->len += neko_gui_utf_len(str, len);
    return len;
}
NEKO_GUI_API int neko_gui_str_append_str_char(struct neko_gui_str *s, const char *str) { return neko_gui_str_append_text_char(s, str, neko_gui_strlen(str)); }
NEKO_GUI_API int neko_gui_str_append_text_utf8(struct neko_gui_str *str, const char *text, int len) {
    int i = 0;
    int byte_len = 0;
    neko_gui_rune unicode;
    if (!str || !text || !len) return 0;
    for (i = 0; i < len; ++i) byte_len += neko_gui_utf_decode(text + byte_len, &unicode, 4);
    neko_gui_str_append_text_char(str, text, byte_len);
    return len;
}
NEKO_GUI_API int neko_gui_str_append_str_utf8(struct neko_gui_str *str, const char *text) {
    int byte_len = 0;
    int num_runes = 0;
    int glyph_len = 0;
    neko_gui_rune unicode;
    if (!str || !text) return 0;

    glyph_len = byte_len = neko_gui_utf_decode(text + byte_len, &unicode, 4);
    while (unicode != '\0' && glyph_len) {
        glyph_len = neko_gui_utf_decode(text + byte_len, &unicode, 4);
        byte_len += glyph_len;
        num_runes++;
    }
    neko_gui_str_append_text_char(str, text, byte_len);
    return num_runes;
}
NEKO_GUI_API int neko_gui_str_append_text_runes(struct neko_gui_str *str, const neko_gui_rune *text, int len) {
    int i = 0;
    int byte_len = 0;
    neko_gui_glyph glyph;

    NEKO_GUI_ASSERT(str);
    if (!str || !text || !len) return 0;
    for (i = 0; i < len; ++i) {
        byte_len = neko_gui_utf_encode(text[i], glyph, NEKO_GUI_UTF_SIZE);
        if (!byte_len) break;
        neko_gui_str_append_text_char(str, glyph, byte_len);
    }
    return len;
}
NEKO_GUI_API int neko_gui_str_append_str_runes(struct neko_gui_str *str, const neko_gui_rune *runes) {
    int i = 0;
    neko_gui_glyph glyph;
    int byte_len;
    NEKO_GUI_ASSERT(str);
    if (!str || !runes) return 0;
    while (runes[i] != '\0') {
        byte_len = neko_gui_utf_encode(runes[i], glyph, NEKO_GUI_UTF_SIZE);
        neko_gui_str_append_text_char(str, glyph, byte_len);
        i++;
    }
    return i;
}
NEKO_GUI_API int neko_gui_str_insert_at_char(struct neko_gui_str *s, int pos, const char *str, int len) {
    int i;
    void *mem;
    char *src;
    char *dst;

    int copylen;
    NEKO_GUI_ASSERT(s);
    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(len >= 0);
    if (!s || !str || !len || (neko_gui_size)pos > s->buffer.allocated) return 0;
    if ((s->buffer.allocated + (neko_gui_size)len >= s->buffer.memory.size) && (s->buffer.type == NEKO_GUI_BUFFER_FIXED)) return 0;

    copylen = (int)s->buffer.allocated - pos;
    if (!copylen) {
        neko_gui_str_append_text_char(s, str, len);
        return 1;
    }
    mem = neko_gui_buffer_alloc(&s->buffer, NEKO_GUI_BUFFER_FRONT, (neko_gui_size)len * sizeof(char), 0);
    if (!mem) return 0;

    NEKO_GUI_ASSERT(((int)pos + (int)len + ((int)copylen - 1)) >= 0);
    NEKO_GUI_ASSERT(((int)pos + ((int)copylen - 1)) >= 0);
    dst = neko_gui_ptr_add(char, s->buffer.memory.ptr, pos + len + (copylen - 1));
    src = neko_gui_ptr_add(char, s->buffer.memory.ptr, pos + (copylen - 1));
    for (i = 0; i < copylen; ++i) *dst-- = *src--;
    mem = neko_gui_ptr_add(void, s->buffer.memory.ptr, pos);
    NEKO_GUI_MEMCPY(mem, str, (neko_gui_size)len * sizeof(char));
    s->len = neko_gui_utf_len((char *)s->buffer.memory.ptr, (int)s->buffer.allocated);
    return 1;
}
NEKO_GUI_API int neko_gui_str_insert_at_rune(struct neko_gui_str *str, int pos, const char *cstr, int len) {
    int glyph_len;
    neko_gui_rune unicode;
    const char *begin;
    const char *buffer;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(cstr);
    NEKO_GUI_ASSERT(len);
    if (!str || !cstr || !len) return 0;
    begin = neko_gui_str_at_rune(str, pos, &unicode, &glyph_len);
    if (!str->len) return neko_gui_str_append_text_char(str, cstr, len);
    buffer = neko_gui_str_get_const(str);
    if (!begin) return 0;
    return neko_gui_str_insert_at_char(str, (int)(begin - buffer), cstr, len);
}
NEKO_GUI_API int neko_gui_str_insert_text_char(struct neko_gui_str *str, int pos, const char *text, int len) { return neko_gui_str_insert_text_utf8(str, pos, text, len); }
NEKO_GUI_API int neko_gui_str_insert_str_char(struct neko_gui_str *str, int pos, const char *text) { return neko_gui_str_insert_text_utf8(str, pos, text, neko_gui_strlen(text)); }
NEKO_GUI_API int neko_gui_str_insert_text_utf8(struct neko_gui_str *str, int pos, const char *text, int len) {
    int i = 0;
    int byte_len = 0;
    neko_gui_rune unicode;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(text);
    if (!str || !text || !len) return 0;
    for (i = 0; i < len; ++i) byte_len += neko_gui_utf_decode(text + byte_len, &unicode, 4);
    neko_gui_str_insert_at_rune(str, pos, text, byte_len);
    return len;
}
NEKO_GUI_API int neko_gui_str_insert_str_utf8(struct neko_gui_str *str, int pos, const char *text) {
    int byte_len = 0;
    int num_runes = 0;
    int glyph_len = 0;
    neko_gui_rune unicode;
    if (!str || !text) return 0;

    glyph_len = byte_len = neko_gui_utf_decode(text + byte_len, &unicode, 4);
    while (unicode != '\0' && glyph_len) {
        glyph_len = neko_gui_utf_decode(text + byte_len, &unicode, 4);
        byte_len += glyph_len;
        num_runes++;
    }
    neko_gui_str_insert_at_rune(str, pos, text, byte_len);
    return num_runes;
}
NEKO_GUI_API int neko_gui_str_insert_text_runes(struct neko_gui_str *str, int pos, const neko_gui_rune *runes, int len) {
    int i = 0;
    int byte_len = 0;
    neko_gui_glyph glyph;

    NEKO_GUI_ASSERT(str);
    if (!str || !runes || !len) return 0;
    for (i = 0; i < len; ++i) {
        byte_len = neko_gui_utf_encode(runes[i], glyph, NEKO_GUI_UTF_SIZE);
        if (!byte_len) break;
        neko_gui_str_insert_at_rune(str, pos + i, glyph, byte_len);
    }
    return len;
}
NEKO_GUI_API int neko_gui_str_insert_str_runes(struct neko_gui_str *str, int pos, const neko_gui_rune *runes) {
    int i = 0;
    neko_gui_glyph glyph;
    int byte_len;
    NEKO_GUI_ASSERT(str);
    if (!str || !runes) return 0;
    while (runes[i] != '\0') {
        byte_len = neko_gui_utf_encode(runes[i], glyph, NEKO_GUI_UTF_SIZE);
        neko_gui_str_insert_at_rune(str, pos + i, glyph, byte_len);
        i++;
    }
    return i;
}
NEKO_GUI_API void neko_gui_str_remove_chars(struct neko_gui_str *s, int len) {
    NEKO_GUI_ASSERT(s);
    NEKO_GUI_ASSERT(len >= 0);
    if (!s || len < 0 || (neko_gui_size)len > s->buffer.allocated) return;
    NEKO_GUI_ASSERT(((int)s->buffer.allocated - (int)len) >= 0);
    s->buffer.allocated -= (neko_gui_size)len;
    s->len = neko_gui_utf_len((char *)s->buffer.memory.ptr, (int)s->buffer.allocated);
}
NEKO_GUI_API void neko_gui_str_remove_runes(struct neko_gui_str *str, int len) {
    int index;
    const char *begin;
    const char *end;
    neko_gui_rune unicode;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(len >= 0);
    if (!str || len < 0) return;
    if (len >= str->len) {
        str->len = 0;
        return;
    }

    index = str->len - len;
    begin = neko_gui_str_at_rune(str, index, &unicode, &len);
    end = (const char *)str->buffer.memory.ptr + str->buffer.allocated;
    neko_gui_str_remove_chars(str, (int)(end - begin) + 1);
}
NEKO_GUI_API void neko_gui_str_delete_chars(struct neko_gui_str *s, int pos, int len) {
    NEKO_GUI_ASSERT(s);
    if (!s || !len || (neko_gui_size)pos > s->buffer.allocated || (neko_gui_size)(pos + len) > s->buffer.allocated) return;

    if ((neko_gui_size)(pos + len) < s->buffer.allocated) {

        char *dst = neko_gui_ptr_add(char, s->buffer.memory.ptr, pos);
        char *src = neko_gui_ptr_add(char, s->buffer.memory.ptr, pos + len);
        NEKO_GUI_MEMCPY(dst, src, s->buffer.allocated - (neko_gui_size)(pos + len));
        NEKO_GUI_ASSERT(((int)s->buffer.allocated - (int)len) >= 0);
        s->buffer.allocated -= (neko_gui_size)len;
    } else
        neko_gui_str_remove_chars(s, len);
    s->len = neko_gui_utf_len((char *)s->buffer.memory.ptr, (int)s->buffer.allocated);
}
NEKO_GUI_API void neko_gui_str_delete_runes(struct neko_gui_str *s, int pos, int len) {
    char *temp;
    neko_gui_rune unicode;
    char *begin;
    char *end;
    int unused;

    NEKO_GUI_ASSERT(s);
    NEKO_GUI_ASSERT(s->len >= pos + len);
    if (s->len < pos + len) len = NEKO_GUI_CLAMP(0, (s->len - pos), s->len);
    if (!len) return;

    temp = (char *)s->buffer.memory.ptr;
    begin = neko_gui_str_at_rune(s, pos, &unicode, &unused);
    if (!begin) return;
    s->buffer.memory.ptr = begin;
    end = neko_gui_str_at_rune(s, len, &unicode, &unused);
    s->buffer.memory.ptr = temp;
    if (!end) return;
    neko_gui_str_delete_chars(s, (int)(begin - temp), (int)(end - begin));
}
NEKO_GUI_API char *neko_gui_str_at_char(struct neko_gui_str *s, int pos) {
    NEKO_GUI_ASSERT(s);
    if (!s || pos > (int)s->buffer.allocated) return 0;
    return neko_gui_ptr_add(char, s->buffer.memory.ptr, pos);
}
NEKO_GUI_API char *neko_gui_str_at_rune(struct neko_gui_str *str, int pos, neko_gui_rune *unicode, int *len) {
    int i = 0;
    int src_len = 0;
    int glyph_len = 0;
    char *text;
    int text_len;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(unicode);
    NEKO_GUI_ASSERT(len);

    if (!str || !unicode || !len) return 0;
    if (pos < 0) {
        *unicode = 0;
        *len = 0;
        return 0;
    }

    text = (char *)str->buffer.memory.ptr;
    text_len = (int)str->buffer.allocated;
    glyph_len = neko_gui_utf_decode(text, unicode, text_len);
    while (glyph_len) {
        if (i == pos) {
            *len = glyph_len;
            break;
        }

        i++;
        src_len = src_len + glyph_len;
        glyph_len = neko_gui_utf_decode(text + src_len, unicode, text_len - src_len);
    }
    if (i != pos) return 0;
    return text + src_len;
}
NEKO_GUI_API const char *neko_gui_str_at_char_const(const struct neko_gui_str *s, int pos) {
    NEKO_GUI_ASSERT(s);
    if (!s || pos > (int)s->buffer.allocated) return 0;
    return neko_gui_ptr_add(char, s->buffer.memory.ptr, pos);
}
NEKO_GUI_API const char *neko_gui_str_at_const(const struct neko_gui_str *str, int pos, neko_gui_rune *unicode, int *len) {
    int i = 0;
    int src_len = 0;
    int glyph_len = 0;
    char *text;
    int text_len;

    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(unicode);
    NEKO_GUI_ASSERT(len);

    if (!str || !unicode || !len) return 0;
    if (pos < 0) {
        *unicode = 0;
        *len = 0;
        return 0;
    }

    text = (char *)str->buffer.memory.ptr;
    text_len = (int)str->buffer.allocated;
    glyph_len = neko_gui_utf_decode(text, unicode, text_len);
    while (glyph_len) {
        if (i == pos) {
            *len = glyph_len;
            break;
        }

        i++;
        src_len = src_len + glyph_len;
        glyph_len = neko_gui_utf_decode(text + src_len, unicode, text_len - src_len);
    }
    if (i != pos) return 0;
    return text + src_len;
}
NEKO_GUI_API neko_gui_rune neko_gui_str_rune_at(const struct neko_gui_str *str, int pos) {
    int len;
    neko_gui_rune unicode = 0;
    neko_gui_str_at_const(str, pos, &unicode, &len);
    return unicode;
}
NEKO_GUI_API char *neko_gui_str_get(struct neko_gui_str *s) {
    NEKO_GUI_ASSERT(s);
    if (!s || !s->len || !s->buffer.allocated) return 0;
    return (char *)s->buffer.memory.ptr;
}
NEKO_GUI_API const char *neko_gui_str_get_const(const struct neko_gui_str *s) {
    NEKO_GUI_ASSERT(s);
    if (!s || !s->len || !s->buffer.allocated) return 0;
    return (const char *)s->buffer.memory.ptr;
}
NEKO_GUI_API int neko_gui_str_len(struct neko_gui_str *s) {
    NEKO_GUI_ASSERT(s);
    if (!s || !s->len || !s->buffer.allocated) return 0;
    return s->len;
}
NEKO_GUI_API int neko_gui_str_len_char(struct neko_gui_str *s) {
    NEKO_GUI_ASSERT(s);
    if (!s || !s->len || !s->buffer.allocated) return 0;
    return (int)s->buffer.allocated;
}
NEKO_GUI_API void neko_gui_str_clear(struct neko_gui_str *str) {
    NEKO_GUI_ASSERT(str);
    neko_gui_buffer_clear(&str->buffer);
    str->len = 0;
}
NEKO_GUI_API void neko_gui_str_free(struct neko_gui_str *str) {
    NEKO_GUI_ASSERT(str);
    neko_gui_buffer_free(&str->buffer);
    str->len = 0;
}

NEKO_GUI_LIB void neko_gui_command_buffer_init(struct neko_gui_command_buffer *cb, struct neko_gui_buffer *b, enum neko_gui_command_clipping clip) {
    NEKO_GUI_ASSERT(cb);
    NEKO_GUI_ASSERT(b);
    if (!cb || !b) return;
    cb->base = b;
    cb->use_clipping = (int)clip;
    cb->begin = b->allocated;
    cb->end = b->allocated;
    cb->last = b->allocated;
}
NEKO_GUI_LIB void neko_gui_command_buffer_reset(struct neko_gui_command_buffer *b) {
    NEKO_GUI_ASSERT(b);
    if (!b) return;
    b->begin = 0;
    b->end = 0;
    b->last = 0;
    b->clip = neko_gui_null_rect;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    b->userdata.ptr = 0;
#endif
}
NEKO_GUI_LIB void *neko_gui_command_buffer_push(struct neko_gui_command_buffer *b, enum neko_gui_command_type t, neko_gui_size size) {
    NEKO_GUI_STORAGE const neko_gui_size align = NEKO_GUI_ALIGNOF(struct neko_gui_command);
    struct neko_gui_command *cmd;
    neko_gui_size alignment;
    void *unaligned;
    void *memory;

    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(b->base);
    if (!b) return 0;
    cmd = (struct neko_gui_command *)neko_gui_buffer_alloc(b->base, NEKO_GUI_BUFFER_FRONT, size, align);
    if (!cmd) return 0;

    b->last = (neko_gui_size)((neko_gui_byte *)cmd - (neko_gui_byte *)b->base->memory.ptr);
    unaligned = (neko_gui_byte *)cmd + size;
    memory = NEKO_GUI_ALIGN_PTR(unaligned, align);
    alignment = (neko_gui_size)((neko_gui_byte *)memory - (neko_gui_byte *)unaligned);
#ifdef NEKO_GUI_ZERO_COMMAND_MEMORY
    NEKO_GUI_MEMSET(cmd, 0, size + alignment);
#endif

    cmd->type = t;
    cmd->next = b->base->allocated + alignment;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    cmd->userdata = b->userdata;
#endif
    b->end = cmd->next;
    return cmd;
}
NEKO_GUI_API void neko_gui_push_scissor(struct neko_gui_command_buffer *b, struct neko_gui_rect r) {
    struct neko_gui_command_scissor *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b) return;

    b->clip.x = r.x;
    b->clip.y = r.y;
    b->clip.w = r.w;
    b->clip.h = r.h;
    cmd = (struct neko_gui_command_scissor *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_SCISSOR, sizeof(*cmd));

    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, r.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, r.h);
}
NEKO_GUI_API void neko_gui_stroke_line(struct neko_gui_command_buffer *b, float x0, float y0, float x1, float y1, float line_thickness, struct neko_gui_color c) {
    struct neko_gui_command_line *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || line_thickness <= 0) return;
    cmd = (struct neko_gui_command_line *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_LINE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->begin.x = (short)x0;
    cmd->begin.y = (short)y0;
    cmd->end.x = (short)x1;
    cmd->end.y = (short)y1;
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_stroke_curve(struct neko_gui_command_buffer *b, float ax, float ay, float ctrl0x, float ctrl0y, float ctrl1x, float ctrl1y, float bx, float by, float line_thickness,
                                        struct neko_gui_color col) {
    struct neko_gui_command_curve *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;

    cmd = (struct neko_gui_command_curve *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_CURVE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->begin.x = (short)ax;
    cmd->begin.y = (short)ay;
    cmd->ctrl[0].x = (short)ctrl0x;
    cmd->ctrl[0].y = (short)ctrl0y;
    cmd->ctrl[1].x = (short)ctrl1x;
    cmd->ctrl[1].y = (short)ctrl1y;
    cmd->end.x = (short)bx;
    cmd->end.y = (short)by;
    cmd->color = col;
}
NEKO_GUI_API void neko_gui_stroke_rect(struct neko_gui_command_buffer *b, struct neko_gui_rect rect, float rounding, float line_thickness, struct neko_gui_color c) {
    struct neko_gui_command_rect *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0 || rect.w == 0 || rect.h == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INTERSECT(rect.x, rect.y, rect.w, rect.h, clip->x, clip->y, clip->w, clip->h)) return;
    }
    cmd = (struct neko_gui_command_rect *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_RECT, sizeof(*cmd));
    if (!cmd) return;
    cmd->rounding = (unsigned short)rounding;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, rect.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, rect.h);
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_fill_rect(struct neko_gui_command_buffer *b, struct neko_gui_rect rect, float rounding, struct neko_gui_color c) {
    struct neko_gui_command_rect_filled *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0 || rect.w == 0 || rect.h == 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INTERSECT(rect.x, rect.y, rect.w, rect.h, clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct neko_gui_command_rect_filled *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_RECT_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->rounding = (unsigned short)rounding;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, rect.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, rect.h);
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_fill_rect_multi_color(struct neko_gui_command_buffer *b, struct neko_gui_rect rect, struct neko_gui_color left, struct neko_gui_color top, struct neko_gui_color right,
                                                 struct neko_gui_color bottom) {
    struct neko_gui_command_rect_multi_color *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || rect.w == 0 || rect.h == 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INTERSECT(rect.x, rect.y, rect.w, rect.h, clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct neko_gui_command_rect_multi_color *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_RECT_MULTI_COLOR, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, rect.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, rect.h);
    cmd->left = left;
    cmd->top = top;
    cmd->right = right;
    cmd->bottom = bottom;
}
NEKO_GUI_API void neko_gui_stroke_circle(struct neko_gui_command_buffer *b, struct neko_gui_rect r, float line_thickness, struct neko_gui_color c) {
    struct neko_gui_command_circle *cmd;
    if (!b || r.w == 0 || r.h == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INTERSECT(r.x, r.y, r.w, r.h, clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct neko_gui_command_circle *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_CIRCLE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(r.w, 0);
    cmd->h = (unsigned short)NEKO_GUI_MAX(r.h, 0);
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_fill_circle(struct neko_gui_command_buffer *b, struct neko_gui_rect r, struct neko_gui_color c) {
    struct neko_gui_command_circle_filled *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0 || r.w == 0 || r.h == 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INTERSECT(r.x, r.y, r.w, r.h, clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct neko_gui_command_circle_filled *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_CIRCLE_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(r.w, 0);
    cmd->h = (unsigned short)NEKO_GUI_MAX(r.h, 0);
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_stroke_arc(struct neko_gui_command_buffer *b, float cx, float cy, float radius, float a_min, float a_max, float line_thickness, struct neko_gui_color c) {
    struct neko_gui_command_arc *cmd;
    if (!b || c.a == 0 || line_thickness <= 0) return;
    cmd = (struct neko_gui_command_arc *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_ARC, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->cx = (short)cx;
    cmd->cy = (short)cy;
    cmd->r = (unsigned short)radius;
    cmd->a[0] = a_min;
    cmd->a[1] = a_max;
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_fill_arc(struct neko_gui_command_buffer *b, float cx, float cy, float radius, float a_min, float a_max, struct neko_gui_color c) {
    struct neko_gui_command_arc_filled *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0) return;
    cmd = (struct neko_gui_command_arc_filled *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_ARC_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->cx = (short)cx;
    cmd->cy = (short)cy;
    cmd->r = (unsigned short)radius;
    cmd->a[0] = a_min;
    cmd->a[1] = a_max;
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_stroke_triangle(struct neko_gui_command_buffer *b, float x0, float y0, float x1, float y1, float x2, float y2, float line_thickness, struct neko_gui_color c) {
    struct neko_gui_command_triangle *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INBOX(x0, y0, clip->x, clip->y, clip->w, clip->h) && !NEKO_GUI_INBOX(x1, y1, clip->x, clip->y, clip->w, clip->h) && !NEKO_GUI_INBOX(x2, y2, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct neko_gui_command_triangle *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_TRIANGLE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->a.x = (short)x0;
    cmd->a.y = (short)y0;
    cmd->b.x = (short)x1;
    cmd->b.y = (short)y1;
    cmd->c.x = (short)x2;
    cmd->c.y = (short)y2;
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_fill_triangle(struct neko_gui_command_buffer *b, float x0, float y0, float x1, float y1, float x2, float y2, struct neko_gui_color c) {
    struct neko_gui_command_triangle_filled *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b || c.a == 0) return;
    if (!b) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *clip = &b->clip;
        if (!NEKO_GUI_INBOX(x0, y0, clip->x, clip->y, clip->w, clip->h) && !NEKO_GUI_INBOX(x1, y1, clip->x, clip->y, clip->w, clip->h) && !NEKO_GUI_INBOX(x2, y2, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct neko_gui_command_triangle_filled *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_TRIANGLE_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->a.x = (short)x0;
    cmd->a.y = (short)y0;
    cmd->b.x = (short)x1;
    cmd->b.y = (short)y1;
    cmd->c.x = (short)x2;
    cmd->c.y = (short)y2;
    cmd->color = c;
}
NEKO_GUI_API void neko_gui_stroke_polygon(struct neko_gui_command_buffer *b, float *points, int point_count, float line_thickness, struct neko_gui_color col) {
    int i;
    neko_gui_size size = 0;
    struct neko_gui_command_polygon *cmd;

    NEKO_GUI_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (neko_gui_size)point_count;
    cmd = (struct neko_gui_command_polygon *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_POLYGON, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->point_count = (unsigned short)point_count;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i * 2];
        cmd->points[i].y = (short)points[i * 2 + 1];
    }
}
NEKO_GUI_API void neko_gui_fill_polygon(struct neko_gui_command_buffer *b, float *points, int point_count, struct neko_gui_color col) {
    int i;
    neko_gui_size size = 0;
    struct neko_gui_command_polygon_filled *cmd;

    NEKO_GUI_ASSERT(b);
    if (!b || col.a == 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (neko_gui_size)point_count;
    cmd = (struct neko_gui_command_polygon_filled *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_POLYGON_FILLED, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->point_count = (unsigned short)point_count;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i * 2 + 0];
        cmd->points[i].y = (short)points[i * 2 + 1];
    }
}
NEKO_GUI_API void neko_gui_stroke_polyline(struct neko_gui_command_buffer *b, float *points, int point_count, float line_thickness, struct neko_gui_color col) {
    int i;
    neko_gui_size size = 0;
    struct neko_gui_command_polyline *cmd;

    NEKO_GUI_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (neko_gui_size)point_count;
    cmd = (struct neko_gui_command_polyline *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_POLYLINE, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->point_count = (unsigned short)point_count;
    cmd->line_thickness = (unsigned short)line_thickness;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i * 2];
        cmd->points[i].y = (short)points[i * 2 + 1];
    }
}
NEKO_GUI_API void neko_gui_draw_image(struct neko_gui_command_buffer *b, struct neko_gui_rect r, const struct neko_gui_image *img, struct neko_gui_color col) {
    struct neko_gui_command_image *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NEKO_GUI_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h)) return;
    }

    cmd = (struct neko_gui_command_image *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_IMAGE, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, r.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, r.h);
    cmd->img = *img;
    cmd->col = col;
}
NEKO_GUI_API void neko_gui_draw_nine_slice(struct neko_gui_command_buffer *b, struct neko_gui_rect r, const struct neko_gui_nine_slice *slc, struct neko_gui_color col) {
    struct neko_gui_image img;
    const struct neko_gui_image *slcimg = (const struct neko_gui_image *)slc;
    neko_gui_ushort rgnX, rgnY, rgnW, rgnH;
    rgnX = slcimg->region[0];
    rgnY = slcimg->region[1];
    rgnW = slcimg->region[2];
    rgnH = slcimg->region[3];

    img.handle = slcimg->handle;
    img.w = slcimg->w;
    img.h = slcimg->h;
    img.region[0] = rgnX;
    img.region[1] = rgnY;
    img.region[2] = slc->l;
    img.region[3] = slc->t;

    neko_gui_draw_image(b, neko_gui_rect(r.x, r.y, (float)slc->l, (float)slc->t), &img, col);

#define IMG_RGN(x, y, w, h)               \
    img.region[0] = (neko_gui_ushort)(x); \
    img.region[1] = (neko_gui_ushort)(y); \
    img.region[2] = (neko_gui_ushort)(w); \
    img.region[3] = (neko_gui_ushort)(h);

    IMG_RGN(rgnX + slc->l, rgnY, rgnW - slc->l - slc->r, slc->t);
    neko_gui_draw_image(b, neko_gui_rect(r.x + (float)slc->l, r.y, (float)(r.w - slc->l - slc->r), (float)slc->t), &img, col);

    IMG_RGN(rgnX + rgnW - slc->r, rgnY, slc->r, slc->t);
    neko_gui_draw_image(b, neko_gui_rect(r.x + r.w - (float)slc->r, r.y, (float)slc->r, (float)slc->t), &img, col);

    IMG_RGN(rgnX, rgnY + slc->t, slc->l, rgnH - slc->t - slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x, r.y + (float)slc->t, (float)slc->l, (float)(r.h - slc->t - slc->b)), &img, col);

    IMG_RGN(rgnX + slc->l, rgnY + slc->t, rgnW - slc->l - slc->r, rgnH - slc->t - slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x + (float)slc->l, r.y + (float)slc->t, (float)(r.w - slc->l - slc->r), (float)(r.h - slc->t - slc->b)), &img, col);

    IMG_RGN(rgnX + rgnW - slc->r, rgnY + slc->t, slc->r, rgnH - slc->t - slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x + r.w - (float)slc->r, r.y + (float)slc->t, (float)slc->r, (float)(r.h - slc->t - slc->b)), &img, col);

    IMG_RGN(rgnX, rgnY + rgnH - slc->b, slc->l, slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x, r.y + r.h - (float)slc->b, (float)slc->l, (float)slc->b), &img, col);

    IMG_RGN(rgnX + slc->l, rgnY + rgnH - slc->b, rgnW - slc->l - slc->r, slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x + (float)slc->l, r.y + r.h - (float)slc->b, (float)(r.w - slc->l - slc->r), (float)slc->b), &img, col);

    IMG_RGN(rgnX + rgnW - slc->r, rgnY + rgnH - slc->b, slc->r, slc->b);
    neko_gui_draw_image(b, neko_gui_rect(r.x + r.w - (float)slc->r, r.y + r.h - (float)slc->b, (float)slc->r, (float)slc->b), &img, col);

#undef IMG_RGN
}
NEKO_GUI_API void neko_gui_push_custom(struct neko_gui_command_buffer *b, struct neko_gui_rect r, neko_gui_command_custom_callback cb, neko_gui_handle usr) {
    struct neko_gui_command_custom *cmd;
    NEKO_GUI_ASSERT(b);
    if (!b) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NEKO_GUI_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h)) return;
    }

    cmd = (struct neko_gui_command_custom *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_CUSTOM, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NEKO_GUI_MAX(0, r.w);
    cmd->h = (unsigned short)NEKO_GUI_MAX(0, r.h);
    cmd->callback_data = usr;
    cmd->callback = cb;
}
NEKO_GUI_API void neko_gui_draw_text(struct neko_gui_command_buffer *b, struct neko_gui_rect r, const char *string, int length, const struct neko_gui_user_font *font, struct neko_gui_color bg,
                                     struct neko_gui_color fg) {
    float text_width = 0;
    struct neko_gui_command_text *cmd;

    NEKO_GUI_ASSERT(b);
    NEKO_GUI_ASSERT(font);
    if (!b || !string || !length || (bg.a == 0 && fg.a == 0)) return;
    if (b->use_clipping) {
        const struct neko_gui_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NEKO_GUI_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h)) return;
    }

    text_width = font->width(font->userdata, font->height, string, length);
    if (text_width > r.w) {
        int glyphs = 0;
        float txt_width = (float)text_width;
        length = neko_gui_text_clamp(font, string, length, r.w, &glyphs, &txt_width, 0, 0);
    }

    if (!length) return;
    cmd = (struct neko_gui_command_text *)neko_gui_command_buffer_push(b, NEKO_GUI_COMMAND_TEXT, sizeof(*cmd) + (neko_gui_size)(length + 1));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)r.w;
    cmd->h = (unsigned short)r.h;
    cmd->background = bg;
    cmd->foreground = fg;
    cmd->font = font;
    cmd->length = length;
    cmd->height = font->height;
    NEKO_GUI_MEMCPY(cmd->string, string, (neko_gui_size)length);
    cmd->string[length] = '\0';
}

#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
NEKO_GUI_API void neko_gui_draw_list_init(struct neko_gui_draw_list *list) {
    neko_gui_size i = 0;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    neko_gui_zero(list, sizeof(*list));
    for (i = 0; i < NEKO_GUI_LEN(list->circle_vtx); ++i) {
        const float a = ((float)i / (float)NEKO_GUI_LEN(list->circle_vtx)) * 2 * NEKO_GUI_PI;
        list->circle_vtx[i].x = (float)NEKO_GUI_COS(a);
        list->circle_vtx[i].y = (float)NEKO_GUI_SIN(a);
    }
}
NEKO_GUI_API void neko_gui_draw_list_setup(struct neko_gui_draw_list *canvas, const struct neko_gui_convert_config *config, struct neko_gui_buffer *cmds, struct neko_gui_buffer *vertices,
                                           struct neko_gui_buffer *elements, enum neko_gui_anti_aliasing line_aa, enum neko_gui_anti_aliasing shape_aa) {
    NEKO_GUI_ASSERT(canvas);
    NEKO_GUI_ASSERT(config);
    NEKO_GUI_ASSERT(cmds);
    NEKO_GUI_ASSERT(vertices);
    NEKO_GUI_ASSERT(elements);
    if (!canvas || !config || !cmds || !vertices || !elements) return;

    canvas->buffer = cmds;
    canvas->config = *config;
    canvas->elements = elements;
    canvas->vertices = vertices;
    canvas->line_AA = line_aa;
    canvas->shape_AA = shape_aa;
    canvas->clip_rect = neko_gui_null_rect;

    canvas->cmd_offset = 0;
    canvas->element_count = 0;
    canvas->vertex_count = 0;
    canvas->cmd_offset = 0;
    canvas->cmd_count = 0;
    canvas->path_count = 0;
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_begin(const struct neko_gui_draw_list *canvas, const struct neko_gui_buffer *buffer) {
    neko_gui_byte *memory;
    neko_gui_size offset;
    const struct neko_gui_draw_command *cmd;

    NEKO_GUI_ASSERT(buffer);
    if (!buffer || !buffer->size || !canvas->cmd_count) return 0;

    memory = (neko_gui_byte *)buffer->memory.ptr;
    offset = buffer->memory.size - canvas->cmd_offset;
    cmd = neko_gui_ptr_add(const struct neko_gui_draw_command, memory, offset);
    return cmd;
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_end(const struct neko_gui_draw_list *canvas, const struct neko_gui_buffer *buffer) {
    neko_gui_size size;
    neko_gui_size offset;
    neko_gui_byte *memory;
    const struct neko_gui_draw_command *end;

    NEKO_GUI_ASSERT(buffer);
    NEKO_GUI_ASSERT(canvas);
    if (!buffer || !canvas) return 0;

    memory = (neko_gui_byte *)buffer->memory.ptr;
    size = buffer->memory.size;
    offset = size - canvas->cmd_offset;
    end = neko_gui_ptr_add(const struct neko_gui_draw_command, memory, offset);
    end -= (canvas->cmd_count - 1);
    return end;
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_list_next(const struct neko_gui_draw_command *cmd, const struct neko_gui_buffer *buffer, const struct neko_gui_draw_list *canvas) {
    const struct neko_gui_draw_command *end;
    NEKO_GUI_ASSERT(buffer);
    NEKO_GUI_ASSERT(canvas);
    if (!cmd || !buffer || !canvas) return 0;

    end = neko_gui__draw_list_end(canvas, buffer);
    if (cmd <= end) return 0;
    return (cmd - 1);
}
NEKO_GUI_INTERN struct neko_gui_vec2 *neko_gui_draw_list_alloc_path(struct neko_gui_draw_list *list, int count) {
    struct neko_gui_vec2 *points;
    NEKO_GUI_STORAGE const neko_gui_size point_align = NEKO_GUI_ALIGNOF(struct neko_gui_vec2);
    NEKO_GUI_STORAGE const neko_gui_size point_size = sizeof(struct neko_gui_vec2);
    points = (struct neko_gui_vec2 *)neko_gui_buffer_alloc(list->buffer, NEKO_GUI_BUFFER_FRONT, point_size * (neko_gui_size)count, point_align);

    if (!points) return 0;
    if (!list->path_offset) {
        void *memory = neko_gui_buffer_memory(list->buffer);
        list->path_offset = (unsigned int)((neko_gui_byte *)points - (neko_gui_byte *)memory);
    }
    list->path_count += (unsigned int)count;
    return points;
}
NEKO_GUI_INTERN struct neko_gui_vec2 neko_gui_draw_list_path_last(struct neko_gui_draw_list *list) {
    void *memory;
    struct neko_gui_vec2 *point;
    NEKO_GUI_ASSERT(list->path_count);
    memory = neko_gui_buffer_memory(list->buffer);
    point = neko_gui_ptr_add(struct neko_gui_vec2, memory, list->path_offset);
    point += (list->path_count - 1);
    return *point;
}
NEKO_GUI_INTERN struct neko_gui_draw_command *neko_gui_draw_list_push_command(struct neko_gui_draw_list *list, struct neko_gui_rect clip, neko_gui_handle texture) {
    NEKO_GUI_STORAGE const neko_gui_size cmd_align = NEKO_GUI_ALIGNOF(struct neko_gui_draw_command);
    NEKO_GUI_STORAGE const neko_gui_size cmd_size = sizeof(struct neko_gui_draw_command);
    struct neko_gui_draw_command *cmd;

    NEKO_GUI_ASSERT(list);
    cmd = (struct neko_gui_draw_command *)neko_gui_buffer_alloc(list->buffer, NEKO_GUI_BUFFER_BACK, cmd_size, cmd_align);

    if (!cmd) return 0;
    if (!list->cmd_count) {
        neko_gui_byte *memory = (neko_gui_byte *)neko_gui_buffer_memory(list->buffer);
        neko_gui_size total = neko_gui_buffer_total(list->buffer);
        memory = neko_gui_ptr_add(neko_gui_byte, memory, total);
        list->cmd_offset = (neko_gui_size)(memory - (neko_gui_byte *)cmd);
    }

    cmd->elem_count = 0;
    cmd->clip_rect = clip;
    cmd->texture = texture;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    cmd->userdata = list->userdata;
#endif

    list->cmd_count++;
    list->clip_rect = clip;
    return cmd;
}
NEKO_GUI_INTERN struct neko_gui_draw_command *neko_gui_draw_list_command_last(struct neko_gui_draw_list *list) {
    void *memory;
    neko_gui_size size;
    struct neko_gui_draw_command *cmd;
    NEKO_GUI_ASSERT(list->cmd_count);

    memory = neko_gui_buffer_memory(list->buffer);
    size = neko_gui_buffer_total(list->buffer);
    cmd = neko_gui_ptr_add(struct neko_gui_draw_command, memory, size - list->cmd_offset);
    return (cmd - (list->cmd_count - 1));
}
NEKO_GUI_INTERN void neko_gui_draw_list_add_clip(struct neko_gui_draw_list *list, struct neko_gui_rect rect) {
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    if (!list->cmd_count) {
        neko_gui_draw_list_push_command(list, rect, list->config.tex_null.texture);
    } else {
        struct neko_gui_draw_command *prev = neko_gui_draw_list_command_last(list);
        if (prev->elem_count == 0) prev->clip_rect = rect;
        neko_gui_draw_list_push_command(list, rect, prev->texture);
    }
}
NEKO_GUI_INTERN void neko_gui_draw_list_push_image(struct neko_gui_draw_list *list, neko_gui_handle texture) {
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    if (!list->cmd_count) {
        neko_gui_draw_list_push_command(list, neko_gui_null_rect, texture);
    } else {
        struct neko_gui_draw_command *prev = neko_gui_draw_list_command_last(list);
        if (prev->elem_count == 0) {
            prev->texture = texture;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
            prev->userdata = list->userdata;
#endif
        } else if (prev->texture.id != texture.id
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
                   || prev->userdata.id != list->userdata.id
#endif
        )
            neko_gui_draw_list_push_command(list, prev->clip_rect, texture);
    }
}
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
NEKO_GUI_API void neko_gui_draw_list_push_userdata(struct neko_gui_draw_list *list, neko_gui_handle userdata) { list->userdata = userdata; }
#endif
NEKO_GUI_INTERN void *neko_gui_draw_list_alloc_vertices(struct neko_gui_draw_list *list, neko_gui_size count) {
    void *vtx;
    NEKO_GUI_ASSERT(list);
    if (!list) return 0;
    vtx = neko_gui_buffer_alloc(list->vertices, NEKO_GUI_BUFFER_FRONT, list->config.vertex_size * count, list->config.vertex_alignment);
    if (!vtx) return 0;
    list->vertex_count += (unsigned int)count;

    if (sizeof(neko_gui_draw_index) == 2)
        NEKO_GUI_ASSERT((list->vertex_count < NEKO_GUI_USHORT_MAX && "To many vertices for 16-bit vertex indices. Please read comment above on how to solve this problem"));
    return vtx;
}
NEKO_GUI_INTERN neko_gui_draw_index *neko_gui_draw_list_alloc_elements(struct neko_gui_draw_list *list, neko_gui_size count) {
    neko_gui_draw_index *ids;
    struct neko_gui_draw_command *cmd;
    NEKO_GUI_STORAGE const neko_gui_size elem_align = NEKO_GUI_ALIGNOF(neko_gui_draw_index);
    NEKO_GUI_STORAGE const neko_gui_size elem_size = sizeof(neko_gui_draw_index);
    NEKO_GUI_ASSERT(list);
    if (!list) return 0;

    ids = (neko_gui_draw_index *)neko_gui_buffer_alloc(list->elements, NEKO_GUI_BUFFER_FRONT, elem_size * count, elem_align);
    if (!ids) return 0;
    cmd = neko_gui_draw_list_command_last(list);
    list->element_count += (unsigned int)count;
    cmd->elem_count += (unsigned int)count;
    return ids;
}
NEKO_GUI_INTERN int neko_gui_draw_vertex_layout_element_is_end_of_layout(const struct neko_gui_draw_vertex_layout_element *element) {
    return (element->attribute == NEKO_GUI_VERTEX_ATTRIBUTE_COUNT || element->format == NEKO_GUI_FORMAT_COUNT);
}
NEKO_GUI_INTERN void neko_gui_draw_vertex_color(void *attr, const float *vals, enum neko_gui_draw_vertex_layout_format format) {

    float val[4];
    NEKO_GUI_ASSERT(format >= NEKO_GUI_FORMAT_COLOR_BEGIN);
    NEKO_GUI_ASSERT(format <= NEKO_GUI_FORMAT_COLOR_END);
    if (format < NEKO_GUI_FORMAT_COLOR_BEGIN || format > NEKO_GUI_FORMAT_COLOR_END) return;

    val[0] = NEKO_GUI_SATURATE(vals[0]);
    val[1] = NEKO_GUI_SATURATE(vals[1]);
    val[2] = NEKO_GUI_SATURATE(vals[2]);
    val[3] = NEKO_GUI_SATURATE(vals[3]);

    switch (format) {
        default:
            NEKO_GUI_ASSERT(0 && "Invalid vertex layout color format");
            break;
        case NEKO_GUI_FORMAT_R8G8B8A8:
        case NEKO_GUI_FORMAT_R8G8B8: {
            struct neko_gui_color col = neko_gui_rgba_fv(val);
            NEKO_GUI_MEMCPY(attr, &col.r, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_B8G8R8A8: {
            struct neko_gui_color col = neko_gui_rgba_fv(val);
            struct neko_gui_color bgra = neko_gui_rgba(col.b, col.g, col.r, col.a);
            NEKO_GUI_MEMCPY(attr, &bgra, sizeof(bgra));
        } break;
        case NEKO_GUI_FORMAT_R16G15B16: {
            neko_gui_ushort col[3];
            col[0] = (neko_gui_ushort)(val[0] * (float)NEKO_GUI_USHORT_MAX);
            col[1] = (neko_gui_ushort)(val[1] * (float)NEKO_GUI_USHORT_MAX);
            col[2] = (neko_gui_ushort)(val[2] * (float)NEKO_GUI_USHORT_MAX);
            NEKO_GUI_MEMCPY(attr, col, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_R16G15B16A16: {
            neko_gui_ushort col[4];
            col[0] = (neko_gui_ushort)(val[0] * (float)NEKO_GUI_USHORT_MAX);
            col[1] = (neko_gui_ushort)(val[1] * (float)NEKO_GUI_USHORT_MAX);
            col[2] = (neko_gui_ushort)(val[2] * (float)NEKO_GUI_USHORT_MAX);
            col[3] = (neko_gui_ushort)(val[3] * (float)NEKO_GUI_USHORT_MAX);
            NEKO_GUI_MEMCPY(attr, col, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_R32G32B32: {
            neko_gui_uint col[3];
            col[0] = (neko_gui_uint)(val[0] * (float)NEKO_GUI_UINT_MAX);
            col[1] = (neko_gui_uint)(val[1] * (float)NEKO_GUI_UINT_MAX);
            col[2] = (neko_gui_uint)(val[2] * (float)NEKO_GUI_UINT_MAX);
            NEKO_GUI_MEMCPY(attr, col, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_R32G32B32A32: {
            neko_gui_uint col[4];
            col[0] = (neko_gui_uint)(val[0] * (float)NEKO_GUI_UINT_MAX);
            col[1] = (neko_gui_uint)(val[1] * (float)NEKO_GUI_UINT_MAX);
            col[2] = (neko_gui_uint)(val[2] * (float)NEKO_GUI_UINT_MAX);
            col[3] = (neko_gui_uint)(val[3] * (float)NEKO_GUI_UINT_MAX);
            NEKO_GUI_MEMCPY(attr, col, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_R32G32B32A32_FLOAT:
            NEKO_GUI_MEMCPY(attr, val, sizeof(float) * 4);
            break;
        case NEKO_GUI_FORMAT_R32G32B32A32_DOUBLE: {
            double col[4];
            col[0] = (double)val[0];
            col[1] = (double)val[1];
            col[2] = (double)val[2];
            col[3] = (double)val[3];
            NEKO_GUI_MEMCPY(attr, col, sizeof(col));
        } break;
        case NEKO_GUI_FORMAT_RGB32:
        case NEKO_GUI_FORMAT_RGBA32: {
            struct neko_gui_color col = neko_gui_rgba_fv(val);
            neko_gui_uint color = neko_gui_color_u32(col);
            NEKO_GUI_MEMCPY(attr, &color, sizeof(color));
        } break;
    }
}
NEKO_GUI_INTERN void neko_gui_draw_vertex_element(void *dst, const float *values, int value_count, enum neko_gui_draw_vertex_layout_format format) {
    int value_index;
    void *attribute = dst;

    NEKO_GUI_ASSERT(format < NEKO_GUI_FORMAT_COLOR_BEGIN);
    if (format >= NEKO_GUI_FORMAT_COLOR_BEGIN && format <= NEKO_GUI_FORMAT_COLOR_END) return;
    for (value_index = 0; value_index < value_count; ++value_index) {
        switch (format) {
            default:
                NEKO_GUI_ASSERT(0 && "invalid vertex layout format");
                break;
            case NEKO_GUI_FORMAT_SCHAR: {
                char value = (char)NEKO_GUI_CLAMP((float)NEKO_GUI_SCHAR_MIN, values[value_index], (float)NEKO_GUI_SCHAR_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(char));
            } break;
            case NEKO_GUI_FORMAT_SSHORT: {
                neko_gui_short value = (neko_gui_short)NEKO_GUI_CLAMP((float)NEKO_GUI_SSHORT_MIN, values[value_index], (float)NEKO_GUI_SSHORT_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(value));
            } break;
            case NEKO_GUI_FORMAT_SINT: {
                neko_gui_int value = (neko_gui_int)NEKO_GUI_CLAMP((float)NEKO_GUI_SINT_MIN, values[value_index], (float)NEKO_GUI_SINT_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(neko_gui_int));
            } break;
            case NEKO_GUI_FORMAT_UCHAR: {
                unsigned char value = (unsigned char)NEKO_GUI_CLAMP((float)NEKO_GUI_UCHAR_MIN, values[value_index], (float)NEKO_GUI_UCHAR_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(unsigned char));
            } break;
            case NEKO_GUI_FORMAT_USHORT: {
                neko_gui_ushort value = (neko_gui_ushort)NEKO_GUI_CLAMP((float)NEKO_GUI_USHORT_MIN, values[value_index], (float)NEKO_GUI_USHORT_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(value));
            } break;
            case NEKO_GUI_FORMAT_UINT: {
                neko_gui_uint value = (neko_gui_uint)NEKO_GUI_CLAMP((float)NEKO_GUI_UINT_MIN, values[value_index], (float)NEKO_GUI_UINT_MAX);
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(neko_gui_uint));
            } break;
            case NEKO_GUI_FORMAT_FLOAT:
                NEKO_GUI_MEMCPY(attribute, &values[value_index], sizeof(values[value_index]));
                attribute = (void *)((char *)attribute + sizeof(float));
                break;
            case NEKO_GUI_FORMAT_DOUBLE: {
                double value = (double)values[value_index];
                NEKO_GUI_MEMCPY(attribute, &value, sizeof(value));
                attribute = (void *)((char *)attribute + sizeof(double));
            } break;
        }
    }
}
NEKO_GUI_INTERN void *neko_gui_draw_vertex(void *dst, const struct neko_gui_convert_config *config, struct neko_gui_vec2 pos, struct neko_gui_vec2 uv, struct neko_gui_colorf color) {
    void *result = (void *)((char *)dst + config->vertex_size);
    const struct neko_gui_draw_vertex_layout_element *elem_iter = config->vertex_layout;
    while (!neko_gui_draw_vertex_layout_element_is_end_of_layout(elem_iter)) {
        void *address = (void *)((char *)dst + elem_iter->offset);
        switch (elem_iter->attribute) {
            case NEKO_GUI_VERTEX_ATTRIBUTE_COUNT:
            default:
                NEKO_GUI_ASSERT(0 && "wrong element attribute");
                break;
            case NEKO_GUI_VERTEX_POSITION:
                neko_gui_draw_vertex_element(address, &pos.x, 2, elem_iter->format);
                break;
            case NEKO_GUI_VERTEX_TEXCOORD:
                neko_gui_draw_vertex_element(address, &uv.x, 2, elem_iter->format);
                break;
            case NEKO_GUI_VERTEX_COLOR:
                neko_gui_draw_vertex_color(address, &color.r, elem_iter->format);
                break;
        }
        elem_iter++;
    }
    return result;
}
NEKO_GUI_API void neko_gui_draw_list_stroke_poly_line(struct neko_gui_draw_list *list, const struct neko_gui_vec2 *points, const unsigned int points_count, struct neko_gui_color color,
                                                      enum neko_gui_draw_list_stroke closed, float thickness, enum neko_gui_anti_aliasing aliasing) {
    neko_gui_size count;
    int thick_line;
    struct neko_gui_colorf col;
    struct neko_gui_colorf col_trans;
    NEKO_GUI_ASSERT(list);
    if (!list || points_count < 2) return;

    color.a = (neko_gui_byte)((float)color.a * list->config.global_alpha);
    count = points_count;
    if (!closed) count = points_count - 1;
    thick_line = thickness > 1.0f;

#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_draw_list_push_userdata(list, list->userdata);
#endif

    color.a = (neko_gui_byte)((float)color.a * list->config.global_alpha);
    neko_gui_color_fv(&col.r, color);
    col_trans = col;
    col_trans.a = 0;

    if (aliasing == NEKO_GUI_ANTI_ALIASING_ON) {

        const float AA_SIZE = 1.0f;
        NEKO_GUI_STORAGE const neko_gui_size pnt_align = NEKO_GUI_ALIGNOF(struct neko_gui_vec2);
        NEKO_GUI_STORAGE const neko_gui_size pnt_size = sizeof(struct neko_gui_vec2);

        neko_gui_size i1 = 0;
        neko_gui_size vertex_offset;
        neko_gui_size index = list->vertex_count;

        const neko_gui_size idx_count = (thick_line) ? (count * 18) : (count * 12);
        const neko_gui_size vtx_count = (thick_line) ? (points_count * 4) : (points_count * 3);

        void *vtx = neko_gui_draw_list_alloc_vertices(list, vtx_count);
        neko_gui_draw_index *ids = neko_gui_draw_list_alloc_elements(list, idx_count);

        neko_gui_size size;
        struct neko_gui_vec2 *normals, *temp;
        if (!vtx || !ids) return;

        vertex_offset = (neko_gui_size)((neko_gui_byte *)vtx - (neko_gui_byte *)list->vertices->memory.ptr);
        neko_gui_buffer_mark(list->vertices, NEKO_GUI_BUFFER_FRONT);
        size = pnt_size * ((thick_line) ? 5 : 3) * points_count;
        normals = (struct neko_gui_vec2 *)neko_gui_buffer_alloc(list->vertices, NEKO_GUI_BUFFER_FRONT, size, pnt_align);
        if (!normals) return;
        temp = normals + points_count;

        vtx = (void *)((neko_gui_byte *)list->vertices->memory.ptr + vertex_offset);

        for (i1 = 0; i1 < count; ++i1) {
            const neko_gui_size i2 = ((i1 + 1) == points_count) ? 0 : (i1 + 1);
            struct neko_gui_vec2 diff = neko_gui_vec2_sub(points[i2], points[i1]);
            float len;

            len = neko_gui_vec2_len_sqr(diff);
            if (len != 0.0f)
                len = NEKO_GUI_INV_SQRT(len);
            else
                len = 1.0f;

            diff = neko_gui_vec2_muls(diff, len);
            normals[i1].x = diff.y;
            normals[i1].y = -diff.x;
        }

        if (!closed) normals[points_count - 1] = normals[points_count - 2];

        if (!thick_line) {
            neko_gui_size idx1, i;
            if (!closed) {
                struct neko_gui_vec2 d;
                temp[0] = neko_gui_vec2_add(points[0], neko_gui_vec2_muls(normals[0], AA_SIZE));
                temp[1] = neko_gui_vec2_sub(points[0], neko_gui_vec2_muls(normals[0], AA_SIZE));
                d = neko_gui_vec2_muls(normals[points_count - 1], AA_SIZE);
                temp[(points_count - 1) * 2 + 0] = neko_gui_vec2_add(points[points_count - 1], d);
                temp[(points_count - 1) * 2 + 1] = neko_gui_vec2_sub(points[points_count - 1], d);
            }

            idx1 = index;
            for (i1 = 0; i1 < count; i1++) {
                struct neko_gui_vec2 dm;
                float dmr2;
                neko_gui_size i2 = ((i1 + 1) == points_count) ? 0 : (i1 + 1);
                neko_gui_size idx2 = ((i1 + 1) == points_count) ? index : (idx1 + 3);

                dm = neko_gui_vec2_muls(neko_gui_vec2_add(normals[i1], normals[i2]), 0.5f);
                dmr2 = dm.x * dm.x + dm.y * dm.y;
                if (dmr2 > 0.000001f) {
                    float scale = 1.0f / dmr2;
                    scale = NEKO_GUI_MIN(100.0f, scale);
                    dm = neko_gui_vec2_muls(dm, scale);
                }

                dm = neko_gui_vec2_muls(dm, AA_SIZE);
                temp[i2 * 2 + 0] = neko_gui_vec2_add(points[i2], dm);
                temp[i2 * 2 + 1] = neko_gui_vec2_sub(points[i2], dm);

                ids[0] = (neko_gui_draw_index)(idx2 + 0);
                ids[1] = (neko_gui_draw_index)(idx1 + 0);
                ids[2] = (neko_gui_draw_index)(idx1 + 2);
                ids[3] = (neko_gui_draw_index)(idx1 + 2);
                ids[4] = (neko_gui_draw_index)(idx2 + 2);
                ids[5] = (neko_gui_draw_index)(idx2 + 0);
                ids[6] = (neko_gui_draw_index)(idx2 + 1);
                ids[7] = (neko_gui_draw_index)(idx1 + 1);
                ids[8] = (neko_gui_draw_index)(idx1 + 0);
                ids[9] = (neko_gui_draw_index)(idx1 + 0);
                ids[10] = (neko_gui_draw_index)(idx2 + 0);
                ids[11] = (neko_gui_draw_index)(idx2 + 1);
                ids += 12;
                idx1 = idx2;
            }

            for (i = 0; i < points_count; ++i) {
                const struct neko_gui_vec2 uv = list->config.tex_null.uv;
                vtx = neko_gui_draw_vertex(vtx, &list->config, points[i], uv, col);
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 2 + 0], uv, col_trans);
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 2 + 1], uv, col_trans);
            }
        } else {
            neko_gui_size idx1, i;
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed) {
                struct neko_gui_vec2 d1 = neko_gui_vec2_muls(normals[0], half_inner_thickness + AA_SIZE);
                struct neko_gui_vec2 d2 = neko_gui_vec2_muls(normals[0], half_inner_thickness);

                temp[0] = neko_gui_vec2_add(points[0], d1);
                temp[1] = neko_gui_vec2_add(points[0], d2);
                temp[2] = neko_gui_vec2_sub(points[0], d2);
                temp[3] = neko_gui_vec2_sub(points[0], d1);

                d1 = neko_gui_vec2_muls(normals[points_count - 1], half_inner_thickness + AA_SIZE);
                d2 = neko_gui_vec2_muls(normals[points_count - 1], half_inner_thickness);

                temp[(points_count - 1) * 4 + 0] = neko_gui_vec2_add(points[points_count - 1], d1);
                temp[(points_count - 1) * 4 + 1] = neko_gui_vec2_add(points[points_count - 1], d2);
                temp[(points_count - 1) * 4 + 2] = neko_gui_vec2_sub(points[points_count - 1], d2);
                temp[(points_count - 1) * 4 + 3] = neko_gui_vec2_sub(points[points_count - 1], d1);
            }

            idx1 = index;
            for (i1 = 0; i1 < count; ++i1) {
                struct neko_gui_vec2 dm_out, dm_in;
                const neko_gui_size i2 = ((i1 + 1) == points_count) ? 0 : (i1 + 1);
                neko_gui_size idx2 = ((i1 + 1) == points_count) ? index : (idx1 + 4);

                struct neko_gui_vec2 dm = neko_gui_vec2_muls(neko_gui_vec2_add(normals[i1], normals[i2]), 0.5f);
                float dmr2 = dm.x * dm.x + dm.y * dm.y;
                if (dmr2 > 0.000001f) {
                    float scale = 1.0f / dmr2;
                    scale = NEKO_GUI_MIN(100.0f, scale);
                    dm = neko_gui_vec2_muls(dm, scale);
                }

                dm_out = neko_gui_vec2_muls(dm, ((half_inner_thickness) + AA_SIZE));
                dm_in = neko_gui_vec2_muls(dm, half_inner_thickness);
                temp[i2 * 4 + 0] = neko_gui_vec2_add(points[i2], dm_out);
                temp[i2 * 4 + 1] = neko_gui_vec2_add(points[i2], dm_in);
                temp[i2 * 4 + 2] = neko_gui_vec2_sub(points[i2], dm_in);
                temp[i2 * 4 + 3] = neko_gui_vec2_sub(points[i2], dm_out);

                ids[0] = (neko_gui_draw_index)(idx2 + 1);
                ids[1] = (neko_gui_draw_index)(idx1 + 1);
                ids[2] = (neko_gui_draw_index)(idx1 + 2);
                ids[3] = (neko_gui_draw_index)(idx1 + 2);
                ids[4] = (neko_gui_draw_index)(idx2 + 2);
                ids[5] = (neko_gui_draw_index)(idx2 + 1);
                ids[6] = (neko_gui_draw_index)(idx2 + 1);
                ids[7] = (neko_gui_draw_index)(idx1 + 1);
                ids[8] = (neko_gui_draw_index)(idx1 + 0);
                ids[9] = (neko_gui_draw_index)(idx1 + 0);
                ids[10] = (neko_gui_draw_index)(idx2 + 0);
                ids[11] = (neko_gui_draw_index)(idx2 + 1);
                ids[12] = (neko_gui_draw_index)(idx2 + 2);
                ids[13] = (neko_gui_draw_index)(idx1 + 2);
                ids[14] = (neko_gui_draw_index)(idx1 + 3);
                ids[15] = (neko_gui_draw_index)(idx1 + 3);
                ids[16] = (neko_gui_draw_index)(idx2 + 3);
                ids[17] = (neko_gui_draw_index)(idx2 + 2);
                ids += 18;
                idx1 = idx2;
            }

            for (i = 0; i < points_count; ++i) {
                const struct neko_gui_vec2 uv = list->config.tex_null.uv;
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 4 + 0], uv, col_trans);
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 4 + 1], uv, col);
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 4 + 2], uv, col);
                vtx = neko_gui_draw_vertex(vtx, &list->config, temp[i * 4 + 3], uv, col_trans);
            }
        }

        neko_gui_buffer_reset(list->vertices, NEKO_GUI_BUFFER_FRONT);
    } else {

        neko_gui_size i1 = 0;
        neko_gui_size idx = list->vertex_count;
        const neko_gui_size idx_count = count * 6;
        const neko_gui_size vtx_count = count * 4;
        void *vtx = neko_gui_draw_list_alloc_vertices(list, vtx_count);
        neko_gui_draw_index *ids = neko_gui_draw_list_alloc_elements(list, idx_count);
        if (!vtx || !ids) return;

        for (i1 = 0; i1 < count; ++i1) {
            float dx, dy;
            const struct neko_gui_vec2 uv = list->config.tex_null.uv;
            const neko_gui_size i2 = ((i1 + 1) == points_count) ? 0 : i1 + 1;
            const struct neko_gui_vec2 p1 = points[i1];
            const struct neko_gui_vec2 p2 = points[i2];
            struct neko_gui_vec2 diff = neko_gui_vec2_sub(p2, p1);
            float len;

            len = neko_gui_vec2_len_sqr(diff);
            if (len != 0.0f)
                len = NEKO_GUI_INV_SQRT(len);
            else
                len = 1.0f;
            diff = neko_gui_vec2_muls(diff, len);

            dx = diff.x * (thickness * 0.5f);
            dy = diff.y * (thickness * 0.5f);

            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(p1.x + dy, p1.y - dx), uv, col);
            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(p2.x + dy, p2.y - dx), uv, col);
            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(p2.x - dy, p2.y + dx), uv, col);
            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(p1.x - dy, p1.y + dx), uv, col);

            ids[0] = (neko_gui_draw_index)(idx + 0);
            ids[1] = (neko_gui_draw_index)(idx + 1);
            ids[2] = (neko_gui_draw_index)(idx + 2);
            ids[3] = (neko_gui_draw_index)(idx + 0);
            ids[4] = (neko_gui_draw_index)(idx + 2);
            ids[5] = (neko_gui_draw_index)(idx + 3);

            ids += 6;
            idx += 4;
        }
    }
}
NEKO_GUI_API void neko_gui_draw_list_fill_poly_convex(struct neko_gui_draw_list *list, const struct neko_gui_vec2 *points, const unsigned int points_count, struct neko_gui_color color,
                                                      enum neko_gui_anti_aliasing aliasing) {
    struct neko_gui_colorf col;
    struct neko_gui_colorf col_trans;

    NEKO_GUI_STORAGE const neko_gui_size pnt_align = NEKO_GUI_ALIGNOF(struct neko_gui_vec2);
    NEKO_GUI_STORAGE const neko_gui_size pnt_size = sizeof(struct neko_gui_vec2);
    NEKO_GUI_ASSERT(list);
    if (!list || points_count < 3) return;

#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    neko_gui_draw_list_push_userdata(list, list->userdata);
#endif

    color.a = (neko_gui_byte)((float)color.a * list->config.global_alpha);
    neko_gui_color_fv(&col.r, color);
    col_trans = col;
    col_trans.a = 0;

    if (aliasing == NEKO_GUI_ANTI_ALIASING_ON) {
        neko_gui_size i = 0;
        neko_gui_size i0 = 0;
        neko_gui_size i1 = 0;

        const float AA_SIZE = 1.0f;
        neko_gui_size vertex_offset = 0;
        neko_gui_size index = list->vertex_count;

        const neko_gui_size idx_count = (points_count - 2) * 3 + points_count * 6;
        const neko_gui_size vtx_count = (points_count * 2);

        void *vtx = neko_gui_draw_list_alloc_vertices(list, vtx_count);
        neko_gui_draw_index *ids = neko_gui_draw_list_alloc_elements(list, idx_count);

        neko_gui_size size = 0;
        struct neko_gui_vec2 *normals = 0;
        unsigned int vtx_inner_idx = (unsigned int)(index + 0);
        unsigned int vtx_outer_idx = (unsigned int)(index + 1);
        if (!vtx || !ids) return;

        vertex_offset = (neko_gui_size)((neko_gui_byte *)vtx - (neko_gui_byte *)list->vertices->memory.ptr);
        neko_gui_buffer_mark(list->vertices, NEKO_GUI_BUFFER_FRONT);
        size = pnt_size * points_count;
        normals = (struct neko_gui_vec2 *)neko_gui_buffer_alloc(list->vertices, NEKO_GUI_BUFFER_FRONT, size, pnt_align);
        if (!normals) return;
        vtx = (void *)((neko_gui_byte *)list->vertices->memory.ptr + vertex_offset);

        for (i = 2; i < points_count; i++) {
            ids[0] = (neko_gui_draw_index)(vtx_inner_idx);
            ids[1] = (neko_gui_draw_index)(vtx_inner_idx + ((i - 1) << 1));
            ids[2] = (neko_gui_draw_index)(vtx_inner_idx + (i << 1));
            ids += 3;
        }

        for (i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++) {
            struct neko_gui_vec2 p0 = points[i0];
            struct neko_gui_vec2 p1 = points[i1];
            struct neko_gui_vec2 diff = neko_gui_vec2_sub(p1, p0);

            float len = neko_gui_vec2_len_sqr(diff);
            if (len != 0.0f)
                len = NEKO_GUI_INV_SQRT(len);
            else
                len = 1.0f;
            diff = neko_gui_vec2_muls(diff, len);

            normals[i0].x = diff.y;
            normals[i0].y = -diff.x;
        }

        for (i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++) {
            const struct neko_gui_vec2 uv = list->config.tex_null.uv;
            struct neko_gui_vec2 n0 = normals[i0];
            struct neko_gui_vec2 n1 = normals[i1];
            struct neko_gui_vec2 dm = neko_gui_vec2_muls(neko_gui_vec2_add(n0, n1), 0.5f);
            float dmr2 = dm.x * dm.x + dm.y * dm.y;
            if (dmr2 > 0.000001f) {
                float scale = 1.0f / dmr2;
                scale = NEKO_GUI_MIN(scale, 100.0f);
                dm = neko_gui_vec2_muls(dm, scale);
            }
            dm = neko_gui_vec2_muls(dm, AA_SIZE * 0.5f);

            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2_sub(points[i1], dm), uv, col);
            vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2_add(points[i1], dm), uv, col_trans);

            ids[0] = (neko_gui_draw_index)(vtx_inner_idx + (i1 << 1));
            ids[1] = (neko_gui_draw_index)(vtx_inner_idx + (i0 << 1));
            ids[2] = (neko_gui_draw_index)(vtx_outer_idx + (i0 << 1));
            ids[3] = (neko_gui_draw_index)(vtx_outer_idx + (i0 << 1));
            ids[4] = (neko_gui_draw_index)(vtx_outer_idx + (i1 << 1));
            ids[5] = (neko_gui_draw_index)(vtx_inner_idx + (i1 << 1));
            ids += 6;
        }

        neko_gui_buffer_reset(list->vertices, NEKO_GUI_BUFFER_FRONT);
    } else {
        neko_gui_size i = 0;
        neko_gui_size index = list->vertex_count;
        const neko_gui_size idx_count = (points_count - 2) * 3;
        const neko_gui_size vtx_count = points_count;
        void *vtx = neko_gui_draw_list_alloc_vertices(list, vtx_count);
        neko_gui_draw_index *ids = neko_gui_draw_list_alloc_elements(list, idx_count);

        if (!vtx || !ids) return;
        for (i = 0; i < vtx_count; ++i) vtx = neko_gui_draw_vertex(vtx, &list->config, points[i], list->config.tex_null.uv, col);
        for (i = 2; i < points_count; ++i) {
            ids[0] = (neko_gui_draw_index)index;
            ids[1] = (neko_gui_draw_index)(index + i - 1);
            ids[2] = (neko_gui_draw_index)(index + i);
            ids += 3;
        }
    }
}
NEKO_GUI_API void neko_gui_draw_list_path_clear(struct neko_gui_draw_list *list) {
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    neko_gui_buffer_reset(list->buffer, NEKO_GUI_BUFFER_FRONT);
    list->path_count = 0;
    list->path_offset = 0;
}
NEKO_GUI_API void neko_gui_draw_list_path_line_to(struct neko_gui_draw_list *list, struct neko_gui_vec2 pos) {
    struct neko_gui_vec2 *points = 0;
    struct neko_gui_draw_command *cmd = 0;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    if (!list->cmd_count) neko_gui_draw_list_add_clip(list, neko_gui_null_rect);

    cmd = neko_gui_draw_list_command_last(list);
    if (cmd && cmd->texture.ptr != list->config.tex_null.texture.ptr) neko_gui_draw_list_push_image(list, list->config.tex_null.texture);

    points = neko_gui_draw_list_alloc_path(list, 1);
    if (!points) return;
    points[0] = pos;
}
NEKO_GUI_API void neko_gui_draw_list_path_arc_to_fast(struct neko_gui_draw_list *list, struct neko_gui_vec2 center, float radius, int a_min, int a_max) {
    int a = 0;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    if (a_min <= a_max) {
        for (a = a_min; a <= a_max; a++) {
            const struct neko_gui_vec2 c = list->circle_vtx[(neko_gui_size)a % NEKO_GUI_LEN(list->circle_vtx)];
            const float x = center.x + c.x * radius;
            const float y = center.y + c.y * radius;
            neko_gui_draw_list_path_line_to(list, neko_gui_vec2(x, y));
        }
    }
}
NEKO_GUI_API void neko_gui_draw_list_path_arc_to(struct neko_gui_draw_list *list, struct neko_gui_vec2 center, float radius, float a_min, float a_max, unsigned int segments) {
    unsigned int i = 0;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    if (radius == 0.0f) return;

    {
        const float d_angle = (a_max - a_min) / (float)segments;
        const float sin_d = (float)NEKO_GUI_SIN(d_angle);
        const float cos_d = (float)NEKO_GUI_COS(d_angle);

        float cx = (float)NEKO_GUI_COS(a_min) * radius;
        float cy = (float)NEKO_GUI_SIN(a_min) * radius;
        for (i = 0; i <= segments; ++i) {
            float new_cx, new_cy;
            const float x = center.x + cx;
            const float y = center.y + cy;
            neko_gui_draw_list_path_line_to(list, neko_gui_vec2(x, y));

            new_cx = cx * cos_d - cy * sin_d;
            new_cy = cy * cos_d + cx * sin_d;
            cx = new_cx;
            cy = new_cy;
        }
    }
}
NEKO_GUI_API void neko_gui_draw_list_path_rect_to(struct neko_gui_draw_list *list, struct neko_gui_vec2 a, struct neko_gui_vec2 b, float rounding) {
    float r;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    r = rounding;
    r = NEKO_GUI_MIN(r, ((b.x - a.x) < 0) ? -(b.x - a.x) : (b.x - a.x));
    r = NEKO_GUI_MIN(r, ((b.y - a.y) < 0) ? -(b.y - a.y) : (b.y - a.y));

    if (r == 0.0f) {
        neko_gui_draw_list_path_line_to(list, a);
        neko_gui_draw_list_path_line_to(list, neko_gui_vec2(b.x, a.y));
        neko_gui_draw_list_path_line_to(list, b);
        neko_gui_draw_list_path_line_to(list, neko_gui_vec2(a.x, b.y));
    } else {
        neko_gui_draw_list_path_arc_to_fast(list, neko_gui_vec2(a.x + r, a.y + r), r, 6, 9);
        neko_gui_draw_list_path_arc_to_fast(list, neko_gui_vec2(b.x - r, a.y + r), r, 9, 12);
        neko_gui_draw_list_path_arc_to_fast(list, neko_gui_vec2(b.x - r, b.y - r), r, 0, 3);
        neko_gui_draw_list_path_arc_to_fast(list, neko_gui_vec2(a.x + r, b.y - r), r, 3, 6);
    }
}
NEKO_GUI_API void neko_gui_draw_list_path_curve_to(struct neko_gui_draw_list *list, struct neko_gui_vec2 p2, struct neko_gui_vec2 p3, struct neko_gui_vec2 p4, unsigned int num_segments) {
    float t_step;
    unsigned int i_step;
    struct neko_gui_vec2 p1;

    NEKO_GUI_ASSERT(list);
    NEKO_GUI_ASSERT(list->path_count);
    if (!list || !list->path_count) return;
    num_segments = NEKO_GUI_MAX(num_segments, 1);

    p1 = neko_gui_draw_list_path_last(list);
    t_step = 1.0f / (float)num_segments;
    for (i_step = 1; i_step <= num_segments; ++i_step) {
        float t = t_step * (float)i_step;
        float u = 1.0f - t;
        float w1 = u * u * u;
        float w2 = 3 * u * u * t;
        float w3 = 3 * u * t * t;
        float w4 = t * t * t;
        float x = w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x;
        float y = w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y;
        neko_gui_draw_list_path_line_to(list, neko_gui_vec2(x, y));
    }
}
NEKO_GUI_API void neko_gui_draw_list_path_fill(struct neko_gui_draw_list *list, struct neko_gui_color color) {
    struct neko_gui_vec2 *points;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    points = (struct neko_gui_vec2 *)neko_gui_buffer_memory(list->buffer);
    neko_gui_draw_list_fill_poly_convex(list, points, list->path_count, color, list->config.shape_AA);
    neko_gui_draw_list_path_clear(list);
}
NEKO_GUI_API void neko_gui_draw_list_path_stroke(struct neko_gui_draw_list *list, struct neko_gui_color color, enum neko_gui_draw_list_stroke closed, float thickness) {
    struct neko_gui_vec2 *points;
    NEKO_GUI_ASSERT(list);
    if (!list) return;
    points = (struct neko_gui_vec2 *)neko_gui_buffer_memory(list->buffer);
    neko_gui_draw_list_stroke_poly_line(list, points, list->path_count, color, closed, thickness, list->config.line_AA);
    neko_gui_draw_list_path_clear(list);
}
NEKO_GUI_API void neko_gui_draw_list_stroke_line(struct neko_gui_draw_list *list, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_color col, float thickness) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    if (list->line_AA == NEKO_GUI_ANTI_ALIASING_ON) {
        neko_gui_draw_list_path_line_to(list, a);
        neko_gui_draw_list_path_line_to(list, b);
    } else {
        neko_gui_draw_list_path_line_to(list, neko_gui_vec2_sub(a, neko_gui_vec2(0.5f, 0.5f)));
        neko_gui_draw_list_path_line_to(list, neko_gui_vec2_sub(b, neko_gui_vec2(0.5f, 0.5f)));
    }
    neko_gui_draw_list_path_stroke(list, col, NEKO_GUI_STROKE_OPEN, thickness);
}
NEKO_GUI_API void neko_gui_draw_list_fill_rect(struct neko_gui_draw_list *list, struct neko_gui_rect rect, struct neko_gui_color col, float rounding) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;

    if (list->line_AA == NEKO_GUI_ANTI_ALIASING_ON) {
        neko_gui_draw_list_path_rect_to(list, neko_gui_vec2(rect.x, rect.y), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), rounding);
    } else {
        neko_gui_draw_list_path_rect_to(list, neko_gui_vec2(rect.x - 0.5f, rect.y - 0.5f), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), rounding);
    }
    neko_gui_draw_list_path_fill(list, col);
}
NEKO_GUI_API void neko_gui_draw_list_stroke_rect(struct neko_gui_draw_list *list, struct neko_gui_rect rect, struct neko_gui_color col, float rounding, float thickness) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    if (list->line_AA == NEKO_GUI_ANTI_ALIASING_ON) {
        neko_gui_draw_list_path_rect_to(list, neko_gui_vec2(rect.x, rect.y), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), rounding);
    } else {
        neko_gui_draw_list_path_rect_to(list, neko_gui_vec2(rect.x - 0.5f, rect.y - 0.5f), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), rounding);
    }
    neko_gui_draw_list_path_stroke(list, col, NEKO_GUI_STROKE_CLOSED, thickness);
}
NEKO_GUI_API void neko_gui_draw_list_fill_rect_multi_color(struct neko_gui_draw_list *list, struct neko_gui_rect rect, struct neko_gui_color left, struct neko_gui_color top,
                                                           struct neko_gui_color right, struct neko_gui_color bottom) {
    void *vtx;
    struct neko_gui_colorf col_left, col_top;
    struct neko_gui_colorf col_right, col_bottom;
    neko_gui_draw_index *idx;
    neko_gui_draw_index index;

    neko_gui_color_fv(&col_left.r, left);
    neko_gui_color_fv(&col_right.r, right);
    neko_gui_color_fv(&col_top.r, top);
    neko_gui_color_fv(&col_bottom.r, bottom);

    NEKO_GUI_ASSERT(list);
    if (!list) return;

    neko_gui_draw_list_push_image(list, list->config.tex_null.texture);
    index = (neko_gui_draw_index)list->vertex_count;
    vtx = neko_gui_draw_list_alloc_vertices(list, 4);
    idx = neko_gui_draw_list_alloc_elements(list, 6);
    if (!vtx || !idx) return;

    idx[0] = (neko_gui_draw_index)(index + 0);
    idx[1] = (neko_gui_draw_index)(index + 1);
    idx[2] = (neko_gui_draw_index)(index + 2);
    idx[3] = (neko_gui_draw_index)(index + 0);
    idx[4] = (neko_gui_draw_index)(index + 2);
    idx[5] = (neko_gui_draw_index)(index + 3);

    vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(rect.x, rect.y), list->config.tex_null.uv, col_left);
    vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(rect.x + rect.w, rect.y), list->config.tex_null.uv, col_top);
    vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), list->config.tex_null.uv, col_right);
    vtx = neko_gui_draw_vertex(vtx, &list->config, neko_gui_vec2(rect.x, rect.y + rect.h), list->config.tex_null.uv, col_bottom);
}
NEKO_GUI_API void neko_gui_draw_list_fill_triangle(struct neko_gui_draw_list *list, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_vec2 c, struct neko_gui_color col) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    neko_gui_draw_list_path_line_to(list, a);
    neko_gui_draw_list_path_line_to(list, b);
    neko_gui_draw_list_path_line_to(list, c);
    neko_gui_draw_list_path_fill(list, col);
}
NEKO_GUI_API void neko_gui_draw_list_stroke_triangle(struct neko_gui_draw_list *list, struct neko_gui_vec2 a, struct neko_gui_vec2 b, struct neko_gui_vec2 c, struct neko_gui_color col,
                                                     float thickness) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    neko_gui_draw_list_path_line_to(list, a);
    neko_gui_draw_list_path_line_to(list, b);
    neko_gui_draw_list_path_line_to(list, c);
    neko_gui_draw_list_path_stroke(list, col, NEKO_GUI_STROKE_CLOSED, thickness);
}
NEKO_GUI_API void neko_gui_draw_list_fill_circle(struct neko_gui_draw_list *list, struct neko_gui_vec2 center, float radius, struct neko_gui_color col, unsigned int segs) {
    float a_max;
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    a_max = NEKO_GUI_PI * 2.0f * ((float)segs - 1.0f) / (float)segs;
    neko_gui_draw_list_path_arc_to(list, center, radius, 0.0f, a_max, segs);
    neko_gui_draw_list_path_fill(list, col);
}
NEKO_GUI_API void neko_gui_draw_list_stroke_circle(struct neko_gui_draw_list *list, struct neko_gui_vec2 center, float radius, struct neko_gui_color col, unsigned int segs, float thickness) {
    float a_max;
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    a_max = NEKO_GUI_PI * 2.0f * ((float)segs - 1.0f) / (float)segs;
    neko_gui_draw_list_path_arc_to(list, center, radius, 0.0f, a_max, segs);
    neko_gui_draw_list_path_stroke(list, col, NEKO_GUI_STROKE_CLOSED, thickness);
}
NEKO_GUI_API void neko_gui_draw_list_stroke_curve(struct neko_gui_draw_list *list, struct neko_gui_vec2 p0, struct neko_gui_vec2 cp0, struct neko_gui_vec2 cp1, struct neko_gui_vec2 p1,
                                                  struct neko_gui_color col, unsigned int segments, float thickness) {
    NEKO_GUI_ASSERT(list);
    if (!list || !col.a) return;
    neko_gui_draw_list_path_line_to(list, p0);
    neko_gui_draw_list_path_curve_to(list, cp0, cp1, p1, segments);
    neko_gui_draw_list_path_stroke(list, col, NEKO_GUI_STROKE_OPEN, thickness);
}
NEKO_GUI_INTERN void neko_gui_draw_list_push_rect_uv(struct neko_gui_draw_list *list, struct neko_gui_vec2 a, struct neko_gui_vec2 c, struct neko_gui_vec2 uva, struct neko_gui_vec2 uvc,
                                                     struct neko_gui_color color) {
    void *vtx;
    struct neko_gui_vec2 uvb;
    struct neko_gui_vec2 uvd;
    struct neko_gui_vec2 b;
    struct neko_gui_vec2 d;

    struct neko_gui_colorf col;
    neko_gui_draw_index *idx;
    neko_gui_draw_index index;
    NEKO_GUI_ASSERT(list);
    if (!list) return;

    neko_gui_color_fv(&col.r, color);
    uvb = neko_gui_vec2(uvc.x, uva.y);
    uvd = neko_gui_vec2(uva.x, uvc.y);
    b = neko_gui_vec2(c.x, a.y);
    d = neko_gui_vec2(a.x, c.y);

    index = (neko_gui_draw_index)list->vertex_count;
    vtx = neko_gui_draw_list_alloc_vertices(list, 4);
    idx = neko_gui_draw_list_alloc_elements(list, 6);
    if (!vtx || !idx) return;

    idx[0] = (neko_gui_draw_index)(index + 0);
    idx[1] = (neko_gui_draw_index)(index + 1);
    idx[2] = (neko_gui_draw_index)(index + 2);
    idx[3] = (neko_gui_draw_index)(index + 0);
    idx[4] = (neko_gui_draw_index)(index + 2);
    idx[5] = (neko_gui_draw_index)(index + 3);

    vtx = neko_gui_draw_vertex(vtx, &list->config, a, uva, col);
    vtx = neko_gui_draw_vertex(vtx, &list->config, b, uvb, col);
    vtx = neko_gui_draw_vertex(vtx, &list->config, c, uvc, col);
    vtx = neko_gui_draw_vertex(vtx, &list->config, d, uvd, col);
}
NEKO_GUI_API void neko_gui_draw_list_add_image(struct neko_gui_draw_list *list, struct neko_gui_image texture, struct neko_gui_rect rect, struct neko_gui_color color) {
    NEKO_GUI_ASSERT(list);
    if (!list) return;

    neko_gui_draw_list_push_image(list, texture.handle);
    if (neko_gui_image_is_subimage(&texture)) {

        struct neko_gui_vec2 uv[2];
        uv[0].x = (float)texture.region[0] / (float)texture.w;
        uv[0].y = (float)texture.region[1] / (float)texture.h;
        uv[1].x = (float)(texture.region[0] + texture.region[2]) / (float)texture.w;
        uv[1].y = (float)(texture.region[1] + texture.region[3]) / (float)texture.h;
        neko_gui_draw_list_push_rect_uv(list, neko_gui_vec2(rect.x, rect.y), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), uv[0], uv[1], color);
    } else
        neko_gui_draw_list_push_rect_uv(list, neko_gui_vec2(rect.x, rect.y), neko_gui_vec2(rect.x + rect.w, rect.y + rect.h), neko_gui_vec2(0.0f, 0.0f), neko_gui_vec2(1.0f, 1.0f), color);
}
NEKO_GUI_API void neko_gui_draw_list_add_text(struct neko_gui_draw_list *list, const struct neko_gui_user_font *font, struct neko_gui_rect rect, const char *text, int len, float font_height,
                                              struct neko_gui_color fg) {
    float x = 0;
    int text_len = 0;
    neko_gui_rune unicode = 0;
    neko_gui_rune next = 0;
    int glyph_len = 0;
    int next_glyph_len = 0;
    struct neko_gui_user_font_glyph g;

    NEKO_GUI_ASSERT(list);
    if (!list || !len || !text) return;
    if (!NEKO_GUI_INTERSECT(rect.x, rect.y, rect.w, rect.h, list->clip_rect.x, list->clip_rect.y, list->clip_rect.w, list->clip_rect.h)) return;

    neko_gui_draw_list_push_image(list, font->texture);
    x = rect.x;
    glyph_len = neko_gui_utf_decode(text, &unicode, len);
    if (!glyph_len) return;

    fg.a = (neko_gui_byte)((float)fg.a * list->config.global_alpha);
    while (text_len < len && glyph_len) {
        float gx, gy, gh, gw;
        float char_width = 0;
        if (unicode == NEKO_GUI_UTF_INVALID) break;

        next_glyph_len = neko_gui_utf_decode(text + text_len + glyph_len, &next, (int)len - text_len);
        font->query(font->userdata, font_height, &g, unicode, (next == NEKO_GUI_UTF_INVALID) ? '\0' : next);

        gx = x + g.offset.x;
        gy = rect.y + g.offset.y;
        gw = g.width;
        gh = g.height;
        char_width = g.xadvance;
        neko_gui_draw_list_push_rect_uv(list, neko_gui_vec2(gx, gy), neko_gui_vec2(gx + gw, gy + gh), g.uv[0], g.uv[1], fg);

        text_len += glyph_len;
        x += char_width;
        glyph_len = next_glyph_len;
        unicode = next;
    }
}
NEKO_GUI_API neko_gui_flags neko_gui_convert(struct neko_gui_context *ctx, struct neko_gui_buffer *cmds, struct neko_gui_buffer *vertices, struct neko_gui_buffer *elements,
                                             const struct neko_gui_convert_config *config) {
    neko_gui_flags res = NEKO_GUI_CONVERT_SUCCESS;
    const struct neko_gui_command *cmd;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(cmds);
    NEKO_GUI_ASSERT(vertices);
    NEKO_GUI_ASSERT(elements);
    NEKO_GUI_ASSERT(config);
    NEKO_GUI_ASSERT(config->vertex_layout);
    NEKO_GUI_ASSERT(config->vertex_size);
    if (!ctx || !cmds || !vertices || !elements || !config || !config->vertex_layout) return NEKO_GUI_CONVERT_INVALID_PARAM;

    neko_gui_draw_list_setup(&ctx->draw_list, config, cmds, vertices, elements, config->line_AA, config->shape_AA);
    neko_gui_foreach(cmd, ctx) {
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
        ctx->draw_list.userdata = cmd->userdata;
#endif
        switch (cmd->type) {
            case NEKO_GUI_COMMAND_NOP:
                break;
            case NEKO_GUI_COMMAND_SCISSOR: {
                const struct neko_gui_command_scissor *s = (const struct neko_gui_command_scissor *)cmd;
                neko_gui_draw_list_add_clip(&ctx->draw_list, neko_gui_rect(s->x, s->y, s->w, s->h));
            } break;
            case NEKO_GUI_COMMAND_LINE: {
                const struct neko_gui_command_line *l = (const struct neko_gui_command_line *)cmd;
                neko_gui_draw_list_stroke_line(&ctx->draw_list, neko_gui_vec2(l->begin.x, l->begin.y), neko_gui_vec2(l->end.x, l->end.y), l->color, l->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_CURVE: {
                const struct neko_gui_command_curve *q = (const struct neko_gui_command_curve *)cmd;
                neko_gui_draw_list_stroke_curve(&ctx->draw_list, neko_gui_vec2(q->begin.x, q->begin.y), neko_gui_vec2(q->ctrl[0].x, q->ctrl[0].y), neko_gui_vec2(q->ctrl[1].x, q->ctrl[1].y),
                                                neko_gui_vec2(q->end.x, q->end.y), q->color, config->curve_segment_count, q->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_RECT: {
                const struct neko_gui_command_rect *r = (const struct neko_gui_command_rect *)cmd;
                neko_gui_draw_list_stroke_rect(&ctx->draw_list, neko_gui_rect(r->x, r->y, r->w, r->h), r->color, (float)r->rounding, r->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_RECT_FILLED: {
                const struct neko_gui_command_rect_filled *r = (const struct neko_gui_command_rect_filled *)cmd;
                neko_gui_draw_list_fill_rect(&ctx->draw_list, neko_gui_rect(r->x, r->y, r->w, r->h), r->color, (float)r->rounding);
            } break;
            case NEKO_GUI_COMMAND_RECT_MULTI_COLOR: {
                const struct neko_gui_command_rect_multi_color *r = (const struct neko_gui_command_rect_multi_color *)cmd;
                neko_gui_draw_list_fill_rect_multi_color(&ctx->draw_list, neko_gui_rect(r->x, r->y, r->w, r->h), r->left, r->top, r->right, r->bottom);
            } break;
            case NEKO_GUI_COMMAND_CIRCLE: {
                const struct neko_gui_command_circle *c = (const struct neko_gui_command_circle *)cmd;
                neko_gui_draw_list_stroke_circle(&ctx->draw_list, neko_gui_vec2((float)c->x + (float)c->w / 2, (float)c->y + (float)c->h / 2), (float)c->w / 2, c->color, config->circle_segment_count,
                                                 c->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_CIRCLE_FILLED: {
                const struct neko_gui_command_circle_filled *c = (const struct neko_gui_command_circle_filled *)cmd;
                neko_gui_draw_list_fill_circle(&ctx->draw_list, neko_gui_vec2((float)c->x + (float)c->w / 2, (float)c->y + (float)c->h / 2), (float)c->w / 2, c->color, config->circle_segment_count);
            } break;
            case NEKO_GUI_COMMAND_ARC: {
                const struct neko_gui_command_arc *c = (const struct neko_gui_command_arc *)cmd;
                neko_gui_draw_list_path_line_to(&ctx->draw_list, neko_gui_vec2(c->cx, c->cy));
                neko_gui_draw_list_path_arc_to(&ctx->draw_list, neko_gui_vec2(c->cx, c->cy), c->r, c->a[0], c->a[1], config->arc_segment_count);
                neko_gui_draw_list_path_stroke(&ctx->draw_list, c->color, NEKO_GUI_STROKE_CLOSED, c->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_ARC_FILLED: {
                const struct neko_gui_command_arc_filled *c = (const struct neko_gui_command_arc_filled *)cmd;
                neko_gui_draw_list_path_line_to(&ctx->draw_list, neko_gui_vec2(c->cx, c->cy));
                neko_gui_draw_list_path_arc_to(&ctx->draw_list, neko_gui_vec2(c->cx, c->cy), c->r, c->a[0], c->a[1], config->arc_segment_count);
                neko_gui_draw_list_path_fill(&ctx->draw_list, c->color);
            } break;
            case NEKO_GUI_COMMAND_TRIANGLE: {
                const struct neko_gui_command_triangle *t = (const struct neko_gui_command_triangle *)cmd;
                neko_gui_draw_list_stroke_triangle(&ctx->draw_list, neko_gui_vec2(t->a.x, t->a.y), neko_gui_vec2(t->b.x, t->b.y), neko_gui_vec2(t->c.x, t->c.y), t->color, t->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_TRIANGLE_FILLED: {
                const struct neko_gui_command_triangle_filled *t = (const struct neko_gui_command_triangle_filled *)cmd;
                neko_gui_draw_list_fill_triangle(&ctx->draw_list, neko_gui_vec2(t->a.x, t->a.y), neko_gui_vec2(t->b.x, t->b.y), neko_gui_vec2(t->c.x, t->c.y), t->color);
            } break;
            case NEKO_GUI_COMMAND_POLYGON: {
                int i;
                const struct neko_gui_command_polygon *p = (const struct neko_gui_command_polygon *)cmd;
                for (i = 0; i < p->point_count; ++i) {
                    struct neko_gui_vec2 pnt = neko_gui_vec2((float)p->points[i].x, (float)p->points[i].y);
                    neko_gui_draw_list_path_line_to(&ctx->draw_list, pnt);
                }
                neko_gui_draw_list_path_stroke(&ctx->draw_list, p->color, NEKO_GUI_STROKE_CLOSED, p->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_POLYGON_FILLED: {
                int i;
                const struct neko_gui_command_polygon_filled *p = (const struct neko_gui_command_polygon_filled *)cmd;
                for (i = 0; i < p->point_count; ++i) {
                    struct neko_gui_vec2 pnt = neko_gui_vec2((float)p->points[i].x, (float)p->points[i].y);
                    neko_gui_draw_list_path_line_to(&ctx->draw_list, pnt);
                }
                neko_gui_draw_list_path_fill(&ctx->draw_list, p->color);
            } break;
            case NEKO_GUI_COMMAND_POLYLINE: {
                int i;
                const struct neko_gui_command_polyline *p = (const struct neko_gui_command_polyline *)cmd;
                for (i = 0; i < p->point_count; ++i) {
                    struct neko_gui_vec2 pnt = neko_gui_vec2((float)p->points[i].x, (float)p->points[i].y);
                    neko_gui_draw_list_path_line_to(&ctx->draw_list, pnt);
                }
                neko_gui_draw_list_path_stroke(&ctx->draw_list, p->color, NEKO_GUI_STROKE_OPEN, p->line_thickness);
            } break;
            case NEKO_GUI_COMMAND_TEXT: {
                const struct neko_gui_command_text *t = (const struct neko_gui_command_text *)cmd;
                neko_gui_draw_list_add_text(&ctx->draw_list, t->font, neko_gui_rect(t->x, t->y, t->w, t->h), t->string, t->length, t->height, t->foreground);
            } break;
            case NEKO_GUI_COMMAND_IMAGE: {
                const struct neko_gui_command_image *i = (const struct neko_gui_command_image *)cmd;
                neko_gui_draw_list_add_image(&ctx->draw_list, i->img, neko_gui_rect(i->x, i->y, i->w, i->h), i->col);
            } break;
            case NEKO_GUI_COMMAND_CUSTOM: {
                const struct neko_gui_command_custom *c = (const struct neko_gui_command_custom *)cmd;
                c->callback(&ctx->draw_list, c->x, c->y, c->w, c->h, c->callback_data);
            } break;
            default:
                break;
        }
    }
    res |= (cmds->needed > cmds->allocated + (cmds->memory.size - cmds->size)) ? NEKO_GUI_CONVERT_COMMAND_BUFFER_FULL : 0;
    res |= (vertices->needed > vertices->allocated) ? NEKO_GUI_CONVERT_VERTEX_BUFFER_FULL : 0;
    res |= (elements->needed > elements->allocated) ? NEKO_GUI_CONVERT_ELEMENT_BUFFER_FULL : 0;
    return res;
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_begin(const struct neko_gui_context *ctx, const struct neko_gui_buffer *buffer) {
    return neko_gui__draw_list_begin(&ctx->draw_list, buffer);
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_end(const struct neko_gui_context *ctx, const struct neko_gui_buffer *buffer) {
    return neko_gui__draw_list_end(&ctx->draw_list, buffer);
}
NEKO_GUI_API const struct neko_gui_draw_command *neko_gui__draw_next(const struct neko_gui_draw_command *cmd, const struct neko_gui_buffer *buffer, const struct neko_gui_context *ctx) {
    return neko_gui__draw_list_next(cmd, buffer, &ctx->draw_list);
}
#endif

#ifdef NEKO_GUI_INCLUDE_FONT_BAKING

#include "libs/stb/stb_rect_pack.h"

#define STBTT_MAX_OVERSAMPLE 8
#include "libs/stb/stb_truetype.h"

struct neko_gui_font_bake_data {
    struct stbtt_fontinfo info;
    struct stbrp_rect *rects;
    stbtt_pack_range *ranges;
    neko_gui_rune range_count;
};

struct neko_gui_font_baker {
    struct neko_gui_allocator alloc;
    struct stbtt_pack_context spc;
    struct neko_gui_font_bake_data *build;
    stbtt_packedchar *packed_chars;
    struct stbrp_rect *rects;
    stbtt_pack_range *ranges;
};

NEKO_GUI_GLOBAL const neko_gui_size neko_gui_rect_align = NEKO_GUI_ALIGNOF(struct stbrp_rect);
NEKO_GUI_GLOBAL const neko_gui_size neko_gui_range_align = NEKO_GUI_ALIGNOF(stbtt_pack_range);
NEKO_GUI_GLOBAL const neko_gui_size neko_gui_char_align = NEKO_GUI_ALIGNOF(stbtt_packedchar);
NEKO_GUI_GLOBAL const neko_gui_size neko_gui_build_align = NEKO_GUI_ALIGNOF(struct neko_gui_font_bake_data);
NEKO_GUI_GLOBAL const neko_gui_size neko_gui_baker_align = NEKO_GUI_ALIGNOF(struct neko_gui_font_baker);

NEKO_GUI_INTERN int neko_gui_range_count(const neko_gui_rune *range) {
    const neko_gui_rune *iter = range;
    NEKO_GUI_ASSERT(range);
    if (!range) return 0;
    while (*(iter++) != 0)
        ;
    return (iter == range) ? 0 : (int)((iter - range) / 2);
}
NEKO_GUI_INTERN int neko_gui_range_glyph_count(const neko_gui_rune *range, int count) {
    int i = 0;
    int total_glyphs = 0;
    for (i = 0; i < count; ++i) {
        int diff;
        neko_gui_rune f = range[(i * 2) + 0];
        neko_gui_rune t = range[(i * 2) + 1];
        NEKO_GUI_ASSERT(t >= f);
        diff = (int)((t - f) + 1);
        total_glyphs += diff;
    }
    return total_glyphs;
}
NEKO_GUI_API const neko_gui_rune *neko_gui_font_default_glyph_ranges(void) {
    NEKO_GUI_STORAGE const neko_gui_rune ranges[] = {0x0020, 0x00FF, 0};
    return ranges;
}
NEKO_GUI_API const neko_gui_rune *neko_gui_font_chinese_glyph_ranges(void) {
    NEKO_GUI_STORAGE const neko_gui_rune ranges[] = {0x0020, 0x00FF, 0x3000, 0x30FF, 0x31F0, 0x31FF, 0xFF00, 0xFFEF, 0x4E00, 0x9FAF, 0};
    return ranges;
}
NEKO_GUI_API const neko_gui_rune *neko_gui_font_cyrillic_glyph_ranges(void) {
    NEKO_GUI_STORAGE const neko_gui_rune ranges[] = {0x0020, 0x00FF, 0x0400, 0x052F, 0x2DE0, 0x2DFF, 0xA640, 0xA69F, 0};
    return ranges;
}
NEKO_GUI_API const neko_gui_rune *neko_gui_font_korean_glyph_ranges(void) {
    NEKO_GUI_STORAGE const neko_gui_rune ranges[] = {0x0020, 0x00FF, 0x3131, 0x3163, 0xAC00, 0xD79D, 0};
    return ranges;
}
NEKO_GUI_INTERN void neko_gui_font_baker_memory(neko_gui_size *temp, int *glyph_count, struct neko_gui_font_config *config_list, int count) {
    int range_count = 0;
    int total_range_count = 0;
    struct neko_gui_font_config *iter, *i;

    NEKO_GUI_ASSERT(config_list);
    NEKO_GUI_ASSERT(glyph_count);
    if (!config_list) {
        *temp = 0;
        *glyph_count = 0;
        return;
    }
    *glyph_count = 0;
    for (iter = config_list; iter; iter = iter->next) {
        i = iter;
        do {
            if (!i->range) iter->range = neko_gui_font_default_glyph_ranges();
            range_count = neko_gui_range_count(i->range);
            total_range_count += range_count;
            *glyph_count += neko_gui_range_glyph_count(i->range, range_count);
        } while ((i = i->n) != iter);
    }
    *temp = (neko_gui_size)*glyph_count * sizeof(struct stbrp_rect);
    *temp += (neko_gui_size)total_range_count * sizeof(stbtt_pack_range);
    *temp += (neko_gui_size)*glyph_count * sizeof(stbtt_packedchar);
    *temp += (neko_gui_size)count * sizeof(struct neko_gui_font_bake_data);
    *temp += sizeof(struct neko_gui_font_baker);
    *temp += neko_gui_rect_align + neko_gui_range_align + neko_gui_char_align;
    *temp += neko_gui_build_align + neko_gui_baker_align;
}
NEKO_GUI_INTERN struct neko_gui_font_baker *neko_gui_font_baker(void *memory, int glyph_count, int count, struct neko_gui_allocator *alloc) {
    struct neko_gui_font_baker *baker;
    if (!memory) return 0;

    baker = (struct neko_gui_font_baker *)NEKO_GUI_ALIGN_PTR(memory, neko_gui_baker_align);
    baker->build = (struct neko_gui_font_bake_data *)NEKO_GUI_ALIGN_PTR((baker + 1), neko_gui_build_align);
    baker->packed_chars = (stbtt_packedchar *)NEKO_GUI_ALIGN_PTR((baker->build + count), neko_gui_char_align);
    baker->rects = (struct stbrp_rect *)NEKO_GUI_ALIGN_PTR((baker->packed_chars + glyph_count), neko_gui_rect_align);
    baker->ranges = (stbtt_pack_range *)NEKO_GUI_ALIGN_PTR((baker->rects + glyph_count), neko_gui_range_align);
    baker->alloc = *alloc;
    return baker;
}
NEKO_GUI_INTERN int neko_gui_font_bake_pack(struct neko_gui_font_baker *baker, neko_gui_size *image_memory, int *width, int *height, struct neko_gui_recti *custom,
                                            const struct neko_gui_font_config *config_list, int count, struct neko_gui_allocator *alloc) {
    NEKO_GUI_STORAGE const neko_gui_size max_height = 1024 * 32;
    const struct neko_gui_font_config *config_iter, *it;
    int total_glyph_count = 0;
    int total_range_count = 0;
    int range_count = 0;
    int i = 0;

    NEKO_GUI_ASSERT(image_memory);
    NEKO_GUI_ASSERT(width);
    NEKO_GUI_ASSERT(height);
    NEKO_GUI_ASSERT(config_list);
    NEKO_GUI_ASSERT(count);
    NEKO_GUI_ASSERT(alloc);

    if (!image_memory || !width || !height || !config_list || !count) return neko_gui_false;
    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {
            range_count = neko_gui_range_count(it->range);
            total_range_count += range_count;
            total_glyph_count += neko_gui_range_glyph_count(it->range, range_count);
        } while ((it = it->n) != config_iter);
    }

    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {
            struct stbtt_fontinfo *font_info = &baker->build[i++].info;
            font_info->userdata = alloc;

            if (!stbtt_InitFont(font_info, (const unsigned char *)it->ttf_blob, stbtt_GetFontOffsetForIndex((const unsigned char *)it->ttf_blob, 0))) return neko_gui_false;
        } while ((it = it->n) != config_iter);
    }
    *height = 0;
    *width = (total_glyph_count > 1000) ? 1024 : 512;
    stbtt_PackBegin(&baker->spc, 0, (int)*width, (int)max_height, 0, 1, alloc);
    {
        int input_i = 0;
        int range_n = 0;
        int rect_n = 0;
        int char_n = 0;

        if (custom) {

            struct stbrp_rect custom_space;
            neko_gui_zero(&custom_space, sizeof(custom_space));
            custom_space.w = (stbrp_coord)(custom->w);
            custom_space.h = (stbrp_coord)(custom->h);

            stbtt_PackSetOversampling(&baker->spc, 1, 1);
            stbrp_pack_rects((struct stbrp_context *)baker->spc.pack_info, &custom_space, 1);
            *height = NEKO_GUI_MAX(*height, (int)(custom_space.y + custom_space.h));

            custom->x = (short)custom_space.x;
            custom->y = (short)custom_space.y;
            custom->w = (short)custom_space.w;
            custom->h = (short)custom_space.h;
        }

        for (input_i = 0, config_iter = config_list; input_i < count && config_iter; config_iter = config_iter->next) {
            it = config_iter;
            do {
                int n = 0;
                int glyph_count;
                const neko_gui_rune *in_range;
                const struct neko_gui_font_config *cfg = it;
                struct neko_gui_font_bake_data *tmp = &baker->build[input_i++];

                glyph_count = 0;
                range_count = 0;
                for (in_range = cfg->range; in_range[0] && in_range[1]; in_range += 2) {
                    glyph_count += (int)(in_range[1] - in_range[0]) + 1;
                    range_count++;
                }

                tmp->ranges = baker->ranges + range_n;
                tmp->range_count = (neko_gui_rune)range_count;
                range_n += range_count;
                for (i = 0; i < range_count; ++i) {
                    in_range = &cfg->range[i * 2];
                    tmp->ranges[i].font_size = cfg->size;
                    tmp->ranges[i].first_unicode_codepoint_in_range = (int)in_range[0];
                    tmp->ranges[i].num_chars = (int)(in_range[1] - in_range[0]) + 1;
                    tmp->ranges[i].chardata_for_range = baker->packed_chars + char_n;
                    char_n += tmp->ranges[i].num_chars;
                }

                tmp->rects = baker->rects + rect_n;
                rect_n += glyph_count;
                stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
                n = stbtt_PackFontRangesGatherRects(&baker->spc, &tmp->info, tmp->ranges, (int)tmp->range_count, tmp->rects);
                stbrp_pack_rects((struct stbrp_context *)baker->spc.pack_info, tmp->rects, (int)n);

                for (i = 0; i < n; ++i) {
                    if (tmp->rects[i].was_packed) *height = NEKO_GUI_MAX(*height, tmp->rects[i].y + tmp->rects[i].h);
                }
            } while ((it = it->n) != config_iter);
        }
        NEKO_GUI_ASSERT(rect_n == total_glyph_count);
        NEKO_GUI_ASSERT(char_n == total_glyph_count);
        NEKO_GUI_ASSERT(range_n == total_range_count);
    }
    *height = (int)neko_gui_round_up_pow2((neko_gui_uint)*height);
    *image_memory = (neko_gui_size)(*width) * (neko_gui_size)(*height);
    return neko_gui_true;
}
NEKO_GUI_INTERN void neko_gui_font_bake(struct neko_gui_font_baker *baker, void *image_memory, int width, int height, struct neko_gui_font_glyph *glyphs, int glyphs_count,
                                        const struct neko_gui_font_config *config_list, int font_count) {
    int input_i = 0;
    neko_gui_rune glyph_n = 0;
    const struct neko_gui_font_config *config_iter;
    const struct neko_gui_font_config *it;

    NEKO_GUI_ASSERT(image_memory);
    NEKO_GUI_ASSERT(width);
    NEKO_GUI_ASSERT(height);
    NEKO_GUI_ASSERT(config_list);
    NEKO_GUI_ASSERT(baker);
    NEKO_GUI_ASSERT(font_count);
    NEKO_GUI_ASSERT(glyphs_count);
    if (!image_memory || !width || !height || !config_list || !font_count || !glyphs || !glyphs_count) return;

    neko_gui_zero(image_memory, (neko_gui_size)((neko_gui_size)width * (neko_gui_size)height));
    baker->spc.pixels = (unsigned char *)image_memory;
    baker->spc.height = (int)height;
    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {
            const struct neko_gui_font_config *cfg = it;
            struct neko_gui_font_bake_data *tmp = &baker->build[input_i++];
            stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
            stbtt_PackFontRangesRenderIntoRects(&baker->spc, &tmp->info, tmp->ranges, (int)tmp->range_count, tmp->rects);
        } while ((it = it->n) != config_iter);
    }
    stbtt_PackEnd(&baker->spc);

    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {
            neko_gui_size i = 0;
            int char_idx = 0;
            neko_gui_rune glyph_count = 0;
            const struct neko_gui_font_config *cfg = it;
            struct neko_gui_font_bake_data *tmp = &baker->build[input_i++];
            struct neko_gui_baked_font *dst_font = cfg->font;

            float font_scale = stbtt_ScaleForPixelHeight(&tmp->info, cfg->size);
            int unscaled_ascent, unscaled_descent, unscaled_line_gap;
            stbtt_GetFontVMetrics(&tmp->info, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

            if (!cfg->merge_mode) {
                dst_font->ranges = cfg->range;
                dst_font->height = cfg->size;
                dst_font->ascent = ((float)unscaled_ascent * font_scale);
                dst_font->descent = ((float)unscaled_descent * font_scale);
                dst_font->glyph_offset = glyph_n;

                dst_font->glyph_count = 0;
            }

            for (i = 0; i < tmp->range_count; ++i) {
                stbtt_pack_range *range = &tmp->ranges[i];
                for (char_idx = 0; char_idx < range->num_chars; char_idx++) {
                    neko_gui_rune codepoint = 0;
                    float dummy_x = 0, dummy_y = 0;
                    stbtt_aligned_quad q;
                    struct neko_gui_font_glyph *glyph;

                    const stbtt_packedchar *pc = &range->chardata_for_range[char_idx];
                    codepoint = (neko_gui_rune)(range->first_unicode_codepoint_in_range + char_idx);
                    stbtt_GetPackedQuad(range->chardata_for_range, (int)width, (int)height, char_idx, &dummy_x, &dummy_y, &q, 0);

                    glyph = &glyphs[dst_font->glyph_offset + dst_font->glyph_count + (unsigned int)glyph_count];
                    glyph->codepoint = codepoint;
                    glyph->x0 = q.x0;
                    glyph->y0 = q.y0;
                    glyph->x1 = q.x1;
                    glyph->y1 = q.y1;
                    glyph->y0 += (dst_font->ascent + 0.5f);
                    glyph->y1 += (dst_font->ascent + 0.5f);
                    glyph->w = glyph->x1 - glyph->x0 + 0.5f;
                    glyph->h = glyph->y1 - glyph->y0;

                    if (cfg->coord_type == NEKO_GUI_COORD_PIXEL) {
                        glyph->u0 = q.s0 * (float)width;
                        glyph->v0 = q.t0 * (float)height;
                        glyph->u1 = q.s1 * (float)width;
                        glyph->v1 = q.t1 * (float)height;
                    } else {
                        glyph->u0 = q.s0;
                        glyph->v0 = q.t0;
                        glyph->u1 = q.s1;
                        glyph->v1 = q.t1;
                    }
                    glyph->xadvance = (pc->xadvance + cfg->spacing.x);
                    if (cfg->pixel_snap) glyph->xadvance = (float)(int)(glyph->xadvance + 0.5f);
                    glyph_count++;
                }
            }
            dst_font->glyph_count += glyph_count;
            glyph_n += glyph_count;
        } while ((it = it->n) != config_iter);
    }
}
NEKO_GUI_INTERN void neko_gui_font_bake_custom_data(void *img_memory, int img_width, int img_height, struct neko_gui_recti img_dst, const char *texture_data_mask, int tex_width, int tex_height,
                                                    char white, char black) {
    neko_gui_byte *pixels;
    int y = 0;
    int x = 0;
    int n = 0;

    NEKO_GUI_ASSERT(img_memory);
    NEKO_GUI_ASSERT(img_width);
    NEKO_GUI_ASSERT(img_height);
    NEKO_GUI_ASSERT(texture_data_mask);
    NEKO_GUI_UNUSED(tex_height);
    if (!img_memory || !img_width || !img_height || !texture_data_mask) return;

    pixels = (neko_gui_byte *)img_memory;
    for (y = 0, n = 0; y < tex_height; ++y) {
        for (x = 0; x < tex_width; ++x, ++n) {
            const int off0 = ((img_dst.x + x) + (img_dst.y + y) * img_width);
            const int off1 = off0 + 1 + tex_width;
            pixels[off0] = (texture_data_mask[n] == white) ? 0xFF : 0x00;
            pixels[off1] = (texture_data_mask[n] == black) ? 0xFF : 0x00;
        }
    }
}
NEKO_GUI_INTERN void neko_gui_font_bake_convert(void *out_memory, int img_width, int img_height, const void *in_memory) {
    int n = 0;
    neko_gui_rune *dst;
    const neko_gui_byte *src;

    NEKO_GUI_ASSERT(out_memory);
    NEKO_GUI_ASSERT(in_memory);
    NEKO_GUI_ASSERT(img_width);
    NEKO_GUI_ASSERT(img_height);
    if (!out_memory || !in_memory || !img_height || !img_width) return;

    dst = (neko_gui_rune *)out_memory;
    src = (const neko_gui_byte *)in_memory;
    for (n = (int)(img_width * img_height); n > 0; n--) *dst++ = ((neko_gui_rune)(*src++) << 24) | 0x00FFFFFF;
}

NEKO_GUI_INTERN float neko_gui_font_text_width(neko_gui_handle handle, float height, const char *text, int len) {
    neko_gui_rune unicode;
    int text_len = 0;
    float text_width = 0;
    int glyph_len = 0;
    float scale = 0;

    struct neko_gui_font *font = (struct neko_gui_font *)handle.ptr;
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(font->glyphs);
    if (!font || !text || !len) return 0;

    scale = height / font->info.height;
    glyph_len = text_len = neko_gui_utf_decode(text, &unicode, (int)len);
    if (!glyph_len) return 0;
    while (text_len <= (int)len && glyph_len) {
        const struct neko_gui_font_glyph *g;
        if (unicode == NEKO_GUI_UTF_INVALID) break;

        g = neko_gui_font_find_glyph(font, unicode);
        text_width += g->xadvance * scale;

        glyph_len = neko_gui_utf_decode(text + text_len, &unicode, (int)len - text_len);
        text_len += glyph_len;
    }
    return text_width;
}
#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
NEKO_GUI_INTERN void neko_gui_font_query_font_glyph(neko_gui_handle handle, float height, struct neko_gui_user_font_glyph *glyph, neko_gui_rune codepoint, neko_gui_rune next_codepoint) {
    float scale;
    const struct neko_gui_font_glyph *g;
    struct neko_gui_font *font;

    NEKO_GUI_ASSERT(glyph);
    NEKO_GUI_UNUSED(next_codepoint);

    font = (struct neko_gui_font *)handle.ptr;
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(font->glyphs);
    if (!font || !glyph) return;

    scale = height / font->info.height;
    g = neko_gui_font_find_glyph(font, codepoint);
    glyph->width = (g->x1 - g->x0) * scale;
    glyph->height = (g->y1 - g->y0) * scale;
    glyph->offset = neko_gui_vec2(g->x0 * scale, g->y0 * scale);
    glyph->xadvance = (g->xadvance * scale);
    glyph->uv[0] = neko_gui_vec2(g->u0, g->v0);
    glyph->uv[1] = neko_gui_vec2(g->u1, g->v1);
}
#endif
NEKO_GUI_API const struct neko_gui_font_glyph *neko_gui_font_find_glyph(struct neko_gui_font *font, neko_gui_rune unicode) {
    int i = 0;
    int count;
    int total_glyphs = 0;
    const struct neko_gui_font_glyph *glyph = 0;
    const struct neko_gui_font_config *iter = 0;

    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(font->glyphs);
    NEKO_GUI_ASSERT(font->info.ranges);
    if (!font || !font->glyphs) return 0;

    glyph = font->fallback;
    iter = font->config;
    do {
        count = neko_gui_range_count(iter->range);
        for (i = 0; i < count; ++i) {
            neko_gui_rune f = iter->range[(i * 2) + 0];
            neko_gui_rune t = iter->range[(i * 2) + 1];
            int diff = (int)((t - f) + 1);
            if (unicode >= f && unicode <= t) return &font->glyphs[((neko_gui_rune)total_glyphs + (unicode - f))];
            total_glyphs += diff;
        }
    } while ((iter = iter->n) != font->config);
    return glyph;
}
NEKO_GUI_INTERN void neko_gui_font_init(struct neko_gui_font *font, float pixel_height, neko_gui_rune fallback_codepoint, struct neko_gui_font_glyph *glyphs,
                                        const struct neko_gui_baked_font *baked_font, neko_gui_handle atlas) {
    struct neko_gui_baked_font baked;
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(glyphs);
    NEKO_GUI_ASSERT(baked_font);
    if (!font || !glyphs || !baked_font) return;

    baked = *baked_font;
    font->fallback = 0;
    font->info = baked;
    font->scale = (float)pixel_height / (float)font->info.height;
    font->glyphs = &glyphs[baked_font->glyph_offset];
    font->texture = atlas;
    font->fallback_codepoint = fallback_codepoint;
    font->fallback = neko_gui_font_find_glyph(font, fallback_codepoint);

    font->handle.height = font->info.height * font->scale;
    font->handle.width = neko_gui_font_text_width;
    font->handle.userdata.ptr = font;
#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
    font->handle.query = neko_gui_font_query_font_glyph;
    font->handle.texture = font->texture;
#endif
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverlength-strings"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

#ifdef NEKO_GUI_INCLUDE_DEFAULT_FONT

NEKO_GUI_GLOBAL const char neko_gui_proggy_clean_ttf_compressed_data_base85[11980 + 1] =
        "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
        "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
        "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
        "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
        "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
        "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
        "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
        "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
        "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
        "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
        "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
        "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
        "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
        "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
        "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
        "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
        "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
        "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
        "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
        "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
        "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
        "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
        "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
        "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
        "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
        "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
        "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
        "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
        "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
        "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
        ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
        "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
        "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
        "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
        "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
        "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
        "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
        ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
        "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
        "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
        "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
        "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
        "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
        "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
        "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
        "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
        ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
        "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
        "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
        ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
        "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
        "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
        "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
        ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
        "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
        "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
        "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
        "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
        "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
        "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
        "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
        "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
        "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
        "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
        "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
        "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
        "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
        "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
        ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
        "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
        "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
        "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
        "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
        "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
        "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
        "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
        "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
        ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
        "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
        "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
        "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
        "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
        "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
        "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
        "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
        "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

#endif /* NEKO_GUI_INCLUDE_DEFAULT_FONT */

#define NEKO_GUI_CURSOR_DATA_W 90
#define NEKO_GUI_CURSOR_DATA_H 27
NEKO_GUI_GLOBAL const char neko_gui_custom_cursor_data[NEKO_GUI_CURSOR_DATA_W * NEKO_GUI_CURSOR_DATA_H + 1] = {
        "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
        "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
        "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
        "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
        "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
        "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
        "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
        "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
        "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
        "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
        "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
        "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
        "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
        "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
        "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
        "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
        "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
        "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
        "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
        "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
        "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
        "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
        "------------        -    X    -           X           -X.....................X-           "
        "                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
        "                                                      -  X..X           X..X  -           "
        "                                                      -   X.X           X.X   -           "
        "                                                      -    XX           XX    -           "};

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

NEKO_GUI_GLOBAL unsigned char *neko_gui__barrier;
NEKO_GUI_GLOBAL unsigned char *neko_gui__barrier2;
NEKO_GUI_GLOBAL unsigned char *neko_gui__barrier3;
NEKO_GUI_GLOBAL unsigned char *neko_gui__barrier4;
NEKO_GUI_GLOBAL unsigned char *neko_gui__dout;

NEKO_GUI_INTERN unsigned int neko_gui_decompress_length(unsigned char *input) { return (unsigned int)((input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11]); }
NEKO_GUI_INTERN void neko_gui__match(unsigned char *data, unsigned int length) {

    NEKO_GUI_ASSERT(neko_gui__dout + length <= neko_gui__barrier);
    if (neko_gui__dout + length > neko_gui__barrier) {
        neko_gui__dout += length;
        return;
    }
    if (data < neko_gui__barrier4) {
        neko_gui__dout = neko_gui__barrier + 1;
        return;
    }
    while (length--) *neko_gui__dout++ = *data++;
}
NEKO_GUI_INTERN void neko_gui__lit(unsigned char *data, unsigned int length) {
    NEKO_GUI_ASSERT(neko_gui__dout + length <= neko_gui__barrier);
    if (neko_gui__dout + length > neko_gui__barrier) {
        neko_gui__dout += length;
        return;
    }
    if (data < neko_gui__barrier2) {
        neko_gui__dout = neko_gui__barrier + 1;
        return;
    }
    NEKO_GUI_MEMCPY(neko_gui__dout, data, length);
    neko_gui__dout += length;
}
NEKO_GUI_INTERN unsigned char *neko_gui_decompress_token(unsigned char *i) {
#define neko_gui__in2(x) ((i[x] << 8) + i[(x) + 1])
#define neko_gui__in3(x) ((i[x] << 16) + neko_gui__in2((x) + 1))
#define neko_gui__in4(x) ((i[x] << 24) + neko_gui__in3((x) + 1))

    if (*i >= 0x20) {
        if (*i >= 0x80)
            neko_gui__match(neko_gui__dout - i[1] - 1, (unsigned int)i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)
            neko_gui__match(neko_gui__dout - (neko_gui__in2(0) - 0x4000 + 1), (unsigned int)i[2] + 1), i += 3;
        else
            neko_gui__lit(i + 1, (unsigned int)i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else {
        if (*i >= 0x18)
            neko_gui__match(neko_gui__dout - (unsigned int)(neko_gui__in3(0) - 0x180000 + 1), (unsigned int)i[3] + 1), i += 4;
        else if (*i >= 0x10)
            neko_gui__match(neko_gui__dout - (unsigned int)(neko_gui__in3(0) - 0x100000 + 1), (unsigned int)neko_gui__in2(3) + 1), i += 5;
        else if (*i >= 0x08)
            neko_gui__lit(i + 2, (unsigned int)neko_gui__in2(0) - 0x0800 + 1), i += 2 + (neko_gui__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)
            neko_gui__lit(i + 3, (unsigned int)neko_gui__in2(1) + 1), i += 3 + (neko_gui__in2(1) + 1);
        else if (*i == 0x06)
            neko_gui__match(neko_gui__dout - (unsigned int)(neko_gui__in3(1) + 1), i[4] + 1u), i += 5;
        else if (*i == 0x04)
            neko_gui__match(neko_gui__dout - (unsigned int)(neko_gui__in3(1) + 1), (unsigned int)neko_gui__in2(4) + 1u), i += 6;
    }
    return i;
}
NEKO_GUI_INTERN unsigned int neko_gui_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen) {
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0];
            s2 += s1;
            s1 += buffer[1];
            s2 += s1;
            s1 += buffer[2];
            s2 += s1;
            s1 += buffer[3];
            s2 += s1;
            s1 += buffer[4];
            s2 += s1;
            s1 += buffer[5];
            s2 += s1;
            s1 += buffer[6];
            s2 += s1;
            s1 += buffer[7];
            s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i) {
            s1 += *buffer++;
            s2 += s1;
        }

        s1 %= ADLER_MOD;
        s2 %= ADLER_MOD;
        buflen -= (unsigned int)blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}
NEKO_GUI_INTERN unsigned int neko_gui_decompress(unsigned char *output, unsigned char *i, unsigned int length) {
    unsigned int olen;
    if (neko_gui__in4(0) != 0x57bC0000) return 0;
    if (neko_gui__in4(4) != 0) return 0;
    olen = neko_gui_decompress_length(i);
    neko_gui__barrier2 = i;
    neko_gui__barrier3 = i + length;
    neko_gui__barrier = output + olen;
    neko_gui__barrier4 = output;
    i += 16;

    neko_gui__dout = output;
    for (;;) {
        unsigned char *old_i = i;
        i = neko_gui_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                NEKO_GUI_ASSERT(neko_gui__dout == output + olen);
                if (neko_gui__dout != output + olen) return 0;
                if (neko_gui_adler32(1, output, olen) != (unsigned int)neko_gui__in4(2)) return 0;
                return olen;
            } else {
                NEKO_GUI_ASSERT(0);
                return 0;
            }
        }
        NEKO_GUI_ASSERT(neko_gui__dout <= output + olen);
        if (neko_gui__dout > output + olen) return 0;
    }
}
NEKO_GUI_INTERN unsigned int neko_gui_decode_85_byte(char c) { return (unsigned int)((c >= '\\') ? c - 36 : c - 35); }
NEKO_GUI_INTERN void neko_gui_decode_85(unsigned char *dst, const unsigned char *src) {
    while (*src) {
        unsigned int tmp =
                neko_gui_decode_85_byte((char)src[0]) +
                85 * (neko_gui_decode_85_byte((char)src[1]) + 85 * (neko_gui_decode_85_byte((char)src[2]) + 85 * (neko_gui_decode_85_byte((char)src[3]) + 85 * neko_gui_decode_85_byte((char)src[4]))));

        dst[0] = (unsigned char)((tmp >> 0) & 0xFF);
        dst[1] = (unsigned char)((tmp >> 8) & 0xFF);
        dst[2] = (unsigned char)((tmp >> 16) & 0xFF);
        dst[3] = (unsigned char)((tmp >> 24) & 0xFF);

        src += 5;
        dst += 4;
    }
}

NEKO_GUI_API struct neko_gui_font_config neko_gui_font_config(float pixel_height) {
    struct neko_gui_font_config cfg;
    neko_gui_zero_struct(cfg);
    cfg.ttf_blob = 0;
    cfg.ttf_size = 0;
    cfg.ttf_data_owned_by_atlas = 0;
    cfg.size = pixel_height;
    cfg.oversample_h = 3;
    cfg.oversample_v = 1;
    cfg.pixel_snap = 0;
    cfg.coord_type = NEKO_GUI_COORD_UV;
    cfg.spacing = neko_gui_vec2(0, 0);
    cfg.range = neko_gui_font_default_glyph_ranges();
    cfg.merge_mode = 0;
    cfg.fallback_glyph = '?';
    cfg.font = 0;
    cfg.n = 0;
    return cfg;
}
#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_font_atlas_init_default(struct neko_gui_font_atlas *atlas) {
    NEKO_GUI_ASSERT(atlas);
    if (!atlas) return;
    neko_gui_zero_struct(*atlas);
    atlas->temporary.userdata.ptr = 0;
    atlas->temporary.alloc = neko_gui_malloc;
    atlas->temporary.free = neko_gui_mfree;
    atlas->permanent.userdata.ptr = 0;
    atlas->permanent.alloc = neko_gui_malloc;
    atlas->permanent.free = neko_gui_mfree;
}
#endif
NEKO_GUI_API void neko_gui_font_atlas_init(struct neko_gui_font_atlas *atlas, struct neko_gui_allocator *alloc) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(alloc);
    if (!atlas || !alloc) return;
    neko_gui_zero_struct(*atlas);
    atlas->permanent = *alloc;
    atlas->temporary = *alloc;
}
NEKO_GUI_API void neko_gui_font_atlas_init_custom(struct neko_gui_font_atlas *atlas, struct neko_gui_allocator *permanent, struct neko_gui_allocator *temporary) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(permanent);
    NEKO_GUI_ASSERT(temporary);
    if (!atlas || !permanent || !temporary) return;
    neko_gui_zero_struct(*atlas);
    atlas->permanent = *permanent;
    atlas->temporary = *temporary;
}
NEKO_GUI_API void neko_gui_font_atlas_begin(struct neko_gui_font_atlas *atlas) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc && atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc && atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free || !atlas->temporary.alloc || !atlas->temporary.free) return;
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
}
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add(struct neko_gui_font_atlas *atlas, const struct neko_gui_font_config *config) {
    struct neko_gui_font *font = 0;
    struct neko_gui_font_config *cfg;

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);

    NEKO_GUI_ASSERT(config);
    NEKO_GUI_ASSERT(config->ttf_blob);
    NEKO_GUI_ASSERT(config->ttf_size);
    NEKO_GUI_ASSERT(config->size > 0.0f);

    if (!atlas || !config || !config->ttf_blob || !config->ttf_size || config->size <= 0.0f || !atlas->permanent.alloc || !atlas->permanent.free || !atlas->temporary.alloc || !atlas->temporary.free)
        return 0;

    cfg = (struct neko_gui_font_config *)atlas->permanent.alloc(atlas->permanent.userdata, 0, sizeof(struct neko_gui_font_config));
    NEKO_GUI_MEMCPY(cfg, config, sizeof(*config));
    cfg->n = cfg;
    cfg->p = cfg;

    if (!config->merge_mode) {

        if (!atlas->config) {
            atlas->config = cfg;
            cfg->next = 0;
        } else {
            struct neko_gui_font_config *i = atlas->config;
            while (i->next) i = i->next;
            i->next = cfg;
            cfg->next = 0;
        }

        font = (struct neko_gui_font *)atlas->permanent.alloc(atlas->permanent.userdata, 0, sizeof(struct neko_gui_font));
        NEKO_GUI_ASSERT(font);
        neko_gui_zero(font, sizeof(*font));
        if (!font) return 0;
        font->config = cfg;

        if (!atlas->fonts) {
            atlas->fonts = font;
            font->next = 0;
        } else {
            struct neko_gui_font *i = atlas->fonts;
            while (i->next) i = i->next;
            i->next = font;
            font->next = 0;
        }
        cfg->font = &font->info;
    } else {

        struct neko_gui_font *f = 0;
        struct neko_gui_font_config *c = 0;
        NEKO_GUI_ASSERT(atlas->font_num);
        f = atlas->fonts;
        c = f->config;
        cfg->font = &f->info;

        cfg->n = c;
        cfg->p = c->p;
        c->p->n = cfg;
        c->p = cfg;
    }

    if (!config->ttf_data_owned_by_atlas) {
        cfg->ttf_blob = atlas->permanent.alloc(atlas->permanent.userdata, 0, cfg->ttf_size);
        NEKO_GUI_ASSERT(cfg->ttf_blob);
        if (!cfg->ttf_blob) {
            atlas->font_num++;
            return 0;
        }
        NEKO_GUI_MEMCPY(cfg->ttf_blob, config->ttf_blob, cfg->ttf_size);
        cfg->ttf_data_owned_by_atlas = 1;
    }
    atlas->font_num++;
    return font;
}
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_from_memory(struct neko_gui_font_atlas *atlas, void *memory, neko_gui_size size, float height, const struct neko_gui_font_config *config) {
    struct neko_gui_font_config cfg;
    NEKO_GUI_ASSERT(memory);
    NEKO_GUI_ASSERT(size);

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->temporary.alloc || !atlas->temporary.free || !memory || !size || !atlas->permanent.alloc || !atlas->permanent.free) return 0;

    cfg = (config) ? *config : neko_gui_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 0;
    return neko_gui_font_atlas_add(atlas, &cfg);
}
#ifdef NEKO_GUI_INCLUDE_STANDARD_IO
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_from_file(struct neko_gui_font_atlas *atlas, const char *file_path, float height, const struct neko_gui_font_config *config) {
    neko_gui_size size;
    char *memory;
    struct neko_gui_font_config cfg;

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);

    if (!atlas || !file_path) return 0;
    memory = neko_gui_file_load(file_path, &size, &atlas->permanent);
    if (!memory) return 0;

    cfg = (config) ? *config : neko_gui_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return neko_gui_font_atlas_add(atlas, &cfg);
}
#endif
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_compressed(struct neko_gui_font_atlas *atlas, void *compressed_data, neko_gui_size compressed_size, float height,
                                                                      const struct neko_gui_font_config *config) {
    unsigned int decompressed_size;
    void *decompressed_data;
    struct neko_gui_font_config cfg;

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);

    NEKO_GUI_ASSERT(compressed_data);
    NEKO_GUI_ASSERT(compressed_size);
    if (!atlas || !compressed_data || !atlas->temporary.alloc || !atlas->temporary.free || !atlas->permanent.alloc || !atlas->permanent.free) return 0;

    decompressed_size = neko_gui_decompress_length((unsigned char *)compressed_data);
    decompressed_data = atlas->permanent.alloc(atlas->permanent.userdata, 0, decompressed_size);
    NEKO_GUI_ASSERT(decompressed_data);
    if (!decompressed_data) return 0;
    neko_gui_decompress((unsigned char *)decompressed_data, (unsigned char *)compressed_data, (unsigned int)compressed_size);

    cfg = (config) ? *config : neko_gui_font_config(height);
    cfg.ttf_blob = decompressed_data;
    cfg.ttf_size = decompressed_size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return neko_gui_font_atlas_add(atlas, &cfg);
}
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_compressed_base85(struct neko_gui_font_atlas *atlas, const char *data_base85, float height, const struct neko_gui_font_config *config) {
    int compressed_size;
    void *compressed_data;
    struct neko_gui_font *font;

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);

    NEKO_GUI_ASSERT(data_base85);
    if (!atlas || !data_base85 || !atlas->temporary.alloc || !atlas->temporary.free || !atlas->permanent.alloc || !atlas->permanent.free) return 0;

    compressed_size = (((int)neko_gui_strlen(data_base85) + 4) / 5) * 4;
    compressed_data = atlas->temporary.alloc(atlas->temporary.userdata, 0, (neko_gui_size)compressed_size);
    NEKO_GUI_ASSERT(compressed_data);
    if (!compressed_data) return 0;
    neko_gui_decode_85((unsigned char *)compressed_data, (const unsigned char *)data_base85);
    font = neko_gui_font_atlas_add_compressed(atlas, compressed_data, (neko_gui_size)compressed_size, height, config);
    atlas->temporary.free(atlas->temporary.userdata, compressed_data);
    return font;
}

#ifdef NEKO_GUI_INCLUDE_DEFAULT_FONT
NEKO_GUI_API struct neko_gui_font *neko_gui_font_atlas_add_default(struct neko_gui_font_atlas *atlas, float pixel_height, const struct neko_gui_font_config *config) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);
    return neko_gui_font_atlas_add_compressed_base85(atlas, neko_gui_proggy_clean_ttf_compressed_data_base85, pixel_height, config);
}
#endif
NEKO_GUI_API const void *neko_gui_font_atlas_bake(struct neko_gui_font_atlas *atlas, int *width, int *height, enum neko_gui_font_atlas_format fmt) {
    int i = 0;
    void *tmp = 0;
    neko_gui_size tmp_size, img_size;
    struct neko_gui_font *font_iter;
    struct neko_gui_font_baker *baker;

    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);

    NEKO_GUI_ASSERT(width);
    NEKO_GUI_ASSERT(height);
    if (!atlas || !width || !height || !atlas->temporary.alloc || !atlas->temporary.free || !atlas->permanent.alloc || !atlas->permanent.free) return 0;

#ifdef NEKO_GUI_INCLUDE_DEFAULT_FONT

    if (!atlas->font_num) atlas->default_font = neko_gui_font_atlas_add_default(atlas, 18.0f, 0);
#endif
    NEKO_GUI_ASSERT(atlas->font_num);
    if (!atlas->font_num) return 0;

    neko_gui_font_baker_memory(&tmp_size, &atlas->glyph_count, atlas->config, atlas->font_num);
    tmp = atlas->temporary.alloc(atlas->temporary.userdata, 0, tmp_size);
    NEKO_GUI_ASSERT(tmp);
    if (!tmp) goto failed;
    NEKO_GUI_MEMSET(tmp, 0, tmp_size);

    baker = neko_gui_font_baker(tmp, atlas->glyph_count, atlas->font_num, &atlas->temporary);
    atlas->glyphs = (struct neko_gui_font_glyph *)atlas->permanent.alloc(atlas->permanent.userdata, 0, sizeof(struct neko_gui_font_glyph) * (neko_gui_size)atlas->glyph_count);
    NEKO_GUI_ASSERT(atlas->glyphs);
    if (!atlas->glyphs) goto failed;

    atlas->custom.w = (NEKO_GUI_CURSOR_DATA_W * 2) + 1;
    atlas->custom.h = NEKO_GUI_CURSOR_DATA_H + 1;
    if (!neko_gui_font_bake_pack(baker, &img_size, width, height, &atlas->custom, atlas->config, atlas->font_num, &atlas->temporary)) goto failed;

    atlas->pixel = atlas->temporary.alloc(atlas->temporary.userdata, 0, img_size);
    NEKO_GUI_ASSERT(atlas->pixel);
    if (!atlas->pixel) goto failed;

    neko_gui_font_bake(baker, atlas->pixel, *width, *height, atlas->glyphs, atlas->glyph_count, atlas->config, atlas->font_num);
    neko_gui_font_bake_custom_data(atlas->pixel, *width, *height, atlas->custom, neko_gui_custom_cursor_data, NEKO_GUI_CURSOR_DATA_W, NEKO_GUI_CURSOR_DATA_H, '.', 'X');

    if (fmt == NEKO_GUI_FONT_ATLAS_RGBA32) {

        void *img_rgba = atlas->temporary.alloc(atlas->temporary.userdata, 0, (neko_gui_size)(*width * *height * 4));
        NEKO_GUI_ASSERT(img_rgba);
        if (!img_rgba) goto failed;
        neko_gui_font_bake_convert(img_rgba, *width, *height, atlas->pixel);
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = img_rgba;
    }
    atlas->tex_width = *width;
    atlas->tex_height = *height;

    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        struct neko_gui_font *font = font_iter;
        struct neko_gui_font_config *config = font->config;
        neko_gui_font_init(font, config->size, config->fallback_glyph, atlas->glyphs, config->font, neko_gui_handle_ptr(0));
    }

    {
        NEKO_GUI_STORAGE const struct neko_gui_vec2 neko_gui_cursor_data[NEKO_GUI_CURSOR_COUNT][3] = {{{0, 3}, {12, 19}, {0, 0}},  {{13, 0}, {7, 16}, {4, 8}},   {{31, 0}, {23, 23}, {11, 11}},
                                                                                                      {{21, 0}, {9, 23}, {5, 11}}, {{55, 18}, {23, 9}, {11, 5}}, {{73, 0}, {17, 17}, {9, 9}},
                                                                                                      {{55, 0}, {17, 17}, {9, 9}}};
        for (i = 0; i < NEKO_GUI_CURSOR_COUNT; ++i) {
            struct neko_gui_cursor *cursor = &atlas->cursors[i];
            cursor->img.w = (unsigned short)*width;
            cursor->img.h = (unsigned short)*height;
            cursor->img.region[0] = (unsigned short)(atlas->custom.x + neko_gui_cursor_data[i][0].x);
            cursor->img.region[1] = (unsigned short)(atlas->custom.y + neko_gui_cursor_data[i][0].y);
            cursor->img.region[2] = (unsigned short)neko_gui_cursor_data[i][1].x;
            cursor->img.region[3] = (unsigned short)neko_gui_cursor_data[i][1].y;
            cursor->size = neko_gui_cursor_data[i][1];
            cursor->offset = neko_gui_cursor_data[i][2];
        }
    }

    atlas->temporary.free(atlas->temporary.userdata, tmp);
    return atlas->pixel;

failed:

    if (tmp) atlas->temporary.free(atlas->temporary.userdata, tmp);
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
    return 0;
}
NEKO_GUI_API void neko_gui_font_atlas_end(struct neko_gui_font_atlas *atlas, neko_gui_handle texture, struct neko_gui_draw_null_texture *tex_null) {
    int i = 0;
    struct neko_gui_font *font_iter;
    NEKO_GUI_ASSERT(atlas);
    if (!atlas) {
        if (!tex_null) return;
        tex_null->texture = texture;
        tex_null->uv = neko_gui_vec2(0.5f, 0.5f);
    }
    if (tex_null) {
        tex_null->texture = texture;
        tex_null->uv.x = (atlas->custom.x + 0.5f) / (float)atlas->tex_width;
        tex_null->uv.y = (atlas->custom.y + 0.5f) / (float)atlas->tex_height;
    }
    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        font_iter->texture = texture;
#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
        font_iter->handle.texture = texture;
#endif
    }
    for (i = 0; i < NEKO_GUI_CURSOR_COUNT; ++i) atlas->cursors[i].img.handle = texture;

    atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
    atlas->pixel = 0;
    atlas->tex_width = 0;
    atlas->tex_height = 0;
    atlas->custom.x = 0;
    atlas->custom.y = 0;
    atlas->custom.w = 0;
    atlas->custom.h = 0;
}
NEKO_GUI_API void neko_gui_font_atlas_cleanup(struct neko_gui_font_atlas *atlas) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;
    if (atlas->config) {
        struct neko_gui_font_config *iter;
        for (iter = atlas->config; iter; iter = iter->next) {
            struct neko_gui_font_config *i;
            for (i = iter->n; i != iter; i = i->n) {
                atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                i->ttf_blob = 0;
            }
            atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            iter->ttf_blob = 0;
        }
    }
}
NEKO_GUI_API void neko_gui_font_atlas_clear(struct neko_gui_font_atlas *atlas) {
    NEKO_GUI_ASSERT(atlas);
    NEKO_GUI_ASSERT(atlas->temporary.alloc);
    NEKO_GUI_ASSERT(atlas->temporary.free);
    NEKO_GUI_ASSERT(atlas->permanent.alloc);
    NEKO_GUI_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;

    if (atlas->config) {
        struct neko_gui_font_config *iter, *next;
        for (iter = atlas->config; iter; iter = next) {
            struct neko_gui_font_config *i, *n;
            for (i = iter->n; i != iter; i = n) {
                n = i->n;
                if (i->ttf_blob) atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                atlas->permanent.free(atlas->permanent.userdata, i);
            }
            next = iter->next;
            if (i->ttf_blob) atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->config = 0;
    }
    if (atlas->fonts) {
        struct neko_gui_font *iter, *next;
        for (iter = atlas->fonts; iter; iter = next) {
            next = iter->next;
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->fonts = 0;
    }
    if (atlas->glyphs) atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
    neko_gui_zero_struct(*atlas);
}
#endif

NEKO_GUI_API void neko_gui_input_begin(struct neko_gui_context *ctx) {
    int i;
    struct neko_gui_input *in;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;
    for (i = 0; i < NEKO_GUI_BUTTON_MAX; ++i) in->mouse.buttons[i].clicked = 0;

    in->keyboard.text_len = 0;
    in->mouse.scroll_delta = neko_gui_vec2(0, 0);
    in->mouse.prev.x = in->mouse.pos.x;
    in->mouse.prev.y = in->mouse.pos.y;
    in->mouse.delta.x = 0;
    in->mouse.delta.y = 0;
    for (i = 0; i < NEKO_GUI_KEY_MAX; i++) in->keyboard.keys[i].clicked = 0;
}
NEKO_GUI_API void neko_gui_input_end(struct neko_gui_context *ctx) {
    struct neko_gui_input *in;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;
    if (in->mouse.grab) in->mouse.grab = 0;
    if (in->mouse.ungrab) {
        in->mouse.grabbed = 0;
        in->mouse.ungrab = 0;
        in->mouse.grab = 0;
    }
}
NEKO_GUI_API void neko_gui_input_motion(struct neko_gui_context *ctx, int x, int y) {
    struct neko_gui_input *in;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;
    in->mouse.pos.x = (float)x;
    in->mouse.pos.y = (float)y;
    in->mouse.delta.x = in->mouse.pos.x - in->mouse.prev.x;
    in->mouse.delta.y = in->mouse.pos.y - in->mouse.prev.y;
}
NEKO_GUI_API void neko_gui_input_key(struct neko_gui_context *ctx, enum neko_gui_keys key, neko_gui_bool down) {
    struct neko_gui_input *in;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;
#ifdef NEKO_GUI_KEYSTATE_BASED_INPUT
    if (in->keyboard.keys[key].down != down) in->keyboard.keys[key].clicked++;
#else
    in->keyboard.keys[key].clicked++;
#endif
    in->keyboard.keys[key].down = down;
}
NEKO_GUI_API void neko_gui_input_button(struct neko_gui_context *ctx, enum neko_gui_buttons id, int x, int y, neko_gui_bool down) {
    struct neko_gui_mouse_button *btn;
    struct neko_gui_input *in;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;
    if (in->mouse.buttons[id].down == down) return;

    btn = &in->mouse.buttons[id];
    btn->clicked_pos.x = (float)x;
    btn->clicked_pos.y = (float)y;
    btn->down = down;
    btn->clicked++;

    in->mouse.delta.x = 0;
    in->mouse.delta.y = 0;
#ifdef NEKO_GUI_BUTTON_TRIGGER_ON_RELEASE
    if (down == 1 && id == NEKO_GUI_BUTTON_LEFT) {
        in->mouse.down_pos.x = btn->clicked_pos.x;
        in->mouse.down_pos.y = btn->clicked_pos.y;
    }
#endif
}
NEKO_GUI_API void neko_gui_input_scroll(struct neko_gui_context *ctx, struct neko_gui_vec2 val) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    ctx->input.mouse.scroll_delta.x += val.x;
    ctx->input.mouse.scroll_delta.y += val.y;
}
NEKO_GUI_API void neko_gui_input_glyph(struct neko_gui_context *ctx, const neko_gui_glyph glyph) {
    int len = 0;
    neko_gui_rune unicode;
    struct neko_gui_input *in;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    in = &ctx->input;

    len = neko_gui_utf_decode(glyph, &unicode, NEKO_GUI_UTF_SIZE);
    if (len && ((in->keyboard.text_len + len) < NEKO_GUI_INPUT_MAX)) {
        neko_gui_utf_encode(unicode, &in->keyboard.text[in->keyboard.text_len], NEKO_GUI_INPUT_MAX - in->keyboard.text_len);
        in->keyboard.text_len += len;
    }
}
NEKO_GUI_API void neko_gui_input_char(struct neko_gui_context *ctx, char c) {
    neko_gui_glyph glyph;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    glyph[0] = c;
    neko_gui_input_glyph(ctx, glyph);
}
NEKO_GUI_API void neko_gui_input_unicode(struct neko_gui_context *ctx, neko_gui_rune unicode) {
    neko_gui_glyph rune;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    neko_gui_utf_encode(unicode, rune, NEKO_GUI_UTF_SIZE);
    neko_gui_input_glyph(ctx, rune);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click(const struct neko_gui_input *i, enum neko_gui_buttons id) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
    return (btn->clicked && btn->down == neko_gui_false) ? neko_gui_true : neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_in_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
    if (!NEKO_GUI_INBOX(btn->clicked_pos.x, btn->clicked_pos.y, b.x, b.y, b.w, b.h)) return neko_gui_false;
    return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_in_button_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
#ifdef NEKO_GUI_BUTTON_TRIGGER_ON_RELEASE
    if (!NEKO_GUI_INBOX(btn->clicked_pos.x, btn->clicked_pos.y, b.x, b.y, b.w, b.h) || !NEKO_GUI_INBOX(i->mouse.down_pos.x, i->mouse.down_pos.y, b.x, b.y, b.w, b.h))
#else
    if (!NEKO_GUI_INBOX(btn->clicked_pos.x, btn->clicked_pos.y, b.x, b.y, b.w, b.h))
#endif
        return neko_gui_false;
    return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_has_mouse_click_down_in_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b, neko_gui_bool down) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
    return neko_gui_input_has_mouse_click_in_rect(i, id, b) && (btn->down == down);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_click_in_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
    return (neko_gui_input_has_mouse_click_down_in_rect(i, id, b, neko_gui_false) && btn->clicked) ? neko_gui_true : neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_click_down_in_rect(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect b, neko_gui_bool down) {
    const struct neko_gui_mouse_button *btn;
    if (!i) return neko_gui_false;
    btn = &i->mouse.buttons[id];
    return (neko_gui_input_has_mouse_click_down_in_rect(i, id, b, down) && btn->clicked) ? neko_gui_true : neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_any_mouse_click_in_rect(const struct neko_gui_input *in, struct neko_gui_rect b) {
    int i, down = 0;
    for (i = 0; i < NEKO_GUI_BUTTON_MAX; ++i) down = down || neko_gui_input_is_mouse_click_in_rect(in, (enum neko_gui_buttons)i, b);
    return down;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_hovering_rect(const struct neko_gui_input *i, struct neko_gui_rect rect) {
    if (!i) return neko_gui_false;
    return NEKO_GUI_INBOX(i->mouse.pos.x, i->mouse.pos.y, rect.x, rect.y, rect.w, rect.h);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_prev_hovering_rect(const struct neko_gui_input *i, struct neko_gui_rect rect) {
    if (!i) return neko_gui_false;
    return NEKO_GUI_INBOX(i->mouse.prev.x, i->mouse.prev.y, rect.x, rect.y, rect.w, rect.h);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_mouse_clicked(const struct neko_gui_input *i, enum neko_gui_buttons id, struct neko_gui_rect rect) {
    if (!i) return neko_gui_false;
    if (!neko_gui_input_is_mouse_hovering_rect(i, rect)) return neko_gui_false;
    return neko_gui_input_is_mouse_click_in_rect(i, id, rect);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_down(const struct neko_gui_input *i, enum neko_gui_buttons id) {
    if (!i) return neko_gui_false;
    return i->mouse.buttons[id].down;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_pressed(const struct neko_gui_input *i, enum neko_gui_buttons id) {
    const struct neko_gui_mouse_button *b;
    if (!i) return neko_gui_false;
    b = &i->mouse.buttons[id];
    if (b->down && b->clicked) return neko_gui_true;
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_mouse_released(const struct neko_gui_input *i, enum neko_gui_buttons id) {
    if (!i) return neko_gui_false;
    return (!i->mouse.buttons[id].down && i->mouse.buttons[id].clicked);
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_pressed(const struct neko_gui_input *i, enum neko_gui_keys key) {
    const struct neko_gui_key *k;
    if (!i) return neko_gui_false;
    k = &i->keyboard.keys[key];
    if ((k->down && k->clicked) || (!k->down && k->clicked >= 2)) return neko_gui_true;
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_released(const struct neko_gui_input *i, enum neko_gui_keys key) {
    const struct neko_gui_key *k;
    if (!i) return neko_gui_false;
    k = &i->keyboard.keys[key];
    if ((!k->down && k->clicked) || (k->down && k->clicked >= 2)) return neko_gui_true;
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_input_is_key_down(const struct neko_gui_input *i, enum neko_gui_keys key) {
    const struct neko_gui_key *k;
    if (!i) return neko_gui_false;
    k = &i->keyboard.keys[key];
    if (k->down) return neko_gui_true;
    return neko_gui_false;
}

NEKO_GUI_API void neko_gui_style_default(struct neko_gui_context *ctx) { neko_gui_style_from_table(ctx, 0); }
#define NEKO_GUI_COLOR_MAP(NEKO_GUI_COLOR)                                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_TEXT, 175, 175, 175, 255)                    \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_WINDOW, 45, 45, 45, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_HEADER, 40, 40, 40, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_BORDER, 65, 65, 65, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_BUTTON, 50, 50, 50, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_BUTTON_HOVER, 40, 40, 40, 255)               \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_BUTTON_ACTIVE, 35, 35, 35, 255)              \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_TOGGLE, 100, 100, 100, 255)                  \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_TOGGLE_HOVER, 120, 120, 120, 255)            \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_TOGGLE_CURSOR, 45, 45, 45, 255)              \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SELECT, 45, 45, 45, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SELECT_ACTIVE, 35, 35, 35, 255)              \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SLIDER, 38, 38, 38, 255)                     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SLIDER_CURSOR, 100, 100, 100, 255)           \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER, 120, 120, 120, 255)     \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE, 150, 150, 150, 255)    \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_PROPERTY, 38, 38, 38, 255)                   \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_EDIT, 38, 38, 38, 255)                       \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_EDIT_CURSOR, 175, 175, 175, 255)             \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_COMBO, 45, 45, 45, 255)                      \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_CHART, 120, 120, 120, 255)                   \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_CHART_COLOR, 45, 45, 45, 255)                \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT, 255, 0, 0, 255)       \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SCROLLBAR, 40, 40, 40, 255)                  \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SCROLLBAR_CURSOR, 100, 100, 100, 255)        \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER, 120, 120, 120, 255)  \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE, 150, 150, 150, 255) \
    NEKO_GUI_COLOR(NEKO_GUI_COLOR_TAB_HEADER, 40, 40, 40, 255)

NEKO_GUI_GLOBAL const struct neko_gui_color neko_gui_default_color_style[NEKO_GUI_COLOR_COUNT] = {
#define NEKO_GUI_COLOR(a, b, c, d, e) {b, c, d, e},
        NEKO_GUI_COLOR_MAP(NEKO_GUI_COLOR)
#undef NEKO_GUI_COLOR
};
NEKO_GUI_GLOBAL const char *neko_gui_color_names[NEKO_GUI_COLOR_COUNT] = {
#define NEKO_GUI_COLOR(a, b, c, d, e) #a,
        NEKO_GUI_COLOR_MAP(NEKO_GUI_COLOR)
#undef NEKO_GUI_COLOR
};

NEKO_GUI_API const char *neko_gui_style_get_color_by_name(enum neko_gui_style_colors c) { return neko_gui_color_names[c]; }
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_color(struct neko_gui_color col) {
    struct neko_gui_style_item i;
    i.type = NEKO_GUI_STYLE_ITEM_COLOR;
    i.data.color = col;
    return i;
}
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_image(struct neko_gui_image img) {
    struct neko_gui_style_item i;
    i.type = NEKO_GUI_STYLE_ITEM_IMAGE;
    i.data.image = img;
    return i;
}
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_nine_slice(struct neko_gui_nine_slice slice) {
    struct neko_gui_style_item i;
    i.type = NEKO_GUI_STYLE_ITEM_NINE_SLICE;
    i.data.slice = slice;
    return i;
}
NEKO_GUI_API struct neko_gui_style_item neko_gui_style_item_hide(void) {
    struct neko_gui_style_item i;
    i.type = NEKO_GUI_STYLE_ITEM_COLOR;
    i.data.color = neko_gui_rgba(0, 0, 0, 0);
    return i;
}
NEKO_GUI_API void neko_gui_style_from_table(struct neko_gui_context *ctx, const struct neko_gui_color *table) {
    struct neko_gui_style *style;
    struct neko_gui_style_text *text;
    struct neko_gui_style_button *button;
    struct neko_gui_style_toggle *toggle;
    struct neko_gui_style_selectable *select;
    struct neko_gui_style_slider *slider;
    struct neko_gui_style_progress *prog;
    struct neko_gui_style_scrollbar *scroll;
    struct neko_gui_style_edit *edit;
    struct neko_gui_style_property *property;
    struct neko_gui_style_combo *combo;
    struct neko_gui_style_chart *chart;
    struct neko_gui_style_tab *tab;
    struct neko_gui_style_window *win;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    style = &ctx->style;
    table = (!table) ? neko_gui_default_color_style : table;

    text = &style->text;
    text->color = table[NEKO_GUI_COLOR_TEXT];
    text->padding = neko_gui_vec2(0, 0);

    button = &style->button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_BUTTON]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_BUTTON_HOVER]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_BUTTON_ACTIVE]);
    button->border_color = table[NEKO_GUI_COLOR_BORDER];
    button->text_background = table[NEKO_GUI_COLOR_BUTTON];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->image_padding = neko_gui_vec2(0.0f, 0.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 1.0f;
    button->rounding = 4.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    button = &style->contextual_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_BUTTON_HOVER]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_BUTTON_ACTIVE]);
    button->border_color = table[NEKO_GUI_COLOR_WINDOW];
    button->text_background = table[NEKO_GUI_COLOR_WINDOW];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    button = &style->menu_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->border_color = table[NEKO_GUI_COLOR_WINDOW];
    button->text_background = table[NEKO_GUI_COLOR_WINDOW];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 1.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    toggle = &style->checkbox;
    neko_gui_zero_struct(*toggle);
    toggle->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE]);
    toggle->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_HOVER]);
    toggle->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_HOVER]);
    toggle->cursor_normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_CURSOR]);
    toggle->cursor_hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_CURSOR]);
    toggle->userdata = neko_gui_handle_ptr(0);
    toggle->text_background = table[NEKO_GUI_COLOR_WINDOW];
    toggle->text_normal = table[NEKO_GUI_COLOR_TEXT];
    toggle->text_hover = table[NEKO_GUI_COLOR_TEXT];
    toggle->text_active = table[NEKO_GUI_COLOR_TEXT];
    toggle->padding = neko_gui_vec2(2.0f, 2.0f);
    toggle->touch_padding = neko_gui_vec2(0, 0);
    toggle->border_color = neko_gui_rgba(0, 0, 0, 0);
    toggle->border = 0.0f;
    toggle->spacing = 4;

    toggle = &style->option;
    neko_gui_zero_struct(*toggle);
    toggle->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE]);
    toggle->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_HOVER]);
    toggle->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_HOVER]);
    toggle->cursor_normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_CURSOR]);
    toggle->cursor_hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TOGGLE_CURSOR]);
    toggle->userdata = neko_gui_handle_ptr(0);
    toggle->text_background = table[NEKO_GUI_COLOR_WINDOW];
    toggle->text_normal = table[NEKO_GUI_COLOR_TEXT];
    toggle->text_hover = table[NEKO_GUI_COLOR_TEXT];
    toggle->text_active = table[NEKO_GUI_COLOR_TEXT];
    toggle->padding = neko_gui_vec2(3.0f, 3.0f);
    toggle->touch_padding = neko_gui_vec2(0, 0);
    toggle->border_color = neko_gui_rgba(0, 0, 0, 0);
    toggle->border = 0.0f;
    toggle->spacing = 4;

    select = &style->selectable;
    neko_gui_zero_struct(*select);
    select->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT]);
    select->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT]);
    select->pressed = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT]);
    select->normal_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT_ACTIVE]);
    select->hover_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT_ACTIVE]);
    select->pressed_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SELECT_ACTIVE]);
    select->text_normal = table[NEKO_GUI_COLOR_TEXT];
    select->text_hover = table[NEKO_GUI_COLOR_TEXT];
    select->text_pressed = table[NEKO_GUI_COLOR_TEXT];
    select->text_normal_active = table[NEKO_GUI_COLOR_TEXT];
    select->text_hover_active = table[NEKO_GUI_COLOR_TEXT];
    select->text_pressed_active = table[NEKO_GUI_COLOR_TEXT];
    select->padding = neko_gui_vec2(2.0f, 2.0f);
    select->image_padding = neko_gui_vec2(2.0f, 2.0f);
    select->touch_padding = neko_gui_vec2(0, 0);
    select->userdata = neko_gui_handle_ptr(0);
    select->rounding = 0.0f;
    select->draw_begin = 0;
    select->draw_end = 0;

    slider = &style->slider;
    neko_gui_zero_struct(*slider);
    slider->normal = neko_gui_style_item_hide();
    slider->hover = neko_gui_style_item_hide();
    slider->active = neko_gui_style_item_hide();
    slider->bar_normal = table[NEKO_GUI_COLOR_SLIDER];
    slider->bar_hover = table[NEKO_GUI_COLOR_SLIDER];
    slider->bar_active = table[NEKO_GUI_COLOR_SLIDER];
    slider->bar_filled = table[NEKO_GUI_COLOR_SLIDER_CURSOR];
    slider->cursor_normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR]);
    slider->cursor_hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER]);
    slider->cursor_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE]);
    slider->inc_symbol = NEKO_GUI_SYMBOL_TRIANGLE_RIGHT;
    slider->dec_symbol = NEKO_GUI_SYMBOL_TRIANGLE_LEFT;
    slider->cursor_size = neko_gui_vec2(16, 16);
    slider->padding = neko_gui_vec2(2, 2);
    slider->spacing = neko_gui_vec2(2, 2);
    slider->userdata = neko_gui_handle_ptr(0);
    slider->show_buttons = neko_gui_false;
    slider->bar_height = 8;
    slider->rounding = 0;
    slider->draw_begin = 0;
    slider->draw_end = 0;

    button = &style->slider.inc_button;
    button->normal = neko_gui_style_item_color(neko_gui_rgb(40, 40, 40));
    button->hover = neko_gui_style_item_color(neko_gui_rgb(42, 42, 42));
    button->active = neko_gui_style_item_color(neko_gui_rgb(44, 44, 44));
    button->border_color = neko_gui_rgb(65, 65, 65);
    button->text_background = neko_gui_rgb(40, 40, 40);
    button->text_normal = neko_gui_rgb(175, 175, 175);
    button->text_hover = neko_gui_rgb(175, 175, 175);
    button->text_active = neko_gui_rgb(175, 175, 175);
    button->padding = neko_gui_vec2(8.0f, 8.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 1.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;
    style->slider.dec_button = style->slider.inc_button;

    prog = &style->progress;
    neko_gui_zero_struct(*prog);
    prog->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER]);
    prog->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER]);
    prog->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER]);
    prog->cursor_normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR]);
    prog->cursor_hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR_HOVER]);
    prog->cursor_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SLIDER_CURSOR_ACTIVE]);
    prog->border_color = neko_gui_rgba(0, 0, 0, 0);
    prog->cursor_border_color = neko_gui_rgba(0, 0, 0, 0);
    prog->userdata = neko_gui_handle_ptr(0);
    prog->padding = neko_gui_vec2(4, 4);
    prog->rounding = 0;
    prog->border = 0;
    prog->cursor_rounding = 0;
    prog->cursor_border = 0;
    prog->draw_begin = 0;
    prog->draw_end = 0;

    scroll = &style->scrollh;
    neko_gui_zero_struct(*scroll);
    scroll->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR]);
    scroll->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR]);
    scroll->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR]);
    scroll->cursor_normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR]);
    scroll->cursor_hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_HOVER]);
    scroll->cursor_active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_SCROLLBAR_CURSOR_ACTIVE]);
    scroll->dec_symbol = NEKO_GUI_SYMBOL_CIRCLE_SOLID;
    scroll->inc_symbol = NEKO_GUI_SYMBOL_CIRCLE_SOLID;
    scroll->userdata = neko_gui_handle_ptr(0);
    scroll->border_color = table[NEKO_GUI_COLOR_SCROLLBAR];
    scroll->cursor_border_color = table[NEKO_GUI_COLOR_SCROLLBAR];
    scroll->padding = neko_gui_vec2(0, 0);
    scroll->show_buttons = neko_gui_false;
    scroll->border = 0;
    scroll->rounding = 0;
    scroll->border_cursor = 0;
    scroll->rounding_cursor = 0;
    scroll->draw_begin = 0;
    scroll->draw_end = 0;
    style->scrollv = style->scrollh;

    button = &style->scrollh.inc_button;
    button->normal = neko_gui_style_item_color(neko_gui_rgb(40, 40, 40));
    button->hover = neko_gui_style_item_color(neko_gui_rgb(42, 42, 42));
    button->active = neko_gui_style_item_color(neko_gui_rgb(44, 44, 44));
    button->border_color = neko_gui_rgb(65, 65, 65);
    button->text_background = neko_gui_rgb(40, 40, 40);
    button->text_normal = neko_gui_rgb(175, 175, 175);
    button->text_hover = neko_gui_rgb(175, 175, 175);
    button->text_active = neko_gui_rgb(175, 175, 175);
    button->padding = neko_gui_vec2(4.0f, 4.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 1.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;
    style->scrollh.dec_button = style->scrollh.inc_button;
    style->scrollv.inc_button = style->scrollh.inc_button;
    style->scrollv.dec_button = style->scrollh.inc_button;

    edit = &style->edit;
    neko_gui_zero_struct(*edit);
    edit->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_EDIT]);
    edit->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_EDIT]);
    edit->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_EDIT]);
    edit->cursor_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->cursor_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->cursor_text_normal = table[NEKO_GUI_COLOR_EDIT];
    edit->cursor_text_hover = table[NEKO_GUI_COLOR_EDIT];
    edit->border_color = table[NEKO_GUI_COLOR_BORDER];
    edit->text_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->text_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->text_active = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_text_normal = table[NEKO_GUI_COLOR_EDIT];
    edit->selected_text_hover = table[NEKO_GUI_COLOR_EDIT];
    edit->scrollbar_size = neko_gui_vec2(10, 10);
    edit->scrollbar = style->scrollv;
    edit->padding = neko_gui_vec2(4, 4);
    edit->row_padding = 2;
    edit->cursor_size = 4;
    edit->border = 1;
    edit->rounding = 0;

    property = &style->property;
    neko_gui_zero_struct(*property);
    property->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    property->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    property->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    property->border_color = table[NEKO_GUI_COLOR_BORDER];
    property->label_normal = table[NEKO_GUI_COLOR_TEXT];
    property->label_hover = table[NEKO_GUI_COLOR_TEXT];
    property->label_active = table[NEKO_GUI_COLOR_TEXT];
    property->sym_left = NEKO_GUI_SYMBOL_TRIANGLE_LEFT;
    property->sym_right = NEKO_GUI_SYMBOL_TRIANGLE_RIGHT;
    property->userdata = neko_gui_handle_ptr(0);
    property->padding = neko_gui_vec2(4, 4);
    property->border = 1;
    property->rounding = 10;
    property->draw_begin = 0;
    property->draw_end = 0;

    button = &style->property.dec_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_PROPERTY];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(0.0f, 0.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;
    style->property.inc_button = style->property.dec_button;

    edit = &style->property.edit;
    neko_gui_zero_struct(*edit);
    edit->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    edit->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    edit->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_PROPERTY]);
    edit->border_color = neko_gui_rgba(0, 0, 0, 0);
    edit->cursor_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->cursor_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->cursor_text_normal = table[NEKO_GUI_COLOR_EDIT];
    edit->cursor_text_hover = table[NEKO_GUI_COLOR_EDIT];
    edit->text_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->text_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->text_active = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_normal = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_hover = table[NEKO_GUI_COLOR_TEXT];
    edit->selected_text_normal = table[NEKO_GUI_COLOR_EDIT];
    edit->selected_text_hover = table[NEKO_GUI_COLOR_EDIT];
    edit->padding = neko_gui_vec2(0, 0);
    edit->cursor_size = 8;
    edit->border = 0;
    edit->rounding = 0;

    chart = &style->chart;
    neko_gui_zero_struct(*chart);
    chart->background = neko_gui_style_item_color(table[NEKO_GUI_COLOR_CHART]);
    chart->border_color = table[NEKO_GUI_COLOR_BORDER];
    chart->selected_color = table[NEKO_GUI_COLOR_CHART_COLOR_HIGHLIGHT];
    chart->color = table[NEKO_GUI_COLOR_CHART_COLOR];
    chart->padding = neko_gui_vec2(4, 4);
    chart->border = 0;
    chart->rounding = 0;

    combo = &style->combo;
    combo->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    combo->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    combo->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    combo->border_color = table[NEKO_GUI_COLOR_BORDER];
    combo->label_normal = table[NEKO_GUI_COLOR_TEXT];
    combo->label_hover = table[NEKO_GUI_COLOR_TEXT];
    combo->label_active = table[NEKO_GUI_COLOR_TEXT];
    combo->sym_normal = NEKO_GUI_SYMBOL_TRIANGLE_DOWN;
    combo->sym_hover = NEKO_GUI_SYMBOL_TRIANGLE_DOWN;
    combo->sym_active = NEKO_GUI_SYMBOL_TRIANGLE_DOWN;
    combo->content_padding = neko_gui_vec2(4, 4);
    combo->button_padding = neko_gui_vec2(0, 4);
    combo->spacing = neko_gui_vec2(4, 0);
    combo->border = 1;
    combo->rounding = 0;

    button = &style->combo.button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_COMBO]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_COMBO];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    tab = &style->tab;
    tab->background = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TAB_HEADER]);
    tab->border_color = table[NEKO_GUI_COLOR_BORDER];
    tab->text = table[NEKO_GUI_COLOR_TEXT];
    tab->sym_minimize = NEKO_GUI_SYMBOL_TRIANGLE_RIGHT;
    tab->sym_maximize = NEKO_GUI_SYMBOL_TRIANGLE_DOWN;
    tab->padding = neko_gui_vec2(4, 4);
    tab->spacing = neko_gui_vec2(4, 4);
    tab->indent = 10.0f;
    tab->border = 1;
    tab->rounding = 0;

    button = &style->tab.tab_minimize_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TAB_HEADER]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TAB_HEADER]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TAB_HEADER]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_TAB_HEADER];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;
    style->tab.tab_maximize_button = *button;

    button = &style->tab.node_minimize_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_TAB_HEADER];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(2.0f, 2.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;
    style->tab.node_maximize_button = *button;

    win = &style->window;
    win->header.align = NEKO_GUI_HEADER_RIGHT;
    win->header.close_symbol = NEKO_GUI_SYMBOL_X;
    win->header.minimize_symbol = NEKO_GUI_SYMBOL_MINUS;
    win->header.maximize_symbol = NEKO_GUI_SYMBOL_PLUS;
    win->header.normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    win->header.hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    win->header.active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    win->header.label_normal = table[NEKO_GUI_COLOR_TEXT];
    win->header.label_hover = table[NEKO_GUI_COLOR_TEXT];
    win->header.label_active = table[NEKO_GUI_COLOR_TEXT];
    win->header.label_padding = neko_gui_vec2(4, 4);
    win->header.padding = neko_gui_vec2(4, 4);
    win->header.spacing = neko_gui_vec2(0, 0);

    button = &style->window.header.close_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_HEADER];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(0.0f, 0.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    button = &style->window.header.minimize_button;
    neko_gui_zero_struct(*button);
    button->normal = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->hover = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->active = neko_gui_style_item_color(table[NEKO_GUI_COLOR_HEADER]);
    button->border_color = neko_gui_rgba(0, 0, 0, 0);
    button->text_background = table[NEKO_GUI_COLOR_HEADER];
    button->text_normal = table[NEKO_GUI_COLOR_TEXT];
    button->text_hover = table[NEKO_GUI_COLOR_TEXT];
    button->text_active = table[NEKO_GUI_COLOR_TEXT];
    button->padding = neko_gui_vec2(0.0f, 0.0f);
    button->touch_padding = neko_gui_vec2(0.0f, 0.0f);
    button->userdata = neko_gui_handle_ptr(0);
    button->text_alignment = NEKO_GUI_TEXT_CENTERED;
    button->border = 0.0f;
    button->rounding = 0.0f;
    button->draw_begin = 0;
    button->draw_end = 0;

    win->background = table[NEKO_GUI_COLOR_WINDOW];
    win->fixed_background = neko_gui_style_item_color(table[NEKO_GUI_COLOR_WINDOW]);
    win->border_color = table[NEKO_GUI_COLOR_BORDER];
    win->popup_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->combo_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->contextual_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->menu_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->group_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->tooltip_border_color = table[NEKO_GUI_COLOR_BORDER];
    win->scaler = neko_gui_style_item_color(table[NEKO_GUI_COLOR_TEXT]);

    win->rounding = 0.0f;
    win->spacing = neko_gui_vec2(4, 4);
    win->scrollbar_size = neko_gui_vec2(10, 10);
    win->min_size = neko_gui_vec2(64, 64);

    win->combo_border = 1.0f;
    win->contextual_border = 1.0f;
    win->menu_border = 1.0f;
    win->group_border = 1.0f;
    win->tooltip_border = 1.0f;
    win->popup_border = 1.0f;
    win->border = 2.0f;
    win->min_row_height_padding = 8;

    win->padding = neko_gui_vec2(4, 4);
    win->group_padding = neko_gui_vec2(4, 4);
    win->popup_padding = neko_gui_vec2(4, 4);
    win->combo_padding = neko_gui_vec2(4, 4);
    win->contextual_padding = neko_gui_vec2(4, 4);
    win->menu_padding = neko_gui_vec2(4, 4);
    win->tooltip_padding = neko_gui_vec2(4, 4);
}
NEKO_GUI_API void neko_gui_style_set_font(struct neko_gui_context *ctx, const struct neko_gui_user_font *font) {
    struct neko_gui_style *style;
    NEKO_GUI_ASSERT(ctx);

    if (!ctx) return;
    style = &ctx->style;
    style->font = font;
    ctx->stacks.fonts.head = 0;
    if (ctx->current) neko_gui_layout_reset_min_row_height(ctx);
}
NEKO_GUI_API neko_gui_bool neko_gui_style_push_font(struct neko_gui_context *ctx, const struct neko_gui_user_font *font) {
    struct neko_gui_config_stack_user_font *font_stack;
    struct neko_gui_config_stack_user_font_element *element;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    font_stack = &ctx->stacks.fonts;
    NEKO_GUI_ASSERT(font_stack->head < (int)NEKO_GUI_LEN(font_stack->elements));
    if (font_stack->head >= (int)NEKO_GUI_LEN(font_stack->elements)) return 0;

    element = &font_stack->elements[font_stack->head++];
    element->address = &ctx->style.font;
    element->old_value = ctx->style.font;
    ctx->style.font = font;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_style_pop_font(struct neko_gui_context *ctx) {
    struct neko_gui_config_stack_user_font *font_stack;
    struct neko_gui_config_stack_user_font_element *element;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    font_stack = &ctx->stacks.fonts;
    NEKO_GUI_ASSERT(font_stack->head > 0);
    if (font_stack->head < 1) return 0;

    element = &font_stack->elements[--font_stack->head];
    *element->address = element->old_value;
    return 1;
}
#define NEKO_GUI_STYLE_PUSH_IMPLEMENATION(prefix, type, stack)                                                  \
    neko_gui_style_push_##type(struct neko_gui_context *ctx, prefix##_##type *address, prefix##_##type value) { \
        struct neko_gui_config_stack_##type *type_stack;                                                        \
        struct neko_gui_config_stack_##type##_element *element;                                                 \
        NEKO_GUI_ASSERT(ctx);                                                                                   \
        if (!ctx) return 0;                                                                                     \
        type_stack = &ctx->stacks.stack;                                                                        \
        NEKO_GUI_ASSERT(type_stack->head < (int)NEKO_GUI_LEN(type_stack->elements));                            \
        if (type_stack->head >= (int)NEKO_GUI_LEN(type_stack->elements)) return 0;                              \
        element = &type_stack->elements[type_stack->head++];                                                    \
        element->address = address;                                                                             \
        element->old_value = *address;                                                                          \
        *address = value;                                                                                       \
        return 1;                                                                                               \
    }
#define NEKO_GUI_STYLE_POP_IMPLEMENATION(type, stack)           \
    neko_gui_style_pop_##type(struct neko_gui_context *ctx) {   \
        struct neko_gui_config_stack_##type *type_stack;        \
        struct neko_gui_config_stack_##type##_element *element; \
        NEKO_GUI_ASSERT(ctx);                                   \
        if (!ctx) return 0;                                     \
        type_stack = &ctx->stacks.stack;                        \
        NEKO_GUI_ASSERT(type_stack->head > 0);                  \
        if (type_stack->head < 1) return 0;                     \
        element = &type_stack->elements[--type_stack->head];    \
        *element->address = element->old_value;                 \
        return 1;                                               \
    }

NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_PUSH_IMPLEMENATION(struct neko_gui, style_item, style_items)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_PUSH_IMPLEMENATION(neko_gui, float, floats)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_PUSH_IMPLEMENATION(struct neko_gui, vec2, vectors)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_PUSH_IMPLEMENATION(neko_gui, flags, flags)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_PUSH_IMPLEMENATION(struct neko_gui, color, colors)

NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_POP_IMPLEMENATION(style_item, style_items)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_POP_IMPLEMENATION(float, floats)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_POP_IMPLEMENATION(vec2, vectors)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_POP_IMPLEMENATION(flags, flags)
NEKO_GUI_API neko_gui_bool NEKO_GUI_STYLE_POP_IMPLEMENATION(color, colors)

NEKO_GUI_API neko_gui_bool neko_gui_style_set_cursor(struct neko_gui_context *ctx, enum neko_gui_style_cursor c) {
    struct neko_gui_style *style;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    style = &ctx->style;
    if (style->cursors[c]) {
        style->cursor_active = style->cursors[c];
        return 1;
    }
    return 0;
}
NEKO_GUI_API void neko_gui_style_show_cursor(struct neko_gui_context *ctx) { ctx->style.cursor_visible = neko_gui_true; }
NEKO_GUI_API void neko_gui_style_hide_cursor(struct neko_gui_context *ctx) { ctx->style.cursor_visible = neko_gui_false; }
NEKO_GUI_API void neko_gui_style_load_cursor(struct neko_gui_context *ctx, enum neko_gui_style_cursor cursor, const struct neko_gui_cursor *c) {
    struct neko_gui_style *style;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    style = &ctx->style;
    style->cursors[cursor] = c;
}
NEKO_GUI_API void neko_gui_style_load_all_cursors(struct neko_gui_context *ctx, struct neko_gui_cursor *cursors) {
    int i = 0;
    struct neko_gui_style *style;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    style = &ctx->style;
    for (i = 0; i < NEKO_GUI_CURSOR_COUNT; ++i) style->cursors[i] = &cursors[i];
    style->cursor_visible = neko_gui_true;
}

NEKO_GUI_INTERN void neko_gui_setup(struct neko_gui_context *ctx, const struct neko_gui_user_font *font) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    neko_gui_zero_struct(*ctx);
    neko_gui_style_default(ctx);
    ctx->seq = 1;
    if (font) ctx->style.font = font;
#ifdef NEKO_GUI_INCLUDE_VERTEX_BUFFER_OUTPUT
    neko_gui_draw_list_init(&ctx->draw_list);
#endif
}
#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API neko_gui_bool neko_gui_init_default(struct neko_gui_context *ctx, const struct neko_gui_user_font *font) {
    struct neko_gui_allocator alloc;
    alloc.userdata.ptr = 0;
    alloc.alloc = neko_gui_malloc;
    alloc.free = neko_gui_mfree;
    return neko_gui_init_impl(ctx, &alloc, font);
}
#endif
NEKO_GUI_API neko_gui_bool neko_gui_init_fixed(struct neko_gui_context *ctx, void *memory, neko_gui_size size, const struct neko_gui_user_font *font) {
    NEKO_GUI_ASSERT(memory);
    if (!memory) return 0;
    neko_gui_setup(ctx, font);
    neko_gui_buffer_init_fixed(&ctx->memory, memory, size);
    ctx->use_pool = neko_gui_false;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_init_custom(struct neko_gui_context *ctx, struct neko_gui_buffer *cmds, struct neko_gui_buffer *pool, const struct neko_gui_user_font *font) {
    NEKO_GUI_ASSERT(cmds);
    NEKO_GUI_ASSERT(pool);
    if (!cmds || !pool) return 0;

    neko_gui_setup(ctx, font);
    ctx->memory = *cmds;
    if (pool->type == NEKO_GUI_BUFFER_FIXED) {

        neko_gui_pool_init_fixed(&ctx->pool, pool->memory.ptr, pool->memory.size);
    } else {

        struct neko_gui_allocator *alloc = &pool->pool;
        neko_gui_pool_init(&ctx->pool, alloc, NEKO_GUI_POOL_DEFAULT_CAPACITY);
    }
    ctx->use_pool = neko_gui_true;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_init_impl(struct neko_gui_context *ctx, struct neko_gui_allocator *alloc, const struct neko_gui_user_font *font) {
    NEKO_GUI_ASSERT(alloc);
    if (!alloc) return 0;
    neko_gui_setup(ctx, font);
    neko_gui_buffer_init(&ctx->memory, alloc, NEKO_GUI_DEFAULT_COMMAND_BUFFER_SIZE);
    neko_gui_pool_init(&ctx->pool, alloc, NEKO_GUI_POOL_DEFAULT_CAPACITY);
    ctx->use_pool = neko_gui_true;
    return 1;
}
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
NEKO_GUI_API void neko_gui_set_user_data(struct neko_gui_context *ctx, neko_gui_handle handle) {
    if (!ctx) return;
    ctx->userdata = handle;
    if (ctx->current) ctx->current->buffer.userdata = handle;
}
#endif
NEKO_GUI_API void neko_gui_free(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    neko_gui_buffer_free(&ctx->memory);
    if (ctx->use_pool) neko_gui_pool_free(&ctx->pool);

    neko_gui_zero(&ctx->input, sizeof(ctx->input));
    neko_gui_zero(&ctx->style, sizeof(ctx->style));
    neko_gui_zero(&ctx->memory, sizeof(ctx->memory));

    ctx->seq = 0;
    ctx->build = 0;
    ctx->begin = 0;
    ctx->end = 0;
    ctx->active = 0;
    ctx->current = 0;
    ctx->freelist = 0;
    ctx->count = 0;
}
NEKO_GUI_API void neko_gui_clear(struct neko_gui_context *ctx) {
    struct neko_gui_window *iter;
    struct neko_gui_window *next;
    NEKO_GUI_ASSERT(ctx);

    if (!ctx) return;
    if (ctx->use_pool)
        neko_gui_buffer_clear(&ctx->memory);
    else
        neko_gui_buffer_reset(&ctx->memory, NEKO_GUI_BUFFER_FRONT);

    ctx->build = 0;
    ctx->memory.calls = 0;
    ctx->last_widget_state = 0;
    ctx->style.cursor_active = ctx->style.cursors[NEKO_GUI_CURSOR_ARROW];
    NEKO_GUI_MEMSET(&ctx->overlay, 0, sizeof(ctx->overlay));

    iter = ctx->begin;
    while (iter) {

        if ((iter->flags & NEKO_GUI_WINDOW_MINIMIZED) && !(iter->flags & NEKO_GUI_WINDOW_CLOSED) && iter->seq == ctx->seq) {
            iter = iter->next;
            continue;
        }

        if (((iter->flags & NEKO_GUI_WINDOW_HIDDEN) || (iter->flags & NEKO_GUI_WINDOW_CLOSED)) && iter == ctx->active) {
            ctx->active = iter->prev;
            ctx->end = iter->prev;
            if (!ctx->end) ctx->begin = 0;
            if (ctx->active) ctx->active->flags &= ~(unsigned)NEKO_GUI_WINDOW_ROM;
        }

        if (iter->popup.win && iter->popup.win->seq != ctx->seq) {
            neko_gui_free_window(ctx, iter->popup.win);
            iter->popup.win = 0;
        }

        {
            struct neko_gui_table *n, *it = iter->tables;
            while (it) {
                n = it->next;
                if (it->seq != ctx->seq) {
                    neko_gui_remove_table(iter, it);
                    neko_gui_zero(it, sizeof(union neko_gui_page_data));
                    neko_gui_free_table(ctx, it);
                    if (it == iter->tables) iter->tables = n;
                }
                it = n;
            }
        }

        if (iter->seq != ctx->seq || iter->flags & NEKO_GUI_WINDOW_CLOSED) {
            next = iter->next;
            neko_gui_remove_window(ctx, iter);
            neko_gui_free_window(ctx, iter);
            iter = next;
        } else
            iter = iter->next;
    }
    ctx->seq++;
}
NEKO_GUI_LIB void neko_gui_start_buffer(struct neko_gui_context *ctx, struct neko_gui_command_buffer *buffer) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(buffer);
    if (!ctx || !buffer) return;
    buffer->begin = ctx->memory.allocated;
    buffer->end = buffer->begin;
    buffer->last = buffer->begin;
    buffer->clip = neko_gui_null_rect;
}
NEKO_GUI_LIB void neko_gui_start(struct neko_gui_context *ctx, struct neko_gui_window *win) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    neko_gui_start_buffer(ctx, &win->buffer);
}
NEKO_GUI_LIB void neko_gui_start_popup(struct neko_gui_context *ctx, struct neko_gui_window *win) {
    struct neko_gui_popup_buffer *buf;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    if (!ctx || !win) return;

    buf = &win->popup.buf;
    buf->begin = win->buffer.end;
    buf->end = win->buffer.end;
    buf->parent = win->buffer.last;
    buf->last = buf->begin;
    buf->active = neko_gui_true;
}
NEKO_GUI_LIB void neko_gui_finish_popup(struct neko_gui_context *ctx, struct neko_gui_window *win) {
    struct neko_gui_popup_buffer *buf;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    if (!ctx || !win) return;

    buf = &win->popup.buf;
    buf->last = win->buffer.last;
    buf->end = win->buffer.end;
}
NEKO_GUI_LIB void neko_gui_finish_buffer(struct neko_gui_context *ctx, struct neko_gui_command_buffer *buffer) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(buffer);
    if (!ctx || !buffer) return;
    buffer->end = ctx->memory.allocated;
}
NEKO_GUI_LIB void neko_gui_finish(struct neko_gui_context *ctx, struct neko_gui_window *win) {
    struct neko_gui_popup_buffer *buf;
    struct neko_gui_command *parent_last;
    void *memory;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    if (!ctx || !win) return;
    neko_gui_finish_buffer(ctx, &win->buffer);
    if (!win->popup.buf.active) return;

    buf = &win->popup.buf;
    memory = ctx->memory.memory.ptr;
    parent_last = neko_gui_ptr_add(struct neko_gui_command, memory, buf->parent);
    parent_last->next = buf->end;
}
NEKO_GUI_LIB void neko_gui_build(struct neko_gui_context *ctx) {
    struct neko_gui_window *it = 0;
    struct neko_gui_command *cmd = 0;
    neko_gui_byte *buffer = 0;

    if (!ctx->style.cursor_active) ctx->style.cursor_active = ctx->style.cursors[NEKO_GUI_CURSOR_ARROW];
    if (ctx->style.cursor_active && !ctx->input.mouse.grabbed && ctx->style.cursor_visible) {
        struct neko_gui_rect mouse_bounds;
        const struct neko_gui_cursor *cursor = ctx->style.cursor_active;
        neko_gui_command_buffer_init(&ctx->overlay, &ctx->memory, NEKO_GUI_CLIPPING_OFF);
        neko_gui_start_buffer(ctx, &ctx->overlay);

        mouse_bounds.x = ctx->input.mouse.pos.x - cursor->offset.x;
        mouse_bounds.y = ctx->input.mouse.pos.y - cursor->offset.y;
        mouse_bounds.w = cursor->size.x;
        mouse_bounds.h = cursor->size.y;

        neko_gui_draw_image(&ctx->overlay, mouse_bounds, &cursor->img, neko_gui_white);
        neko_gui_finish_buffer(ctx, &ctx->overlay);
    }

    it = ctx->begin;
    buffer = (neko_gui_byte *)ctx->memory.memory.ptr;
    while (it != 0) {
        struct neko_gui_window *next = it->next;
        if (it->buffer.last == it->buffer.begin || (it->flags & NEKO_GUI_WINDOW_HIDDEN) || it->seq != ctx->seq) goto cont;

        cmd = neko_gui_ptr_add(struct neko_gui_command, buffer, it->buffer.last);
        while (next && ((next->buffer.last == next->buffer.begin) || (next->flags & NEKO_GUI_WINDOW_HIDDEN) || next->seq != ctx->seq)) next = next->next;

        if (next) cmd->next = next->buffer.begin;
    cont:
        it = next;
    }

    it = ctx->begin;
    while (it != 0) {
        struct neko_gui_window *next = it->next;
        struct neko_gui_popup_buffer *buf;
        if (!it->popup.buf.active) goto skip;

        buf = &it->popup.buf;
        cmd->next = buf->begin;
        cmd = neko_gui_ptr_add(struct neko_gui_command, buffer, buf->last);
        buf->active = neko_gui_false;
    skip:
        it = next;
    }
    if (cmd) {

        if (ctx->overlay.end != ctx->overlay.begin)
            cmd->next = ctx->overlay.begin;
        else
            cmd->next = ctx->memory.allocated;
    }
}
NEKO_GUI_API const struct neko_gui_command *neko_gui__begin(struct neko_gui_context *ctx) {
    struct neko_gui_window *iter;
    neko_gui_byte *buffer;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    if (!ctx->count) return 0;

    buffer = (neko_gui_byte *)ctx->memory.memory.ptr;
    if (!ctx->build) {
        neko_gui_build(ctx);
        ctx->build = neko_gui_true;
    }
    iter = ctx->begin;
    while (iter && ((iter->buffer.begin == iter->buffer.end) || (iter->flags & NEKO_GUI_WINDOW_HIDDEN) || iter->seq != ctx->seq)) iter = iter->next;
    if (!iter) return 0;
    return neko_gui_ptr_add_const(struct neko_gui_command, buffer, iter->buffer.begin);
}

NEKO_GUI_API const struct neko_gui_command *neko_gui__next(struct neko_gui_context *ctx, const struct neko_gui_command *cmd) {
    neko_gui_byte *buffer;
    const struct neko_gui_command *next;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx || !cmd || !ctx->count) return 0;
    if (cmd->next >= ctx->memory.allocated) return 0;
    buffer = (neko_gui_byte *)ctx->memory.memory.ptr;
    next = neko_gui_ptr_add_const(struct neko_gui_command, buffer, cmd->next);
    return next;
}

NEKO_GUI_LIB void neko_gui_pool_init(struct neko_gui_pool *pool, struct neko_gui_allocator *alloc, unsigned int capacity) {
    NEKO_GUI_ASSERT(capacity >= 1);
    neko_gui_zero(pool, sizeof(*pool));
    pool->alloc = *alloc;
    pool->capacity = capacity;
    pool->type = NEKO_GUI_BUFFER_DYNAMIC;
    pool->pages = 0;
}
NEKO_GUI_LIB void neko_gui_pool_free(struct neko_gui_pool *pool) {
    struct neko_gui_page *iter;
    if (!pool) return;
    iter = pool->pages;
    if (pool->type == NEKO_GUI_BUFFER_FIXED) return;
    while (iter) {
        struct neko_gui_page *next = iter->next;
        pool->alloc.free(pool->alloc.userdata, iter);
        iter = next;
    }
}
NEKO_GUI_LIB void neko_gui_pool_init_fixed(struct neko_gui_pool *pool, void *memory, neko_gui_size size) {
    neko_gui_zero(pool, sizeof(*pool));
    NEKO_GUI_ASSERT(size >= sizeof(struct neko_gui_page));
    if (size < sizeof(struct neko_gui_page)) return;

    pool->capacity = (unsigned)(1 + (size - sizeof(struct neko_gui_page)) / sizeof(struct neko_gui_page_element));
    pool->pages = (struct neko_gui_page *)memory;
    pool->type = NEKO_GUI_BUFFER_FIXED;
    pool->size = size;
}
NEKO_GUI_LIB struct neko_gui_page_element *neko_gui_pool_alloc(struct neko_gui_pool *pool) {
    if (!pool->pages || pool->pages->size >= pool->capacity) {

        struct neko_gui_page *page;
        if (pool->type == NEKO_GUI_BUFFER_FIXED) {
            NEKO_GUI_ASSERT(pool->pages);
            if (!pool->pages) return 0;
            NEKO_GUI_ASSERT(pool->pages->size < pool->capacity);
            return 0;
        } else {
            neko_gui_size size = sizeof(struct neko_gui_page);
            size += (pool->capacity - 1) * sizeof(struct neko_gui_page_element);
            page = (struct neko_gui_page *)pool->alloc.alloc(pool->alloc.userdata, 0, size);
            page->next = pool->pages;
            pool->pages = page;
            page->size = 0;
        }
    }
    return &pool->pages->win[pool->pages->size++];
}

NEKO_GUI_LIB struct neko_gui_page_element *neko_gui_create_page_element(struct neko_gui_context *ctx) {
    struct neko_gui_page_element *elem;
    if (ctx->freelist) {

        elem = ctx->freelist;
        ctx->freelist = elem->next;
    } else if (ctx->use_pool) {

        elem = neko_gui_pool_alloc(&ctx->pool);
        NEKO_GUI_ASSERT(elem);
        if (!elem) return 0;
    } else {

        NEKO_GUI_STORAGE const neko_gui_size size = sizeof(struct neko_gui_page_element);
        NEKO_GUI_STORAGE const neko_gui_size align = NEKO_GUI_ALIGNOF(struct neko_gui_page_element);
        elem = (struct neko_gui_page_element *)neko_gui_buffer_alloc(&ctx->memory, NEKO_GUI_BUFFER_BACK, size, align);
        NEKO_GUI_ASSERT(elem);
        if (!elem) return 0;
    }
    neko_gui_zero_struct(*elem);
    elem->next = 0;
    elem->prev = 0;
    return elem;
}
NEKO_GUI_LIB void neko_gui_lineko_gui_page_element_into_freelist(struct neko_gui_context *ctx, struct neko_gui_page_element *elem) {

    if (!ctx->freelist) {
        ctx->freelist = elem;
    } else {
        elem->next = ctx->freelist;
        ctx->freelist = elem;
    }
}
NEKO_GUI_LIB void neko_gui_free_page_element(struct neko_gui_context *ctx, struct neko_gui_page_element *elem) {

    if (ctx->use_pool) {
        neko_gui_lineko_gui_page_element_into_freelist(ctx, elem);
        return;
    }

    {
        void *elem_end = (void *)(elem + 1);
        void *buffer_end = (neko_gui_byte *)ctx->memory.memory.ptr + ctx->memory.size;
        if (elem_end == buffer_end)
            ctx->memory.size -= sizeof(struct neko_gui_page_element);
        else
            neko_gui_lineko_gui_page_element_into_freelist(ctx, elem);
    }
}

NEKO_GUI_LIB struct neko_gui_table *neko_gui_create_table(struct neko_gui_context *ctx) {
    struct neko_gui_page_element *elem;
    elem = neko_gui_create_page_element(ctx);
    if (!elem) return 0;
    neko_gui_zero_struct(*elem);
    return &elem->data.tbl;
}
NEKO_GUI_LIB void neko_gui_free_table(struct neko_gui_context *ctx, struct neko_gui_table *tbl) {
    union neko_gui_page_data *pd = NEKO_GUI_CONTAINER_OF(tbl, union neko_gui_page_data, tbl);
    struct neko_gui_page_element *pe = NEKO_GUI_CONTAINER_OF(pd, struct neko_gui_page_element, data);
    neko_gui_free_page_element(ctx, pe);
}
NEKO_GUI_LIB void neko_gui_push_table(struct neko_gui_window *win, struct neko_gui_table *tbl) {
    if (!win->tables) {
        win->tables = tbl;
        tbl->next = 0;
        tbl->prev = 0;
        tbl->size = 0;
        win->table_count = 1;
        return;
    }
    win->tables->prev = tbl;
    tbl->next = win->tables;
    tbl->prev = 0;
    tbl->size = 0;
    win->tables = tbl;
    win->table_count++;
}
NEKO_GUI_LIB void neko_gui_remove_table(struct neko_gui_window *win, struct neko_gui_table *tbl) {
    if (win->tables == tbl) win->tables = tbl->next;
    if (tbl->next) tbl->next->prev = tbl->prev;
    if (tbl->prev) tbl->prev->next = tbl->next;
    tbl->next = 0;
    tbl->prev = 0;
}
NEKO_GUI_LIB neko_gui_uint *neko_gui_add_value(struct neko_gui_context *ctx, struct neko_gui_window *win, neko_gui_hash name, neko_gui_uint value) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    if (!win || !ctx) return 0;
    if (!win->tables || win->tables->size >= NEKO_GUI_VALUE_PAGE_CAPACITY) {
        struct neko_gui_table *tbl = neko_gui_create_table(ctx);
        NEKO_GUI_ASSERT(tbl);
        if (!tbl) return 0;
        neko_gui_push_table(win, tbl);
    }
    win->tables->seq = win->seq;
    win->tables->keys[win->tables->size] = name;
    win->tables->values[win->tables->size] = value;
    return &win->tables->values[win->tables->size++];
}
NEKO_GUI_LIB neko_gui_uint *neko_gui_find_value(struct neko_gui_window *win, neko_gui_hash name) {
    struct neko_gui_table *iter = win->tables;
    while (iter) {
        unsigned int i = 0;
        unsigned int size = iter->size;
        for (i = 0; i < size; ++i) {
            if (iter->keys[i] == name) {
                iter->seq = win->seq;
                return &iter->values[i];
            }
        }
        size = NEKO_GUI_VALUE_PAGE_CAPACITY;
        iter = iter->next;
    }
    return 0;
}

NEKO_GUI_LIB void *neko_gui_create_panel(struct neko_gui_context *ctx) {
    struct neko_gui_page_element *elem;
    elem = neko_gui_create_page_element(ctx);
    if (!elem) return 0;
    neko_gui_zero_struct(*elem);
    return &elem->data.pan;
}
NEKO_GUI_LIB void neko_gui_free_panel(struct neko_gui_context *ctx, struct neko_gui_panel *pan) {
    union neko_gui_page_data *pd = NEKO_GUI_CONTAINER_OF(pan, union neko_gui_page_data, pan);
    struct neko_gui_page_element *pe = NEKO_GUI_CONTAINER_OF(pd, struct neko_gui_page_element, data);
    neko_gui_free_page_element(ctx, pe);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_has_header(neko_gui_flags flags, const char *title) {
    neko_gui_bool active = 0;
    active = (flags & (NEKO_GUI_WINDOW_CLOSABLE | NEKO_GUI_WINDOW_MINIMIZABLE));
    active = active || (flags & NEKO_GUI_WINDOW_TITLE);
    active = active && !(flags & NEKO_GUI_WINDOW_HIDDEN) && title;
    return active;
}
NEKO_GUI_LIB struct neko_gui_vec2 neko_gui_panel_get_padding(const struct neko_gui_style *style, enum neko_gui_panel_type type) {
    switch (type) {
        default:
        case NEKO_GUI_PANEL_WINDOW:
            return style->window.padding;
        case NEKO_GUI_PANEL_GROUP:
            return style->window.group_padding;
        case NEKO_GUI_PANEL_POPUP:
            return style->window.popup_padding;
        case NEKO_GUI_PANEL_CONTEXTUAL:
            return style->window.contextual_padding;
        case NEKO_GUI_PANEL_COMBO:
            return style->window.combo_padding;
        case NEKO_GUI_PANEL_MENU:
            return style->window.menu_padding;
        case NEKO_GUI_PANEL_TOOLTIP:
            return style->window.menu_padding;
    }
}
NEKO_GUI_LIB float neko_gui_panel_get_border(const struct neko_gui_style *style, neko_gui_flags flags, enum neko_gui_panel_type type) {
    if (flags & NEKO_GUI_WINDOW_BORDER) {
        switch (type) {
            default:
            case NEKO_GUI_PANEL_WINDOW:
                return style->window.border;
            case NEKO_GUI_PANEL_GROUP:
                return style->window.group_border;
            case NEKO_GUI_PANEL_POPUP:
                return style->window.popup_border;
            case NEKO_GUI_PANEL_CONTEXTUAL:
                return style->window.contextual_border;
            case NEKO_GUI_PANEL_COMBO:
                return style->window.combo_border;
            case NEKO_GUI_PANEL_MENU:
                return style->window.menu_border;
            case NEKO_GUI_PANEL_TOOLTIP:
                return style->window.menu_border;
        }
    } else
        return 0;
}
NEKO_GUI_LIB struct neko_gui_color neko_gui_panel_get_border_color(const struct neko_gui_style *style, enum neko_gui_panel_type type) {
    switch (type) {
        default:
        case NEKO_GUI_PANEL_WINDOW:
            return style->window.border_color;
        case NEKO_GUI_PANEL_GROUP:
            return style->window.group_border_color;
        case NEKO_GUI_PANEL_POPUP:
            return style->window.popup_border_color;
        case NEKO_GUI_PANEL_CONTEXTUAL:
            return style->window.contextual_border_color;
        case NEKO_GUI_PANEL_COMBO:
            return style->window.combo_border_color;
        case NEKO_GUI_PANEL_MENU:
            return style->window.menu_border_color;
        case NEKO_GUI_PANEL_TOOLTIP:
            return style->window.menu_border_color;
    }
}
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_is_sub(enum neko_gui_panel_type type) { return (type & NEKO_GUI_PANEL_SET_SUB) ? 1 : 0; }
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_is_nonblock(enum neko_gui_panel_type type) { return (type & NEKO_GUI_PANEL_SET_NONBLOCK) ? 1 : 0; }
NEKO_GUI_LIB neko_gui_bool neko_gui_panel_begin(struct neko_gui_context *ctx, const char *title, enum neko_gui_panel_type panel_type) {
    struct neko_gui_input *in;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    struct neko_gui_command_buffer *out;
    const struct neko_gui_style *style;
    const struct neko_gui_user_font *font;

    struct neko_gui_vec2 scrollbar_size;
    struct neko_gui_vec2 panel_padding;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;
    neko_gui_zero(ctx->current->layout, sizeof(*ctx->current->layout));
    if ((ctx->current->flags & NEKO_GUI_WINDOW_HIDDEN) || (ctx->current->flags & NEKO_GUI_WINDOW_CLOSED)) {
        neko_gui_zero(ctx->current->layout, sizeof(struct neko_gui_panel));
        ctx->current->layout->type = panel_type;
        return 0;
    }

    style = &ctx->style;
    font = style->font;
    win = ctx->current;
    layout = win->layout;
    out = &win->buffer;
    in = (win->flags & NEKO_GUI_WINDOW_NO_INPUT) ? 0 : &ctx->input;
#ifdef NEKO_GUI_INCLUDE_COMMAND_USERDATA
    win->buffer.userdata = ctx->userdata;
#endif

    scrollbar_size = style->window.scrollbar_size;
    panel_padding = neko_gui_panel_get_padding(style, panel_type);

    if ((win->flags & NEKO_GUI_WINDOW_MOVABLE) && !(win->flags & NEKO_GUI_WINDOW_ROM)) {
        neko_gui_bool left_mouse_down;
        unsigned int left_mouse_clicked;
        int left_mouse_click_in_cursor;

        struct neko_gui_rect header;
        header.x = win->bounds.x;
        header.y = win->bounds.y;
        header.w = win->bounds.w;
        if (neko_gui_panel_has_header(win->flags, title)) {
            header.h = font->height + 2.0f * style->window.header.padding.y;
            header.h += 2.0f * style->window.header.label_padding.y;
        } else
            header.h = panel_padding.y;

        left_mouse_down = in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
        left_mouse_clicked = in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked;
        left_mouse_click_in_cursor = neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, header, neko_gui_true);
        if (left_mouse_down && left_mouse_click_in_cursor && !left_mouse_clicked) {
            win->bounds.x = win->bounds.x + in->mouse.delta.x;
            win->bounds.y = win->bounds.y + in->mouse.delta.y;
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.x += in->mouse.delta.x;
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.y += in->mouse.delta.y;
            ctx->style.cursor_active = ctx->style.cursors[NEKO_GUI_CURSOR_MOVE];
        }
    }

    layout->type = panel_type;
    layout->flags = win->flags;
    layout->bounds = win->bounds;
    layout->bounds.x += panel_padding.x;
    layout->bounds.w -= 2 * panel_padding.x;
    if (win->flags & NEKO_GUI_WINDOW_BORDER) {
        layout->border = neko_gui_panel_get_border(style, win->flags, panel_type);
        layout->bounds = neko_gui_shrineko_gui_rect(layout->bounds, layout->border);
    } else
        layout->border = 0;
    layout->at_y = layout->bounds.y;
    layout->at_x = layout->bounds.x;
    layout->max_x = 0;
    layout->header_height = 0;
    layout->footer_height = 0;
    neko_gui_layout_reset_min_row_height(ctx);
    layout->row.index = 0;
    layout->row.columns = 0;
    layout->row.ratio = 0;
    layout->row.item_width = 0;
    layout->row.tree_depth = 0;
    layout->row.height = panel_padding.y;
    layout->has_scrolling = neko_gui_true;
    if (!(win->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR)) layout->bounds.w -= scrollbar_size.x;
    if (!neko_gui_panel_is_nonblock(panel_type)) {
        layout->footer_height = 0;
        if (!(win->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR) || win->flags & NEKO_GUI_WINDOW_SCALABLE) layout->footer_height = scrollbar_size.y;
        layout->bounds.h -= layout->footer_height;
    }

    if (neko_gui_panel_has_header(win->flags, title)) {
        struct neko_gui_text text;
        struct neko_gui_rect header;
        const struct neko_gui_style_item *background = 0;

        header.x = win->bounds.x;
        header.y = win->bounds.y;
        header.w = win->bounds.w;
        header.h = font->height + 2.0f * style->window.header.padding.y;
        header.h += (2.0f * style->window.header.label_padding.y);

        layout->header_height = header.h;
        layout->bounds.y += header.h;
        layout->bounds.h -= header.h;
        layout->at_y += header.h;

        if (ctx->active == win) {
            background = &style->window.header.active;
            text.text = style->window.header.label_active;
        } else if (neko_gui_input_is_mouse_hovering_rect(&ctx->input, header)) {
            background = &style->window.header.hover;
            text.text = style->window.header.label_hover;
        } else {
            background = &style->window.header.normal;
            text.text = style->window.header.label_normal;
        }

        header.h += 1.0f;

        switch (background->type) {
            case NEKO_GUI_STYLE_ITEM_IMAGE:
                text.background = neko_gui_rgba(0, 0, 0, 0);
                neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
                text.background = neko_gui_rgba(0, 0, 0, 0);
                neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_COLOR:
                text.background = background->data.color;
                neko_gui_fill_rect(out, header, 0, background->data.color);
                break;
        }

        {
            struct neko_gui_rect button;
            button.y = header.y + style->window.header.padding.y;
            button.h = header.h - 2 * style->window.header.padding.y;
            button.w = button.h;
            if (win->flags & NEKO_GUI_WINDOW_CLOSABLE) {
                neko_gui_flags ws = 0;
                if (style->window.header.align == NEKO_GUI_HEADER_RIGHT) {
                    button.x = (header.w + header.x) - (button.w + style->window.header.padding.x);
                    header.w -= button.w + style->window.header.spacing.x + style->window.header.padding.x;
                } else {
                    button.x = header.x + style->window.header.padding.x;
                    header.x += button.w + style->window.header.spacing.x + style->window.header.padding.x;
                }

                if (neko_gui_do_button_symbol(&ws, &win->buffer, button, style->window.header.close_symbol, NEKO_GUI_BUTTON_DEFAULT, &style->window.header.close_button, in, style->font) &&
                    !(win->flags & NEKO_GUI_WINDOW_ROM)) {
                    layout->flags |= NEKO_GUI_WINDOW_HIDDEN;
                    layout->flags &= (neko_gui_flags)~NEKO_GUI_WINDOW_MINIMIZED;
                }
            }

            if (win->flags & NEKO_GUI_WINDOW_MINIMIZABLE) {
                neko_gui_flags ws = 0;
                if (style->window.header.align == NEKO_GUI_HEADER_RIGHT) {
                    button.x = (header.w + header.x) - button.w;
                    if (!(win->flags & NEKO_GUI_WINDOW_CLOSABLE)) {
                        button.x -= style->window.header.padding.x;
                        header.w -= style->window.header.padding.x;
                    }
                    header.w -= button.w + style->window.header.spacing.x;
                } else {
                    button.x = header.x;
                    header.x += button.w + style->window.header.spacing.x + style->window.header.padding.x;
                }
                if (neko_gui_do_button_symbol(&ws, &win->buffer, button, (layout->flags & NEKO_GUI_WINDOW_MINIMIZED) ? style->window.header.maximize_symbol : style->window.header.minimize_symbol,
                                              NEKO_GUI_BUTTON_DEFAULT, &style->window.header.minimize_button, in, style->font) &&
                    !(win->flags & NEKO_GUI_WINDOW_ROM))
                    layout->flags = (layout->flags & NEKO_GUI_WINDOW_MINIMIZED) ? layout->flags & (neko_gui_flags)~NEKO_GUI_WINDOW_MINIMIZED : layout->flags | NEKO_GUI_WINDOW_MINIMIZED;
            }
        }

        {
            int text_len = neko_gui_strlen(title);
            struct neko_gui_rect label = {0, 0, 0, 0};
            float t = font->width(font->userdata, font->height, title, text_len);
            text.padding = neko_gui_vec2(0, 0);

            label.x = header.x + style->window.header.padding.x;
            label.x += style->window.header.label_padding.x;
            label.y = header.y + style->window.header.label_padding.y;
            label.h = font->height + 2 * style->window.header.label_padding.y;
            label.w = t + 2 * style->window.header.spacing.x;
            label.w = NEKO_GUI_CLAMP(0, label.w, header.x + header.w - label.x);
            neko_gui_widget_text(out, label, (const char *)title, text_len, &text, NEKO_GUI_TEXT_LEFT, font);
        }
    }

    if (!(layout->flags & NEKO_GUI_WINDOW_MINIMIZED) && !(layout->flags & NEKO_GUI_WINDOW_DYNAMIC)) {
        struct neko_gui_rect body;
        body.x = win->bounds.x;
        body.w = win->bounds.w;
        body.y = (win->bounds.y + layout->header_height);
        body.h = (win->bounds.h - layout->header_height);

        switch (style->window.fixed_background.type) {
            case NEKO_GUI_STYLE_ITEM_IMAGE:
                neko_gui_draw_image(out, body, &style->window.fixed_background.data.image, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
                neko_gui_draw_nine_slice(out, body, &style->window.fixed_background.data.slice, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_COLOR:
                neko_gui_fill_rect(out, body, 0, style->window.fixed_background.data.color);
                break;
        }
    }

    {
        struct neko_gui_rect clip;
        layout->clip = layout->bounds;
        neko_gui_unify(&clip, &win->buffer.clip, layout->clip.x, layout->clip.y, layout->clip.x + layout->clip.w, layout->clip.y + layout->clip.h);
        neko_gui_push_scissor(out, clip);
        layout->clip = clip;
    }
    return !(layout->flags & NEKO_GUI_WINDOW_HIDDEN) && !(layout->flags & NEKO_GUI_WINDOW_MINIMIZED);
}
NEKO_GUI_LIB void neko_gui_panel_end(struct neko_gui_context *ctx) {
    struct neko_gui_input *in;
    struct neko_gui_window *window;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_command_buffer *out;

    struct neko_gui_vec2 scrollbar_size;
    struct neko_gui_vec2 panel_padding;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    window = ctx->current;
    layout = window->layout;
    style = &ctx->style;
    out = &window->buffer;
    in = (layout->flags & NEKO_GUI_WINDOW_ROM || layout->flags & NEKO_GUI_WINDOW_NO_INPUT) ? 0 : &ctx->input;
    if (!neko_gui_panel_is_sub(layout->type)) neko_gui_push_scissor(out, neko_gui_null_rect);

    scrollbar_size = style->window.scrollbar_size;
    panel_padding = neko_gui_panel_get_padding(style, layout->type);

    layout->at_y += layout->row.height;

    if (layout->flags & NEKO_GUI_WINDOW_DYNAMIC && !(layout->flags & NEKO_GUI_WINDOW_MINIMIZED)) {

        struct neko_gui_rect empty_space;
        if (layout->at_y < (layout->bounds.y + layout->bounds.h)) layout->bounds.h = layout->at_y - layout->bounds.y;

        empty_space.x = window->bounds.x;
        empty_space.y = layout->bounds.y;
        empty_space.h = panel_padding.y;
        empty_space.w = window->bounds.w;
        neko_gui_fill_rect(out, empty_space, 0, style->window.background);

        empty_space.x = window->bounds.x;
        empty_space.y = layout->bounds.y;
        empty_space.w = panel_padding.x + layout->border;
        empty_space.h = layout->bounds.h;
        neko_gui_fill_rect(out, empty_space, 0, style->window.background);

        empty_space.x = layout->bounds.x + layout->bounds.w;
        empty_space.y = layout->bounds.y;
        empty_space.w = panel_padding.x + layout->border;
        empty_space.h = layout->bounds.h;
        if (*layout->offset_y == 0 && !(layout->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR)) empty_space.w += scrollbar_size.x;
        neko_gui_fill_rect(out, empty_space, 0, style->window.background);

        if (layout->footer_height > 0) {
            empty_space.x = window->bounds.x;
            empty_space.y = layout->bounds.y + layout->bounds.h;
            empty_space.w = window->bounds.w;
            empty_space.h = layout->footer_height;
            neko_gui_fill_rect(out, empty_space, 0, style->window.background);
        }
    }

    if (!(layout->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR) && !(layout->flags & NEKO_GUI_WINDOW_MINIMIZED) && window->scrollbar_hiding_timer < NEKO_GUI_SCROLLBAR_HIDING_TIMEOUT) {
        struct neko_gui_rect scroll;
        int scroll_has_scrolling;
        float scroll_target;
        float scroll_offset;
        float scroll_step;
        float scroll_inc;

        if (neko_gui_panel_is_sub(layout->type)) {

            struct neko_gui_window *root_window = window;
            struct neko_gui_panel *root_panel = window->layout;
            while (root_panel->parent) root_panel = root_panel->parent;
            while (root_window->parent) root_window = root_window->parent;

            scroll_has_scrolling = 0;
            if ((root_window == ctx->active) && layout->has_scrolling) {

                if (neko_gui_input_is_mouse_hovering_rect(in, layout->bounds) &&
                    NEKO_GUI_INTERSECT(layout->bounds.x, layout->bounds.y, layout->bounds.w, layout->bounds.h, root_panel->clip.x, root_panel->clip.y, root_panel->clip.w, root_panel->clip.h)) {

                    root_panel = window->layout;
                    while (root_panel->parent) {
                        root_panel->has_scrolling = neko_gui_false;
                        root_panel = root_panel->parent;
                    }
                    root_panel->has_scrolling = neko_gui_false;
                    scroll_has_scrolling = neko_gui_true;
                }
            }
        } else if (!neko_gui_panel_is_sub(layout->type)) {

            scroll_has_scrolling = (window == ctx->active) && layout->has_scrolling;
            if (in && (in->mouse.scroll_delta.y > 0 || in->mouse.scroll_delta.x > 0) && scroll_has_scrolling)
                window->scrolled = neko_gui_true;
            else
                window->scrolled = neko_gui_false;
        } else
            scroll_has_scrolling = neko_gui_false;

        {

            neko_gui_flags state = 0;
            scroll.x = layout->bounds.x + layout->bounds.w + panel_padding.x;
            scroll.y = layout->bounds.y;
            scroll.w = scrollbar_size.x;
            scroll.h = layout->bounds.h;

            scroll_offset = (float)*layout->offset_y;
            scroll_step = scroll.h * 0.10f;
            scroll_inc = scroll.h * 0.01f;
            scroll_target = (float)(int)(layout->at_y - scroll.y);
            scroll_offset = neko_gui_do_scrollbarv(&state, out, scroll, scroll_has_scrolling, scroll_offset, scroll_target, scroll_step, scroll_inc, &ctx->style.scrollv, in, style->font);
            *layout->offset_y = (neko_gui_uint)scroll_offset;
            if (in && scroll_has_scrolling) in->mouse.scroll_delta.y = 0;
        }
        {

            neko_gui_flags state = 0;
            scroll.x = layout->bounds.x;
            scroll.y = layout->bounds.y + layout->bounds.h;
            scroll.w = layout->bounds.w;
            scroll.h = scrollbar_size.y;

            scroll_offset = (float)*layout->offset_x;
            scroll_target = (float)(int)(layout->max_x - scroll.x);
            scroll_step = layout->max_x * 0.05f;
            scroll_inc = layout->max_x * 0.005f;
            scroll_offset = neko_gui_do_scrollbarh(&state, out, scroll, scroll_has_scrolling, scroll_offset, scroll_target, scroll_step, scroll_inc, &ctx->style.scrollh, in, style->font);
            *layout->offset_x = (neko_gui_uint)scroll_offset;
        }
    }

    if (window->flags & NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE) {
        int has_input = ctx->input.mouse.delta.x != 0 || ctx->input.mouse.delta.y != 0 || ctx->input.mouse.scroll_delta.y != 0;
        int is_window_hovered = neko_gui_window_is_hovered(ctx);
        int any_item_active = (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_MODIFIED);
        if ((!has_input && is_window_hovered) || (!is_window_hovered && !any_item_active))
            window->scrollbar_hiding_timer += ctx->delta_time_seconds;
        else
            window->scrollbar_hiding_timer = 0;
    } else
        window->scrollbar_hiding_timer = 0;

    if (layout->flags & NEKO_GUI_WINDOW_BORDER) {
        struct neko_gui_color border_color = neko_gui_panel_get_border_color(style, layout->type);
        const float padding_y = (layout->flags & NEKO_GUI_WINDOW_MINIMIZED)
                                        ? (style->window.border + window->bounds.y + layout->header_height)
                                        : ((layout->flags & NEKO_GUI_WINDOW_DYNAMIC) ? (layout->bounds.y + layout->bounds.h + layout->footer_height) : (window->bounds.y + window->bounds.h));
        struct neko_gui_rect b = window->bounds;
        b.h = padding_y - window->bounds.y;
        neko_gui_stroke_rect(out, b, 0, layout->border, border_color);
    }

    if ((layout->flags & NEKO_GUI_WINDOW_SCALABLE) && in && !(layout->flags & NEKO_GUI_WINDOW_MINIMIZED)) {

        struct neko_gui_rect scaler;
        scaler.w = scrollbar_size.x;
        scaler.h = scrollbar_size.y;
        scaler.y = layout->bounds.y + layout->bounds.h;
        if (layout->flags & NEKO_GUI_WINDOW_SCALE_LEFT)
            scaler.x = layout->bounds.x - panel_padding.x * 0.5f;
        else
            scaler.x = layout->bounds.x + layout->bounds.w + panel_padding.x;
        if (layout->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR) scaler.x -= scaler.w;

        {
            const struct neko_gui_style_item *item = &style->window.scaler;
            if (item->type == NEKO_GUI_STYLE_ITEM_IMAGE)
                neko_gui_draw_image(out, scaler, &item->data.image, neko_gui_white);
            else {
                if (layout->flags & NEKO_GUI_WINDOW_SCALE_LEFT) {
                    neko_gui_fill_triangle(out, scaler.x, scaler.y, scaler.x, scaler.y + scaler.h, scaler.x + scaler.w, scaler.y + scaler.h, item->data.color);
                } else {
                    neko_gui_fill_triangle(out, scaler.x + scaler.w, scaler.y, scaler.x + scaler.w, scaler.y + scaler.h, scaler.x, scaler.y + scaler.h, item->data.color);
                }
            }
        }

        if (!(window->flags & NEKO_GUI_WINDOW_ROM)) {
            struct neko_gui_vec2 window_size = style->window.min_size;
            int left_mouse_down = in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
            int left_mouse_click_in_scaler = neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, scaler, neko_gui_true);

            if (left_mouse_down && left_mouse_click_in_scaler) {
                float delta_x = in->mouse.delta.x;
                if (layout->flags & NEKO_GUI_WINDOW_SCALE_LEFT) {
                    delta_x = -delta_x;
                    window->bounds.x += in->mouse.delta.x;
                }

                if (window->bounds.w + delta_x >= window_size.x) {
                    if ((delta_x < 0) || (delta_x > 0 && in->mouse.pos.x >= scaler.x)) {
                        window->bounds.w = window->bounds.w + delta_x;
                        scaler.x += in->mouse.delta.x;
                    }
                }

                if (!(layout->flags & NEKO_GUI_WINDOW_DYNAMIC)) {
                    if (window_size.y < window->bounds.h + in->mouse.delta.y) {
                        if ((in->mouse.delta.y < 0) || (in->mouse.delta.y > 0 && in->mouse.pos.y >= scaler.y)) {
                            window->bounds.h = window->bounds.h + in->mouse.delta.y;
                            scaler.y += in->mouse.delta.y;
                        }
                    }
                }
                ctx->style.cursor_active = ctx->style.cursors[NEKO_GUI_CURSOR_RESIZE_TOP_RIGHT_DOWN_LEFT];
                in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.x = scaler.x + scaler.w / 2.0f;
                in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.y = scaler.y + scaler.h / 2.0f;
            }
        }
    }
    if (!neko_gui_panel_is_sub(layout->type)) {

        if (layout->flags & NEKO_GUI_WINDOW_HIDDEN)
            neko_gui_command_buffer_reset(&window->buffer);

        else
            neko_gui_finish(ctx, window);
    }

    if (layout->flags & NEKO_GUI_WINDOW_REMOVE_ROM) {
        layout->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
        layout->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_REMOVE_ROM;
    }
    window->flags = layout->flags;

    if (window->property.active && window->property.old != window->property.seq && window->property.active == window->property.prev) {
        neko_gui_zero(&window->property, sizeof(window->property));
    } else {
        window->property.old = window->property.seq;
        window->property.prev = window->property.active;
        window->property.seq = 0;
    }

    if (window->edit.active && window->edit.old != window->edit.seq && window->edit.active == window->edit.prev) {
        neko_gui_zero(&window->edit, sizeof(window->edit));
    } else {
        window->edit.old = window->edit.seq;
        window->edit.prev = window->edit.active;
        window->edit.seq = 0;
    }

    if (window->popup.active_con && window->popup.con_old != window->popup.con_count) {
        window->popup.con_count = 0;
        window->popup.con_old = 0;
        window->popup.active_con = 0;
    } else {
        window->popup.con_old = window->popup.con_count;
        window->popup.con_count = 0;
    }
    window->popup.combo_count = 0;

    NEKO_GUI_ASSERT(!layout->row.tree_depth);
}

NEKO_GUI_LIB void *neko_gui_create_window(struct neko_gui_context *ctx) {
    struct neko_gui_page_element *elem;
    elem = neko_gui_create_page_element(ctx);
    if (!elem) return 0;
    elem->data.win.seq = ctx->seq;
    return &elem->data.win;
}
NEKO_GUI_LIB void neko_gui_free_window(struct neko_gui_context *ctx, struct neko_gui_window *win) {

    struct neko_gui_table *it = win->tables;
    if (win->popup.win) {
        neko_gui_free_window(ctx, win->popup.win);
        win->popup.win = 0;
    }
    win->next = 0;
    win->prev = 0;

    while (it) {

        struct neko_gui_table *n = it->next;
        neko_gui_remove_table(win, it);
        neko_gui_free_table(ctx, it);
        if (it == win->tables) win->tables = n;
        it = n;
    }

    {
        union neko_gui_page_data *pd = NEKO_GUI_CONTAINER_OF(win, union neko_gui_page_data, win);
        struct neko_gui_page_element *pe = NEKO_GUI_CONTAINER_OF(pd, struct neko_gui_page_element, data);
        neko_gui_free_page_element(ctx, pe);
    }
}
NEKO_GUI_LIB struct neko_gui_window *neko_gui_find_window(struct neko_gui_context *ctx, neko_gui_hash hash, const char *name) {
    struct neko_gui_window *iter;
    iter = ctx->begin;
    while (iter) {
        NEKO_GUI_ASSERT(iter != iter->next);
        if (iter->name == hash) {
            int max_len = neko_gui_strlen(iter->name_string);
            if (!neko_gui_stricmpn(iter->name_string, name, max_len)) return iter;
        }
        iter = iter->next;
    }
    return 0;
}
NEKO_GUI_LIB void neko_gui_insert_window(struct neko_gui_context *ctx, struct neko_gui_window *win, enum neko_gui_window_insert_location loc) {
    const struct neko_gui_window *iter;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(win);
    if (!win || !ctx) return;

    iter = ctx->begin;
    while (iter) {
        NEKO_GUI_ASSERT(iter != iter->next);
        NEKO_GUI_ASSERT(iter != win);
        if (iter == win) return;
        iter = iter->next;
    }

    if (!ctx->begin) {
        win->next = 0;
        win->prev = 0;
        ctx->begin = win;
        ctx->end = win;
        ctx->count = 1;
        return;
    }
    if (loc == NEKO_GUI_INSERT_BACK) {
        struct neko_gui_window *end;
        end = ctx->end;
        end->flags |= NEKO_GUI_WINDOW_ROM;
        end->next = win;
        win->prev = ctx->end;
        win->next = 0;
        ctx->end = win;
        ctx->active = ctx->end;
        ctx->end->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
    } else {

        ctx->begin->prev = win;
        win->next = ctx->begin;
        win->prev = 0;
        ctx->begin = win;
        ctx->begin->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
    }
    ctx->count++;
}
NEKO_GUI_LIB void neko_gui_remove_window(struct neko_gui_context *ctx, struct neko_gui_window *win) {
    if (win == ctx->begin || win == ctx->end) {
        if (win == ctx->begin) {
            ctx->begin = win->next;
            if (win->next) win->next->prev = 0;
        }
        if (win == ctx->end) {
            ctx->end = win->prev;
            if (win->prev) win->prev->next = 0;
        }
    } else {
        if (win->next) win->next->prev = win->prev;
        if (win->prev) win->prev->next = win->next;
    }
    if (win == ctx->active || !ctx->active) {
        ctx->active = ctx->end;
        if (ctx->end) ctx->end->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
    }
    win->next = 0;
    win->prev = 0;
    ctx->count--;
}
NEKO_GUI_API neko_gui_bool neko_gui_begin(struct neko_gui_context *ctx, const char *title, struct neko_gui_rect bounds, neko_gui_flags flags) {
    return neko_gui_begin_titled(ctx, title, title, bounds, flags);
}
NEKO_GUI_API neko_gui_bool neko_gui_begin_titled(struct neko_gui_context *ctx, const char *name, const char *title, struct neko_gui_rect bounds, neko_gui_flags flags) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    neko_gui_hash name_hash;
    int name_len;
    int ret = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);
    NEKO_GUI_ASSERT(title);
    NEKO_GUI_ASSERT(ctx->style.font && ctx->style.font->width && "if this triggers you forgot to add a font");
    NEKO_GUI_ASSERT(!ctx->current && "if this triggers you missed a `neko_gui_end` call");
    if (!ctx || ctx->current || !title || !name) return 0;

    style = &ctx->style;
    name_len = (int)neko_gui_strlen(name);
    name_hash = neko_gui_murmur_hash(name, (int)name_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, name_hash, name);
    if (!win) {

        neko_gui_size name_length = (neko_gui_size)name_len;
        win = (struct neko_gui_window *)neko_gui_create_window(ctx);
        NEKO_GUI_ASSERT(win);
        if (!win) return 0;

        if (flags & NEKO_GUI_WINDOW_BACKGROUND)
            neko_gui_insert_window(ctx, win, NEKO_GUI_INSERT_FRONT);
        else
            neko_gui_insert_window(ctx, win, NEKO_GUI_INSERT_BACK);
        neko_gui_command_buffer_init(&win->buffer, &ctx->memory, NEKO_GUI_CLIPPING_ON);

        win->flags = flags;
        win->bounds = bounds;
        win->name = name_hash;
        name_length = NEKO_GUI_MIN(name_length, NEKO_GUI_WINDOW_MAX_NAME - 1);
        NEKO_GUI_MEMCPY(win->name_string, name, name_length);
        win->name_string[name_length] = 0;
        win->popup.win = 0;
        if (!ctx->active) ctx->active = win;
    } else {

        win->flags &= ~(neko_gui_flags)(NEKO_GUI_WINDOW_PRIVATE - 1);
        win->flags |= flags;
        if (!(win->flags & (NEKO_GUI_WINDOW_MOVABLE | NEKO_GUI_WINDOW_SCALABLE))) win->bounds = bounds;

        NEKO_GUI_ASSERT(win->seq != ctx->seq);
        win->seq = ctx->seq;
        if (!ctx->active && !(win->flags & NEKO_GUI_WINDOW_HIDDEN)) {
            ctx->active = win;
            ctx->end = win;
        }
    }
    if (win->flags & NEKO_GUI_WINDOW_HIDDEN) {
        ctx->current = win;
        win->layout = 0;
        return 0;
    } else
        neko_gui_start(ctx, win);

    if (!(win->flags & NEKO_GUI_WINDOW_HIDDEN) && !(win->flags & NEKO_GUI_WINDOW_NO_INPUT)) {
        int inpanel, ishovered;
        struct neko_gui_window *iter = win;
        float h = ctx->style.font->height + 2.0f * style->window.header.padding.y + (2.0f * style->window.header.label_padding.y);
        struct neko_gui_rect win_bounds = (!(win->flags & NEKO_GUI_WINDOW_MINIMIZED)) ? win->bounds : neko_gui_rect(win->bounds.x, win->bounds.y, win->bounds.w, h);

        inpanel = neko_gui_input_has_mouse_click_down_in_rect(&ctx->input, NEKO_GUI_BUTTON_LEFT, win_bounds, neko_gui_true);
        inpanel = inpanel && ctx->input.mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked;
        ishovered = neko_gui_input_is_mouse_hovering_rect(&ctx->input, win_bounds);
        if ((win != ctx->active) && ishovered && !ctx->input.mouse.buttons[NEKO_GUI_BUTTON_LEFT].down) {
            iter = win->next;
            while (iter) {
                struct neko_gui_rect iter_bounds = (!(iter->flags & NEKO_GUI_WINDOW_MINIMIZED)) ? iter->bounds : neko_gui_rect(iter->bounds.x, iter->bounds.y, iter->bounds.w, h);
                if (NEKO_GUI_INTERSECT(win_bounds.x, win_bounds.y, win_bounds.w, win_bounds.h, iter_bounds.x, iter_bounds.y, iter_bounds.w, iter_bounds.h) && (!(iter->flags & NEKO_GUI_WINDOW_HIDDEN)))
                    break;

                if (iter->popup.win && iter->popup.active && !(iter->flags & NEKO_GUI_WINDOW_HIDDEN) &&
                    NEKO_GUI_INTERSECT(win->bounds.x, win_bounds.y, win_bounds.w, win_bounds.h, iter->popup.win->bounds.x, iter->popup.win->bounds.y, iter->popup.win->bounds.w,
                                       iter->popup.win->bounds.h))
                    break;
                iter = iter->next;
            }
        }

        if (iter && inpanel && (win != ctx->end)) {
            iter = win->next;
            while (iter) {

                struct neko_gui_rect iter_bounds = (!(iter->flags & NEKO_GUI_WINDOW_MINIMIZED)) ? iter->bounds : neko_gui_rect(iter->bounds.x, iter->bounds.y, iter->bounds.w, h);
                if (NEKO_GUI_INBOX(ctx->input.mouse.pos.x, ctx->input.mouse.pos.y, iter_bounds.x, iter_bounds.y, iter_bounds.w, iter_bounds.h) && !(iter->flags & NEKO_GUI_WINDOW_HIDDEN)) break;
                if (iter->popup.win && iter->popup.active && !(iter->flags & NEKO_GUI_WINDOW_HIDDEN) &&
                    NEKO_GUI_INTERSECT(win_bounds.x, win_bounds.y, win_bounds.w, win_bounds.h, iter->popup.win->bounds.x, iter->popup.win->bounds.y, iter->popup.win->bounds.w,
                                       iter->popup.win->bounds.h))
                    break;
                iter = iter->next;
            }
        }
        if (iter && !(win->flags & NEKO_GUI_WINDOW_ROM) && (win->flags & NEKO_GUI_WINDOW_BACKGROUND)) {
            win->flags |= (neko_gui_flags)NEKO_GUI_WINDOW_ROM;
            iter->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
            ctx->active = iter;
            if (!(iter->flags & NEKO_GUI_WINDOW_BACKGROUND)) {

                neko_gui_remove_window(ctx, iter);
                neko_gui_insert_window(ctx, iter, NEKO_GUI_INSERT_BACK);
            }
        } else {
            if (!iter && ctx->end != win) {
                if (!(win->flags & NEKO_GUI_WINDOW_BACKGROUND)) {

                    neko_gui_remove_window(ctx, win);
                    neko_gui_insert_window(ctx, win, NEKO_GUI_INSERT_BACK);
                }
                win->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
                ctx->active = win;
            }
            if (ctx->end != win && !(win->flags & NEKO_GUI_WINDOW_BACKGROUND)) win->flags |= NEKO_GUI_WINDOW_ROM;
        }
    }
    win->layout = (struct neko_gui_panel *)neko_gui_create_panel(ctx);
    ctx->current = win;
    ret = neko_gui_panel_begin(ctx, title, NEKO_GUI_PANEL_WINDOW);
    win->layout->offset_x = &win->scrollbar.x;
    win->layout->offset_y = &win->scrollbar.y;
    return ret;
}
NEKO_GUI_API void neko_gui_end(struct neko_gui_context *ctx) {
    struct neko_gui_panel *layout;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current && "if this triggers you forgot to call `neko_gui_begin`");
    if (!ctx || !ctx->current) return;

    layout = ctx->current->layout;
    if (!layout || (layout->type == NEKO_GUI_PANEL_WINDOW && (ctx->current->flags & NEKO_GUI_WINDOW_HIDDEN))) {
        ctx->current = 0;
        return;
    }
    neko_gui_panel_end(ctx);
    neko_gui_free_panel(ctx, ctx->current->layout);
    ctx->current = 0;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_window_get_bounds(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_rect(0, 0, 0, 0);
    return ctx->current->bounds;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_position(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);
    return neko_gui_vec2(ctx->current->bounds.x, ctx->current->bounds.y);
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_size(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);
    return neko_gui_vec2(ctx->current->bounds.w, ctx->current->bounds.h);
}
NEKO_GUI_API float neko_gui_window_get_width(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return 0;
    return ctx->current->bounds.w;
}
NEKO_GUI_API float neko_gui_window_get_height(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return 0;
    return ctx->current->bounds.h;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_window_get_content_region(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_rect(0, 0, 0, 0);
    return ctx->current->layout->clip;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_min(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);
    return neko_gui_vec2(ctx->current->layout->clip.x, ctx->current->layout->clip.y);
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_max(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);
    return neko_gui_vec2(ctx->current->layout->clip.x + ctx->current->layout->clip.w, ctx->current->layout->clip.y + ctx->current->layout->clip.h);
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_window_get_content_region_size(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);
    return neko_gui_vec2(ctx->current->layout->clip.w, ctx->current->layout->clip.h);
}
NEKO_GUI_API struct neko_gui_command_buffer *neko_gui_window_get_canvas(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current) return 0;
    return &ctx->current->buffer;
}
NEKO_GUI_API struct neko_gui_panel *neko_gui_window_get_panel(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return 0;
    return ctx->current->layout;
}
NEKO_GUI_API void neko_gui_window_get_scroll(struct neko_gui_context *ctx, neko_gui_uint *offset_x, neko_gui_uint *offset_y) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;
    win = ctx->current;
    if (offset_x) *offset_x = win->scrollbar.x;
    if (offset_y) *offset_y = win->scrollbar.y;
}
NEKO_GUI_API neko_gui_bool neko_gui_window_has_focus(const struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current) return 0;
    return ctx->current == ctx->active;
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_hovered(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current || (ctx->current->flags & NEKO_GUI_WINDOW_HIDDEN))
        return 0;
    else {
        struct neko_gui_rect actual_bounds = ctx->current->bounds;
        if (ctx->begin->flags & NEKO_GUI_WINDOW_MINIMIZED) {
            actual_bounds.h = ctx->current->layout->header_height;
        }
        return neko_gui_input_is_mouse_hovering_rect(&ctx->input, actual_bounds);
    }
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_any_hovered(struct neko_gui_context *ctx) {
    struct neko_gui_window *iter;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    iter = ctx->begin;
    while (iter) {

        if (!(iter->flags & NEKO_GUI_WINDOW_HIDDEN)) {

            if (iter->popup.active && iter->popup.win && neko_gui_input_is_mouse_hovering_rect(&ctx->input, iter->popup.win->bounds)) return 1;

            if (iter->flags & NEKO_GUI_WINDOW_MINIMIZED) {
                struct neko_gui_rect header = iter->bounds;
                header.h = ctx->style.font->height + 2 * ctx->style.window.header.padding.y;
                if (neko_gui_input_is_mouse_hovering_rect(&ctx->input, header)) return 1;
            } else if (neko_gui_input_is_mouse_hovering_rect(&ctx->input, iter->bounds)) {
                return 1;
            }
        }
        iter = iter->next;
    }
    return 0;
}
NEKO_GUI_API neko_gui_bool neko_gui_item_is_any_active(struct neko_gui_context *ctx) {
    int any_hovered = neko_gui_window_is_any_hovered(ctx);
    int any_active = (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_MODIFIED);
    return any_hovered || any_active;
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_collapsed(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return 0;
    return win->flags & NEKO_GUI_WINDOW_MINIMIZED;
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_closed(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 1;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return 1;
    return (win->flags & NEKO_GUI_WINDOW_CLOSED);
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_hidden(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 1;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return 1;
    return (win->flags & NEKO_GUI_WINDOW_HIDDEN);
}
NEKO_GUI_API neko_gui_bool neko_gui_window_is_active(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return 0;
    return win == ctx->active;
}
NEKO_GUI_API struct neko_gui_window *neko_gui_window_find(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    return neko_gui_find_window(ctx, title_hash, name);
}
NEKO_GUI_API void neko_gui_window_close(struct neko_gui_context *ctx, const char *name) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    win = neko_gui_window_find(ctx, name);
    if (!win) return;
    NEKO_GUI_ASSERT(ctx->current != win && "You cannot close a currently active window");
    if (ctx->current == win) return;
    win->flags |= NEKO_GUI_WINDOW_HIDDEN;
    win->flags |= NEKO_GUI_WINDOW_CLOSED;
}
NEKO_GUI_API void neko_gui_window_set_bounds(struct neko_gui_context *ctx, const char *name, struct neko_gui_rect bounds) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    win = neko_gui_window_find(ctx, name);
    if (!win) return;
    NEKO_GUI_ASSERT(ctx->current != win && "You cannot update a currently in procecss window");
    win->bounds = bounds;
}
NEKO_GUI_API void neko_gui_window_set_position(struct neko_gui_context *ctx, const char *name, struct neko_gui_vec2 pos) {
    struct neko_gui_window *win = neko_gui_window_find(ctx, name);
    if (!win) return;
    win->bounds.x = pos.x;
    win->bounds.y = pos.y;
}
NEKO_GUI_API void neko_gui_window_set_size(struct neko_gui_context *ctx, const char *name, struct neko_gui_vec2 size) {
    struct neko_gui_window *win = neko_gui_window_find(ctx, name);
    if (!win) return;
    win->bounds.w = size.x;
    win->bounds.h = size.y;
}
NEKO_GUI_API void neko_gui_window_set_scroll(struct neko_gui_context *ctx, neko_gui_uint offset_x, neko_gui_uint offset_y) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;
    win = ctx->current;
    win->scrollbar.x = offset_x;
    win->scrollbar.y = offset_y;
}
NEKO_GUI_API void neko_gui_window_collapse(struct neko_gui_context *ctx, const char *name, enum neko_gui_collapse_states c) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return;
    if (c == NEKO_GUI_MINIMIZED)
        win->flags |= NEKO_GUI_WINDOW_MINIMIZED;
    else
        win->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_MINIMIZED;
}
NEKO_GUI_API void neko_gui_window_collapse_if(struct neko_gui_context *ctx, const char *name, enum neko_gui_collapse_states c, int cond) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx || !cond) return;
    neko_gui_window_collapse(ctx, name, c);
}
NEKO_GUI_API void neko_gui_window_show(struct neko_gui_context *ctx, const char *name, enum neko_gui_show_states s) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (!win) return;
    if (s == NEKO_GUI_HIDDEN) {
        win->flags |= NEKO_GUI_WINDOW_HIDDEN;
    } else
        win->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_HIDDEN;
}
NEKO_GUI_API void neko_gui_window_show_if(struct neko_gui_context *ctx, const char *name, enum neko_gui_show_states s, int cond) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx || !cond) return;
    neko_gui_window_show(ctx, name, s);
}

NEKO_GUI_API void neko_gui_window_set_focus(struct neko_gui_context *ctx, const char *name) {
    int title_len;
    neko_gui_hash title_hash;
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;

    title_len = (int)neko_gui_strlen(name);
    title_hash = neko_gui_murmur_hash(name, (int)title_len, NEKO_GUI_WINDOW_TITLE);
    win = neko_gui_find_window(ctx, title_hash, name);
    if (win && ctx->end != win) {
        neko_gui_remove_window(ctx, win);
        neko_gui_insert_window(ctx, win, NEKO_GUI_INSERT_BACK);
    }
    ctx->active = win;
}

NEKO_GUI_API neko_gui_bool neko_gui_popup_begin(struct neko_gui_context *ctx, enum neko_gui_popup_type type, const char *title, neko_gui_flags flags, struct neko_gui_rect rect) {
    struct neko_gui_window *popup;
    struct neko_gui_window *win;
    struct neko_gui_panel *panel;

    int title_len;
    neko_gui_hash title_hash;
    neko_gui_size allocated;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(title);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    panel = win->layout;
    NEKO_GUI_ASSERT(!(panel->type & NEKO_GUI_PANEL_SET_POPUP) && "popups are not allowed to have popups");
    (void)panel;
    title_len = (int)neko_gui_strlen(title);
    title_hash = neko_gui_murmur_hash(title, (int)title_len, NEKO_GUI_PANEL_POPUP);

    popup = win->popup.win;
    if (!popup) {
        popup = (struct neko_gui_window *)neko_gui_create_window(ctx);
        popup->parent = win;
        win->popup.win = popup;
        win->popup.active = 0;
        win->popup.type = NEKO_GUI_PANEL_POPUP;
    }

    if (win->popup.name != title_hash) {
        if (!win->popup.active) {
            neko_gui_zero(popup, sizeof(*popup));
            win->popup.name = title_hash;
            win->popup.active = 1;
            win->popup.type = NEKO_GUI_PANEL_POPUP;
        } else
            return 0;
    }

    ctx->current = popup;
    rect.x += win->layout->clip.x;
    rect.y += win->layout->clip.y;

    popup->parent = win;
    popup->bounds = rect;
    popup->seq = ctx->seq;
    popup->layout = (struct neko_gui_panel *)neko_gui_create_panel(ctx);
    popup->flags = flags;
    popup->flags |= NEKO_GUI_WINDOW_BORDER;
    if (type == NEKO_GUI_POPUP_DYNAMIC) popup->flags |= NEKO_GUI_WINDOW_DYNAMIC;

    popup->buffer = win->buffer;
    neko_gui_start_popup(ctx, win);
    allocated = ctx->memory.allocated;
    neko_gui_push_scissor(&popup->buffer, neko_gui_null_rect);

    if (neko_gui_panel_begin(ctx, title, NEKO_GUI_PANEL_POPUP)) {

        struct neko_gui_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NEKO_GUI_WINDOW_ROM;
            root->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.active = 1;
        popup->layout->offset_x = &popup->scrollbar.x;
        popup->layout->offset_y = &popup->scrollbar.y;
        popup->layout->parent = win->layout;
        return 1;
    } else {

        struct neko_gui_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NEKO_GUI_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.buf.active = 0;
        win->popup.active = 0;
        ctx->memory.allocated = allocated;
        ctx->current = win;
        neko_gui_free_panel(ctx, popup->layout);
        popup->layout = 0;
        return 0;
    }
}
NEKO_GUI_LIB neko_gui_bool neko_gui_nonblock_begin(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_rect body, struct neko_gui_rect header, enum neko_gui_panel_type panel_type) {
    struct neko_gui_window *popup;
    struct neko_gui_window *win;
    struct neko_gui_panel *panel;
    int is_active = neko_gui_true;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    panel = win->layout;
    NEKO_GUI_ASSERT(!(panel->type & NEKO_GUI_PANEL_SET_POPUP));
    (void)panel;
    popup = win->popup.win;
    if (!popup) {

        popup = (struct neko_gui_window *)neko_gui_create_window(ctx);
        popup->parent = win;
        win->popup.win = popup;
        win->popup.type = panel_type;
        neko_gui_command_buffer_init(&popup->buffer, &ctx->memory, NEKO_GUI_CLIPPING_ON);
    } else {

        int pressed, in_body, in_header;
#ifdef NEKO_GUI_BUTTON_TRIGGER_ON_RELEASE
        pressed = neko_gui_input_is_mouse_released(&ctx->input, NEKO_GUI_BUTTON_LEFT);
#else
        pressed = neko_gui_input_is_mouse_pressed(&ctx->input, NEKO_GUI_BUTTON_LEFT);
#endif
        in_body = neko_gui_input_is_mouse_hovering_rect(&ctx->input, body);
        in_header = neko_gui_input_is_mouse_hovering_rect(&ctx->input, header);
        if (pressed && (!in_body || in_header)) is_active = neko_gui_false;
    }
    win->popup.header = header;

    if (!is_active) {

        struct neko_gui_panel *root = win->layout;
        while (root) {
            root->flags |= NEKO_GUI_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        return is_active;
    }
    popup->bounds = body;
    popup->parent = win;
    popup->layout = (struct neko_gui_panel *)neko_gui_create_panel(ctx);
    popup->flags = flags;
    popup->flags |= NEKO_GUI_WINDOW_BORDER;
    popup->flags |= NEKO_GUI_WINDOW_DYNAMIC;
    popup->seq = ctx->seq;
    win->popup.active = 1;
    NEKO_GUI_ASSERT(popup->layout);

    neko_gui_start_popup(ctx, win);
    popup->buffer = win->buffer;
    neko_gui_push_scissor(&popup->buffer, neko_gui_null_rect);
    ctx->current = popup;

    neko_gui_panel_begin(ctx, 0, panel_type);
    win->buffer = popup->buffer;
    popup->layout->parent = win->layout;
    popup->layout->offset_x = &popup->scrollbar.x;
    popup->layout->offset_y = &popup->scrollbar.y;

    {
        struct neko_gui_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NEKO_GUI_WINDOW_ROM;
            root = root->parent;
        }
    }
    return is_active;
}
NEKO_GUI_API void neko_gui_popup_close(struct neko_gui_context *ctx) {
    struct neko_gui_window *popup;
    NEKO_GUI_ASSERT(ctx);
    if (!ctx || !ctx->current) return;

    popup = ctx->current;
    NEKO_GUI_ASSERT(popup->parent);
    NEKO_GUI_ASSERT(popup->layout->type & NEKO_GUI_PANEL_SET_POPUP);
    popup->flags |= NEKO_GUI_WINDOW_HIDDEN;
}
NEKO_GUI_API void neko_gui_popup_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_window *popup;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    popup = ctx->current;
    if (!popup->parent) return;
    win = popup->parent;
    if (popup->flags & NEKO_GUI_WINDOW_HIDDEN) {
        struct neko_gui_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NEKO_GUI_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.active = 0;
    }
    neko_gui_push_scissor(&popup->buffer, neko_gui_null_rect);
    neko_gui_end(ctx);

    win->buffer = popup->buffer;
    neko_gui_finish_popup(ctx, win);
    ctx->current = win;
    neko_gui_push_scissor(&win->buffer, win->layout->clip);
}
NEKO_GUI_API void neko_gui_popup_get_scroll(struct neko_gui_context *ctx, neko_gui_uint *offset_x, neko_gui_uint *offset_y) {
    struct neko_gui_window *popup;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    popup = ctx->current;
    if (offset_x) *offset_x = popup->scrollbar.x;
    if (offset_y) *offset_y = popup->scrollbar.y;
}
NEKO_GUI_API void neko_gui_popup_set_scroll(struct neko_gui_context *ctx, neko_gui_uint offset_x, neko_gui_uint offset_y) {
    struct neko_gui_window *popup;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    popup = ctx->current;
    popup->scrollbar.x = offset_x;
    popup->scrollbar.y = offset_y;
}

NEKO_GUI_API neko_gui_bool neko_gui_contextual_begin(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_vec2 size, struct neko_gui_rect trigger_bounds) {
    struct neko_gui_window *win;
    struct neko_gui_window *popup;
    struct neko_gui_rect body;

    NEKO_GUI_STORAGE const struct neko_gui_rect null_rect = {-1, -1, 0, 0};
    int is_clicked = 0;
    int is_open = 0;
    int ret = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    ++win->popup.con_count;
    if (ctx->current != ctx->active) return 0;

    popup = win->popup.win;
    is_open = (popup && win->popup.type == NEKO_GUI_PANEL_CONTEXTUAL);
    is_clicked = neko_gui_input_mouse_clicked(&ctx->input, NEKO_GUI_BUTTON_RIGHT, trigger_bounds);
    if (win->popup.active_con && win->popup.con_count != win->popup.active_con) return 0;
    if (!is_open && win->popup.active_con) win->popup.active_con = 0;
    if ((!is_open && !is_clicked)) return 0;

    win->popup.active_con = win->popup.con_count;
    if (is_clicked) {
        body.x = ctx->input.mouse.pos.x;
        body.y = ctx->input.mouse.pos.y;
    } else {
        body.x = popup->bounds.x;
        body.y = popup->bounds.y;
    }
    body.w = size.x;
    body.h = size.y;

    ret = neko_gui_nonblock_begin(ctx, flags | NEKO_GUI_WINDOW_NO_SCROLLBAR, body, null_rect, NEKO_GUI_PANEL_CONTEXTUAL);
    if (ret)
        win->popup.type = NEKO_GUI_PANEL_CONTEXTUAL;
    else {
        win->popup.active_con = 0;
        win->popup.type = NEKO_GUI_PANEL_NONE;
        if (win->popup.win) win->popup.win->flags = 0;
    }
    return ret;
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_flags alignment) {
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    state = neko_gui_widget_fitting(&bounds, ctx, style->contextual_button.padding);
    if (!state) return neko_gui_false;

    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text(&ctx->last_widget_state, &win->buffer, bounds, text, len, alignment, NEKO_GUI_BUTTON_DEFAULT, &style->contextual_button, in, style->font)) {
        neko_gui_contextual_close(ctx);
        return neko_gui_true;
    }
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_label(struct neko_gui_context *ctx, const char *label, neko_gui_flags align) {
    return neko_gui_contextual_item_text(ctx, label, neko_gui_strlen(label), align);
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *text, int len, neko_gui_flags align) {
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    state = neko_gui_widget_fitting(&bounds, ctx, style->contextual_button.padding);
    if (!state) return neko_gui_false;

    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text_image(&ctx->last_widget_state, &win->buffer, bounds, img, text, len, align, NEKO_GUI_BUTTON_DEFAULT, &style->contextual_button, style->font, in)) {
        neko_gui_contextual_close(ctx);
        return neko_gui_true;
    }
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *label, neko_gui_flags align) {
    return neko_gui_contextual_item_image_text(ctx, img, label, neko_gui_strlen(label), align);
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol, const char *text, int len, neko_gui_flags align) {
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    state = neko_gui_widget_fitting(&bounds, ctx, style->contextual_button.padding);
    if (!state) return neko_gui_false;

    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text_symbol(&ctx->last_widget_state, &win->buffer, bounds, symbol, text, len, align, NEKO_GUI_BUTTON_DEFAULT, &style->contextual_button, style->font, in)) {
        neko_gui_contextual_close(ctx);
        return neko_gui_true;
    }
    return neko_gui_false;
}
NEKO_GUI_API neko_gui_bool neko_gui_contextual_item_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol, const char *text, neko_gui_flags align) {
    return neko_gui_contextual_item_symbol_text(ctx, symbol, text, neko_gui_strlen(text), align);
}
NEKO_GUI_API void neko_gui_contextual_close(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;
    neko_gui_popup_close(ctx);
}
NEKO_GUI_API void neko_gui_contextual_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *popup;
    struct neko_gui_panel *panel;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;

    popup = ctx->current;
    panel = popup->layout;
    NEKO_GUI_ASSERT(popup->parent);
    NEKO_GUI_ASSERT(panel->type & NEKO_GUI_PANEL_SET_POPUP);
    if (panel->flags & NEKO_GUI_WINDOW_DYNAMIC) {

        struct neko_gui_rect body = {0, 0, 0, 0};
        if (panel->at_y < (panel->bounds.y + panel->bounds.h)) {
            struct neko_gui_vec2 padding = neko_gui_panel_get_padding(&ctx->style, panel->type);
            body = panel->bounds;
            body.y = (panel->at_y + panel->footer_height + panel->border + padding.y + panel->row.height);
            body.h = (panel->bounds.y + panel->bounds.h) - body.y;
        }
        {
            int pressed = neko_gui_input_is_mouse_pressed(&ctx->input, NEKO_GUI_BUTTON_LEFT);
            int in_body = neko_gui_input_is_mouse_hovering_rect(&ctx->input, body);
            if (pressed && in_body) popup->flags |= NEKO_GUI_WINDOW_HIDDEN;
        }
    }
    if (popup->flags & NEKO_GUI_WINDOW_HIDDEN) popup->seq = 0;
    neko_gui_popup_end(ctx);
    return;
}

NEKO_GUI_API void neko_gui_menubar_begin(struct neko_gui_context *ctx) {
    struct neko_gui_panel *layout;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    layout = ctx->current->layout;
    NEKO_GUI_ASSERT(layout->at_y == layout->bounds.y);

    if (layout->flags & NEKO_GUI_WINDOW_HIDDEN || layout->flags & NEKO_GUI_WINDOW_MINIMIZED) return;

    layout->menu.x = layout->at_x;
    layout->menu.y = layout->at_y + layout->row.height;
    layout->menu.w = layout->bounds.w;
    layout->menu.offset.x = *layout->offset_x;
    layout->menu.offset.y = *layout->offset_y;
    *layout->offset_y = 0;
}
NEKO_GUI_API void neko_gui_menubar_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    struct neko_gui_command_buffer *out;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    out = &win->buffer;
    layout = win->layout;
    if (layout->flags & NEKO_GUI_WINDOW_HIDDEN || layout->flags & NEKO_GUI_WINDOW_MINIMIZED) return;

    layout->menu.h = layout->at_y - layout->menu.y;
    layout->menu.h += layout->row.height + ctx->style.window.spacing.y;

    layout->bounds.y += layout->menu.h;
    layout->bounds.h -= layout->menu.h;

    *layout->offset_x = layout->menu.offset.x;
    *layout->offset_y = layout->menu.offset.y;
    layout->at_y = layout->bounds.y - layout->row.height;

    layout->clip.y = layout->bounds.y;
    layout->clip.h = layout->bounds.h;
    neko_gui_push_scissor(out, layout->clip);
}
NEKO_GUI_INTERN int neko_gui_menu_begin(struct neko_gui_context *ctx, struct neko_gui_window *win, const char *id, int is_clicked, struct neko_gui_rect header, struct neko_gui_vec2 size) {
    int is_open = 0;
    int is_active = 0;
    struct neko_gui_rect body;
    struct neko_gui_window *popup;
    neko_gui_hash hash = neko_gui_murmur_hash(id, (int)neko_gui_strlen(id), NEKO_GUI_PANEL_MENU);

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    body.x = header.x;
    body.w = size.x;
    body.y = header.y + header.h;
    body.h = size.y;

    popup = win->popup.win;
    is_open = popup ? neko_gui_true : neko_gui_false;
    is_active = (popup && (win->popup.name == hash) && win->popup.type == NEKO_GUI_PANEL_MENU);
    if ((is_clicked && is_open && !is_active) || (is_open && !is_active) || (!is_open && !is_active && !is_clicked)) return 0;
    if (!neko_gui_nonblock_begin(ctx, NEKO_GUI_WINDOW_NO_SCROLLBAR, body, header, NEKO_GUI_PANEL_MENU)) return 0;

    win->popup.type = NEKO_GUI_PANEL_MENU;
    win->popup.name = hash;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_text(struct neko_gui_context *ctx, const char *title, int len, neko_gui_flags align, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    neko_gui_flags state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    state = neko_gui_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || win->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text(&ctx->last_widget_state, &win->buffer, header, title, len, align, NEKO_GUI_BUTTON_DEFAULT, &ctx->style.menu_button, in, ctx->style.font)) is_clicked = neko_gui_true;
    return neko_gui_menu_begin(ctx, win, title, is_clicked, header, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_label(struct neko_gui_context *ctx, const char *text, neko_gui_flags align, struct neko_gui_vec2 size) {
    return neko_gui_menu_begin_text(ctx, text, neko_gui_strlen(text), align, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image(struct neko_gui_context *ctx, const char *id, struct neko_gui_image img, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_rect header;
    const struct neko_gui_input *in;
    int is_clicked = neko_gui_false;
    neko_gui_flags state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    state = neko_gui_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_image(&ctx->last_widget_state, &win->buffer, header, img, NEKO_GUI_BUTTON_DEFAULT, &ctx->style.menu_button, in)) is_clicked = neko_gui_true;
    return neko_gui_menu_begin(ctx, win, id, is_clicked, header, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol(struct neko_gui_context *ctx, const char *id, enum neko_gui_symbol_type sym, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    neko_gui_flags state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    state = neko_gui_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_symbol(&ctx->last_widget_state, &win->buffer, header, sym, NEKO_GUI_BUTTON_DEFAULT, &ctx->style.menu_button, in, ctx->style.font)) is_clicked = neko_gui_true;
    return neko_gui_menu_begin(ctx, win, id, is_clicked, header, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image_text(struct neko_gui_context *ctx, const char *title, int len, neko_gui_flags align, struct neko_gui_image img, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_rect header;
    const struct neko_gui_input *in;
    int is_clicked = neko_gui_false;
    neko_gui_flags state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    state = neko_gui_widget(&header, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text_image(&ctx->last_widget_state, &win->buffer, header, img, title, len, align, NEKO_GUI_BUTTON_DEFAULT, &ctx->style.menu_button, ctx->style.font, in))
        is_clicked = neko_gui_true;
    return neko_gui_menu_begin(ctx, win, title, is_clicked, header, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_image_label(struct neko_gui_context *ctx, const char *title, neko_gui_flags align, struct neko_gui_image img, struct neko_gui_vec2 size) {
    return neko_gui_menu_begin_image_text(ctx, title, neko_gui_strlen(title), align, img, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol_text(struct neko_gui_context *ctx, const char *title, int len, neko_gui_flags align, enum neko_gui_symbol_type sym, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_rect header;
    const struct neko_gui_input *in;
    int is_clicked = neko_gui_false;
    neko_gui_flags state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    state = neko_gui_widget(&header, ctx);
    if (!state) return 0;

    in = (state == NEKO_GUI_WIDGET_ROM || win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    if (neko_gui_do_button_text_symbol(&ctx->last_widget_state, &win->buffer, header, sym, title, len, align, NEKO_GUI_BUTTON_DEFAULT, &ctx->style.menu_button, ctx->style.font, in))
        is_clicked = neko_gui_true;
    return neko_gui_menu_begin(ctx, win, title, is_clicked, header, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_begin_symbol_label(struct neko_gui_context *ctx, const char *title, neko_gui_flags align, enum neko_gui_symbol_type sym, struct neko_gui_vec2 size) {
    return neko_gui_menu_begin_symbol_text(ctx, title, neko_gui_strlen(title), align, sym, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_text(struct neko_gui_context *ctx, const char *title, int len, neko_gui_flags align) { return neko_gui_contextual_item_text(ctx, title, len, align); }
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_label(struct neko_gui_context *ctx, const char *label, neko_gui_flags align) { return neko_gui_contextual_item_label(ctx, label, align); }
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *label, neko_gui_flags align) {
    return neko_gui_contextual_item_image_label(ctx, img, label, align);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *text, int len, neko_gui_flags align) {
    return neko_gui_contextual_item_image_text(ctx, img, text, len, align);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *text, int len, neko_gui_flags align) {
    return neko_gui_contextual_item_symbol_text(ctx, sym, text, len, align);
}
NEKO_GUI_API neko_gui_bool neko_gui_menu_item_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *label, neko_gui_flags align) {
    return neko_gui_contextual_item_symbol_label(ctx, sym, label, align);
}
NEKO_GUI_API void neko_gui_menu_close(struct neko_gui_context *ctx) { neko_gui_contextual_close(ctx); }
NEKO_GUI_API void neko_gui_menu_end(struct neko_gui_context *ctx) { neko_gui_contextual_end(ctx); }

NEKO_GUI_API void neko_gui_layout_set_min_row_height(struct neko_gui_context *ctx, float height) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    layout->row.min_height = height;
}
NEKO_GUI_API void neko_gui_layout_reset_min_row_height(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    layout->row.min_height = ctx->style.font->height;
    layout->row.min_height += ctx->style.text.padding.y * 2;
    layout->row.min_height += ctx->style.window.min_row_height_padding * 2;
}
NEKO_GUI_LIB float neko_gui_layout_row_calculate_usable_space(const struct neko_gui_style *style, enum neko_gui_panel_type type, float total_space, int columns) {
    float panel_spacing;
    float panel_space;

    struct neko_gui_vec2 spacing;

    NEKO_GUI_UNUSED(type);

    spacing = style->window.spacing;

    panel_spacing = (float)NEKO_GUI_MAX(columns - 1, 0) * spacing.x;
    panel_space = total_space - panel_spacing;
    return panel_space;
}
NEKO_GUI_LIB void neko_gui_panel_layout(const struct neko_gui_context *ctx, struct neko_gui_window *win, float height, int cols) {
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_command_buffer *out;

    struct neko_gui_vec2 item_spacing;
    struct neko_gui_color color;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    layout = win->layout;
    style = &ctx->style;
    out = &win->buffer;
    color = style->window.background;
    item_spacing = style->window.spacing;

    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_MINIMIZED));
    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_HIDDEN));
    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_CLOSED));

    layout->row.index = 0;
    layout->at_y += layout->row.height;
    layout->row.columns = cols;
    if (height == 0.0f)
        layout->row.height = NEKO_GUI_MAX(height, layout->row.min_height) + item_spacing.y;
    else
        layout->row.height = height + item_spacing.y;

    layout->row.item_offset = 0;
    if (layout->flags & NEKO_GUI_WINDOW_DYNAMIC) {

        struct neko_gui_rect background;
        background.x = win->bounds.x;
        background.w = win->bounds.w;
        background.y = layout->at_y - 1.0f;
        background.h = layout->row.height + 1.0f;
        neko_gui_fill_rect(out, background, 0, color);
    }
}
NEKO_GUI_LIB void neko_gui_row_layout(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float height, int cols, int width) {

    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    neko_gui_panel_layout(ctx, win, height, cols);
    if (fmt == NEKO_GUI_DYNAMIC)
        win->layout->row.type = NEKO_GUI_LAYOUT_DYNAMIC_FIXED;
    else
        win->layout->row.type = NEKO_GUI_LAYOUT_STATIC_FIXED;

    win->layout->row.ratio = 0;
    win->layout->row.filled = 0;
    win->layout->row.item_offset = 0;
    win->layout->row.item_width = (float)width;
}
NEKO_GUI_API float neko_gui_layout_ratio_from_pixel(struct neko_gui_context *ctx, float pixel_width) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(pixel_width);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;
    win = ctx->current;
    return NEKO_GUI_CLAMP(0.0f, pixel_width / win->bounds.x, 1.0f);
}
NEKO_GUI_API void neko_gui_layout_row_dynamic(struct neko_gui_context *ctx, float height, int cols) { neko_gui_row_layout(ctx, NEKO_GUI_DYNAMIC, height, cols, 0); }
NEKO_GUI_API void neko_gui_layout_row_static(struct neko_gui_context *ctx, float height, int item_width, int cols) { neko_gui_row_layout(ctx, NEKO_GUI_STATIC, height, cols, item_width); }
NEKO_GUI_API void neko_gui_layout_row_begin(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float row_height, int cols) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    neko_gui_panel_layout(ctx, win, row_height, cols);
    if (fmt == NEKO_GUI_DYNAMIC)
        layout->row.type = NEKO_GUI_LAYOUT_DYNAMIC_ROW;
    else
        layout->row.type = NEKO_GUI_LAYOUT_STATIC_ROW;

    layout->row.ratio = 0;
    layout->row.filled = 0;
    layout->row.item_width = 0;
    layout->row.item_offset = 0;
    layout->row.columns = cols;
}
NEKO_GUI_API void neko_gui_layout_row_push(struct neko_gui_context *ctx, float ratio_or_width) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_STATIC_ROW || layout->row.type == NEKO_GUI_LAYOUT_DYNAMIC_ROW);
    if (layout->row.type != NEKO_GUI_LAYOUT_STATIC_ROW && layout->row.type != NEKO_GUI_LAYOUT_DYNAMIC_ROW) return;

    if (layout->row.type == NEKO_GUI_LAYOUT_DYNAMIC_ROW) {
        float ratio = ratio_or_width;
        if ((ratio + layout->row.filled) > 1.0f) return;
        if (ratio > 0.0f)
            layout->row.item_width = NEKO_GUI_SATURATE(ratio);
        else
            layout->row.item_width = 1.0f - layout->row.filled;
    } else
        layout->row.item_width = ratio_or_width;
}
NEKO_GUI_API void neko_gui_layout_row_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_STATIC_ROW || layout->row.type == NEKO_GUI_LAYOUT_DYNAMIC_ROW);
    if (layout->row.type != NEKO_GUI_LAYOUT_STATIC_ROW && layout->row.type != NEKO_GUI_LAYOUT_DYNAMIC_ROW) return;
    layout->row.item_width = 0;
    layout->row.item_offset = 0;
}
NEKO_GUI_API void neko_gui_layout_row(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float height, int cols, const float *ratio) {
    int i;
    int n_undef = 0;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    neko_gui_panel_layout(ctx, win, height, cols);
    if (fmt == NEKO_GUI_DYNAMIC) {

        float r = 0;
        layout->row.ratio = ratio;
        for (i = 0; i < cols; ++i) {
            if (ratio[i] < 0.0f)
                n_undef++;
            else
                r += ratio[i];
        }
        r = NEKO_GUI_SATURATE(1.0f - r);
        layout->row.type = NEKO_GUI_LAYOUT_DYNAMIC;
        layout->row.item_width = (r > 0 && n_undef > 0) ? (r / (float)n_undef) : 0;
    } else {
        layout->row.ratio = ratio;
        layout->row.type = NEKO_GUI_LAYOUT_STATIC;
        layout->row.item_width = 0;
        layout->row.item_offset = 0;
    }
    layout->row.item_offset = 0;
    layout->row.filled = 0;
}
NEKO_GUI_API void neko_gui_layout_row_template_begin(struct neko_gui_context *ctx, float height) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    neko_gui_panel_layout(ctx, win, height, 1);
    layout->row.type = NEKO_GUI_LAYOUT_TEMPLATE;
    layout->row.columns = 0;
    layout->row.ratio = 0;
    layout->row.item_width = 0;
    layout->row.item_height = 0;
    layout->row.item_offset = 0;
    layout->row.filled = 0;
    layout->row.item.x = 0;
    layout->row.item.y = 0;
    layout->row.item.w = 0;
    layout->row.item.h = 0;
}
NEKO_GUI_API void neko_gui_layout_row_template_push_dynamic(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_TEMPLATE);
    NEKO_GUI_ASSERT(layout->row.columns < NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS);
    if (layout->row.type != NEKO_GUI_LAYOUT_TEMPLATE) return;
    if (layout->row.columns >= NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS) return;
    layout->row.templates[layout->row.columns++] = -1.0f;
}
NEKO_GUI_API void neko_gui_layout_row_template_push_variable(struct neko_gui_context *ctx, float min_width) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_TEMPLATE);
    NEKO_GUI_ASSERT(layout->row.columns < NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS);
    if (layout->row.type != NEKO_GUI_LAYOUT_TEMPLATE) return;
    if (layout->row.columns >= NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS) return;
    layout->row.templates[layout->row.columns++] = -min_width;
}
NEKO_GUI_API void neko_gui_layout_row_template_push_static(struct neko_gui_context *ctx, float width) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_TEMPLATE);
    NEKO_GUI_ASSERT(layout->row.columns < NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS);
    if (layout->row.type != NEKO_GUI_LAYOUT_TEMPLATE) return;
    if (layout->row.columns >= NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS) return;
    layout->row.templates[layout->row.columns++] = width;
}
NEKO_GUI_API void neko_gui_layout_row_template_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    int i = 0;
    int variable_count = 0;
    int min_variable_count = 0;
    float min_fixed_width = 0.0f;
    float total_fixed_width = 0.0f;
    float max_variable_width = 0.0f;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    NEKO_GUI_ASSERT(layout->row.type == NEKO_GUI_LAYOUT_TEMPLATE);
    if (layout->row.type != NEKO_GUI_LAYOUT_TEMPLATE) return;
    for (i = 0; i < layout->row.columns; ++i) {
        float width = layout->row.templates[i];
        if (width >= 0.0f) {
            total_fixed_width += width;
            min_fixed_width += width;
        } else if (width < -1.0f) {
            width = -width;
            total_fixed_width += width;
            max_variable_width = NEKO_GUI_MAX(max_variable_width, width);
            variable_count++;
        } else {
            min_variable_count++;
            variable_count++;
        }
    }
    if (variable_count) {
        float space = neko_gui_layout_row_calculate_usable_space(&ctx->style, layout->type, layout->bounds.w, layout->row.columns);
        float var_width = (NEKO_GUI_MAX(space - min_fixed_width, 0.0f)) / (float)variable_count;
        int enough_space = var_width >= max_variable_width;
        if (!enough_space) var_width = (NEKO_GUI_MAX(space - total_fixed_width, 0)) / (float)min_variable_count;
        for (i = 0; i < layout->row.columns; ++i) {
            float *width = &layout->row.templates[i];
            *width = (*width >= 0.0f) ? *width : (*width < -1.0f && !enough_space) ? -(*width) : var_width;
        }
    }
}
NEKO_GUI_API void neko_gui_layout_space_begin(struct neko_gui_context *ctx, enum neko_gui_layout_format fmt, float height, int widget_count) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    neko_gui_panel_layout(ctx, win, height, widget_count);
    if (fmt == NEKO_GUI_STATIC)
        layout->row.type = NEKO_GUI_LAYOUT_STATIC_FREE;
    else
        layout->row.type = NEKO_GUI_LAYOUT_DYNAMIC_FREE;

    layout->row.ratio = 0;
    layout->row.filled = 0;
    layout->row.item_width = 0;
    layout->row.item_offset = 0;
}
NEKO_GUI_API void neko_gui_layout_space_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    layout->row.item_width = 0;
    layout->row.item_height = 0;
    layout->row.item_offset = 0;
    neko_gui_zero(&layout->row.item, sizeof(layout->row.item));
}
NEKO_GUI_API void neko_gui_layout_space_push(struct neko_gui_context *ctx, struct neko_gui_rect rect) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    layout->row.item = rect;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_bounds(struct neko_gui_context *ctx) {
    struct neko_gui_rect ret;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x = layout->clip.x;
    ret.y = layout->clip.y;
    ret.w = layout->clip.w;
    ret.h = layout->row.height;
    return ret;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_layout_widget_bounds(struct neko_gui_context *ctx) {
    struct neko_gui_rect ret;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x = layout->at_x;
    ret.y = layout->at_y;
    ret.w = layout->bounds.w - NEKO_GUI_MAX(layout->at_x - layout->bounds.x, 0);
    ret.h = layout->row.height;
    return ret;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_layout_space_to_screen(struct neko_gui_context *ctx, struct neko_gui_vec2 ret) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x += layout->at_x - (float)*layout->offset_x;
    ret.y += layout->at_y - (float)*layout->offset_y;
    return ret;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_layout_space_to_local(struct neko_gui_context *ctx, struct neko_gui_vec2 ret) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x += -layout->at_x + (float)*layout->offset_x;
    ret.y += -layout->at_y + (float)*layout->offset_y;
    return ret;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_rect_to_screen(struct neko_gui_context *ctx, struct neko_gui_rect ret) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x += layout->at_x - (float)*layout->offset_x;
    ret.y += layout->at_y - (float)*layout->offset_y;
    return ret;
}
NEKO_GUI_API struct neko_gui_rect neko_gui_layout_space_rect_to_local(struct neko_gui_context *ctx, struct neko_gui_rect ret) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    win = ctx->current;
    layout = win->layout;

    ret.x += -layout->at_x + (float)*layout->offset_x;
    ret.y += -layout->at_y + (float)*layout->offset_y;
    return ret;
}
NEKO_GUI_LIB void neko_gui_panel_alloc_row(const struct neko_gui_context *ctx, struct neko_gui_window *win) {
    struct neko_gui_panel *layout = win->layout;
    struct neko_gui_vec2 spacing = ctx->style.window.spacing;
    const float row_height = layout->row.height - spacing.y;
    neko_gui_panel_layout(ctx, win, row_height, layout->row.columns);
}
NEKO_GUI_LIB void neko_gui_layout_widget_space(struct neko_gui_rect *bounds, const struct neko_gui_context *ctx, struct neko_gui_window *win, int modify) {
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;

    struct neko_gui_vec2 spacing;

    float item_offset = 0;
    float item_width = 0;
    float item_spacing = 0;
    float panel_space = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;
    NEKO_GUI_ASSERT(bounds);

    spacing = style->window.spacing;
    panel_space = neko_gui_layout_row_calculate_usable_space(&ctx->style, layout->type, layout->bounds.w, layout->row.columns);

#define NEKO_GUI_FRAC(x) (x - (float)(int)x)

    switch (layout->row.type) {
        case NEKO_GUI_LAYOUT_DYNAMIC_FIXED: {

            float w = NEKO_GUI_MAX(1.0f, panel_space) / (float)layout->row.columns;
            item_offset = (float)layout->row.index * w;
            item_width = w + NEKO_GUI_FRAC(item_offset);
            item_spacing = (float)layout->row.index * spacing.x;
        } break;
        case NEKO_GUI_LAYOUT_DYNAMIC_ROW: {

            float w = layout->row.item_width * panel_space;
            item_offset = layout->row.item_offset;
            item_width = w + NEKO_GUI_FRAC(item_offset);
            item_spacing = 0;

            if (modify) {
                layout->row.item_offset += w + spacing.x;
                layout->row.filled += layout->row.item_width;
                layout->row.index = 0;
            }
        } break;
        case NEKO_GUI_LAYOUT_DYNAMIC_FREE: {

            bounds->x = layout->at_x + (layout->bounds.w * layout->row.item.x);
            bounds->x -= (float)*layout->offset_x;
            bounds->y = layout->at_y + (layout->row.height * layout->row.item.y);
            bounds->y -= (float)*layout->offset_y;
            bounds->w = layout->bounds.w * layout->row.item.w + NEKO_GUI_FRAC(bounds->x);
            bounds->h = layout->row.height * layout->row.item.h + NEKO_GUI_FRAC(bounds->y);
            return;
        }
        case NEKO_GUI_LAYOUT_DYNAMIC: {

            float ratio, w;
            NEKO_GUI_ASSERT(layout->row.ratio);
            ratio = (layout->row.ratio[layout->row.index] < 0) ? layout->row.item_width : layout->row.ratio[layout->row.index];

            w = (ratio * panel_space);
            item_spacing = (float)layout->row.index * spacing.x;
            item_offset = layout->row.item_offset;
            item_width = w + NEKO_GUI_FRAC(item_offset);

            if (modify) {
                layout->row.item_offset += w;
                layout->row.filled += ratio;
            }
        } break;
        case NEKO_GUI_LAYOUT_STATIC_FIXED: {

            item_width = layout->row.item_width;
            item_offset = (float)layout->row.index * item_width;
            item_spacing = (float)layout->row.index * spacing.x;
        } break;
        case NEKO_GUI_LAYOUT_STATIC_ROW: {

            item_width = layout->row.item_width;
            item_offset = layout->row.item_offset;
            item_spacing = (float)layout->row.index * spacing.x;
            if (modify) layout->row.item_offset += item_width;
        } break;
        case NEKO_GUI_LAYOUT_STATIC_FREE: {

            bounds->x = layout->at_x + layout->row.item.x;
            bounds->w = layout->row.item.w;
            if (((bounds->x + bounds->w) > layout->max_x) && modify) layout->max_x = (bounds->x + bounds->w);
            bounds->x -= (float)*layout->offset_x;
            bounds->y = layout->at_y + layout->row.item.y;
            bounds->y -= (float)*layout->offset_y;
            bounds->h = layout->row.item.h;
            return;
        }
        case NEKO_GUI_LAYOUT_STATIC: {

            item_spacing = (float)layout->row.index * spacing.x;
            item_width = layout->row.ratio[layout->row.index];
            item_offset = layout->row.item_offset;
            if (modify) layout->row.item_offset += item_width;
        } break;
        case NEKO_GUI_LAYOUT_TEMPLATE: {

            float w;
            NEKO_GUI_ASSERT(layout->row.index < layout->row.columns);
            NEKO_GUI_ASSERT(layout->row.index < NEKO_GUI_MAX_LAYOUT_ROW_TEMPLATE_COLUMNS);
            w = layout->row.templates[layout->row.index];
            item_offset = layout->row.item_offset;
            item_width = w + NEKO_GUI_FRAC(item_offset);
            item_spacing = (float)layout->row.index * spacing.x;
            if (modify) layout->row.item_offset += w;
        } break;
#undef NEKO_GUI_FRAC
        default:
            NEKO_GUI_ASSERT(0);
            break;
    };

    bounds->w = item_width;
    bounds->h = layout->row.height - spacing.y;
    bounds->y = layout->at_y - (float)*layout->offset_y;
    bounds->x = layout->at_x + item_offset + item_spacing;
    if (((bounds->x + bounds->w) > layout->max_x) && modify) layout->max_x = bounds->x + bounds->w;
    bounds->x -= (float)*layout->offset_x;
}
NEKO_GUI_LIB void neko_gui_panel_alloc_space(struct neko_gui_rect *bounds, const struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    if (layout->row.index >= layout->row.columns) neko_gui_panel_alloc_row(ctx, win);

    neko_gui_layout_widget_space(bounds, ctx, win, neko_gui_true);
    layout->row.index++;
}
NEKO_GUI_LIB void neko_gui_layout_peek(struct neko_gui_rect *bounds, struct neko_gui_context *ctx) {
    float y;
    int index;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) {
        *bounds = neko_gui_rect(0, 0, 0, 0);
        return;
    }

    win = ctx->current;
    layout = win->layout;
    y = layout->at_y;
    index = layout->row.index;
    if (layout->row.index >= layout->row.columns) {
        layout->at_y += layout->row.height;
        layout->row.index = 0;
    }
    neko_gui_layout_widget_space(bounds, ctx, win, neko_gui_false);
    if (!layout->row.index) {
        bounds->x -= layout->row.item_offset;
    }
    layout->at_y = y;
    layout->row.index = index;
}
NEKO_GUI_API void neko_gui_spacer(struct neko_gui_context *ctx) {
    struct neko_gui_rect dummy_rect = {0, 0, 0, 0};
    neko_gui_panel_alloc_space(&dummy_rect, ctx);
}

NEKO_GUI_INTERN int neko_gui_tree_state_base(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image *img, const char *title, enum neko_gui_collapse_states *state) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_command_buffer *out;
    const struct neko_gui_input *in;
    const struct neko_gui_style_button *button;
    enum neko_gui_symbol_type symbol;
    float row_height;

    struct neko_gui_vec2 item_spacing;
    struct neko_gui_rect header = {0, 0, 0, 0};
    struct neko_gui_rect sym = {0, 0, 0, 0};
    struct neko_gui_text text;

    neko_gui_flags ws = 0;
    enum neko_gui_widget_layout_states widget_state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;
    out = &win->buffer;
    style = &ctx->style;
    item_spacing = style->window.spacing;

    row_height = style->font->height + 2 * style->tab.padding.y;
    neko_gui_layout_set_min_row_height(ctx, row_height);
    neko_gui_layout_row_dynamic(ctx, row_height, 1);
    neko_gui_layout_reset_min_row_height(ctx);

    widget_state = neko_gui_widget(&header, ctx);
    if (type == NEKO_GUI_TREE_TAB) {
        const struct neko_gui_style_item *background = &style->tab.background;

        switch (background->type) {
            case NEKO_GUI_STYLE_ITEM_IMAGE:
                neko_gui_draw_image(out, header, &background->data.image, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
                neko_gui_draw_nine_slice(out, header, &background->data.slice, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_COLOR:
                neko_gui_fill_rect(out, header, 0, style->tab.border_color);
                neko_gui_fill_rect(out, neko_gui_shrineko_gui_rect(header, style->tab.border), style->tab.rounding, background->data.color);
                break;
        }
    } else
        text.background = style->window.background;

    in = (!(layout->flags & NEKO_GUI_WINDOW_ROM)) ? &ctx->input : 0;
    in = (in && widget_state == NEKO_GUI_WIDGET_VALID) ? &ctx->input : 0;
    if (neko_gui_button_behavior(&ws, header, in, NEKO_GUI_BUTTON_DEFAULT)) *state = (*state == NEKO_GUI_MAXIMIZED) ? NEKO_GUI_MINIMIZED : NEKO_GUI_MAXIMIZED;

    if (*state == NEKO_GUI_MAXIMIZED) {
        symbol = style->tab.sym_maximize;
        if (type == NEKO_GUI_TREE_TAB)
            button = &style->tab.tab_maximize_button;
        else
            button = &style->tab.node_maximize_button;
    } else {
        symbol = style->tab.sym_minimize;
        if (type == NEKO_GUI_TREE_TAB)
            button = &style->tab.tab_minimize_button;
        else
            button = &style->tab.node_minimize_button;
    }

    {
        sym.w = sym.h = style->font->height;
        sym.y = header.y + style->tab.padding.y;
        sym.x = header.x + style->tab.padding.x;
        neko_gui_do_button_symbol(&ws, &win->buffer, sym, symbol, NEKO_GUI_BUTTON_DEFAULT, button, 0, style->font);

        if (img) {

            sym.x = sym.x + sym.w + 4 * item_spacing.x;
            neko_gui_draw_image(&win->buffer, sym, img, neko_gui_white);
            sym.w = style->font->height + style->tab.spacing.x;
        }
    }

    {
        struct neko_gui_rect label;
        header.w = NEKO_GUI_MAX(header.w, sym.w + item_spacing.x);
        label.x = sym.x + sym.w + item_spacing.x;
        label.y = sym.y;
        label.w = header.w - (sym.w + item_spacing.y + style->tab.indent);
        label.h = style->font->height;
        text.text = style->tab.text;
        text.padding = neko_gui_vec2(0, 0);
        neko_gui_widget_text(out, label, title, neko_gui_strlen(title), &text, NEKO_GUI_TEXT_LEFT, style->font);
    }

    if (*state == NEKO_GUI_MAXIMIZED) {
        layout->at_x = header.x + (float)*layout->offset_x + style->tab.indent;
        layout->bounds.w = NEKO_GUI_MAX(layout->bounds.w, style->tab.indent);
        layout->bounds.w -= (style->tab.indent + style->window.padding.x);
        layout->row.tree_depth++;
        return neko_gui_true;
    } else
        return neko_gui_false;
}
NEKO_GUI_INTERN int neko_gui_tree_base(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image *img, const char *title, enum neko_gui_collapse_states initial_state,
                                       const char *hash, int len, int line) {
    struct neko_gui_window *win = ctx->current;
    int title_len = 0;
    neko_gui_hash tree_hash = 0;
    neko_gui_uint *state = 0;

    if (!hash) {
        title_len = (int)neko_gui_strlen(title);
        tree_hash = neko_gui_murmur_hash(title, (int)title_len, (neko_gui_hash)line);
    } else
        tree_hash = neko_gui_murmur_hash(hash, len, (neko_gui_hash)line);
    state = neko_gui_find_value(win, tree_hash);
    if (!state) {
        state = neko_gui_add_value(ctx, win, tree_hash, 0);
        *state = initial_state;
    }
    return neko_gui_tree_state_base(ctx, type, img, title, (enum neko_gui_collapse_states *)state);
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_state_push(struct neko_gui_context *ctx, enum neko_gui_tree_type type, const char *title, enum neko_gui_collapse_states *state) {
    return neko_gui_tree_state_base(ctx, type, 0, title, state);
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_state_image_push(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image img, const char *title,
                                                          enum neko_gui_collapse_states *state) {
    return neko_gui_tree_state_base(ctx, type, &img, title, state);
}
NEKO_GUI_API void neko_gui_tree_state_pop(struct neko_gui_context *ctx) {
    struct neko_gui_window *win = 0;
    struct neko_gui_panel *layout = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    layout->at_x -= ctx->style.tab.indent + (float)*layout->offset_x;
    layout->bounds.w += ctx->style.tab.indent + ctx->style.window.padding.x;
    NEKO_GUI_ASSERT(layout->row.tree_depth);
    layout->row.tree_depth--;
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_push_hashed(struct neko_gui_context *ctx, enum neko_gui_tree_type type, const char *title, enum neko_gui_collapse_states initial_state, const char *hash,
                                                     int len, int line) {
    return neko_gui_tree_base(ctx, type, 0, title, initial_state, hash, len, line);
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_image_push_hashed(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image img, const char *title,
                                                           enum neko_gui_collapse_states initial_state, const char *hash, int len, int seed) {
    return neko_gui_tree_base(ctx, type, &img, title, initial_state, hash, len, seed);
}
NEKO_GUI_API void neko_gui_tree_pop(struct neko_gui_context *ctx) { neko_gui_tree_state_pop(ctx); }
NEKO_GUI_INTERN int neko_gui_tree_element_image_push_hashed_base(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image *img, const char *title, int title_len,
                                                                 enum neko_gui_collapse_states *state, neko_gui_bool *selected) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_command_buffer *out;
    const struct neko_gui_input *in;
    const struct neko_gui_style_button *button;
    enum neko_gui_symbol_type symbol;
    float row_height;
    struct neko_gui_vec2 padding;

    int text_len;
    float text_width;

    struct neko_gui_vec2 item_spacing;
    struct neko_gui_rect header = {0, 0, 0, 0};
    struct neko_gui_rect sym = {0, 0, 0, 0};

    neko_gui_flags ws = 0;
    enum neko_gui_widget_layout_states widget_state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;
    out = &win->buffer;
    style = &ctx->style;
    item_spacing = style->window.spacing;
    padding = style->selectable.padding;

    row_height = style->font->height + 2 * style->tab.padding.y;
    neko_gui_layout_set_min_row_height(ctx, row_height);
    neko_gui_layout_row_dynamic(ctx, row_height, 1);
    neko_gui_layout_reset_min_row_height(ctx);

    widget_state = neko_gui_widget(&header, ctx);
    if (type == NEKO_GUI_TREE_TAB) {
        const struct neko_gui_style_item *background = &style->tab.background;

        switch (background->type) {
            case NEKO_GUI_STYLE_ITEM_IMAGE:
                neko_gui_draw_image(out, header, &background->data.image, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
                neko_gui_draw_nine_slice(out, header, &background->data.slice, neko_gui_white);
                break;
            case NEKO_GUI_STYLE_ITEM_COLOR:
                neko_gui_fill_rect(out, header, 0, style->tab.border_color);
                neko_gui_fill_rect(out, neko_gui_shrineko_gui_rect(header, style->tab.border), style->tab.rounding, background->data.color);
                break;
        }
    }

    in = (!(layout->flags & NEKO_GUI_WINDOW_ROM)) ? &ctx->input : 0;
    in = (in && widget_state == NEKO_GUI_WIDGET_VALID) ? &ctx->input : 0;

    if (*state == NEKO_GUI_MAXIMIZED) {
        symbol = style->tab.sym_maximize;
        if (type == NEKO_GUI_TREE_TAB)
            button = &style->tab.tab_maximize_button;
        else
            button = &style->tab.node_maximize_button;
    } else {
        symbol = style->tab.sym_minimize;
        if (type == NEKO_GUI_TREE_TAB)
            button = &style->tab.tab_minimize_button;
        else
            button = &style->tab.node_minimize_button;
    }
    {
        sym.w = sym.h = style->font->height;
        sym.y = header.y + style->tab.padding.y;
        sym.x = header.x + style->tab.padding.x;
        if (neko_gui_do_button_symbol(&ws, &win->buffer, sym, symbol, NEKO_GUI_BUTTON_DEFAULT, button, in, style->font))
            *state = (*state == NEKO_GUI_MAXIMIZED) ? NEKO_GUI_MINIMIZED : NEKO_GUI_MAXIMIZED;
    }

    {
        neko_gui_flags dummy = 0;
        struct neko_gui_rect label;

        text_len = neko_gui_strlen(title);
        text_width = style->font->width(style->font->userdata, style->font->height, title, text_len);
        text_width += (4 * padding.x);

        header.w = NEKO_GUI_MAX(header.w, sym.w + item_spacing.x);
        label.x = sym.x + sym.w + item_spacing.x;
        label.y = sym.y;
        label.w = NEKO_GUI_MIN(header.w - (sym.w + item_spacing.y + style->tab.indent), text_width);
        label.h = style->font->height;

        if (img) {
            neko_gui_do_selectable_image(&dummy, &win->buffer, label, title, title_len, NEKO_GUI_TEXT_LEFT, selected, img, &style->selectable, in, style->font);
        } else
            neko_gui_do_selectable(&dummy, &win->buffer, label, title, title_len, NEKO_GUI_TEXT_LEFT, selected, &style->selectable, in, style->font);
    }

    if (*state == NEKO_GUI_MAXIMIZED) {
        layout->at_x = header.x + (float)*layout->offset_x + style->tab.indent;
        layout->bounds.w = NEKO_GUI_MAX(layout->bounds.w, style->tab.indent);
        layout->bounds.w -= (style->tab.indent + style->window.padding.x);
        layout->row.tree_depth++;
        return neko_gui_true;
    } else
        return neko_gui_false;
}
NEKO_GUI_INTERN int neko_gui_tree_element_base(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image *img, const char *title, enum neko_gui_collapse_states initial_state,
                                               neko_gui_bool *selected, const char *hash, int len, int line) {
    struct neko_gui_window *win = ctx->current;
    int title_len = 0;
    neko_gui_hash tree_hash = 0;
    neko_gui_uint *state = 0;

    if (!hash) {
        title_len = (int)neko_gui_strlen(title);
        tree_hash = neko_gui_murmur_hash(title, (int)title_len, (neko_gui_hash)line);
    } else
        tree_hash = neko_gui_murmur_hash(hash, len, (neko_gui_hash)line);
    state = neko_gui_find_value(win, tree_hash);
    if (!state) {
        state = neko_gui_add_value(ctx, win, tree_hash, 0);
        *state = initial_state;
    }
    return neko_gui_tree_element_image_push_hashed_base(ctx, type, img, title, neko_gui_strlen(title), (enum neko_gui_collapse_states *)state, selected);
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_element_push_hashed(struct neko_gui_context *ctx, enum neko_gui_tree_type type, const char *title, enum neko_gui_collapse_states initial_state,
                                                             neko_gui_bool *selected, const char *hash, int len, int seed) {
    return neko_gui_tree_element_base(ctx, type, 0, title, initial_state, selected, hash, len, seed);
}
NEKO_GUI_API neko_gui_bool neko_gui_tree_element_image_push_hashed(struct neko_gui_context *ctx, enum neko_gui_tree_type type, struct neko_gui_image img, const char *title,
                                                                   enum neko_gui_collapse_states initial_state, neko_gui_bool *selected, const char *hash, int len, int seed) {
    return neko_gui_tree_element_base(ctx, type, &img, title, initial_state, selected, hash, len, seed);
}
NEKO_GUI_API void neko_gui_tree_element_pop(struct neko_gui_context *ctx) { neko_gui_tree_state_pop(ctx); }

NEKO_GUI_API neko_gui_bool neko_gui_group_scrolled_offset_begin(struct neko_gui_context *ctx, neko_gui_uint *x_offset, neko_gui_uint *y_offset, const char *title, neko_gui_flags flags) {
    struct neko_gui_rect bounds;
    struct neko_gui_window panel;
    struct neko_gui_window *win;

    win = ctx->current;
    neko_gui_panel_alloc_space(&bounds, ctx);
    {
        const struct neko_gui_rect *c = &win->layout->clip;
        if (!NEKO_GUI_INTERSECT(c->x, c->y, c->w, c->h, bounds.x, bounds.y, bounds.w, bounds.h) && !(flags & NEKO_GUI_WINDOW_MOVABLE)) {
            return 0;
        }
    }
    if (win->flags & NEKO_GUI_WINDOW_ROM) flags |= NEKO_GUI_WINDOW_ROM;

    neko_gui_zero(&panel, sizeof(panel));
    panel.bounds = bounds;
    panel.flags = flags;
    panel.scrollbar.x = *x_offset;
    panel.scrollbar.y = *y_offset;
    panel.buffer = win->buffer;
    panel.layout = (struct neko_gui_panel *)neko_gui_create_panel(ctx);
    ctx->current = &panel;
    neko_gui_panel_begin(ctx, (flags & NEKO_GUI_WINDOW_TITLE) ? title : 0, NEKO_GUI_PANEL_GROUP);

    win->buffer = panel.buffer;
    win->buffer.clip = panel.layout->clip;
    panel.layout->offset_x = x_offset;
    panel.layout->offset_y = y_offset;
    panel.layout->parent = win->layout;
    win->layout = panel.layout;

    ctx->current = win;
    if ((panel.layout->flags & NEKO_GUI_WINDOW_CLOSED) || (panel.layout->flags & NEKO_GUI_WINDOW_MINIMIZED)) {
        neko_gui_flags f = panel.layout->flags;
        neko_gui_group_scrolled_end(ctx);
        if (f & NEKO_GUI_WINDOW_CLOSED) return NEKO_GUI_WINDOW_CLOSED;
        if (f & NEKO_GUI_WINDOW_MINIMIZED) return NEKO_GUI_WINDOW_MINIMIZED;
    }
    return 1;
}
NEKO_GUI_API void neko_gui_group_scrolled_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_panel *parent;
    struct neko_gui_panel *g;

    struct neko_gui_rect clip;
    struct neko_gui_window pan;
    struct neko_gui_vec2 panel_padding;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;

    NEKO_GUI_ASSERT(ctx->current);
    win = ctx->current;
    NEKO_GUI_ASSERT(win->layout);
    g = win->layout;
    NEKO_GUI_ASSERT(g->parent);
    parent = g->parent;

    neko_gui_zero_struct(pan);
    panel_padding = neko_gui_panel_get_padding(&ctx->style, NEKO_GUI_PANEL_GROUP);
    pan.bounds.y = g->bounds.y - (g->header_height + g->menu.h);
    pan.bounds.x = g->bounds.x - panel_padding.x;
    pan.bounds.w = g->bounds.w + 2 * panel_padding.x;
    pan.bounds.h = g->bounds.h + g->header_height + g->menu.h;
    if (g->flags & NEKO_GUI_WINDOW_BORDER) {
        pan.bounds.x -= g->border;
        pan.bounds.y -= g->border;
        pan.bounds.w += 2 * g->border;
        pan.bounds.h += 2 * g->border;
    }
    if (!(g->flags & NEKO_GUI_WINDOW_NO_SCROLLBAR)) {
        pan.bounds.w += ctx->style.window.scrollbar_size.x;
        pan.bounds.h += ctx->style.window.scrollbar_size.y;
    }
    pan.scrollbar.x = *g->offset_x;
    pan.scrollbar.y = *g->offset_y;
    pan.flags = g->flags;
    pan.buffer = win->buffer;
    pan.layout = g;
    pan.parent = win;
    ctx->current = &pan;

    neko_gui_unify(&clip, &parent->clip, pan.bounds.x, pan.bounds.y, pan.bounds.x + pan.bounds.w, pan.bounds.y + pan.bounds.h + panel_padding.x);
    neko_gui_push_scissor(&pan.buffer, clip);
    neko_gui_end(ctx);

    win->buffer = pan.buffer;
    neko_gui_push_scissor(&win->buffer, parent->clip);
    ctx->current = win;
    win->layout = parent;
    g->bounds = pan.bounds;
    return;
}
NEKO_GUI_API neko_gui_bool neko_gui_group_scrolled_begin(struct neko_gui_context *ctx, struct neko_gui_scroll *scroll, const char *title, neko_gui_flags flags) {
    return neko_gui_group_scrolled_offset_begin(ctx, &scroll->x, &scroll->y, title, flags);
}
NEKO_GUI_API neko_gui_bool neko_gui_group_begin_titled(struct neko_gui_context *ctx, const char *id, const char *title, neko_gui_flags flags) {
    int id_len;
    neko_gui_hash id_hash;
    struct neko_gui_window *win;
    neko_gui_uint *x_offset;
    neko_gui_uint *y_offset;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(id);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !id) return 0;

    win = ctx->current;
    id_len = (int)neko_gui_strlen(id);
    id_hash = neko_gui_murmur_hash(id, (int)id_len, NEKO_GUI_PANEL_GROUP);
    x_offset = neko_gui_find_value(win, id_hash);
    if (!x_offset) {
        x_offset = neko_gui_add_value(ctx, win, id_hash, 0);
        y_offset = neko_gui_add_value(ctx, win, id_hash + 1, 0);

        NEKO_GUI_ASSERT(x_offset);
        NEKO_GUI_ASSERT(y_offset);
        if (!x_offset || !y_offset) return 0;
        *x_offset = *y_offset = 0;
    } else
        y_offset = neko_gui_find_value(win, id_hash + 1);
    return neko_gui_group_scrolled_offset_begin(ctx, x_offset, y_offset, title, flags);
}
NEKO_GUI_API neko_gui_bool neko_gui_group_begin(struct neko_gui_context *ctx, const char *title, neko_gui_flags flags) { return neko_gui_group_begin_titled(ctx, title, title, flags); }
NEKO_GUI_API void neko_gui_group_end(struct neko_gui_context *ctx) { neko_gui_group_scrolled_end(ctx); }
NEKO_GUI_API void neko_gui_group_get_scroll(struct neko_gui_context *ctx, const char *id, neko_gui_uint *x_offset, neko_gui_uint *y_offset) {
    int id_len;
    neko_gui_hash id_hash;
    struct neko_gui_window *win;
    neko_gui_uint *x_offset_ptr;
    neko_gui_uint *y_offset_ptr;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(id);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !id) return;

    win = ctx->current;
    id_len = (int)neko_gui_strlen(id);
    id_hash = neko_gui_murmur_hash(id, (int)id_len, NEKO_GUI_PANEL_GROUP);
    x_offset_ptr = neko_gui_find_value(win, id_hash);
    if (!x_offset_ptr) {
        x_offset_ptr = neko_gui_add_value(ctx, win, id_hash, 0);
        y_offset_ptr = neko_gui_add_value(ctx, win, id_hash + 1, 0);

        NEKO_GUI_ASSERT(x_offset_ptr);
        NEKO_GUI_ASSERT(y_offset_ptr);
        if (!x_offset_ptr || !y_offset_ptr) return;
        *x_offset_ptr = *y_offset_ptr = 0;
    } else
        y_offset_ptr = neko_gui_find_value(win, id_hash + 1);
    if (x_offset) *x_offset = *x_offset_ptr;
    if (y_offset) *y_offset = *y_offset_ptr;
}
NEKO_GUI_API void neko_gui_group_set_scroll(struct neko_gui_context *ctx, const char *id, neko_gui_uint x_offset, neko_gui_uint y_offset) {
    int id_len;
    neko_gui_hash id_hash;
    struct neko_gui_window *win;
    neko_gui_uint *x_offset_ptr;
    neko_gui_uint *y_offset_ptr;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(id);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !id) return;

    win = ctx->current;
    id_len = (int)neko_gui_strlen(id);
    id_hash = neko_gui_murmur_hash(id, (int)id_len, NEKO_GUI_PANEL_GROUP);
    x_offset_ptr = neko_gui_find_value(win, id_hash);
    if (!x_offset_ptr) {
        x_offset_ptr = neko_gui_add_value(ctx, win, id_hash, 0);
        y_offset_ptr = neko_gui_add_value(ctx, win, id_hash + 1, 0);

        NEKO_GUI_ASSERT(x_offset_ptr);
        NEKO_GUI_ASSERT(y_offset_ptr);
        if (!x_offset_ptr || !y_offset_ptr) return;
        *x_offset_ptr = *y_offset_ptr = 0;
    } else
        y_offset_ptr = neko_gui_find_value(win, id_hash + 1);
    *x_offset_ptr = x_offset;
    *y_offset_ptr = y_offset;
}

NEKO_GUI_API neko_gui_bool neko_gui_list_view_begin(struct neko_gui_context *ctx, struct neko_gui_list_view *view, const char *title, neko_gui_flags flags, int row_height, int row_count) {
    int title_len;
    neko_gui_hash title_hash;
    neko_gui_uint *x_offset;
    neko_gui_uint *y_offset;

    int result;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_vec2 item_spacing;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(view);
    NEKO_GUI_ASSERT(title);
    if (!ctx || !view || !title) return 0;

    win = ctx->current;
    style = &ctx->style;
    item_spacing = style->window.spacing;
    row_height += NEKO_GUI_MAX(0, (int)item_spacing.y);

    title_len = (int)neko_gui_strlen(title);
    title_hash = neko_gui_murmur_hash(title, (int)title_len, NEKO_GUI_PANEL_GROUP);
    x_offset = neko_gui_find_value(win, title_hash);
    if (!x_offset) {
        x_offset = neko_gui_add_value(ctx, win, title_hash, 0);
        y_offset = neko_gui_add_value(ctx, win, title_hash + 1, 0);

        NEKO_GUI_ASSERT(x_offset);
        NEKO_GUI_ASSERT(y_offset);
        if (!x_offset || !y_offset) return 0;
        *x_offset = *y_offset = 0;
    } else
        y_offset = neko_gui_find_value(win, title_hash + 1);
    view->scroll_value = *y_offset;
    view->scroll_pointer = y_offset;

    *y_offset = 0;
    result = neko_gui_group_scrolled_offset_begin(ctx, x_offset, y_offset, title, flags);
    win = ctx->current;
    layout = win->layout;

    view->total_height = row_height * NEKO_GUI_MAX(row_count, 1);
    view->begin = (int)NEKO_GUI_MAX(((float)view->scroll_value / (float)row_height), 0.0f);
    view->count = (int)NEKO_GUI_MAX(neko_gui_iceilf((layout->clip.h) / (float)row_height), 0);
    view->count = NEKO_GUI_MIN(view->count, row_count - view->begin);
    view->end = view->begin + view->count;
    view->ctx = ctx;
    return result;
}
NEKO_GUI_API void neko_gui_list_view_end(struct neko_gui_list_view *view) {
    struct neko_gui_context *ctx;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;

    NEKO_GUI_ASSERT(view);
    NEKO_GUI_ASSERT(view->ctx);
    NEKO_GUI_ASSERT(view->scroll_pointer);
    if (!view || !view->ctx) return;

    ctx = view->ctx;
    win = ctx->current;
    layout = win->layout;
    layout->at_y = layout->bounds.y + (float)view->total_height;
    *view->scroll_pointer = *view->scroll_pointer + view->scroll_value;
    neko_gui_group_end(view->ctx);
}

NEKO_GUI_API struct neko_gui_rect neko_gui_widget_bounds(struct neko_gui_context *ctx) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_rect(0, 0, 0, 0);
    neko_gui_layout_peek(&bounds, ctx);
    return bounds;
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_widget_position(struct neko_gui_context *ctx) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);

    neko_gui_layout_peek(&bounds, ctx);
    return neko_gui_vec2(bounds.x, bounds.y);
}
NEKO_GUI_API struct neko_gui_vec2 neko_gui_widget_size(struct neko_gui_context *ctx) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return neko_gui_vec2(0, 0);

    neko_gui_layout_peek(&bounds, ctx);
    return neko_gui_vec2(bounds.w, bounds.h);
}
NEKO_GUI_API float neko_gui_widget_width(struct neko_gui_context *ctx) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return 0;

    neko_gui_layout_peek(&bounds, ctx);
    return bounds.w;
}
NEKO_GUI_API float neko_gui_widget_height(struct neko_gui_context *ctx) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return 0;

    neko_gui_layout_peek(&bounds, ctx);
    return bounds.h;
}
NEKO_GUI_API neko_gui_bool neko_gui_widget_is_hovered(struct neko_gui_context *ctx) {
    struct neko_gui_rect c, v;
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current || ctx->active != ctx->current) return 0;

    c = ctx->current->layout->clip;
    c.x = (float)((int)c.x);
    c.y = (float)((int)c.y);
    c.w = (float)((int)c.w);
    c.h = (float)((int)c.h);

    neko_gui_layout_peek(&bounds, ctx);
    neko_gui_unify(&v, &c, bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h);
    if (!NEKO_GUI_INTERSECT(c.x, c.y, c.w, c.h, bounds.x, bounds.y, bounds.w, bounds.h)) return 0;
    return neko_gui_input_is_mouse_hovering_rect(&ctx->input, bounds);
}
NEKO_GUI_API neko_gui_bool neko_gui_widget_is_mouse_clicked(struct neko_gui_context *ctx, enum neko_gui_buttons btn) {
    struct neko_gui_rect c, v;
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current || ctx->active != ctx->current) return 0;

    c = ctx->current->layout->clip;
    c.x = (float)((int)c.x);
    c.y = (float)((int)c.y);
    c.w = (float)((int)c.w);
    c.h = (float)((int)c.h);

    neko_gui_layout_peek(&bounds, ctx);
    neko_gui_unify(&v, &c, bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h);
    if (!NEKO_GUI_INTERSECT(c.x, c.y, c.w, c.h, bounds.x, bounds.y, bounds.w, bounds.h)) return 0;
    return neko_gui_input_mouse_clicked(&ctx->input, btn, bounds);
}
NEKO_GUI_API neko_gui_bool neko_gui_widget_has_mouse_click_down(struct neko_gui_context *ctx, enum neko_gui_buttons btn, neko_gui_bool down) {
    struct neko_gui_rect c, v;
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current || ctx->active != ctx->current) return 0;

    c = ctx->current->layout->clip;
    c.x = (float)((int)c.x);
    c.y = (float)((int)c.y);
    c.w = (float)((int)c.w);
    c.h = (float)((int)c.h);

    neko_gui_layout_peek(&bounds, ctx);
    neko_gui_unify(&v, &c, bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h);
    if (!NEKO_GUI_INTERSECT(c.x, c.y, c.w, c.h, bounds.x, bounds.y, bounds.w, bounds.h)) return 0;
    return neko_gui_input_has_mouse_click_down_in_rect(&ctx->input, btn, bounds, down);
}
NEKO_GUI_API enum neko_gui_widget_layout_states neko_gui_widget(struct neko_gui_rect *bounds, const struct neko_gui_context *ctx) {
    struct neko_gui_rect c, v;
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return NEKO_GUI_WIDGET_INVALID;

    neko_gui_panel_alloc_space(bounds, ctx);
    win = ctx->current;
    layout = win->layout;
    in = &ctx->input;
    c = layout->clip;

    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_MINIMIZED));
    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_HIDDEN));
    NEKO_GUI_ASSERT(!(layout->flags & NEKO_GUI_WINDOW_CLOSED));

    bounds->x = (float)((int)bounds->x);
    bounds->y = (float)((int)bounds->y);
    bounds->w = (float)((int)bounds->w);
    bounds->h = (float)((int)bounds->h);

    c.x = (float)((int)c.x);
    c.y = (float)((int)c.y);
    c.w = (float)((int)c.w);
    c.h = (float)((int)c.h);

    neko_gui_unify(&v, &c, bounds->x, bounds->y, bounds->x + bounds->w, bounds->y + bounds->h);
    if (!NEKO_GUI_INTERSECT(c.x, c.y, c.w, c.h, bounds->x, bounds->y, bounds->w, bounds->h)) return NEKO_GUI_WIDGET_INVALID;
    if (!NEKO_GUI_INBOX(in->mouse.pos.x, in->mouse.pos.y, v.x, v.y, v.w, v.h)) return NEKO_GUI_WIDGET_ROM;
    return NEKO_GUI_WIDGET_VALID;
}
NEKO_GUI_API enum neko_gui_widget_layout_states neko_gui_widget_fitting(struct neko_gui_rect *bounds, struct neko_gui_context *ctx, struct neko_gui_vec2 item_padding) {

    enum neko_gui_widget_layout_states state;
    NEKO_GUI_UNUSED(item_padding);

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return NEKO_GUI_WIDGET_INVALID;

    state = neko_gui_widget(bounds, ctx);
    return state;
}
NEKO_GUI_API void neko_gui_spacing(struct neko_gui_context *ctx, int cols) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    struct neko_gui_rect none;
    int i, index, rows;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    index = (layout->row.index + cols) % layout->row.columns;
    rows = (layout->row.index + cols) / layout->row.columns;
    if (rows) {
        for (i = 0; i < rows; ++i) neko_gui_panel_alloc_row(ctx, win);
        cols = index;
    }

    if (layout->row.type != NEKO_GUI_LAYOUT_DYNAMIC_FIXED && layout->row.type != NEKO_GUI_LAYOUT_STATIC_FIXED) {
        for (i = 0; i < cols; ++i) neko_gui_panel_alloc_space(&none, ctx);
    }
    layout->row.index = index;
}

NEKO_GUI_LIB void neko_gui_widget_text(struct neko_gui_command_buffer *o, struct neko_gui_rect b, const char *string, int len, const struct neko_gui_text *t, neko_gui_flags a,
                                       const struct neko_gui_user_font *f) {
    struct neko_gui_rect label;
    float text_width;

    NEKO_GUI_ASSERT(o);
    NEKO_GUI_ASSERT(t);
    if (!o || !t) return;

    b.h = NEKO_GUI_MAX(b.h, 2 * t->padding.y);
    label.x = 0;
    label.w = 0;
    label.y = b.y + t->padding.y;
    label.h = NEKO_GUI_MIN(f->height, b.h - 2 * t->padding.y);

    text_width = f->width(f->userdata, f->height, (const char *)string, len);
    text_width += (2.0f * t->padding.x);

    if (a & NEKO_GUI_TEXT_ALIGN_LEFT) {
        label.x = b.x + t->padding.x;
        label.w = NEKO_GUI_MAX(0, b.w - 2 * t->padding.x);
    } else if (a & NEKO_GUI_TEXT_ALIGN_CENTERED) {
        label.w = NEKO_GUI_MAX(1, 2 * t->padding.x + (float)text_width);
        label.x = (b.x + t->padding.x + ((b.w - 2 * t->padding.x) - label.w) / 2);
        label.x = NEKO_GUI_MAX(b.x + t->padding.x, label.x);
        label.w = NEKO_GUI_MIN(b.x + b.w, label.x + label.w);
        if (label.w >= label.x) label.w -= label.x;
    } else if (a & NEKO_GUI_TEXT_ALIGN_RIGHT) {
        label.x = NEKO_GUI_MAX(b.x + t->padding.x, (b.x + b.w) - (2 * t->padding.x + (float)text_width));
        label.w = (float)text_width + 2 * t->padding.x;
    } else
        return;

    if (a & NEKO_GUI_TEXT_ALIGN_MIDDLE) {
        label.y = b.y + b.h / 2.0f - (float)f->height / 2.0f;
        label.h = NEKO_GUI_MAX(b.h / 2.0f, b.h - (b.h / 2.0f + f->height / 2.0f));
    } else if (a & NEKO_GUI_TEXT_ALIGN_BOTTOM) {
        label.y = b.y + b.h - f->height;
        label.h = f->height;
    }
    neko_gui_draw_text(o, label, (const char *)string, len, f, t->background, t->text);
}
NEKO_GUI_LIB void neko_gui_widget_text_wrap(struct neko_gui_command_buffer *o, struct neko_gui_rect b, const char *string, int len, const struct neko_gui_text *t, const struct neko_gui_user_font *f) {
    float width;
    int glyphs = 0;
    int fitting = 0;
    int done = 0;
    struct neko_gui_rect line;
    struct neko_gui_text text;
    NEKO_GUI_INTERN neko_gui_rune seperator[] = {' '};

    NEKO_GUI_ASSERT(o);
    NEKO_GUI_ASSERT(t);
    if (!o || !t) return;

    text.padding = neko_gui_vec2(0, 0);
    text.background = t->background;
    text.text = t->text;

    b.w = NEKO_GUI_MAX(b.w, 2 * t->padding.x);
    b.h = NEKO_GUI_MAX(b.h, 2 * t->padding.y);
    b.h = b.h - 2 * t->padding.y;

    line.x = b.x + t->padding.x;
    line.y = b.y + t->padding.y;
    line.w = b.w - 2 * t->padding.x;
    line.h = 2 * t->padding.y + f->height;

    fitting = neko_gui_text_clamp(f, string, len, line.w, &glyphs, &width, seperator, NEKO_GUI_LEN(seperator));
    while (done < len) {
        if (!fitting || line.y + line.h >= (b.y + b.h)) break;
        neko_gui_widget_text(o, line, &string[done], fitting, &text, NEKO_GUI_TEXT_LEFT, f);
        done += fitting;
        line.y += f->height + 2 * t->padding.y;
        fitting = neko_gui_text_clamp(f, &string[done], len - done, line.w, &glyphs, &width, seperator, NEKO_GUI_LEN(seperator));
    }
}
NEKO_GUI_API void neko_gui_text_colored(struct neko_gui_context *ctx, const char *str, int len, neko_gui_flags alignment, struct neko_gui_color color) {
    struct neko_gui_window *win;
    const struct neko_gui_style *style;

    struct neko_gui_vec2 item_padding;
    struct neko_gui_rect bounds;
    struct neko_gui_text text;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    style = &ctx->style;
    neko_gui_panel_alloc_space(&bounds, ctx);
    item_padding = style->text.padding;

    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = color;
    neko_gui_widget_text(&win->buffer, bounds, str, len, &text, alignment, style->font);
}
NEKO_GUI_API void neko_gui_text_wrap_colored(struct neko_gui_context *ctx, const char *str, int len, struct neko_gui_color color) {
    struct neko_gui_window *win;
    const struct neko_gui_style *style;

    struct neko_gui_vec2 item_padding;
    struct neko_gui_rect bounds;
    struct neko_gui_text text;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    style = &ctx->style;
    neko_gui_panel_alloc_space(&bounds, ctx);
    item_padding = style->text.padding;

    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = color;
    neko_gui_widget_text_wrap(&win->buffer, bounds, str, len, &text, style->font);
}
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
NEKO_GUI_API void neko_gui_labelf_colored(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_color color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    neko_gui_labelfv_colored(ctx, flags, color, fmt, args);
    va_end(args);
}
NEKO_GUI_API void neko_gui_labelf_colored_wrap(struct neko_gui_context *ctx, struct neko_gui_color color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    neko_gui_labelfv_colored_wrap(ctx, color, fmt, args);
    va_end(args);
}
NEKO_GUI_API void neko_gui_labelf(struct neko_gui_context *ctx, neko_gui_flags flags, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    neko_gui_labelfv(ctx, flags, fmt, args);
    va_end(args);
}
NEKO_GUI_API void neko_gui_labelf_wrap(struct neko_gui_context *ctx, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    neko_gui_labelfv_wrap(ctx, fmt, args);
    va_end(args);
}
NEKO_GUI_API void neko_gui_labelfv_colored(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_color color, const char *fmt, va_list args) {
    char buf[256];
    neko_gui_strfmt(buf, NEKO_GUI_LEN(buf), fmt, args);
    neko_gui_label_colored(ctx, buf, flags, color);
}

NEKO_GUI_API void neko_gui_labelfv_colored_wrap(struct neko_gui_context *ctx, struct neko_gui_color color, const char *fmt, va_list args) {
    char buf[256];
    neko_gui_strfmt(buf, NEKO_GUI_LEN(buf), fmt, args);
    neko_gui_label_colored_wrap(ctx, buf, color);
}

NEKO_GUI_API void neko_gui_labelfv(struct neko_gui_context *ctx, neko_gui_flags flags, const char *fmt, va_list args) {
    char buf[256];
    neko_gui_strfmt(buf, NEKO_GUI_LEN(buf), fmt, args);
    neko_gui_label(ctx, buf, flags);
}

NEKO_GUI_API void neko_gui_labelfv_wrap(struct neko_gui_context *ctx, const char *fmt, va_list args) {
    char buf[256];
    neko_gui_strfmt(buf, NEKO_GUI_LEN(buf), fmt, args);
    neko_gui_label_wrap(ctx, buf);
}

NEKO_GUI_API void neko_gui_value_bool(struct neko_gui_context *ctx, const char *prefix, int value) { neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: %s", prefix, ((value) ? "true" : "false")); }
NEKO_GUI_API void neko_gui_value_int(struct neko_gui_context *ctx, const char *prefix, int value) { neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: %d", prefix, value); }
NEKO_GUI_API void neko_gui_value_uint(struct neko_gui_context *ctx, const char *prefix, unsigned int value) { neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: %u", prefix, value); }
NEKO_GUI_API void neko_gui_value_float(struct neko_gui_context *ctx, const char *prefix, float value) {
    double double_value = (double)value;
    neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: %.3f", prefix, double_value);
}
NEKO_GUI_API void neko_gui_value_color_byte(struct neko_gui_context *ctx, const char *p, struct neko_gui_color c) {
    neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: (%d, %d, %d, %d)", p, c.r, c.g, c.b, c.a);
}
NEKO_GUI_API void neko_gui_value_color_float(struct neko_gui_context *ctx, const char *p, struct neko_gui_color color) {
    double c[4];
    neko_gui_color_dv(c, color);
    neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: (%.2f, %.2f, %.2f, %.2f)", p, c[0], c[1], c[2], c[3]);
}
NEKO_GUI_API void neko_gui_value_color_hex(struct neko_gui_context *ctx, const char *prefix, struct neko_gui_color color) {
    char hex[16];
    neko_gui_color_hex_rgba(hex, color);
    neko_gui_labelf(ctx, NEKO_GUI_TEXT_LEFT, "%s: %s", prefix, hex);
}
#endif
NEKO_GUI_API void neko_gui_text(struct neko_gui_context *ctx, const char *str, int len, neko_gui_flags alignment) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    neko_gui_text_colored(ctx, str, len, alignment, ctx->style.text.color);
}
NEKO_GUI_API void neko_gui_text_wrap(struct neko_gui_context *ctx, const char *str, int len) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    neko_gui_text_wrap_colored(ctx, str, len, ctx->style.text.color);
}
NEKO_GUI_API void neko_gui_label(struct neko_gui_context *ctx, const char *str, neko_gui_flags alignment) { neko_gui_text(ctx, str, neko_gui_strlen(str), alignment); }
NEKO_GUI_API void neko_gui_label_colored(struct neko_gui_context *ctx, const char *str, neko_gui_flags align, struct neko_gui_color color) {
    neko_gui_text_colored(ctx, str, neko_gui_strlen(str), align, color);
}
NEKO_GUI_API void neko_gui_label_wrap(struct neko_gui_context *ctx, const char *str) { neko_gui_text_wrap(ctx, str, neko_gui_strlen(str)); }
NEKO_GUI_API void neko_gui_label_colored_wrap(struct neko_gui_context *ctx, const char *str, struct neko_gui_color color) { neko_gui_text_wrap_colored(ctx, str, neko_gui_strlen(str), color); }

NEKO_GUI_API neko_gui_handle neko_gui_handle_ptr(void *ptr) {
    neko_gui_handle handle = {0};
    handle.ptr = ptr;
    return handle;
}
NEKO_GUI_API neko_gui_handle neko_gui_handle_id(int id) {
    neko_gui_handle handle;
    neko_gui_zero_struct(handle);
    handle.id = id;
    return handle;
}
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_ptr(void *ptr, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect r) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    s.handle.ptr = ptr;
    s.w = w;
    s.h = h;
    s.region[0] = (neko_gui_ushort)r.x;
    s.region[1] = (neko_gui_ushort)r.y;
    s.region[2] = (neko_gui_ushort)r.w;
    s.region[3] = (neko_gui_ushort)r.h;
    return s;
}
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_id(int id, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect r) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    s.handle.id = id;
    s.w = w;
    s.h = h;
    s.region[0] = (neko_gui_ushort)r.x;
    s.region[1] = (neko_gui_ushort)r.y;
    s.region[2] = (neko_gui_ushort)r.w;
    s.region[3] = (neko_gui_ushort)r.h;
    return s;
}
NEKO_GUI_API struct neko_gui_image neko_gui_subimage_handle(neko_gui_handle handle, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect r) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    s.handle = handle;
    s.w = w;
    s.h = h;
    s.region[0] = (neko_gui_ushort)r.x;
    s.region[1] = (neko_gui_ushort)r.y;
    s.region[2] = (neko_gui_ushort)r.w;
    s.region[3] = (neko_gui_ushort)r.h;
    return s;
}
NEKO_GUI_API struct neko_gui_image neko_gui_image_handle(neko_gui_handle handle) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    s.handle = handle;
    s.w = 0;
    s.h = 0;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NEKO_GUI_API struct neko_gui_image neko_gui_image_ptr(void *ptr) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    NEKO_GUI_ASSERT(ptr);
    s.handle.ptr = ptr;
    s.w = 0;
    s.h = 0;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NEKO_GUI_API struct neko_gui_image neko_gui_image_id(int id) {
    struct neko_gui_image s;
    neko_gui_zero(&s, sizeof(s));
    s.handle.id = id;
    s.w = 0;
    s.h = 0;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NEKO_GUI_API neko_gui_bool neko_gui_image_is_subimage(const struct neko_gui_image *img) {
    NEKO_GUI_ASSERT(img);
    return !(img->w == 0 && img->h == 0);
}
NEKO_GUI_API void neko_gui_image(struct neko_gui_context *ctx, struct neko_gui_image img) {
    struct neko_gui_window *win;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    if (!neko_gui_widget(&bounds, ctx)) return;
    neko_gui_draw_image(&win->buffer, bounds, &img, neko_gui_white);
}
NEKO_GUI_API void neko_gui_image_color(struct neko_gui_context *ctx, struct neko_gui_image img, struct neko_gui_color col) {
    struct neko_gui_window *win;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    if (!neko_gui_widget(&bounds, ctx)) return;
    neko_gui_draw_image(&win->buffer, bounds, &img, col);
}

NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_ptr(void *ptr, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect rgn, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r,
                                                               neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    i->handle.ptr = ptr;
    i->w = w;
    i->h = h;
    i->region[0] = (neko_gui_ushort)rgn.x;
    i->region[1] = (neko_gui_ushort)rgn.y;
    i->region[2] = (neko_gui_ushort)rgn.w;
    i->region[3] = (neko_gui_ushort)rgn.h;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_id(int id, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect rgn, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r,
                                                              neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    i->handle.id = id;
    i->w = w;
    i->h = h;
    i->region[0] = (neko_gui_ushort)rgn.x;
    i->region[1] = (neko_gui_ushort)rgn.y;
    i->region[2] = (neko_gui_ushort)rgn.w;
    i->region[3] = (neko_gui_ushort)rgn.h;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_sub9slice_handle(neko_gui_handle handle, neko_gui_ushort w, neko_gui_ushort h, struct neko_gui_rect rgn, neko_gui_ushort l, neko_gui_ushort t,
                                                                  neko_gui_ushort r, neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    i->handle = handle;
    i->w = w;
    i->h = h;
    i->region[0] = (neko_gui_ushort)rgn.x;
    i->region[1] = (neko_gui_ushort)rgn.y;
    i->region[2] = (neko_gui_ushort)rgn.w;
    i->region[3] = (neko_gui_ushort)rgn.h;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_handle(neko_gui_handle handle, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    i->handle = handle;
    i->w = 0;
    i->h = 0;
    i->region[0] = 0;
    i->region[1] = 0;
    i->region[2] = 0;
    i->region[3] = 0;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_ptr(void *ptr, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    NEKO_GUI_ASSERT(ptr);
    i->handle.ptr = ptr;
    i->w = 0;
    i->h = 0;
    i->region[0] = 0;
    i->region[1] = 0;
    i->region[2] = 0;
    i->region[3] = 0;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API struct neko_gui_nine_slice neko_gui_nine_slice_id(int id, neko_gui_ushort l, neko_gui_ushort t, neko_gui_ushort r, neko_gui_ushort b) {
    struct neko_gui_nine_slice s;
    struct neko_gui_image *i = &s.img;
    neko_gui_zero(&s, sizeof(s));
    i->handle.id = id;
    i->w = 0;
    i->h = 0;
    i->region[0] = 0;
    i->region[1] = 0;
    i->region[2] = 0;
    i->region[3] = 0;
    s.l = l;
    s.t = t;
    s.r = r;
    s.b = b;
    return s;
}
NEKO_GUI_API int neko_gui_nine_slice_is_sub9slice(const struct neko_gui_nine_slice *slice) {
    NEKO_GUI_ASSERT(slice);
    return !(slice->img.w == 0 && slice->img.h == 0);
}

NEKO_GUI_LIB void neko_gui_draw_symbol(struct neko_gui_command_buffer *out, enum neko_gui_symbol_type type, struct neko_gui_rect content, struct neko_gui_color background,
                                       struct neko_gui_color foreground, float border_width, const struct neko_gui_user_font *font) {
    switch (type) {
        case NEKO_GUI_SYMBOL_X:
        case NEKO_GUI_SYMBOL_UNDERSCORE:
        case NEKO_GUI_SYMBOL_PLUS:
        case NEKO_GUI_SYMBOL_MINUS: {

            const char *X = (type == NEKO_GUI_SYMBOL_X) ? "x" : (type == NEKO_GUI_SYMBOL_UNDERSCORE) ? "_" : (type == NEKO_GUI_SYMBOL_PLUS) ? "+" : "-";
            struct neko_gui_text text;
            text.padding = neko_gui_vec2(0, 0);
            text.background = background;
            text.text = foreground;
            neko_gui_widget_text(out, content, X, 1, &text, NEKO_GUI_TEXT_CENTERED, font);
        } break;
        case NEKO_GUI_SYMBOL_CIRCLE_SOLID:
        case NEKO_GUI_SYMBOL_CIRCLE_OUTLINE:
        case NEKO_GUI_SYMBOL_RECT_SOLID:
        case NEKO_GUI_SYMBOL_RECT_OUTLINE: {

            if (type == NEKO_GUI_SYMBOL_RECT_SOLID || type == NEKO_GUI_SYMBOL_RECT_OUTLINE) {
                neko_gui_fill_rect(out, content, 0, foreground);
                if (type == NEKO_GUI_SYMBOL_RECT_OUTLINE) neko_gui_fill_rect(out, neko_gui_shrineko_gui_rect(content, border_width), 0, background);
            } else {
                neko_gui_fill_circle(out, content, foreground);
                if (type == NEKO_GUI_SYMBOL_CIRCLE_OUTLINE) neko_gui_fill_circle(out, neko_gui_shrineko_gui_rect(content, 1), background);
            }
        } break;
        case NEKO_GUI_SYMBOL_TRIANGLE_UP:
        case NEKO_GUI_SYMBOL_TRIANGLE_DOWN:
        case NEKO_GUI_SYMBOL_TRIANGLE_LEFT:
        case NEKO_GUI_SYMBOL_TRIANGLE_RIGHT: {
            enum neko_gui_heading heading;
            struct neko_gui_vec2 points[3];
            heading = (type == NEKO_GUI_SYMBOL_TRIANGLE_RIGHT)  ? NEKO_GUI_RIGHT
                      : (type == NEKO_GUI_SYMBOL_TRIANGLE_LEFT) ? NEKO_GUI_LEFT
                      : (type == NEKO_GUI_SYMBOL_TRIANGLE_UP)   ? NEKO_GUI_UP
                                                                : NEKO_GUI_DOWN;
            neko_gui_triangle_from_direction(points, content, 0, 0, heading);
            neko_gui_fill_triangle(out, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, foreground);
        } break;
        default:
        case NEKO_GUI_SYMBOL_NONE:
        case NEKO_GUI_SYMBOL_MAX:
            break;
    }
}
NEKO_GUI_LIB neko_gui_bool neko_gui_button_behavior(neko_gui_flags *state, struct neko_gui_rect r, const struct neko_gui_input *i, enum neko_gui_button_behavior behavior) {
    int ret = 0;
    neko_gui_widget_state_reset(state);
    if (!i) return 0;
    if (neko_gui_input_is_mouse_hovering_rect(i, r)) {
        *state = NEKO_GUI_WIDGET_STATE_HOVERED;
        if (neko_gui_input_is_mouse_down(i, NEKO_GUI_BUTTON_LEFT)) *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
        if (neko_gui_input_has_mouse_click_in_button_rect(i, NEKO_GUI_BUTTON_LEFT, r)) {
            ret = (behavior != NEKO_GUI_BUTTON_DEFAULT) ? neko_gui_input_is_mouse_down(i, NEKO_GUI_BUTTON_LEFT) :
#ifdef NEKO_GUI_BUTTON_TRIGGER_ON_RELEASE
                                                        neko_gui_input_is_mouse_released(i, NEKO_GUI_BUTTON_LEFT);
#else
                                                        neko_gui_input_is_mouse_pressed(i, NEKO_GUI_BUTTON_LEFT);
#endif
        }
    }
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(i, r))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(i, r))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return ret;
}
NEKO_GUI_LIB const struct neko_gui_style_item *neko_gui_draw_button(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, neko_gui_flags state,
                                                                    const struct neko_gui_style_button *style) {
    const struct neko_gui_style_item *background;
    if (state & NEKO_GUI_WIDGET_STATE_HOVER)
        background = &style->hover;
    else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        background = &style->active;
    else
        background = &style->normal;

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            neko_gui_stroke_rect(out, *bounds, style->rounding, style->border, style->border_color);
            break;
    }
    return background;
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect r, const struct neko_gui_style_button *style,
                                              const struct neko_gui_input *in, enum neko_gui_button_behavior behavior, struct neko_gui_rect *content) {
    struct neko_gui_rect bounds;
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(out);
    if (!out || !style) return neko_gui_false;

    content->x = r.x + style->padding.x + style->border + style->rounding;
    content->y = r.y + style->padding.y + style->border + style->rounding;
    content->w = r.w - (2 * style->padding.x + style->border + style->rounding * 2);
    content->h = r.h - (2 * style->padding.y + style->border + style->rounding * 2);

    bounds.x = r.x - style->touch_padding.x;
    bounds.y = r.y - style->touch_padding.y;
    bounds.w = r.w + 2 * style->touch_padding.x;
    bounds.h = r.h + 2 * style->touch_padding.y;
    return neko_gui_button_behavior(state, bounds, in, behavior);
}
NEKO_GUI_LIB void neko_gui_draw_button_text(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                            const struct neko_gui_style_button *style, const char *txt, int len, neko_gui_flags text_alignment, const struct neko_gui_user_font *font) {
    struct neko_gui_text text;
    const struct neko_gui_style_item *background;
    background = neko_gui_draw_button(out, bounds, state, style);

    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR)
        text.background = background->data.color;
    else
        text.background = style->text_background;
    if (state & NEKO_GUI_WIDGET_STATE_HOVER)
        text.text = style->text_hover;
    else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        text.text = style->text_active;
    else
        text.text = style->text_normal;

    text.padding = neko_gui_vec2(0, 0);
    neko_gui_widget_text(out, *content, txt, len, &text, text_alignment, font);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *string, int len, neko_gui_flags align,
                                                   enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_input *in,
                                                   const struct neko_gui_user_font *font) {
    struct neko_gui_rect content;
    int ret = neko_gui_false;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(string);
    NEKO_GUI_ASSERT(font);
    if (!out || !style || !font || !string) return neko_gui_false;

    ret = neko_gui_do_button(state, out, bounds, style, in, behavior, &content);
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_button_text(out, &bounds, &content, *state, style, string, len, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}
NEKO_GUI_LIB void neko_gui_draw_button_symbol(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                              const struct neko_gui_style_button *style, enum neko_gui_symbol_type type, const struct neko_gui_user_font *font) {
    struct neko_gui_color sym, bg;
    const struct neko_gui_style_item *background;

    background = neko_gui_draw_button(out, bounds, state, style);
    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR)
        bg = background->data.color;
    else
        bg = style->text_background;

    if (state & NEKO_GUI_WIDGET_STATE_HOVER)
        sym = style->text_hover;
    else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        sym = style->text_active;
    else
        sym = style->text_normal;
    neko_gui_draw_symbol(out, type, *content, bg, sym, 1, font);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_symbol(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, enum neko_gui_symbol_type symbol,
                                                     enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_input *in,
                                                     const struct neko_gui_user_font *font) {
    int ret;
    struct neko_gui_rect content;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(out);
    if (!out || !style || !font || !state) return neko_gui_false;

    ret = neko_gui_do_button(state, out, bounds, style, in, behavior, &content);
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_button_symbol(out, &bounds, &content, *state, style, symbol, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}
NEKO_GUI_LIB void neko_gui_draw_button_image(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *content, neko_gui_flags state,
                                             const struct neko_gui_style_button *style, const struct neko_gui_image *img) {
    neko_gui_draw_button(out, bounds, state, style);
    neko_gui_draw_image(out, *content, img, neko_gui_white);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, struct neko_gui_image img, enum neko_gui_button_behavior b,
                                                    const struct neko_gui_style_button *style, const struct neko_gui_input *in) {
    int ret;
    struct neko_gui_rect content;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    if (!out || !style || !state) return neko_gui_false;

    ret = neko_gui_do_button(state, out, bounds, style, in, b, &content);
    content.x += style->image_padding.x;
    content.y += style->image_padding.y;
    content.w -= 2 * style->image_padding.x;
    content.h -= 2 * style->image_padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_button_image(out, &bounds, &content, *state, style, &img);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}
NEKO_GUI_LIB void neko_gui_draw_button_text_symbol(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label, const struct neko_gui_rect *symbol,
                                                   neko_gui_flags state, const struct neko_gui_style_button *style, const char *str, int len, enum neko_gui_symbol_type type,
                                                   const struct neko_gui_user_font *font) {
    struct neko_gui_color sym;
    struct neko_gui_text text;
    const struct neko_gui_style_item *background;

    background = neko_gui_draw_button(out, bounds, state, style);
    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR)
        text.background = background->data.color;
    else
        text.background = style->text_background;

    if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        sym = style->text_hover;
        text.text = style->text_hover;
    } else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        sym = style->text_active;
        text.text = style->text_active;
    } else {
        sym = style->text_normal;
        text.text = style->text_normal;
    }

    text.padding = neko_gui_vec2(0, 0);
    neko_gui_draw_symbol(out, type, *symbol, style->text_background, sym, 0, font);
    neko_gui_widget_text(out, *label, str, len, &text, NEKO_GUI_TEXT_CENTERED, font);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text_symbol(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, enum neko_gui_symbol_type symbol, const char *str,
                                                          int len, neko_gui_flags align, enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style,
                                                          const struct neko_gui_user_font *font, const struct neko_gui_input *in) {
    int ret;
    struct neko_gui_rect tri = {0, 0, 0, 0};
    struct neko_gui_rect content;

    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(font);
    if (!out || !style || !font) return neko_gui_false;

    ret = neko_gui_do_button(state, out, bounds, style, in, behavior, &content);
    tri.y = content.y + (content.h / 2) - font->height / 2;
    tri.w = font->height;
    tri.h = font->height;
    if (align & NEKO_GUI_TEXT_ALIGN_LEFT) {
        tri.x = (content.x + content.w) - (2 * style->padding.x + tri.w);
        tri.x = NEKO_GUI_MAX(tri.x, 0);
    } else
        tri.x = content.x + 2 * style->padding.x;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_button_text_symbol(out, &bounds, &content, &tri, *state, style, str, len, symbol, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}
NEKO_GUI_LIB void neko_gui_draw_button_text_image(struct neko_gui_command_buffer *out, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label, const struct neko_gui_rect *image,
                                                  neko_gui_flags state, const struct neko_gui_style_button *style, const char *str, int len, const struct neko_gui_user_font *font,
                                                  const struct neko_gui_image *img) {
    struct neko_gui_text text;
    const struct neko_gui_style_item *background;
    background = neko_gui_draw_button(out, bounds, state, style);

    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR)
        text.background = background->data.color;
    else
        text.background = style->text_background;
    if (state & NEKO_GUI_WIDGET_STATE_HOVER)
        text.text = style->text_hover;
    else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        text.text = style->text_active;
    else
        text.text = style->text_normal;

    text.padding = neko_gui_vec2(0, 0);
    neko_gui_widget_text(out, *label, str, len, &text, NEKO_GUI_TEXT_CENTERED, font);
    neko_gui_draw_image(out, *image, img, neko_gui_white);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_button_text_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, struct neko_gui_image img, const char *str, int len,
                                                         neko_gui_flags align, enum neko_gui_button_behavior behavior, const struct neko_gui_style_button *style, const struct neko_gui_user_font *font,
                                                         const struct neko_gui_input *in) {
    int ret;
    struct neko_gui_rect icon;
    struct neko_gui_rect content;

    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(out);
    if (!out || !font || !style || !str) return neko_gui_false;

    ret = neko_gui_do_button(state, out, bounds, style, in, behavior, &content);
    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NEKO_GUI_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NEKO_GUI_MAX(icon.x, 0);
    } else
        icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_button_text_image(out, &bounds, &content, &icon, *state, style, str, len, font, &img);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}
NEKO_GUI_API void neko_gui_button_set_behavior(struct neko_gui_context *ctx, enum neko_gui_button_behavior behavior) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return;
    ctx->button_behavior = behavior;
}
NEKO_GUI_API neko_gui_bool neko_gui_button_push_behavior(struct neko_gui_context *ctx, enum neko_gui_button_behavior behavior) {
    struct neko_gui_config_stack_button_behavior *button_stack;
    struct neko_gui_config_stack_button_behavior_element *element;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    button_stack = &ctx->stacks.button_behaviors;
    NEKO_GUI_ASSERT(button_stack->head < (int)NEKO_GUI_LEN(button_stack->elements));
    if (button_stack->head >= (int)NEKO_GUI_LEN(button_stack->elements)) return 0;

    element = &button_stack->elements[button_stack->head++];
    element->address = &ctx->button_behavior;
    element->old_value = ctx->button_behavior;
    ctx->button_behavior = behavior;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_button_pop_behavior(struct neko_gui_context *ctx) {
    struct neko_gui_config_stack_button_behavior *button_stack;
    struct neko_gui_config_stack_button_behavior_element *element;

    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;

    button_stack = &ctx->stacks.button_behaviors;
    NEKO_GUI_ASSERT(button_stack->head > 0);
    if (button_stack->head < 1) return 0;

    element = &button_stack->elements[--button_stack->head];
    *element->address = element->old_value;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_button_text_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, const char *title, int len) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!style || !ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;
    state = neko_gui_widget(&bounds, ctx);

    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_button_text(&ctx->last_widget_state, &win->buffer, bounds, title, len, style->text_alignment, ctx->button_behavior, style, in, ctx->style.font);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_text(struct neko_gui_context *ctx, const char *title, int len) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    return neko_gui_button_text_styled(ctx, &ctx->style.button, title, len);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_label_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, const char *title) {
    return neko_gui_button_text_styled(ctx, style, title, neko_gui_strlen(title));
}
NEKO_GUI_API neko_gui_bool neko_gui_button_label(struct neko_gui_context *ctx, const char *title) { return neko_gui_button_text(ctx, title, neko_gui_strlen(title)); }
NEKO_GUI_API neko_gui_bool neko_gui_button_color(struct neko_gui_context *ctx, struct neko_gui_color color) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    struct neko_gui_style_button button;

    int ret = 0;
    struct neko_gui_rect bounds;
    struct neko_gui_rect content;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;

    button = ctx->style.button;
    button.normal = neko_gui_style_item_color(color);
    button.hover = neko_gui_style_item_color(color);
    button.active = neko_gui_style_item_color(color);
    ret = neko_gui_do_button(&ctx->last_widget_state, &win->buffer, bounds, &button, in, ctx->button_behavior, &content);
    neko_gui_draw_button(&win->buffer, &bounds, ctx->last_widget_state, &button);
    return ret;
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, enum neko_gui_symbol_type symbol) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;
    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_button_symbol(&ctx->last_widget_state, &win->buffer, bounds, symbol, ctx->button_behavior, style, in, ctx->style.font);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    return neko_gui_button_symbol_styled(ctx, &ctx->style.button, symbol);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, struct neko_gui_image img) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_button_image(&ctx->last_widget_state, &win->buffer, bounds, img, ctx->button_behavior, style, in);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image(struct neko_gui_context *ctx, struct neko_gui_image img) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    return neko_gui_button_image_styled(ctx, &ctx->style.button, img);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_text_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, enum neko_gui_symbol_type symbol, const char *text, int len,
                                                              neko_gui_flags align) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_button_text_symbol(&ctx->last_widget_state, &win->buffer, bounds, symbol, text, len, align, ctx->button_behavior, style, ctx->style.font, in);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol, const char *text, int len, neko_gui_flags align) {
    NEKO_GUI_ASSERT(ctx);
    if (!ctx) return 0;
    return neko_gui_button_symbol_text_styled(ctx, &ctx->style.button, symbol, text, len, align);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol, const char *label, neko_gui_flags align) {
    return neko_gui_button_symbol_text(ctx, symbol, label, neko_gui_strlen(label), align);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_symbol_label_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, enum neko_gui_symbol_type symbol, const char *title,
                                                               neko_gui_flags align) {
    return neko_gui_button_symbol_text_styled(ctx, style, symbol, title, neko_gui_strlen(title), align);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image_text_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, struct neko_gui_image img, const char *text, int len,
                                                             neko_gui_flags align) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_button_text_image(&ctx->last_widget_state, &win->buffer, bounds, img, text, len, align, ctx->button_behavior, style, ctx->style.font, in);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *text, int len, neko_gui_flags align) {
    return neko_gui_button_image_text_styled(ctx, &ctx->style.button, img, text, len, align);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *label, neko_gui_flags align) {
    return neko_gui_button_image_text(ctx, img, label, neko_gui_strlen(label), align);
}
NEKO_GUI_API neko_gui_bool neko_gui_button_image_label_styled(struct neko_gui_context *ctx, const struct neko_gui_style_button *style, struct neko_gui_image img, const char *label,
                                                              neko_gui_flags text_alignment) {
    return neko_gui_button_image_text_styled(ctx, style, img, label, neko_gui_strlen(label), text_alignment);
}

NEKO_GUI_LIB neko_gui_bool neko_gui_toggle_behavior(const struct neko_gui_input *in, struct neko_gui_rect select, neko_gui_flags *state, neko_gui_bool active) {
    neko_gui_widget_state_reset(state);
    if (neko_gui_button_behavior(state, select, in, NEKO_GUI_BUTTON_DEFAULT)) {
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
        active = !active;
    }
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, select))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, select))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return active;
}
NEKO_GUI_LIB void neko_gui_draw_checkbox(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_toggle *style, neko_gui_bool active, const struct neko_gui_rect *label,
                                         const struct neko_gui_rect *selector, const struct neko_gui_rect *cursors, const char *string, int len, const struct neko_gui_user_font *font) {
    const struct neko_gui_style_item *background;
    const struct neko_gui_style_item *cursor;
    struct neko_gui_text text;

    if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_hover;
    } else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_active;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
        text.text = style->text_normal;
    }

    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR) {
        neko_gui_fill_rect(out, *selector, 0, style->border_color);
        neko_gui_fill_rect(out, neko_gui_shrineko_gui_rect(*selector, style->border), 0, background->data.color);
    } else
        neko_gui_draw_image(out, *selector, &background->data.image, neko_gui_white);
    if (active) {
        if (cursor->type == NEKO_GUI_STYLE_ITEM_IMAGE)
            neko_gui_draw_image(out, *cursors, &cursor->data.image, neko_gui_white);
        else
            neko_gui_fill_rect(out, *cursors, 0, cursor->data.color);
    }

    text.padding.x = 0;
    text.padding.y = 0;
    text.background = style->text_background;
    neko_gui_widget_text(out, *label, string, len, &text, NEKO_GUI_TEXT_LEFT, font);
}
NEKO_GUI_LIB void neko_gui_draw_option(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_toggle *style, neko_gui_bool active, const struct neko_gui_rect *label,
                                       const struct neko_gui_rect *selector, const struct neko_gui_rect *cursors, const char *string, int len, const struct neko_gui_user_font *font) {
    const struct neko_gui_style_item *background;
    const struct neko_gui_style_item *cursor;
    struct neko_gui_text text;

    if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_hover;
    } else if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_active;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
        text.text = style->text_normal;
    }

    if (background->type == NEKO_GUI_STYLE_ITEM_COLOR) {
        neko_gui_fill_circle(out, *selector, style->border_color);
        neko_gui_fill_circle(out, neko_gui_shrineko_gui_rect(*selector, style->border), background->data.color);
    } else
        neko_gui_draw_image(out, *selector, &background->data.image, neko_gui_white);
    if (active) {
        if (cursor->type == NEKO_GUI_STYLE_ITEM_IMAGE)
            neko_gui_draw_image(out, *cursors, &cursor->data.image, neko_gui_white);
        else
            neko_gui_fill_circle(out, *cursors, cursor->data.color);
    }

    text.padding.x = 0;
    text.padding.y = 0;
    text.background = style->text_background;
    neko_gui_widget_text(out, *label, string, len, &text, NEKO_GUI_TEXT_LEFT, font);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_toggle(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect r, neko_gui_bool *active, const char *str, int len,
                                              enum neko_gui_toggle_type type, const struct neko_gui_style_toggle *style, const struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    int was_active;
    struct neko_gui_rect bounds;
    struct neko_gui_rect select;
    struct neko_gui_rect cursor;
    struct neko_gui_rect label;

    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(font);
    if (!out || !style || !font || !active) return 0;

    r.w = NEKO_GUI_MAX(r.w, font->height + 2 * style->padding.x);
    r.h = NEKO_GUI_MAX(r.h, font->height + 2 * style->padding.y);

    bounds.x = r.x - style->touch_padding.x;
    bounds.y = r.y - style->touch_padding.y;
    bounds.w = r.w + 2 * style->touch_padding.x;
    bounds.h = r.h + 2 * style->touch_padding.y;

    select.w = font->height;
    select.h = select.w;
    select.y = r.y + r.h / 2.0f - select.h / 2.0f;
    select.x = r.x;

    cursor.x = select.x + style->padding.x + style->border;
    cursor.y = select.y + style->padding.y + style->border;
    cursor.w = select.w - (2 * style->padding.x + 2 * style->border);
    cursor.h = select.h - (2 * style->padding.y + 2 * style->border);

    label.x = select.x + select.w + style->spacing;
    label.y = select.y;
    label.w = NEKO_GUI_MAX(r.x + r.w, label.x) - label.x;
    label.h = select.w;

    was_active = *active;
    *active = neko_gui_toggle_behavior(in, bounds, state, *active);

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    if (type == NEKO_GUI_TOGGLE_CHECK) {
        neko_gui_draw_checkbox(out, *state, style, *active, &label, &select, &cursor, str, len, font);
    } else {
        neko_gui_draw_option(out, *state, style, *active, &label, &select, &cursor, str, len, font);
    }
    if (style->draw_end) style->draw_end(out, style->userdata);
    return (was_active != (int)(*active));
}

NEKO_GUI_API neko_gui_bool neko_gui_check_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_bool active) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return active;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    neko_gui_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &active, text, len, NEKO_GUI_TOGGLE_CHECK, &style->checkbox, in, style->font);
    return active;
}
NEKO_GUI_API unsigned int neko_gui_check_flags_text(struct neko_gui_context *ctx, const char *text, int len, unsigned int flags, unsigned int value) {
    int old_active;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(text);
    if (!ctx || !text) return flags;
    old_active = (int)((flags & value) & value);
    if (neko_gui_check_text(ctx, text, len, old_active))
        flags |= value;
    else
        flags &= ~value;
    return flags;
}
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_bool *active) {
    int old_val;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(text);
    NEKO_GUI_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_val = *active;
    *active = neko_gui_check_text(ctx, text, len, *active);
    return old_val != (int)(*active);
}
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_flags_text(struct neko_gui_context *ctx, const char *text, int len, unsigned int *flags, unsigned int value) {
    neko_gui_bool active;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(text);
    NEKO_GUI_ASSERT(flags);
    if (!ctx || !text || !flags) return 0;

    active = (int)((*flags & value) & value);
    if (neko_gui_checkbox_text(ctx, text, len, &active)) {
        if (active)
            *flags |= value;
        else
            *flags &= ~value;
        return 1;
    }
    return 0;
}
NEKO_GUI_API neko_gui_bool neko_gui_check_label(struct neko_gui_context *ctx, const char *label, neko_gui_bool active) { return neko_gui_check_text(ctx, label, neko_gui_strlen(label), active); }
NEKO_GUI_API unsigned int neko_gui_check_flags_label(struct neko_gui_context *ctx, const char *label, unsigned int flags, unsigned int value) {
    return neko_gui_check_flags_text(ctx, label, neko_gui_strlen(label), flags, value);
}
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_label(struct neko_gui_context *ctx, const char *label, neko_gui_bool *active) {
    return neko_gui_checkbox_text(ctx, label, neko_gui_strlen(label), active);
}
NEKO_GUI_API neko_gui_bool neko_gui_checkbox_flags_label(struct neko_gui_context *ctx, const char *label, unsigned int *flags, unsigned int value) {
    return neko_gui_checkbox_flags_text(ctx, label, neko_gui_strlen(label), flags, value);
}

NEKO_GUI_API neko_gui_bool neko_gui_option_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_bool is_active) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return is_active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return (int)state;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    neko_gui_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &is_active, text, len, NEKO_GUI_TOGGLE_OPTION, &style->option, in, style->font);
    return is_active;
}
NEKO_GUI_API neko_gui_bool neko_gui_radio_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_bool *active) {
    int old_value;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(text);
    NEKO_GUI_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_value = *active;
    *active = neko_gui_option_text(ctx, text, len, old_value);
    return old_value != (int)(*active);
}
NEKO_GUI_API neko_gui_bool neko_gui_option_label(struct neko_gui_context *ctx, const char *label, neko_gui_bool active) { return neko_gui_option_text(ctx, label, neko_gui_strlen(label), active); }
NEKO_GUI_API neko_gui_bool neko_gui_radio_label(struct neko_gui_context *ctx, const char *label, neko_gui_bool *active) { return neko_gui_radio_text(ctx, label, neko_gui_strlen(label), active); }

NEKO_GUI_LIB void neko_gui_draw_selectable(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_selectable *style, neko_gui_bool active,
                                           const struct neko_gui_rect *bounds, const struct neko_gui_rect *icon, const struct neko_gui_image *img, enum neko_gui_symbol_type sym, const char *string,
                                           int len, neko_gui_flags align, const struct neko_gui_user_font *font) {
    const struct neko_gui_style_item *background;
    struct neko_gui_text text;
    text.padding = style->padding;

    if (!active) {
        if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
            background = &style->pressed;
            text.text = style->text_pressed;
        } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
            background = &style->hover;
            text.text = style->text_hover;
        } else {
            background = &style->normal;
            text.text = style->text_normal;
        }
    } else {
        if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
            background = &style->pressed_active;
            text.text = style->text_pressed_active;
        } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
            background = &style->hover_active;
            text.text = style->text_hover_active;
        } else {
            background = &style->normal_active;
            text.text = style->text_normal_active;
        }
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            break;
    }
    if (icon) {
        if (img)
            neko_gui_draw_image(out, *icon, img, neko_gui_white);
        else
            neko_gui_draw_symbol(out, sym, *icon, text.background, text.text, 1, font);
    }
    neko_gui_widget_text(out, *bounds, string, len, &text, align, font);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_selectable(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *str, int len, neko_gui_flags align,
                                                  neko_gui_bool *value, const struct neko_gui_style_selectable *style, const struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    int old_value;
    struct neko_gui_rect touch;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(len);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(font);

    if (!state || !out || !str || !len || !value || !style || !font) return 0;
    old_value = *value;

    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;

    if (neko_gui_button_behavior(state, touch, in, NEKO_GUI_BUTTON_DEFAULT)) *value = !(*value);

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_selectable(out, *state, style, *value, &bounds, 0, 0, NEKO_GUI_SYMBOL_NONE, str, len, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != (int)(*value);
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_selectable_image(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *str, int len, neko_gui_flags align,
                                                        neko_gui_bool *value, const struct neko_gui_image *img, const struct neko_gui_style_selectable *style, const struct neko_gui_input *in,
                                                        const struct neko_gui_user_font *font) {
    neko_gui_bool old_value;
    struct neko_gui_rect touch;
    struct neko_gui_rect icon;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(len);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(font);

    if (!state || !out || !str || !len || !value || !style || !font) return 0;
    old_value = *value;

    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;
    if (neko_gui_button_behavior(state, touch, in, NEKO_GUI_BUTTON_DEFAULT)) *value = !(*value);

    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NEKO_GUI_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NEKO_GUI_MAX(icon.x, 0);
    } else
        icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_selectable(out, *state, style, *value, &bounds, &icon, img, NEKO_GUI_SYMBOL_NONE, str, len, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != *value;
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_selectable_symbol(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, const char *str, int len, neko_gui_flags align,
                                                         neko_gui_bool *value, enum neko_gui_symbol_type sym, const struct neko_gui_style_selectable *style, const struct neko_gui_input *in,
                                                         const struct neko_gui_user_font *font) {
    int old_value;
    struct neko_gui_rect touch;
    struct neko_gui_rect icon;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(str);
    NEKO_GUI_ASSERT(len);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(font);

    if (!state || !out || !str || !len || !value || !style || !font) return 0;
    old_value = *value;

    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;
    if (neko_gui_button_behavior(state, touch, in, NEKO_GUI_BUTTON_DEFAULT)) *value = !(*value);

    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NEKO_GUI_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NEKO_GUI_MAX(icon.x, 0);
    } else
        icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_selectable(out, *state, style, *value, &bounds, &icon, 0, sym, str, len, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != (int)(*value);
}

NEKO_GUI_API neko_gui_bool neko_gui_selectable_text(struct neko_gui_context *ctx, const char *str, int len, neko_gui_flags align, neko_gui_bool *value) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    enum neko_gui_widget_layout_states state;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value) return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_selectable(&ctx->last_widget_state, &win->buffer, bounds, str, len, align, value, &style->selectable, in, style->font);
}
NEKO_GUI_API neko_gui_bool neko_gui_selectable_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *str, int len, neko_gui_flags align, neko_gui_bool *value) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    enum neko_gui_widget_layout_states state;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value) return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_selectable_image(&ctx->last_widget_state, &win->buffer, bounds, str, len, align, value, &img, &style->selectable, in, style->font);
}
NEKO_GUI_API neko_gui_bool neko_gui_selectable_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *str, int len, neko_gui_flags align, neko_gui_bool *value) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_input *in;
    const struct neko_gui_style *style;

    enum neko_gui_widget_layout_states state;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(value);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value) return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_selectable_symbol(&ctx->last_widget_state, &win->buffer, bounds, str, len, align, value, sym, &style->selectable, in, style->font);
}
NEKO_GUI_API neko_gui_bool neko_gui_selectable_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *title, neko_gui_flags align, neko_gui_bool *value) {
    return neko_gui_selectable_symbol_text(ctx, sym, title, neko_gui_strlen(title), align, value);
}
NEKO_GUI_API neko_gui_bool neko_gui_select_text(struct neko_gui_context *ctx, const char *str, int len, neko_gui_flags align, neko_gui_bool value) {
    neko_gui_selectable_text(ctx, str, len, align, &value);
    return value;
}
NEKO_GUI_API neko_gui_bool neko_gui_selectable_label(struct neko_gui_context *ctx, const char *str, neko_gui_flags align, neko_gui_bool *value) {
    return neko_gui_selectable_text(ctx, str, neko_gui_strlen(str), align, value);
}
NEKO_GUI_API neko_gui_bool neko_gui_selectable_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *str, neko_gui_flags align, neko_gui_bool *value) {
    return neko_gui_selectable_image_text(ctx, img, str, neko_gui_strlen(str), align, value);
}
NEKO_GUI_API neko_gui_bool neko_gui_select_label(struct neko_gui_context *ctx, const char *str, neko_gui_flags align, neko_gui_bool value) {
    neko_gui_selectable_text(ctx, str, neko_gui_strlen(str), align, &value);
    return value;
}
NEKO_GUI_API neko_gui_bool neko_gui_select_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *str, neko_gui_flags align, neko_gui_bool value) {
    neko_gui_selectable_image_text(ctx, img, str, neko_gui_strlen(str), align, &value);
    return value;
}
NEKO_GUI_API neko_gui_bool neko_gui_select_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *str, int len, neko_gui_flags align, neko_gui_bool value) {
    neko_gui_selectable_image_text(ctx, img, str, len, align, &value);
    return value;
}
NEKO_GUI_API neko_gui_bool neko_gui_select_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *title, int title_len, neko_gui_flags align, neko_gui_bool value) {
    neko_gui_selectable_symbol_text(ctx, sym, title, title_len, align, &value);
    return value;
}
NEKO_GUI_API neko_gui_bool neko_gui_select_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *title, neko_gui_flags align, neko_gui_bool value) {
    return neko_gui_select_symbol_text(ctx, sym, title, neko_gui_strlen(title), align, value);
}

NEKO_GUI_LIB float neko_gui_slider_behavior(neko_gui_flags *state, struct neko_gui_rect *logical_cursor, struct neko_gui_rect *visual_cursor, struct neko_gui_input *in, struct neko_gui_rect bounds,
                                            float slider_min, float slider_max, float slider_value, float slider_step, float slider_steps) {
    int left_mouse_down;
    int left_mouse_click_in_cursor;

    neko_gui_widget_state_reset(state);
    left_mouse_down = in && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
    left_mouse_click_in_cursor = in && neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, *visual_cursor, neko_gui_true);

    if (left_mouse_down && left_mouse_click_in_cursor) {
        float ratio = 0;
        const float d = in->mouse.pos.x - (visual_cursor->x + visual_cursor->w * 0.5f);
        const float pxstep = bounds.w / slider_steps;

        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
        if (NEKO_GUI_ABS(d) >= pxstep) {
            const float steps = (float)((int)(NEKO_GUI_ABS(d) / pxstep));
            slider_value += (d > 0) ? (slider_step * steps) : -(slider_step * steps);
            slider_value = NEKO_GUI_CLAMP(slider_min, slider_value, slider_max);
            ratio = (slider_value - slider_min) / slider_step;
            logical_cursor->x = bounds.x + (logical_cursor->w * ratio);
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.x = logical_cursor->x;
        }
    }

    if (neko_gui_input_is_mouse_hovering_rect(in, bounds)) *state = NEKO_GUI_WIDGET_STATE_HOVERED;
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, bounds))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, bounds))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return slider_value;
}
NEKO_GUI_LIB void neko_gui_draw_slider(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_slider *style, const struct neko_gui_rect *bounds,
                                       const struct neko_gui_rect *visual_cursor, float min, float value, float max) {
    struct neko_gui_rect fill;
    struct neko_gui_rect bar;
    const struct neko_gui_style_item *background;

    struct neko_gui_color bar_color;
    const struct neko_gui_style_item *cursor;

    NEKO_GUI_UNUSED(min);
    NEKO_GUI_UNUSED(max);
    NEKO_GUI_UNUSED(value);

    if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->active;
        bar_color = style->bar_active;
        cursor = &style->cursor_active;
    } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        bar_color = style->bar_hover;
        cursor = &style->cursor_hover;
    } else {
        background = &style->normal;
        bar_color = style->bar_normal;
        cursor = &style->cursor_normal;
    }

    bar.x = bounds->x;
    bar.y = (visual_cursor->y + visual_cursor->h / 2) - bounds->h / 12;
    bar.w = bounds->w;
    bar.h = bounds->h / 6;

    fill.w = (visual_cursor->x + (visual_cursor->w / 2.0f)) - bar.x;
    fill.x = bar.x;
    fill.y = bar.y;
    fill.h = bar.h;

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            neko_gui_stroke_rect(out, *bounds, style->rounding, style->border, style->border_color);
            break;
    }

    neko_gui_fill_rect(out, bar, style->rounding, bar_color);
    neko_gui_fill_rect(out, fill, style->rounding, style->bar_filled);

    if (cursor->type == NEKO_GUI_STYLE_ITEM_IMAGE)
        neko_gui_draw_image(out, *visual_cursor, &cursor->data.image, neko_gui_white);
    else
        neko_gui_fill_circle(out, *visual_cursor, cursor->data.color);
}
NEKO_GUI_LIB float neko_gui_do_slider(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, float min, float val, float max, float step,
                                      const struct neko_gui_style_slider *style, struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    float slider_range;
    float slider_min;
    float slider_max;
    float slider_value;
    float slider_steps;
    float cursor_offset;

    struct neko_gui_rect visual_cursor;
    struct neko_gui_rect logical_cursor;

    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    if (!out || !style) return 0;

    bounds.x = bounds.x + style->padding.x;
    bounds.y = bounds.y + style->padding.y;
    bounds.h = NEKO_GUI_MAX(bounds.h, 2 * style->padding.y);
    bounds.w = NEKO_GUI_MAX(bounds.w, 2 * style->padding.x + style->cursor_size.x);
    bounds.w -= 2 * style->padding.x;
    bounds.h -= 2 * style->padding.y;

    if (style->show_buttons) {
        neko_gui_flags ws;
        struct neko_gui_rect button;
        button.y = bounds.y;
        button.w = bounds.h;
        button.h = bounds.h;

        button.x = bounds.x;
        if (neko_gui_do_button_symbol(&ws, out, button, style->dec_symbol, NEKO_GUI_BUTTON_DEFAULT, &style->dec_button, in, font)) val -= step;

        button.x = (bounds.x + bounds.w) - button.w;
        if (neko_gui_do_button_symbol(&ws, out, button, style->inc_symbol, NEKO_GUI_BUTTON_DEFAULT, &style->inc_button, in, font)) val += step;

        bounds.x = bounds.x + button.w + style->spacing.x;
        bounds.w = bounds.w - (2 * button.w + 2 * style->spacing.x);
    }

    bounds.x += style->cursor_size.x * 0.5f;
    bounds.w -= style->cursor_size.x;

    slider_max = NEKO_GUI_MAX(min, max);
    slider_min = NEKO_GUI_MIN(min, max);
    slider_value = NEKO_GUI_CLAMP(slider_min, val, slider_max);
    slider_range = slider_max - slider_min;
    slider_steps = slider_range / step;
    cursor_offset = (slider_value - slider_min) / step;

    logical_cursor.h = bounds.h;
    logical_cursor.w = bounds.w / slider_steps;
    logical_cursor.x = bounds.x + (logical_cursor.w * cursor_offset);
    logical_cursor.y = bounds.y;

    visual_cursor.h = style->cursor_size.y;
    visual_cursor.w = style->cursor_size.x;
    visual_cursor.y = (bounds.y + bounds.h * 0.5f) - visual_cursor.h * 0.5f;
    visual_cursor.x = logical_cursor.x - visual_cursor.w * 0.5f;

    slider_value = neko_gui_slider_behavior(state, &logical_cursor, &visual_cursor, in, bounds, slider_min, slider_max, slider_value, step, slider_steps);
    visual_cursor.x = logical_cursor.x - visual_cursor.w * 0.5f;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_slider(out, *state, style, &bounds, &visual_cursor, slider_min, slider_value, slider_max);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return slider_value;
}
NEKO_GUI_API neko_gui_bool neko_gui_slider_float(struct neko_gui_context *ctx, float min_value, float *value, float max_value, float value_step) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    struct neko_gui_input *in;
    const struct neko_gui_style *style;

    int ret = 0;
    float old_value;
    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    NEKO_GUI_ASSERT(value);
    if (!ctx || !ctx->current || !ctx->current->layout || !value) return ret;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = neko_gui_widget(&bounds, ctx);
    if (!state) return ret;
    in = (layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;

    old_value = *value;
    *value = neko_gui_do_slider(&ctx->last_widget_state, &win->buffer, bounds, min_value, old_value, max_value, value_step, &style->slider, in, style->font);
    return (old_value > *value || old_value < *value);
}
NEKO_GUI_API float neko_gui_slide_float(struct neko_gui_context *ctx, float min, float val, float max, float step) {
    neko_gui_slider_float(ctx, min, &val, max, step);
    return val;
}
NEKO_GUI_API int neko_gui_slide_int(struct neko_gui_context *ctx, int min, int val, int max, int step) {
    float value = (float)val;
    neko_gui_slider_float(ctx, (float)min, &value, (float)max, (float)step);
    return (int)value;
}
NEKO_GUI_API neko_gui_bool neko_gui_slider_int(struct neko_gui_context *ctx, int min, int *val, int max, int step) {
    int ret;
    float value = (float)*val;
    ret = neko_gui_slider_float(ctx, (float)min, &value, (float)max, (float)step);
    *val = (int)value;
    return ret;
}

NEKO_GUI_LIB neko_gui_size neko_gui_progress_behavior(neko_gui_flags *state, struct neko_gui_input *in, struct neko_gui_rect r, struct neko_gui_rect cursor, neko_gui_size max, neko_gui_size value,
                                                      neko_gui_bool modifiable) {
    int left_mouse_down = 0;
    int left_mouse_click_in_cursor = 0;

    neko_gui_widget_state_reset(state);
    if (!in || !modifiable) return value;
    left_mouse_down = in && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
    left_mouse_click_in_cursor = in && neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, cursor, neko_gui_true);
    if (neko_gui_input_is_mouse_hovering_rect(in, r)) *state = NEKO_GUI_WIDGET_STATE_HOVERED;

    if (in && left_mouse_down && left_mouse_click_in_cursor) {
        if (left_mouse_down && left_mouse_click_in_cursor) {
            float ratio = NEKO_GUI_MAX(0, (float)(in->mouse.pos.x - cursor.x)) / (float)cursor.w;
            value = (neko_gui_size)NEKO_GUI_CLAMP(0, (float)max * ratio, (float)max);
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.x = cursor.x + cursor.w / 2.0f;
            *state |= NEKO_GUI_WIDGET_STATE_ACTIVE;
        }
    }

    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, r))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, r))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return value;
}
NEKO_GUI_LIB void neko_gui_draw_progress(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_progress *style, const struct neko_gui_rect *bounds,
                                         const struct neko_gui_rect *scursor, neko_gui_size value, neko_gui_size max) {
    const struct neko_gui_style_item *background;
    const struct neko_gui_style_item *cursor;

    NEKO_GUI_UNUSED(max);
    NEKO_GUI_UNUSED(value);

    if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->active;
        cursor = &style->cursor_active;
    } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            neko_gui_stroke_rect(out, *bounds, style->rounding, style->border, style->border_color);
            break;
    }

    switch (cursor->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *scursor, &cursor->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *scursor, &cursor->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *scursor, style->rounding, cursor->data.color);
            neko_gui_stroke_rect(out, *scursor, style->rounding, style->border, style->border_color);
            break;
    }
}
NEKO_GUI_LIB neko_gui_size neko_gui_do_progress(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, neko_gui_size value, neko_gui_size max,
                                                neko_gui_bool modifiable, const struct neko_gui_style_progress *style, struct neko_gui_input *in) {
    float prog_scale;
    neko_gui_size prog_value;
    struct neko_gui_rect cursor;

    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(out);
    if (!out || !style) return 0;

    cursor.w = NEKO_GUI_MAX(bounds.w, 2 * style->padding.x + 2 * style->border);
    cursor.h = NEKO_GUI_MAX(bounds.h, 2 * style->padding.y + 2 * style->border);
    cursor = neko_gui_pad_rect(bounds, neko_gui_vec2(style->padding.x + style->border, style->padding.y + style->border));
    prog_scale = (float)value / (float)max;

    prog_value = NEKO_GUI_MIN(value, max);
    prog_value = neko_gui_progress_behavior(state, in, bounds, cursor, max, prog_value, modifiable);
    cursor.w = cursor.w * prog_scale;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_progress(out, *state, style, &bounds, &cursor, value, max);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return prog_value;
}
NEKO_GUI_API neko_gui_bool neko_gui_progress(struct neko_gui_context *ctx, neko_gui_size *cur, neko_gui_size max, neko_gui_bool is_modifyable) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *style;
    struct neko_gui_input *in;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states state;
    neko_gui_size old_value;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(cur);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !cur) return 0;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;
    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;

    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    old_value = *cur;
    *cur = neko_gui_do_progress(&ctx->last_widget_state, &win->buffer, bounds, *cur, max, is_modifyable, &style->progress, in);
    return (*cur != old_value);
}
NEKO_GUI_API neko_gui_size neko_gui_prog(struct neko_gui_context *ctx, neko_gui_size cur, neko_gui_size max, neko_gui_bool modifyable) {
    neko_gui_progress(ctx, &cur, max, modifyable);
    return cur;
}

NEKO_GUI_LIB float neko_gui_scrollbar_behavior(neko_gui_flags *state, struct neko_gui_input *in, int has_scrolling, const struct neko_gui_rect *scroll, const struct neko_gui_rect *cursor,
                                               const struct neko_gui_rect *empty0, const struct neko_gui_rect *empty1, float scroll_offset, float target, float scroll_step,
                                               enum neko_gui_orientation o) {
    neko_gui_flags ws = 0;
    int left_mouse_down;
    unsigned int left_mouse_clicked;
    int left_mouse_click_in_cursor;
    float scroll_delta;

    neko_gui_widget_state_reset(state);
    if (!in) return scroll_offset;

    left_mouse_down = in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
    left_mouse_clicked = in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked;
    left_mouse_click_in_cursor = neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, *cursor, neko_gui_true);
    if (neko_gui_input_is_mouse_hovering_rect(in, *scroll)) *state = NEKO_GUI_WIDGET_STATE_HOVERED;

    scroll_delta = (o == NEKO_GUI_VERTICAL) ? in->mouse.scroll_delta.y : in->mouse.scroll_delta.x;
    if (left_mouse_down && left_mouse_click_in_cursor && !left_mouse_clicked) {

        float pixel, delta;
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
        if (o == NEKO_GUI_VERTICAL) {
            float cursor_y;
            pixel = in->mouse.delta.y;
            delta = (pixel / scroll->h) * target;
            scroll_offset = NEKO_GUI_CLAMP(0, scroll_offset + delta, target - scroll->h);
            cursor_y = scroll->y + ((scroll_offset / target) * scroll->h);
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.y = cursor_y + cursor->h / 2.0f;
        } else {
            float cursor_x;
            pixel = in->mouse.delta.x;
            delta = (pixel / scroll->w) * target;
            scroll_offset = NEKO_GUI_CLAMP(0, scroll_offset + delta, target - scroll->w);
            cursor_x = scroll->x + ((scroll_offset / target) * scroll->w);
            in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked_pos.x = cursor_x + cursor->w / 2.0f;
        }
    } else if ((neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_SCROLL_UP) && o == NEKO_GUI_VERTICAL && has_scrolling) || neko_gui_button_behavior(&ws, *empty0, in, NEKO_GUI_BUTTON_DEFAULT)) {

        if (o == NEKO_GUI_VERTICAL)
            scroll_offset = NEKO_GUI_MAX(0, scroll_offset - scroll->h);
        else
            scroll_offset = NEKO_GUI_MAX(0, scroll_offset - scroll->w);
    } else if ((neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_SCROLL_DOWN) && o == NEKO_GUI_VERTICAL && has_scrolling) || neko_gui_button_behavior(&ws, *empty1, in, NEKO_GUI_BUTTON_DEFAULT)) {

        if (o == NEKO_GUI_VERTICAL)
            scroll_offset = NEKO_GUI_MIN(scroll_offset + scroll->h, target - scroll->h);
        else
            scroll_offset = NEKO_GUI_MIN(scroll_offset + scroll->w, target - scroll->w);
    } else if (has_scrolling) {
        if ((scroll_delta < 0 || (scroll_delta > 0))) {

            scroll_offset = scroll_offset + scroll_step * (-scroll_delta);
            if (o == NEKO_GUI_VERTICAL)
                scroll_offset = NEKO_GUI_CLAMP(0, scroll_offset, target - scroll->h);
            else
                scroll_offset = NEKO_GUI_CLAMP(0, scroll_offset, target - scroll->w);
        } else if (neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_SCROLL_START)) {

            if (o == NEKO_GUI_VERTICAL) scroll_offset = 0;
        } else if (neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_SCROLL_END)) {

            if (o == NEKO_GUI_VERTICAL) scroll_offset = target - scroll->h;
        }
    }
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, *scroll))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, *scroll))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return scroll_offset;
}
NEKO_GUI_LIB void neko_gui_draw_scrollbar(struct neko_gui_command_buffer *out, neko_gui_flags state, const struct neko_gui_style_scrollbar *style, const struct neko_gui_rect *bounds,
                                          const struct neko_gui_rect *scroll) {
    const struct neko_gui_style_item *background;
    const struct neko_gui_style_item *cursor;

    if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->active;
        cursor = &style->cursor_active;
    } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            neko_gui_stroke_rect(out, *bounds, style->rounding, style->border, style->border_color);
            break;
    }

    switch (cursor->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(out, *scroll, &cursor->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(out, *scroll, &cursor->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(out, *scroll, style->rounding_cursor, cursor->data.color);
            neko_gui_stroke_rect(out, *scroll, style->rounding_cursor, style->border_cursor, style->cursor_border_color);
            break;
    }
}
NEKO_GUI_LIB float neko_gui_do_scrollbarv(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect scroll, int has_scrolling, float offset, float target, float step,
                                          float button_pixel_inc, const struct neko_gui_style_scrollbar *style, struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    struct neko_gui_rect empty_north;
    struct neko_gui_rect empty_south;
    struct neko_gui_rect cursor;

    float scroll_step;
    float scroll_offset;
    float scroll_off;
    float scroll_ratio;

    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(style);
    NEKO_GUI_ASSERT(state);
    if (!out || !style) return 0;

    scroll.w = NEKO_GUI_MAX(scroll.w, 1);
    scroll.h = NEKO_GUI_MAX(scroll.h, 0);
    if (target <= scroll.h) return 0;

    if (style->show_buttons) {
        neko_gui_flags ws;
        float scroll_h;
        struct neko_gui_rect button;

        button.x = scroll.x;
        button.w = scroll.w;
        button.h = scroll.w;

        scroll_h = NEKO_GUI_MAX(scroll.h - 2 * button.h, 0);
        scroll_step = NEKO_GUI_MIN(step, button_pixel_inc);

        button.y = scroll.y;
        if (neko_gui_do_button_symbol(&ws, out, button, style->dec_symbol, NEKO_GUI_BUTTON_REPEATER, &style->dec_button, in, font)) offset = offset - scroll_step;

        button.y = scroll.y + scroll.h - button.h;
        if (neko_gui_do_button_symbol(&ws, out, button, style->inc_symbol, NEKO_GUI_BUTTON_REPEATER, &style->inc_button, in, font)) offset = offset + scroll_step;

        scroll.y = scroll.y + button.h;
        scroll.h = scroll_h;
    }

    scroll_step = NEKO_GUI_MIN(step, scroll.h);
    scroll_offset = NEKO_GUI_CLAMP(0, offset, target - scroll.h);
    scroll_ratio = scroll.h / target;
    scroll_off = scroll_offset / target;

    cursor.h = NEKO_GUI_MAX((scroll_ratio * scroll.h) - (2 * style->border + 2 * style->padding.y), 0);
    cursor.y = scroll.y + (scroll_off * scroll.h) + style->border + style->padding.y;
    cursor.w = scroll.w - (2 * style->border + 2 * style->padding.x);
    cursor.x = scroll.x + style->border + style->padding.x;

    empty_north.x = scroll.x;
    empty_north.y = scroll.y;
    empty_north.w = scroll.w;
    empty_north.h = NEKO_GUI_MAX(cursor.y - scroll.y, 0);

    empty_south.x = scroll.x;
    empty_south.y = cursor.y + cursor.h;
    empty_south.w = scroll.w;
    empty_south.h = NEKO_GUI_MAX((scroll.y + scroll.h) - (cursor.y + cursor.h), 0);

    scroll_offset = neko_gui_scrollbar_behavior(state, in, has_scrolling, &scroll, &cursor, &empty_north, &empty_south, scroll_offset, target, scroll_step, NEKO_GUI_VERTICAL);
    scroll_off = scroll_offset / target;
    cursor.y = scroll.y + (scroll_off * scroll.h) + style->border_cursor + style->padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_scrollbar(out, *state, style, &scroll, &cursor);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return scroll_offset;
}
NEKO_GUI_LIB float neko_gui_do_scrollbarh(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect scroll, int has_scrolling, float offset, float target, float step,
                                          float button_pixel_inc, const struct neko_gui_style_scrollbar *style, struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    struct neko_gui_rect cursor;
    struct neko_gui_rect empty_west;
    struct neko_gui_rect empty_east;

    float scroll_step;
    float scroll_offset;
    float scroll_off;
    float scroll_ratio;

    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(style);
    if (!out || !style) return 0;

    scroll.h = NEKO_GUI_MAX(scroll.h, 1);
    scroll.w = NEKO_GUI_MAX(scroll.w, 2 * scroll.h);
    if (target <= scroll.w) return 0;

    if (style->show_buttons) {
        neko_gui_flags ws;
        float scroll_w;
        struct neko_gui_rect button;
        button.y = scroll.y;
        button.w = scroll.h;
        button.h = scroll.h;

        scroll_w = scroll.w - 2 * button.w;
        scroll_step = NEKO_GUI_MIN(step, button_pixel_inc);

        button.x = scroll.x;
        if (neko_gui_do_button_symbol(&ws, out, button, style->dec_symbol, NEKO_GUI_BUTTON_REPEATER, &style->dec_button, in, font)) offset = offset - scroll_step;

        button.x = scroll.x + scroll.w - button.w;
        if (neko_gui_do_button_symbol(&ws, out, button, style->inc_symbol, NEKO_GUI_BUTTON_REPEATER, &style->inc_button, in, font)) offset = offset + scroll_step;

        scroll.x = scroll.x + button.w;
        scroll.w = scroll_w;
    }

    scroll_step = NEKO_GUI_MIN(step, scroll.w);
    scroll_offset = NEKO_GUI_CLAMP(0, offset, target - scroll.w);
    scroll_ratio = scroll.w / target;
    scroll_off = scroll_offset / target;

    cursor.w = (scroll_ratio * scroll.w) - (2 * style->border + 2 * style->padding.x);
    cursor.x = scroll.x + (scroll_off * scroll.w) + style->border + style->padding.x;
    cursor.h = scroll.h - (2 * style->border + 2 * style->padding.y);
    cursor.y = scroll.y + style->border + style->padding.y;

    empty_west.x = scroll.x;
    empty_west.y = scroll.y;
    empty_west.w = cursor.x - scroll.x;
    empty_west.h = scroll.h;

    empty_east.x = cursor.x + cursor.w;
    empty_east.y = scroll.y;
    empty_east.w = (scroll.x + scroll.w) - (cursor.x + cursor.w);
    empty_east.h = scroll.h;

    scroll_offset = neko_gui_scrollbar_behavior(state, in, has_scrolling, &scroll, &cursor, &empty_west, &empty_east, scroll_offset, target, scroll_step, NEKO_GUI_HORIZONTAL);
    scroll_off = scroll_offset / target;
    cursor.x = scroll.x + (scroll_off * scroll.w);

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_scrollbar(out, *state, style, &scroll, &cursor);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return scroll_offset;
}

struct neko_gui_text_find {
    float x, y;
    float height;
    int first_char, length;
    int prev_first;
};

struct neko_gui_text_edit_row {
    float x0, x1;

    float baseline_y_delta;

    float ymin, ymax;

    int num_chars;
};

NEKO_GUI_INTERN void neko_gui_textedit_makeundo_delete(struct neko_gui_text_edit *, int, int);
NEKO_GUI_INTERN void neko_gui_textedit_makeundo_insert(struct neko_gui_text_edit *, int, int);
NEKO_GUI_INTERN void neko_gui_textedit_makeundo_replace(struct neko_gui_text_edit *, int, int, int);
#define NEKO_GUI_TEXT_HAS_SELECTION(s) ((s)->select_start != (s)->select_end)

NEKO_GUI_INTERN float neko_gui_textedit_get_width(const struct neko_gui_text_edit *edit, int line_start, int char_id, const struct neko_gui_user_font *font) {
    int len = 0;
    neko_gui_rune unicode = 0;
    const char *str = neko_gui_str_at_const(&edit->string, line_start + char_id, &unicode, &len);
    return font->width(font->userdata, font->height, str, len);
}
NEKO_GUI_INTERN void neko_gui_textedit_layout_row(struct neko_gui_text_edit_row *r, struct neko_gui_text_edit *edit, int line_start_id, float row_height, const struct neko_gui_user_font *font) {
    int l;
    int glyphs = 0;
    neko_gui_rune unicode;
    const char *remaining;
    int len = neko_gui_str_len_char(&edit->string);
    const char *end = neko_gui_str_get_const(&edit->string) + len;
    const char *text = neko_gui_str_at_const(&edit->string, line_start_id, &unicode, &l);
    const struct neko_gui_vec2 size = neko_gui_text_calculate_text_bounds(font, text, (int)(end - text), row_height, &remaining, 0, &glyphs, NEKO_GUI_STOP_ON_NEW_LINE);

    r->x0 = 0.0f;
    r->x1 = size.x;
    r->baseline_y_delta = size.y;
    r->ymin = 0.0f;
    r->ymax = size.y;
    r->num_chars = glyphs;
}
NEKO_GUI_INTERN int neko_gui_textedit_locate_coord(struct neko_gui_text_edit *edit, float x, float y, const struct neko_gui_user_font *font, float row_height) {
    struct neko_gui_text_edit_row r;
    int n = edit->string.len;
    float base_y = 0, prev_x;
    int i = 0, k;

    r.x0 = r.x1 = 0;
    r.ymin = r.ymax = 0;
    r.num_chars = 0;

    while (i < n) {
        neko_gui_textedit_layout_row(&r, edit, i, row_height, font);
        if (r.num_chars <= 0) return n;

        if (i == 0 && y < base_y + r.ymin) return 0;

        if (y < base_y + r.ymax) break;

        i += r.num_chars;
        base_y += r.baseline_y_delta;
    }

    if (i >= n) return n;

    if (x < r.x0) return i;

    if (x < r.x1) {

        k = i;
        prev_x = r.x0;
        for (i = 0; i < r.num_chars; ++i) {
            float w = neko_gui_textedit_get_width(edit, k, i, font);
            if (x < prev_x + w) {
                if (x < prev_x + w / 2)
                    return k + i;
                else
                    return k + i + 1;
            }
            prev_x += w;
        }
    }

    if (neko_gui_str_rune_at(&edit->string, i + r.num_chars - 1) == '\n')
        return i + r.num_chars - 1;
    else
        return i + r.num_chars;
}
NEKO_GUI_LIB void neko_gui_textedit_click(struct neko_gui_text_edit *state, float x, float y, const struct neko_gui_user_font *font, float row_height) {

    state->cursor = neko_gui_textedit_locate_coord(state, x, y, font, row_height);
    state->select_start = state->cursor;
    state->select_end = state->cursor;
    state->has_preferred_x = 0;
}
NEKO_GUI_LIB void neko_gui_textedit_drag(struct neko_gui_text_edit *state, float x, float y, const struct neko_gui_user_font *font, float row_height) {

    int p = neko_gui_textedit_locate_coord(state, x, y, font, row_height);
    if (state->select_start == state->select_end) state->select_start = state->cursor;
    state->cursor = state->select_end = p;
}
NEKO_GUI_INTERN void neko_gui_textedit_find_charpos(struct neko_gui_text_find *find, struct neko_gui_text_edit *state, int n, int single_line, const struct neko_gui_user_font *font,
                                                    float row_height) {

    struct neko_gui_text_edit_row r;
    int prev_start = 0;
    int z = state->string.len;
    int i = 0, first;

    neko_gui_zero_struct(r);
    if (n == z) {

        neko_gui_textedit_layout_row(&r, state, 0, row_height, font);
        if (single_line) {
            find->first_char = 0;
            find->length = z;
        } else {
            while (i < z) {
                prev_start = i;
                i += r.num_chars;
                neko_gui_textedit_layout_row(&r, state, i, row_height, font);
            }

            find->first_char = i;
            find->length = r.num_chars;
        }
        find->x = r.x1;
        find->y = r.ymin;
        find->height = r.ymax - r.ymin;
        find->prev_first = prev_start;
        return;
    }

    find->y = 0;

    for (;;) {
        neko_gui_textedit_layout_row(&r, state, i, row_height, font);
        if (n < i + r.num_chars) break;
        prev_start = i;
        i += r.num_chars;
        find->y += r.baseline_y_delta;
    }

    find->first_char = first = i;
    find->length = r.num_chars;
    find->height = r.ymax - r.ymin;
    find->prev_first = prev_start;

    find->x = r.x0;
    for (i = 0; first + i < n; ++i) find->x += neko_gui_textedit_get_width(state, first, i, font);
}
NEKO_GUI_INTERN void neko_gui_textedit_clamp(struct neko_gui_text_edit *state) {

    int n = state->string.len;
    if (NEKO_GUI_TEXT_HAS_SELECTION(state)) {
        if (state->select_start > n) state->select_start = n;
        if (state->select_end > n) state->select_end = n;

        if (state->select_start == state->select_end) state->cursor = state->select_start;
    }
    if (state->cursor > n) state->cursor = n;
}
NEKO_GUI_API void neko_gui_textedit_delete(struct neko_gui_text_edit *state, int where, int len) {

    neko_gui_textedit_makeundo_delete(state, where, len);
    neko_gui_str_delete_runes(&state->string, where, len);
    state->has_preferred_x = 0;
}
NEKO_GUI_API void neko_gui_textedit_delete_selection(struct neko_gui_text_edit *state) {

    neko_gui_textedit_clamp(state);
    if (NEKO_GUI_TEXT_HAS_SELECTION(state)) {
        if (state->select_start < state->select_end) {
            neko_gui_textedit_delete(state, state->select_start, state->select_end - state->select_start);
            state->select_end = state->cursor = state->select_start;
        } else {
            neko_gui_textedit_delete(state, state->select_end, state->select_start - state->select_end);
            state->select_start = state->cursor = state->select_end;
        }
        state->has_preferred_x = 0;
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_sortselection(struct neko_gui_text_edit *state) {

    if (state->select_end < state->select_start) {
        int temp = state->select_end;
        state->select_end = state->select_start;
        state->select_start = temp;
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_move_to_first(struct neko_gui_text_edit *state) {

    if (NEKO_GUI_TEXT_HAS_SELECTION(state)) {
        neko_gui_textedit_sortselection(state);
        state->cursor = state->select_start;
        state->select_end = state->select_start;
        state->has_preferred_x = 0;
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_move_to_last(struct neko_gui_text_edit *state) {

    if (NEKO_GUI_TEXT_HAS_SELECTION(state)) {
        neko_gui_textedit_sortselection(state);
        neko_gui_textedit_clamp(state);
        state->cursor = state->select_end;
        state->select_start = state->select_end;
        state->has_preferred_x = 0;
    }
}
NEKO_GUI_INTERN int neko_gui_is_word_boundary(struct neko_gui_text_edit *state, int idx) {
    int len;
    neko_gui_rune c;
    if (idx <= 0) return 1;
    if (!neko_gui_str_at_rune(&state->string, idx, &c, &len)) return 1;
    return (c == ' ' || c == '\t' || c == 0x3000 || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == '|');
}
NEKO_GUI_INTERN int neko_gui_textedit_move_to_word_previous(struct neko_gui_text_edit *state) {
    int c = state->cursor - 1;
    while (c >= 0 && !neko_gui_is_word_boundary(state, c)) --c;

    if (c < 0) c = 0;

    return c;
}
NEKO_GUI_INTERN int neko_gui_textedit_move_to_word_next(struct neko_gui_text_edit *state) {
    const int len = state->string.len;
    int c = state->cursor + 1;
    while (c < len && !neko_gui_is_word_boundary(state, c)) ++c;

    if (c > len) c = len;

    return c;
}
NEKO_GUI_INTERN void neko_gui_textedit_prep_selection_at_cursor(struct neko_gui_text_edit *state) {

    if (!NEKO_GUI_TEXT_HAS_SELECTION(state))
        state->select_start = state->select_end = state->cursor;
    else
        state->cursor = state->select_end;
}
NEKO_GUI_API neko_gui_bool neko_gui_textedit_cut(struct neko_gui_text_edit *state) {

    if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) return 0;
    if (NEKO_GUI_TEXT_HAS_SELECTION(state)) {
        neko_gui_textedit_delete_selection(state);
        state->has_preferred_x = 0;
        return 1;
    }
    return 0;
}
NEKO_GUI_API neko_gui_bool neko_gui_textedit_paste(struct neko_gui_text_edit *state, char const *ctext, int len) {

    int glyphs;
    const char *text = (const char *)ctext;
    if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) return 0;

    neko_gui_textedit_clamp(state);
    neko_gui_textedit_delete_selection(state);

    glyphs = neko_gui_utf_len(ctext, len);
    if (neko_gui_str_insert_text_char(&state->string, state->cursor, text, len)) {
        neko_gui_textedit_makeundo_insert(state, state->cursor, glyphs);
        state->cursor += len;
        state->has_preferred_x = 0;
        return 1;
    }

    if (state->undo.undo_point) --state->undo.undo_point;
    return 0;
}
NEKO_GUI_API void neko_gui_textedit_text(struct neko_gui_text_edit *state, const char *text, int total_len) {
    neko_gui_rune unicode;
    int glyph_len;
    int text_len = 0;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(text);
    if (!text || !total_len || state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) return;

    glyph_len = neko_gui_utf_decode(text, &unicode, total_len);
    while ((text_len < total_len) && glyph_len) {

        if (unicode == 127) goto next;

        if (unicode == '\n' && state->single_line) goto next;

        if (state->filter && !state->filter(state, unicode)) goto next;

        if (!NEKO_GUI_TEXT_HAS_SELECTION(state) && state->cursor < state->string.len) {
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_REPLACE) {
                neko_gui_textedit_makeundo_replace(state, state->cursor, 1, 1);
                neko_gui_str_delete_runes(&state->string, state->cursor, 1);
            }
            if (neko_gui_str_insert_text_utf8(&state->string, state->cursor, text + text_len, 1)) {
                ++state->cursor;
                state->has_preferred_x = 0;
            }
        } else {
            neko_gui_textedit_delete_selection(state);
            if (neko_gui_str_insert_text_utf8(&state->string, state->cursor, text + text_len, 1)) {
                neko_gui_textedit_makeundo_insert(state, state->cursor, 1);
                state->cursor = NEKO_GUI_MIN(state->cursor + 1, state->string.len);
                state->has_preferred_x = 0;
            }
        }
    next:
        text_len += glyph_len;
        glyph_len = neko_gui_utf_decode(text + text_len, &unicode, total_len - text_len);
    }
}
NEKO_GUI_LIB void neko_gui_textedit_key(struct neko_gui_text_edit *state, enum neko_gui_keys key, int shift_mod, const struct neko_gui_user_font *font, float row_height) {
retry:
    switch (key) {
        case NEKO_GUI_KEY_NONE:
        case NEKO_GUI_KEY_CTRL:
        case NEKO_GUI_KEY_ENTER:
        case NEKO_GUI_KEY_SHIFT:
        case NEKO_GUI_KEY_TAB:
        case NEKO_GUI_KEY_COPY:
        case NEKO_GUI_KEY_CUT:
        case NEKO_GUI_KEY_PASTE:
        case NEKO_GUI_KEY_MAX:
        default:
            break;
        case NEKO_GUI_KEY_TEXT_UNDO:
            neko_gui_textedit_undo(state);
            state->has_preferred_x = 0;
            break;

        case NEKO_GUI_KEY_TEXT_REDO:
            neko_gui_textedit_redo(state);
            state->has_preferred_x = 0;
            break;

        case NEKO_GUI_KEY_TEXT_SELECT_ALL:
            neko_gui_textedit_select_all(state);
            state->has_preferred_x = 0;
            break;

        case NEKO_GUI_KEY_TEXT_INSERT_MODE:
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) state->mode = NEKO_GUI_TEXT_EDIT_MODE_INSERT;
            break;
        case NEKO_GUI_KEY_TEXT_REPLACE_MODE:
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) state->mode = NEKO_GUI_TEXT_EDIT_MODE_REPLACE;
            break;
        case NEKO_GUI_KEY_TEXT_RESET_MODE:
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_INSERT || state->mode == NEKO_GUI_TEXT_EDIT_MODE_REPLACE) state->mode = NEKO_GUI_TEXT_EDIT_MODE_VIEW;
            break;

        case NEKO_GUI_KEY_LEFT:
            if (shift_mod) {
                neko_gui_textedit_clamp(state);
                neko_gui_textedit_prep_selection_at_cursor(state);

                if (state->select_end > 0) --state->select_end;
                state->cursor = state->select_end;
                state->has_preferred_x = 0;
            } else {

                if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                    neko_gui_textedit_move_to_first(state);
                else if (state->cursor > 0)
                    --state->cursor;
                state->has_preferred_x = 0;
            }
            break;

        case NEKO_GUI_KEY_RIGHT:
            if (shift_mod) {
                neko_gui_textedit_prep_selection_at_cursor(state);

                ++state->select_end;
                neko_gui_textedit_clamp(state);
                state->cursor = state->select_end;
                state->has_preferred_x = 0;
            } else {

                if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                    neko_gui_textedit_move_to_last(state);
                else
                    ++state->cursor;
                neko_gui_textedit_clamp(state);
                state->has_preferred_x = 0;
            }
            break;

        case NEKO_GUI_KEY_TEXT_WORD_LEFT:
            if (shift_mod) {
                if (!NEKO_GUI_TEXT_HAS_SELECTION(state)) neko_gui_textedit_prep_selection_at_cursor(state);
                state->cursor = neko_gui_textedit_move_to_word_previous(state);
                state->select_end = state->cursor;
                neko_gui_textedit_clamp(state);
            } else {
                if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                    neko_gui_textedit_move_to_first(state);
                else {
                    state->cursor = neko_gui_textedit_move_to_word_previous(state);
                    neko_gui_textedit_clamp(state);
                }
            }
            break;

        case NEKO_GUI_KEY_TEXT_WORD_RIGHT:
            if (shift_mod) {
                if (!NEKO_GUI_TEXT_HAS_SELECTION(state)) neko_gui_textedit_prep_selection_at_cursor(state);
                state->cursor = neko_gui_textedit_move_to_word_next(state);
                state->select_end = state->cursor;
                neko_gui_textedit_clamp(state);
            } else {
                if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                    neko_gui_textedit_move_to_last(state);
                else {
                    state->cursor = neko_gui_textedit_move_to_word_next(state);
                    neko_gui_textedit_clamp(state);
                }
            }
            break;

        case NEKO_GUI_KEY_DOWN: {
            struct neko_gui_text_find find;
            struct neko_gui_text_edit_row row;
            int i, sel = shift_mod;

            if (state->single_line) {

                key = NEKO_GUI_KEY_RIGHT;
                goto retry;
            }

            if (sel)
                neko_gui_textedit_prep_selection_at_cursor(state);
            else if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                neko_gui_textedit_move_to_last(state);

            neko_gui_textedit_clamp(state);
            neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);

            if (find.length) {
                float x;
                float goal_x = state->has_preferred_x ? state->preferred_x : find.x;
                int start = find.first_char + find.length;

                state->cursor = start;
                neko_gui_textedit_layout_row(&row, state, state->cursor, row_height, font);
                x = row.x0;

                for (i = 0; i < row.num_chars && x < row.x1; ++i) {
                    float dx = neko_gui_textedit_get_width(state, start, i, font);
                    x += dx;
                    if (x > goal_x) break;
                    ++state->cursor;
                }
                neko_gui_textedit_clamp(state);

                state->has_preferred_x = 1;
                state->preferred_x = goal_x;
                if (sel) state->select_end = state->cursor;
            }
        } break;

        case NEKO_GUI_KEY_UP: {
            struct neko_gui_text_find find;
            struct neko_gui_text_edit_row row;
            int i, sel = shift_mod;

            if (state->single_line) {

                key = NEKO_GUI_KEY_LEFT;
                goto retry;
            }

            if (sel)
                neko_gui_textedit_prep_selection_at_cursor(state);
            else if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                neko_gui_textedit_move_to_first(state);

            neko_gui_textedit_clamp(state);
            neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);

            if (find.prev_first != find.first_char) {

                float x;
                float goal_x = state->has_preferred_x ? state->preferred_x : find.x;

                state->cursor = find.prev_first;
                neko_gui_textedit_layout_row(&row, state, state->cursor, row_height, font);
                x = row.x0;

                for (i = 0; i < row.num_chars && x < row.x1; ++i) {
                    float dx = neko_gui_textedit_get_width(state, find.prev_first, i, font);
                    x += dx;
                    if (x > goal_x) break;
                    ++state->cursor;
                }
                neko_gui_textedit_clamp(state);

                state->has_preferred_x = 1;
                state->preferred_x = goal_x;
                if (sel) state->select_end = state->cursor;
            }
        } break;

        case NEKO_GUI_KEY_DEL:
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) break;
            if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                neko_gui_textedit_delete_selection(state);
            else {
                int n = state->string.len;
                if (state->cursor < n) neko_gui_textedit_delete(state, state->cursor, 1);
            }
            state->has_preferred_x = 0;
            break;

        case NEKO_GUI_KEY_BACKSPACE:
            if (state->mode == NEKO_GUI_TEXT_EDIT_MODE_VIEW) break;
            if (NEKO_GUI_TEXT_HAS_SELECTION(state))
                neko_gui_textedit_delete_selection(state);
            else {
                neko_gui_textedit_clamp(state);
                if (state->cursor > 0) {
                    neko_gui_textedit_delete(state, state->cursor - 1, 1);
                    --state->cursor;
                }
            }
            state->has_preferred_x = 0;
            break;

        case NEKO_GUI_KEY_TEXT_START:
            if (shift_mod) {
                neko_gui_textedit_prep_selection_at_cursor(state);
                state->cursor = state->select_end = 0;
                state->has_preferred_x = 0;
            } else {
                state->cursor = state->select_start = state->select_end = 0;
                state->has_preferred_x = 0;
            }
            break;

        case NEKO_GUI_KEY_TEXT_END:
            if (shift_mod) {
                neko_gui_textedit_prep_selection_at_cursor(state);
                state->cursor = state->select_end = state->string.len;
                state->has_preferred_x = 0;
            } else {
                state->cursor = state->string.len;
                state->select_start = state->select_end = 0;
                state->has_preferred_x = 0;
            }
            break;

        case NEKO_GUI_KEY_TEXT_LINE_START: {
            if (shift_mod) {
                struct neko_gui_text_find find;
                neko_gui_textedit_clamp(state);
                neko_gui_textedit_prep_selection_at_cursor(state);
                if (state->string.len && state->cursor == state->string.len) --state->cursor;
                neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);
                state->cursor = state->select_end = find.first_char;
                state->has_preferred_x = 0;
            } else {
                struct neko_gui_text_find find;
                if (state->string.len && state->cursor == state->string.len) --state->cursor;
                neko_gui_textedit_clamp(state);
                neko_gui_textedit_move_to_first(state);
                neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);
                state->cursor = find.first_char;
                state->has_preferred_x = 0;
            }
        } break;

        case NEKO_GUI_KEY_TEXT_LINE_END: {
            if (shift_mod) {
                struct neko_gui_text_find find;
                neko_gui_textedit_clamp(state);
                neko_gui_textedit_prep_selection_at_cursor(state);
                neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);
                state->has_preferred_x = 0;
                state->cursor = find.first_char + find.length;
                if (find.length > 0 && neko_gui_str_rune_at(&state->string, state->cursor - 1) == '\n') --state->cursor;
                state->select_end = state->cursor;
            } else {
                struct neko_gui_text_find find;
                neko_gui_textedit_clamp(state);
                neko_gui_textedit_move_to_first(state);
                neko_gui_textedit_find_charpos(&find, state, state->cursor, state->single_line, font, row_height);

                state->has_preferred_x = 0;
                state->cursor = find.first_char + find.length;
                if (find.length > 0 && neko_gui_str_rune_at(&state->string, state->cursor - 1) == '\n') --state->cursor;
            }
        } break;
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_flush_redo(struct neko_gui_text_undo_state *state) {
    state->redo_point = NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT;
    state->redo_char_point = NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT;
}
NEKO_GUI_INTERN void neko_gui_textedit_discard_undo(struct neko_gui_text_undo_state *state) {

    if (state->undo_point > 0) {

        if (state->undo_rec[0].char_storage >= 0) {
            int n = state->undo_rec[0].insert_length, i;

            state->undo_char_point = (short)(state->undo_char_point - n);
            NEKO_GUI_MEMCPY(state->undo_char, state->undo_char + n, (neko_gui_size)state->undo_char_point * sizeof(neko_gui_rune));
            for (i = 0; i < state->undo_point; ++i) {
                if (state->undo_rec[i].char_storage >= 0) state->undo_rec[i].char_storage = (short)(state->undo_rec[i].char_storage - n);
            }
        }
        --state->undo_point;
        NEKO_GUI_MEMCPY(state->undo_rec, state->undo_rec + 1, (neko_gui_size)((neko_gui_size)state->undo_point * sizeof(state->undo_rec[0])));
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_discard_redo(struct neko_gui_text_undo_state *state) {

    neko_gui_size num;
    int k = NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT - 1;
    if (state->redo_point <= k) {

        if (state->undo_rec[k].char_storage >= 0) {
            int n = state->undo_rec[k].insert_length, i;

            state->redo_char_point = (short)(state->redo_char_point + n);
            num = (neko_gui_size)(NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT - state->redo_char_point);
            NEKO_GUI_MEMCPY(state->undo_char + state->redo_char_point, state->undo_char + state->redo_char_point - n, num * sizeof(char));
            for (i = state->redo_point; i < k; ++i) {
                if (state->undo_rec[i].char_storage >= 0) {
                    state->undo_rec[i].char_storage = (short)(state->undo_rec[i].char_storage + n);
                }
            }
        }
        ++state->redo_point;
        num = (neko_gui_size)(NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT - state->redo_point);
        if (num) NEKO_GUI_MEMCPY(state->undo_rec + state->redo_point - 1, state->undo_rec + state->redo_point, num * sizeof(state->undo_rec[0]));
    }
}
NEKO_GUI_INTERN struct neko_gui_text_undo_record *neko_gui_textedit_create_undo_record(struct neko_gui_text_undo_state *state, int numchars) {

    neko_gui_textedit_flush_redo(state);

    if (state->undo_point == NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT) neko_gui_textedit_discard_undo(state);

    if (numchars > NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT) {
        state->undo_point = 0;
        state->undo_char_point = 0;
        return 0;
    }

    while (state->undo_char_point + numchars > NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT) neko_gui_textedit_discard_undo(state);
    return &state->undo_rec[state->undo_point++];
}
NEKO_GUI_INTERN neko_gui_rune *neko_gui_textedit_createundo(struct neko_gui_text_undo_state *state, int pos, int insert_len, int delete_len) {
    struct neko_gui_text_undo_record *r = neko_gui_textedit_create_undo_record(state, insert_len);
    if (r == 0) return 0;

    r->where = pos;
    r->insert_length = (short)insert_len;
    r->delete_length = (short)delete_len;

    if (insert_len == 0) {
        r->char_storage = -1;
        return 0;
    } else {
        r->char_storage = state->undo_char_point;
        state->undo_char_point = (short)(state->undo_char_point + insert_len);
        return &state->undo_char[r->char_storage];
    }
}
NEKO_GUI_API void neko_gui_textedit_undo(struct neko_gui_text_edit *state) {
    struct neko_gui_text_undo_state *s = &state->undo;
    struct neko_gui_text_undo_record u, *r;
    if (s->undo_point == 0) return;

    u = s->undo_rec[s->undo_point - 1];
    r = &s->undo_rec[s->redo_point - 1];
    r->char_storage = -1;

    r->insert_length = u.delete_length;
    r->delete_length = u.insert_length;
    r->where = u.where;

    if (u.delete_length) {

        if (s->undo_char_point + u.delete_length >= NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT) {

            r->insert_length = 0;
        } else {
            int i;

            while (s->undo_char_point + u.delete_length > s->redo_char_point) {

                neko_gui_textedit_discard_redo(s);

                if (s->redo_point == NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT) return;
            }

            r = &s->undo_rec[s->redo_point - 1];
            r->char_storage = (short)(s->redo_char_point - u.delete_length);
            s->redo_char_point = (short)(s->redo_char_point - u.delete_length);

            for (i = 0; i < u.delete_length; ++i) s->undo_char[r->char_storage + i] = neko_gui_str_rune_at(&state->string, u.where + i);
        }

        neko_gui_str_delete_runes(&state->string, u.where, u.delete_length);
    }

    if (u.insert_length) {

        neko_gui_str_insert_text_runes(&state->string, u.where, &s->undo_char[u.char_storage], u.insert_length);
        s->undo_char_point = (short)(s->undo_char_point - u.insert_length);
    }
    state->cursor = (short)(u.where + u.insert_length);

    s->undo_point--;
    s->redo_point--;
}
NEKO_GUI_API void neko_gui_textedit_redo(struct neko_gui_text_edit *state) {
    struct neko_gui_text_undo_state *s = &state->undo;
    struct neko_gui_text_undo_record *u, r;
    if (s->redo_point == NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT) return;

    u = &s->undo_rec[s->undo_point];
    r = s->undo_rec[s->redo_point];

    u->delete_length = r.insert_length;
    u->insert_length = r.delete_length;
    u->where = r.where;
    u->char_storage = -1;

    if (r.delete_length) {

        if (s->undo_char_point + u->insert_length > s->redo_char_point) {
            u->insert_length = 0;
            u->delete_length = 0;
        } else {
            int i;
            u->char_storage = s->undo_char_point;
            s->undo_char_point = (short)(s->undo_char_point + u->insert_length);

            for (i = 0; i < u->insert_length; ++i) {
                s->undo_char[u->char_storage + i] = neko_gui_str_rune_at(&state->string, u->where + i);
            }
        }
        neko_gui_str_delete_runes(&state->string, r.where, r.delete_length);
    }

    if (r.insert_length) {

        neko_gui_str_insert_text_runes(&state->string, r.where, &s->undo_char[r.char_storage], r.insert_length);
    }
    state->cursor = r.where + r.insert_length;

    s->undo_point++;
    s->redo_point++;
}
NEKO_GUI_INTERN void neko_gui_textedit_makeundo_insert(struct neko_gui_text_edit *state, int where, int length) { neko_gui_textedit_createundo(&state->undo, where, 0, length); }
NEKO_GUI_INTERN void neko_gui_textedit_makeundo_delete(struct neko_gui_text_edit *state, int where, int length) {
    int i;
    neko_gui_rune *p = neko_gui_textedit_createundo(&state->undo, where, length, 0);
    if (p) {
        for (i = 0; i < length; ++i) p[i] = neko_gui_str_rune_at(&state->string, where + i);
    }
}
NEKO_GUI_INTERN void neko_gui_textedit_makeundo_replace(struct neko_gui_text_edit *state, int where, int old_length, int new_length) {
    int i;
    neko_gui_rune *p = neko_gui_textedit_createundo(&state->undo, where, old_length, new_length);
    if (p) {
        for (i = 0; i < old_length; ++i) p[i] = neko_gui_str_rune_at(&state->string, where + i);
    }
}
NEKO_GUI_LIB void neko_gui_textedit_clear_state(struct neko_gui_text_edit *state, enum neko_gui_text_edit_type type, neko_gui_plugin_filter filter) {

    state->undo.undo_point = 0;
    state->undo.undo_char_point = 0;
    state->undo.redo_point = NEKO_GUI_TEXTEDIT_UNDOSTATECOUNT;
    state->undo.redo_char_point = NEKO_GUI_TEXTEDIT_UNDOCHARCOUNT;
    state->select_end = state->select_start = 0;
    state->cursor = 0;
    state->has_preferred_x = 0;
    state->preferred_x = 0;
    state->cursor_at_end_of_line = 0;
    state->initialized = 1;
    state->single_line = (unsigned char)(type == NEKO_GUI_TEXT_EDIT_SINGLE_LINE);
    state->mode = NEKO_GUI_TEXT_EDIT_MODE_VIEW;
    state->filter = filter;
    state->scrollbar = neko_gui_vec2(0, 0);
}
NEKO_GUI_API void neko_gui_textedit_init_fixed(struct neko_gui_text_edit *state, void *memory, neko_gui_size size) {
    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(memory);
    if (!state || !memory || !size) return;
    NEKO_GUI_MEMSET(state, 0, sizeof(struct neko_gui_text_edit));
    neko_gui_textedit_clear_state(state, NEKO_GUI_TEXT_EDIT_SINGLE_LINE, 0);
    neko_gui_str_init_fixed(&state->string, memory, size);
}
NEKO_GUI_API void neko_gui_textedit_init(struct neko_gui_text_edit *state, struct neko_gui_allocator *alloc, neko_gui_size size) {
    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(alloc);
    if (!state || !alloc) return;
    NEKO_GUI_MEMSET(state, 0, sizeof(struct neko_gui_text_edit));
    neko_gui_textedit_clear_state(state, NEKO_GUI_TEXT_EDIT_SINGLE_LINE, 0);
    neko_gui_str_init(&state->string, alloc, size);
}
#ifdef NEKO_GUI_INCLUDE_DEFAULT_ALLOCATOR
NEKO_GUI_API void neko_gui_textedit_init_default(struct neko_gui_text_edit *state) {
    NEKO_GUI_ASSERT(state);
    if (!state) return;
    NEKO_GUI_MEMSET(state, 0, sizeof(struct neko_gui_text_edit));
    neko_gui_textedit_clear_state(state, NEKO_GUI_TEXT_EDIT_SINGLE_LINE, 0);
    neko_gui_str_init_default(&state->string);
}
#endif
NEKO_GUI_API void neko_gui_textedit_select_all(struct neko_gui_text_edit *state) {
    NEKO_GUI_ASSERT(state);
    state->select_start = 0;
    state->select_end = state->string.len;
}
NEKO_GUI_API void neko_gui_textedit_free(struct neko_gui_text_edit *state) {
    NEKO_GUI_ASSERT(state);
    if (!state) return;
    neko_gui_str_free(&state->string);
}

NEKO_GUI_API neko_gui_bool neko_gui_filter_default(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(unicode);
    NEKO_GUI_UNUSED(box);
    return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_ascii(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if (unicode > 128)
        return neko_gui_false;
    else
        return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_float(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if ((unicode < '0' || unicode > '9') && unicode != '.' && unicode != '-')
        return neko_gui_false;
    else
        return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_decimal(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if ((unicode < '0' || unicode > '9') && unicode != '-')
        return neko_gui_false;
    else
        return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_hex(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if ((unicode < '0' || unicode > '9') && (unicode < 'a' || unicode > 'f') && (unicode < 'A' || unicode > 'F'))
        return neko_gui_false;
    else
        return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_oct(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if (unicode < '0' || unicode > '7')
        return neko_gui_false;
    else
        return neko_gui_true;
}
NEKO_GUI_API neko_gui_bool neko_gui_filter_binary(const struct neko_gui_text_edit *box, neko_gui_rune unicode) {
    NEKO_GUI_UNUSED(box);
    if (unicode != '0' && unicode != '1')
        return neko_gui_false;
    else
        return neko_gui_true;
}

NEKO_GUI_LIB void neko_gui_edit_draw_text(struct neko_gui_command_buffer *out, const struct neko_gui_style_edit *style, float pos_x, float pos_y, float x_offset, const char *text, int byte_len,
                                          float row_height, const struct neko_gui_user_font *font, struct neko_gui_color background, struct neko_gui_color foreground, neko_gui_bool is_selected) {
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(font);
    NEKO_GUI_ASSERT(style);
    if (!text || !byte_len || !out || !style) return;

    {
        int glyph_len = 0;
        neko_gui_rune unicode = 0;
        int text_len = 0;
        float line_width = 0;
        float glyph_width;
        const char *line = text;
        float line_offset = 0;
        int line_count = 0;

        struct neko_gui_text txt;
        txt.padding = neko_gui_vec2(0, 0);
        txt.background = background;
        txt.text = foreground;

        glyph_len = neko_gui_utf_decode(text + text_len, &unicode, byte_len - text_len);
        if (!glyph_len) return;
        while ((text_len < byte_len) && glyph_len) {
            if (unicode == '\n') {

                struct neko_gui_rect label;
                label.y = pos_y + line_offset;
                label.h = row_height;
                label.w = line_width;
                label.x = pos_x;
                if (!line_count) label.x += x_offset;

                if (is_selected) neko_gui_fill_rect(out, label, 0, background);
                neko_gui_widget_text(out, label, line, (int)((text + text_len) - line), &txt, NEKO_GUI_TEXT_CENTERED, font);

                text_len++;
                line_count++;
                line_width = 0;
                line = text + text_len;
                line_offset += row_height;
                glyph_len = neko_gui_utf_decode(text + text_len, &unicode, (int)(byte_len - text_len));
                continue;
            }
            if (unicode == '\r') {
                text_len++;
                glyph_len = neko_gui_utf_decode(text + text_len, &unicode, byte_len - text_len);
                continue;
            }
            glyph_width = font->width(font->userdata, font->height, text + text_len, glyph_len);
            line_width += (float)glyph_width;
            text_len += glyph_len;
            glyph_len = neko_gui_utf_decode(text + text_len, &unicode, byte_len - text_len);
            continue;
        }
        if (line_width > 0) {

            struct neko_gui_rect label;
            label.y = pos_y + line_offset;
            label.h = row_height;
            label.w = line_width;
            label.x = pos_x;
            if (!line_count) label.x += x_offset;

            if (is_selected) neko_gui_fill_rect(out, label, 0, background);
            neko_gui_widget_text(out, label, line, (int)((text + text_len) - line), &txt, NEKO_GUI_TEXT_LEFT, font);
        }
    }
}
NEKO_GUI_LIB neko_gui_flags neko_gui_do_edit(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_rect bounds, neko_gui_flags flags, neko_gui_plugin_filter filter,
                                             struct neko_gui_text_edit *edit, const struct neko_gui_style_edit *style, struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    struct neko_gui_rect area;
    neko_gui_flags ret = 0;
    float row_height;
    char prev_state = 0;
    char is_hovered = 0;
    char select_all = 0;
    char cursor_follow = 0;
    struct neko_gui_rect old_clip;
    struct neko_gui_rect clip;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(style);
    if (!state || !out || !style) return ret;

    area.x = bounds.x + style->padding.x + style->border;
    area.y = bounds.y + style->padding.y + style->border;
    area.w = bounds.w - (2.0f * style->padding.x + 2 * style->border);
    area.h = bounds.h - (2.0f * style->padding.y + 2 * style->border);
    if (flags & NEKO_GUI_EDIT_MULTILINE) area.w = NEKO_GUI_MAX(0, area.w - style->scrollbar_size.x);
    row_height = (flags & NEKO_GUI_EDIT_MULTILINE) ? font->height + style->row_padding : area.h;

    old_clip = out->clip;
    neko_gui_unify(&clip, &old_clip, area.x, area.y, area.x + area.w, area.y + area.h);

    prev_state = (char)edit->active;
    is_hovered = (char)neko_gui_input_is_mouse_hovering_rect(in, bounds);
    if (in && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down) {
        edit->active = NEKO_GUI_INBOX(in->mouse.pos.x, in->mouse.pos.y, bounds.x, bounds.y, bounds.w, bounds.h);
    }

    if (!prev_state && edit->active) {
        const enum neko_gui_text_edit_type type = (flags & NEKO_GUI_EDIT_MULTILINE) ? NEKO_GUI_TEXT_EDIT_MULTI_LINE : NEKO_GUI_TEXT_EDIT_SINGLE_LINE;

        struct neko_gui_vec2 oldscrollbar = edit->scrollbar;
        neko_gui_textedit_clear_state(edit, type, filter);
        edit->scrollbar = oldscrollbar;
        if (flags & NEKO_GUI_EDIT_AUTO_SELECT) select_all = neko_gui_true;
        if (flags & NEKO_GUI_EDIT_GOTO_END_ON_ACTIVATE) {
            edit->cursor = edit->string.len;
            in = 0;
        }
    } else if (!edit->active)
        edit->mode = NEKO_GUI_TEXT_EDIT_MODE_VIEW;
    if (flags & NEKO_GUI_EDIT_READ_ONLY)
        edit->mode = NEKO_GUI_TEXT_EDIT_MODE_VIEW;
    else if (flags & NEKO_GUI_EDIT_ALWAYS_INSERT_MODE)
        edit->mode = NEKO_GUI_TEXT_EDIT_MODE_INSERT;

    ret = (edit->active) ? NEKO_GUI_EDIT_ACTIVE : NEKO_GUI_EDIT_INACTIVE;
    if (prev_state != edit->active) ret |= (edit->active) ? NEKO_GUI_EDIT_ACTIVATED : NEKO_GUI_EDIT_DEACTIVATED;

    if (edit->active && in) {
        int shift_mod = in->keyboard.keys[NEKO_GUI_KEY_SHIFT].down;
        const float mouse_x = (in->mouse.pos.x - area.x) + edit->scrollbar.x;
        const float mouse_y = (in->mouse.pos.y - area.y) + edit->scrollbar.y;

        is_hovered = (char)neko_gui_input_is_mouse_hovering_rect(in, area);
        if (select_all) {
            neko_gui_textedit_select_all(edit);
        } else if (is_hovered && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked) {
            neko_gui_textedit_click(edit, mouse_x, mouse_y, font, row_height);
        } else if (is_hovered && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down && (in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f)) {
            neko_gui_textedit_drag(edit, mouse_x, mouse_y, font, row_height);
            cursor_follow = neko_gui_true;
        } else if (is_hovered && in->mouse.buttons[NEKO_GUI_BUTTON_RIGHT].clicked && in->mouse.buttons[NEKO_GUI_BUTTON_RIGHT].down) {
            neko_gui_textedit_key(edit, NEKO_GUI_KEY_TEXT_WORD_LEFT, neko_gui_false, font, row_height);
            neko_gui_textedit_key(edit, NEKO_GUI_KEY_TEXT_WORD_RIGHT, neko_gui_true, font, row_height);
            cursor_follow = neko_gui_true;
        }

        {
            int i;
            int old_mode = edit->mode;
            for (i = 0; i < NEKO_GUI_KEY_MAX; ++i) {
                if (i == NEKO_GUI_KEY_ENTER || i == NEKO_GUI_KEY_TAB) continue;
                if (neko_gui_input_is_key_pressed(in, (enum neko_gui_keys)i)) {
                    neko_gui_textedit_key(edit, (enum neko_gui_keys)i, shift_mod, font, row_height);
                    cursor_follow = neko_gui_true;
                }
            }
            if (old_mode != edit->mode) {
                in->keyboard.text_len = 0;
            }
        }

        edit->filter = filter;
        if (in->keyboard.text_len) {
            neko_gui_textedit_text(edit, in->keyboard.text, in->keyboard.text_len);
            cursor_follow = neko_gui_true;
            in->keyboard.text_len = 0;
        }

        if (neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_ENTER)) {
            cursor_follow = neko_gui_true;
            if (flags & NEKO_GUI_EDIT_CTRL_ENTER_NEWLINE && shift_mod)
                neko_gui_textedit_text(edit, "\n", 1);
            else if (flags & NEKO_GUI_EDIT_SIG_ENTER)
                ret |= NEKO_GUI_EDIT_COMMITED;
            else
                neko_gui_textedit_text(edit, "\n", 1);
        }

        {
            int copy = neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_COPY);
            int cut = neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_CUT);
            if ((copy || cut) && (flags & NEKO_GUI_EDIT_CLIPBOARD)) {
                int glyph_len;
                neko_gui_rune unicode;
                const char *text;
                int b = edit->select_start;
                int e = edit->select_end;

                int begin = NEKO_GUI_MIN(b, e);
                int end = NEKO_GUI_MAX(b, e);
                text = neko_gui_str_at_const(&edit->string, begin, &unicode, &glyph_len);
                if (edit->clip.copy) edit->clip.copy(edit->clip.userdata, text, end - begin);
                if (cut && !(flags & NEKO_GUI_EDIT_READ_ONLY)) {
                    neko_gui_textedit_cut(edit);
                    cursor_follow = neko_gui_true;
                }
            }
        }

        {
            int paste = neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_PASTE);
            if (paste && (flags & NEKO_GUI_EDIT_CLIPBOARD) && edit->clip.paste) {
                edit->clip.paste(edit->clip.userdata, edit);
                cursor_follow = neko_gui_true;
            }
        }

        {
            int tab = neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_TAB);
            if (tab && (flags & NEKO_GUI_EDIT_ALLOW_TAB)) {
                neko_gui_textedit_text(edit, "    ", 4);
                cursor_follow = neko_gui_true;
            }
        }
    }

    if (edit->active)
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
    else
        neko_gui_widget_state_reset(state);

    if (is_hovered) *state |= NEKO_GUI_WIDGET_STATE_HOVERED;

    {
        const char *text = neko_gui_str_get_const(&edit->string);
        int len = neko_gui_str_len_char(&edit->string);

        {
            const struct neko_gui_style_item *background;
            if (*state & NEKO_GUI_WIDGET_STATE_ACTIVED)
                background = &style->active;
            else if (*state & NEKO_GUI_WIDGET_STATE_HOVER)
                background = &style->hover;
            else
                background = &style->normal;

            switch (background->type) {
                case NEKO_GUI_STYLE_ITEM_IMAGE:
                    neko_gui_draw_image(out, bounds, &background->data.image, neko_gui_white);
                    break;
                case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
                    neko_gui_draw_nine_slice(out, bounds, &background->data.slice, neko_gui_white);
                    break;
                case NEKO_GUI_STYLE_ITEM_COLOR:
                    neko_gui_fill_rect(out, bounds, style->rounding, background->data.color);
                    neko_gui_stroke_rect(out, bounds, style->rounding, style->border, style->border_color);
                    break;
            }
        }

        area.w = NEKO_GUI_MAX(0, area.w - style->cursor_size);
        if (edit->active) {
            int total_lines = 1;
            struct neko_gui_vec2 text_size = neko_gui_vec2(0, 0);

            const char *cursor_ptr = 0;
            const char *select_begin_ptr = 0;
            const char *select_end_ptr = 0;

            struct neko_gui_vec2 cursor_pos = neko_gui_vec2(0, 0);
            struct neko_gui_vec2 selection_offset_start = neko_gui_vec2(0, 0);
            struct neko_gui_vec2 selection_offset_end = neko_gui_vec2(0, 0);

            int selection_begin = NEKO_GUI_MIN(edit->select_start, edit->select_end);
            int selection_end = NEKO_GUI_MAX(edit->select_start, edit->select_end);

            float line_width = 0.0f;
            if (text && len) {

                float glyph_width;
                int glyph_len = 0;
                neko_gui_rune unicode = 0;
                int text_len = 0;
                int glyphs = 0;
                int row_begin = 0;

                glyph_len = neko_gui_utf_decode(text, &unicode, len);
                glyph_width = font->width(font->userdata, font->height, text, glyph_len);
                line_width = 0;

                while ((text_len < len) && glyph_len) {

                    if (!cursor_ptr && glyphs == edit->cursor) {
                        int glyph_offset;
                        struct neko_gui_vec2 out_offset;
                        struct neko_gui_vec2 row_size;
                        const char *remaining;

                        cursor_pos.y = (float)(total_lines - 1) * row_height;
                        row_size = neko_gui_text_calculate_text_bounds(font, text + row_begin, text_len - row_begin, row_height, &remaining, &out_offset, &glyph_offset, NEKO_GUI_STOP_ON_NEW_LINE);
                        cursor_pos.x = row_size.x;
                        cursor_ptr = text + text_len;
                    }

                    if (!select_begin_ptr && edit->select_start != edit->select_end && glyphs == selection_begin) {
                        int glyph_offset;
                        struct neko_gui_vec2 out_offset;
                        struct neko_gui_vec2 row_size;
                        const char *remaining;

                        selection_offset_start.y = (float)(NEKO_GUI_MAX(total_lines - 1, 0)) * row_height;
                        row_size = neko_gui_text_calculate_text_bounds(font, text + row_begin, text_len - row_begin, row_height, &remaining, &out_offset, &glyph_offset, NEKO_GUI_STOP_ON_NEW_LINE);
                        selection_offset_start.x = row_size.x;
                        select_begin_ptr = text + text_len;
                    }

                    if (!select_end_ptr && edit->select_start != edit->select_end && glyphs == selection_end) {
                        int glyph_offset;
                        struct neko_gui_vec2 out_offset;
                        struct neko_gui_vec2 row_size;
                        const char *remaining;

                        selection_offset_end.y = (float)(total_lines - 1) * row_height;
                        row_size = neko_gui_text_calculate_text_bounds(font, text + row_begin, text_len - row_begin, row_height, &remaining, &out_offset, &glyph_offset, NEKO_GUI_STOP_ON_NEW_LINE);
                        selection_offset_end.x = row_size.x;
                        select_end_ptr = text + text_len;
                    }
                    if (unicode == '\n') {
                        text_size.x = NEKO_GUI_MAX(text_size.x, line_width);
                        total_lines++;
                        line_width = 0;
                        text_len++;
                        glyphs++;
                        row_begin = text_len;
                        glyph_len = neko_gui_utf_decode(text + text_len, &unicode, len - text_len);
                        glyph_width = font->width(font->userdata, font->height, text + text_len, glyph_len);
                        continue;
                    }

                    glyphs++;
                    text_len += glyph_len;
                    line_width += (float)glyph_width;

                    glyph_len = neko_gui_utf_decode(text + text_len, &unicode, len - text_len);
                    glyph_width = font->width(font->userdata, font->height, text + text_len, glyph_len);
                    continue;
                }
                text_size.y = (float)total_lines * row_height;

                if (!cursor_ptr && edit->cursor == edit->string.len) {
                    cursor_pos.x = line_width;
                    cursor_pos.y = text_size.y - row_height;
                }
            }
            {

                if (cursor_follow) {

                    if (!(flags & NEKO_GUI_EDIT_NO_HORIZONTAL_SCROLL)) {

                        const float scroll_increment = area.w * 0.25f;
                        if (cursor_pos.x < edit->scrollbar.x) edit->scrollbar.x = (float)(int)NEKO_GUI_MAX(0.0f, cursor_pos.x - scroll_increment);
                        if (cursor_pos.x >= edit->scrollbar.x + area.w) edit->scrollbar.x = (float)(int)NEKO_GUI_MAX(0.0f, cursor_pos.x - area.w + scroll_increment);
                    } else
                        edit->scrollbar.x = 0;

                    if (flags & NEKO_GUI_EDIT_MULTILINE) {

                        if (cursor_pos.y < edit->scrollbar.y) edit->scrollbar.y = NEKO_GUI_MAX(0.0f, cursor_pos.y - row_height);
                        if (cursor_pos.y >= edit->scrollbar.y + row_height) edit->scrollbar.y = edit->scrollbar.y + row_height;
                    } else
                        edit->scrollbar.y = 0;
                }

                if (flags & NEKO_GUI_EDIT_MULTILINE) {
                    neko_gui_flags ws;
                    struct neko_gui_rect scroll;
                    float scroll_target;
                    float scroll_offset;
                    float scroll_step;
                    float scroll_inc;

                    scroll = area;
                    scroll.x = (bounds.x + bounds.w - style->border) - style->scrollbar_size.x;
                    scroll.w = style->scrollbar_size.x;

                    scroll_offset = edit->scrollbar.y;
                    scroll_step = scroll.h * 0.10f;
                    scroll_inc = scroll.h * 0.01f;
                    scroll_target = text_size.y;
                    edit->scrollbar.y = neko_gui_do_scrollbarv(&ws, out, scroll, 0, scroll_offset, scroll_target, scroll_step, scroll_inc, &style->scrollbar, in, font);
                }
            }

            {
                struct neko_gui_color background_color;
                struct neko_gui_color text_color;
                struct neko_gui_color sel_background_color;
                struct neko_gui_color sel_text_color;
                struct neko_gui_color cursor_color;
                struct neko_gui_color cursor_text_color;
                const struct neko_gui_style_item *background;
                neko_gui_push_scissor(out, clip);

                if (*state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
                    background = &style->active;
                    text_color = style->text_active;
                    sel_text_color = style->selected_text_hover;
                    sel_background_color = style->selected_hover;
                    cursor_color = style->cursor_hover;
                    cursor_text_color = style->cursor_text_hover;
                } else if (*state & NEKO_GUI_WIDGET_STATE_HOVER) {
                    background = &style->hover;
                    text_color = style->text_hover;
                    sel_text_color = style->selected_text_hover;
                    sel_background_color = style->selected_hover;
                    cursor_text_color = style->cursor_text_hover;
                    cursor_color = style->cursor_hover;
                } else {
                    background = &style->normal;
                    text_color = style->text_normal;
                    sel_text_color = style->selected_text_normal;
                    sel_background_color = style->selected_normal;
                    cursor_color = style->cursor_normal;
                    cursor_text_color = style->cursor_text_normal;
                }
                if (background->type == NEKO_GUI_STYLE_ITEM_IMAGE)
                    background_color = neko_gui_rgba(0, 0, 0, 0);
                else
                    background_color = background->data.color;

                if (edit->select_start == edit->select_end) {

                    const char *begin = neko_gui_str_get_const(&edit->string);
                    int l = neko_gui_str_len_char(&edit->string);
                    neko_gui_edit_draw_text(out, style, area.x - edit->scrollbar.x, area.y - edit->scrollbar.y, 0, begin, l, row_height, font, background_color, text_color, neko_gui_false);
                } else {

                    if (edit->select_start != edit->select_end && selection_begin > 0) {

                        const char *begin = neko_gui_str_get_const(&edit->string);
                        NEKO_GUI_ASSERT(select_begin_ptr);
                        neko_gui_edit_draw_text(out, style, area.x - edit->scrollbar.x, area.y - edit->scrollbar.y, 0, begin, (int)(select_begin_ptr - begin), row_height, font, background_color,
                                                text_color, neko_gui_false);
                    }
                    if (edit->select_start != edit->select_end) {

                        NEKO_GUI_ASSERT(select_begin_ptr);
                        if (!select_end_ptr) {
                            const char *begin = neko_gui_str_get_const(&edit->string);
                            select_end_ptr = begin + neko_gui_str_len_char(&edit->string);
                        }
                        neko_gui_edit_draw_text(out, style, area.x - edit->scrollbar.x, area.y + selection_offset_start.y - edit->scrollbar.y, selection_offset_start.x, select_begin_ptr,
                                                (int)(select_end_ptr - select_begin_ptr), row_height, font, sel_background_color, sel_text_color, neko_gui_true);
                    }
                    if ((edit->select_start != edit->select_end && selection_end < edit->string.len)) {

                        const char *begin = select_end_ptr;
                        const char *end = neko_gui_str_get_const(&edit->string) + neko_gui_str_len_char(&edit->string);
                        NEKO_GUI_ASSERT(select_end_ptr);
                        neko_gui_edit_draw_text(out, style, area.x - edit->scrollbar.x, area.y + selection_offset_end.y - edit->scrollbar.y, selection_offset_end.x, begin, (int)(end - begin),
                                                row_height, font, background_color, text_color, neko_gui_true);
                    }
                }

                if (edit->select_start == edit->select_end) {
                    if (edit->cursor >= neko_gui_str_len(&edit->string) || (cursor_ptr && *cursor_ptr == '\n')) {

                        struct neko_gui_rect cursor;
                        cursor.w = style->cursor_size;
                        cursor.h = font->height;
                        cursor.x = area.x + cursor_pos.x - edit->scrollbar.x;
                        cursor.y = area.y + cursor_pos.y + row_height / 2.0f - cursor.h / 2.0f;
                        cursor.y -= edit->scrollbar.y;
                        neko_gui_fill_rect(out, cursor, 0, cursor_color);
                    } else {

                        int glyph_len;
                        struct neko_gui_rect label;
                        struct neko_gui_text txt;

                        neko_gui_rune unicode;
                        NEKO_GUI_ASSERT(cursor_ptr);
                        glyph_len = neko_gui_utf_decode(cursor_ptr, &unicode, 4);

                        label.x = area.x + cursor_pos.x - edit->scrollbar.x;
                        label.y = area.y + cursor_pos.y - edit->scrollbar.y;
                        label.w = font->width(font->userdata, font->height, cursor_ptr, glyph_len);
                        label.h = row_height;

                        txt.padding = neko_gui_vec2(0, 0);
                        txt.background = cursor_color;
                        ;
                        txt.text = cursor_text_color;
                        neko_gui_fill_rect(out, label, 0, cursor_color);
                        neko_gui_widget_text(out, label, cursor_ptr, glyph_len, &txt, NEKO_GUI_TEXT_LEFT, font);
                    }
                }
            }
        } else {

            int l = neko_gui_str_len_char(&edit->string);
            const char *begin = neko_gui_str_get_const(&edit->string);

            const struct neko_gui_style_item *background;
            struct neko_gui_color background_color;
            struct neko_gui_color text_color;
            neko_gui_push_scissor(out, clip);
            if (*state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
                background = &style->active;
                text_color = style->text_active;
            } else if (*state & NEKO_GUI_WIDGET_STATE_HOVER) {
                background = &style->hover;
                text_color = style->text_hover;
            } else {
                background = &style->normal;
                text_color = style->text_normal;
            }
            if (background->type == NEKO_GUI_STYLE_ITEM_IMAGE)
                background_color = neko_gui_rgba(0, 0, 0, 0);
            else
                background_color = background->data.color;
            neko_gui_edit_draw_text(out, style, area.x - edit->scrollbar.x, area.y - edit->scrollbar.y, 0, begin, l, row_height, font, background_color, text_color, neko_gui_false);
        }
        neko_gui_push_scissor(out, old_clip);
    }
    return ret;
}
NEKO_GUI_API void neko_gui_edit_focus(struct neko_gui_context *ctx, neko_gui_flags flags) {
    neko_gui_hash hash;
    struct neko_gui_window *win;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;

    win = ctx->current;
    hash = win->edit.seq;
    win->edit.active = neko_gui_true;
    win->edit.name = hash;
    if (flags & NEKO_GUI_EDIT_ALWAYS_INSERT_MODE) win->edit.mode = NEKO_GUI_TEXT_EDIT_MODE_INSERT;
}
NEKO_GUI_API void neko_gui_edit_unfocus(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;

    win = ctx->current;
    win->edit.active = neko_gui_false;
    win->edit.name = 0;
}
NEKO_GUI_API neko_gui_flags neko_gui_edit_string(struct neko_gui_context *ctx, neko_gui_flags flags, char *memory, int *len, int max, neko_gui_plugin_filter filter) {
    neko_gui_hash hash;
    neko_gui_flags state;
    struct neko_gui_text_edit *edit;
    struct neko_gui_window *win;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(memory);
    NEKO_GUI_ASSERT(len);
    if (!ctx || !memory || !len) return 0;

    filter = (!filter) ? neko_gui_filter_default : filter;
    win = ctx->current;
    hash = win->edit.seq;
    edit = &ctx->text_edit;
    neko_gui_textedit_clear_state(&ctx->text_edit, (flags & NEKO_GUI_EDIT_MULTILINE) ? NEKO_GUI_TEXT_EDIT_MULTI_LINE : NEKO_GUI_TEXT_EDIT_SINGLE_LINE, filter);

    if (win->edit.active && hash == win->edit.name) {
        if (flags & NEKO_GUI_EDIT_NO_CURSOR)
            edit->cursor = neko_gui_utf_len(memory, *len);
        else
            edit->cursor = win->edit.cursor;
        if (!(flags & NEKO_GUI_EDIT_SELECTABLE)) {
            edit->select_start = win->edit.cursor;
            edit->select_end = win->edit.cursor;
        } else {
            edit->select_start = win->edit.sel_start;
            edit->select_end = win->edit.sel_end;
        }
        edit->mode = win->edit.mode;
        edit->scrollbar.x = (float)win->edit.scrollbar.x;
        edit->scrollbar.y = (float)win->edit.scrollbar.y;
        edit->active = neko_gui_true;
    } else
        edit->active = neko_gui_false;

    max = NEKO_GUI_MAX(1, max);
    *len = NEKO_GUI_MIN(*len, max - 1);
    neko_gui_str_init_fixed(&edit->string, memory, (neko_gui_size)max);
    edit->string.buffer.allocated = (neko_gui_size)*len;
    edit->string.len = neko_gui_utf_len(memory, *len);
    state = neko_gui_edit_buffer(ctx, flags, edit, filter);
    *len = (int)edit->string.buffer.allocated;

    if (edit->active) {
        win->edit.cursor = edit->cursor;
        win->edit.sel_start = edit->select_start;
        win->edit.sel_end = edit->select_end;
        win->edit.mode = edit->mode;
        win->edit.scrollbar.x = (neko_gui_uint)edit->scrollbar.x;
        win->edit.scrollbar.y = (neko_gui_uint)edit->scrollbar.y;
    }
    return state;
}
NEKO_GUI_API neko_gui_flags neko_gui_edit_buffer(struct neko_gui_context *ctx, neko_gui_flags flags, struct neko_gui_text_edit *edit, neko_gui_plugin_filter filter) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    struct neko_gui_input *in;

    enum neko_gui_widget_layout_states state;
    struct neko_gui_rect bounds;

    neko_gui_flags ret_flags = 0;
    unsigned char prev_state;
    neko_gui_hash hash;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(edit);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    state = neko_gui_widget(&bounds, ctx);
    if (!state) return state;
    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;

    hash = win->edit.seq++;
    if (win->edit.active && hash == win->edit.name) {
        if (flags & NEKO_GUI_EDIT_NO_CURSOR) edit->cursor = edit->string.len;
        if (!(flags & NEKO_GUI_EDIT_SELECTABLE)) {
            edit->select_start = edit->cursor;
            edit->select_end = edit->cursor;
        }
        if (flags & NEKO_GUI_EDIT_CLIPBOARD) edit->clip = ctx->clip;
        edit->active = (unsigned char)win->edit.active;
    } else
        edit->active = neko_gui_false;
    edit->mode = win->edit.mode;

    filter = (!filter) ? neko_gui_filter_default : filter;
    prev_state = (unsigned char)edit->active;
    in = (flags & NEKO_GUI_EDIT_READ_ONLY) ? 0 : in;
    ret_flags = neko_gui_do_edit(&ctx->last_widget_state, &win->buffer, bounds, flags, filter, edit, &style->edit, in, style->font);

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER) ctx->style.cursor_active = ctx->style.cursors[NEKO_GUI_CURSOR_TEXT];
    if (edit->active && prev_state != edit->active) {

        win->edit.active = neko_gui_true;
        win->edit.name = hash;
    } else if (prev_state && !edit->active) {

        win->edit.active = neko_gui_false;
    }
    return ret_flags;
}
NEKO_GUI_API neko_gui_flags neko_gui_edit_string_zero_terminated(struct neko_gui_context *ctx, neko_gui_flags flags, char *buffer, int max, neko_gui_plugin_filter filter) {
    neko_gui_flags result;
    int len = neko_gui_strlen(buffer);
    result = neko_gui_edit_string(ctx, flags, buffer, &len, max, filter);
    buffer[NEKO_GUI_MIN(NEKO_GUI_MAX(max - 1, 0), len)] = '\0';
    return result;
}

NEKO_GUI_LIB void neko_gui_drag_behavior(neko_gui_flags *state, const struct neko_gui_input *in, struct neko_gui_rect drag, struct neko_gui_property_variant *variant, float inc_per_pixel) {
    int left_mouse_down = in && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down;
    int left_mouse_click_in_cursor = in && neko_gui_input_has_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, drag, neko_gui_true);

    neko_gui_widget_state_reset(state);
    if (neko_gui_input_is_mouse_hovering_rect(in, drag)) *state = NEKO_GUI_WIDGET_STATE_HOVERED;

    if (left_mouse_down && left_mouse_click_in_cursor) {
        float delta, pixels;
        pixels = in->mouse.delta.x;
        delta = pixels * inc_per_pixel;
        switch (variant->kind) {
            default:
                break;
            case NEKO_GUI_PROPERTY_INT:
                variant->value.i = variant->value.i + (int)delta;
                variant->value.i = NEKO_GUI_CLAMP(variant->min_value.i, variant->value.i, variant->max_value.i);
                break;
            case NEKO_GUI_PROPERTY_FLOAT:
                variant->value.f = variant->value.f + (float)delta;
                variant->value.f = NEKO_GUI_CLAMP(variant->min_value.f, variant->value.f, variant->max_value.f);
                break;
            case NEKO_GUI_PROPERTY_DOUBLE:
                variant->value.d = variant->value.d + (double)delta;
                variant->value.d = NEKO_GUI_CLAMP(variant->min_value.d, variant->value.d, variant->max_value.d);
                break;
        }
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
    }
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, drag))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, drag))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
}
NEKO_GUI_LIB void neko_gui_property_behavior(neko_gui_flags *ws, const struct neko_gui_input *in, struct neko_gui_rect property, struct neko_gui_rect label, struct neko_gui_rect edit,
                                             struct neko_gui_rect empty, int *state, struct neko_gui_property_variant *variant, float inc_per_pixel) {
    neko_gui_widget_state_reset(ws);
    if (in && *state == NEKO_GUI_PROPERTY_DEFAULT) {
        if (neko_gui_button_behavior(ws, edit, in, NEKO_GUI_BUTTON_DEFAULT))
            *state = NEKO_GUI_PROPERTY_EDIT;
        else if (neko_gui_input_is_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, label, neko_gui_true))
            *state = NEKO_GUI_PROPERTY_DRAG;
        else if (neko_gui_input_is_mouse_click_down_in_rect(in, NEKO_GUI_BUTTON_LEFT, empty, neko_gui_true))
            *state = NEKO_GUI_PROPERTY_DRAG;
    }
    if (*state == NEKO_GUI_PROPERTY_DRAG) {
        neko_gui_drag_behavior(ws, in, property, variant, inc_per_pixel);
        if (!(*ws & NEKO_GUI_WIDGET_STATE_ACTIVED)) *state = NEKO_GUI_PROPERTY_DEFAULT;
    }
}
NEKO_GUI_LIB void neko_gui_draw_property(struct neko_gui_command_buffer *out, const struct neko_gui_style_property *style, const struct neko_gui_rect *bounds, const struct neko_gui_rect *label,
                                         neko_gui_flags state, const char *name, int len, const struct neko_gui_user_font *font) {
    struct neko_gui_text text;
    const struct neko_gui_style_item *background;

    if (state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->active;
        text.text = style->label_active;
    } else if (state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->hover;
        text.text = style->label_hover;
    } else {
        background = &style->normal;
        text.text = style->label_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(out, *bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(out, *bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            neko_gui_fill_rect(out, *bounds, style->rounding, background->data.color);
            neko_gui_stroke_rect(out, *bounds, style->rounding, style->border, background->data.color);
            break;
    }

    text.padding = neko_gui_vec2(0, 0);
    neko_gui_widget_text(out, *label, name, len, &text, NEKO_GUI_TEXT_CENTERED, font);
}
NEKO_GUI_LIB void neko_gui_do_property(neko_gui_flags *ws, struct neko_gui_command_buffer *out, struct neko_gui_rect property, const char *name, struct neko_gui_property_variant *variant,
                                       float inc_per_pixel, char *buffer, int *len, int *state, int *cursor, int *select_begin, int *select_end, const struct neko_gui_style_property *style,
                                       enum neko_gui_property_filter filter, struct neko_gui_input *in, const struct neko_gui_user_font *font, struct neko_gui_text_edit *text_edit,
                                       enum neko_gui_button_behavior behavior) {
    const neko_gui_plugin_filter filters[] = {neko_gui_filter_decimal, neko_gui_filter_float};
    neko_gui_bool active, old;
    int num_len = 0, name_len;
    char string[NEKO_GUI_MAX_NUMBER_BUFFER];
    float size;

    char *dst = 0;
    int *length;

    struct neko_gui_rect left;
    struct neko_gui_rect right;
    struct neko_gui_rect label;
    struct neko_gui_rect edit;
    struct neko_gui_rect empty;

    left.h = font->height / 2;
    left.w = left.h;
    left.x = property.x + style->border + style->padding.x;
    left.y = property.y + style->border + property.h / 2.0f - left.h / 2;

    name_len = neko_gui_strlen(name);
    size = font->width(font->userdata, font->height, name, name_len);
    label.x = left.x + left.w + style->padding.x;
    label.w = (float)size + 2 * style->padding.x;
    label.y = property.y + style->border + style->padding.y;
    label.h = property.h - (2 * style->border + 2 * style->padding.y);

    right.y = left.y;
    right.w = left.w;
    right.h = left.h;
    right.x = property.x + property.w - (right.w + style->padding.x);

    if (*state == NEKO_GUI_PROPERTY_EDIT) {
        size = font->width(font->userdata, font->height, buffer, *len);
        size += style->edit.cursor_size;
        length = len;
        dst = buffer;
    } else {
        switch (variant->kind) {
            default:
                break;
            case NEKO_GUI_PROPERTY_INT:
                neko_gui_itoa(string, variant->value.i);
                num_len = neko_gui_strlen(string);
                break;
            case NEKO_GUI_PROPERTY_FLOAT:
                NEKO_GUI_DTOA(string, (double)variant->value.f);
                num_len = neko_gui_string_float_limit(string, NEKO_GUI_MAX_FLOAT_PRECISION);
                break;
            case NEKO_GUI_PROPERTY_DOUBLE:
                NEKO_GUI_DTOA(string, variant->value.d);
                num_len = neko_gui_string_float_limit(string, NEKO_GUI_MAX_FLOAT_PRECISION);
                break;
        }
        size = font->width(font->userdata, font->height, string, num_len);
        dst = string;
        length = &num_len;
    }

    edit.w = (float)size + 2 * style->padding.x;
    edit.w = NEKO_GUI_MIN(edit.w, right.x - (label.x + label.w));
    edit.x = right.x - (edit.w + style->padding.x);
    edit.y = property.y + style->border;
    edit.h = property.h - (2 * style->border);

    empty.w = edit.x - (label.x + label.w);
    empty.x = label.x + label.w;
    empty.y = property.y;
    empty.h = property.h;

    old = (*state == NEKO_GUI_PROPERTY_EDIT);
    neko_gui_property_behavior(ws, in, property, label, edit, empty, state, variant, inc_per_pixel);

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    neko_gui_draw_property(out, style, &property, &label, *ws, name, name_len, font);
    if (style->draw_end) style->draw_end(out, style->userdata);

    if (neko_gui_do_button_symbol(ws, out, left, style->sym_left, behavior, &style->dec_button, in, font)) {
        switch (variant->kind) {
            default:
                break;
            case NEKO_GUI_PROPERTY_INT:
                variant->value.i = NEKO_GUI_CLAMP(variant->min_value.i, variant->value.i - variant->step.i, variant->max_value.i);
                break;
            case NEKO_GUI_PROPERTY_FLOAT:
                variant->value.f = NEKO_GUI_CLAMP(variant->min_value.f, variant->value.f - variant->step.f, variant->max_value.f);
                break;
            case NEKO_GUI_PROPERTY_DOUBLE:
                variant->value.d = NEKO_GUI_CLAMP(variant->min_value.d, variant->value.d - variant->step.d, variant->max_value.d);
                break;
        }
    }

    if (neko_gui_do_button_symbol(ws, out, right, style->sym_right, behavior, &style->inc_button, in, font)) {
        switch (variant->kind) {
            default:
                break;
            case NEKO_GUI_PROPERTY_INT:
                variant->value.i = NEKO_GUI_CLAMP(variant->min_value.i, variant->value.i + variant->step.i, variant->max_value.i);
                break;
            case NEKO_GUI_PROPERTY_FLOAT:
                variant->value.f = NEKO_GUI_CLAMP(variant->min_value.f, variant->value.f + variant->step.f, variant->max_value.f);
                break;
            case NEKO_GUI_PROPERTY_DOUBLE:
                variant->value.d = NEKO_GUI_CLAMP(variant->min_value.d, variant->value.d + variant->step.d, variant->max_value.d);
                break;
        }
    }
    if (old != NEKO_GUI_PROPERTY_EDIT && (*state == NEKO_GUI_PROPERTY_EDIT)) {

        NEKO_GUI_MEMCPY(buffer, dst, (neko_gui_size)*length);
        *cursor = neko_gui_utf_len(buffer, *length);
        *len = *length;
        length = len;
        dst = buffer;
        active = 0;
    } else
        active = (*state == NEKO_GUI_PROPERTY_EDIT);

    neko_gui_textedit_clear_state(text_edit, NEKO_GUI_TEXT_EDIT_SINGLE_LINE, filters[filter]);
    text_edit->active = (unsigned char)active;
    text_edit->string.len = *length;
    text_edit->cursor = NEKO_GUI_CLAMP(0, *cursor, *length);
    text_edit->select_start = NEKO_GUI_CLAMP(0, *select_begin, *length);
    text_edit->select_end = NEKO_GUI_CLAMP(0, *select_end, *length);
    text_edit->string.buffer.allocated = (neko_gui_size)*length;
    text_edit->string.buffer.memory.size = NEKO_GUI_MAX_NUMBER_BUFFER;
    text_edit->string.buffer.memory.ptr = dst;
    text_edit->string.buffer.size = NEKO_GUI_MAX_NUMBER_BUFFER;
    text_edit->mode = NEKO_GUI_TEXT_EDIT_MODE_INSERT;
    neko_gui_do_edit(ws, out, edit, NEKO_GUI_EDIT_FIELD | NEKO_GUI_EDIT_AUTO_SELECT, filters[filter], text_edit, &style->edit, (*state == NEKO_GUI_PROPERTY_EDIT) ? in : 0, font);

    *length = text_edit->string.len;
    *cursor = text_edit->cursor;
    *select_begin = text_edit->select_start;
    *select_end = text_edit->select_end;
    if (text_edit->active && neko_gui_input_is_key_pressed(in, NEKO_GUI_KEY_ENTER)) text_edit->active = neko_gui_false;

    if (active && !text_edit->active) {

        *state = NEKO_GUI_PROPERTY_DEFAULT;
        buffer[*len] = '\0';
        switch (variant->kind) {
            default:
                break;
            case NEKO_GUI_PROPERTY_INT:
                variant->value.i = neko_gui_strtoi(buffer, 0);
                variant->value.i = NEKO_GUI_CLAMP(variant->min_value.i, variant->value.i, variant->max_value.i);
                break;
            case NEKO_GUI_PROPERTY_FLOAT:
                neko_gui_string_float_limit(buffer, NEKO_GUI_MAX_FLOAT_PRECISION);
                variant->value.f = neko_gui_strtof(buffer, 0);
                variant->value.f = NEKO_GUI_CLAMP(variant->min_value.f, variant->value.f, variant->max_value.f);
                break;
            case NEKO_GUI_PROPERTY_DOUBLE:
                neko_gui_string_float_limit(buffer, NEKO_GUI_MAX_FLOAT_PRECISION);
                variant->value.d = neko_gui_strtod(buffer, 0);
                variant->value.d = NEKO_GUI_CLAMP(variant->min_value.d, variant->value.d, variant->max_value.d);
                break;
        }
    }
}
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_int(int value, int min_value, int max_value, int step) {
    struct neko_gui_property_variant result;
    result.kind = NEKO_GUI_PROPERTY_INT;
    result.value.i = value;
    result.min_value.i = min_value;
    result.max_value.i = max_value;
    result.step.i = step;
    return result;
}
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_float(float value, float min_value, float max_value, float step) {
    struct neko_gui_property_variant result;
    result.kind = NEKO_GUI_PROPERTY_FLOAT;
    result.value.f = value;
    result.min_value.f = min_value;
    result.max_value.f = max_value;
    result.step.f = step;
    return result;
}
NEKO_GUI_LIB struct neko_gui_property_variant neko_gui_property_variant_double(double value, double min_value, double max_value, double step) {
    struct neko_gui_property_variant result;
    result.kind = NEKO_GUI_PROPERTY_DOUBLE;
    result.value.d = value;
    result.min_value.d = min_value;
    result.max_value.d = max_value;
    result.step.d = step;
    return result;
}
NEKO_GUI_LIB void neko_gui_property(struct neko_gui_context *ctx, const char *name, struct neko_gui_property_variant *variant, float inc_per_pixel, const enum neko_gui_property_filter filter) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    struct neko_gui_input *in;
    const struct neko_gui_style *style;

    struct neko_gui_rect bounds;
    enum neko_gui_widget_layout_states s;

    int *state = 0;
    neko_gui_hash hash = 0;
    char *buffer = 0;
    int *len = 0;
    int *cursor = 0;
    int *select_begin = 0;
    int *select_end = 0;
    int old_state;

    char dummy_buffer[NEKO_GUI_MAX_NUMBER_BUFFER];
    int dummy_state = NEKO_GUI_PROPERTY_DEFAULT;
    int dummy_length = 0;
    int dummy_cursor = 0;
    int dummy_select_begin = 0;
    int dummy_select_end = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;
    s = neko_gui_widget(&bounds, ctx);
    if (!s) return;

    if (name[0] == '#') {
        hash = neko_gui_murmur_hash(name, (int)neko_gui_strlen(name), win->property.seq++);
        name++;
    } else
        hash = neko_gui_murmur_hash(name, (int)neko_gui_strlen(name), 42);

    if (win->property.active && hash == win->property.name) {
        buffer = win->property.buffer;
        len = &win->property.length;
        cursor = &win->property.cursor;
        state = &win->property.state;
        select_begin = &win->property.select_start;
        select_end = &win->property.select_end;
    } else {
        buffer = dummy_buffer;
        len = &dummy_length;
        cursor = &dummy_cursor;
        state = &dummy_state;
        select_begin = &dummy_select_begin;
        select_end = &dummy_select_end;
    }

    old_state = *state;
    ctx->text_edit.clip = ctx->clip;
    in = ((s == NEKO_GUI_WIDGET_ROM && !win->property.active) || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    neko_gui_do_property(&ctx->last_widget_state, &win->buffer, bounds, name, variant, inc_per_pixel, buffer, len, state, cursor, select_begin, select_end, &style->property, filter, in, style->font,
                         &ctx->text_edit, ctx->button_behavior);

    if (in && *state != NEKO_GUI_PROPERTY_DEFAULT && !win->property.active) {

        win->property.active = 1;
        NEKO_GUI_MEMCPY(win->property.buffer, buffer, (neko_gui_size)*len);
        win->property.length = *len;
        win->property.cursor = *cursor;
        win->property.state = *state;
        win->property.name = hash;
        win->property.select_start = *select_begin;
        win->property.select_end = *select_end;
        if (*state == NEKO_GUI_PROPERTY_DRAG) {
            ctx->input.mouse.grab = neko_gui_true;
            ctx->input.mouse.grabbed = neko_gui_true;
        }
    }

    if (*state == NEKO_GUI_PROPERTY_DEFAULT && old_state != NEKO_GUI_PROPERTY_DEFAULT) {
        if (old_state == NEKO_GUI_PROPERTY_DRAG) {
            ctx->input.mouse.grab = neko_gui_false;
            ctx->input.mouse.grabbed = neko_gui_false;
            ctx->input.mouse.ungrab = neko_gui_true;
        }
        win->property.select_start = 0;
        win->property.select_end = 0;
        win->property.active = 0;
    }
}
NEKO_GUI_API void neko_gui_property_int(struct neko_gui_context *ctx, const char *name, int min, int *val, int max, int step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);
    NEKO_GUI_ASSERT(val);

    if (!ctx || !ctx->current || !name || !val) return;
    variant = neko_gui_property_variant_int(*val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_INT);
    *val = variant.value.i;
}
NEKO_GUI_API void neko_gui_property_float(struct neko_gui_context *ctx, const char *name, float min, float *val, float max, float step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);
    NEKO_GUI_ASSERT(val);

    if (!ctx || !ctx->current || !name || !val) return;
    variant = neko_gui_property_variant_float(*val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_FLOAT);
    *val = variant.value.f;
}
NEKO_GUI_API void neko_gui_property_double(struct neko_gui_context *ctx, const char *name, double min, double *val, double max, double step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);
    NEKO_GUI_ASSERT(val);

    if (!ctx || !ctx->current || !name || !val) return;
    variant = neko_gui_property_variant_double(*val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_FLOAT);
    *val = variant.value.d;
}
NEKO_GUI_API int neko_gui_propertyi(struct neko_gui_context *ctx, const char *name, int min, int val, int max, int step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);

    if (!ctx || !ctx->current || !name) return val;
    variant = neko_gui_property_variant_int(val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_INT);
    val = variant.value.i;
    return val;
}
NEKO_GUI_API float neko_gui_propertyf(struct neko_gui_context *ctx, const char *name, float min, float val, float max, float step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);

    if (!ctx || !ctx->current || !name) return val;
    variant = neko_gui_property_variant_float(val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_FLOAT);
    val = variant.value.f;
    return val;
}
NEKO_GUI_API double neko_gui_propertyd(struct neko_gui_context *ctx, const char *name, double min, double val, double max, double step, float inc_per_pixel) {
    struct neko_gui_property_variant variant;
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(name);

    if (!ctx || !ctx->current || !name) return val;
    variant = neko_gui_property_variant_double(val, min, max, step);
    neko_gui_property(ctx, name, &variant, inc_per_pixel, NEKO_GUI_FILTER_FLOAT);
    val = variant.value.d;
    return val;
}

NEKO_GUI_API neko_gui_bool neko_gui_chart_begin_colored(struct neko_gui_context *ctx, enum neko_gui_chart_type type, struct neko_gui_color color, struct neko_gui_color highlight, int count,
                                                        float min_value, float max_value) {
    struct neko_gui_window *win;
    struct neko_gui_chart *chart;
    const struct neko_gui_style *config;
    const struct neko_gui_style_chart *style;

    const struct neko_gui_style_item *background;
    struct neko_gui_rect bounds = {0, 0, 0, 0};

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);

    if (!ctx || !ctx->current || !ctx->current->layout) return 0;
    if (!neko_gui_widget(&bounds, ctx)) {
        chart = &ctx->current->layout->chart;
        neko_gui_zero(chart, sizeof(*chart));
        return 0;
    }

    win = ctx->current;
    config = &ctx->style;
    chart = &win->layout->chart;
    style = &config->chart;

    neko_gui_zero(chart, sizeof(*chart));
    chart->x = bounds.x + style->padding.x;
    chart->y = bounds.y + style->padding.y;
    chart->w = bounds.w - 2 * style->padding.x;
    chart->h = bounds.h - 2 * style->padding.y;
    chart->w = NEKO_GUI_MAX(chart->w, 2 * style->padding.x);
    chart->h = NEKO_GUI_MAX(chart->h, 2 * style->padding.y);

    {
        struct neko_gui_chart_slot *slot = &chart->slots[chart->slot++];
        slot->type = type;
        slot->count = count;
        slot->color = color;
        slot->highlight = highlight;
        slot->min = NEKO_GUI_MIN(min_value, max_value);
        slot->max = NEKO_GUI_MAX(min_value, max_value);
        slot->range = slot->max - slot->min;
    }

    background = &style->background;

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(&win->buffer, bounds, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(&win->buffer, bounds, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(&win->buffer, bounds, style->rounding, style->border_color);
            neko_gui_fill_rect(&win->buffer, neko_gui_shrineko_gui_rect(bounds, style->border), style->rounding, style->background.data.color);
            break;
    }
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_chart_begin(struct neko_gui_context *ctx, const enum neko_gui_chart_type type, int count, float min_value, float max_value) {
    return neko_gui_chart_begin_colored(ctx, type, ctx->style.chart.color, ctx->style.chart.selected_color, count, min_value, max_value);
}
NEKO_GUI_API void neko_gui_chart_add_slot_colored(struct neko_gui_context *ctx, const enum neko_gui_chart_type type, struct neko_gui_color color, struct neko_gui_color highlight, int count,
                                                  float min_value, float max_value) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    NEKO_GUI_ASSERT(ctx->current->layout->chart.slot < NEKO_GUI_CHART_MAX_SLOT);
    if (!ctx || !ctx->current || !ctx->current->layout) return;
    if (ctx->current->layout->chart.slot >= NEKO_GUI_CHART_MAX_SLOT) return;

    {
        struct neko_gui_chart *chart = &ctx->current->layout->chart;
        struct neko_gui_chart_slot *slot = &chart->slots[chart->slot++];
        slot->type = type;
        slot->count = count;
        slot->color = color;
        slot->highlight = highlight;
        slot->min = NEKO_GUI_MIN(min_value, max_value);
        slot->max = NEKO_GUI_MAX(min_value, max_value);
        slot->range = slot->max - slot->min;
    }
}
NEKO_GUI_API void neko_gui_chart_add_slot(struct neko_gui_context *ctx, const enum neko_gui_chart_type type, int count, float min_value, float max_value) {
    neko_gui_chart_add_slot_colored(ctx, type, ctx->style.chart.color, ctx->style.chart.selected_color, count, min_value, max_value);
}
NEKO_GUI_INTERN neko_gui_flags neko_gui_chart_push_line(struct neko_gui_context *ctx, struct neko_gui_window *win, struct neko_gui_chart *g, float value, int slot) {
    struct neko_gui_panel *layout = win->layout;
    const struct neko_gui_input *i = &ctx->input;
    struct neko_gui_command_buffer *out = &win->buffer;

    neko_gui_flags ret = 0;
    struct neko_gui_vec2 cur;
    struct neko_gui_rect bounds;
    struct neko_gui_color color;
    float step;
    float range;
    float ratio;

    NEKO_GUI_ASSERT(slot >= 0 && slot < NEKO_GUI_CHART_MAX_SLOT);
    step = g->w / (float)g->slots[slot].count;
    range = g->slots[slot].max - g->slots[slot].min;
    ratio = (value - g->slots[slot].min) / range;

    if (g->slots[slot].index == 0) {

        g->slots[slot].last.x = g->x;
        g->slots[slot].last.y = (g->y + g->h) - ratio * (float)g->h;

        bounds.x = g->slots[slot].last.x - 2;
        bounds.y = g->slots[slot].last.y - 2;
        bounds.w = bounds.h = 4;

        color = g->slots[slot].color;
        if (!(layout->flags & NEKO_GUI_WINDOW_ROM) && NEKO_GUI_INBOX(i->mouse.pos.x, i->mouse.pos.y, g->slots[slot].last.x - 3, g->slots[slot].last.y - 3, 6, 6)) {
            ret = neko_gui_input_is_mouse_hovering_rect(i, bounds) ? NEKO_GUI_CHART_HOVERING : 0;
            ret |= (i->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down && i->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked) ? NEKO_GUI_CHART_CLICKED : 0;
            color = g->slots[slot].highlight;
        }
        neko_gui_fill_rect(out, bounds, 0, color);
        g->slots[slot].index += 1;
        return ret;
    }

    color = g->slots[slot].color;
    cur.x = g->x + (float)(step * (float)g->slots[slot].index);
    cur.y = (g->y + g->h) - (ratio * (float)g->h);
    neko_gui_stroke_line(out, g->slots[slot].last.x, g->slots[slot].last.y, cur.x, cur.y, 1.0f, color);

    bounds.x = cur.x - 3;
    bounds.y = cur.y - 3;
    bounds.w = bounds.h = 6;

    if (!(layout->flags & NEKO_GUI_WINDOW_ROM)) {
        if (neko_gui_input_is_mouse_hovering_rect(i, bounds)) {
            ret = NEKO_GUI_CHART_HOVERING;
            ret |= (!i->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down && i->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked) ? NEKO_GUI_CHART_CLICKED : 0;
            color = g->slots[slot].highlight;
        }
    }
    neko_gui_fill_rect(out, neko_gui_rect(cur.x - 2, cur.y - 2, 4, 4), 0, color);

    g->slots[slot].last.x = cur.x;
    g->slots[slot].last.y = cur.y;
    g->slots[slot].index += 1;
    return ret;
}
NEKO_GUI_INTERN neko_gui_flags neko_gui_chart_push_column(const struct neko_gui_context *ctx, struct neko_gui_window *win, struct neko_gui_chart *chart, float value, int slot) {
    struct neko_gui_command_buffer *out = &win->buffer;
    const struct neko_gui_input *in = &ctx->input;
    struct neko_gui_panel *layout = win->layout;

    float ratio;
    neko_gui_flags ret = 0;
    struct neko_gui_color color;
    struct neko_gui_rect item = {0, 0, 0, 0};

    NEKO_GUI_ASSERT(slot >= 0 && slot < NEKO_GUI_CHART_MAX_SLOT);
    if (chart->slots[slot].index >= chart->slots[slot].count) return neko_gui_false;
    if (chart->slots[slot].count) {
        float padding = (float)(chart->slots[slot].count - 1);
        item.w = (chart->w - padding) / (float)(chart->slots[slot].count);
    }

    color = chart->slots[slot].color;
    ;
    item.h = chart->h * NEKO_GUI_ABS((value / chart->slots[slot].range));
    if (value >= 0) {
        ratio = (value + NEKO_GUI_ABS(chart->slots[slot].min)) / NEKO_GUI_ABS(chart->slots[slot].range);
        item.y = (chart->y + chart->h) - chart->h * ratio;
    } else {
        ratio = (value - chart->slots[slot].max) / chart->slots[slot].range;
        item.y = chart->y + (chart->h * NEKO_GUI_ABS(ratio)) - item.h;
    }
    item.x = chart->x + ((float)chart->slots[slot].index * item.w);
    item.x = item.x + ((float)chart->slots[slot].index);

    if (!(layout->flags & NEKO_GUI_WINDOW_ROM) && NEKO_GUI_INBOX(in->mouse.pos.x, in->mouse.pos.y, item.x, item.y, item.w, item.h)) {
        ret = NEKO_GUI_CHART_HOVERING;
        ret |= (!in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].down && in->mouse.buttons[NEKO_GUI_BUTTON_LEFT].clicked) ? NEKO_GUI_CHART_CLICKED : 0;
        color = chart->slots[slot].highlight;
    }
    neko_gui_fill_rect(out, item, 0, color);
    chart->slots[slot].index += 1;
    return ret;
}
NEKO_GUI_API neko_gui_flags neko_gui_chart_push_slot(struct neko_gui_context *ctx, float value, int slot) {
    neko_gui_flags flags;
    struct neko_gui_window *win;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(slot >= 0 && slot < NEKO_GUI_CHART_MAX_SLOT);
    NEKO_GUI_ASSERT(slot < ctx->current->layout->chart.slot);
    if (!ctx || !ctx->current || slot >= NEKO_GUI_CHART_MAX_SLOT) return neko_gui_false;
    if (slot >= ctx->current->layout->chart.slot) return neko_gui_false;

    win = ctx->current;
    if (win->layout->chart.slot < slot) return neko_gui_false;
    switch (win->layout->chart.slots[slot].type) {
        case NEKO_GUI_CHART_LINES:
            flags = neko_gui_chart_push_line(ctx, win, &win->layout->chart, value, slot);
            break;
        case NEKO_GUI_CHART_COLUMN:
            flags = neko_gui_chart_push_column(ctx, win, &win->layout->chart, value, slot);
            break;
        default:
        case NEKO_GUI_CHART_MAX:
            flags = 0;
    }
    return flags;
}
NEKO_GUI_API neko_gui_flags neko_gui_chart_push(struct neko_gui_context *ctx, float value) { return neko_gui_chart_push_slot(ctx, value, 0); }
NEKO_GUI_API void neko_gui_chart_end(struct neko_gui_context *ctx) {
    struct neko_gui_window *win;
    struct neko_gui_chart *chart;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;

    win = ctx->current;
    chart = &win->layout->chart;
    NEKO_GUI_MEMSET(chart, 0, sizeof(*chart));
    return;
}
NEKO_GUI_API void neko_gui_plot(struct neko_gui_context *ctx, enum neko_gui_chart_type type, const float *values, int count, int offset) {
    int i = 0;
    float min_value;
    float max_value;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(values);
    if (!ctx || !values || !count) return;

    min_value = values[offset];
    max_value = values[offset];
    for (i = 0; i < count; ++i) {
        min_value = NEKO_GUI_MIN(values[i + offset], min_value);
        max_value = NEKO_GUI_MAX(values[i + offset], max_value);
    }

    if (neko_gui_chart_begin(ctx, type, count, min_value, max_value)) {
        for (i = 0; i < count; ++i) neko_gui_chart_push(ctx, values[i + offset]);
        neko_gui_chart_end(ctx);
    }
}
NEKO_GUI_API void neko_gui_plot_function(struct neko_gui_context *ctx, enum neko_gui_chart_type type, void *userdata, float (*value_getter)(void *user, int index), int count, int offset) {
    int i = 0;
    float min_value;
    float max_value;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(value_getter);
    if (!ctx || !value_getter || !count) return;

    max_value = min_value = value_getter(userdata, offset);
    for (i = 0; i < count; ++i) {
        float value = value_getter(userdata, i + offset);
        min_value = NEKO_GUI_MIN(value, min_value);
        max_value = NEKO_GUI_MAX(value, max_value);
    }

    if (neko_gui_chart_begin(ctx, type, count, min_value, max_value)) {
        for (i = 0; i < count; ++i) neko_gui_chart_push(ctx, value_getter(userdata, i + offset));
        neko_gui_chart_end(ctx);
    }
}

NEKO_GUI_LIB neko_gui_bool neko_gui_color_picker_behavior(neko_gui_flags *state, const struct neko_gui_rect *bounds, const struct neko_gui_rect *matrix, const struct neko_gui_rect *hue_bar,
                                                          const struct neko_gui_rect *alpha_bar, struct neko_gui_colorf *color, const struct neko_gui_input *in) {
    float hsva[4];
    neko_gui_bool value_changed = 0;
    neko_gui_bool hsv_changed = 0;

    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(matrix);
    NEKO_GUI_ASSERT(hue_bar);
    NEKO_GUI_ASSERT(color);

    neko_gui_colorf_hsva_fv(hsva, *color);
    if (neko_gui_button_behavior(state, *matrix, in, NEKO_GUI_BUTTON_REPEATER)) {
        hsva[1] = NEKO_GUI_SATURATE((in->mouse.pos.x - matrix->x) / (matrix->w - 1));
        hsva[2] = 1.0f - NEKO_GUI_SATURATE((in->mouse.pos.y - matrix->y) / (matrix->h - 1));
        value_changed = hsv_changed = 1;
    }

    if (neko_gui_button_behavior(state, *hue_bar, in, NEKO_GUI_BUTTON_REPEATER)) {
        hsva[0] = NEKO_GUI_SATURATE((in->mouse.pos.y - hue_bar->y) / (hue_bar->h - 1));
        value_changed = hsv_changed = 1;
    }

    if (alpha_bar) {
        if (neko_gui_button_behavior(state, *alpha_bar, in, NEKO_GUI_BUTTON_REPEATER)) {
            hsva[3] = 1.0f - NEKO_GUI_SATURATE((in->mouse.pos.y - alpha_bar->y) / (alpha_bar->h - 1));
            value_changed = 1;
        }
    }
    neko_gui_widget_state_reset(state);
    if (hsv_changed) {
        *color = neko_gui_hsva_colorfv(hsva);
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
    }
    if (value_changed) {
        color->a = hsva[3];
        *state = NEKO_GUI_WIDGET_STATE_ACTIVE;
    }

    if (neko_gui_input_is_mouse_hovering_rect(in, *bounds)) *state = NEKO_GUI_WIDGET_STATE_HOVERED;
    if (*state & NEKO_GUI_WIDGET_STATE_HOVER && !neko_gui_input_is_mouse_prev_hovering_rect(in, *bounds))
        *state |= NEKO_GUI_WIDGET_STATE_ENTERED;
    else if (neko_gui_input_is_mouse_prev_hovering_rect(in, *bounds))
        *state |= NEKO_GUI_WIDGET_STATE_LEFT;
    return value_changed;
}
NEKO_GUI_LIB void neko_gui_draw_color_picker(struct neko_gui_command_buffer *o, const struct neko_gui_rect *matrix, const struct neko_gui_rect *hue_bar, const struct neko_gui_rect *alpha_bar,
                                             struct neko_gui_colorf col) {
    NEKO_GUI_STORAGE const struct neko_gui_color black = {0, 0, 0, 255};
    NEKO_GUI_STORAGE const struct neko_gui_color white = {255, 255, 255, 255};
    NEKO_GUI_STORAGE const struct neko_gui_color black_trans = {0, 0, 0, 0};

    const float crosshair_size = 7.0f;
    struct neko_gui_color temp;
    float hsva[4];
    float line_y;
    int i;

    NEKO_GUI_ASSERT(o);
    NEKO_GUI_ASSERT(matrix);
    NEKO_GUI_ASSERT(hue_bar);

    neko_gui_colorf_hsva_fv(hsva, col);
    for (i = 0; i < 6; ++i) {
        NEKO_GUI_GLOBAL const struct neko_gui_color hue_colors[] = {{255, 0, 0, 255}, {255, 255, 0, 255}, {0, 255, 0, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 0, 255, 255}, {255, 0, 0, 255}};
        neko_gui_fill_rect_multi_color(o, neko_gui_rect(hue_bar->x, hue_bar->y + (float)i * (hue_bar->h / 6.0f) + 0.5f, hue_bar->w, (hue_bar->h / 6.0f) + 0.5f), hue_colors[i], hue_colors[i],
                                       hue_colors[i + 1], hue_colors[i + 1]);
    }
    line_y = (float)(int)(hue_bar->y + hsva[0] * matrix->h + 0.5f);
    neko_gui_stroke_line(o, hue_bar->x - 1, line_y, hue_bar->x + hue_bar->w + 2, line_y, 1, neko_gui_rgb(255, 255, 255));

    if (alpha_bar) {
        float alpha = NEKO_GUI_SATURATE(col.a);
        line_y = (float)(int)(alpha_bar->y + (1.0f - alpha) * matrix->h + 0.5f);

        neko_gui_fill_rect_multi_color(o, *alpha_bar, white, white, black, black);
        neko_gui_stroke_line(o, alpha_bar->x - 1, line_y, alpha_bar->x + alpha_bar->w + 2, line_y, 1, neko_gui_rgb(255, 255, 255));
    }

    temp = neko_gui_hsv_f(hsva[0], 1.0f, 1.0f);
    neko_gui_fill_rect_multi_color(o, *matrix, white, temp, temp, white);
    neko_gui_fill_rect_multi_color(o, *matrix, black_trans, black_trans, black, black);

    {
        struct neko_gui_vec2 p;
        float S = hsva[1];
        float V = hsva[2];
        p.x = (float)(int)(matrix->x + S * matrix->w);
        p.y = (float)(int)(matrix->y + (1.0f - V) * matrix->h);
        neko_gui_stroke_line(o, p.x - crosshair_size, p.y, p.x - 2, p.y, 1.0f, white);
        neko_gui_stroke_line(o, p.x + crosshair_size + 1, p.y, p.x + 3, p.y, 1.0f, white);
        neko_gui_stroke_line(o, p.x, p.y + crosshair_size + 1, p.x, p.y + 3, 1.0f, white);
        neko_gui_stroke_line(o, p.x, p.y - crosshair_size, p.x, p.y - 2, 1.0f, white);
    }
}
NEKO_GUI_LIB neko_gui_bool neko_gui_do_color_picker(neko_gui_flags *state, struct neko_gui_command_buffer *out, struct neko_gui_colorf *col, enum neko_gui_color_format fmt,
                                                    struct neko_gui_rect bounds, struct neko_gui_vec2 padding, const struct neko_gui_input *in, const struct neko_gui_user_font *font) {
    int ret = 0;
    struct neko_gui_rect matrix;
    struct neko_gui_rect hue_bar;
    struct neko_gui_rect alpha_bar;
    float bar_w;

    NEKO_GUI_ASSERT(out);
    NEKO_GUI_ASSERT(col);
    NEKO_GUI_ASSERT(state);
    NEKO_GUI_ASSERT(font);
    if (!out || !col || !state || !font) return ret;

    bar_w = font->height;
    bounds.x += padding.x;
    bounds.y += padding.x;
    bounds.w -= 2 * padding.x;
    bounds.h -= 2 * padding.y;

    matrix.x = bounds.x;
    matrix.y = bounds.y;
    matrix.h = bounds.h;
    matrix.w = bounds.w - (3 * padding.x + 2 * bar_w);

    hue_bar.w = bar_w;
    hue_bar.y = bounds.y;
    hue_bar.h = matrix.h;
    hue_bar.x = matrix.x + matrix.w + padding.x;

    alpha_bar.x = hue_bar.x + hue_bar.w + padding.x;
    alpha_bar.y = bounds.y;
    alpha_bar.w = bar_w;
    alpha_bar.h = matrix.h;

    ret = neko_gui_color_picker_behavior(state, &bounds, &matrix, &hue_bar, (fmt == NEKO_GUI_RGBA) ? &alpha_bar : 0, col, in);
    neko_gui_draw_color_picker(out, &matrix, &hue_bar, (fmt == NEKO_GUI_RGBA) ? &alpha_bar : 0, *col);
    return ret;
}
NEKO_GUI_API neko_gui_bool neko_gui_color_pick(struct neko_gui_context *ctx, struct neko_gui_colorf *color, enum neko_gui_color_format fmt) {
    struct neko_gui_window *win;
    struct neko_gui_panel *layout;
    const struct neko_gui_style *config;
    const struct neko_gui_input *in;

    enum neko_gui_widget_layout_states state;
    struct neko_gui_rect bounds;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(color);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !color) return 0;

    win = ctx->current;
    config = &ctx->style;
    layout = win->layout;
    state = neko_gui_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NEKO_GUI_WIDGET_ROM || layout->flags & NEKO_GUI_WINDOW_ROM) ? 0 : &ctx->input;
    return neko_gui_do_color_picker(&ctx->last_widget_state, &win->buffer, color, fmt, bounds, neko_gui_vec2(0, 0), in, config->font);
}
NEKO_GUI_API struct neko_gui_colorf neko_gui_color_picker(struct neko_gui_context *ctx, struct neko_gui_colorf color, enum neko_gui_color_format fmt) {
    neko_gui_color_pick(ctx, &color, fmt);
    return color;
}

NEKO_GUI_INTERN neko_gui_bool neko_gui_combo_begin(struct neko_gui_context *ctx, struct neko_gui_window *win, struct neko_gui_vec2 size, neko_gui_bool is_clicked, struct neko_gui_rect header) {
    struct neko_gui_window *popup;
    int is_open = 0;
    int is_active = 0;
    struct neko_gui_rect body;
    neko_gui_hash hash;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    popup = win->popup.win;
    body.x = header.x;
    body.w = size.x;
    body.y = header.y + header.h - ctx->style.window.combo_border;
    body.h = size.y;

    hash = win->popup.combo_count++;
    is_open = (popup) ? neko_gui_true : neko_gui_false;
    is_active = (popup && (win->popup.name == hash) && win->popup.type == NEKO_GUI_PANEL_COMBO);
    if ((is_clicked && is_open && !is_active) || (is_open && !is_active) || (!is_open && !is_active && !is_clicked)) return 0;
    if (!neko_gui_nonblock_begin(ctx, 0, body, (is_clicked && is_open) ? neko_gui_rect(0, 0, 0, 0) : header, NEKO_GUI_PANEL_COMBO)) return 0;

    win->popup.type = NEKO_GUI_PANEL_COMBO;
    win->popup.name = hash;
    return 1;
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_text(struct neko_gui_context *ctx, const char *selected, int len, struct neko_gui_vec2 size) {
    const struct neko_gui_input *in;
    struct neko_gui_window *win;
    struct neko_gui_style *style;

    enum neko_gui_widget_layout_states s;
    int is_clicked = neko_gui_false;
    struct neko_gui_rect header;
    const struct neko_gui_style_item *background;
    struct neko_gui_text text;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(selected);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !selected) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (s == NEKO_GUI_WIDGET_INVALID) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->combo.active;
        text.text = style->combo.label_active;
    } else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->combo.hover;
        text.text = style->combo.label_hover;
    } else {
        background = &style->combo.normal;
        text.text = style->combo.label_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {

        struct neko_gui_rect label;
        struct neko_gui_rect button;
        struct neko_gui_rect content;
        int draw_button_symbol;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        draw_button_symbol = sym != NEKO_GUI_SYMBOL_NONE;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.x;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;

        text.padding = neko_gui_vec2(0, 0);
        label.x = header.x + style->combo.content_padding.x;
        label.y = header.y + style->combo.content_padding.y;
        label.h = header.h - 2 * style->combo.content_padding.y;
        if (draw_button_symbol)
            label.w = button.x - (style->combo.content_padding.x + style->combo.spacing.x) - label.x;
        else
            label.w = header.w - 2 * style->combo.content_padding.x;
        neko_gui_widget_text(&win->buffer, label, selected, len, &text, NEKO_GUI_TEXT_LEFT, ctx->style.font);

        if (draw_button_symbol) neko_gui_draw_button_symbol(&win->buffer, &button, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_label(struct neko_gui_context *ctx, const char *selected, struct neko_gui_vec2 size) {
    return neko_gui_combo_begin_text(ctx, selected, neko_gui_strlen(selected), size);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_color(struct neko_gui_context *ctx, struct neko_gui_color color, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    const struct neko_gui_input *in;

    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    enum neko_gui_widget_layout_states s;
    const struct neko_gui_style_item *background;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (s == NEKO_GUI_WIDGET_INVALID) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        background = &style->combo.active;
    else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
        background = &style->combo.hover;
    else
        background = &style->combo.normal;

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {
        struct neko_gui_rect content;
        struct neko_gui_rect button;
        struct neko_gui_rect bounds;
        int draw_button_symbol;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        draw_button_symbol = sym != NEKO_GUI_SYMBOL_NONE;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.x;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;

        bounds.h = header.h - 4 * style->combo.content_padding.y;
        bounds.y = header.y + 2 * style->combo.content_padding.y;
        bounds.x = header.x + 2 * style->combo.content_padding.x;
        if (draw_button_symbol)
            bounds.w = (button.x - (style->combo.content_padding.x + style->combo.spacing.x)) - bounds.x;
        else
            bounds.w = header.w - 4 * style->combo.content_padding.x;
        neko_gui_fill_rect(&win->buffer, bounds, 0, color);

        if (draw_button_symbol) neko_gui_draw_button_symbol(&win->buffer, &button, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol(struct neko_gui_context *ctx, enum neko_gui_symbol_type symbol, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    const struct neko_gui_input *in;

    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    enum neko_gui_widget_layout_states s;
    const struct neko_gui_style_item *background;
    struct neko_gui_color sym_background;
    struct neko_gui_color symbol_color;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (s == NEKO_GUI_WIDGET_INVALID) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->combo.active;
        symbol_color = style->combo.symbol_active;
    } else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->combo.hover;
        symbol_color = style->combo.symbol_hover;
    } else {
        background = &style->combo.normal;
        symbol_color = style->combo.symbol_hover;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            sym_background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            sym_background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            sym_background = background->data.color;
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {
        struct neko_gui_rect bounds = {0, 0, 0, 0};
        struct neko_gui_rect content;
        struct neko_gui_rect button;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.y;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;

        bounds.h = header.h - 2 * style->combo.content_padding.y;
        bounds.y = header.y + style->combo.content_padding.y;
        bounds.x = header.x + style->combo.content_padding.x;
        bounds.w = (button.x - style->combo.content_padding.y) - bounds.x;
        neko_gui_draw_symbol(&win->buffer, symbol, bounds, sym_background, symbol_color, 1.0f, style->font);

        neko_gui_draw_button_symbol(&win->buffer, &bounds, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol_text(struct neko_gui_context *ctx, const char *selected, int len, enum neko_gui_symbol_type symbol, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    struct neko_gui_input *in;

    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    enum neko_gui_widget_layout_states s;
    const struct neko_gui_style_item *background;
    struct neko_gui_color symbol_color;
    struct neko_gui_text text;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (!s) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->combo.active;
        symbol_color = style->combo.symbol_active;
        text.text = style->combo.label_active;
    } else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->combo.hover;
        symbol_color = style->combo.symbol_hover;
        text.text = style->combo.label_hover;
    } else {
        background = &style->combo.normal;
        symbol_color = style->combo.symbol_normal;
        text.text = style->combo.label_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {
        struct neko_gui_rect content;
        struct neko_gui_rect button;
        struct neko_gui_rect label;
        struct neko_gui_rect image;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.x;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;
        neko_gui_draw_button_symbol(&win->buffer, &button, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);

        image.x = header.x + style->combo.content_padding.x;
        image.y = header.y + style->combo.content_padding.y;
        image.h = header.h - 2 * style->combo.content_padding.y;
        image.w = image.h;
        neko_gui_draw_symbol(&win->buffer, symbol, image, text.background, symbol_color, 1.0f, style->font);

        text.padding = neko_gui_vec2(0, 0);
        label.x = image.x + image.w + style->combo.spacing.x + style->combo.content_padding.x;
        label.y = header.y + style->combo.content_padding.y;
        label.w = (button.x - style->combo.content_padding.x) - label.x;
        label.h = header.h - 2 * style->combo.content_padding.y;
        neko_gui_widget_text(&win->buffer, label, selected, len, &text, NEKO_GUI_TEXT_LEFT, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image(struct neko_gui_context *ctx, struct neko_gui_image img, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    const struct neko_gui_input *in;

    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    enum neko_gui_widget_layout_states s;
    const struct neko_gui_style_item *background;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (s == NEKO_GUI_WIDGET_INVALID) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED)
        background = &style->combo.active;
    else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
        background = &style->combo.hover;
    else
        background = &style->combo.normal;

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {
        struct neko_gui_rect bounds = {0, 0, 0, 0};
        struct neko_gui_rect content;
        struct neko_gui_rect button;
        int draw_button_symbol;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        draw_button_symbol = sym != NEKO_GUI_SYMBOL_NONE;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.y;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;

        bounds.h = header.h - 2 * style->combo.content_padding.y;
        bounds.y = header.y + style->combo.content_padding.y;
        bounds.x = header.x + style->combo.content_padding.x;
        if (draw_button_symbol)
            bounds.w = (button.x - style->combo.content_padding.y) - bounds.x;
        else
            bounds.w = header.w - 2 * style->combo.content_padding.x;
        neko_gui_draw_image(&win->buffer, bounds, &img, neko_gui_white);

        if (draw_button_symbol) neko_gui_draw_button_symbol(&win->buffer, &bounds, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image_text(struct neko_gui_context *ctx, const char *selected, int len, struct neko_gui_image img, struct neko_gui_vec2 size) {
    struct neko_gui_window *win;
    struct neko_gui_style *style;
    struct neko_gui_input *in;

    struct neko_gui_rect header;
    int is_clicked = neko_gui_false;
    enum neko_gui_widget_layout_states s;
    const struct neko_gui_style_item *background;
    struct neko_gui_text text;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    style = &ctx->style;
    s = neko_gui_widget(&header, ctx);
    if (!s) return 0;

    in = (win->layout->flags & NEKO_GUI_WINDOW_ROM || s == NEKO_GUI_WIDGET_ROM) ? 0 : &ctx->input;
    if (neko_gui_button_behavior(&ctx->last_widget_state, header, in, NEKO_GUI_BUTTON_DEFAULT)) is_clicked = neko_gui_true;

    if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_ACTIVED) {
        background = &style->combo.active;
        text.text = style->combo.label_active;
    } else if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER) {
        background = &style->combo.hover;
        text.text = style->combo.label_hover;
    } else {
        background = &style->combo.normal;
        text.text = style->combo.label_normal;
    }

    switch (background->type) {
        case NEKO_GUI_STYLE_ITEM_IMAGE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_image(&win->buffer, header, &background->data.image, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_NINE_SLICE:
            text.background = neko_gui_rgba(0, 0, 0, 0);
            neko_gui_draw_nine_slice(&win->buffer, header, &background->data.slice, neko_gui_white);
            break;
        case NEKO_GUI_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            neko_gui_fill_rect(&win->buffer, header, style->combo.rounding, background->data.color);
            neko_gui_stroke_rect(&win->buffer, header, style->combo.rounding, style->combo.border, style->combo.border_color);
            break;
    }
    {
        struct neko_gui_rect content;
        struct neko_gui_rect button;
        struct neko_gui_rect label;
        struct neko_gui_rect image;
        int draw_button_symbol;

        enum neko_gui_symbol_type sym;
        if (ctx->last_widget_state & NEKO_GUI_WIDGET_STATE_HOVER)
            sym = style->combo.sym_hover;
        else if (is_clicked)
            sym = style->combo.sym_active;
        else
            sym = style->combo.sym_normal;

        draw_button_symbol = sym != NEKO_GUI_SYMBOL_NONE;

        button.w = header.h - 2 * style->combo.button_padding.y;
        button.x = (header.x + header.w - header.h) - style->combo.button_padding.x;
        button.y = header.y + style->combo.button_padding.y;
        button.h = button.w;

        content.x = button.x + style->combo.button.padding.x;
        content.y = button.y + style->combo.button.padding.y;
        content.w = button.w - 2 * style->combo.button.padding.x;
        content.h = button.h - 2 * style->combo.button.padding.y;
        if (draw_button_symbol) neko_gui_draw_button_symbol(&win->buffer, &button, &content, ctx->last_widget_state, &ctx->style.combo.button, sym, style->font);

        image.x = header.x + style->combo.content_padding.x;
        image.y = header.y + style->combo.content_padding.y;
        image.h = header.h - 2 * style->combo.content_padding.y;
        image.w = image.h;
        neko_gui_draw_image(&win->buffer, image, &img, neko_gui_white);

        text.padding = neko_gui_vec2(0, 0);
        label.x = image.x + image.w + style->combo.spacing.x + style->combo.content_padding.x;
        label.y = header.y + style->combo.content_padding.y;
        label.h = header.h - 2 * style->combo.content_padding.y;
        if (draw_button_symbol)
            label.w = (button.x - style->combo.content_padding.x) - label.x;
        else
            label.w = (header.x + header.w - style->combo.content_padding.x) - label.x;
        neko_gui_widget_text(&win->buffer, label, selected, len, &text, NEKO_GUI_TEXT_LEFT, style->font);
    }
    return neko_gui_combo_begin(ctx, win, size, is_clicked, header);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_symbol_label(struct neko_gui_context *ctx, const char *selected, enum neko_gui_symbol_type type, struct neko_gui_vec2 size) {
    return neko_gui_combo_begin_symbol_text(ctx, selected, neko_gui_strlen(selected), type, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_begin_image_label(struct neko_gui_context *ctx, const char *selected, struct neko_gui_image img, struct neko_gui_vec2 size) {
    return neko_gui_combo_begin_image_text(ctx, selected, neko_gui_strlen(selected), img, size);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_text(struct neko_gui_context *ctx, const char *text, int len, neko_gui_flags align) { return neko_gui_contextual_item_text(ctx, text, len, align); }
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_label(struct neko_gui_context *ctx, const char *label, neko_gui_flags align) { return neko_gui_contextual_item_label(ctx, label, align); }
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_image_text(struct neko_gui_context *ctx, struct neko_gui_image img, const char *text, int len, neko_gui_flags alignment) {
    return neko_gui_contextual_item_image_text(ctx, img, text, len, alignment);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_image_label(struct neko_gui_context *ctx, struct neko_gui_image img, const char *text, neko_gui_flags alignment) {
    return neko_gui_contextual_item_image_label(ctx, img, text, alignment);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_symbol_text(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *text, int len, neko_gui_flags alignment) {
    return neko_gui_contextual_item_symbol_text(ctx, sym, text, len, alignment);
}
NEKO_GUI_API neko_gui_bool neko_gui_combo_item_symbol_label(struct neko_gui_context *ctx, enum neko_gui_symbol_type sym, const char *label, neko_gui_flags alignment) {
    return neko_gui_contextual_item_symbol_label(ctx, sym, label, alignment);
}
NEKO_GUI_API void neko_gui_combo_end(struct neko_gui_context *ctx) { neko_gui_contextual_end(ctx); }
NEKO_GUI_API void neko_gui_combo_close(struct neko_gui_context *ctx) { neko_gui_contextual_close(ctx); }
NEKO_GUI_API int neko_gui_combo(struct neko_gui_context *ctx, const char **items, int count, int selected, int item_height, struct neko_gui_vec2 size) {
    int i = 0;
    int max_height;
    struct neko_gui_vec2 item_spacing;
    struct neko_gui_vec2 window_padding;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(items);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !items || !count) return selected;

    item_spacing = ctx->style.window.spacing;
    window_padding = neko_gui_panel_get_padding(&ctx->style, ctx->current->layout->type);
    max_height = count * item_height + count * (int)item_spacing.y;
    max_height += (int)item_spacing.y * 2 + (int)window_padding.y * 2;
    size.y = NEKO_GUI_MIN(size.y, (float)max_height);
    if (neko_gui_combo_begin_label(ctx, items[selected], size)) {
        neko_gui_layout_row_dynamic(ctx, (float)item_height, 1);
        for (i = 0; i < count; ++i) {
            if (neko_gui_combo_item_label(ctx, items[i], NEKO_GUI_TEXT_LEFT)) selected = i;
        }
        neko_gui_combo_end(ctx);
    }
    return selected;
}
NEKO_GUI_API int neko_gui_combo_separator(struct neko_gui_context *ctx, const char *items_separated_by_separator, int separator, int selected, int count, int item_height, struct neko_gui_vec2 size) {
    int i;
    int max_height;
    struct neko_gui_vec2 item_spacing;
    struct neko_gui_vec2 window_padding;
    const char *current_item;
    const char *iter;
    int length = 0;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(items_separated_by_separator);
    if (!ctx || !items_separated_by_separator) return selected;

    item_spacing = ctx->style.window.spacing;
    window_padding = neko_gui_panel_get_padding(&ctx->style, ctx->current->layout->type);
    max_height = count * item_height + count * (int)item_spacing.y;
    max_height += (int)item_spacing.y * 2 + (int)window_padding.y * 2;
    size.y = NEKO_GUI_MIN(size.y, (float)max_height);

    current_item = items_separated_by_separator;
    for (i = 0; i < count; ++i) {
        iter = current_item;
        while (*iter && *iter != separator) iter++;
        length = (int)(iter - current_item);
        if (i == selected) break;
        current_item = iter + 1;
    }

    if (neko_gui_combo_begin_text(ctx, current_item, length, size)) {
        current_item = items_separated_by_separator;
        neko_gui_layout_row_dynamic(ctx, (float)item_height, 1);
        for (i = 0; i < count; ++i) {
            iter = current_item;
            while (*iter && *iter != separator) iter++;
            length = (int)(iter - current_item);
            if (neko_gui_combo_item_text(ctx, current_item, length, NEKO_GUI_TEXT_LEFT)) selected = i;
            current_item = current_item + length + 1;
        }
        neko_gui_combo_end(ctx);
    }
    return selected;
}
NEKO_GUI_API int neko_gui_combo_string(struct neko_gui_context *ctx, const char *items_separated_by_zeros, int selected, int count, int item_height, struct neko_gui_vec2 size) {
    return neko_gui_combo_separator(ctx, items_separated_by_zeros, '\0', selected, count, item_height, size);
}
NEKO_GUI_API int neko_gui_combo_callback(struct neko_gui_context *ctx, void (*item_getter)(void *, int, const char **), void *userdata, int selected, int count, int item_height,
                                         struct neko_gui_vec2 size) {
    int i;
    int max_height;
    struct neko_gui_vec2 item_spacing;
    struct neko_gui_vec2 window_padding;
    const char *item;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(item_getter);
    if (!ctx || !item_getter) return selected;

    item_spacing = ctx->style.window.spacing;
    window_padding = neko_gui_panel_get_padding(&ctx->style, ctx->current->layout->type);
    max_height = count * item_height + count * (int)item_spacing.y;
    max_height += (int)item_spacing.y * 2 + (int)window_padding.y * 2;
    size.y = NEKO_GUI_MIN(size.y, (float)max_height);

    item_getter(userdata, selected, &item);
    if (neko_gui_combo_begin_label(ctx, item, size)) {
        neko_gui_layout_row_dynamic(ctx, (float)item_height, 1);
        for (i = 0; i < count; ++i) {
            item_getter(userdata, i, &item);
            if (neko_gui_combo_item_label(ctx, item, NEKO_GUI_TEXT_LEFT)) selected = i;
        }
        neko_gui_combo_end(ctx);
    }
    return selected;
}
NEKO_GUI_API void neko_gui_combobox(struct neko_gui_context *ctx, const char **items, int count, int *selected, int item_height, struct neko_gui_vec2 size) {
    *selected = neko_gui_combo(ctx, items, count, *selected, item_height, size);
}
NEKO_GUI_API void neko_gui_combobox_string(struct neko_gui_context *ctx, const char *items_separated_by_zeros, int *selected, int count, int item_height, struct neko_gui_vec2 size) {
    *selected = neko_gui_combo_string(ctx, items_separated_by_zeros, *selected, count, item_height, size);
}
NEKO_GUI_API void neko_gui_combobox_separator(struct neko_gui_context *ctx, const char *items_separated_by_separator, int separator, int *selected, int count, int item_height,
                                              struct neko_gui_vec2 size) {
    *selected = neko_gui_combo_separator(ctx, items_separated_by_separator, separator, *selected, count, item_height, size);
}
NEKO_GUI_API void neko_gui_combobox_callback(struct neko_gui_context *ctx, void (*item_getter)(void *data, int id, const char **out_text), void *userdata, int *selected, int count, int item_height,
                                             struct neko_gui_vec2 size) {
    *selected = neko_gui_combo_callback(ctx, item_getter, userdata, *selected, count, item_height, size);
}

NEKO_GUI_API neko_gui_bool neko_gui_tooltip_begin(struct neko_gui_context *ctx, float width) {
    int x, y, w, h;
    struct neko_gui_window *win;
    const struct neko_gui_input *in;
    struct neko_gui_rect bounds;
    int ret;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return 0;

    win = ctx->current;
    in = &ctx->input;
    if (win->popup.win && (win->popup.type & NEKO_GUI_PANEL_SET_NONBLOCK)) return 0;

    w = neko_gui_iceilf(width);
    h = neko_gui_iceilf(neko_gui_null_rect.h);
    x = neko_gui_ifloorf(in->mouse.pos.x + 1) - (int)win->layout->clip.x;
    y = neko_gui_ifloorf(in->mouse.pos.y + 1) - (int)win->layout->clip.y;

    bounds.x = (float)x;
    bounds.y = (float)y;
    bounds.w = (float)w;
    bounds.h = (float)h;

    ret = neko_gui_popup_begin(ctx, NEKO_GUI_POPUP_DYNAMIC, "__##Tooltip##__", NEKO_GUI_WINDOW_NO_SCROLLBAR | NEKO_GUI_WINDOW_BORDER, bounds);
    if (ret) win->layout->flags &= ~(neko_gui_flags)NEKO_GUI_WINDOW_ROM;
    win->popup.type = NEKO_GUI_PANEL_TOOLTIP;
    ctx->current->layout->type = NEKO_GUI_PANEL_TOOLTIP;
    return ret;
}

NEKO_GUI_API void neko_gui_tooltip_end(struct neko_gui_context *ctx) {
    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;
    ctx->current->seq--;
    neko_gui_popup_close(ctx);
    neko_gui_popup_end(ctx);
}
NEKO_GUI_API void neko_gui_tooltip(struct neko_gui_context *ctx, const char *text) {
    const struct neko_gui_style *style;
    struct neko_gui_vec2 padding;

    int text_len;
    float text_width;
    float text_height;

    NEKO_GUI_ASSERT(ctx);
    NEKO_GUI_ASSERT(ctx->current);
    NEKO_GUI_ASSERT(ctx->current->layout);
    NEKO_GUI_ASSERT(text);
    if (!ctx || !ctx->current || !ctx->current->layout || !text) return;

    style = &ctx->style;
    padding = style->window.padding;

    text_len = neko_gui_strlen(text);
    text_width = style->font->width(style->font->userdata, style->font->height, text, text_len);
    text_width += (4 * padding.x);
    text_height = (style->font->height + 2 * padding.y);

    if (neko_gui_tooltip_begin(ctx, (float)text_width)) {
        neko_gui_layout_row_dynamic(ctx, (float)text_height, 1);
        neko_gui_text(ctx, text, text_len, NEKO_GUI_TEXT_LEFT);
        neko_gui_tooltip_end(ctx);
    }
}
#ifdef NEKO_GUI_INCLUDE_STANDARD_VARARGS
NEKO_GUI_API void neko_gui_tooltipf(struct neko_gui_context *ctx, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    neko_gui_tooltipfv(ctx, fmt, args);
    va_end(args);
}
NEKO_GUI_API void neko_gui_tooltipfv(struct neko_gui_context *ctx, const char *fmt, va_list args) {
    char buf[256];
    neko_gui_strfmt(buf, NEKO_GUI_LEN(buf), fmt, args);
    neko_gui_tooltip(ctx, buf);
}
#endif
