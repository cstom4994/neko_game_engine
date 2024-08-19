#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/base.h"
#include "engine/game.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/prelude.h"
#include "engine/ui.h"

#if 1

// 32bit fnv-1a hash
#define UI_HASH_INITIAL 2166136261

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
                {25, 25, 25, 255},     // UI_COLOR_BACKGROUND
                {255, 255, 255, 255},  // UI_COLOR_CONTENT
                {29, 29, 29, 76},      // UI_COLOR_BORDER
                {0, 0, 0, 31},         // UI_COLOR_SHADOW
                {0, 0, 0, 0},          // UI_COLOR_CONTENT_BACKGROUND
                {0, 0, 0, 0},          // UI_COLOR_CONTENT_SHADOW
                {0, 0, 0, 0}           // UI_COLOR_CONTENT_BORDER
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
        UI_DIRECTION_COLUMN,
        UI_JUSTIFY_START,
        UI_ALIGN_CENTER,

        // shadow x, y
        1,
        1};

ui_style_sheet_t ui_default_style_sheet = NEKO_DEFAULT_VAL();

ui_style_t ui_get_current_element_style(ui_context_t* ctx, const ui_selector_desc_t* desc, i32 elementid, i32 state) {

#define UI_APPLY_STYLE(SE)                                               \
    do {                                                                 \
        switch ((SE)->type) {                                            \
            case UI_STYLE_WIDTH:                                         \
                style.size.x = (float)(SE)->value;                       \
                break;                                                   \
            case UI_STYLE_HEIGHT:                                        \
                style.size.y = (float)(SE)->value;                       \
                break;                                                   \
                                                                         \
            case UI_STYLE_PADDING: {                                     \
                style.padding[UI_PADDING_LEFT] = (i32)(SE)->value;       \
                style.padding[UI_PADDING_TOP] = (i32)(SE)->value;        \
                style.padding[UI_PADDING_RIGHT] = (i32)(SE)->value;      \
                style.padding[UI_PADDING_BOTTOM] = (i32)(SE)->value;     \
            }                                                            \
                                                                         \
            case UI_STYLE_PADDING_LEFT:                                  \
                style.padding[UI_PADDING_LEFT] = (i32)(SE)->value;       \
                break;                                                   \
            case UI_STYLE_PADDING_TOP:                                   \
                style.padding[UI_PADDING_TOP] = (i32)(SE)->value;        \
                break;                                                   \
            case UI_STYLE_PADDING_RIGHT:                                 \
                style.padding[UI_PADDING_RIGHT] = (i32)(SE)->value;      \
                break;                                                   \
            case UI_STYLE_PADDING_BOTTOM:                                \
                style.padding[UI_PADDING_BOTTOM] = (i32)(SE)->value;     \
                break;                                                   \
                                                                         \
            case UI_STYLE_MARGIN: {                                      \
                style.margin[UI_MARGIN_LEFT] = (i32)(SE)->value;         \
                style.margin[UI_MARGIN_TOP] = (i32)(SE)->value;          \
                style.margin[UI_MARGIN_RIGHT] = (i32)(SE)->value;        \
                style.margin[UI_MARGIN_BOTTOM] = (i32)(SE)->value;       \
            } break;                                                     \
                                                                         \
            case UI_STYLE_MARGIN_LEFT:                                   \
                style.margin[UI_MARGIN_LEFT] = (i32)(SE)->value;         \
                break;                                                   \
            case UI_STYLE_MARGIN_TOP:                                    \
                style.margin[UI_MARGIN_TOP] = (i32)(SE)->value;          \
                break;                                                   \
            case UI_STYLE_MARGIN_RIGHT:                                  \
                style.margin[UI_MARGIN_RIGHT] = (i32)(SE)->value;        \
                break;                                                   \
            case UI_STYLE_MARGIN_BOTTOM:                                 \
                style.margin[UI_MARGIN_BOTTOM] = (i32)(SE)->value;       \
                break;                                                   \
                                                                         \
            case UI_STYLE_BORDER_RADIUS: {                               \
                style.border_radius[0] = (SE)->value;                    \
                style.border_radius[1] = (SE)->value;                    \
                style.border_radius[2] = (SE)->value;                    \
                style.border_radius[3] = (SE)->value;                    \
            } break;                                                     \
                                                                         \
            case UI_STYLE_BORDER_RADIUS_LEFT:                            \
                style.border_radius[0] = (SE)->value;                    \
                break;                                                   \
            case UI_STYLE_BORDER_RADIUS_RIGHT:                           \
                style.border_radius[1] = (SE)->value;                    \
                break;                                                   \
            case UI_STYLE_BORDER_RADIUS_TOP:                             \
                style.border_radius[2] = (SE)->value;                    \
                break;                                                   \
            case UI_STYLE_BORDER_RADIUS_BOTTOM:                          \
                style.border_radius[3] = (SE)->value;                    \
                break;                                                   \
                                                                         \
            case UI_STYLE_BORDER_WIDTH: {                                \
                style.border_width[0] = (SE)->value;                     \
                style.border_width[1] = (SE)->value;                     \
                style.border_width[2] = (SE)->value;                     \
                style.border_width[3] = (SE)->value;                     \
            } break;                                                     \
                                                                         \
            case UI_STYLE_BORDER_WIDTH_LEFT:                             \
                style.border_width[0] = (SE)->value;                     \
                break;                                                   \
            case UI_STYLE_BORDER_WIDTH_RIGHT:                            \
                style.border_width[1] = (SE)->value;                     \
                break;                                                   \
            case UI_STYLE_BORDER_WIDTH_TOP:                              \
                style.border_width[2] = (SE)->value;                     \
                break;                                                   \
            case UI_STYLE_BORDER_WIDTH_BOTTOM:                           \
                style.border_width[3] = (SE)->value;                     \
                break;                                                   \
                                                                         \
            case UI_STYLE_DIRECTION:                                     \
                style.direction = (i32)(SE)->value;                      \
                break;                                                   \
            case UI_STYLE_ALIGN_CONTENT:                                 \
                style.align_content = (i32)(SE)->value;                  \
                break;                                                   \
            case UI_STYLE_JUSTIFY_CONTENT:                               \
                style.justify_content = (i32)(SE)->value;                \
                break;                                                   \
                                                                         \
            case UI_STYLE_SHADOW_X:                                      \
                style.shadow_x = (i32)(SE)->value;                       \
                break;                                                   \
            case UI_STYLE_SHADOW_Y:                                      \
                style.shadow_y = (i32)(SE)->value;                       \
                break;                                                   \
                                                                         \
            case UI_STYLE_COLOR_BACKGROUND:                              \
                style.colors[UI_COLOR_BACKGROUND] = (SE)->color;         \
                break;                                                   \
            case UI_STYLE_COLOR_BORDER:                                  \
                style.colors[UI_COLOR_BORDER] = (SE)->color;             \
                break;                                                   \
            case UI_STYLE_COLOR_SHADOW:                                  \
                style.colors[UI_COLOR_SHADOW] = (SE)->color;             \
                break;                                                   \
            case UI_STYLE_COLOR_CONTENT:                                 \
                style.colors[UI_COLOR_CONTENT] = (SE)->color;            \
                break;                                                   \
            case UI_STYLE_COLOR_CONTENT_BACKGROUND:                      \
                style.colors[UI_COLOR_CONTENT_BACKGROUND] = (SE)->color; \
                break;                                                   \
            case UI_STYLE_COLOR_CONTENT_BORDER:                          \
                style.colors[UI_COLOR_CONTENT_BORDER] = (SE)->color;     \
                break;                                                   \
            case UI_STYLE_COLOR_CONTENT_SHADOW:                          \
                style.colors[UI_COLOR_CONTENT_SHADOW] = (SE)->color;     \
                break;                                                   \
                                                                         \
            case UI_STYLE_FONT:                                          \
                style.font = (SE)->font;                                 \
                break;                                                   \
        }                                                                \
    } while (0)

    ui_style_t style = ctx->style_sheet->styles[elementid][state];

    // Look for id tag style
    ui_style_list_t* id_styles = NULL;
    ui_style_list_t* cls_styles[UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();

    if (desc) {
        char TMP[256] = NEKO_DEFAULT_VAL();

        // ID selector
        neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const u64 id_hash = neko_hash_str64(TMP);
        id_styles = neko_hash_table_exists(ctx->style_sheet->cid_styles, id_hash) ? neko_hash_table_getp(ctx->style_sheet->cid_styles, id_hash) : NULL;

        // Class selectors
        for (u32 i = 0; i < UI_CLS_SELECTOR_MAX; ++i) {
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
        for (u32 i = 0; i < UI_CLS_SELECTOR_MAX; ++i) {
            if (!cls_styles[i]) break;
            for (u32 s = 0; s < neko_dyn_array_size(cls_styles[i]->styles[state]); ++s) {
                ui_style_element_t* se = &cls_styles[i]->styles[state][s];
                UI_APPLY_STYLE(se);
            }
        }
    }

    // Override with id styles
    if (id_styles) {
        for (u32 i = 0; i < neko_dyn_array_size(id_styles->styles[state]); ++i) {
            ui_style_element_t* se = &id_styles->styles[state][i];
            UI_APPLY_STYLE(se);
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
                UI_APPLY_STYLE(se);
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

    if (anim->direction == UI_ANIMATION_DIRECTION_FORWARD) {
        ret = s1;
    } else {
        ret = s0;
    }

    const ui_animation_property_list_t* list = NULL;
    if (neko_hash_table_exists(ctx->style_sheet->animations, (ui_element_type)elementid)) {
        list = neko_hash_table_getp(ctx->style_sheet->animations, (ui_element_type)elementid);
    }

    const ui_animation_property_list_t* id_list = NULL;
    const ui_animation_property_list_t* cls_list[UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();
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
            for (u32 i = 0; i < UI_CLS_SELECTOR_MAX; ++i) {
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

#define UI_BLEND_COLOR(TYPE)                                                                     \
    do {                                                                                         \
        Color256* c0 = &s0.colors[TYPE];                                                         \
        Color256* c1 = &s1.colors[TYPE];                                                         \
        float r = 255.f * neko_interp_smoothstep((float)c0->r / 255.f, (float)c1->r / 255.f, t); \
        float g = 255.f * neko_interp_smoothstep((float)c0->g / 255.f, (float)c1->g / 255.f, t); \
        float b = 255.f * neko_interp_smoothstep((float)c0->b / 255.f, (float)c1->b / 255.f, t); \
        float a = 255.f * neko_interp_smoothstep((float)c0->a / 255.f, (float)c1->a / 255.f, t); \
        ret.colors[TYPE] = color256((u8)r, (u8)g, (u8)b, (u8)a);                                 \
    } while (0)

#define UI_BLEND_VALUE(FIELD, TYPE)                          \
    do {                                                     \
        float v0 = (float)s0.FIELD;                          \
        float v1 = (float)s1.FIELD;                          \
        ret.FIELD = (TYPE)neko_interp_smoothstep(v0, v1, t); \
    } while (0)

#define UI_BLEND_PROPERTIES(LIST)                                                                                                                      \
    do {                                                                                                                                               \
        for (u32 i = 0; i < neko_dyn_array_size(LIST); ++i) {                                                                                          \
            const ui_animation_property_t* prop = &LIST[i];                                                                                            \
            float t = 0.f;                                                                                                                             \
            switch (anim->direction) {                                                                                                                 \
                default:                                                                                                                               \
                case UI_ANIMATION_DIRECTION_FORWARD: {                                                                                                 \
                    t = NEKO_CLAMP(neko_map_range((float)prop->delay, (float)prop->time + (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f); \
                } break;                                                                                                                               \
                case UI_ANIMATION_DIRECTION_BACKWARD: {                                                                                                \
                    if (prop->time <= 0.f)                                                                                                             \
                        t = 1.f;                                                                                                                       \
                    else                                                                                                                               \
                        t = NEKO_CLAMP(neko_map_range((float)0.f, (float)anim->max - (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f);      \
                } break;                                                                                                                               \
            }                                                                                                                                          \
                                                                                                                                                       \
            switch (prop->type) {                                                                                                                      \
                case UI_STYLE_COLOR_BACKGROUND: {                                                                                                      \
                    UI_BLEND_COLOR(UI_COLOR_BACKGROUND);                                                                                               \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_SHADOW: {                                                                                                          \
                    UI_BLEND_COLOR(UI_COLOR_SHADOW);                                                                                                   \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_BORDER: {                                                                                                          \
                    UI_BLEND_COLOR(UI_COLOR_BORDER);                                                                                                   \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_CONTENT: {                                                                                                         \
                    UI_BLEND_COLOR(UI_COLOR_CONTENT);                                                                                                  \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_CONTENT_BACKGROUND: {                                                                                              \
                    UI_BLEND_COLOR(UI_COLOR_CONTENT_BACKGROUND);                                                                                       \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_CONTENT_SHADOW: {                                                                                                  \
                    UI_BLEND_COLOR(UI_COLOR_CONTENT_SHADOW);                                                                                           \
                } break;                                                                                                                               \
                case UI_STYLE_COLOR_CONTENT_BORDER: {                                                                                                  \
                    UI_BLEND_COLOR(UI_COLOR_CONTENT_BORDER);                                                                                           \
                } break;                                                                                                                               \
                case UI_STYLE_WIDTH: {                                                                                                                 \
                    UI_BLEND_VALUE(size.x, float);                                                                                                     \
                } break;                                                                                                                               \
                case UI_STYLE_HEIGHT: {                                                                                                                \
                    UI_BLEND_VALUE(size.y, float);                                                                                                     \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_WIDTH: {                                                                                                          \
                    UI_BLEND_VALUE(border_width[0], i16);                                                                                              \
                    UI_BLEND_VALUE(border_width[1], i16);                                                                                              \
                    UI_BLEND_VALUE(border_width[2], i16);                                                                                              \
                    UI_BLEND_VALUE(border_width[3], i16);                                                                                              \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_WIDTH_LEFT: {                                                                                                     \
                    UI_BLEND_VALUE(border_width[0], i16);                                                                                              \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_WIDTH_RIGHT: {                                                                                                    \
                    UI_BLEND_VALUE(border_width[1], i16);                                                                                              \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_WIDTH_TOP: {                                                                                                      \
                    UI_BLEND_VALUE(border_width[2], i16);                                                                                              \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_WIDTH_BOTTOM: {                                                                                                   \
                    UI_BLEND_VALUE(border_width[3], i16);                                                                                              \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_RADIUS: {                                                                                                         \
                    UI_BLEND_VALUE(border_radius[0], i16);                                                                                             \
                    UI_BLEND_VALUE(border_radius[1], i16);                                                                                             \
                    UI_BLEND_VALUE(border_radius[2], i16);                                                                                             \
                    UI_BLEND_VALUE(border_radius[3], i16);                                                                                             \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_RADIUS_LEFT: {                                                                                                    \
                    UI_BLEND_VALUE(border_radius[0], i16);                                                                                             \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_RADIUS_RIGHT: {                                                                                                   \
                    UI_BLEND_VALUE(border_radius[1], i16);                                                                                             \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_RADIUS_TOP: {                                                                                                     \
                    UI_BLEND_VALUE(border_radius[2], i16);                                                                                             \
                } break;                                                                                                                               \
                case UI_STYLE_BORDER_RADIUS_BOTTOM: {                                                                                                  \
                    UI_BLEND_VALUE(border_radius[3], i16);                                                                                             \
                } break;                                                                                                                               \
                case UI_STYLE_MARGIN_BOTTOM: {                                                                                                         \
                    UI_BLEND_VALUE(margin[UI_MARGIN_BOTTOM], i16);                                                                                     \
                } break;                                                                                                                               \
                case UI_STYLE_MARGIN_TOP: {                                                                                                            \
                    UI_BLEND_VALUE(margin[UI_MARGIN_TOP], i16);                                                                                        \
                } break;                                                                                                                               \
                case UI_STYLE_MARGIN_LEFT: {                                                                                                           \
                    UI_BLEND_VALUE(margin[UI_MARGIN_LEFT], i16);                                                                                       \
                } break;                                                                                                                               \
                case UI_STYLE_MARGIN_RIGHT: {                                                                                                          \
                    UI_BLEND_VALUE(margin[UI_MARGIN_RIGHT], i16);                                                                                      \
                } break;                                                                                                                               \
                case UI_STYLE_MARGIN: {                                                                                                                \
                    UI_BLEND_VALUE(margin[0], i16);                                                                                                    \
                    UI_BLEND_VALUE(margin[1], i16);                                                                                                    \
                    UI_BLEND_VALUE(margin[2], i16);                                                                                                    \
                    UI_BLEND_VALUE(margin[3], i16);                                                                                                    \
                } break;                                                                                                                               \
                case UI_STYLE_PADDING_BOTTOM: {                                                                                                        \
                    UI_BLEND_VALUE(padding[UI_PADDING_BOTTOM], i32);                                                                                   \
                } break;                                                                                                                               \
                case UI_STYLE_PADDING_TOP: {                                                                                                           \
                    UI_BLEND_VALUE(padding[UI_PADDING_TOP], i32);                                                                                      \
                } break;                                                                                                                               \
                case UI_STYLE_PADDING_LEFT: {                                                                                                          \
                    UI_BLEND_VALUE(padding[UI_PADDING_LEFT], i32);                                                                                     \
                } break;                                                                                                                               \
                case UI_STYLE_PADDING_RIGHT: {                                                                                                         \
                    UI_BLEND_VALUE(padding[UI_PADDING_RIGHT], i32);                                                                                    \
                } break;                                                                                                                               \
                case UI_STYLE_PADDING: {                                                                                                               \
                    UI_BLEND_VALUE(padding[0], i32);                                                                                                   \
                    UI_BLEND_VALUE(padding[1], i32);                                                                                                   \
                    UI_BLEND_VALUE(padding[2], i32);                                                                                                   \
                    UI_BLEND_VALUE(padding[3], i32);                                                                                                   \
                } break;                                                                                                                               \
                case UI_STYLE_SHADOW_X: {                                                                                                              \
                    UI_BLEND_VALUE(shadow_x, i16);                                                                                                     \
                } break;                                                                                                                               \
                case UI_STYLE_SHADOW_Y: {                                                                                                              \
                    UI_BLEND_VALUE(shadow_y, i16);                                                                                                     \
                } break;                                                                                                                               \
            }                                                                                                                                          \
        }                                                                                                                                              \
    } while (0)

    // Get final blends
    if (list && !neko_dyn_array_empty(list->properties[anim->end_state])) {
        UI_BLEND_PROPERTIES(list->properties[anim->end_state]);
    }

    // Class list
    if (has_class_animations) {
        for (u32 c = 0; c < UI_CLS_SELECTOR_MAX; ++c) {
            if (!cls_list[c]) continue;
            if (!neko_dyn_array_empty(cls_list[c]->properties[anim->end_state])) {
                UI_BLEND_PROPERTIES(cls_list[c]->properties[anim->end_state]);
            }
        }
    }

    // Id list
    if (id_list && !neko_dyn_array_empty(id_list->properties[anim->end_state])) {
        UI_BLEND_PROPERTIES(id_list->properties[anim->end_state]);
    }

    if (iss) {
        UI_BLEND_PROPERTIES(iss->animations[anim->end_state]);
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
    ui_animation_property_list_t* cls_list[UI_CLS_SELECTOR_MAX] = NEKO_DEFAULT_VAL();
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
        for (u32 i = 0; i < UI_CLS_SELECTOR_MAX; ++i) {
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
    ui_animation_property_t properties[UI_STYLE_COUNT] = NEKO_DEFAULT_VAL();
    for (u32 i = 0; i < UI_STYLE_COUNT; ++i) {
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
        for (u32 c = 0; c < UI_CLS_SELECTOR_MAX; ++c) {
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
    for (u32 i = 0; i < UI_STYLE_COUNT; ++i) {
        if (properties[i].time > anim->max) anim->max = properties[i].time;
        if (properties[i].delay > anim->delay) anim->delay = properties[i].delay;
    }

    // Finalize time
    anim->max += anim->delay;
    anim->max = NEKO_MAX(anim->max, 5);
}

ui_animation_t* ui_get_animation(ui_context_t* ctx, ui_id id, const ui_selector_desc_t* desc, i32 elementid) {
    ui_animation_t* anim = NULL;

    const bool valid_eid = (elementid >= 0 && elementid < UI_ELEMENT_COUNT);

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
            case UI_ELEMENT_STATE_OFF_FOCUS: {
                if (ctx->hover == id) {
                    anim->direction = UI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = UI_ELEMENT_STATE_HOVER;
                    anim->end_state = UI_ELEMENT_STATE_FOCUS;
                    time_state = UI_ELEMENT_STATE_HOVER;
                    if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                } else {
                    anim->direction = UI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = UI_ELEMENT_STATE_DEFAULT;
                    anim->end_state = UI_ELEMENT_STATE_FOCUS;
                    time_state = UI_ELEMENT_STATE_DEFAULT;
                    if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                }
            } break;

            case UI_ELEMENT_STATE_ON_FOCUS: {
                anim->direction = UI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = UI_ELEMENT_STATE_HOVER;
                anim->end_state = UI_ELEMENT_STATE_FOCUS;
                time_state = UI_ELEMENT_STATE_FOCUS;
                if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;

            case UI_ELEMENT_STATE_OFF_HOVER: {
                anim->direction = UI_ANIMATION_DIRECTION_BACKWARD;
                anim->start_state = UI_ELEMENT_STATE_DEFAULT;
                anim->end_state = UI_ELEMENT_STATE_HOVER;
                time_state = UI_ELEMENT_STATE_DEFAULT;
                if (valid_eid) __ui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = anim->max;
            } break;

            case UI_ELEMENT_STATE_ON_HOVER: {
                anim->direction = UI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = UI_ELEMENT_STATE_DEFAULT;
                anim->end_state = UI_ELEMENT_STATE_HOVER;
                time_state = UI_ELEMENT_STATE_HOVER;
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
            case (UI_ANIMATION_DIRECTION_FORWARD): {
                anim->time += dt;
                if (anim->time >= anim->max) {
                    anim->time = anim->max;
                    anim->playing = false;
                }
            } break;

            case (UI_ANIMATION_DIRECTION_BACKWARD): {
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

ui_rect_t ui_expand_rect(ui_rect_t rect, i16 v[4]) { return ui_rect(rect.x - v[0], rect.y - v[2], rect.w + v[0] + v[1], rect.h + v[2] + v[3]); }

ui_rect_t ui_intersect_rects(ui_rect_t r1, ui_rect_t r2) {
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

i32 ui_rect_overlaps_vec2(ui_rect_t r, vec2 p) { return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h; }

ui_container_t* ui_get_top_most_container(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return NULL;
    if (split->children[0].type == UI_SPLIT_NODE_CONTAINER) return split->children[0].container;
    if (split->children[1].type == UI_SPLIT_NODE_CONTAINER) return split->children[1].container;
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

    if (c0->type == UI_SPLIT_NODE_CONTAINER) {
        ui_bring_to_front(ctx, c0->container);
        // ctx->hover = c0;
    } else {
        ui_split_t* s = neko_slot_array_getp(ctx->splits, c0->split);
        ui_bring_split_to_front(ctx, s);
    }

    if (c1->type == UI_SPLIT_NODE_CONTAINER) {
        ui_bring_to_front(ctx, c1->container);
        // ctx->hover = c1;
    } else {
        ui_split_t* s = neko_slot_array_getp(ctx->splits, c1->split);
        ui_bring_split_to_front(ctx, s);
    }
}

void ui_update_split(ui_context_t* ctx, ui_split_t* split) {
    // Iterate through children, resize them based on size/position
    const ui_rect_t* sr = &split->rect;
    const float ratio = split->ratio;
    switch (split->type) {
        case UI_SPLIT_LEFT: {
            if (split->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x + sr->w * ratio, sr->y, sr->w * (1.f - ratio), sr->h);
                ui_update_split(ctx, s);
            }

            if (split->children[UI_SPLIT_NODE_CHILD].type == UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w * (ratio), sr->h);
                ui_update_split(ctx, s);
            }
        } break;

        case UI_SPLIT_RIGHT: {
            if (split->children[UI_SPLIT_NODE_CHILD].type == UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x + sr->w * (1.f - ratio), sr->y, sr->w * (ratio), sr->h);
                ui_update_split(ctx, s);
            }

            if (split->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w * (1.f - ratio), sr->h);
                ui_update_split(ctx, s);
            }

        } break;

        case UI_SPLIT_TOP: {
            if (split->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y + sr->h * (ratio), sr->w, sr->h * (1.f - ratio));
                ui_update_split(ctx, s);
            }

            if (split->children[UI_SPLIT_NODE_CHILD].type == UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w, sr->h * (ratio));
                ui_update_split(ctx, s);
            }
        } break;

        case UI_SPLIT_BOTTOM: {
            if (split->children[UI_SPLIT_NODE_CHILD].type == UI_SPLIT_NODE_SPLIT) {
                // Update split
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_CHILD].split);
                s->rect = ui_rect(sr->x, sr->y + sr->h * (1.f - ratio), sr->w, sr->h * (ratio));
                ui_update_split(ctx, s);
            }

            if (split->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
                ui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[UI_SPLIT_NODE_PARENT].split);
                s->rect = ui_rect(sr->x, sr->y, sr->w, sr->h * (1.f - ratio));
                ui_update_split(ctx, s);
            }
        } break;
    }
}

ui_split_t* ui_get_root_split_from_split(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return NULL;

    // Cache top root level split
    ui_split_t* root_split = split && split->parent ? neko_slot_array_getp(ctx->splits, split->parent) : split ? split : NULL;
    while (root_split && root_split->parent) {
        root_split = neko_slot_array_getp(ctx->splits, root_split->parent);
    }

    return root_split;
}

ui_split_t* ui_get_root_split(ui_context_t* ctx, ui_container_t* cnt) {
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
        if (s->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
            s = neko_slot_array_getp(ctx->splits, s->children[UI_SPLIT_NODE_PARENT].split);
        } else if (s->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_CONTAINER) {
            c = s->children[UI_SPLIT_NODE_PARENT].container;
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
            if (s->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_SPLIT) {
                s = neko_slot_array_getp(ctx->splits, s->children[UI_SPLIT_NODE_PARENT].split);
            } else if (s->children[UI_SPLIT_NODE_PARENT].type == UI_SPLIT_NODE_CONTAINER) {
                c = s->children[UI_SPLIT_NODE_PARENT].container;
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
    cmd = ui_push_command(ctx, UI_COMMAND_JUMP, sizeof(ui_jumpcommand_t));
    cmd->jump.dst = dst;
    return cmd;
}

void ui_draw_frame(ui_context_t* ctx, ui_rect_t rect, ui_style_t* style) {
    ui_draw_rect(ctx, rect, style->colors[UI_COLOR_BACKGROUND]);

    // draw border
    if (style->colors[UI_COLOR_BORDER].a) {
        ui_draw_box(ctx, ui_expand_rect(rect, (i16*)style->border_width), (i16*)style->border_width, style->colors[UI_COLOR_BORDER]);
    }
}

static i32 ui_compare_zindex(const void* a, const void* b) { return (*(ui_container_t**)a)->zindex - (*(ui_container_t**)b)->zindex; }

ui_style_t* ui_push_style(ui_context_t* ctx, ui_style_t* style) {
    ui_style_t* save = ctx->style;
    ctx->style = style;
    return save;
}

void ui_push_inline_style(ui_context_t* ctx, ui_element_type elementid, ui_inline_style_desc_t* desc) {
    if (elementid >= UI_ELEMENT_COUNT || !desc) {
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

#define UI_COPY_INLINE_STYLE(TYPE, INDEX)                                                  \
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
    UI_COPY_INLINE_STYLE(def, 0);
    UI_COPY_INLINE_STYLE(hover, 1);
    UI_COPY_INLINE_STYLE(focus, 2);

    // Add final counts
    neko_dyn_array_push(iss->style_counts, style_ct[0]);
    neko_dyn_array_push(iss->style_counts, style_ct[1]);
    neko_dyn_array_push(iss->style_counts, style_ct[2]);

    neko_dyn_array_push(iss->animation_counts, anim_ct[0]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[1]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[2]);
}

void ui_pop_inline_style(ui_context_t* ctx, ui_element_type elementid) {
    if (elementid >= UI_ELEMENT_COUNT) {
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

static void ui_push_layout(ui_context_t* ctx, ui_rect_t body, vec2 scroll) {
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

void ui_pop_container(ui_context_t* ctx) {
    ui_container_t* cnt = ui_get_current_container(ctx);
    ui_layout_t* layout = ui_get_layout(ctx);
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;

    // pop container, layout and id
    ui_stack_pop(ctx->container_stack);
    ui_stack_pop(ctx->layout_stack);
    ui_pop_id(ctx);
}

#define ui_scrollbar(ctx, cnt, b, cs, x, y, w, h)                                                                                           \
    do {                                                                                                                                    \
        /* only add scrollbar if content size is larger than body */                                                                        \
        i32 maxscroll = (i32)(cs.y - b->h);                                                                                                 \
                                                                                                                                            \
        if (maxscroll > 0 && b->h > 0) {                                                                                                    \
            ui_rect_t base, thumb;                                                                                                          \
            ui_id id = ui_get_id(ctx, "!scrollbar" #y, 11);                                                                                 \
            const i32 elementid = UI_ELEMENT_SCROLL;                                                                                        \
            ui_style_t style = NEKO_DEFAULT_VAL();                                                                                          \
            ui_animation_t* anim = ui_get_animation(ctx, id, desc, elementid);                                                              \
                                                                                                                                            \
            /* Update anim (keep states locally within animation, only way to do this)*/                                                    \
            if (anim) {                                                                                                                     \
                ui_animation_update(ctx, anim);                                                                                             \
                                                                                                                                            \
                /* Get blended style based on animation*/                                                                                   \
                style = ui_animation_get_blend_style(ctx, anim, desc, elementid);                                                           \
            } else {                                                                                                                        \
                style = ctx->focus == id   ? ui_get_current_element_style(ctx, desc, elementid, 0x02)                                       \
                        : ctx->hover == id ? ui_get_current_element_style(ctx, desc, elementid, 0x01)                                       \
                                           : ui_get_current_element_style(ctx, desc, elementid, 0x00);                                      \
            }                                                                                                                               \
                                                                                                                                            \
            i32 sz = (i32)style.size.x;                                                                                                     \
            if (cs.y > cnt->body.h) {                                                                                                       \
                body->w -= sz;                                                                                                              \
            }                                                                                                                               \
            if (cs.x > cnt->body.w) {                                                                                                       \
                body->h -= sz;                                                                                                              \
            }                                                                                                                               \
                                                                                                                                            \
            /* get sizing / positioning */                                                                                                  \
            base = *b;                                                                                                                      \
            base.x = b->x + b->w;                                                                                                           \
            base.w = style.size.x;                                                                                                          \
                                                                                                                                            \
            /* handle input */                                                                                                              \
            ui_update_control(ctx, id, base, 0);                                                                                            \
            if (ctx->focus == id && ctx->mouse_down == UI_MOUSE_LEFT) {                                                                     \
                cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;                                                                        \
            }                                                                                                                               \
            /* clamp scroll to limits */                                                                                                    \
            cnt->scroll.y = NEKO_CLAMP(cnt->scroll.y, 0, maxscroll);                                                                        \
            i32 state = ctx->focus == id ? UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? UI_ELEMENT_STATE_HOVER : 0x00;                       \
                                                                                                                                            \
            /* draw base and thumb */                                                                                                       \
            ui_draw_rect(ctx, base, style.colors[UI_COLOR_BACKGROUND]);                                                                     \
            /* draw border*/                                                                                                                \
            if (style.colors[UI_COLOR_BORDER].a) {                                                                                          \
                ui_draw_box(ctx, ui_expand_rect(base, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]);  \
            }                                                                                                                               \
            float pl = ((float)style.padding[UI_PADDING_LEFT]);                                                                             \
            float pr = ((float)style.padding[UI_PADDING_RIGHT]);                                                                            \
            float pt = ((float)style.padding[UI_PADDING_TOP]);                                                                              \
            float pb = ((float)style.padding[UI_PADDING_BOTTOM]);                                                                           \
            float w = ((float)base.w - pr);                                                                                                 \
            float x = (float)(base.x + pl);                                                                                                 \
            thumb = base;                                                                                                                   \
            thumb.x = x;                                                                                                                    \
            thumb.w = w;                                                                                                                    \
            thumb.h = NEKO_MAX(style.thumb_size, base.h * b->h / cs.y) - pb;                                                                \
            thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll + pt;                                                                 \
            ui_draw_rect(ctx, thumb, style.colors[UI_COLOR_CONTENT]);                                                                       \
            /* draw border*/                                                                                                                \
            if (style.colors[UI_COLOR_BORDER].a) {                                                                                          \
                ui_draw_box(ctx, ui_expand_rect(thumb, (i16*)style.border_width), (i16*)style.border_width, style.colors[UI_COLOR_BORDER]); \
            }                                                                                                                               \
                                                                                                                                            \
            /* set this as the scroll_target (will get scrolled on mousewheel) */                                                           \
            /* if the mouse is over it */                                                                                                   \
            if (ui_mouse_over(ctx, *b) || ui_mouse_over(ctx, base) || ui_mouse_over(ctx, thumb)) {                                          \
                ctx->scroll_target = cnt;                                                                                                   \
            }                                                                                                                               \
        }                                                                                                                                   \
    } while (0)

static void ui_scrollbars(ui_context_t* ctx, ui_container_t* cnt, ui_rect_t* body, const ui_selector_desc_t* desc, u64 opt) {
    i32 sz = (i32)ctx->style_sheet->styles[UI_ELEMENT_SCROLL][0x00].size.x;
    vec2 cs = cnt->content_size;
    cs.x += ctx->style->padding[UI_PADDING_LEFT] * 2;
    cs.y += ctx->style->padding[UI_PADDING_TOP] * 2;
    ui_push_clip_rect(ctx, *body);

    // resize body to make room for scrollbars
    if (cs.y > cnt->body.h) {
        body->w -= sz;
    }
    if (cs.x > cnt->body.w) {
        body->h -= sz;
    }

    /* to create a horizontal or vertical scrollbar almost-identical code is
    ** used; only the references to `x|y` `w|h` need to be switched */
    ui_scrollbar(ctx, cnt, body, cs, x, y, w, h);

    if (~opt & UI_OPT_NOSCROLLHORIZONTAL) {
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

void ui_push_container_body(ui_context_t* ctx, ui_container_t* cnt, ui_rect_t body, const ui_selector_desc_t* desc, u64 opt) {
    if (~opt & UI_OPT_NOSCROLL) {
        ui_scrollbars(ctx, cnt, &body, desc, opt);
    }
    i32* padding = ctx->style->padding;
    float l = body.x + padding[UI_PADDING_LEFT];
    float t = body.y + padding[UI_PADDING_TOP];
    float r = body.x + body.w - padding[UI_PADDING_RIGHT];
    float b = body.y + body.h - padding[UI_PADDING_BOTTOM];

    ui_rect_t rect = ui_rect(l, t, r - l, b - t);
    ui_push_layout(ctx, rect, cnt->scroll);
    cnt->body = body;
}

void ui_begin_root_container(ui_context_t* ctx, ui_container_t* cnt, u64 opt) {
    ui_stack_push(ctx->container_stack, cnt);

    // push container to roots list and push head command
    ui_stack_push(ctx->root_list, cnt);
    cnt->head = ui_push_jump(ctx, NULL);

    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (ui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex) && ~opt & UI_OPT_NOHOVER && cnt->flags & UI_WINDOW_FLAGS_VISIBLE) {
        ctx->next_hover_root = cnt;
    }

    /* clipping is reset here in case a root-container is made within
    ** another root-containers's begin/end block; this prevents the inner
    ** root-container being clipped to the outer */
    ui_stack_push(ctx->clip_stack, ui_unclipped_rect);
}

void ui_root_container_end(ui_context_t* ctx) {
    /* push tail 'goto' jump command and set head 'skip' command. the final steps
    ** on initing these are done in ui_end() */
    ui_container_t* cnt = ui_get_current_container(ctx);
    cnt->tail = ui_push_jump(ctx, NULL);
    cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;

    // pop base clip rect and container
    ui_pop_clip_rect(ctx);
    ui_pop_container(ctx);
}

#define UI_COPY_STYLES(DST, SRC, ELEM)     \
    do {                                   \
        DST[ELEM][0x00] = SRC[ELEM][0x00]; \
        DST[ELEM][0x01] = SRC[ELEM][0x01]; \
        DST[ELEM][0x02] = SRC[ELEM][0x02]; \
    } while (0)

ui_style_sheet_t ui_style_sheet_create(ui_context_t* ctx, ui_style_sheet_desc_t* desc) {
    // Generate new style sheet based on default element styles
    ui_style_sheet_t style_sheet = NEKO_DEFAULT_VAL();

    // Copy all default styles
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_CONTAINER);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_LABEL);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_TEXT);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_PANEL);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_INPUT);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_BUTTON);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_SCROLL);
    UI_COPY_STYLES(style_sheet.styles, ui_default_style_sheet.styles, UI_ELEMENT_IMAGE);

    //  void ui_style_sheet_set_element_styles(ui_style_sheet_t* style_sheet, ui_element_type element, ui_element_state state,
    // ui_style_element_t* styles, size_t size);

#define UI_APPLY_STYLE_ELEMENT(ELEMENT, TYPE)                                                                                                      \
    do {                                                                                                                                           \
        if ((ELEMENT).all.style.data) {                                                                                                            \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, UI_ELEMENT_STATE_NEG, (ELEMENT).all.style.data, (ELEMENT).all.style.size);       \
        } else if ((ELEMENT).def.style.data) {                                                                                                     \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, UI_ELEMENT_STATE_DEFAULT, (ELEMENT).def.style.data, (ELEMENT).def.style.size);   \
        }                                                                                                                                          \
        if ((ELEMENT).hover.style.data) {                                                                                                          \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, UI_ELEMENT_STATE_HOVER, (ELEMENT).hover.style.data, (ELEMENT).hover.style.size); \
        }                                                                                                                                          \
        if ((ELEMENT).focus.style.data) {                                                                                                          \
            ui_style_sheet_set_element_styles(&style_sheet, TYPE, UI_ELEMENT_STATE_FOCUS, (ELEMENT).focus.style.data, (ELEMENT).focus.style.size); \
        }                                                                                                                                          \
    } while (0)

    // Iterate through descriptor
    if (desc) {
        UI_APPLY_STYLE_ELEMENT(desc->button, UI_ELEMENT_BUTTON);
        UI_APPLY_STYLE_ELEMENT(desc->container, UI_ELEMENT_CONTAINER);
        UI_APPLY_STYLE_ELEMENT(desc->panel, UI_ELEMENT_PANEL);
        UI_APPLY_STYLE_ELEMENT(desc->scroll, UI_ELEMENT_SCROLL);
        UI_APPLY_STYLE_ELEMENT(desc->image, UI_ELEMENT_IMAGE);
        UI_APPLY_STYLE_ELEMENT(desc->label, UI_ELEMENT_LABEL);
        UI_APPLY_STYLE_ELEMENT(desc->text, UI_ELEMENT_TEXT);
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
    COPY_ANIM_DATA(button, UI_ELEMENT_BUTTON);
    COPY_ANIM_DATA(label, UI_ELEMENT_LABEL);
    COPY_ANIM_DATA(scroll, UI_ELEMENT_SCROLL);
    COPY_ANIM_DATA(image, UI_ELEMENT_IMAGE);
    COPY_ANIM_DATA(panel, UI_ELEMENT_PANEL);
    COPY_ANIM_DATA(text, UI_ELEMENT_TEXT);
    COPY_ANIM_DATA(container, UI_ELEMENT_CONTAINER);

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
        case UI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case UI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case UI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        ui_style_t* cs = &ss->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            ui_style_element_t* se = &styles[i];

            switch (se->type) {
                // Width/Height
                case UI_STYLE_WIDTH:
                    cs->size.x = (float)se->value;
                    break;
                case UI_STYLE_HEIGHT:
                    cs->size.y = (float)se->value;
                    break;

                // Padding
                case UI_STYLE_PADDING: {
                    cs->padding[UI_PADDING_LEFT] = (i32)se->value;
                    cs->padding[UI_PADDING_TOP] = (i32)se->value;
                    cs->padding[UI_PADDING_RIGHT] = (i32)se->value;
                    cs->padding[UI_PADDING_BOTTOM] = (i32)se->value;
                }
                case UI_STYLE_PADDING_LEFT:
                    cs->padding[UI_PADDING_LEFT] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_TOP:
                    cs->padding[UI_PADDING_TOP] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_RIGHT:
                    cs->padding[UI_PADDING_RIGHT] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_BOTTOM:
                    cs->padding[UI_PADDING_BOTTOM] = (i32)se->value;
                    break;

                case UI_STYLE_MARGIN: {
                    cs->margin[UI_MARGIN_LEFT] = (i32)se->value;
                    cs->margin[UI_MARGIN_TOP] = (i32)se->value;
                    cs->margin[UI_MARGIN_RIGHT] = (i32)se->value;
                    cs->margin[UI_MARGIN_BOTTOM] = (i32)se->value;
                } break;

                case UI_STYLE_MARGIN_LEFT:
                    cs->margin[UI_MARGIN_LEFT] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_TOP:
                    cs->margin[UI_MARGIN_TOP] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_RIGHT:
                    cs->margin[UI_MARGIN_RIGHT] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_BOTTOM:
                    cs->margin[UI_MARGIN_BOTTOM] = (i32)se->value;
                    break;

                // Border
                case UI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case UI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case UI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case UI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case UI_STYLE_DIRECTION:
                    cs->direction = (i32)se->value;
                    break;
                case UI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (i32)se->value;
                    break;
                case UI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (i32)se->value;
                    break;

                // Shadow
                case UI_STYLE_SHADOW_X:
                    cs->shadow_x = (i32)se->value;
                    break;
                case UI_STYLE_SHADOW_Y:
                    cs->shadow_y = (i32)se->value;
                    break;

                // Colors
                case UI_STYLE_COLOR_BACKGROUND:
                    cs->colors[UI_COLOR_BACKGROUND] = se->color;
                    break;
                case UI_STYLE_COLOR_BORDER:
                    cs->colors[UI_COLOR_BORDER] = se->color;
                    break;
                case UI_STYLE_COLOR_SHADOW:
                    cs->colors[UI_COLOR_SHADOW] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT:
                    cs->colors[UI_COLOR_CONTENT] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[UI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[UI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[UI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case UI_STYLE_FONT:
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
        case UI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case UI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case UI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        ui_style_t* cs = &ctx->style_sheet->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            ui_style_element_t* se = &style[i];

            switch (se->type) {
                // Width/Height
                case UI_STYLE_WIDTH:
                    cs->size.x = (float)se->value;
                    break;
                case UI_STYLE_HEIGHT:
                    cs->size.y = (float)se->value;
                    break;

                // Padding
                case UI_STYLE_PADDING: {
                    cs->padding[UI_PADDING_LEFT] = (i32)se->value;
                    cs->padding[UI_PADDING_TOP] = (i32)se->value;
                    cs->padding[UI_PADDING_RIGHT] = (i32)se->value;
                    cs->padding[UI_PADDING_BOTTOM] = (i32)se->value;
                }
                case UI_STYLE_PADDING_LEFT:
                    cs->padding[UI_PADDING_LEFT] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_TOP:
                    cs->padding[UI_PADDING_TOP] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_RIGHT:
                    cs->padding[UI_PADDING_RIGHT] = (i32)se->value;
                    break;
                case UI_STYLE_PADDING_BOTTOM:
                    cs->padding[UI_PADDING_BOTTOM] = (i32)se->value;
                    break;

                case UI_STYLE_MARGIN: {
                    cs->margin[UI_MARGIN_LEFT] = (i32)se->value;
                    cs->margin[UI_MARGIN_TOP] = (i32)se->value;
                    cs->margin[UI_MARGIN_RIGHT] = (i32)se->value;
                    cs->margin[UI_MARGIN_BOTTOM] = (i32)se->value;
                } break;

                case UI_STYLE_MARGIN_LEFT:
                    cs->margin[UI_MARGIN_LEFT] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_TOP:
                    cs->margin[UI_MARGIN_TOP] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_RIGHT:
                    cs->margin[UI_MARGIN_RIGHT] = (i32)se->value;
                    break;
                case UI_STYLE_MARGIN_BOTTOM:
                    cs->margin[UI_MARGIN_BOTTOM] = (i32)se->value;
                    break;

                // Border
                case UI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case UI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case UI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case UI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case UI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case UI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case UI_STYLE_DIRECTION:
                    cs->direction = (i32)se->value;
                    break;
                case UI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (i32)se->value;
                    break;
                case UI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (i32)se->value;
                    break;

                // Shadow
                case UI_STYLE_SHADOW_X:
                    cs->shadow_x = (i32)se->value;
                    break;
                case UI_STYLE_SHADOW_Y:
                    cs->shadow_y = (i32)se->value;
                    break;

                // Colors
                case UI_STYLE_COLOR_BACKGROUND:
                    cs->colors[UI_COLOR_BACKGROUND] = se->color;
                    break;
                case UI_STYLE_COLOR_BORDER:
                    cs->colors[UI_COLOR_BORDER] = se->color;
                    break;
                case UI_STYLE_COLOR_SHADOW:
                    cs->colors[UI_COLOR_SHADOW] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT:
                    cs->colors[UI_COLOR_CONTENT] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[UI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[UI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case UI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[UI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case UI_STYLE_FONT:
                    cs->font = se->font;
                    break;
            }
        }
    }
}

ui_container_t* ui_get_container_ex(ui_context_t* ctx, ui_id id, u64 opt) {
    ui_container_t* cnt;

    // try to get existing container from pool
    i32 idx = ui_pool_get(ctx, ctx->container_pool, UI_CONTAINERPOOL_SIZE, id);

    if (idx >= 0) {
        if (ctx->containers[idx].open || ~opt & UI_OPT_CLOSED) {
            ui_pool_update(ctx, ctx->container_pool, idx);
        }
        return &ctx->containers[idx];
    }

    if (opt & UI_OPT_CLOSED) {
        return NULL;
    }

    // container not found in pool: init new container
    idx = ui_pool_init(ctx, ctx->container_pool, UI_CONTAINERPOOL_SIZE, id);
    cnt = &ctx->containers[idx];
    memset(cnt, 0, sizeof(*cnt));
    cnt->open = 1;
    cnt->id = id;
    cnt->flags |= UI_WINDOW_FLAGS_VISIBLE | UI_WINDOW_FLAGS_FIRST_INIT;
    ui_bring_to_front(ctx, cnt);

    // neko_println("CONSTRUCTING: %zu", id);

    return cnt;
}

i32 ui_text_width(FontFamily* font, const char* text, i32 len) {
    // vec2 td = neko_asset_font_text_dimensions(font, String(text, len));
    return ((len > 0) ? len : neko_strlen(text)) * default_font_size / 1.5f;
}

// 获取给定字体的最大高度
i32 ui_font_height(FontFamily* font) { return default_font_size * 1.4f; }

i32 ui_text_height(FontFamily* font, const char* text, i32 len) {
    // return (i32)neko_asset_font_max_height(font);
    // vec2 td = neko_asset_font_text_dimensions(font, String(text, len));
    return ui_font_height(font);
}

vec2 ui_text_dimensions(FontFamily* font, const char* text, i32 len) { return neko_v2(ui_text_width(font, text, len), ui_text_height(font, text, len)); }

// =========================== //
// ======== Docking ========== //
// =========================== //

void ui_dock_ex(ui_context_t* ctx, const char* dst, const char* src, i32 split_type, float ratio) {
    ui_hints_t hints = NEKO_DEFAULT_VAL();
    // hints.framebuffer_size = neko_os_framebuffer_sizev(ctx->window_hndl);
    LuaVec2 fb_size = game_get_window_size();
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

    if (split_type == UI_SPLIT_TAB) {
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
        c0.type = UI_SPLIT_NODE_CONTAINER;
        c0.container = child;
        ui_split_node_t c1 = NEKO_DEFAULT_VAL();
        c1.type = UI_SPLIT_NODE_CONTAINER;
        c1.container = parent;
        split.children[UI_SPLIT_NODE_CHILD] = c0;
        split.children[UI_SPLIT_NODE_PARENT] = c1;
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
            sp->children[UI_SPLIT_NODE_CHILD].type = UI_SPLIT_NODE_SPLIT;
            sp->children[UI_SPLIT_NODE_CHILD].split = cs->id;

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
            if (ps->children[UI_SPLIT_NODE_PARENT].container == parent) {
                ps->children[UI_SPLIT_NODE_PARENT].type = UI_SPLIT_NODE_SPLIT;
                ps->children[UI_SPLIT_NODE_PARENT].split = hndl;
            } else {
                ps->children[UI_SPLIT_NODE_CHILD].type = UI_SPLIT_NODE_SPLIT;
                ps->children[UI_SPLIT_NODE_CHILD].split = hndl;
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
            sp->children[UI_SPLIT_NODE_CHILD].type = UI_SPLIT_NODE_SPLIT;
            sp->children[UI_SPLIT_NODE_CHILD].split = cs->id;

            // Need to check which node to replace
            if (ps->children[UI_SPLIT_NODE_CHILD].container == parent) {
                ps->children[UI_SPLIT_NODE_CHILD].type = UI_SPLIT_NODE_SPLIT;
                ps->children[UI_SPLIT_NODE_CHILD].split = hndl;
            } else {
                ps->children[UI_SPLIT_NODE_PARENT].type = UI_SPLIT_NODE_SPLIT;
                ps->children[UI_SPLIT_NODE_PARENT].split = hndl;
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
                    fcnt->flags |= UI_WINDOW_FLAGS_VISIBLE;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == UI_SPLIT_NODE_CONTAINER && split->children[0].container == cnt) {
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
                    fcnt->flags |= UI_WINDOW_FLAGS_VISIBLE;
                }

                // Fix up split reference
                if (parent == cnt) {
                    tab_bar->focus = idx ? idx - 1 : idx;

                    neko_assert((ui_container_t*)tab_bar->items[tab_bar->focus].data != cnt);

                    ui_container_t* fcnt = (ui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == UI_SPLIT_NODE_CONTAINER && split->children[0].container == parent) {
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
                    fcnt->flags |= UI_WINDOW_FLAGS_VISIBLE;
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

        ui_split_node_t* remain_node = split->children[UI_SPLIT_NODE_CHILD].container == cnt    ? &split->children[UI_SPLIT_NODE_PARENT]
                                       : split->children[UI_SPLIT_NODE_PARENT].container == cnt ? &split->children[UI_SPLIT_NODE_CHILD]
                                                                                                : NULL;

        neko_assert(remain_node);

        // Set child split in prev container split to split container parent
        if (ps) {
            u32 node_id = ps->children[UI_SPLIT_NODE_CHILD].split == split->id ? UI_SPLIT_NODE_CHILD : UI_SPLIT_NODE_PARENT;
            ui_split_node_t* fix_node = &ps->children[node_id];
            *fix_node = *remain_node;
            switch (fix_node->type) {
                case UI_SPLIT_NODE_CONTAINER: {
                    ui_container_t* remcnt = fix_node->container;
                    remcnt->split = ps->id;
                } break;

                case UI_SPLIT_NODE_SPLIT: {
                    ui_split_t* remsplit = neko_slot_array_getp(ctx->splits, fix_node->split);
                    remsplit->parent = ps->id;
                } break;
            }

            root_split = ui_get_root_split_from_split(ctx, ps);
        }
        // Otherwise, we were root dock and have to treat that case for remaining nodes
        else {
            switch (remain_node->type) {
                case UI_SPLIT_NODE_CONTAINER: {
                    ui_container_t* remcnt = remain_node->container;
                    remcnt->rect = split->rect;
                    remcnt->split = 0x00;
                    root_split = ui_get_root_split(ctx, remcnt);
                } break;

                case UI_SPLIT_NODE_SPLIT: {
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

#define UI_COPY_STYLE(DST, SRC)              \
    do {                                     \
        DST[UI_ELEMENT_STATE_DEFAULT] = SRC; \
        DST[UI_ELEMENT_STATE_HOVER] = SRC;   \
        DST[UI_ELEMENT_STATE_FOCUS] = SRC;   \
    } while (0)

static void ui_init_default_styles(ui_context_t* ctx) {
    // Set up main default style
    ui_default_style.font = neko_default_font();
    ui_default_style.size.x = 68.f;
    ui_default_style.size.y = 18.f;
    ui_default_style.spacing = 2;
    ui_default_style.indent = 10;
    ui_default_style.title_height = 20;
    ui_default_style.scrollbar_size = 5;
    ui_default_style.thumb_size = 5;

    // Initialize all default styles
    UI_COPY_STYLE(ui_default_container_style, ui_default_style);
    UI_COPY_STYLE(ui_default_button_style, ui_default_style);
    UI_COPY_STYLE(ui_default_text_style, ui_default_style);
    UI_COPY_STYLE(ui_default_label_style, ui_default_style);
    UI_COPY_STYLE(ui_default_panel_style, ui_default_style);
    UI_COPY_STYLE(ui_default_input_style, ui_default_style);
    UI_COPY_STYLE(ui_default_scroll_style, ui_default_style);
    UI_COPY_STYLE(ui_default_image_style, ui_default_style);

    ui_style_t* style = NULL;

    // button
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_button_style[i];
        style->justify_content = UI_JUSTIFY_CENTER;
    }
    ui_default_button_style[UI_ELEMENT_STATE_DEFAULT].colors[UI_COLOR_BACKGROUND] = color256(35, 35, 35, 255);
    ui_default_button_style[UI_ELEMENT_STATE_HOVER].colors[UI_COLOR_BACKGROUND] = color256(40, 40, 40, 255);
    ui_default_button_style[UI_ELEMENT_STATE_FOCUS].colors[UI_COLOR_BACKGROUND] = color256(0, 214, 121, 255);

    // panel
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_panel_style[i];
        style->colors[UI_COLOR_BACKGROUND] = color256(30, 30, 30, 160);
        style->size.y = 19;
    }

    // input
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_input_style[i];
        style->colors[UI_COLOR_BACKGROUND] = color256(20, 20, 20, 255);
        style->size.y = 19;
    }

    // text
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_text_style[i];
        style->colors[UI_COLOR_BACKGROUND] = color256(0, 0, 0, 0);
        style->colors[UI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
    }

    // label
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_label_style[i];
        style->colors[UI_COLOR_BACKGROUND] = color256(0, 0, 0, 0);
        style->colors[UI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
        style->size.y = 19;
    }

    // scroll
    for (u32 i = 0; i < 3; ++i) {
        style = &ui_default_scroll_style[i];
        style->size.x = 10;
        style->padding[UI_PADDING_RIGHT] = 4;
    }

    style = &ui_default_scroll_style[UI_ELEMENT_STATE_DEFAULT];
    style->colors[UI_COLOR_BACKGROUND] = color256(22, 22, 22, 255);
    style->colors[UI_COLOR_CONTENT] = color256(255, 255, 255, 100);

#define UI_COPY(DST, SRC)      \
    do {                       \
        DST[0x00] = SRC[0x00]; \
        DST[0x01] = SRC[0x01]; \
        DST[0x02] = SRC[0x02]; \
    } while (0)

    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_CONTAINER], ui_default_container_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_LABEL], ui_default_label_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_TEXT], ui_default_text_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_PANEL], ui_default_panel_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_INPUT], ui_default_input_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_BUTTON], ui_default_button_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_SCROLL], ui_default_scroll_style);
    UI_COPY(ui_default_style_sheet.styles[UI_ELEMENT_IMAGE], ui_default_image_style);

    ctx->style_sheet = &ui_default_style_sheet;
    ctx->style = &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][0x00];
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
    neko_slot_array_reserve(ctx->splits, UI_UI_SPLIT_SIZE);
    ui_split_t split = NEKO_DEFAULT_VAL();
    neko_slot_array_insert(ctx->splits, split);  // 第一项设置为 0x00 无效
    neko_slot_array_reserve(ctx->tab_bars, UI_UI_TAB_SIZE);
    ui_tab_bar_t tb = NEKO_DEFAULT_VAL();
    neko_slot_array_insert(ctx->tab_bars, tb);

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

void ui_draw_splits(ui_context_t* ctx, ui_split_t* split) {
    if (!split) return;

    ui_split_t* root_split = ui_get_root_split_from_split(ctx, split);

    // Draw split
    const ui_rect_t* sr = &split->rect;
    vec2 hd = neko_v2(sr->w * 0.5f, sr->h * 0.5f);
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
        if (split->children[i].type == UI_SPLIT_NODE_CONTAINER && can_draw) {
            ui_container_t* cnt = split->children[i].container;

            // Don't draw split if this container belongs to a editor_dockspace
            if (cnt->opt & UI_OPT_DOCKSPACE) {
                can_draw = false;
                continue;
            }

            switch (split->type) {
                case UI_SPLIT_LEFT: {
                    r = ui_rect(sr->x + sr->w * ratio - UI_SPLIT_SIZE * 0.5f, sr->y + UI_SPLIT_SIZE, UI_SPLIT_SIZE, sr->h - 2.f * UI_SPLIT_SIZE);
                } break;

                case UI_SPLIT_RIGHT: {
                    r = ui_rect(sr->x + sr->w * (1.f - ratio) - UI_SPLIT_SIZE * 0.5f, sr->y + UI_SPLIT_SIZE, UI_SPLIT_SIZE, sr->h - 2.f * UI_SPLIT_SIZE);
                } break;

                case UI_SPLIT_TOP: {
                    r = ui_rect(sr->x + UI_SPLIT_SIZE, sr->y + sr->h * (ratio)-UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * UI_SPLIT_SIZE, UI_SPLIT_SIZE);
                } break;

                case UI_SPLIT_BOTTOM: {
                    r = ui_rect(sr->x + UI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * UI_SPLIT_SIZE, UI_SPLIT_SIZE);
                } break;
            }

            i16 exp[4] = {1, 1, 1, 1};
            ui_rect_t expand = ui_expand_rect(r, exp);
            bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && ui_rect_overlaps_vec2(expand, ctx->mouse_pos) && !ctx->lock_hover_id;
            if (hover) ctx->next_hover_split = split;
            if (hover && ctx->mouse_down == UI_MOUSE_LEFT) {
                if (!ctx->focus_split) ctx->focus_split = split;
            }
            bool active = ctx->focus_split == split;
            if (active && valid) {
                ctx->next_hover_root = top;
                ui_request_t req = NEKO_DEFAULT_VAL();
                req.type = UI_SPLIT_RATIO;
                req.split = split;
                neko_dyn_array_push(ctx->requests, req);
            }

            if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                Color256 bc = ctx->focus_split == split ? ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_FOCUS].colors[UI_COLOR_BACKGROUND]
                                                        : ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_HOVER].colors[UI_COLOR_BACKGROUND];
                ui_draw_rect(ctx, r, bc);
                can_draw = false;
            }
        } else if (split->children[i].type == UI_SPLIT_NODE_SPLIT) {
            if (can_draw) {
                switch (split->type) {
                    case UI_SPLIT_LEFT: {
                        r = ui_rect(sr->x + sr->w * ratio - UI_SPLIT_SIZE * 0.5f, sr->y + UI_SPLIT_SIZE, UI_SPLIT_SIZE, sr->h - 2.f * UI_SPLIT_SIZE);
                    } break;

                    case UI_SPLIT_RIGHT: {
                        r = ui_rect(sr->x + sr->w * (1.f - ratio) - UI_SPLIT_SIZE * 0.5f, sr->y + UI_SPLIT_SIZE, UI_SPLIT_SIZE, sr->h - 2.f * UI_SPLIT_SIZE);
                    } break;

                    case UI_SPLIT_TOP: {
                        r = ui_rect(sr->x + UI_SPLIT_SIZE, sr->y + sr->h * (ratio)-UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * UI_SPLIT_SIZE, UI_SPLIT_SIZE);
                    } break;

                    case UI_SPLIT_BOTTOM: {
                        r = ui_rect(sr->x + UI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - UI_SPLIT_SIZE * 0.5f, sr->w - 2.f * UI_SPLIT_SIZE, UI_SPLIT_SIZE);
                    } break;
                }

                i16 exp[] = {1, 1, 1, 1};
                ui_rect_t expand = ui_expand_rect(r, exp);
                bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && ui_rect_overlaps_vec2(expand, ctx->mouse_pos);
                if (hover) ctx->next_hover_split = split;
                if (hover && ctx->mouse_down == UI_MOUSE_LEFT) {
                    if (!ctx->focus_split) ctx->focus_split = split;
                }
                bool active = ctx->focus_split == split;
                if (active) {
                    ctx->next_hover_root = top;
                    ui_request_t req = NEKO_DEFAULT_VAL();
                    req.type = UI_SPLIT_RATIO;
                    req.split = split;
                    neko_dyn_array_push(ctx->requests, req);
                }

                if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                    Color256 bc = ctx->focus_split == split ? ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_FOCUS].colors[UI_COLOR_BACKGROUND]
                                                            : ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_HOVER].colors[UI_COLOR_BACKGROUND];
                    ui_draw_rect(ctx, r, bc);
                    can_draw = false;
                }
            }

            ui_split_t* child = neko_slot_array_getp(ctx->splits, split->children[i].split);
            ui_draw_splits(ctx, child);
        }
    }
    if (ctx->focus_split == split && ctx->mouse_down != UI_MOUSE_LEFT) {
        ctx->focus_split = NULL;
    }
}

void ui_get_split_lowest_zindex(ui_context_t* ctx, ui_split_t* split, i32* index) {
    if (!split) return;

    if (split->children[0].type == UI_SPLIT_NODE_CONTAINER && split->children[0].container->zindex < *index) {
        *index = split->children[0].container->zindex;
    }
    if (split->children[0].type == UI_SPLIT_NODE_CONTAINER && split->children[0].container->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[0].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((ui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[1].type == UI_SPLIT_NODE_CONTAINER && split->children[1].container->zindex < *index) {
        *index = split->children[1].container->zindex;
    }
    if (split->children[1].type == UI_SPLIT_NODE_CONTAINER && split->children[1].container->tab_bar) {
        ui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[1].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((ui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((ui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[0].type == UI_SPLIT_NODE_SPLIT) {
        ui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[0].split), index);
    }

    if (split->children[1].type == UI_SPLIT_NODE_SPLIT) {
        ui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[1].split), index);
    }
}

void ui_begin(ui_context_t* ctx, const ui_hints_t* hints) {
    ui_hints_t default_hints = NEKO_DEFAULT_VAL();
    // default_hints.framebuffer_size = neko_os_framebuffer_sizev(ctx->window_hndl);
    LuaVec2 fb_size = game_get_window_size();
    default_hints.framebuffer_size = {fb_size.x, fb_size.y};
    default_hints.viewport = ui_rect(0.f, 0.f, default_hints.framebuffer_size.x, default_hints.framebuffer_size.y);
    ui_hints_t hint = hints ? *hints : default_hints;

    // Set up viewport
    ui_rect_t vp = hint.viewport;
    if (~hint.flags & UI_HINT_FLAG_NO_INVERT_Y) {
        vp.y = hint.viewport.y;
    } else {
        vp.y = hint.framebuffer_size.y - hint.viewport.h - hint.viewport.y;
    }

    // 捕获事件信息
    auto mouse_pos_tmp = input_get_mouse_pos_pixels_fix();
    vec2 mouse_pos = {mouse_pos_tmp.x, mouse_pos_tmp.y};

    // Check for scale and bias
    if (hint.flags & UI_HINT_FLAG_NO_SCALE_BIAS_MOUSE) {
        float px = (mouse_pos.x - vp.x) / vp.w;
        float py = (mouse_pos.y - vp.y) / vp.h;
        float xv = vp.w * px;
        float yv = vp.h * py;
        mouse_pos = neko_v2(xv, yv);
    } else {
        vec2 fb_vp_ratio = neko_v2(hint.framebuffer_size.x / vp.w, hint.framebuffer_size.y / vp.h);
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
                case INPUT_WRAP_KEY_PRESSED:
                case INPUT_WRAP_KEY_REPEATED:
                    ui_input_keydown(ctx, e.keyboard.key);
                    break;
                case INPUT_WRAP_KEY_RELEASED:
                    ui_input_keyup(ctx, e.keyboard.key);
                    break;
                case INPUT_WRAP_CODEPOINT_INPUT: {
                    char txt[2] = {(char)(e.codepoint & 255), 0};
                    ui_input_text(ctx, txt);
                } break;
                case INPUT_WRAP_BUTTON_PRESSED: {
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
    vec2 fbs = ctx->framebuffer_size;
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
            u64 opt = UI_OPT_NOFRAME | UI_OPT_FORCESETRECT | UI_OPT_NOMOVE | UI_OPT_NOTITLE | UI_OPT_NOSCROLL | UI_OPT_NOCLIP | UI_OPT_NODOCK | UI_OPT_DOCKSPACE | UI_OPT_NOBORDER;
            ui_window_begin_ex(ctx, TMP, r, NULL, NULL, opt);
            {
                // Set zindex for sorting (always below the bottom most window in this split tree)
                ui_container_t* ds = ui_get_current_container(ctx);
                i32 zindex = INT32_MAX - 1;
                ui_get_split_lowest_zindex(ctx, split, &zindex);
                if (root_cnt->opt & UI_OPT_DOCKSPACE)
                    ds->zindex = 0;
                else
                    ds->zindex = NEKO_CLAMP((i32)zindex - 1, 0, INT32_MAX);

                ui_rect_t fr = split->rect;
                fr.x += UI_SPLIT_SIZE;
                fr.y += UI_SPLIT_SIZE;
                fr.w -= 2.f * UI_SPLIT_SIZE;
                fr.h -= 2.f * UI_SPLIT_SIZE;
                // ui_draw_frame(ctx, fr, &ctx->style_sheet->styles[UI_ELEMENT_CONTAINER][0x00]);

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
                    ui_rect_t lr = ui_rect(fr.x - 2.f * UI_SPLIT_SIZE, fr.y, UI_SPLIT_SIZE, fr.h);
                    ui_rect_t ex = lr;
                    ex.x -= 10.f;
                    ex.w += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_l", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, lr, UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = UI_SPLIT_RESIZE_W;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // E
                {
                    // Cache rect
                    ui_rect_t rr = ui_rect(fr.x + fr.w + UI_SPLIT_SIZE, fr.y, UI_SPLIT_SIZE, fr.h);
                    ui_rect_t ex = rr;
                    ex.w += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_r", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, rr, UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = UI_SPLIT_RESIZE_E;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // N
                {
                    // Cache rect
                    ui_rect_t tr = ui_rect(fr.x, fr.y - 2.f * UI_SPLIT_SIZE, fr.w, UI_SPLIT_SIZE);
                    ui_rect_t ex = tr;
                    ex.y -= 10.f;
                    ex.h += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_t", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, tr, UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = UI_SPLIT_RESIZE_N;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // S
                {
                    // Cache rect
                    ui_rect_t br = ui_rect(fr.x, fr.y + fr.h + UI_SPLIT_SIZE, fr.w, UI_SPLIT_SIZE);
                    ui_rect_t ex = br;
                    ex.h += 20.f;
                    ui_id id = ui_get_id(ctx, "!hov_b", 6);
                    ui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == UI_MOUSE_LEFT) {
                        ui_draw_control_frame(ctx, id, br, UI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        ui_request_t req = NEKO_DEFAULT_VAL();
                        req.type = UI_SPLIT_RESIZE_S;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }
            }
            ui_window_end(ctx);
        }
    }

    if (ctx->mouse_down != UI_MOUSE_LEFT) {
        ctx->lock_focus = 0x00;
    }
}

static void ui_docking(ui_context_t* ctx) {
    if (ctx->undock_root) {
        ui_undock_ex_cnt(ctx, ctx->undock_root);
    }

    if (!ctx->focus_root || ctx->focus_root->opt & UI_OPT_NODOCK) return;

    if (ctx->dockable_root || ctx->prev_dockable_root) {
        ui_container_t* cnt = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;

        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != UI_MOUSE_LEFT) {
            i32 b = 0;
        }

        // Cache hoverable tile information
        vec2 c = neko_v2(cnt->rect.x + cnt->rect.w / 2.f, cnt->rect.y + cnt->rect.h / 2.f);

        const float sz = NEKO_CLAMP(NEKO_MIN(cnt->rect.w * 0.1f, cnt->rect.h * 0.1f), 15.f, 25.f);
        const float off = sz + sz * 0.2f;
        Color256 def_col = color256_alpha(ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_FOCUS].colors[UI_COLOR_BACKGROUND], 100);
        Color256 hov_col = color256_alpha(ctx->style_sheet->styles[UI_ELEMENT_BUTTON][UI_ELEMENT_STATE_FOCUS].colors[UI_COLOR_BACKGROUND], 150);

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
        if (ctx->focus_root->opt & UI_OPT_DOCKSPACE) {
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
            idraw_t* dl = &ctx->overlay_draw_list;

            bool is_dockspace = ctx->dockable_root->opt & UI_OPT_DOCKSPACE;

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
        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != UI_MOUSE_LEFT) {
            ui_container_t* parent = ctx->prev_dockable_root;
            ui_container_t* child = ctx->focus_root;

            bool is_dockspace = ctx->prev_dockable_root->opt & UI_OPT_DOCKSPACE;

            if (is_dockspace) {
                if (hov_c) ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_TOP, 1.0f);
            } else {
                if (hov_c && !ctx->focus_root->split)
                    ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_TAB, 0.5f);
                else if (hov_l)
                    ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_LEFT, 0.5f);
                else if (hov_r)
                    ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_RIGHT, 0.5f);
                else if (hov_t)
                    ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_TOP, 0.5f);
                else if (hov_b)
                    ui_dock_ex_cnt(ctx, child, parent, UI_SPLIT_BOTTOM, 0.5f);
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
            case UI_CNT_MOVE: {
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

            case UI_CNT_FOCUS: {
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
                    fcnt->opt |= UI_OPT_NOHOVER;
                    fcnt->opt |= UI_OPT_NOINTERACT;
                    fcnt->flags &= ~UI_WINDOW_FLAGS_VISIBLE;
                    tab_bar->focus = tab_item->idx;
                    cnt->flags |= UI_WINDOW_FLAGS_VISIBLE;

                    // Bring all tab items to front
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        ui_bring_to_front(ctx, (ui_container_t*)tab_bar->items[i].data);
                    }
                    ui_bring_to_front(ctx, cnt);
                }

            } break;

            case UI_SPLIT_MOVE: {
                if (req->split) {
                    req->split->rect.x += ctx->mouse_delta.x;
                    req->split->rect.y += ctx->mouse_delta.y;
                    ui_update_split(ctx, req->split);
                }

            } break;

            case UI_SPLIT_RESIZE_SE: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case UI_SPLIT_RESIZE_W: {
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

            case UI_SPLIT_RESIZE_E: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->w = NEKO_MAX(r->w + ctx->mouse_delta.x, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case UI_SPLIT_RESIZE_N: {
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

            case UI_SPLIT_RESIZE_NE: {
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

            case UI_SPLIT_RESIZE_NW: {
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

            case UI_SPLIT_RESIZE_S: {
                if (req->split) {
                    ui_rect_t* r = &req->split->rect;
                    r->h = NEKO_MAX(r->h + ctx->mouse_delta.y, 40);
                    ui_update_split(ctx, req->split);
                }
            } break;

            case UI_SPLIT_RESIZE_SW: {
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

            case UI_SPLIT_RATIO: {
                const float smin = 0.05f;
                const float smax = 1.f - smin;
                ui_split_t* split = req->split;

                switch (split->type) {
                    case UI_SPLIT_LEFT: {
                        split->ratio = NEKO_CLAMP(split->ratio + ctx->mouse_delta.x / split->rect.w, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case UI_SPLIT_RIGHT: {
                        split->ratio = NEKO_CLAMP(split->ratio - ctx->mouse_delta.x / split->rect.w, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case UI_SPLIT_TOP: {
                        split->ratio = NEKO_CLAMP(split->ratio + ctx->mouse_delta.y / split->rect.h, smin, smax);
                        ui_update_split(ctx, split);
                    } break;

                    case UI_SPLIT_BOTTOM: {
                        split->ratio = NEKO_CLAMP(split->ratio - ctx->mouse_delta.y / split->rect.h, smin, smax);
                        ui_update_split(ctx, split);
                    } break;
                }

                // Bring to font
                ui_bring_split_to_front(ctx, ui_get_root_split_from_split(ctx, split));

            } break;

            case UI_TAB_SWAP_LEFT: {
                ui_tab_item_swap(ctx, req->cnt, -1);
            } break;

            case UI_TAB_SWAP_RIGHT: {
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
        } else if (~ctx->next_hover_root->opt & UI_OPT_NOFOCUS) {
            ui_bring_to_front(ctx, ctx->next_hover_root);
        }
    }

    if (ctx->mouse_pressed && (!ctx->next_hover_root || ctx->next_hover_root->opt & UI_OPT_NOFOCUS)) {
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
    // if (ctx->mouse_down != UI_MOUSE_LEFT) {
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

void ui_render(ui_context_t* ctx, command_buffer_t* cb) {
    const vec2 fb = ctx->framebuffer_size;
    const ui_rect_t* viewport = &ctx->viewport;
    idraw_t* gui_idraw = &ctx->gui_idraw;

    neko_idraw_defaults(&ctx->gui_idraw);
    // neko_idraw_camera2D(&ctx->gsi, (u32)fb.x, (u32)fb.y);
    neko_idraw_camera2d(&ctx->gui_idraw, (u32)viewport->w, (u32)viewport->h);
    neko_idraw_blend_enabled(&ctx->gui_idraw, true);

    ui_rect_t clip = ui_unclipped_rect;

    ui_command_t* cmd = NULL;
    while (ui_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case UI_COMMAND_CUSTOM: {
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

            case UI_COMMAND_PIPELINE: {
                neko_idraw_pipeline_set(&ctx->gui_idraw, cmd->pipeline.pipeline);

                // Set layout if valid
                if (cmd->pipeline.layout_sz) {
                    switch (cmd->pipeline.layout_type) {
                        case NEKO_IDRAW_LAYOUT_VATTR: {
                            neko_idraw_vattr_list(&ctx->gui_idraw, (neko_idraw_vattr_type*)cmd->pipeline.layout, cmd->pipeline.layout_sz);
                        } break;

                            // case NEKO_IDRAW_LAYOUT_MESH: {
                            //     neko_idraw_vattr_list_mesh(&ctx->gui_idraw, (neko_asset_mesh_layout_t*)cmd->pipeline.layout, cmd->pipeline.layout_sz);
                            // } break;
                    }
                }

                // If not a valid pipeline, then set back to default gui pipeline
                if (!cmd->pipeline.pipeline.id) {
                    neko_idraw_blend_enabled(&ctx->gui_idraw, true);
                }

            } break;

            case UI_COMMAND_UNIFORMS: {
                gfx_bind_desc_t bind = NEKO_DEFAULT_VAL();

                // Set uniform bind
                gfx_bind_uniform_desc_t uniforms[1] = NEKO_DEFAULT_VAL();
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms);

                // Treat as byte buffer, read data
                neko_byte_buffer_t buffer = NEKO_DEFAULT_VAL();
                buffer.capacity = UI_COMMANDLIST_SIZE;
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

            case UI_COMMAND_TEXT: {
                const vec2* tp = &cmd->text.pos;
                const char* ts = cmd->text.str;
                const Color256* tc = &cmd->text.color;
                FontFamily* tf = cmd->text.font;
                // 优化视野外渲染
                if (tp->x > g_app->width || tp->y > g_app->height || tp->x < 0 || tp->y < 0) {
                    break;
                }
                neko_idraw_text(&ctx->gui_idraw, tp->x, tp->y, ts, tf, false, *tc);
            } break;

            case UI_COMMAND_SHAPE: {
                neko_idraw_texture(&ctx->gui_idraw, neko_handle_invalid(gfx_texture_t));
                Color256* c = &cmd->shape.color;

                switch (cmd->shape.type) {
                    case UI_SHAPE_RECT: {
                        ui_rect_t* r = &cmd->shape.rect;
                        neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2s(0.f), neko_v2s(1.f), *c, R_PRIMITIVE_TRIANGLES);
                    } break;

                    case UI_SHAPE_CIRCLE: {
                        vec2* cp = &cmd->shape.circle.center;
                        float* r = &cmd->shape.circle.radius;
                        neko_idraw_circle(&ctx->gui_idraw, cp->x, cp->y, *r, 16, c->r, c->g, c->b, c->a, R_PRIMITIVE_TRIANGLES);
                    } break;

                    case UI_SHAPE_TRIANGLE: {
                        vec2* pa = &cmd->shape.triangle.points[0];
                        vec2* pb = &cmd->shape.triangle.points[1];
                        vec2* pc = &cmd->shape.triangle.points[2];
                        neko_idraw_trianglev(&ctx->gui_idraw, *pa, *pb, *pc, *c, R_PRIMITIVE_TRIANGLES);

                    } break;

                    case UI_SHAPE_LINE: {
                        vec2* s = &cmd->shape.line.start;
                        vec2* e = &cmd->shape.line.end;
                        neko_idraw_linev(&ctx->gui_idraw, *s, *e, *c);
                    } break;
                }

            } break;

            case UI_COMMAND_IMAGE: {
                Color256* c = &cmd->image.color;
                ui_rect_t* r = &cmd->image.rect;
                vec4* uvs = &cmd->image.uvs;
                if (r->x > g_app->width || r->y > g_app->height) {  // 优化视野外渲染
                    break;
                }
                neko_idraw_texture(&ctx->gui_idraw, cmd->image.hndl);
                neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2(uvs->x, uvs->y), neko_v2(uvs->z, uvs->w), *c, R_PRIMITIVE_TRIANGLES);
            } break;

            case UI_COMMAND_CLIP: {
                // Will project scissor/clipping rectangles into framebuffer space
                vec2 clip_off = neko_v2s(0.f);    // (0,0) unless using multi-viewports
                vec2 clip_scale = neko_v2s(1.f);  // (1,1) unless using retina display which are often (2,2)

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

                if (clip.x > g_app->width || clip.y > g_app->height) {  // 优化视野外渲染
                    break;
                }

                neko_idraw_set_view_scissor(&ctx->gui_idraw, (i32)(clip_rect.x), (i32)(fb.y - clip_rect.h - clip_rect.y), (i32)(clip_rect.w), (i32)(clip_rect.h));

            } break;
        }
    }

    // Draw main list
    neko_idraw_draw(&ctx->gui_idraw, cb);

    // Draw overlay list
    neko_idraw_draw(&ctx->overlay_draw_list, cb);
}

void ui_renderpass_submit(ui_context_t* ctx, command_buffer_t* cb, Color256 c) {
    vec2 fbs = ctx->framebuffer_size;
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

void ui_renderpass_submit_ex(ui_context_t* ctx, command_buffer_t* cb, gfx_clear_action_t action) {
    vec2 fbs = ctx->framebuffer_size;
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
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : UI_HASH_INITIAL;
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

void ui_push_id(ui_context_t* ctx, String str) { ui_push_id(ctx, str.data,str.len); }

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
        return UI_CLIP_ALL;
    }

    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y && r.y + r.h <= cr.y + cr.h) {
        return 0;
    }

    return UI_CLIP_PART;
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
    if (root->opt & UI_OPT_NOBRINGTOFRONT) {
        if (cnt->opt & UI_OPT_DOCKSPACE)
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
    NEKO_EXPECT(ctx->command_list.idx + size < UI_COMMANDLIST_SIZE);
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
        if ((*cmd)->type != UI_COMMAND_JUMP) {
            return 1;
        }
        *cmd = (ui_command_t*)((*cmd)->jump.dst);
    }
    return 0;
}

void ui_set_clip(ui_context_t* ctx, ui_rect_t rect) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, UI_COMMAND_CLIP, sizeof(ui_clipcommand_t));
    cmd->clip.rect = rect;
}

void ui_set_pipeline(ui_context_t* ctx, neko_handle(gfx_pipeline_t) pip, void* layout, size_t sz, neko_idraw_layout_type type) {
    ui_command_t* cmd;
    cmd = ui_push_command(ctx, UI_COMMAND_PIPELINE, sizeof(ui_pipelinecommand_t));
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
    cmd = ui_push_command(ctx, UI_COMMAND_UNIFORMS, sizeof(ui_binduniformscommand_t));
    cmd->uniforms.data = ctx->command_list.items + ctx->command_list.idx;

    // Treat as byte buffer, write into data then set size
    neko_byte_buffer_t buffer = NEKO_DEFAULT_VAL();
    buffer.capacity = UI_COMMANDLIST_SIZE;
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

void ui_draw_line(ui_context_t* ctx, vec2 start, vec2 end, Color256 color) {
    ui_command_t* cmd;
    ui_rect_t rect = NEKO_DEFAULT_VAL();
    vec2 s = start.x < end.x ? start : end;
    vec2 e = start.x < end.x ? end : start;
    ui_rect(s.x, s.y, e.x - s.x, e.y - s.y);
    rect = ui_intersect_rects(rect, ui_get_clip_rect(ctx));

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    cmd = ui_push_command(ctx, UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = UI_SHAPE_LINE;
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
        cmd = ui_push_command(ctx, UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
        cmd->shape.type = UI_SHAPE_RECT;
        cmd->shape.rect = rect;
        cmd->shape.color = color;
    }
}

void ui_draw_circle(ui_context_t* ctx, vec2 position, float radius, Color256 color) {
    ui_command_t* cmd;
    ui_rect_t rect = ui_rect(position.x - radius, position.y - radius, 2.f * radius, 2.f * radius);

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    // do shape command
    cmd = ui_push_command(ctx, UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = UI_SHAPE_CIRCLE;
    cmd->shape.circle.center = position;
    cmd->shape.circle.radius = radius;
    cmd->shape.color = color;

    // reset clipping if it was set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_triangle(ui_context_t* ctx, vec2 a, vec2 b, vec2 c, Color256 color) {
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
        clipped = UI_CLIP_ALL;
    else if (ca || cb || cc)
        clipped = UI_CLIP_PART;

    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, clip);
    }

    cmd = ui_push_command(ctx, UI_COMMAND_SHAPE, sizeof(ui_shapecommand_t));
    cmd->shape.type = UI_SHAPE_TRIANGLE;
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
    idraw_t* dl = &ctx->overlay_draw_list;
    // neko_idraw_rectvd(dl, neko_v2(rect.x, rect.y), neko_v2(rect.w, rect.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_RED, R_PRIMITIVE_LINES);

    const float l = (float)w[0], r = (float)w[1], t = (float)w[2], b = (float)w[3];
    ui_draw_rect(ctx, ui_rect(rect.x + l, rect.y, rect.w - r - l, t), color);               // top
    ui_draw_rect(ctx, ui_rect(rect.x + l, rect.y + rect.h - b, rect.w - r - l, b), color);  // bottom
    ui_draw_rect(ctx, ui_rect(rect.x, rect.y, l, rect.h), color);                           // left
    ui_draw_rect(ctx, ui_rect(rect.x + rect.w - r, rect.y, r, rect.h), color);              // right
}

void ui_draw_text(ui_context_t* ctx, FontFamily* font, const char* str, i32 len, vec2 pos, Color256 color, i32 shadow_x, i32 shadow_y, Color256 shadow_color) {
    // Set to default font
    if (!font) {
        font = neko_default_font();
    }

#define DRAW_TEXT(TEXT, RECT, COLOR)                                                 \
    do {                                                                             \
        ui_command_t* cmd;                                                           \
        vec2 td = ui_text_dimensions(font, TEXT, -1);                                \
        ui_rect_t rect = (RECT);                                                     \
        i32 clipped = ui_check_clip(ctx, rect);                                      \
                                                                                     \
        if (clipped == UI_CLIP_ALL) {                                                \
            return;                                                                  \
        }                                                                            \
                                                                                     \
        if (clipped == UI_CLIP_PART) {                                               \
            ui_rect_t crect = ui_get_clip_rect(ctx);                                 \
            ui_set_clip(ctx, crect);                                                 \
        }                                                                            \
                                                                                     \
        /* add command */                                                            \
        if (len < 0) {                                                               \
            len = strlen(TEXT);                                                      \
        }                                                                            \
                                                                                     \
        cmd = ui_push_command(ctx, UI_COMMAND_TEXT, sizeof(ui_textcommand_t) + len); \
        memcpy(cmd->text.str, TEXT, len);                                            \
        cmd->text.str[len] = '\0';                                                   \
        cmd->text.pos = neko_v2(rect.x, rect.y);                                     \
        cmd->text.color = COLOR;                                                     \
        cmd->text.font = font;                                                       \
                                                                                     \
        if (clipped) {                                                               \
            ui_set_clip(ctx, ui_unclipped_rect);                                     \
        }                                                                            \
    } while (0)

    // Draw shadow
    if (shadow_x || shadow_y && shadow_color.a) {
        DRAW_TEXT(str, ui_rect(pos.x + (float)shadow_x, pos.y + (float)shadow_y, td.x, td.y), shadow_color);
    }

    // Draw text
    { DRAW_TEXT(str, ui_rect(pos.x, pos.y, td.x, td.y), color); }
}

void ui_draw_image(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, vec2 uv0, vec2 uv1, Color256 color) {
    ui_command_t* cmd;

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    // do image command
    cmd = ui_push_command(ctx, UI_COMMAND_IMAGE, sizeof(ui_imagecommand_t));
    cmd->image.hndl = hndl;
    cmd->image.rect = rect;
    cmd->image.uvs = neko_v4(uv0.x, uv0.y, uv1.x, uv1.y);
    cmd->image.color = color;

    // reset clipping if it was set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_custom(ui_context_t* ctx, ui_rect_t rect, ui_draw_callback_t cb, void* data, size_t sz) {
    ui_command_t* cmd;

    ui_rect_t viewport = rect;

    rect = ui_intersect_rects(rect, ui_get_clip_rect(ctx));

    // do clip command if the rect isn't fully contained within the cliprect
    i32 clipped = ui_check_clip(ctx, rect);
    if (clipped == UI_CLIP_ALL) {
        return;
    }
    if (clipped == UI_CLIP_PART) {
        ui_set_clip(ctx, ui_get_clip_rect(ctx));
    }

    i32 idx = ctx->id_stack.idx;
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : UI_HASH_INITIAL;

    // do custom command
    cmd = ui_push_command(ctx, UI_COMMAND_CUSTOM, sizeof(ui_customcommand_t));
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

    // reset clipping if it was set
    if (clipped) {
        ui_set_clip(ctx, ui_unclipped_rect);
    }
}

void ui_draw_nine_rect(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, vec2 uv0, vec2 uv1, u32 left, u32 right, u32 top, u32 bottom, Color256 color) {
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
        vec2 st0 = neko_v2(uv0.x, uv0.y);
        vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // tr
    {
        u32 w = right;
        u32 h = top;
        ui_rect_t r = ui_rect(rect.x + rect.w - w, rect.y, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y);
        vec2 st1 = neko_v2(uv1.x, uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // br
    {
        u32 w = right;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x + rect.w - (f32)w, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        vec2 st1 = neko_v2(uv1.x, uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bl
    {
        u32 w = left;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv0.x, uv1.y - ((float)bottom / (float)height));
        vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // top
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = top;
        ui_rect_t r = ui_rect(rect.x + left, rect.y, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y);
        vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bottom
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = bottom;
        ui_rect_t r = ui_rect(rect.x + left, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y);
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // left
    {
        u32 w = left;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x, rect.y + top, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv0.x, uv0.y + ((float)top / (float)height));
        vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // right
    {
        u32 w = right;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x + rect.w - (f32)w, rect.y + top, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        vec2 st1 = neko_v2(uv1.x, uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // center
    {
        u32 w = (u32)rect.w - right - left;
        u32 h = (u32)rect.h - top - bottom;
        ui_rect_t r = ui_rect(rect.x + left, rect.y + top, (f32)w, (f32)h);
        vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        ui_draw_image(ctx, hndl, r, st0, st1, color);
    }
}

/*============================================================================
** layout
**============================================================================*/
enum { UI_RELATIVE = 1, UI_ABSOLUTE = 2 };

ui_rect_t ui_layout_anchor(const ui_rect_t* p, i32 width, i32 height, i32 xoff, i32 yoff, ui_layout_anchor_type type) {
    float w = (float)width;
    float h = (float)height;
    ui_rect_t r = ui_rect(p->x, p->y, w, h);

    switch (type) {
        default:
        case UI_LAYOUT_ANCHOR_TOPLEFT: {
        } break;

        case UI_LAYOUT_ANCHOR_TOPCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
        } break;

        case UI_LAYOUT_ANCHOR_TOPRIGHT: {
            r.x = p->x + (p->w - w);
        } break;

        case UI_LAYOUT_ANCHOR_LEFT: {
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case UI_LAYOUT_ANCHOR_CENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case UI_LAYOUT_ANCHOR_RIGHT: {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case UI_LAYOUT_ANCHOR_BOTTOMLEFT: {
            r.y = p->y + (p->h - h);
        } break;

        case UI_LAYOUT_ANCHOR_BOTTOMCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h);
        } break;

        case UI_LAYOUT_ANCHOR_BOTTOMRIGHT: {
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

    // inherit position/next_row/max from child layout if they are greater
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
        NEKO_EXPECT(items <= UI_MAX_WIDTHS);
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

        case UI_JUSTIFY_CENTER: {
            // Iterate through all widths, calculate total
            // X is center - tw/2
            float w = 0.f;
            for (u32 i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f ? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w) * 0.5f + layout->indent;
        } break;

        case UI_JUSTIFY_END: {
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
    layout->next_type = relative ? UI_RELATIVE : UI_ABSOLUTE;
}

ui_rect_t ui_layout_peek_next(ui_context_t* ctx) {
    ui_layout_t layout = *ui_get_layout(ctx);
    ui_style_t* style = ctx->style;
    ui_rect_t res;

    if (layout.next_type) {
        // handle rect set by `ui_layout_set_next`
        i32 type = layout.next_type;
        res = layout.next;
        if (type == UI_ABSOLUTE) {
            return res;
        }

    } else {
        // handle next row
        if (layout.item_index == layout.items) {
            ui_layout_row(ctx, layout.items, NULL, (i32)layout.size.y);
        }

        const i32 items = layout.items;
        const i32 idx = layout.item_index;

        i32 ml = style->margin[UI_MARGIN_LEFT];
        i32 mr = style->margin[UI_MARGIN_RIGHT];
        i32 mt = style->margin[UI_MARGIN_TOP];
        i32 mb = style->margin[UI_MARGIN_BOTTOM];

        // position
        res.x = layout.position.x + ml;
        res.y = layout.position.y + mt;

        // size
        res.w = layout.items > 0 ? layout.widths[layout.item_index] : layout.size.x;
        res.h = layout.size.y;

        // default fallbacks
        if (res.w == 0) {
            res.w = style->size.x;
        }
        if (res.h == 0) {
            res.h = style->size.y;
        }

        if (res.w < 0) {
            res.w += layout.body.w - res.x + 1;
        }
        if (res.h < 0) {
            res.h += layout.body.h - res.y + 1;
        }

        layout.item_index++;
    }

    // update position
    layout.position.x += res.w + style->margin[UI_MARGIN_RIGHT];
    layout.next_row = (i32)NEKO_MAX(layout.next_row, res.y + res.h + style->margin[UI_MARGIN_BOTTOM]);

    // apply body offset
    res.x += layout.body.x;
    res.y += layout.body.y;

    // update max position
    layout.max.x = NEKO_MAX(layout.max.x, res.x + res.w);
    layout.max.y = NEKO_MAX(layout.max.y, res.y + res.h);

    return res;
}

ui_rect_t ui_layout_next(ui_context_t* ctx) {
    ui_layout_t* layout = ui_get_layout(ctx);
    ui_style_t* style = ctx->style;
    ui_rect_t res;

    if (layout->next_type) {
        // handle rect set by `ui_layout_set_next`
        i32 type = layout->next_type;
        layout->next_type = 0;
        res = layout->next;
        if (type == UI_ABSOLUTE) {
            return (ctx->last_rect = res);
        }

    } else {
        // handle next row
        if (layout->item_index == layout->items) {
            ui_layout_row(ctx, layout->items, NULL, (i32)layout->size.y);
        }

        const i32 items = layout->items;
        const i32 idx = layout->item_index;

        i32 ml = style->margin[UI_MARGIN_LEFT];
        i32 mr = style->margin[UI_MARGIN_RIGHT];
        i32 mt = style->margin[UI_MARGIN_TOP];
        i32 mb = style->margin[UI_MARGIN_BOTTOM];

        // position
        res.x = layout->position.x + ml;
        res.y = layout->position.y + mt;

        // size
        res.w = layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
        res.h = layout->size.y;

        // default fallbacks
        if (res.w == 0) {
            res.w = style->size.x;
        }
        if (res.h == 0) {
            res.h = style->size.y;
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

    // update position
    layout->position.x += res.w + style->margin[UI_MARGIN_RIGHT];
    layout->next_row = (i32)NEKO_MAX(layout->next_row, res.y + res.h + style->margin[UI_MARGIN_BOTTOM]);  //  + style->margin[UI_MARGIN_TOP] * 0.5f);

    // apply body offset
    res.x += layout->body.x;
    res.y += layout->body.y;

    // update max position
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
    if (opt & UI_OPT_NOFRAME) {
        return;
    }
    i32 state = ctx->focus == id ? UI_ELEMENT_STATE_FOCUS : ctx->hover == id ? UI_ELEMENT_STATE_HOVER : 0x00;
    ui_draw_frame(ctx, rect, &ctx->style_sheet->styles[elementid][state]);
}

void ui_draw_control_text(ui_context_t* ctx, const char* str, ui_rect_t rect, const ui_style_t* style, u64 opt) {
    vec2 pos = neko_v2(rect.x, rect.y);
    FontFamily* font = style->font;
    vec2 td = ui_text_dimensions(font, str, -1);
    i32 tw = (i32)td.x;
    i32 th = (i32)td.y;

    ui_push_clip_rect(ctx, rect);

    // Grab stylings
    const i32 padding_left = style->padding[UI_PADDING_LEFT];
    const i32 padding_top = style->padding[UI_PADDING_TOP];
    const i32 padding_right = style->padding[UI_PADDING_RIGHT];
    const i32 padding_bottom = style->padding[UI_PADDING_BOTTOM];
    const i32 align = style->align_content;
    const i32 justify = style->justify_content;

    // Determine x placement based on justification
    switch (justify) {
        default:
        case UI_JUSTIFY_START: {
            pos.x = rect.x + padding_left;
        } break;

        case UI_JUSTIFY_CENTER: {
            pos.x = rect.x + (rect.w - tw) * 0.5f;
        } break;

        case UI_JUSTIFY_END: {
            pos.x = rect.x + (rect.w - tw) - padding_right;
        } break;
    }

    // Determine y placement based on alignment within rect
    switch (align) {
        default:
        case UI_ALIGN_START: {
            pos.y = rect.y + padding_top;
        } break;

        case UI_ALIGN_CENTER: {
            pos.y = rect.y + (rect.h - th) * 0.5f;
        } break;

        case UI_ALIGN_END: {
            pos.y = rect.y + (rect.h - th) - padding_bottom;
        } break;
    }

    bool is_content = (opt & UI_OPT_ISCONTENT);
    i32 bg_color = is_content ? UI_COLOR_CONTENT_BACKGROUND : UI_COLOR_BACKGROUND;
    i32 sh_color = is_content ? UI_COLOR_CONTENT_SHADOW : UI_COLOR_SHADOW;
    i32 bd_color = is_content ? UI_COLOR_CONTENT_BORDER : UI_COLOR_BORDER;

    i32 sx = style->shadow_x;
    i32 sy = style->shadow_y;
    const Color256* sc = &style->colors[sh_color];

    // Border
    const Color256* bc = &style->colors[bd_color];
    if (bc->a && ~opt & UI_OPT_NOSTYLEBORDER) {
        ui_pop_clip_rect(ctx);
        ui_rect_t border_rect = ui_expand_rect(rect, (i16*)style->border_width);
        ui_push_clip_rect(ctx, border_rect);
        ui_draw_box(ctx, border_rect, (i16*)style->border_width, *bc);
    }

    // Background
    if (~opt & UI_OPT_NOSTYLEBACKGROUND) {
        ui_draw_rect(ctx, rect, style->colors[bg_color]);
    }

    // Text
    ui_draw_text(ctx, font, str, -1, pos, style->colors[UI_COLOR_CONTENT], sx, sy, *sc);

    ui_pop_clip_rect(ctx);
}

i32 ui_mouse_over(ui_context_t* ctx, ui_rect_t rect) {
    return ui_rect_overlaps_vec2(rect, ctx->mouse_pos) && !ctx->hover_split && !ctx->lock_hover_id && ui_rect_overlaps_vec2(ui_get_clip_rect(ctx), ctx->mouse_pos) && ui_in_hover_root(ctx);
}

void ui_update_control(ui_context_t* ctx, ui_id id, ui_rect_t rect, u64 opt) {
    i32 mouseover = 0;
    idraw_t* dl = &ctx->overlay_draw_list;

    ui_id prev_hov = ctx->prev_hover;
    ui_id prev_foc = ctx->prev_focus;

    // I should do updates in here

    if (opt & UI_OPT_FORCEFOCUS) {
        mouseover = ui_rect_overlaps_vec2(ui_get_clip_rect(ctx), ctx->mouse_pos);
    } else {
        mouseover = ui_mouse_over(ctx, rect);
    }

    // Check for 'mouse-over' with id selection here

    if (ctx->focus == id) {
        ctx->updated_focus = 1;
    }
    if (opt & UI_OPT_NOINTERACT) {
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
        if (!ctx->mouse_down && ~opt & UI_OPT_HOLDFOCUS) {
            ui_set_focus(ctx, 0);
        }
    }

    if (ctx->prev_hover == id && !mouseover) {
        ctx->prev_hover = ctx->hover;
    }

    if (ctx->hover == id) {
        if (ctx->mouse_pressed) {
            if ((opt & UI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == UI_MOUSE_LEFT) || (~opt & UI_OPT_LEFTCLICKONLY)) {
                ui_set_focus(ctx, id);
            }
        } else if (!mouseover) {
            ui_set_hover(ctx, 0);
        }
    }

    // Do state check
    if (~opt & UI_OPT_NOSWITCHSTATE) {
        if (ctx->focus == id) {
            if (ctx->prev_focus != id)
                ctx->last_focus_state = UI_ELEMENT_STATE_ON_FOCUS;
            else
                ctx->last_focus_state = UI_ELEMENT_STATE_FOCUS;
        } else {
            if (ctx->prev_focus == id)
                ctx->last_focus_state = UI_ELEMENT_STATE_OFF_FOCUS;
            else
                ctx->last_focus_state = UI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->hover == id) {
            if (ctx->prev_hover != id)
                ctx->last_hover_state = UI_ELEMENT_STATE_ON_HOVER;
            else
                ctx->last_hover_state = UI_ELEMENT_STATE_HOVER;
        } else {
            if (ctx->prev_hover == id)
                ctx->last_hover_state = UI_ELEMENT_STATE_OFF_HOVER;
            else
                ctx->last_hover_state = UI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->prev_focus == id && !ctx->mouse_down && ~opt & UI_OPT_HOLDFOCUS) {
            ctx->prev_focus = ctx->focus;
        }

        if (ctx->last_hover_state == UI_ELEMENT_STATE_ON_HOVER || ctx->last_hover_state == UI_ELEMENT_STATE_OFF_HOVER || ctx->last_focus_state == UI_ELEMENT_STATE_ON_FOCUS ||
            ctx->last_focus_state == UI_ELEMENT_STATE_OFF_FOCUS) {
            // Don't have a hover state switch if focused
            ctx->switch_state = ctx->last_focus_state ? ctx->last_focus_state : ctx->focus != id ? ctx->last_hover_state : UI_ELEMENT_STATE_DEFAULT;
            switch (ctx->switch_state) {
                case UI_ELEMENT_STATE_OFF_HOVER:
                case UI_ELEMENT_STATE_ON_HOVER: {
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

#endif