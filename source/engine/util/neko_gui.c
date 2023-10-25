
#include "engine/util/neko_gui.h"

#include "engine/neko_platform.h"

#ifndef NEKO_PHYSICS_IMPL
#define NEKO_PHYSICS_IMPL
#include "neko_physics.h"

// 2D AABB collision detection (rect. vs. rect.)
neko_force_inline b32 neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b) {
    if (a->max.x > b->min.x && a->max.y > b->min.y && a->min.x < b->max.x && a->min.y < b->max.y) {
        return true;
    }

    return false;
}

#endif

#define neko_gui_unused(x) ((void)(x))

#define neko_gui_stack_push(stk, val)                                               \
    do {                                                                            \
        neko_expect((stk).idx < (s32)(sizeof((stk).items) / sizeof(*(stk).items))); \
        (stk).items[(stk).idx] = (val);                                             \
        (stk).idx++; /* incremented after incase `val` uses this value */           \
    } while (0)

#define neko_gui_stack_pop(stk)     \
    do {                            \
        neko_expect((stk).idx > 0); \
        (stk).idx--;                \
    } while (0)

/* 32bit fnv-1a hash */
#define NEKO_GUI_HASH_INITIAL 2166136261

static void neko_gui_hash(neko_gui_id* hash, const void* data, s32 size) {
    const unsigned char* p = (const unsigned char*)data;
    while (size--) {
        *hash = (*hash ^ *p++) * 16777619;
    }
}

static neko_gui_rect_t neko_gui_unclipped_rect = {0, 0, 0x1000000, 0x1000000};

// Default styles
static neko_gui_style_t neko_gui_default_container_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_button_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_text_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_label_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_panel_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_input_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_scroll_style[3] = neko_default_val();
static neko_gui_style_t neko_gui_default_image_style[3] = neko_default_val();

static neko_gui_style_t neko_gui_default_style = {
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
                {25, 25, 25, 255},     // NEKO_GUI_COLOR_BACKGROUND
                {255, 255, 255, 255},  // NEKO_GUI_COLOR_CONTENT
                {29, 29, 29, 76},      // NEKO_GUI_COLOR_BORDER
                {0, 0, 0, 31},         // NEKO_GUI_COLOR_SHADOW
                {0, 0, 0, 0},          // NEKO_GUI_COLOR_CONTENT_BACKGROUND
                {0, 0, 0, 0},          // NEKO_GUI_COLOR_CONTENT_SHADOW
                {0, 0, 0, 0}           // NEKO_GUI_COLOR_CONTENT_BORDER
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
        NEKO_GUI_DIRECTION_COLUMN,
        NEKO_GUI_JUSTIFY_START,
        NEKO_GUI_ALIGN_CENTER,

        // shadow x, y
        1,
        1};

static neko_gui_style_sheet_t neko_gui_default_style_sheet = neko_default_val();

static neko_gui_style_t neko_gui_get_current_element_style(neko_gui_context_t* ctx, const neko_gui_selector_desc_t* desc, s32 elementid, s32 state) {

#define NEKO_GUI_APPLY_STYLE(SE)                                               \
    do {                                                                       \
        switch ((SE)->type) {                                                  \
            case NEKO_GUI_STYLE_WIDTH:                                         \
                style.size[0] = (float)(SE)->value;                            \
                break;                                                         \
            case NEKO_GUI_STYLE_HEIGHT:                                        \
                style.size[1] = (float)(SE)->value;                            \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_PADDING: {                                     \
                style.padding[NEKO_GUI_PADDING_LEFT] = (s32)(SE)->value;       \
                style.padding[NEKO_GUI_PADDING_TOP] = (s32)(SE)->value;        \
                style.padding[NEKO_GUI_PADDING_RIGHT] = (s32)(SE)->value;      \
                style.padding[NEKO_GUI_PADDING_BOTTOM] = (s32)(SE)->value;     \
            }                                                                  \
                                                                               \
            case NEKO_GUI_STYLE_PADDING_LEFT:                                  \
                style.padding[NEKO_GUI_PADDING_LEFT] = (s32)(SE)->value;       \
                break;                                                         \
            case NEKO_GUI_STYLE_PADDING_TOP:                                   \
                style.padding[NEKO_GUI_PADDING_TOP] = (s32)(SE)->value;        \
                break;                                                         \
            case NEKO_GUI_STYLE_PADDING_RIGHT:                                 \
                style.padding[NEKO_GUI_PADDING_RIGHT] = (s32)(SE)->value;      \
                break;                                                         \
            case NEKO_GUI_STYLE_PADDING_BOTTOM:                                \
                style.padding[NEKO_GUI_PADDING_BOTTOM] = (s32)(SE)->value;     \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_MARGIN: {                                      \
                style.margin[NEKO_GUI_MARGIN_LEFT] = (s32)(SE)->value;         \
                style.margin[NEKO_GUI_MARGIN_TOP] = (s32)(SE)->value;          \
                style.margin[NEKO_GUI_MARGIN_RIGHT] = (s32)(SE)->value;        \
                style.margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)(SE)->value;       \
            } break;                                                           \
                                                                               \
            case NEKO_GUI_STYLE_MARGIN_LEFT:                                   \
                style.margin[NEKO_GUI_MARGIN_LEFT] = (s32)(SE)->value;         \
                break;                                                         \
            case NEKO_GUI_STYLE_MARGIN_TOP:                                    \
                style.margin[NEKO_GUI_MARGIN_TOP] = (s32)(SE)->value;          \
                break;                                                         \
            case NEKO_GUI_STYLE_MARGIN_RIGHT:                                  \
                style.margin[NEKO_GUI_MARGIN_RIGHT] = (s32)(SE)->value;        \
                break;                                                         \
            case NEKO_GUI_STYLE_MARGIN_BOTTOM:                                 \
                style.margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)(SE)->value;       \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_BORDER_RADIUS: {                               \
                style.border_radius[0] = (SE)->value;                          \
                style.border_radius[1] = (SE)->value;                          \
                style.border_radius[2] = (SE)->value;                          \
                style.border_radius[3] = (SE)->value;                          \
            } break;                                                           \
                                                                               \
            case NEKO_GUI_STYLE_BORDER_RADIUS_LEFT:                            \
                style.border_radius[0] = (SE)->value;                          \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_RADIUS_RIGHT:                           \
                style.border_radius[1] = (SE)->value;                          \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_RADIUS_TOP:                             \
                style.border_radius[2] = (SE)->value;                          \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_RADIUS_BOTTOM:                          \
                style.border_radius[3] = (SE)->value;                          \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_BORDER_WIDTH: {                                \
                style.border_width[0] = (SE)->value;                           \
                style.border_width[1] = (SE)->value;                           \
                style.border_width[2] = (SE)->value;                           \
                style.border_width[3] = (SE)->value;                           \
            } break;                                                           \
                                                                               \
            case NEKO_GUI_STYLE_BORDER_WIDTH_LEFT:                             \
                style.border_width[0] = (SE)->value;                           \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_WIDTH_RIGHT:                            \
                style.border_width[1] = (SE)->value;                           \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_WIDTH_TOP:                              \
                style.border_width[2] = (SE)->value;                           \
                break;                                                         \
            case NEKO_GUI_STYLE_BORDER_WIDTH_BOTTOM:                           \
                style.border_width[3] = (SE)->value;                           \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_DIRECTION:                                     \
                style.direction = (s32)(SE)->value;                            \
                break;                                                         \
            case NEKO_GUI_STYLE_ALIGN_CONTENT:                                 \
                style.align_content = (s32)(SE)->value;                        \
                break;                                                         \
            case NEKO_GUI_STYLE_JUSTIFY_CONTENT:                               \
                style.justify_content = (s32)(SE)->value;                      \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_SHADOW_X:                                      \
                style.shadow_x = (s32)(SE)->value;                             \
                break;                                                         \
            case NEKO_GUI_STYLE_SHADOW_Y:                                      \
                style.shadow_y = (s32)(SE)->value;                             \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_COLOR_BACKGROUND:                              \
                style.colors[NEKO_GUI_COLOR_BACKGROUND] = (SE)->color;         \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_BORDER:                                  \
                style.colors[NEKO_GUI_COLOR_BORDER] = (SE)->color;             \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_SHADOW:                                  \
                style.colors[NEKO_GUI_COLOR_SHADOW] = (SE)->color;             \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_CONTENT:                                 \
                style.colors[NEKO_GUI_COLOR_CONTENT] = (SE)->color;            \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND:                      \
                style.colors[NEKO_GUI_COLOR_CONTENT_BACKGROUND] = (SE)->color; \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_CONTENT_BORDER:                          \
                style.colors[NEKO_GUI_COLOR_CONTENT_BORDER] = (SE)->color;     \
                break;                                                         \
            case NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW:                          \
                style.colors[NEKO_GUI_COLOR_CONTENT_SHADOW] = (SE)->color;     \
                break;                                                         \
                                                                               \
            case NEKO_GUI_STYLE_FONT:                                          \
                style.font = (SE)->font;                                       \
                break;                                                         \
        }                                                                      \
    } while (0)

    neko_gui_style_t style = ctx->style_sheet->styles[elementid][state];

    // Look for id tag style
    neko_gui_style_list_t* id_styles = NULL;
    neko_gui_style_list_t* cls_styles[NEKO_GUI_CLS_SELECTOR_MAX] = neko_default_val();

    if (desc) {
        char TMP[256] = neko_default_val();

        // ID selector
        neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const u64 id_hash = neko_hash_str64(TMP);
        id_styles = neko_hash_table_exists(ctx->style_sheet->cid_styles, id_hash) ? neko_hash_table_getp(ctx->style_sheet->cid_styles, id_hash) : NULL;

        // Class selectors
        for (u32 i = 0; i < NEKO_GUI_CLS_SELECTOR_MAX; ++i) {
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
        for (u32 i = 0; i < NEKO_GUI_CLS_SELECTOR_MAX; ++i) {
            if (!cls_styles[i]) break;
            for (u32 s = 0; s < neko_dyn_array_size(cls_styles[i]->styles[state]); ++s) {
                neko_gui_style_element_t* se = &cls_styles[i]->styles[state][s];
                NEKO_GUI_APPLY_STYLE(se);
            }
        }
    }

    // Override with id styles
    if (id_styles) {
        for (u32 i = 0; i < neko_dyn_array_size(id_styles->styles[state]); ++i) {
            neko_gui_style_element_t* se = &id_styles->styles[state][i];
            NEKO_GUI_APPLY_STYLE(se);
        }
    }

    if (neko_hash_table_exists(ctx->inline_styles, (neko_gui_element_type)elementid)) {
        neko_gui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, (neko_gui_element_type)elementid);
        if (neko_dyn_array_size(iss->styles[state])) {
            // Get last size to apply for styles for this state
            const u32 scz = neko_dyn_array_size(iss->style_counts);
            const u32 ct = state == 0x00 ? iss->style_counts[scz - 3] : state == 0x01 ? iss->style_counts[scz - 2] : iss->style_counts[scz - 1];
            const u32 ssz = neko_dyn_array_size(iss->styles[state]);

            for (u32 i = 0; i < ct; ++i) {
                u32 idx = (ssz - ct + i);
                neko_gui_style_element_t* se = &iss->styles[state][idx];
                NEKO_GUI_APPLY_STYLE(se);
            }
        }
    }

    return style;
}

NEKO_API_DECL neko_gui_style_t neko_gui_animation_get_blend_style(neko_gui_context_t* ctx, neko_gui_animation_t* anim, const neko_gui_selector_desc_t* desc, s32 elementid) {
    neko_gui_style_t ret = neko_default_val();

    s32 focus_state = anim->focus_state;
    s32 hover_state = anim->hover_state;

    neko_gui_style_t s0 = neko_gui_get_current_element_style(ctx, desc, elementid, anim->start_state);
    neko_gui_style_t s1 = neko_gui_get_current_element_style(ctx, desc, elementid, anim->end_state);

    neko_gui_inline_style_stack_t* iss = NULL;
    if (neko_hash_table_exists(ctx->inline_styles, (neko_gui_element_type)elementid)) {
        iss = neko_hash_table_getp(ctx->inline_styles, (neko_gui_element_type)elementid);
    }

    if (anim->direction == NEKO_GUI_ANIMATION_DIRECTION_FORWARD) {
        ret = s1;
    } else {
        ret = s0;
    }

    const neko_gui_animation_property_list_t* list = NULL;
    if (neko_hash_table_exists(ctx->style_sheet->animations, (neko_gui_element_type)elementid)) {
        list = neko_hash_table_getp(ctx->style_sheet->animations, (neko_gui_element_type)elementid);
    }

    const neko_gui_animation_property_list_t* id_list = NULL;
    const neko_gui_animation_property_list_t* cls_list[NEKO_GUI_CLS_SELECTOR_MAX] = neko_default_val();
    bool has_class_animations = false;

    if (desc) {
        char TMP[256] = neko_default_val();

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
            for (u32 i = 0; i < NEKO_GUI_CLS_SELECTOR_MAX; ++i) {
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

#define NEKO_GUI_BLEND_COLOR(TYPE)                                                               \
    do {                                                                                         \
        neko_color_t* c0 = &s0.colors[TYPE];                                                     \
        neko_color_t* c1 = &s1.colors[TYPE];                                                     \
        float r = 255.f * neko_interp_smoothstep((float)c0->r / 255.f, (float)c1->r / 255.f, t); \
        float g = 255.f * neko_interp_smoothstep((float)c0->g / 255.f, (float)c1->g / 255.f, t); \
        float b = 255.f * neko_interp_smoothstep((float)c0->b / 255.f, (float)c1->b / 255.f, t); \
        float a = 255.f * neko_interp_smoothstep((float)c0->a / 255.f, (float)c1->a / 255.f, t); \
        ret.colors[TYPE] = neko_color((u8)r, (u8)g, (u8)b, (u8)a);                               \
    } while (0)

#define NEKO_GUI_BLEND_VALUE(FIELD, TYPE)                    \
    do {                                                     \
        float v0 = (float)s0.FIELD;                          \
        float v1 = (float)s1.FIELD;                          \
        ret.FIELD = (TYPE)neko_interp_smoothstep(v0, v1, t); \
    } while (0)

#define NEKO_GUI_BLEND_PROPERTIES(LIST)                                                                                                                \
    do {                                                                                                                                               \
        for (u32 i = 0; i < neko_dyn_array_size(LIST); ++i) {                                                                                          \
            const neko_gui_animation_property_t* prop = &LIST[i];                                                                                      \
            float t = 0.f;                                                                                                                             \
            switch (anim->direction) {                                                                                                                 \
                default:                                                                                                                               \
                case NEKO_GUI_ANIMATION_DIRECTION_FORWARD: {                                                                                           \
                    t = neko_clamp(neko_map_range((float)prop->delay, (float)prop->time + (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f); \
                } break;                                                                                                                               \
                case NEKO_GUI_ANIMATION_DIRECTION_BACKWARD: {                                                                                          \
                    if (prop->time <= 0.f)                                                                                                             \
                        t = 1.f;                                                                                                                       \
                    else                                                                                                                               \
                        t = neko_clamp(neko_map_range((float)0.f, (float)anim->max - (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f);      \
                } break;                                                                                                                               \
            }                                                                                                                                          \
                                                                                                                                                       \
            switch (prop->type) {                                                                                                                      \
                case NEKO_GUI_STYLE_COLOR_BACKGROUND: {                                                                                                \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_BACKGROUND);                                                                                   \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_SHADOW: {                                                                                                    \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_SHADOW);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_BORDER: {                                                                                                    \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_BORDER);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_CONTENT: {                                                                                                   \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_CONTENT);                                                                                      \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND: {                                                                                        \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_CONTENT_BACKGROUND);                                                                           \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW: {                                                                                            \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_CONTENT_SHADOW);                                                                               \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_COLOR_CONTENT_BORDER: {                                                                                            \
                    NEKO_GUI_BLEND_COLOR(NEKO_GUI_COLOR_CONTENT_BORDER);                                                                               \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_WIDTH: {                                                                                                           \
                    NEKO_GUI_BLEND_VALUE(size[0], float);                                                                                              \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_HEIGHT: {                                                                                                          \
                    NEKO_GUI_BLEND_VALUE(size[1], float);                                                                                              \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_WIDTH: {                                                                                                    \
                    NEKO_GUI_BLEND_VALUE(border_width[0], s16);                                                                                        \
                    NEKO_GUI_BLEND_VALUE(border_width[1], s16);                                                                                        \
                    NEKO_GUI_BLEND_VALUE(border_width[2], s16);                                                                                        \
                    NEKO_GUI_BLEND_VALUE(border_width[3], s16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_WIDTH_LEFT: {                                                                                               \
                    NEKO_GUI_BLEND_VALUE(border_width[0], s16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_WIDTH_RIGHT: {                                                                                              \
                    NEKO_GUI_BLEND_VALUE(border_width[1], s16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_WIDTH_TOP: {                                                                                                \
                    NEKO_GUI_BLEND_VALUE(border_width[2], s16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_WIDTH_BOTTOM: {                                                                                             \
                    NEKO_GUI_BLEND_VALUE(border_width[3], s16);                                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_RADIUS: {                                                                                                   \
                    NEKO_GUI_BLEND_VALUE(border_radius[0], s16);                                                                                       \
                    NEKO_GUI_BLEND_VALUE(border_radius[1], s16);                                                                                       \
                    NEKO_GUI_BLEND_VALUE(border_radius[2], s16);                                                                                       \
                    NEKO_GUI_BLEND_VALUE(border_radius[3], s16);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_RADIUS_LEFT: {                                                                                              \
                    NEKO_GUI_BLEND_VALUE(border_radius[0], s16);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_RADIUS_RIGHT: {                                                                                             \
                    NEKO_GUI_BLEND_VALUE(border_radius[1], s16);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_RADIUS_TOP: {                                                                                               \
                    NEKO_GUI_BLEND_VALUE(border_radius[2], s16);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_BORDER_RADIUS_BOTTOM: {                                                                                            \
                    NEKO_GUI_BLEND_VALUE(border_radius[3], s16);                                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_MARGIN_BOTTOM: {                                                                                                   \
                    NEKO_GUI_BLEND_VALUE(margin[NEKO_GUI_MARGIN_BOTTOM], s16);                                                                         \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_MARGIN_TOP: {                                                                                                      \
                    NEKO_GUI_BLEND_VALUE(margin[NEKO_GUI_MARGIN_TOP], s16);                                                                            \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_MARGIN_LEFT: {                                                                                                     \
                    NEKO_GUI_BLEND_VALUE(margin[NEKO_GUI_MARGIN_LEFT], s16);                                                                           \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_MARGIN_RIGHT: {                                                                                                    \
                    NEKO_GUI_BLEND_VALUE(margin[NEKO_GUI_MARGIN_RIGHT], s16);                                                                          \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_MARGIN: {                                                                                                          \
                    NEKO_GUI_BLEND_VALUE(margin[0], s16);                                                                                              \
                    NEKO_GUI_BLEND_VALUE(margin[1], s16);                                                                                              \
                    NEKO_GUI_BLEND_VALUE(margin[2], s16);                                                                                              \
                    NEKO_GUI_BLEND_VALUE(margin[3], s16);                                                                                              \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_PADDING_BOTTOM: {                                                                                                  \
                    NEKO_GUI_BLEND_VALUE(padding[NEKO_GUI_PADDING_BOTTOM], s32);                                                                       \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_PADDING_TOP: {                                                                                                     \
                    NEKO_GUI_BLEND_VALUE(padding[NEKO_GUI_PADDING_TOP], s32);                                                                          \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_PADDING_LEFT: {                                                                                                    \
                    NEKO_GUI_BLEND_VALUE(padding[NEKO_GUI_PADDING_LEFT], s32);                                                                         \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_PADDING_RIGHT: {                                                                                                   \
                    NEKO_GUI_BLEND_VALUE(padding[NEKO_GUI_PADDING_RIGHT], s32);                                                                        \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_PADDING: {                                                                                                         \
                    NEKO_GUI_BLEND_VALUE(padding[0], s32);                                                                                             \
                    NEKO_GUI_BLEND_VALUE(padding[1], s32);                                                                                             \
                    NEKO_GUI_BLEND_VALUE(padding[2], s32);                                                                                             \
                    NEKO_GUI_BLEND_VALUE(padding[3], s32);                                                                                             \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_SHADOW_X: {                                                                                                        \
                    NEKO_GUI_BLEND_VALUE(shadow_x, s16);                                                                                               \
                } break;                                                                                                                               \
                case NEKO_GUI_STYLE_SHADOW_Y: {                                                                                                        \
                    NEKO_GUI_BLEND_VALUE(shadow_y, s16);                                                                                               \
                } break;                                                                                                                               \
            }                                                                                                                                          \
        }                                                                                                                                              \
    } while (0)

    // Get final blends
    if (list && !neko_dyn_array_empty(list->properties[anim->end_state])) {
        NEKO_GUI_BLEND_PROPERTIES(list->properties[anim->end_state]);
    }

    // Class list
    if (has_class_animations) {
        for (u32 c = 0; c < NEKO_GUI_CLS_SELECTOR_MAX; ++c) {
            if (!cls_list[c]) continue;
            if (!neko_dyn_array_empty(cls_list[c]->properties[anim->end_state])) {
                NEKO_GUI_BLEND_PROPERTIES(cls_list[c]->properties[anim->end_state]);
            }
        }
    }

    // Id list
    if (id_list && !neko_dyn_array_empty(id_list->properties[anim->end_state])) {
        NEKO_GUI_BLEND_PROPERTIES(id_list->properties[anim->end_state]);
    }

    if (iss) {
        NEKO_GUI_BLEND_PROPERTIES(iss->animations[anim->end_state]);
    }

    return ret;
}

static void _neko_gui_animation_get_time(neko_gui_context_t* ctx, neko_gui_id id, s32 elementid, const neko_gui_selector_desc_t* desc, neko_gui_inline_style_stack_t* iss, s32 state,
                                         neko_gui_animation_t* anim) {
    u32 act = 0, ssz = 0;
    if (iss && neko_dyn_array_size(iss->animations[state])) {
        const u32 scz = neko_dyn_array_size(iss->animation_counts);
        act = state == 0x00 ? iss->animation_counts[scz - 3] : state == 0x01 ? iss->animation_counts[scz - 2] : iss->animation_counts[scz - 1];
        ssz = neko_dyn_array_size(iss->animations[state]);
    }
    neko_gui_animation_property_list_t* cls_list[NEKO_GUI_CLS_SELECTOR_MAX] = neko_default_val();
    const neko_gui_animation_property_list_t* id_list = NULL;
    const neko_gui_animation_property_list_t* list = NULL;
    bool has_class_animations = false;

    if (desc) {
        char TMP[256] = neko_default_val();

        // Id animations
        neko_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const u64 id_hash = neko_hash_str64(TMP);
        if (neko_hash_table_exists(ctx->style_sheet->cid_animations, id_hash)) {
            id_list = neko_hash_table_getp(ctx->style_sheet->cid_animations, id_hash);
        }

        // Class animations
        for (u32 i = 0; i < NEKO_GUI_CLS_SELECTOR_MAX; ++i) {
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
    if (neko_hash_table_exists(ctx->style_sheet->animations, (neko_gui_element_type)elementid)) {
        list = neko_hash_table_getp(ctx->style_sheet->animations, (neko_gui_element_type)elementid);
    }

    // Fill properties in order of specificity
    neko_gui_animation_property_t properties[NEKO_GUI_STYLE_COUNT] = neko_default_val();
    for (u32 i = 0; i < NEKO_GUI_STYLE_COUNT; ++i) {
        properties[i].type = (neko_gui_style_element_type)i;
    }

#define GUI_SET_PROPERTY_TIMES(PROP_LIST)                            \
    do {                                                             \
        for (u32 p = 0; p < neko_dyn_array_size((PROP_LIST)); ++p) { \
            neko_gui_animation_property_t* prop = &(PROP_LIST)[p];   \
            properties[prop->type].time = prop->time;                \
            properties[prop->type].delay = prop->delay;              \
        }                                                            \
    } while (0)

    // Element type list
    if (list) {
        neko_dyn_array(neko_gui_animation_property_t) props = list->properties[state];
        GUI_SET_PROPERTY_TIMES(props);
    }

    // Class list
    if (has_class_animations) {
        for (u32 c = 0; c < NEKO_GUI_CLS_SELECTOR_MAX; ++c) {
            if (!cls_list[c]) continue;
            neko_dyn_array(neko_gui_animation_property_t) props = cls_list[c]->properties[state];
            GUI_SET_PROPERTY_TIMES(props);
        }
    }

    // Id list
    if (id_list) {
        neko_dyn_array(neko_gui_animation_property_t) props = id_list->properties[state];
        GUI_SET_PROPERTY_TIMES(props);
    }

    // Inline style list
    if (act && iss) {
        for (u32 a = 0; a < act; ++a) {
            u32 idx = ssz - act + a;
            neko_gui_animation_property_t* ap = &iss->animations[state][idx];
            properties[ap->type].time = ap->time;
            properties[ap->type].delay = ap->delay;
        }
    }

    // Set max times
    for (u32 i = 0; i < NEKO_GUI_STYLE_COUNT; ++i) {
        if (properties[i].time > anim->max) anim->max = properties[i].time;
        if (properties[i].delay > anim->delay) anim->delay = properties[i].delay;
    }

    // Finalize time
    anim->max += anim->delay;
    anim->max = neko_max(anim->max, 5);
}

NEKO_API_DECL neko_gui_animation_t* neko_gui_get_animation(neko_gui_context_t* ctx, neko_gui_id id, const neko_gui_selector_desc_t* desc, s32 elementid) {
    neko_gui_animation_t* anim = NULL;

    const b32 valid_eid = (elementid >= 0 && elementid < NEKO_GUI_ELEMENT_COUNT);

    // Construct new animation if necessary to insert
    if (ctx->state_switch_id == id) {
        if (!neko_hash_table_exists(ctx->animations, id)) {
            neko_gui_animation_t val = neko_default_val();
            neko_hash_table_insert(ctx->animations, id, val);
        }

        neko_gui_inline_style_stack_t* iss = NULL;
        if (neko_hash_table_exists(ctx->inline_styles, (neko_gui_element_type)elementid)) {
            iss = neko_hash_table_getp(ctx->inline_styles, (neko_gui_element_type)elementid);
        }

#define ANIM_GET_TIME(STATE)

        anim = neko_hash_table_getp(ctx->animations, id);
        anim->playing = true;

        s16 focus_state = 0x00;
        s16 hover_state = 0x00;
        s16 direction = 0x00;
        s16 start_state = 0x00;
        s16 end_state = 0x00;
        s16 time_state = 0x00;

        switch (ctx->switch_state) {
            case NEKO_GUI_ELEMENT_STATE_OFF_FOCUS: {
                if (ctx->hover == id) {
                    anim->direction = NEKO_GUI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                    anim->end_state = NEKO_GUI_ELEMENT_STATE_FOCUS;
                    time_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                    if (valid_eid) _neko_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                } else {
                    anim->direction = NEKO_GUI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
                    anim->end_state = NEKO_GUI_ELEMENT_STATE_FOCUS;
                    time_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
                    if (valid_eid) _neko_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                }
            } break;

            case NEKO_GUI_ELEMENT_STATE_ON_FOCUS: {
                anim->direction = NEKO_GUI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                anim->end_state = NEKO_GUI_ELEMENT_STATE_FOCUS;
                time_state = NEKO_GUI_ELEMENT_STATE_FOCUS;
                if (valid_eid) _neko_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;

            case NEKO_GUI_ELEMENT_STATE_OFF_HOVER: {
                anim->direction = NEKO_GUI_ANIMATION_DIRECTION_BACKWARD;
                anim->start_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
                anim->end_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                time_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
                if (valid_eid) _neko_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = anim->max;
            } break;

            case NEKO_GUI_ELEMENT_STATE_ON_HOVER: {
                anim->direction = NEKO_GUI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
                anim->end_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                time_state = NEKO_GUI_ELEMENT_STATE_HOVER;
                if (valid_eid) _neko_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
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

NEKO_API_DECL void neko_gui_animation_update(neko_gui_context_t* ctx, neko_gui_animation_t* anim) {
    if (ctx->frame == anim->frame) return;

    const s16 dt = (s16)(neko_platform_delta_time() * 1000.f);

    if (anim->playing) {
        // Forward
        switch (anim->direction) {
            default:
            case (NEKO_GUI_ANIMATION_DIRECTION_FORWARD): {
                anim->time += dt;
                if (anim->time >= anim->max) {
                    anim->time = anim->max;
                    anim->playing = false;
                }
            } break;

            case (NEKO_GUI_ANIMATION_DIRECTION_BACKWARD): {
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

NEKO_API_DECL neko_gui_rect_t neko_gui_rect(float x, float y, float w, float h) {
    neko_gui_rect_t res;
    res.x = x;
    res.y = y;
    res.w = w;
    res.h = h;
    return res;
}

static neko_gui_rect_t neko_gui_expand_rect(neko_gui_rect_t rect, s16 v[4]) { return neko_gui_rect(rect.x - v[0], rect.y - v[2], rect.w + v[0] + v[1], rect.h + v[2] + v[3]); }

static neko_gui_rect_t neko_gui_intersect_rects(neko_gui_rect_t r1, neko_gui_rect_t r2) {
    s32 x1 = (s32)neko_max(r1.x, r2.x);
    s32 y1 = (s32)neko_max(r1.y, r2.y);
    s32 x2 = (s32)neko_min(r1.x + r1.w, r2.x + r2.w);
    s32 y2 = (s32)neko_min(r1.y + r1.h, r2.y + r2.h);
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    return neko_gui_rect((float)x1, (float)y1, (float)x2 - (float)x1, (float)y2 - (float)y1);
}

static s32 neko_gui_rect_overlaps_vec2(neko_gui_rect_t r, neko_vec2 p) { return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h; }

NEKO_API_DECL neko_gui_container_t* neko_gui_get_top_most_container(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    if (!split) return NULL;
    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_CONTAINER) return split->children[0].container;
    if (split->children[1].type == NEKO_GUI_SPLIT_NODE_CONTAINER) return split->children[1].container;
    neko_gui_container_t* c0 = neko_gui_get_top_most_container(ctx, neko_slot_array_getp(ctx->splits, split->children[0].split));
    neko_gui_container_t* c1 = neko_gui_get_top_most_container(ctx, neko_slot_array_getp(ctx->splits, split->children[1].split));
    if (c0->zindex > c1->zindex) return c0;
    return c1;
}

NEKO_API_DECL void neko_gui_bring_split_to_front(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    if (!split) return;

    if (!split->parent) {
        neko_snprintfc(TMP, 256, "!dockspace%zu", (size_t)split);
        neko_gui_id id = neko_gui_get_id(ctx, TMP, 256);
        neko_gui_container_t* cnt = neko_gui_get_container(ctx, TMP);
        // if (cnt) neko_gui_bring_to_front(ctx, cnt);
        // cnt->zindex = 0;
    }

    neko_gui_split_node_t* c0 = &split->children[0];
    neko_gui_split_node_t* c1 = &split->children[1];

    if (c0->type == NEKO_GUI_SPLIT_NODE_CONTAINER) {
        neko_gui_bring_to_front(ctx, c0->container);
        // ctx->hover = c0;
    } else {
        neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, c0->split);
        neko_gui_bring_split_to_front(ctx, s);
    }

    if (c1->type == NEKO_GUI_SPLIT_NODE_CONTAINER) {
        neko_gui_bring_to_front(ctx, c1->container);
        // ctx->hover = c1;
    } else {
        neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, c1->split);
        neko_gui_bring_split_to_front(ctx, s);
    }
}

static void neko_gui_update_split(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    // Iterate through children, resize them based on size/position
    const neko_gui_rect_t* sr = &split->rect;
    const float ratio = split->ratio;
    switch (split->type) {
        case NEKO_GUI_SPLIT_LEFT: {
            if (split->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                // Update split
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
                s->rect = neko_gui_rect(sr->x + sr->w * ratio, sr->y, sr->w * (1.f - ratio), sr->h);
                neko_gui_update_split(ctx, s);
            }

            if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_CHILD].split);
                s->rect = neko_gui_rect(sr->x, sr->y, sr->w * (ratio), sr->h);
                neko_gui_update_split(ctx, s);
            }
        } break;

        case NEKO_GUI_SPLIT_RIGHT: {
            if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                // Update split
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_CHILD].split);
                s->rect = neko_gui_rect(sr->x + sr->w * (1.f - ratio), sr->y, sr->w * (ratio), sr->h);
                neko_gui_update_split(ctx, s);
            }

            if (split->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
                s->rect = neko_gui_rect(sr->x, sr->y, sr->w * (1.f - ratio), sr->h);
                neko_gui_update_split(ctx, s);
            }

        } break;

        case NEKO_GUI_SPLIT_TOP: {
            if (split->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                // Update split
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
                s->rect = neko_gui_rect(sr->x, sr->y + sr->h * (ratio), sr->w, sr->h * (1.f - ratio));
                neko_gui_update_split(ctx, s);
            }

            if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_CHILD].split);
                s->rect = neko_gui_rect(sr->x, sr->y, sr->w, sr->h * (ratio));
                neko_gui_update_split(ctx, s);
            }
        } break;

        case NEKO_GUI_SPLIT_BOTTOM: {
            if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                // Update split
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_CHILD].split);
                s->rect = neko_gui_rect(sr->x, sr->y + sr->h * (1.f - ratio), sr->w, sr->h * (ratio));
                neko_gui_update_split(ctx, s);
            }

            if (split->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                neko_gui_split_t* s = neko_slot_array_getp(ctx->splits, split->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
                s->rect = neko_gui_rect(sr->x, sr->y, sr->w, sr->h * (1.f - ratio));
                neko_gui_update_split(ctx, s);
            }
        } break;
    }
}

static neko_gui_split_t* neko_gui_get_root_split_from_split(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    if (!split) return NULL;

    // Cache top root level split
    neko_gui_split_t* root_split = split && split->parent ? neko_slot_array_getp(ctx->splits, split->parent) : split ? split : NULL;
    while (root_split && root_split->parent) {
        root_split = neko_slot_array_getp(ctx->splits, root_split->parent);
    }

    return root_split;
}

static neko_gui_split_t* neko_gui_get_root_split(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    neko_gui_split_t* split = neko_gui_get_split(ctx, cnt);
    if (split)
        return neko_gui_get_root_split_from_split(ctx, split);
    else
        return NULL;
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_root_container_from_split(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    neko_gui_split_t* root = neko_gui_get_root_split_from_split(ctx, split);
    neko_gui_split_t* s = root;
    neko_gui_container_t* c = NULL;
    while (s && !c) {
        if (s->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
            s = neko_slot_array_getp(ctx->splits, s->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
        } else if (s->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_CONTAINER) {
            c = s->children[NEKO_GUI_SPLIT_NODE_PARENT].container;
        } else {
            s = NULL;
        }
    }
    return c;
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_root_container(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    neko_gui_container_t* parent = neko_gui_get_parent(ctx, cnt);
    if (parent->split) {
        neko_gui_split_t* root = neko_gui_get_root_split(ctx, parent);
        neko_gui_split_t* s = root;
        neko_gui_container_t* c = NULL;
        while (s && !c) {
            if (s->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
                s = neko_slot_array_getp(ctx->splits, s->children[NEKO_GUI_SPLIT_NODE_PARENT].split);
            } else if (s->children[NEKO_GUI_SPLIT_NODE_PARENT].type == NEKO_GUI_SPLIT_NODE_CONTAINER) {
                c = s->children[NEKO_GUI_SPLIT_NODE_PARENT].container;
            } else {
                s = NULL;
            }
        }
        return c;
    }

    return parent;
}

NEKO_API_DECL neko_gui_tab_bar_t* neko_gui_get_tab_bar(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    return ((cnt->tab_bar && cnt->tab_bar < neko_slot_array_size(ctx->tab_bars)) ? neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL);
}

NEKO_API_DECL neko_gui_split_t* neko_gui_get_split(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    neko_gui_tab_bar_t* tab_bar = neko_gui_get_tab_bar(ctx, cnt);
    neko_gui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;
    neko_gui_split_t* split = cnt->split ? neko_slot_array_getp(ctx->splits, cnt->split) : NULL;

    // Look at split if in tab group
    if (!split && tab_bar) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((neko_gui_container_t*)tab_bar->items[i].data)->split) {
                split = neko_slot_array_getp(ctx->splits, ((neko_gui_container_t*)tab_bar->items[i].data)->split);
            }
        }
    }

    return split;
}

static neko_gui_command_t* neko_gui_push_jump(neko_gui_context_t* ctx, neko_gui_command_t* dst) {
    neko_gui_command_t* cmd;
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_JUMP, sizeof(neko_gui_jumpcommand_t));
    cmd->jump.dst = dst;
    return cmd;
}

static void neko_gui_draw_frame(neko_gui_context_t* ctx, neko_gui_rect_t rect, neko_gui_style_t* style) {
    neko_gui_draw_rect(ctx, rect, style->colors[NEKO_GUI_COLOR_BACKGROUND]);

    // draw border
    if (style->colors[NEKO_GUI_COLOR_BORDER].a) {
        neko_gui_draw_box(ctx, neko_gui_expand_rect(rect, (s16*)style->border_width), (s16*)style->border_width, style->colors[NEKO_GUI_COLOR_BORDER]);
    }
}

static s32 neko_gui_compare_zindex(const void* a, const void* b) { return (*(neko_gui_container_t**)a)->zindex - (*(neko_gui_container_t**)b)->zindex; }

static neko_gui_style_t* neko_gui_push_style(neko_gui_context_t* ctx, neko_gui_style_t* style) {
    neko_gui_style_t* save = ctx->style;
    ctx->style = style;
    return save;
}

static void neko_gui_push_inline_style(neko_gui_context_t* ctx, neko_gui_element_type elementid, neko_gui_inline_style_desc_t* desc) {
    if (elementid >= NEKO_GUI_ELEMENT_COUNT || !desc) {
        return;
    }

    if (!neko_hash_table_exists(ctx->inline_styles, elementid)) {
        neko_gui_inline_style_stack_t v = neko_default_val();
        neko_hash_table_insert(ctx->inline_styles, elementid, v);
    }

    neko_gui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, elementid);
    neko_assert(iss);

    // Counts to keep for popping off
    u32 style_ct[3] = neko_default_val(), anim_ct[3] = neko_default_val();

    if (desc->all.style.data && desc->all.style.size) {
        // Total amount to write for each section
        u32 ct = desc->all.style.size / sizeof(neko_gui_style_element_t);
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
        u32 ct = desc->all.animation.size / sizeof(neko_gui_animation_property_t);
        anim_ct[0] += ct;
        anim_ct[1] += ct;
        anim_ct[2] += ct;

        for (u32 i = 0; i < ct; ++i) {
            neko_dyn_array_push(iss->animations[0], desc->all.animation.data[i]);
            neko_dyn_array_push(iss->animations[1], desc->all.animation.data[i]);
            neko_dyn_array_push(iss->animations[2], desc->all.animation.data[i]);
        }
    }

#define NEKO_GUI_COPY_INLINE_STYLE(TYPE, INDEX)                                            \
    do {                                                                                   \
        if (desc->TYPE.style.data && desc->TYPE.style.size) {                              \
            u32 ct = desc->TYPE.style.size / sizeof(neko_gui_style_element_t);             \
            style_ct[INDEX] += ct;                                                         \
            for (u32 i = 0; i < ct; ++i) {                                                 \
                neko_dyn_array_push(iss->styles[INDEX], desc->TYPE.style.data[i]);         \
            }                                                                              \
        }                                                                                  \
        if (desc->TYPE.animation.data && desc->TYPE.animation.size) {                      \
            u32 ct = desc->TYPE.animation.size / sizeof(neko_gui_animation_property_t);    \
            anim_ct[INDEX] += ct;                                                          \
                                                                                           \
            for (u32 i = 0; i < ct; ++i) {                                                 \
                neko_dyn_array_push(iss->animations[INDEX], desc->TYPE.animation.data[i]); \
            }                                                                              \
        }                                                                                  \
    } while (0)

    // Copy remaining individual styles
    NEKO_GUI_COPY_INLINE_STYLE(def, 0);
    NEKO_GUI_COPY_INLINE_STYLE(hover, 1);
    NEKO_GUI_COPY_INLINE_STYLE(focus, 2);

    // Add final counts
    neko_dyn_array_push(iss->style_counts, style_ct[0]);
    neko_dyn_array_push(iss->style_counts, style_ct[1]);
    neko_dyn_array_push(iss->style_counts, style_ct[2]);

    neko_dyn_array_push(iss->animation_counts, anim_ct[0]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[1]);
    neko_dyn_array_push(iss->animation_counts, anim_ct[2]);
}

static void neko_gui_pop_inline_style(neko_gui_context_t* ctx, neko_gui_element_type elementid) {
    if (elementid >= NEKO_GUI_ELEMENT_COUNT) {
        return;
    }

    if (!neko_hash_table_exists(ctx->inline_styles, elementid)) {
        return;
    }

    neko_gui_inline_style_stack_t* iss = neko_hash_table_getp(ctx->inline_styles, elementid);
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

static void neko_gui_pop_style(neko_gui_context_t* ctx, neko_gui_style_t* style) { ctx->style = style; }

static void neko_gui_push_layout(neko_gui_context_t* ctx, neko_gui_rect_t body, neko_vec2 scroll) {
    neko_gui_layout_t layout;
    s32 width = 0;
    memset(&layout, 0, sizeof(layout));
    layout.body = neko_gui_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
    layout.max = neko_v2(-0x1000000, -0x1000000);
    layout.direction = ctx->style->direction;
    layout.justify_content = ctx->style->justify_content;
    layout.align_content = ctx->style->align_content;
    memcpy(layout.padding, ctx->style->padding, sizeof(s32) * 4);
    neko_gui_stack_push(ctx->layout_stack, layout);
    neko_gui_layout_row(ctx, 1, &width, 0);
}

static void neko_gui_pop_layout(neko_gui_context_t* ctx) { neko_gui_stack_pop(ctx->layout_stack); }

NEKO_API_DECL neko_gui_layout_t* neko_gui_get_layout(neko_gui_context_t* ctx) { return &ctx->layout_stack.items[ctx->layout_stack.idx - 1]; }

static void neko_gui_pop_container(neko_gui_context_t* ctx) {
    neko_gui_container_t* cnt = neko_gui_get_current_container(ctx);
    neko_gui_layout_t* layout = neko_gui_get_layout(ctx);
    cnt->content_size.x = layout->max.x - layout->body.x;
    cnt->content_size.y = layout->max.y - layout->body.y;

    /* pop container, layout and id */
    neko_gui_stack_pop(ctx->container_stack);
    neko_gui_stack_pop(ctx->layout_stack);
    neko_gui_pop_id(ctx);
}

#define neko_gui_scrollbar(ctx, cnt, b, cs, x, y, w, h)                                                                                                       \
    do {                                                                                                                                                      \
        /* only add scrollbar if content size is larger than body */                                                                                          \
        s32 maxscroll = (s32)(cs.y - b->h);                                                                                                                   \
                                                                                                                                                              \
        if (maxscroll > 0 && b->h > 0) {                                                                                                                      \
            neko_gui_rect_t base, thumb;                                                                                                                      \
            neko_gui_id id = neko_gui_get_id(ctx, "!scrollbar" #y, 11);                                                                                       \
            const s32 elementid = NEKO_GUI_ELEMENT_SCROLL;                                                                                                    \
            neko_gui_style_t style = neko_default_val();                                                                                                      \
            neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);                                                                    \
                                                                                                                                                              \
            /* Update anim (keep states locally within animation, only way to do this)*/                                                                      \
            if (anim) {                                                                                                                                       \
                neko_gui_animation_update(ctx, anim);                                                                                                         \
                                                                                                                                                              \
                /* Get blended style based on animation*/                                                                                                     \
                style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);                                                                       \
            } else {                                                                                                                                          \
                style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)                                                   \
                        : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)                                                   \
                                           : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);                                                  \
            }                                                                                                                                                 \
                                                                                                                                                              \
            s32 sz = (s32)style.size[0];                                                                                                                      \
            if (cs.y > cnt->body.h) {                                                                                                                         \
                body->w -= sz;                                                                                                                                \
            }                                                                                                                                                 \
            if (cs.x > cnt->body.w) {                                                                                                                         \
                body->h -= sz;                                                                                                                                \
            }                                                                                                                                                 \
                                                                                                                                                              \
            /* get sizing / positioning */                                                                                                                    \
            base = *b;                                                                                                                                        \
            base.x = b->x + b->w;                                                                                                                             \
            base.w = style.size[0];                                                                                                                           \
                                                                                                                                                              \
            /* handle input */                                                                                                                                \
            neko_gui_update_control(ctx, id, base, 0);                                                                                                        \
            if (ctx->focus == id && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {                                                                                 \
                cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;                                                                                          \
            }                                                                                                                                                 \
            /* clamp scroll to limits */                                                                                                                      \
            cnt->scroll.y = neko_clamp(cnt->scroll.y, 0, maxscroll);                                                                                          \
            s32 state = ctx->focus == id ? NEKO_GUI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_GUI_ELEMENT_STATE_HOVER : 0x00;                             \
                                                                                                                                                              \
            /* draw base and thumb */                                                                                                                         \
            neko_gui_draw_rect(ctx, base, style.colors[NEKO_GUI_COLOR_BACKGROUND]);                                                                           \
            /* draw border*/                                                                                                                                  \
            if (style.colors[NEKO_GUI_COLOR_BORDER].a) {                                                                                                      \
                neko_gui_draw_box(ctx, neko_gui_expand_rect(base, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]);  \
            }                                                                                                                                                 \
            float pl = ((float)style.padding[NEKO_GUI_PADDING_LEFT]);                                                                                         \
            float pr = ((float)style.padding[NEKO_GUI_PADDING_RIGHT]);                                                                                        \
            float pt = ((float)style.padding[NEKO_GUI_PADDING_TOP]);                                                                                          \
            float pb = ((float)style.padding[NEKO_GUI_PADDING_BOTTOM]);                                                                                       \
            float w = ((float)base.w - pr);                                                                                                                   \
            float x = (float)(base.x + pl);                                                                                                                   \
            thumb = base;                                                                                                                                     \
            thumb.x = x;                                                                                                                                      \
            thumb.w = w;                                                                                                                                      \
            thumb.h = neko_max(style.thumb_size, base.h * b->h / cs.y) - pb;                                                                                  \
            thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll + pt;                                                                                   \
            neko_gui_draw_rect(ctx, thumb, style.colors[NEKO_GUI_COLOR_CONTENT]);                                                                             \
            /* draw border*/                                                                                                                                  \
            if (style.colors[NEKO_GUI_COLOR_BORDER].a) {                                                                                                      \
                neko_gui_draw_box(ctx, neko_gui_expand_rect(thumb, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]); \
            }                                                                                                                                                 \
                                                                                                                                                              \
            /* set this as the scroll_target (will get scrolled on mousewheel) */                                                                             \
            /* if the mouse is over it */                                                                                                                     \
            if (neko_gui_mouse_over(ctx, *b) || neko_gui_mouse_over(ctx, base) || neko_gui_mouse_over(ctx, thumb)) {                                          \
                ctx->scroll_target = cnt;                                                                                                                     \
            }                                                                                                                                                 \
        }                                                                                                                                                     \
    } while (0)

static void neko_gui_scrollbars(neko_gui_context_t* ctx, neko_gui_container_t* cnt, neko_gui_rect_t* body, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 sz = (s32)ctx->style_sheet->styles[NEKO_GUI_ELEMENT_SCROLL][0x00].size[0];
    neko_vec2 cs = cnt->content_size;
    cs.x += ctx->style->padding[NEKO_GUI_PADDING_LEFT] * 2;
    cs.y += ctx->style->padding[NEKO_GUI_PADDING_TOP] * 2;
    neko_gui_push_clip_rect(ctx, *body);

    /* resize body to make room for scrollbars */
    if (cs.y > cnt->body.h) {
        body->w -= sz;
    }
    if (cs.x > cnt->body.w) {
        body->h -= sz;
    }

    /* to create a horizontal or vertical scrollbar almost-identical code is
    ** used; only the references to `x|y` `w|h` need to be switched */
    neko_gui_scrollbar(ctx, cnt, body, cs, x, y, w, h);

    if (~opt & NEKO_GUI_OPT_NOSCROLLHORIZONTAL) {
        neko_gui_scrollbar(ctx, cnt, body, cs, y, x, h, w);
    }

    if (cs.y <= cnt->body.h) {
        cnt->scroll.y = 0;
    }
    if (cs.x <= cnt->body.w) {
        cnt->scroll.x = 0;
    }

    neko_gui_pop_clip_rect(ctx);
}

static void neko_gui_push_container_body(neko_gui_context_t* ctx, neko_gui_container_t* cnt, neko_gui_rect_t body, const neko_gui_selector_desc_t* desc, u64 opt) {
    if (~opt & NEKO_GUI_OPT_NOSCROLL) {
        neko_gui_scrollbars(ctx, cnt, &body, desc, opt);
    }
    s32* padding = ctx->style->padding;
    float l = body.x + padding[NEKO_GUI_PADDING_LEFT];
    float t = body.y + padding[NEKO_GUI_PADDING_TOP];
    float r = body.x + body.w - padding[NEKO_GUI_PADDING_RIGHT];
    float b = body.y + body.h - padding[NEKO_GUI_PADDING_BOTTOM];

    neko_gui_rect_t rect = neko_gui_rect(l, t, r - l, b - t);
    neko_gui_push_layout(ctx, rect, cnt->scroll);
    cnt->body = body;
}

static void neko_gui_begin_root_container(neko_gui_context_t* ctx, neko_gui_container_t* cnt, u64 opt) {
    neko_gui_stack_push(ctx->container_stack, cnt);

    /* push container to roots list and push head command */
    neko_gui_stack_push(ctx->root_list, cnt);
    cnt->head = neko_gui_push_jump(ctx, NULL);

    /* set as hover root if the mouse is overlapping this container and it has a
    ** higher zindex than the current hover root */
    if (neko_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && (!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex) && ~opt & NEKO_GUI_OPT_NOHOVER &&
        cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        ctx->next_hover_root = cnt;
    }

    /* clipping is reset here in case a root-container is made within
    ** another root-containers's begin/end block; this prevents the inner
    ** root-container being clipped to the outer */
    neko_gui_stack_push(ctx->clip_stack, neko_gui_unclipped_rect);
}

static void neko_gui_root_container_end(neko_gui_context_t* ctx) {
    /* push tail 'goto' jump command and set head 'skip' command. the final steps
    ** on initing these are done in neko_gui_end() */
    neko_gui_container_t* cnt = neko_gui_get_current_container(ctx);
    cnt->tail = neko_gui_push_jump(ctx, NULL);
    cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;

    /* pop base clip rect and container */
    neko_gui_pop_clip_rect(ctx);
    neko_gui_pop_container(ctx);
}

#define NEKO_GUI_COPY_STYLES(DST, SRC, ELEM) \
    do {                                     \
        DST[ELEM][0x00] = SRC[ELEM][0x00];   \
        DST[ELEM][0x01] = SRC[ELEM][0x01];   \
        DST[ELEM][0x02] = SRC[ELEM][0x02];   \
    } while (0)

NEKO_API_DECL neko_gui_style_sheet_t neko_gui_style_sheet_create(neko_gui_context_t* ctx, neko_gui_style_sheet_desc_t* desc) {
    // Generate new style sheet based on default element styles
    neko_gui_style_sheet_t style_sheet = neko_default_val();

    // Copy all default styles
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_CONTAINER);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_LABEL);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_TEXT);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_PANEL);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_INPUT);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_BUTTON);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_SCROLL);
    NEKO_GUI_COPY_STYLES(style_sheet.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_IMAGE);

    // NEKO_API_DECL void neko_gui_style_sheet_set_element_styles(neko_gui_style_sheet_t* style_sheet, neko_gui_element_type element, neko_gui_element_state state, neko_gui_style_element_t* styles,
    // size_t size);

#define NEKO_GUI_APPLY_STYLE_ELEMENT(ELEMENT, TYPE)                                                                                                            \
    do {                                                                                                                                                       \
        if ((ELEMENT).all.style.data) {                                                                                                                        \
            neko_gui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_GUI_ELEMENT_STATE_NEG, (ELEMENT).all.style.data, (ELEMENT).all.style.size);       \
        } else if ((ELEMENT).def.style.data) {                                                                                                                 \
            neko_gui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_GUI_ELEMENT_STATE_DEFAULT, (ELEMENT).def.style.data, (ELEMENT).def.style.size);   \
        }                                                                                                                                                      \
        if ((ELEMENT).hover.style.data) {                                                                                                                      \
            neko_gui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_GUI_ELEMENT_STATE_HOVER, (ELEMENT).hover.style.data, (ELEMENT).hover.style.size); \
        }                                                                                                                                                      \
        if ((ELEMENT).focus.style.data) {                                                                                                                      \
            neko_gui_style_sheet_set_element_styles(&style_sheet, TYPE, NEKO_GUI_ELEMENT_STATE_FOCUS, (ELEMENT).focus.style.data, (ELEMENT).focus.style.size); \
        }                                                                                                                                                      \
    } while (0)

    // Iterate through descriptor
    if (desc) {
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->button, NEKO_GUI_ELEMENT_BUTTON);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->container, NEKO_GUI_ELEMENT_CONTAINER);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->panel, NEKO_GUI_ELEMENT_PANEL);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->scroll, NEKO_GUI_ELEMENT_SCROLL);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->image, NEKO_GUI_ELEMENT_IMAGE);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->label, NEKO_GUI_ELEMENT_LABEL);
        NEKO_GUI_APPLY_STYLE_ELEMENT(desc->text, NEKO_GUI_ELEMENT_TEXT);
    }

#define COPY_ANIM_DATA(TYPE, ELEMENT)                                                                         \
    do {                                                                                                      \
        /* Apply animations */                                                                                \
        if (desc->TYPE.all.animation.data) {                                                                  \
            s32 cnt = desc->TYPE.all.animation.size / sizeof(neko_gui_animation_property_t);                  \
            if (!neko_hash_table_exists(style_sheet.animations, ELEMENT)) {                                   \
                neko_gui_animation_property_list_t v = neko_default_val();                                    \
                neko_hash_table_insert(style_sheet.animations, ELEMENT, v);                                   \
            }                                                                                                 \
            neko_gui_animation_property_list_t* list = neko_hash_table_getp(style_sheet.animations, ELEMENT); \
            neko_assert(list);                                                                                \
            /* Register animation properties for all */                                                       \
            for (u32 i = 0; i < 3; ++i) {                                                                     \
                for (u32 c = 0; c < cnt; ++c) {                                                               \
                    neko_dyn_array_push(list->properties[i], desc->TYPE.all.animation.data[c]);               \
                }                                                                                             \
            }                                                                                                 \
        }                                                                                                     \
    } while (0)

    // Copy animations
    COPY_ANIM_DATA(button, NEKO_GUI_ELEMENT_BUTTON);
    COPY_ANIM_DATA(label, NEKO_GUI_ELEMENT_LABEL);
    COPY_ANIM_DATA(scroll, NEKO_GUI_ELEMENT_SCROLL);
    COPY_ANIM_DATA(image, NEKO_GUI_ELEMENT_IMAGE);
    COPY_ANIM_DATA(panel, NEKO_GUI_ELEMENT_PANEL);
    COPY_ANIM_DATA(text, NEKO_GUI_ELEMENT_TEXT);
    COPY_ANIM_DATA(container, NEKO_GUI_ELEMENT_CONTAINER);

    return style_sheet;
}

NEKO_API_DECL void neko_gui_style_sheet_destroy(neko_gui_style_sheet_t* ss) {
    // Need to free all animations
    if (!ss || !ss->animations) {
        neko_log_warning("Trying to destroy invalid style sheet");
        return;
    }

    for (neko_hash_table_iter it = neko_hash_table_iter_new(ss->animations); neko_hash_table_iter_valid(ss->animations, it); neko_hash_table_iter_advance(ss->animations, it)) {
        neko_gui_animation_property_list_t* list = neko_hash_table_iter_getp(ss->animations, it);
        for (u32 i = 0; i < 3; ++i) {
            neko_dyn_array_free(list->properties[i]);
        }
    }
    neko_hash_table_free(ss->animations);
}

NEKO_API_DECL void neko_gui_set_style_sheet(neko_gui_context_t* ctx, neko_gui_style_sheet_t* style_sheet) { ctx->style_sheet = style_sheet ? style_sheet : &neko_gui_default_style_sheet; }

NEKO_API_DECL void neko_gui_style_sheet_set_element_styles(neko_gui_style_sheet_t* ss, neko_gui_element_type element, neko_gui_element_state state, neko_gui_style_element_t* styles, size_t size) {
    const u32 count = size / sizeof(neko_gui_style_element_t);
    u32 idx_cnt = 1;
    u32 idx = 0;

    // Switch on state
    switch (state) {
        // Do all
        default:
            idx_cnt = 3;
            break;
        case NEKO_GUI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case NEKO_GUI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case NEKO_GUI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        neko_gui_style_t* cs = &ss->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            neko_gui_style_element_t* se = &styles[i];

            switch (se->type) {
                // Width/Height
                case NEKO_GUI_STYLE_WIDTH:
                    cs->size[0] = (float)se->value;
                    break;
                case NEKO_GUI_STYLE_HEIGHT:
                    cs->size[1] = (float)se->value;
                    break;

                // Padding
                case NEKO_GUI_STYLE_PADDING: {
                    cs->padding[NEKO_GUI_PADDING_LEFT] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_TOP] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_RIGHT] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_BOTTOM] = (s32)se->value;
                }
                case NEKO_GUI_STYLE_PADDING_LEFT:
                    cs->padding[NEKO_GUI_PADDING_LEFT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_TOP:
                    cs->padding[NEKO_GUI_PADDING_TOP] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_RIGHT:
                    cs->padding[NEKO_GUI_PADDING_RIGHT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_BOTTOM:
                    cs->padding[NEKO_GUI_PADDING_BOTTOM] = (s32)se->value;
                    break;

                case NEKO_GUI_STYLE_MARGIN: {
                    cs->margin[NEKO_GUI_MARGIN_LEFT] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_TOP] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_RIGHT] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)se->value;
                } break;

                case NEKO_GUI_STYLE_MARGIN_LEFT:
                    cs->margin[NEKO_GUI_MARGIN_LEFT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_TOP:
                    cs->margin[NEKO_GUI_MARGIN_TOP] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_RIGHT:
                    cs->margin[NEKO_GUI_MARGIN_RIGHT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_BOTTOM:
                    cs->margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)se->value;
                    break;

                // Border
                case NEKO_GUI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case NEKO_GUI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case NEKO_GUI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case NEKO_GUI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case NEKO_GUI_STYLE_DIRECTION:
                    cs->direction = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (s32)se->value;
                    break;

                // Shadow
                case NEKO_GUI_STYLE_SHADOW_X:
                    cs->shadow_x = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_SHADOW_Y:
                    cs->shadow_y = (s32)se->value;
                    break;

                // Colors
                case NEKO_GUI_STYLE_COLOR_BACKGROUND:
                    cs->colors[NEKO_GUI_COLOR_BACKGROUND] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_BORDER:
                    cs->colors[NEKO_GUI_COLOR_BORDER] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_SHADOW:
                    cs->colors[NEKO_GUI_COLOR_SHADOW] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT:
                    cs->colors[NEKO_GUI_COLOR_CONTENT] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case NEKO_GUI_STYLE_FONT:
                    cs->font = se->font;
                    break;
            }
        }
    }
}

NEKO_API_DECL void neko_gui_set_element_style(neko_gui_context_t* ctx, neko_gui_element_type element, neko_gui_element_state state, neko_gui_style_element_t* style, size_t size) {
    const u32 count = size / sizeof(neko_gui_style_element_t);
    u32 idx_cnt = 1;
    u32 idx = 0;

    // Switch on state
    switch (state) {
        // Do all
        default:
            idx_cnt = 3;
            break;
        case NEKO_GUI_ELEMENT_STATE_DEFAULT:
            idx = 0;
            break;
        case NEKO_GUI_ELEMENT_STATE_HOVER:
            idx = 1;
            break;
        case NEKO_GUI_ELEMENT_STATE_FOCUS:
            idx = 2;
            break;
    }

    for (u32 s = idx, c = 0; c < idx_cnt; ++s, ++c) {
        neko_gui_style_t* cs = &ctx->style_sheet->styles[element][s];
        for (u32 i = 0; i < count; ++i) {
            neko_gui_style_element_t* se = &style[i];

            switch (se->type) {
                // Width/Height
                case NEKO_GUI_STYLE_WIDTH:
                    cs->size[0] = (float)se->value;
                    break;
                case NEKO_GUI_STYLE_HEIGHT:
                    cs->size[1] = (float)se->value;
                    break;

                // Padding
                case NEKO_GUI_STYLE_PADDING: {
                    cs->padding[NEKO_GUI_PADDING_LEFT] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_TOP] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_RIGHT] = (s32)se->value;
                    cs->padding[NEKO_GUI_PADDING_BOTTOM] = (s32)se->value;
                }
                case NEKO_GUI_STYLE_PADDING_LEFT:
                    cs->padding[NEKO_GUI_PADDING_LEFT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_TOP:
                    cs->padding[NEKO_GUI_PADDING_TOP] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_RIGHT:
                    cs->padding[NEKO_GUI_PADDING_RIGHT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_PADDING_BOTTOM:
                    cs->padding[NEKO_GUI_PADDING_BOTTOM] = (s32)se->value;
                    break;

                case NEKO_GUI_STYLE_MARGIN: {
                    cs->margin[NEKO_GUI_MARGIN_LEFT] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_TOP] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_RIGHT] = (s32)se->value;
                    cs->margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)se->value;
                } break;

                case NEKO_GUI_STYLE_MARGIN_LEFT:
                    cs->margin[NEKO_GUI_MARGIN_LEFT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_TOP:
                    cs->margin[NEKO_GUI_MARGIN_TOP] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_RIGHT:
                    cs->margin[NEKO_GUI_MARGIN_RIGHT] = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_MARGIN_BOTTOM:
                    cs->margin[NEKO_GUI_MARGIN_BOTTOM] = (s32)se->value;
                    break;

                // Border
                case NEKO_GUI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case NEKO_GUI_STYLE_BORDER_RADIUS_LEFT:
                    cs->border_radius[0] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_RIGHT:
                    cs->border_radius[1] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_TOP:
                    cs->border_radius[2] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_RADIUS_BOTTOM:
                    cs->border_radius[3] = se->value;
                    break;

                case NEKO_GUI_STYLE_BORDER_WIDTH: {
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case NEKO_GUI_STYLE_BORDER_WIDTH_LEFT:
                    cs->border_width[0] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_RIGHT:
                    cs->border_width[1] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_TOP:
                    cs->border_width[2] = se->value;
                    break;
                case NEKO_GUI_STYLE_BORDER_WIDTH_BOTTOM:
                    cs->border_width[3] = se->value;
                    break;

                // Flex
                case NEKO_GUI_STYLE_DIRECTION:
                    cs->direction = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_ALIGN_CONTENT:
                    cs->align_content = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_JUSTIFY_CONTENT:
                    cs->justify_content = (s32)se->value;
                    break;

                // Shadow
                case NEKO_GUI_STYLE_SHADOW_X:
                    cs->shadow_x = (s32)se->value;
                    break;
                case NEKO_GUI_STYLE_SHADOW_Y:
                    cs->shadow_y = (s32)se->value;
                    break;

                // Colors
                case NEKO_GUI_STYLE_COLOR_BACKGROUND:
                    cs->colors[NEKO_GUI_COLOR_BACKGROUND] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_BORDER:
                    cs->colors[NEKO_GUI_COLOR_BORDER] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_SHADOW:
                    cs->colors[NEKO_GUI_COLOR_SHADOW] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT:
                    cs->colors[NEKO_GUI_COLOR_CONTENT] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_BACKGROUND] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_BORDER:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_BORDER] = se->color;
                    break;
                case NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW:
                    cs->colors[NEKO_GUI_COLOR_CONTENT_SHADOW] = se->color;
                    break;

                // Font
                case NEKO_GUI_STYLE_FONT:
                    cs->font = se->font;
                    break;
            }
        }
    }
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_container_ex(neko_gui_context_t* ctx, neko_gui_id id, u64 opt) {
    neko_gui_container_t* cnt;

    /* try to get existing container from pool */
    s32 idx = neko_gui_pool_get(ctx, ctx->container_pool, NEKO_GUI_CONTAINERPOOL_SIZE, id);

    if (idx >= 0) {
        if (ctx->containers[idx].open || ~opt & NEKO_GUI_OPT_CLOSED) {
            neko_gui_pool_update(ctx, ctx->container_pool, idx);
        }
        return &ctx->containers[idx];
    }

    if (opt & NEKO_GUI_OPT_CLOSED) {
        return NULL;
    }

    /* container not found in pool: init new container */
    idx = neko_gui_pool_init(ctx, ctx->container_pool, NEKO_GUI_CONTAINERPOOL_SIZE, id);
    cnt = &ctx->containers[idx];
    memset(cnt, 0, sizeof(*cnt));
    cnt->open = 1;
    cnt->id = id;
    cnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE | NEKO_GUI_WINDOW_FLAGS_FIRST_INIT;
    neko_gui_bring_to_front(ctx, cnt);

    // neko_println("CONSTRUCTING: %zu", id);

    return cnt;
}

static s32 neko_gui_text_width(neko_asset_ascii_font_t* font, const char* text, s32 len) {
    neko_vec2 td = neko_asset_ascii_font_text_dimensions(font, text, len);
    return (s32)td.x;
}

static s32 neko_gui_text_height(neko_asset_ascii_font_t* font, const char* text, s32 len) {
    return (s32)neko_asset_ascii_font_max_height(font);
    neko_vec2 td = neko_asset_ascii_font_text_dimensions(font, text, len);
    return (s32)td.y;
}

// Grabs max height for a given font
static s32 neko_gui_font_height(neko_asset_ascii_font_t* font) { return (s32)neko_asset_ascii_font_max_height(font); }

static neko_vec2 neko_gui_text_dimensions(neko_asset_ascii_font_t* font, const char* text, s32 len) {
    neko_vec2 td = neko_asset_ascii_font_text_dimensions(font, text, len);
    return td;
}

// =========================== //
// ======== Docking ========== //
// =========================== //

NEKO_API_DECL void neko_gui_dock_ex(neko_gui_context_t* ctx, const char* dst, const char* src, s32 split_type, float ratio) {
    neko_gui_hints_t hints = neko_default_val();
    hints.framebuffer_size = neko_platform_framebuffer_sizev(ctx->window_hndl);
    hints.viewport = neko_gui_rect(0.f, 0.f, 0.f, 0.f);
    u32 f = ctx->frame;
    if (!f) neko_gui_begin(ctx, &hints);
    neko_gui_container_t* dst_cnt = neko_gui_get_container(ctx, dst);
    neko_gui_container_t* src_cnt = neko_gui_get_container(ctx, src);
    neko_gui_dock_ex_cnt(ctx, dst_cnt, src_cnt, split_type, ratio);
    if (f != ctx->frame) neko_gui_end(ctx, true);
}

NEKO_API_DECL void neko_gui_undock_ex(neko_gui_context_t* ctx, const char* name) {
    neko_gui_container_t* cnt = neko_gui_get_container(ctx, name);
    neko_gui_undock_ex_cnt(ctx, cnt);
}

void neko_gui_set_split(neko_gui_context_t* ctx, neko_gui_container_t* cnt, u32 id) {
    cnt->split = id;
    neko_gui_tab_bar_t* tb = neko_gui_get_tab_bar(ctx, cnt);
    if (tb) {
        for (u32 i = 0; i < tb->size; ++i) {
            ((neko_gui_container_t*)tb->items[i].data)->split = id;
        }
    }
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_parent(neko_gui_context_t* ctx, neko_gui_container_t* cnt) { return (cnt->parent ? cnt->parent : cnt); }

NEKO_API_DECL void neko_gui_dock_ex_cnt(neko_gui_context_t* ctx, neko_gui_container_t* child, neko_gui_container_t* parent, s32 split_type, float ratio) {
    // Get top-level parent windows
    parent = neko_gui_get_parent(ctx, parent);
    child = neko_gui_get_parent(ctx, child);

    if (!child || !parent) {
        return;
    }

    if (split_type == NEKO_GUI_SPLIT_TAB) {
        // If the parent window has a tab bar, then need to get that tab bar item and add it
        if (parent->tab_bar) {
            neko_println("add to tab bar");

            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, parent->tab_bar);
            neko_assert(tab_bar);

            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar) {
                u32 tbid = child->tab_bar;
                neko_gui_tab_bar_t* ctb = neko_slot_array_getp(ctx->tab_bars, child->tab_bar);
                for (u32 i = 0; i < ctb->size; ++i) {
                    neko_gui_tab_item_t* cti = &tab_bar->items[tab_bar->size];
                    neko_gui_container_t* c = (neko_gui_container_t*)ctb->items[i].data;
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
                neko_gui_tab_item_t* tab_item = &tab_bar->items[tab_bar->size];
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
        }
        // Otherwise, create new tab bar
        else {
            neko_log_info("create tab bar");

            // Create tab bar
            neko_gui_tab_bar_t tb = neko_default_val();
            u32 hndl = neko_slot_array_insert(ctx->tab_bars, tb);
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, hndl);
            neko_assert(tab_bar);

            // Create tab items
            neko_gui_tab_item_t* parent_tab_item = &tab_bar->items[tab_bar->size];
            parent_tab_item->zindex = tab_bar->size++;
            parent_tab_item->tab_bar = hndl;

            // Set parent tab item
            parent->tab_item = 0;
            parent_tab_item->data = parent;

            u32 tbid = child->tab_bar;

            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar) {
                neko_gui_tab_bar_t* ctb = neko_slot_array_getp(ctx->tab_bars, child->tab_bar);
                for (u32 i = 0; i < ctb->size; ++i) {
                    neko_gui_tab_item_t* cti = &tab_bar->items[tab_bar->size];
                    neko_gui_container_t* c = (neko_gui_container_t*)ctb->items[i].data;
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
                neko_gui_tab_item_t* child_tab_item = &tab_bar->items[tab_bar->size];
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

        neko_gui_split_t* root_split = neko_gui_get_root_split(ctx, parent);
        if (root_split) {
            neko_gui_update_split(ctx, root_split);
            neko_gui_bring_split_to_front(ctx, root_split);
        }
    } else {
        // Cache previous root splits
        neko_gui_split_t* ps = neko_gui_get_root_split(ctx, parent);
        neko_gui_split_t* cs = neko_gui_get_root_split(ctx, child);

        neko_gui_tab_bar_t* tab_bar = neko_gui_get_tab_bar(ctx, parent);

        neko_gui_split_t split = neko_default_val();
        split.type = (neko_gui_split_type)split_type;
        split.ratio = ratio;
        neko_gui_split_node_t c0 = neko_default_val();
        c0.type = NEKO_GUI_SPLIT_NODE_CONTAINER;
        c0.container = child;
        neko_gui_split_node_t c1 = neko_default_val();
        c1.type = NEKO_GUI_SPLIT_NODE_CONTAINER;
        c1.container = parent;
        split.children[NEKO_GUI_SPLIT_NODE_CHILD] = c0;
        split.children[NEKO_GUI_SPLIT_NODE_PARENT] = c1;
        split.rect = ps ? ps->rect : parent->rect;
        split.prev_rect = split.rect;

        // Add new split to array
        u32 hndl = neko_slot_array_insert(ctx->splits, split);

        // Get newly inserted split pointer
        neko_gui_split_t* sp = neko_slot_array_getp(ctx->splits, hndl);
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
            sp->children[NEKO_GUI_SPLIT_NODE_CHILD].type = NEKO_GUI_SPLIT_NODE_SPLIT;
            sp->children[NEKO_GUI_SPLIT_NODE_CHILD].split = cs->id;

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
            if (ps->children[NEKO_GUI_SPLIT_NODE_PARENT].container == parent) {
                ps->children[NEKO_GUI_SPLIT_NODE_PARENT].type = NEKO_GUI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_GUI_SPLIT_NODE_PARENT].split = hndl;
            } else {
                ps->children[NEKO_GUI_SPLIT_NODE_CHILD].type = NEKO_GUI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_GUI_SPLIT_NODE_CHILD].split = hndl;
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
            sp->children[NEKO_GUI_SPLIT_NODE_CHILD].type = NEKO_GUI_SPLIT_NODE_SPLIT;
            sp->children[NEKO_GUI_SPLIT_NODE_CHILD].split = cs->id;

            // Need to check which node to replace
            if (ps->children[NEKO_GUI_SPLIT_NODE_CHILD].container == parent) {
                ps->children[NEKO_GUI_SPLIT_NODE_CHILD].type = NEKO_GUI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_GUI_SPLIT_NODE_CHILD].split = hndl;
            } else {
                ps->children[NEKO_GUI_SPLIT_NODE_PARENT].type = NEKO_GUI_SPLIT_NODE_SPLIT;
                ps->children[NEKO_GUI_SPLIT_NODE_PARENT].split = hndl;
            }

            cs->parent = hndl;
            parent->split = hndl;
        }

        neko_gui_split_t* root_split = neko_gui_get_root_split(ctx, parent);
        neko_gui_update_split(ctx, root_split);
        neko_gui_bring_split_to_front(ctx, root_split);
    }
}

NEKO_API_DECL void neko_gui_undock_ex_cnt(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    // If has a tab item idx, need to grab that
    neko_gui_split_t* split = neko_gui_get_split(ctx, cnt);

    // Get root split for container
    neko_gui_split_t* root_split = NULL;

    // Get parent split of this owning split
    neko_gui_split_t* ps = split && split->parent ? neko_slot_array_getp(ctx->splits, split->parent) : NULL;

    if (cnt->tab_bar) {
        // Get parent container for this container
        neko_gui_container_t* parent = neko_gui_get_parent(ctx, cnt);

        // Check if split
        if (parent->split) {
            // No split, so just do stuff normally...
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            neko_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

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
                    neko_gui_tab_item_swap(ctx, (neko_gui_container_t*)tab_bar->items[i].data, +1);
                }

                // Swap windows as well
                ((neko_gui_container_t*)(tab_item->data))->tab_item = tab_item->idx;

                // Set focus to first window
                tab_bar->focus = idx ? idx - 1 : idx;
                neko_assert(tab_bar->items[tab_bar->focus].data != cnt);

                // Set split for focus
                if (parent == cnt) {
                    // Set parent for other containers (assuming parent was removed)
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        neko_gui_container_t* c = (neko_gui_container_t*)tab_bar->items[i].data;
                        c->parent = (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                        tab_bar->items[i].idx = i;
                        tab_bar->items[i].zindex = i;
                    }

                    neko_gui_container_t* fcnt = (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;
                    fcnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[0].container == cnt) {
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
                    neko_gui_tab_item_swap(ctx, (neko_gui_container_t*)tab_bar->items[i].data, +1);
                }

                for (u32 i = 0; i < tab_bar->size; ++i) {
                    neko_gui_container_t* fcnt = (neko_gui_container_t*)tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect;
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE;
                }

                // Fix up split reference
                if (parent == cnt) {
                    tab_bar->focus = idx ? idx - 1 : idx;

                    neko_assert((neko_gui_container_t*)tab_bar->items[tab_bar->focus].data != cnt);

                    neko_gui_container_t* fcnt = (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;

                    // Fix up split reference
                    split = neko_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[0].container == parent) {
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

            neko_gui_bring_to_front(ctx, cnt);

        } else {
            // No split, so just do stuff normally...
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            neko_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

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
                    neko_gui_tab_item_swap(ctx, (neko_gui_container_t*)tab_bar->items[i].data, +1);
                }

                // Set size
                tab_bar->size--;

                // Set focus to first window
                tab_bar->focus = idx ? idx - 1 : idx;

                // Set parent for other containers
                if (parent == cnt) {
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        neko_gui_container_t* c = (neko_gui_container_t*)tab_bar->items[i].data;
                        c->parent = (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                    }
                }
            }
            // Only 2 windows left in tab bar
            else {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    neko_gui_container_t* fcnt = (neko_gui_container_t*)tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect;
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE;
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

            neko_gui_bring_to_front(ctx, cnt);
        }
    } else {
        // Rmove split reference from container
        cnt->split = 0x00;

        neko_gui_split_node_t* remain_node = split->children[NEKO_GUI_SPLIT_NODE_CHILD].container == cnt    ? &split->children[NEKO_GUI_SPLIT_NODE_PARENT]
                                             : split->children[NEKO_GUI_SPLIT_NODE_PARENT].container == cnt ? &split->children[NEKO_GUI_SPLIT_NODE_CHILD]
                                                                                                            : NULL;

        neko_assert(remain_node);

        // Set child split in prev container split to split container parent
        if (ps) {
            u32 node_id = ps->children[NEKO_GUI_SPLIT_NODE_CHILD].split == split->id ? NEKO_GUI_SPLIT_NODE_CHILD : NEKO_GUI_SPLIT_NODE_PARENT;
            neko_gui_split_node_t* fix_node = &ps->children[node_id];
            *fix_node = *remain_node;
            switch (fix_node->type) {
                case NEKO_GUI_SPLIT_NODE_CONTAINER: {
                    neko_gui_container_t* remcnt = fix_node->container;
                    remcnt->split = ps->id;
                } break;

                case NEKO_GUI_SPLIT_NODE_SPLIT: {
                    neko_gui_split_t* remsplit = neko_slot_array_getp(ctx->splits, fix_node->split);
                    remsplit->parent = ps->id;
                } break;
            }

            root_split = neko_gui_get_root_split_from_split(ctx, ps);
        }
        // Otherwise, we were root dock and have to treat that case for remaining nodes
        else {
            switch (remain_node->type) {
                case NEKO_GUI_SPLIT_NODE_CONTAINER: {
                    neko_gui_container_t* remcnt = remain_node->container;
                    remcnt->rect = split->rect;
                    remcnt->split = 0x00;
                    root_split = neko_gui_get_root_split(ctx, remcnt);
                } break;

                case NEKO_GUI_SPLIT_NODE_SPLIT: {
                    neko_gui_split_t* remsplit = neko_slot_array_getp(ctx->splits, remain_node->split);
                    remsplit->rect = split->rect;
                    remsplit->parent = 0x00;
                    root_split = neko_gui_get_root_split_from_split(ctx, remsplit);
                } break;
            }
        }

        // Erase split
        neko_slot_array_erase(ctx->splits, split->id);

        // Update
        if (root_split) neko_gui_update_split(ctx, root_split);
        if (root_split) neko_gui_bring_split_to_front(ctx, root_split);
        neko_gui_bring_to_front(ctx, cnt);
    }
}

// ============================= //
// ========= Main API ========== //
// ============================= //

#define NEKO_GUI_COPY_STYLE(DST, SRC)              \
    do {                                           \
        DST[NEKO_GUI_ELEMENT_STATE_DEFAULT] = SRC; \
        DST[NEKO_GUI_ELEMENT_STATE_HOVER] = SRC;   \
        DST[NEKO_GUI_ELEMENT_STATE_FOCUS] = SRC;   \
    } while (0)

static void neko_gui_init_default_styles(neko_gui_context_t* ctx) {
    // Set up main default style
    neko_gui_default_style.font = neko_idraw_default_font();
    neko_gui_default_style.size[0] = 68.f;
    neko_gui_default_style.size[1] = 18.f;
    neko_gui_default_style.spacing = 2;
    neko_gui_default_style.indent = 10;
    neko_gui_default_style.title_height = 20;
    neko_gui_default_style.scrollbar_size = 5;
    neko_gui_default_style.thumb_size = 5;

    // Initialize all default styles
    NEKO_GUI_COPY_STYLE(neko_gui_default_container_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_button_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_text_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_label_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_panel_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_input_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_scroll_style, neko_gui_default_style);
    NEKO_GUI_COPY_STYLE(neko_gui_default_image_style, neko_gui_default_style);

    neko_gui_style_t* style = NULL;

    // button
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_button_style[i];
        style->justify_content = NEKO_GUI_JUSTIFY_CENTER;
    }
    neko_gui_default_button_style[NEKO_GUI_ELEMENT_STATE_DEFAULT].colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(35, 35, 35, 255);
    neko_gui_default_button_style[NEKO_GUI_ELEMENT_STATE_HOVER].colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(40, 40, 40, 255);
    neko_gui_default_button_style[NEKO_GUI_ELEMENT_STATE_FOCUS].colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(0, 214, 121, 255);

    // panel
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_panel_style[i];
        style->colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(30, 30, 30, 255);
        style->size[1] = 19;
    }

    // input
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_input_style[i];
        style->colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(20, 20, 20, 255);
        style->size[1] = 19;
    }

    // text
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_text_style[i];
        style->colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(0, 0, 0, 0);
        style->colors[NEKO_GUI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
    }

    // label
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_label_style[i];
        style->colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(0, 0, 0, 0);
        style->colors[NEKO_GUI_COLOR_CONTENT] = NEKO_COLOR_WHITE;
        style->size[1] = 19;
    }

    // scroll
    for (u32 i = 0; i < 3; ++i) {
        style = &neko_gui_default_scroll_style[i];
        style->size[0] = 10;
        style->padding[NEKO_GUI_PADDING_RIGHT] = 4;
    }

    style = &neko_gui_default_scroll_style[NEKO_GUI_ELEMENT_STATE_DEFAULT];
    style->colors[NEKO_GUI_COLOR_BACKGROUND] = neko_color(22, 22, 22, 255);
    style->colors[NEKO_GUI_COLOR_CONTENT] = neko_color(255, 255, 255, 100);

#define NEKO_GUI_COPY(DST, SRC) \
    do {                        \
        DST[0x00] = SRC[0x00];  \
        DST[0x01] = SRC[0x01];  \
        DST[0x02] = SRC[0x02];  \
    } while (0)

    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_CONTAINER], neko_gui_default_container_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_LABEL], neko_gui_default_label_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_TEXT], neko_gui_default_text_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_PANEL], neko_gui_default_panel_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_INPUT], neko_gui_default_input_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_BUTTON], neko_gui_default_button_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_SCROLL], neko_gui_default_scroll_style);
    NEKO_GUI_COPY(neko_gui_default_style_sheet.styles[NEKO_GUI_ELEMENT_IMAGE], neko_gui_default_image_style);

    ctx->style_sheet = &neko_gui_default_style_sheet;
    ctx->style = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][0x00];
}

static char button_map[256] = neko_default_val();
static char key_map[512] = neko_default_val();

NEKO_API_DECL neko_gui_context_t neko_gui_new(u32 window_hndl) {
    neko_gui_context_t ctx = neko_default_val();
    neko_gui_init(&ctx, window_hndl);
    return ctx;
}

NEKO_API_DECL void neko_gui_init(neko_gui_context_t* ctx, u32 window_hndl) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->gui_idraw = neko_immediate_draw_new();
    ctx->overlay_draw_list = neko_immediate_draw_new();
    neko_gui_init_default_styles(ctx);
    ctx->window_hndl = window_hndl;
    ctx->last_zindex = 1000;
    neko_slot_array_reserve(ctx->splits, NEKO_GUI_NEKO_GUI_SPLIT_SIZE);
    neko_gui_split_t split = neko_default_val();
    neko_slot_array_insert(ctx->splits, split);  // First item is set for 0x00 invalid
    neko_slot_array_reserve(ctx->tab_bars, NEKO_GUI_NEKO_GUI_TAB_SIZE);
    neko_gui_tab_bar_t tb = neko_default_val();
    neko_slot_array_insert(ctx->tab_bars, tb);

    button_map[NEKO_MOUSE_LBUTTON & 0xff] = NEKO_GUI_MOUSE_LEFT;
    button_map[NEKO_MOUSE_RBUTTON & 0xff] = NEKO_GUI_MOUSE_RIGHT;
    button_map[NEKO_MOUSE_MBUTTON & 0xff] = NEKO_GUI_MOUSE_MIDDLE;

    key_map[NEKO_KEYCODE_LEFT_SHIFT & 0xff] = NEKO_GUI_KEY_SHIFT;
    key_map[NEKO_KEYCODE_RIGHT_SHIFT & 0xff] = NEKO_GUI_KEY_SHIFT;
    key_map[NEKO_KEYCODE_LEFT_CONTROL & 0xff] = NEKO_GUI_KEY_CTRL;
    key_map[NEKO_KEYCODE_RIGHT_CONTROL & 0xff] = NEKO_GUI_KEY_CTRL;
    key_map[NEKO_KEYCODE_LEFT_ALT & 0xff] = NEKO_GUI_KEY_ALT;
    key_map[NEKO_KEYCODE_RIGHT_ALT & 0xff] = NEKO_GUI_KEY_ALT;
    key_map[NEKO_KEYCODE_ENTER & 0xff] = NEKO_GUI_KEY_RETURN;
    key_map[NEKO_KEYCODE_BACKSPACE & 0xff] = NEKO_GUI_KEY_BACKSPACE;
}

NEKO_API_DECL void neko_gui_init_font_stash(neko_gui_context_t* ctx, neko_gui_font_stash_desc_t* desc) {
    neko_hash_table_clear(ctx->font_stash);
    u32 ct = sizeof(neko_gui_font_desc_t) / desc->size;
    for (u32 i = 0; i < ct; ++i) {
        neko_hash_table_insert(ctx->font_stash, neko_hash_str64(desc->fonts[i].key), desc->fonts[i].font);
    }
}

NEKO_API_DECL neko_gui_context_t neko_gui_context_new(u32 window_hndl) {
    neko_gui_context_t gui = neko_default_val();
    neko_gui_init(&gui, window_hndl);
    return gui;
}

NEKO_API_DECL void neko_gui_free(neko_gui_context_t* ctx) {
    /*
typedef struct neko_gui_context_t
{
    // Core state
    neko_gui_style_t* style;              // Active style
    neko_gui_style_sheet_t* style_sheet;  // Active style sheet
    neko_gui_id hover;
    neko_gui_id focus;
    neko_gui_id last_id;
    neko_gui_id state_switch_id;          // Id that had a state switch
    s32 switch_state;
    neko_gui_id lock_focus;
    s32 last_hover_state;
    s32 last_focus_state;
    neko_gui_id prev_hover;
    neko_gui_id prev_focus;
    neko_gui_rect_t last_rect;
    s32 last_zindex;
    s32 updated_focus;
    s32 frame;
    neko_vec2 framebuffer_size;
    neko_gui_rect_t viewport;
    neko_gui_container_t* active_root;
    neko_gui_container_t* hover_root;
    neko_gui_container_t* next_hover_root;
    neko_gui_container_t* scroll_target;
    neko_gui_container_t* focus_root;
    neko_gui_container_t* next_focus_root;
    neko_gui_container_t* dockable_root;
    neko_gui_container_t* prev_dockable_root;
    neko_gui_container_t* docked_root;
    neko_gui_container_t* undock_root;
    neko_gui_split_t*     focus_split;
    neko_gui_split_t*     next_hover_split;
    neko_gui_split_t*     hover_split;
    neko_gui_id           next_lock_hover_id;
    neko_gui_id           lock_hover_id;
    char number_edit_buf[NEKO_GUI_MAX_FMT];
    neko_gui_id number_edit;
    neko_gui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(neko_gui_request_t) requests;

    // Stacks
    neko_gui_stack(u8, NEKO_GUI_COMMANDLIST_SIZE) command_list;
    neko_gui_stack(neko_gui_container_t*, NEKO_GUI_ROOTLIST_SIZE) root_list;
    neko_gui_stack(neko_gui_container_t*, NEKO_GUI_CONTAINERSTACK_SIZE) container_stack;
    neko_gui_stack(neko_gui_rect_t, NEKO_GUI_CLIPSTACK_SIZE) clip_stack;
    neko_gui_stack(neko_gui_id, NEKO_GUI_IDSTACK_SIZE) id_stack;
    neko_gui_stack(neko_gui_layout_t, NEKO_GUI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(neko_gui_element_type, neko_gui_inline_style_stack_t) inline_styles;

    // Retained state pools
    neko_gui_pool_item_t container_pool[NEKO_GUI_CONTAINERPOOL_SIZE];
    neko_gui_container_t containers[NEKO_GUI_CONTAINERPOOL_SIZE];
    neko_gui_pool_item_t treenode_pool[NEKO_GUI_TREENODEPOOL_SIZE];

    neko_slot_array(neko_gui_split_t) splits;
    neko_slot_array(neko_gui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    s32 mouse_down;
    s32 mouse_pressed;
    s32 key_down;
    s32 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gsi;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(neko_gui_id, neko_gui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, neko_asset_ascii_font_t*) font_stash;

    // Callbacks
    struct {
        neko_gui_on_draw_button_callback button;
    } callbacks;

} neko_gui_context_t;
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
        neko_gui_inline_style_stack_t* stack = neko_hash_table_iter_getp(ctx->inline_styles, it);
        for (u32 i = 0; i < 3; ++i) {
            neko_dyn_array_free(stack->styles[i]);
            neko_dyn_array_free(stack->animations[i]);
        }
        neko_dyn_array_free(stack->style_counts);      // amount of styles to pop off at "top of stack" for each state
        neko_dyn_array_free(stack->animation_counts);  // amount of animations to pop off at "top of stack" for each state
    }

    neko_dyn_array_free(ctx->requests);
}

static void neko_gui_draw_splits(neko_gui_context_t* ctx, neko_gui_split_t* split) {
    if (!split) return;

    neko_gui_split_t* root_split = neko_gui_get_root_split_from_split(ctx, split);

    // Draw split
    const neko_gui_rect_t* sr = &split->rect;
    neko_vec2 hd = neko_v2(sr->w * 0.5f, sr->h * 0.5f);
    neko_gui_rect_t r = neko_default_val();
    neko_color_t c = neko_color(0, 0, 0, 0);
    const float ratio = split->ratio;
    neko_gui_container_t* top = neko_gui_get_top_most_container(ctx, root_split);
    neko_gui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root : ctx->next_hover_root ? ctx->next_hover_root : NULL;
    bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;
    valid_hover = false;
    bool valid = true;

    split->frame = ctx->frame;
    root_split->frame = ctx->frame;

    bool can_draw = true;
    for (u32 i = 0; i < 2; ++i) {
        if (split->children[i].type == NEKO_GUI_SPLIT_NODE_CONTAINER && can_draw) {
            neko_gui_container_t* cnt = split->children[i].container;

            // Don't draw split if this container belongs to a dockspace
            if (cnt->opt & NEKO_GUI_OPT_DOCKSPACE) {
                can_draw = false;
                continue;
            }

            switch (split->type) {
                case NEKO_GUI_SPLIT_LEFT: {
                    r = neko_gui_rect(sr->x + sr->w * ratio - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->y + NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE, sr->h - 2.f * NEKO_GUI_SPLIT_SIZE);
                } break;

                case NEKO_GUI_SPLIT_RIGHT: {
                    r = neko_gui_rect(sr->x + sr->w * (1.f - ratio) - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->y + NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE, sr->h - 2.f * NEKO_GUI_SPLIT_SIZE);
                } break;

                case NEKO_GUI_SPLIT_TOP: {
                    r = neko_gui_rect(sr->x + NEKO_GUI_SPLIT_SIZE, sr->y + sr->h * (ratio)-NEKO_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE);
                } break;

                case NEKO_GUI_SPLIT_BOTTOM: {
                    r = neko_gui_rect(sr->x + NEKO_GUI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE);
                } break;
            }

            s16 exp[4] = {1, 1, 1, 1};
            neko_gui_rect_t expand = neko_gui_expand_rect(r, exp);
            bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && neko_gui_rect_overlaps_vec2(expand, ctx->mouse_pos) && !ctx->lock_hover_id;
            if (hover) ctx->next_hover_split = split;
            if (hover && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
                if (!ctx->focus_split) ctx->focus_split = split;
            }
            bool active = ctx->focus_split == split;
            if (active && valid) {
                ctx->next_hover_root = top;
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_SPLIT_RATIO;
                req.split = split;
                neko_dyn_array_push(ctx->requests, req);
            }

            if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                neko_color_t bc = ctx->focus_split == split ? ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_FOCUS].colors[NEKO_GUI_COLOR_BACKGROUND]
                                                            : ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_HOVER].colors[NEKO_GUI_COLOR_BACKGROUND];
                neko_gui_draw_rect(ctx, r, bc);
                can_draw = false;
            }
        } else if (split->children[i].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
            if (can_draw) {
                switch (split->type) {
                    case NEKO_GUI_SPLIT_LEFT: {
                        r = neko_gui_rect(sr->x + sr->w * ratio - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->y + NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE, sr->h - 2.f * NEKO_GUI_SPLIT_SIZE);
                    } break;

                    case NEKO_GUI_SPLIT_RIGHT: {
                        r = neko_gui_rect(sr->x + sr->w * (1.f - ratio) - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->y + NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE, sr->h - 2.f * NEKO_GUI_SPLIT_SIZE);
                    } break;

                    case NEKO_GUI_SPLIT_TOP: {
                        r = neko_gui_rect(sr->x + NEKO_GUI_SPLIT_SIZE, sr->y + sr->h * (ratio)-NEKO_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE);
                    } break;

                    case NEKO_GUI_SPLIT_BOTTOM: {
                        r = neko_gui_rect(sr->x + NEKO_GUI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - NEKO_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * NEKO_GUI_SPLIT_SIZE, NEKO_GUI_SPLIT_SIZE);
                    } break;
                }

                s16 exp[] = {1, 1, 1, 1};
                neko_gui_rect_t expand = neko_gui_expand_rect(r, exp);
                bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && neko_gui_rect_overlaps_vec2(expand, ctx->mouse_pos);
                if (hover) ctx->next_hover_split = split;
                if (hover && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
                    if (!ctx->focus_split) ctx->focus_split = split;
                }
                bool active = ctx->focus_split == split;
                if (active) {
                    ctx->next_hover_root = top;
                    neko_gui_request_t req = neko_default_val();
                    req.type = NEKO_GUI_SPLIT_RATIO;
                    req.split = split;
                    neko_dyn_array_push(ctx->requests, req);
                }

                if ((hover && (ctx->focus_split == split)) || (hover && !ctx->focus_split) || active) {
                    neko_color_t bc = ctx->focus_split == split ? ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_FOCUS].colors[NEKO_GUI_COLOR_BACKGROUND]
                                                                : ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_HOVER].colors[NEKO_GUI_COLOR_BACKGROUND];
                    neko_gui_draw_rect(ctx, r, bc);
                    can_draw = false;
                }
            }

            neko_gui_split_t* child = neko_slot_array_getp(ctx->splits, split->children[i].split);
            neko_gui_draw_splits(ctx, child);
        }
    }
    if (ctx->focus_split == split && ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
        ctx->focus_split = NULL;
    }
}

static void neko_gui_get_split_lowest_zindex(neko_gui_context_t* ctx, neko_gui_split_t* split, s32* index) {
    if (!split) return;

    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[0].container->zindex < *index) {
        *index = split->children[0].container->zindex;
    }
    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[0].container->tab_bar) {
        neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[0].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((neko_gui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((neko_gui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[1].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[1].container->zindex < *index) {
        *index = split->children[1].container->zindex;
    }
    if (split->children[1].type == NEKO_GUI_SPLIT_NODE_CONTAINER && split->children[1].container->tab_bar) {
        neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, split->children[1].container->tab_bar);
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((neko_gui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((neko_gui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[0].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
        neko_gui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[0].split), index);
    }

    if (split->children[1].type == NEKO_GUI_SPLIT_NODE_SPLIT) {
        neko_gui_get_split_lowest_zindex(ctx, neko_slot_array_getp(ctx->splits, split->children[1].split), index);
    }
}

NEKO_API_DECL void neko_gui_begin(neko_gui_context_t* ctx, const neko_gui_hints_t* hints) {
    neko_gui_hints_t default_hints = neko_default_val();
    default_hints.framebuffer_size = neko_platform_framebuffer_sizev(ctx->window_hndl);
    default_hints.viewport = neko_gui_rect(0.f, 0.f, default_hints.framebuffer_size.x, default_hints.framebuffer_size.y);
    neko_gui_hints_t hint = hints ? *hints : default_hints;

    // Set up viewport
    neko_gui_rect_t vp = hint.viewport;
    if (~hint.flags & NEKO_GUI_HINT_FLAG_NO_INVERT_Y) {
        vp.y = hint.viewport.y;
    } else {
        vp.y = hint.framebuffer_size.y - hint.viewport.h - hint.viewport.y;
    }

    // Capture event information
    neko_vec2 mouse_pos = neko_platform_mouse_positionv();

    // Check for scale and bias
    if (hint.flags & NEKO_GUI_HINT_FLAG_NO_SCALE_BIAS_MOUSE) {
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

    neko_platform_event_t evt = neko_default_val();
    while (neko_platform_poll_events(&evt, false)) {
        switch (evt.type) {
            case NEKO_PLATFORM_EVENT_MOUSE: {
                switch (evt.mouse.action) {
                    case NEKO_PLATFORM_MOUSE_MOVE: {
                        // ctx->mouse_pos = evt.mouse.move;
                    } break;

                    case NEKO_PLATFORM_MOUSE_WHEEL: {
                        neko_gui_input_scroll(ctx, (s32)(-evt.mouse.wheel.x * 30.f), (s32)(-evt.mouse.wheel.y * 30.f));
                    } break;

                    case NEKO_PLATFORM_MOUSE_BUTTON_DOWN:
                    case NEKO_PLATFORM_MOUSE_BUTTON_PRESSED: {
                        s32 code = 1 << evt.mouse.button;
                        neko_gui_input_mousedown(ctx, (s32)mouse_pos.x, (s32)mouse_pos.y, code);
                    } break;

                    case NEKO_PLATFORM_MOUSE_BUTTON_RELEASED: {
                        s32 code = 1 << evt.mouse.button;
                        neko_gui_input_mouseup(ctx, (s32)mouse_pos.x, (s32)mouse_pos.y, code);
                    } break;

                    case NEKO_PLATFORM_MOUSE_ENTER: {
                        // If there are user callbacks, could trigger them here
                    } break;

                    case NEKO_PLATFORM_MOUSE_LEAVE: {
                        // If there are user callbacks, could trigger them here
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PLATFORM_EVENT_TEXT: {
                // Input text
                char txt[2] = {(char)(evt.text.codepoint & 255), 0};
                neko_gui_input_text(ctx, txt);
            } break;

            case NEKO_PLATFORM_EVENT_KEY: {
                switch (evt.key.action) {
                    case NEKO_PLATFORM_KEY_DOWN:
                    case NEKO_PLATFORM_KEY_PRESSED: {
                        neko_gui_input_keydown(ctx, key_map[evt.key.keycode & 511]);
                    } break;

                    case NEKO_PLATFORM_KEY_RELEASED: {
                        neko_gui_input_keyup(ctx, key_map[evt.key.keycode & 511]);
                    } break;

                    default:
                        break;
                }

            } break;

            case NEKO_PLATFORM_EVENT_WINDOW: {
                switch (evt.window.action) {
                    default:
                        break;
                }

            } break;

            default:
                break;
        }
    }

    ctx->framebuffer_size = hint.framebuffer_size;
    ctx->viewport = neko_gui_rect(hint.viewport.x, hint.viewport.y, hint.viewport.w, hint.viewport.h);
    ctx->mouse_pos = mouse_pos;
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
    neko_idraw_camera2D(&ctx->overlay_draw_list, (u32)ctx->viewport.w, (u32)ctx->viewport.h);  // Need to pass in a viewport for this instead

    /*
    for (
        neko_slot_array_iter it = neko_slot_array_iter_new(ctx->splits);
        neko_slot_array_iter_valid(ctx->splits, it);
        neko_slot_array_iter_advance(ctx->splits, it)
    )
    {
        if (!it) continue;

        neko_gui_split_t* split = neko_slot_array_iter_getp(ctx->splits, it);

        // Root split
        if (!split->parent)
        {
            neko_gui_container_t* root_cnt = neko_gui_get_root_container_from_split(ctx, split);
            neko_gui_rect_t r = split->rect;
            r.x -= 10.f;
            r.w += 20.f;
            r.y -= 10.f;
            r.h += 20.f;
            neko_snprintfc(TMP, 256, "!dockspace%zu", (size_t)split);
            s32 opt = NEKO_GUI_OPT_NOFRAME |
                          NEKO_GUI_OPT_FORCESETRECT |
                          NEKO_GUI_OPT_NOMOVE |
                          NEKO_GUI_OPT_NOTITLE |
                          NEKO_GUI_OPT_NOSCROLL |
                          NEKO_GUI_OPT_NOCLIP |
                          NEKO_GUI_OPT_NODOCK |
                          NEKO_GUI_OPT_DOCKSPACE |
                          NEKO_GUI_OPT_NOBORDER;
            neko_gui_window_begin_ex(ctx, TMP, r, NULL, opt);
            {
                // Set zindex for sorting (always below the bottom most window in this split tree)
                neko_gui_container_t* ds = neko_gui_get_current_container(ctx);
                s32 zindex = INT32_MAX - 1;
                neko_gui_get_split_lowest_zindex(ctx, split, &zindex);
                if (root_cnt->opt & NEKO_GUI_OPT_DOCKSPACE) ds->zindex = 0;
                else ds->zindex = neko_clamp((s32)zindex - 1, 0, INT32_MAX);

                neko_gui_rect_t fr = split->rect;
                fr.x += NEKO_GUI_SPLIT_SIZE; fr.y += NEKO_GUI_SPLIT_SIZE; fr.w -= 2.f * NEKO_GUI_SPLIT_SIZE; fr.h -= 2.f * NEKO_GUI_SPLIT_SIZE;
                // neko_gui_draw_frame(ctx, fr, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][0x00]);

                // Draw splits
                neko_gui_draw_splits(ctx, split);

                // Do resize controls for dockspace
                neko_gui_container_t* top = neko_gui_get_top_most_container(ctx, split);
                const neko_gui_rect_t* sr = &split->rect;
                neko_gui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root :
                        ctx->next_hover_root ? ctx->next_hover_root :
                        NULL;
                bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;

                // W
                {
                    // Cache rect
                    neko_gui_rect_t lr = neko_gui_rect(fr.x - 2.f * NEKO_GUI_SPLIT_SIZE, fr.y, NEKO_GUI_SPLIT_SIZE, fr.h);
                    neko_gui_rect_t ex = lr;
                    ex.x -= 10.f; ex.w += 20.f;
                    neko_gui_id id = neko_gui_get_id(ctx, "!hov_l", 6);
                    neko_gui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT)
                    {
                        neko_gui_draw_control_frame(ctx, id, lr, NEKO_GUI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        neko_gui_request_t req = neko_default_val();
                        req.type = NEKO_GUI_SPLIT_RESIZE_W;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // E
                {
                    // Cache rect
                    neko_gui_rect_t rr = neko_gui_rect(fr.x + fr.w + NEKO_GUI_SPLIT_SIZE, fr.y, NEKO_GUI_SPLIT_SIZE, fr.h);
                    neko_gui_rect_t ex = rr;
                    ex.w += 20.f;
                    neko_gui_id id = neko_gui_get_id(ctx, "!hov_r", 6);
                    neko_gui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT)
                    {
                        neko_gui_draw_control_frame(ctx, id, rr, NEKO_GUI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        neko_gui_request_t req = neko_default_val();
                        req.type = NEKO_GUI_SPLIT_RESIZE_E;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // N
                {
                    // Cache rect
                    neko_gui_rect_t tr = neko_gui_rect(fr.x, fr.y - 2.f * NEKO_GUI_SPLIT_SIZE, fr.w, NEKO_GUI_SPLIT_SIZE);
                    neko_gui_rect_t ex = tr;
                    ex.y -= 10.f;
                    ex.h += 20.f;
                    neko_gui_id id = neko_gui_get_id(ctx, "!hov_t", 6);
                    neko_gui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT)
                    {
                        neko_gui_draw_control_frame(ctx, id, tr, NEKO_GUI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        neko_gui_request_t req = neko_default_val();
                        req.type = NEKO_GUI_SPLIT_RESIZE_N;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }

                // S
                {
                    // Cache rect
                    neko_gui_rect_t br = neko_gui_rect(fr.x, fr.y + fr.h + NEKO_GUI_SPLIT_SIZE, fr.w, NEKO_GUI_SPLIT_SIZE);
                    neko_gui_rect_t ex = br;
                    ex.h += 20.f;
                    neko_gui_id id = neko_gui_get_id(ctx, "!hov_b", 6);
                    neko_gui_update_control(ctx, id, ex, opt);

                    if (id == ctx->focus && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT)
                    {
                        neko_gui_draw_control_frame(ctx, id, br, NEKO_GUI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        neko_gui_request_t req = neko_default_val();
                        req.type = NEKO_GUI_SPLIT_RESIZE_S;
                        req.split = split;
                        neko_dyn_array_push(ctx->requests, req);
                    }
                }
            }
            neko_gui_window_end(ctx);
        }
    }
    */

    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
        ctx->lock_focus = 0x00;
    }
}

static void neko_gui_docking(neko_gui_context_t* ctx) {
    if (ctx->undock_root) {
        neko_gui_undock_ex_cnt(ctx, ctx->undock_root);
    }

    if (!ctx->focus_root || ctx->focus_root->opt & NEKO_GUI_OPT_NODOCK) return;

    if (ctx->dockable_root || ctx->prev_dockable_root) {
        neko_gui_container_t* cnt = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;

        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
            s32 b = 0;
        }

        // Cache hoverable tile information
        neko_vec2 c = neko_v2(cnt->rect.x + cnt->rect.w / 2.f, cnt->rect.y + cnt->rect.h / 2.f);

        const float sz = neko_clamp(neko_min(cnt->rect.w * 0.1f, cnt->rect.h * 0.1f), 15.f, 25.f);
        const float off = sz + sz * 0.2f;
        neko_color_t def_col = neko_color_alpha(ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_FOCUS].colors[NEKO_GUI_COLOR_BACKGROUND], 100);
        neko_color_t hov_col = neko_color_alpha(ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_FOCUS].colors[NEKO_GUI_COLOR_BACKGROUND], 150);

        neko_gui_rect_t center = neko_gui_rect(c.x, c.y, sz, sz);
        neko_gui_rect_t left = neko_gui_rect(c.x - off, c.y, sz, sz);
        neko_gui_rect_t right = neko_gui_rect(c.x + off, c.y, sz, sz);
        neko_gui_rect_t top = neko_gui_rect(c.x, c.y - off, sz, sz);
        neko_gui_rect_t bottom = neko_gui_rect(c.x, c.y + off, sz, sz);

        s32 hov_c = (neko_gui_rect_overlaps_vec2(center, ctx->mouse_pos));
        s32 hov_l = neko_gui_rect_overlaps_vec2(left, ctx->mouse_pos);
        s32 hov_r = neko_gui_rect_overlaps_vec2(right, ctx->mouse_pos);
        s32 hov_t = neko_gui_rect_overlaps_vec2(top, ctx->mouse_pos);
        s32 hov_b = neko_gui_rect_overlaps_vec2(bottom, ctx->mouse_pos);
        s32 hov_w = neko_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos);

        bool can_dock = true;

        // Can't dock one dockspace into another
        if (ctx->focus_root->opt & NEKO_GUI_OPT_DOCKSPACE) {
            can_dock = false;
        }

        if (ctx->focus_root->tab_bar) {
            neko_gui_container_t* tcmp = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, ctx->focus_root->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                neko_gui_container_t* tcnt = (neko_gui_container_t*)tab_bar->items[i].data;
                if (tcnt == tcmp) {
                    can_dock = false;
                }
            }
        }

        // Need to make sure you CAN dock here first
        if (ctx->dockable_root && can_dock) {
            // Need to now grab overlay draw list, then draw rects into it
            neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

            bool is_dockspace = ctx->dockable_root->opt & NEKO_GUI_OPT_DOCKSPACE;

            // Draw center rect
            neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_c ? hov_col : def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
            // neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w + 1, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);

            if (!is_dockspace) {
                neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_l ? hov_col : def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_r ? hov_col : def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_t ? hov_col : def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);

                neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_b ? hov_col : def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                // neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
            }

            const float d = 0.5f;
            const float hs = sz * 0.5f;

            if (is_dockspace) {
                if (hov_c) {
                    center = neko_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
            } else {
                if (hov_c && !ctx->focus_root->split) {
                    center = neko_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(center.x, center.y), neko_v2(center.w, center.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                } else if (hov_l) {
                    left = neko_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w * d + hs, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(left.x, left.y), neko_v2(left.w, left.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                } else if (hov_r) {
                    right = neko_gui_rect(cnt->rect.x + cnt->rect.w * d + hs, cnt->rect.y, cnt->rect.w * (1.f - d) - hs, cnt->rect.h);
                    neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(right.x, right.y), neko_v2(right.w, right.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                } else if (hov_b) {
                    bottom = neko_gui_rect(cnt->rect.x, cnt->rect.y + cnt->rect.h * d + hs, cnt->rect.w, cnt->rect.h * (1.f - d) - hs);
                    neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(bottom.x, bottom.y), neko_v2(bottom.w, bottom.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                } else if (hov_t) {
                    top = neko_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h * d + hs);
                    neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), hov_col, NEKO_GRAPHICS_PRIMITIVE_LINES);
                    neko_idraw_rectvd(dl, neko_v2(top.x, top.y), neko_v2(top.w, top.h), neko_v2s(0.f), neko_v2s(1.f), def_col, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
            }
        }

        // Handle docking
        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
            neko_gui_container_t* parent = ctx->prev_dockable_root;
            neko_gui_container_t* child = ctx->focus_root;

            bool is_dockspace = ctx->prev_dockable_root->opt & NEKO_GUI_OPT_DOCKSPACE;

            if (is_dockspace) {
                if (hov_c) neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_TOP, 1.0f);
            } else {
                if (hov_c && !ctx->focus_root->split)
                    neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_TAB, 0.5f);
                else if (hov_l)
                    neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_LEFT, 0.5f);
                else if (hov_r)
                    neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_RIGHT, 0.5f);
                else if (hov_t)
                    neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_TOP, 0.5f);
                else if (hov_b)
                    neko_gui_dock_ex_cnt(ctx, child, parent, NEKO_GUI_SPLIT_BOTTOM, 0.5f);
            }
        }
    }
}

NEKO_API_DECL void neko_gui_end(neko_gui_context_t* ctx, b32 update) {
    s32 i, n;

    // Check for docking, draw overlays
    neko_gui_docking(ctx);

    for (u32 r = 0; r < neko_dyn_array_size(ctx->requests); ++r) {
        const neko_gui_request_t* req = &ctx->requests[r];

        // If split moved, update position for next frame
        switch (req->type) {
            case NEKO_GUI_CNT_MOVE: {
                if (!update) break;
                if (req->cnt) {
                    req->cnt->rect.x += ctx->mouse_delta.x;
                    req->cnt->rect.y += ctx->mouse_delta.y;

                    if (req->cnt->tab_bar) {
                        neko_gui_tab_bar_t* tb = neko_slot_array_getp(ctx->tab_bars, req->cnt->tab_bar);
                        neko_assert(tb);
                        tb->rect.x += ctx->mouse_delta.x;
                        tb->rect.y += ctx->mouse_delta.y;
                    }
                }
            } break;

            case NEKO_GUI_CNT_FOCUS: {
                if (!update) break;
                if (!req->cnt) break;

                neko_gui_container_t* cnt = (neko_gui_container_t*)req->cnt;
                neko_assert(cnt);

                neko_gui_split_t* rs = neko_gui_get_root_split(ctx, cnt);

                if (cnt->tab_bar) {
                    if (rs) {
                        neko_gui_bring_split_to_front(ctx, rs);
                    }

                    neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
                    neko_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];
                    neko_gui_container_t* fcnt = (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->opt |= NEKO_GUI_OPT_NOHOVER;
                    fcnt->opt |= NEKO_GUI_OPT_NOINTERACT;
                    fcnt->flags &= ~NEKO_GUI_WINDOW_FLAGS_VISIBLE;
                    tab_bar->focus = tab_item->idx;
                    cnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE;

                    // Bring all tab items to front
                    for (u32 i = 0; i < tab_bar->size; ++i) {
                        neko_gui_bring_to_front(ctx, (neko_gui_container_t*)tab_bar->items[i].data);
                    }
                    neko_gui_bring_to_front(ctx, cnt);
                }

            } break;

            case NEKO_GUI_SPLIT_MOVE: {
                if (req->split) {
                    req->split->rect.x += ctx->mouse_delta.x;
                    req->split->rect.y += ctx->mouse_delta.y;
                    neko_gui_update_split(ctx, req->split);
                }

            } break;

            case NEKO_GUI_SPLIT_RESIZE_SE: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    r->w = neko_max(r->w + ctx->mouse_delta.x, 40);
                    r->h = neko_max(r->h + ctx->mouse_delta.y, 40);
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_W: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_E: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    r->w = neko_max(r->w + ctx->mouse_delta.x, 40);
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_N: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                    }
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_NE: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    r->w = neko_max(r->w + ctx->mouse_delta.x, 40);
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                    }
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_NW: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                    if (fabsf(r->h - h) > 0.f) {
                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                    }

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_S: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    r->h = neko_max(r->h + ctx->mouse_delta.y, 40);
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RESIZE_SW: {
                if (req->split) {
                    neko_gui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = neko_max(r->h + ctx->mouse_delta.y, 40);

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                    if (fabsf(r->w - w) > 0.f) {
                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    neko_gui_update_split(ctx, req->split);
                }
            } break;

            case NEKO_GUI_SPLIT_RATIO: {
                const float smin = 0.05f;
                const float smax = 1.f - smin;
                neko_gui_split_t* split = req->split;

                switch (split->type) {
                    case NEKO_GUI_SPLIT_LEFT: {
                        split->ratio = neko_clamp(split->ratio + ctx->mouse_delta.x / split->rect.w, smin, smax);
                        neko_gui_update_split(ctx, split);
                    } break;

                    case NEKO_GUI_SPLIT_RIGHT: {
                        split->ratio = neko_clamp(split->ratio - ctx->mouse_delta.x / split->rect.w, smin, smax);
                        neko_gui_update_split(ctx, split);
                    } break;

                    case NEKO_GUI_SPLIT_TOP: {
                        split->ratio = neko_clamp(split->ratio + ctx->mouse_delta.y / split->rect.h, smin, smax);
                        neko_gui_update_split(ctx, split);
                    } break;

                    case NEKO_GUI_SPLIT_BOTTOM: {
                        split->ratio = neko_clamp(split->ratio - ctx->mouse_delta.y / split->rect.h, smin, smax);
                        neko_gui_update_split(ctx, split);
                    } break;
                }

                // Bring to font
                neko_gui_bring_split_to_front(ctx, neko_gui_get_root_split_from_split(ctx, split));

            } break;

            case NEKO_GUI_TAB_SWAP_LEFT: {
                neko_gui_tab_item_swap(ctx, req->cnt, -1);
            } break;

            case NEKO_GUI_TAB_SWAP_RIGHT: {
                neko_gui_tab_item_swap(ctx, req->cnt, +1);
            } break;
        }
    }

    // Clear reqests
    neko_dyn_array_clear(ctx->requests);

    // Check stacks
    neko_expect(ctx->container_stack.idx == 0);
    neko_expect(ctx->clip_stack.idx == 0);
    neko_expect(ctx->id_stack.idx == 0);
    neko_expect(ctx->layout_stack.idx == 0);

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
        neko_gui_split_t* split = neko_gui_get_root_split(ctx, ctx->next_hover_root);

        // Need to bring entire dockspace to front
        if (split) {
            neko_gui_bring_split_to_front(ctx, split);
        } else if (~ctx->next_hover_root->opt & NEKO_GUI_OPT_NOFOCUS) {
            neko_gui_bring_to_front(ctx, ctx->next_hover_root);
        }
    }

    if (ctx->mouse_pressed && (!ctx->next_hover_root || ctx->next_hover_root->opt & NEKO_GUI_OPT_NOFOCUS)) {
        ctx->active_root = NULL;
    }

    // Reset state
    ctx->key_pressed = 0;
    ctx->input_text[0] = '\0';
    ctx->mouse_pressed = 0;
    ctx->scroll_delta = neko_v2(0, 0);
    ctx->last_mouse_pos = ctx->mouse_pos;
    ctx->undock_root = NULL;

    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
        neko_platform_set_cursor(ctx->window_hndl, NEKO_PLATFORM_CURSOR_ARROW);
    }

    // Sort root containers by zindex
    n = ctx->root_list.idx;
    qsort(ctx->root_list.items, n, sizeof(neko_gui_container_t*), neko_gui_compare_zindex);

    // Set root container jump commands
    for (i = 0; i < n; i++) {
        neko_gui_container_t* cnt = ctx->root_list.items[i];

        // If this is the first container then make the first command jump to it.
        // otherwise set the previous container's tail to jump to this one
        if (i == 0) {
            neko_gui_command_t* cmd = (neko_gui_command_t*)ctx->command_list.items;
            cmd->jump.dst = (char*)cnt->head + sizeof(neko_gui_jumpcommand_t);
        } else {
            neko_gui_container_t* prev = ctx->root_list.items[i - 1];
            prev->tail->jump.dst = (char*)cnt->head + sizeof(neko_gui_jumpcommand_t);
        }

        // Make the last container's tail jump to the end of command list
        if (i == n - 1) {
            cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
        }
    }
}

NEKO_API_DECL void neko_gui_render(neko_gui_context_t* ctx, neko_command_buffer_t* cb) {
    const neko_vec2 fb = ctx->framebuffer_size;
    const neko_gui_rect_t* viewport = &ctx->viewport;
    neko_immediate_draw_t* gui_idraw = &ctx->gui_idraw;

    neko_idraw_defaults(&ctx->gui_idraw);
    // neko_idraw_camera2D(&ctx->gsi, (u32)fb.x, (u32)fb.y);
    neko_idraw_camera2D(&ctx->gui_idraw, (u32)viewport->w, (u32)viewport->h);
    neko_idraw_blend_enabled(&ctx->gui_idraw, true);

    neko_gui_rect_t clip = neko_gui_unclipped_rect;

    neko_gui_command_t* cmd = NULL;
    while (neko_gui_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case NEKO_GUI_COMMAND_CUSTOM: {
                neko_idraw_defaults(&ctx->gui_idraw);
                neko_idraw_set_view_scissor(&ctx->gui_idraw, (s32)(cmd->custom.clip.x), (s32)(fb.y - cmd->custom.clip.h - cmd->custom.clip.y), (s32)(cmd->custom.clip.w), (s32)(cmd->custom.clip.h));

                if (cmd->custom.cb) {
                    cmd->custom.cb(ctx, &cmd->custom);
                }

                neko_idraw_defaults(&ctx->gui_idraw);
                // neko_idraw_camera2D(&ctx->gsi, (u32)fb.x, (u32)fb.y);
                neko_idraw_camera2D(&ctx->gui_idraw, (u32)viewport->w, (u32)viewport->h);
                neko_idraw_blend_enabled(&ctx->gui_idraw, true);
                // neko_graphics_set_viewport(&ctx->gsi.commands, 0, 0, (u32)fb.x, (u32)fb.y);
                neko_graphics_set_viewport(&ctx->gui_idraw.commands, (u32)viewport->x, (u32)viewport->y, (u32)viewport->w, (u32)viewport->h);

                neko_idraw_set_view_scissor(&ctx->gui_idraw, (s32)(clip.x), (s32)(fb.y - clip.h - clip.y), (s32)(clip.w), (s32)(clip.h));

            } break;

            case NEKO_GUI_COMMAND_PIPELINE: {
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

            case NEKO_GUI_COMMAND_UNIFORMS: {
                neko_graphics_bind_desc_t bind = neko_default_val();

                // Set uniform bind
                neko_graphics_bind_uniform_desc_t uniforms[1] = neko_default_val();
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms);

                // Treat as byte buffer, read data
                neko_byte_buffer_t buffer = neko_default_val();
                buffer.capacity = NEKO_GUI_COMMANDLIST_SIZE;
                buffer.data = (u8*)cmd->uniforms.data;

                // Write count
                neko_byte_buffer_readc(&buffer, u16, ct);

                // Iterate through all uniforms, memcpy data as needed for each uniform in list
                for (u32 i = 0; i < ct; ++i) {
                    neko_byte_buffer_readc(&buffer, neko_handle(neko_graphics_uniform_t), hndl);
                    neko_byte_buffer_readc(&buffer, size_t, sz);
                    neko_byte_buffer_readc(&buffer, u16, binding);
                    void* udata = (buffer.data + buffer.position);
                    neko_byte_buffer_advance_position(&buffer, sz);

                    uniforms[0].uniform = hndl;
                    uniforms[0].binding = binding;
                    uniforms[0].data = udata;
                    neko_graphics_apply_bindings(&ctx->gui_idraw.commands, &bind);
                }
            } break;

            case NEKO_GUI_COMMAND_TEXT: {
                const neko_vec2* tp = &cmd->text.pos;
                const char* ts = cmd->text.str;
                const neko_color_t* tc = &cmd->text.color;
                const neko_asset_ascii_font_t* tf = cmd->text.font;
                neko_idraw_text(&ctx->gui_idraw, tp->x, tp->y, ts, tf, false, tc->r, tc->g, tc->b, tc->a);
            } break;

            case NEKO_GUI_COMMAND_SHAPE: {
                neko_idraw_texture(&ctx->gui_idraw, neko_handle_invalid(neko_graphics_texture_t));
                neko_color_t* c = &cmd->shape.color;

                switch (cmd->shape.type) {
                    case NEKO_GUI_SHAPE_RECT: {
                        neko_gui_rect_t* r = &cmd->shape.rect;
                        neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2s(0.f), neko_v2s(1.f), *c, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                    } break;

                    case NEKO_GUI_SHAPE_CIRCLE: {
                        neko_vec2* cp = &cmd->shape.circle.center;
                        float* r = &cmd->shape.circle.radius;
                        neko_idraw_circle(&ctx->gui_idraw, cp->x, cp->y, *r, 16, c->r, c->g, c->b, c->a, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
                    } break;

                    case NEKO_GUI_SHAPE_TRIANGLE: {
                        neko_vec2* pa = &cmd->shape.triangle.points[0];
                        neko_vec2* pb = &cmd->shape.triangle.points[1];
                        neko_vec2* pc = &cmd->shape.triangle.points[2];
                        neko_idraw_trianglev(&ctx->gui_idraw, *pa, *pb, *pc, *c, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

                    } break;

                    case NEKO_GUI_SHAPE_LINE: {
                        neko_vec2* s = &cmd->shape.line.start;
                        neko_vec2* e = &cmd->shape.line.end;
                        neko_idraw_linev(&ctx->gui_idraw, *s, *e, *c);
                    } break;
                }

            } break;

            case NEKO_GUI_COMMAND_IMAGE: {
                neko_idraw_texture(&ctx->gui_idraw, cmd->image.hndl);
                neko_color_t* c = &cmd->image.color;
                neko_gui_rect_t* r = &cmd->image.rect;
                neko_vec4* uvs = &cmd->image.uvs;
                neko_idraw_rectvd(&ctx->gui_idraw, neko_v2(r->x, r->y), neko_v2(r->w, r->h), neko_v2(uvs->x, uvs->y), neko_v2(uvs->z, uvs->w), *c, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
            } break;

            case NEKO_GUI_COMMAND_CLIP: {
                // Will project scissor/clipping rectangles into framebuffer space
                neko_vec2 clip_off = neko_v2s(0.f);    // (0,0) unless using multi-viewports
                neko_vec2 clip_scale = neko_v2s(1.f);  // (1,1) unless using retina display which are often (2,2)

                neko_gui_rect_t clip_rect;
                clip_rect.x = (cmd->clip.rect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (cmd->clip.rect.y - clip_off.y) * clip_scale.y;
                clip_rect.w = (cmd->clip.rect.w - clip_off.x) * clip_scale.x;
                clip_rect.h = (cmd->clip.rect.h - clip_off.y) * clip_scale.y;

                clip_rect.x = neko_max(clip_rect.x, 0.f);
                clip_rect.y = neko_max(clip_rect.y, 0.f);
                clip_rect.w = neko_max(clip_rect.w, 0.f);
                clip_rect.h = neko_max(clip_rect.h, 0.f);

                clip = clip_rect;

                neko_idraw_set_view_scissor(&ctx->gui_idraw, (s32)(clip_rect.x), (s32)(fb.y - clip_rect.h - clip_rect.y), (s32)(clip_rect.w), (s32)(clip_rect.h));

            } break;
        }
    }

    // Draw main list
    neko_idraw_draw(&ctx->gui_idraw, cb);

    // Draw overlay list
    neko_idraw_draw(&ctx->overlay_draw_list, cb);
}

NEKO_API_DECL void neko_gui_renderpass_submit(neko_gui_context_t* ctx, neko_command_buffer_t* cb, neko_color_t c) {
    neko_vec2 fbs = ctx->framebuffer_size;
    neko_gui_rect_t* vp = &ctx->viewport;
    neko_graphics_clear_action_t action = neko_default_val();
    action.color[0] = (float)c.r / 255.f;
    action.color[1] = (float)c.g / 255.f;
    action.color[2] = (float)c.b / 255.f;
    action.color[3] = (float)c.a / 255.f;
    neko_graphics_clear_desc_t clear = neko_default_val();
    clear.actions = &action;
    neko_graphics_renderpass_begin(cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        neko_graphics_clear(cb, &clear);
        neko_graphics_set_viewport(cb, (u32)vp->x, (u32)vp->y, (u32)vp->w, (u32)vp->h);
        neko_gui_render(ctx, cb);
    }
    neko_graphics_renderpass_end(cb);
}

NEKO_API_DECL void neko_gui_renderpass_submit_ex(neko_gui_context_t* ctx, neko_command_buffer_t* cb, neko_graphics_clear_action_t* action) {
    neko_vec2 fbs = ctx->framebuffer_size;
    neko_gui_rect_t* vp = &ctx->viewport;
    neko_graphics_clear_desc_t clear = neko_default_val();
    clear.actions = action;
    neko_renderpass_t pass = neko_default_val();
    neko_graphics_renderpass_begin(cb, pass);
    neko_graphics_set_viewport(cb, (u32)vp->x, (u32)vp->y, (u32)vp->w, (u32)vp->h);
    neko_graphics_clear(cb, &clear);
    neko_gui_render(ctx, cb);
    neko_graphics_renderpass_end(cb);
}

NEKO_API_DECL void neko_gui_set_hover(neko_gui_context_t* ctx, neko_gui_id id) {
    ctx->prev_hover = ctx->hover;
    ctx->hover = id;
}

NEKO_API_DECL void neko_gui_set_focus(neko_gui_context_t* ctx, neko_gui_id id) {
    ctx->prev_focus = ctx->focus;
    ctx->focus = id;
    ctx->updated_focus = 1;
}

NEKO_API_DECL neko_gui_id neko_gui_get_id(neko_gui_context_t* ctx, const void* data, s32 size) {
    s32 idx = ctx->id_stack.idx;
    neko_gui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : NEKO_GUI_HASH_INITIAL;
    neko_gui_hash(&res, data, size);
    ctx->last_id = res;

    return res;
}

NEKO_API_DECL neko_gui_id neko_gui_get_id_hash(neko_gui_context_t* ctx, const void* data, s32 size, neko_gui_id hash) {
    neko_gui_id res = hash;
    neko_gui_hash(&res, data, size);
    ctx->last_id = res;
    return res;
}

NEKO_API_DECL void neko_gui_push_id(neko_gui_context_t* ctx, const void* data, s32 size) { neko_gui_stack_push(ctx->id_stack, neko_gui_get_id(ctx, data, size)); }

NEKO_API_DECL void neko_gui_pop_id(neko_gui_context_t* ctx) { neko_gui_stack_pop(ctx->id_stack); }

NEKO_API_DECL void neko_gui_push_clip_rect(neko_gui_context_t* ctx, neko_gui_rect_t rect) {
    neko_gui_rect_t last = neko_gui_get_clip_rect(ctx);
    neko_gui_stack_push(ctx->clip_stack, neko_gui_intersect_rects(rect, last));
}

NEKO_API_DECL void neko_gui_pop_clip_rect(neko_gui_context_t* ctx) { neko_gui_stack_pop(ctx->clip_stack); }

NEKO_API_DECL neko_gui_rect_t neko_gui_get_clip_rect(neko_gui_context_t* ctx) {
    neko_expect(ctx->clip_stack.idx > 0);
    return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
}

NEKO_API_DECL s32 neko_gui_check_clip(neko_gui_context_t* ctx, neko_gui_rect_t r) {
    neko_gui_rect_t cr = neko_gui_get_clip_rect(ctx);

    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h || r.y + r.h < cr.y) {
        return NEKO_GUI_CLIP_ALL;
    }

    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y && r.y + r.h <= cr.y + cr.h) {
        return 0;
    }

    return NEKO_GUI_CLIP_PART;
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_current_container(neko_gui_context_t* ctx) {
    neko_expect(ctx->container_stack.idx > 0);
    return ctx->container_stack.items[ctx->container_stack.idx - 1];
}

NEKO_API_DECL void neko_gui_current_container_close(neko_gui_context_t* ctx) {
    neko_gui_container_t* cnt = neko_gui_get_current_container(ctx);
    cnt->open = false;
}

NEKO_API_DECL neko_gui_container_t* neko_gui_get_container(neko_gui_context_t* ctx, const char* name) {
    neko_gui_id id = neko_gui_get_id(ctx, name, strlen(name));
    return neko_gui_get_container_ex(ctx, id, 0);
}

NEKO_API_DECL void neko_gui_bring_to_front(neko_gui_context_t* ctx, neko_gui_container_t* cnt) {
    neko_gui_container_t* root = neko_gui_get_root_container(ctx, cnt);
    if (root->opt & NEKO_GUI_OPT_NOBRINGTOFRONT) {
        if (cnt->opt & NEKO_GUI_OPT_DOCKSPACE)
            cnt->zindex = 0;
        else
            cnt->zindex = 2;
        if (cnt->tab_bar) {
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                ((neko_gui_container_t*)tab_bar->items[i].data)->zindex = cnt->zindex + i;
            }
        }
    } else {
        cnt->zindex = ++ctx->last_zindex;

        // If container is part of a tab item, then iterate and bring to front as well
        if (cnt->tab_bar) {
            neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (u32 i = 0; i < tab_bar->size; ++i) {
                ((neko_gui_container_t*)tab_bar->items[i].data)->zindex = ++ctx->last_zindex;
            }
        }
    }
}

/*============================================================================
** Pool
**============================================================================*/

NEKO_API_DECL s32 neko_gui_pool_init(neko_gui_context_t* ctx, neko_gui_pool_item_t* items, s32 len, neko_gui_id id) {
    s32 i, n = -1, f = ctx->frame;
    for (i = 0; i < len; i++) {
        if (items[i].last_update < f) {
            f = items[i].last_update;
            n = i;
        }
    }

    neko_expect(n > -1);
    items[n].id = id;
    neko_gui_pool_update(ctx, items, n);

    return n;
}

NEKO_API_DECL s32 neko_gui_pool_get(neko_gui_context_t* ctx, neko_gui_pool_item_t* items, s32 len, neko_gui_id id) {
    // Note: This is a linear hash lookup. Could speed this up with a quadratic lookup.
    s32 i;
    neko_gui_unused(ctx);
    for (i = 0; i < len; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

NEKO_API_DECL void neko_gui_pool_update(neko_gui_context_t* ctx, neko_gui_pool_item_t* items, s32 idx) { items[idx].last_update = ctx->frame; }

/*============================================================================
** input handlers
**============================================================================*/

NEKO_API_DECL void neko_gui_input_mousemove(neko_gui_context_t* ctx, s32 x, s32 y) { ctx->mouse_pos = neko_v2((f32)x, (f32)y); }

NEKO_API_DECL void neko_gui_input_mousedown(neko_gui_context_t* ctx, s32 x, s32 y, s32 btn) {
    neko_gui_input_mousemove(ctx, x, y);
    ctx->mouse_down |= btn;
    ctx->mouse_pressed |= btn;
}

NEKO_API_DECL void neko_gui_input_mouseup(neko_gui_context_t* ctx, s32 x, s32 y, s32 btn) {
    neko_gui_input_mousemove(ctx, x, y);
    ctx->mouse_down &= ~btn;
}

NEKO_API_DECL void neko_gui_input_scroll(neko_gui_context_t* ctx, s32 x, s32 y) {
    ctx->scroll_delta.x += x;
    ctx->scroll_delta.y += y;
}

NEKO_API_DECL void neko_gui_input_keydown(neko_gui_context_t* ctx, s32 key) {
    ctx->key_pressed |= key;
    ctx->key_down |= key;
}

NEKO_API_DECL void neko_gui_input_keyup(neko_gui_context_t* ctx, s32 key) { ctx->key_down &= ~key; }

NEKO_API_DECL void neko_gui_input_text(neko_gui_context_t* ctx, const char* text) {
    s32 len = strlen(ctx->input_text);
    s32 size = strlen(text) + 1;
    if (len + size > (s32)sizeof(ctx->input_text)) return;
    memcpy(ctx->input_text + len, text, size);
}

/*============================================================================
** commandlist
**============================================================================*/

NEKO_API_DECL neko_gui_command_t* neko_gui_push_command(neko_gui_context_t* ctx, s32 type, s32 size) {
    neko_gui_command_t* cmd = (neko_gui_command_t*)(ctx->command_list.items + ctx->command_list.idx);
    neko_expect(ctx->command_list.idx + size < NEKO_GUI_COMMANDLIST_SIZE);
    cmd->base.type = type;
    cmd->base.size = size;
    ctx->command_list.idx += size;
    return cmd;
}

NEKO_API_DECL s32 neko_gui_next_command(neko_gui_context_t* ctx, neko_gui_command_t** cmd) {
    if (*cmd) {
        *cmd = (neko_gui_command_t*)(((char*)*cmd) + (*cmd)->base.size);
    } else {
        *cmd = (neko_gui_command_t*)ctx->command_list.items;
    }

    while ((u8*)*cmd != (u8*)(ctx->command_list.items + ctx->command_list.idx)) {
        if ((*cmd)->type != NEKO_GUI_COMMAND_JUMP) {
            return 1;
        }
        *cmd = (neko_gui_command_t*)((*cmd)->jump.dst);
    }
    return 0;
}

NEKO_API_DECL void neko_gui_set_clip(neko_gui_context_t* ctx, neko_gui_rect_t rect) {
    neko_gui_command_t* cmd;
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_CLIP, sizeof(neko_gui_clipcommand_t));
    cmd->clip.rect = rect;
}

NEKO_API_DECL void neko_gui_set_pipeline(neko_gui_context_t* ctx, neko_handle(neko_graphics_pipeline_t) pip, void* layout, size_t sz, neko_idraw_layout_type type) {
    neko_gui_command_t* cmd;
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_PIPELINE, sizeof(neko_gui_pipelinecommand_t));
    cmd->pipeline.pipeline = pip;
    cmd->pipeline.layout_type = type;
    cmd->pipeline.layout = ctx->command_list.items + ctx->command_list.idx;
    cmd->pipeline.layout_sz = sz;
    cmd->base.size += sz;

    // Copy data and move list forward
    memcpy(ctx->command_list.items + ctx->command_list.idx, layout, sz);
    ctx->command_list.idx += sz;
}

NEKO_API_DECL void neko_gui_bind_uniforms(neko_gui_context_t* ctx, neko_graphics_bind_uniform_desc_t* uniforms, size_t uniforms_sz) {
    neko_gui_command_t* cmd;
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_UNIFORMS, sizeof(neko_gui_binduniformscommand_t));
    cmd->uniforms.data = ctx->command_list.items + ctx->command_list.idx;

    // Treat as byte buffer, write into data then set size
    neko_byte_buffer_t buffer = neko_default_val();
    buffer.capacity = NEKO_GUI_COMMANDLIST_SIZE;
    buffer.data = (u8*)cmd->uniforms.data;

    const u16 ct = uniforms_sz / sizeof(neko_graphics_bind_uniform_desc_t);

    // Write count
    neko_byte_buffer_write(&buffer, u16, ct);

    // Iterate through all uniforms, memcpy data as needed for each uniform in list
    for (u32 i = 0; i < ct; ++i) {
        neko_graphics_bind_uniform_desc_t* decl = &uniforms[i];
        neko_handle(neko_graphics_uniform_t) hndl = decl->uniform;
        const size_t sz = neko_graphics_uniform_size_query(hndl);
        neko_byte_buffer_write(&buffer, neko_handle(neko_graphics_uniform_t), hndl);
        neko_byte_buffer_write(&buffer, size_t, sz);
        neko_byte_buffer_write(&buffer, u16, (u16)decl->binding);
        neko_byte_buffer_write_bulk(&buffer, decl->data, sz);
    }

    // Record final sizes
    const size_t sz = buffer.size;
    cmd->base.size += sz;
    ctx->command_list.idx += sz;
}

NEKO_API_DECL void neko_gui_draw_line(neko_gui_context_t* ctx, neko_vec2 start, neko_vec2 end, neko_color_t color) {
    neko_gui_command_t* cmd;
    neko_gui_rect_t rect = neko_default_val();
    neko_vec2 s = start.x < end.x ? start : end;
    neko_vec2 e = start.x < end.x ? end : start;
    neko_gui_rect(s.x, s.y, e.x - s.x, e.y - s.y);
    rect = neko_gui_intersect_rects(rect, neko_gui_get_clip_rect(ctx));

    // do clip command if the rect isn't fully contained within the cliprect
    s32 clipped = neko_gui_check_clip(ctx, rect);
    if (clipped == NEKO_GUI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_GUI_CLIP_PART) {
        neko_gui_set_clip(ctx, neko_gui_get_clip_rect(ctx));
    }

    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_SHAPE, sizeof(neko_gui_shapecommand_t));
    cmd->shape.type = NEKO_GUI_SHAPE_LINE;
    cmd->shape.line.start = s;
    cmd->shape.line.end = e;
    cmd->shape.color = color;

    // reset clipping if it was set
    if (clipped) {
        neko_gui_set_clip(ctx, neko_gui_unclipped_rect);
    }
}

NEKO_API_DECL void neko_gui_draw_rect(neko_gui_context_t* ctx, neko_gui_rect_t rect, neko_color_t color) {
    neko_gui_command_t* cmd;
    rect = neko_gui_intersect_rects(rect, neko_gui_get_clip_rect(ctx));
    if (rect.w > 0 && rect.h > 0) {
        cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_SHAPE, sizeof(neko_gui_shapecommand_t));
        cmd->shape.type = NEKO_GUI_SHAPE_RECT;
        cmd->shape.rect = rect;
        cmd->shape.color = color;
    }
}

NEKO_API_DECL void neko_gui_draw_circle(neko_gui_context_t* ctx, neko_vec2 position, float radius, neko_color_t color) {
    neko_gui_command_t* cmd;
    neko_gui_rect_t rect = neko_gui_rect(position.x - radius, position.y - radius, 2.f * radius, 2.f * radius);

    // do clip command if the rect isn't fully contained within the cliprect
    s32 clipped = neko_gui_check_clip(ctx, rect);
    if (clipped == NEKO_GUI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_GUI_CLIP_PART) {
        neko_gui_set_clip(ctx, neko_gui_get_clip_rect(ctx));
    }

    // do shape command
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_SHAPE, sizeof(neko_gui_shapecommand_t));
    cmd->shape.type = NEKO_GUI_SHAPE_CIRCLE;
    cmd->shape.circle.center = position;
    cmd->shape.circle.radius = radius;
    cmd->shape.color = color;

    // reset clipping if it was set
    if (clipped) {
        neko_gui_set_clip(ctx, neko_gui_unclipped_rect);
    }
}

NEKO_API_DECL void neko_gui_draw_triangle(neko_gui_context_t* ctx, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color) {
    neko_gui_command_t* cmd;

    // Check each point against rect (if partially clipped, then good
    s32 clipped = 0x00;
    neko_gui_rect_t clip = neko_gui_get_clip_rect(ctx);
    s32 ca = neko_gui_rect_overlaps_vec2(clip, a);
    s32 cb = neko_gui_rect_overlaps_vec2(clip, b);
    s32 cc = neko_gui_rect_overlaps_vec2(clip, c);

    if (ca && cb && cc)
        clipped = 0x00;  // No clip
    else if (!ca && !cb && !cc)
        clipped = NEKO_GUI_CLIP_ALL;
    else if (ca || cb || cc)
        clipped = NEKO_GUI_CLIP_PART;

    if (clipped == NEKO_GUI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_GUI_CLIP_PART) {
        neko_gui_set_clip(ctx, clip);
    }

    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_SHAPE, sizeof(neko_gui_shapecommand_t));
    cmd->shape.type = NEKO_GUI_SHAPE_TRIANGLE;
    cmd->shape.triangle.points[0] = a;
    cmd->shape.triangle.points[1] = b;
    cmd->shape.triangle.points[2] = c;
    cmd->shape.color = color;

    // Reset clipping if set
    if (clipped) {
        neko_gui_set_clip(ctx, neko_gui_unclipped_rect);
    }
}

NEKO_API_DECL void neko_gui_draw_box(neko_gui_context_t* ctx, neko_gui_rect_t rect, s16* w, neko_color_t color) {
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
    // neko_idraw_rectvd(dl, neko_v2(rect.x, rect.y), neko_v2(rect.w, rect.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_RED, NEKO_GRAPHICS_PRIMITIVE_LINES);

    const float l = (float)w[0], r = (float)w[1], t = (float)w[2], b = (float)w[3];
    neko_gui_draw_rect(ctx, neko_gui_rect(rect.x + l, rect.y, rect.w - r - l, t), color);               // top
    neko_gui_draw_rect(ctx, neko_gui_rect(rect.x + l, rect.y + rect.h - b, rect.w - r - l, b), color);  // bottom
    neko_gui_draw_rect(ctx, neko_gui_rect(rect.x, rect.y, l, rect.h), color);                           // left
    neko_gui_draw_rect(ctx, neko_gui_rect(rect.x + rect.w - r, rect.y, r, rect.h), color);              // right
}

NEKO_API_DECL void neko_gui_draw_text(neko_gui_context_t* ctx, neko_asset_ascii_font_t* font, const char* str, s32 len, neko_vec2 pos, neko_color_t color, s32 shadow_x, s32 shadow_y,
                                      neko_color_t shadow_color) {
    // Set to default font
    if (!font) {
        font = neko_idraw_default_font();
    }

#define DRAW_TEXT(TEXT, RECT, COLOR)                                                                   \
    do {                                                                                               \
        neko_gui_command_t* cmd;                                                                       \
        neko_vec2 td = neko_gui_text_dimensions(font, TEXT, -1);                                       \
        neko_gui_rect_t rect = (RECT);                                                                 \
        s32 clipped = neko_gui_check_clip(ctx, rect);                                                  \
                                                                                                       \
        if (clipped == NEKO_GUI_CLIP_ALL) {                                                            \
            return;                                                                                    \
        }                                                                                              \
                                                                                                       \
        if (clipped == NEKO_GUI_CLIP_PART) {                                                           \
            neko_gui_rect_t crect = neko_gui_get_clip_rect(ctx);                                       \
            neko_gui_set_clip(ctx, crect);                                                             \
        }                                                                                              \
                                                                                                       \
        /* add command */                                                                              \
        if (len < 0) {                                                                                 \
            len = strlen(TEXT);                                                                        \
        }                                                                                              \
                                                                                                       \
        cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_TEXT, sizeof(neko_gui_textcommand_t) + len); \
        memcpy(cmd->text.str, TEXT, len);                                                              \
        cmd->text.str[len] = '\0';                                                                     \
        cmd->text.pos = neko_v2(rect.x, rect.y);                                                       \
        cmd->text.color = COLOR;                                                                       \
        cmd->text.font = font;                                                                         \
                                                                                                       \
        if (clipped) {                                                                                 \
            neko_gui_set_clip(ctx, neko_gui_unclipped_rect);                                           \
        }                                                                                              \
    } while (0)

    // Draw shadow
    if (shadow_x || shadow_y && shadow_color.a) {
        DRAW_TEXT(str, neko_gui_rect(pos.x + (float)shadow_x, pos.y + (float)shadow_y, td.x, td.y), shadow_color);
    }

    // Draw text
    { DRAW_TEXT(str, neko_gui_rect(pos.x, pos.y, td.x, td.y), color); }
}

NEKO_API_DECL void neko_gui_draw_image(neko_gui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_gui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color) {
    neko_gui_command_t* cmd;

    /* do clip command if the rect isn't fully contained within the cliprect */
    s32 clipped = neko_gui_check_clip(ctx, rect);
    if (clipped == NEKO_GUI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_GUI_CLIP_PART) {
        neko_gui_set_clip(ctx, neko_gui_get_clip_rect(ctx));
    }

    /* do image command */
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_IMAGE, sizeof(neko_gui_imagecommand_t));
    cmd->image.hndl = hndl;
    cmd->image.rect = rect;
    cmd->image.uvs = neko_v4(uv0.x, uv0.y, uv1.x, uv1.y);
    cmd->image.color = color;

    /* reset clipping if it was set */
    if (clipped) {
        neko_gui_set_clip(ctx, neko_gui_unclipped_rect);
    }
}

NEKO_API_DECL void neko_gui_draw_custom(neko_gui_context_t* ctx, neko_gui_rect_t rect, neko_gui_draw_callback_t cb, void* data, size_t sz) {
    neko_gui_command_t* cmd;

    neko_gui_rect_t viewport = rect;

    rect = neko_gui_intersect_rects(rect, neko_gui_get_clip_rect(ctx));

    /* do clip command if the rect isn't fully contained within the cliprect */
    s32 clipped = neko_gui_check_clip(ctx, rect);
    if (clipped == NEKO_GUI_CLIP_ALL) {
        return;
    }
    if (clipped == NEKO_GUI_CLIP_PART) {
        neko_gui_set_clip(ctx, neko_gui_get_clip_rect(ctx));
    }

    s32 idx = ctx->id_stack.idx;
    neko_gui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : NEKO_GUI_HASH_INITIAL;

    /* do custom command */
    cmd = neko_gui_push_command(ctx, NEKO_GUI_COMMAND_CUSTOM, sizeof(neko_gui_customcommand_t));
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
        neko_gui_set_clip(ctx, neko_gui_unclipped_rect);
    }
}

NEKO_API_DECL void neko_gui_draw_nine_rect(neko_gui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_gui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, u32 left, u32 right, u32 top,
                                           u32 bottom, neko_color_t color) {
    // Draw images based on rect, slice image based on uvs (uv0, uv1), original texture dimensions (width, height) and control margins (left, right, top, bottom)
    neko_graphics_texture_desc_t desc = neko_default_val();
    neko_graphics_texture_desc_query(hndl, &desc);
    u32 width = desc.width;
    u32 height = desc.height;

    // tl
    {
        u32 w = left;
        u32 h = top;
        neko_gui_rect_t r = neko_gui_rect(rect.x, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv0.y);
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // tr
    {
        u32 w = right;
        u32 h = top;
        neko_gui_rect_t r = neko_gui_rect(rect.x + rect.w - w, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y);
        neko_vec2 st1 = neko_v2(uv1.x, uv0.y + ((float)top / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // br
    {
        u32 w = right;
        u32 h = bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x + rect.w - (f32)w, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x, uv1.y);
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bl
    {
        u32 w = left;
        u32 h = bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y);
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // top
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = top;
        neko_gui_rect_t r = neko_gui_rect(rect.x + left, rect.y, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y);
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bottom
    {
        u32 w = (u32)rect.w - left - right;
        u32 h = bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x + left, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y);
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // left
    {
        u32 w = left;
        u32 h = (u32)rect.h - top - bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x, uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // right
    {
        u32 w = right;
        u32 h = (u32)rect.h - top - bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x + rect.w - (f32)w, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x, uv1.y - ((float)bottom / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // center
    {
        u32 w = (u32)rect.w - right - left;
        u32 h = (u32)rect.h - top - bottom;
        neko_gui_rect_t r = neko_gui_rect(rect.x + left, rect.y + top, (f32)w, (f32)h);
        neko_vec2 st0 = neko_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        neko_vec2 st1 = neko_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        neko_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }
}

/*============================================================================
** layout
**============================================================================*/
enum { NEKO_GUI_RELATIVE = 1, NEKO_GUI_ABSOLUTE = 2 };

NEKO_API_DECL neko_gui_rect_t neko_gui_layout_anchor(const neko_gui_rect_t* p, s32 width, s32 height, s32 xoff, s32 yoff, neko_gui_layout_anchor_type type) {
    float w = (float)width;
    float h = (float)height;
    neko_gui_rect_t r = neko_gui_rect(p->x, p->y, w, h);

    switch (type) {
        default:
        case NEKO_GUI_LAYOUT_ANCHOR_TOPLEFT: {
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_TOPCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_TOPRIGHT: {
            r.x = p->x + (p->w - w);
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_LEFT: {
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_CENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_RIGHT: {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_BOTTOMLEFT: {
            r.y = p->y + (p->h - h);
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_BOTTOMCENTER: {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h);
        } break;

        case NEKO_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT: {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h);
        } break;
    }

    // Apply offset
    r.x += xoff;
    r.y += yoff;

    return r;
}

NEKO_API_DECL void neko_gui_layout_column_begin(neko_gui_context_t* ctx) { neko_gui_push_layout(ctx, neko_gui_layout_next(ctx), neko_v2(0, 0)); }

NEKO_API_DECL void neko_gui_layout_column_end(neko_gui_context_t* ctx) {
    neko_gui_layout_t *a, *b;
    b = neko_gui_get_layout(ctx);
    neko_gui_stack_pop(ctx->layout_stack);

    /* inherit position/next_row/max from child layout if they are greater */
    a = neko_gui_get_layout(ctx);
    a->position.x = neko_max(a->position.x, b->position.x + b->body.x - a->body.x);
    a->next_row = (s32)neko_max((f32)a->next_row, (f32)b->next_row + (f32)b->body.y - (f32)a->body.y);
    a->max.x = neko_max(a->max.x, b->max.x);
    a->max.y = neko_max(a->max.y, b->max.y);
}

NEKO_API_DECL void neko_gui_layout_row(neko_gui_context_t* ctx, s32 items, const s32* widths, s32 height) {
    neko_gui_style_t* style = ctx->style;
    neko_gui_layout_t* layout = neko_gui_get_layout(ctx);

    if (widths) {
        neko_expect(items <= NEKO_GUI_MAX_WIDTHS);
        memcpy(layout->widths, widths, items * sizeof(widths[0]));
    }
    layout->items = items;
    layout->position = neko_v2((f32)layout->indent, (f32)layout->next_row);
    layout->size.y = (f32)height;
    layout->item_index = 0;
}

NEKO_API_DECL void neko_gui_layout_row_ex(neko_gui_context_t* ctx, s32 items, const s32* widths, s32 height, s32 justification) {
    neko_gui_layout_row(ctx, items, widths, height);
    neko_gui_layout_t* layout = neko_gui_get_layout(ctx);

    switch (justification) {
        default:
            break;

        case NEKO_GUI_JUSTIFY_CENTER: {
            // Iterate through all widths, calculate total
            // X is center - tw/2
            float w = 0.f;
            for (u32 i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f ? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w) * 0.5f + layout->indent;
        } break;

        case NEKO_GUI_JUSTIFY_END: {
            float w = 0.f;
            for (u32 i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f ? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w);
        } break;
    }
}

NEKO_API_DECL void neko_gui_layout_width(neko_gui_context_t* ctx, s32 width) { neko_gui_get_layout(ctx)->size.x = (f32)width; }

NEKO_API_DECL void neko_gui_layout_height(neko_gui_context_t* ctx, s32 height) { neko_gui_get_layout(ctx)->size.y = (f32)height; }

NEKO_API_DECL void neko_gui_layout_set_next(neko_gui_context_t* ctx, neko_gui_rect_t r, s32 relative) {
    neko_gui_layout_t* layout = neko_gui_get_layout(ctx);
    layout->next = r;
    layout->next_type = relative ? NEKO_GUI_RELATIVE : NEKO_GUI_ABSOLUTE;
}

NEKO_API_DECL neko_gui_rect_t neko_gui_layout_peek_next(neko_gui_context_t* ctx) {
    neko_gui_layout_t layout = *neko_gui_get_layout(ctx);
    neko_gui_style_t* style = ctx->style;
    neko_gui_rect_t res;

    if (layout.next_type) {
        /* handle rect set by `neko_gui_layout_set_next` */
        s32 type = layout.next_type;
        res = layout.next;
        if (type == NEKO_GUI_ABSOLUTE) {
            return res;
        }

    } else {
        // handle next row
        if (layout.item_index == layout.items) {
            neko_gui_layout_row(ctx, layout.items, NULL, (s32)layout.size.y);
        }

        const s32 items = layout.items;
        const s32 idx = layout.item_index;

        s32 ml = style->margin[NEKO_GUI_MARGIN_LEFT];
        s32 mr = style->margin[NEKO_GUI_MARGIN_RIGHT];
        s32 mt = style->margin[NEKO_GUI_MARGIN_TOP];
        s32 mb = style->margin[NEKO_GUI_MARGIN_BOTTOM];

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
    layout.position.x += res.w + style->margin[NEKO_GUI_MARGIN_RIGHT];
    layout.next_row = (s32)neko_max(layout.next_row, res.y + res.h + style->margin[NEKO_GUI_MARGIN_BOTTOM]);

    /* apply body offset */
    res.x += layout.body.x;
    res.y += layout.body.y;

    /* update max position */
    layout.max.x = neko_max(layout.max.x, res.x + res.w);
    layout.max.y = neko_max(layout.max.y, res.y + res.h);

    return res;
}

NEKO_API_DECL neko_gui_rect_t neko_gui_layout_next(neko_gui_context_t* ctx) {
    neko_gui_layout_t* layout = neko_gui_get_layout(ctx);
    neko_gui_style_t* style = ctx->style;
    neko_gui_rect_t res;

    if (layout->next_type) {
        /* handle rect set by `neko_gui_layout_set_next` */
        s32 type = layout->next_type;
        layout->next_type = 0;
        res = layout->next;
        if (type == NEKO_GUI_ABSOLUTE) {
            return (ctx->last_rect = res);
        }

    } else {
        // handle next row
        if (layout->item_index == layout->items) {
            neko_gui_layout_row(ctx, layout->items, NULL, (s32)layout->size.y);
        }

        const s32 items = layout->items;
        const s32 idx = layout->item_index;

        s32 ml = style->margin[NEKO_GUI_MARGIN_LEFT];
        s32 mr = style->margin[NEKO_GUI_MARGIN_RIGHT];
        s32 mt = style->margin[NEKO_GUI_MARGIN_TOP];
        s32 mb = style->margin[NEKO_GUI_MARGIN_BOTTOM];

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
    layout->position.x += res.w + style->margin[NEKO_GUI_MARGIN_RIGHT];
    layout->next_row = (s32)neko_max(layout->next_row, res.y + res.h + style->margin[NEKO_GUI_MARGIN_BOTTOM]);  //  + style->margin[NEKO_GUI_MARGIN_TOP] * 0.5f);

    /* apply body offset */
    res.x += layout->body.x;
    res.y += layout->body.y;

    /* update max position */
    layout->max.x = neko_max(layout->max.x, res.x + res.w);
    layout->max.y = neko_max(layout->max.y, res.y + res.h);

    return (ctx->last_rect = res);
}

/*============================================================================
** controls
**============================================================================*/

static s32 neko_gui_in_hover_root(neko_gui_context_t* ctx) {
    s32 i = ctx->container_stack.idx;
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

NEKO_API_DECL void neko_gui_draw_control_frame(neko_gui_context_t* ctx, neko_gui_id id, neko_gui_rect_t rect, s32 elementid, u64 opt) {
    if (opt & NEKO_GUI_OPT_NOFRAME) {
        return;
    }
    s32 state = ctx->focus == id ? NEKO_GUI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_GUI_ELEMENT_STATE_HOVER : 0x00;
    neko_gui_draw_frame(ctx, rect, &ctx->style_sheet->styles[elementid][state]);
}

NEKO_API_DECL void neko_gui_draw_control_text(neko_gui_context_t* ctx, const char* str, neko_gui_rect_t rect, const neko_gui_style_t* style, u64 opt) {
    neko_vec2 pos = neko_v2(rect.x, rect.y);
    neko_asset_ascii_font_t* font = style->font;
    neko_vec2 td = neko_gui_text_dimensions(font, str, -1);
    s32 tw = (s32)td.x;
    s32 th = (s32)td.y;

    neko_gui_push_clip_rect(ctx, rect);

    // Grab stylings
    const s32 padding_left = style->padding[NEKO_GUI_PADDING_LEFT];
    const s32 padding_top = style->padding[NEKO_GUI_PADDING_TOP];
    const s32 padding_right = style->padding[NEKO_GUI_PADDING_RIGHT];
    const s32 padding_bottom = style->padding[NEKO_GUI_PADDING_BOTTOM];
    const s32 align = style->align_content;
    const s32 justify = style->justify_content;

    // Determine x placement based on justification
    switch (justify) {
        default:
        case NEKO_GUI_JUSTIFY_START: {
            pos.x = rect.x + padding_left;
        } break;

        case NEKO_GUI_JUSTIFY_CENTER: {
            pos.x = rect.x + (rect.w - tw) * 0.5f;
        } break;

        case NEKO_GUI_JUSTIFY_END: {
            pos.x = rect.x + (rect.w - tw) - padding_right;
        } break;
    }

    // Determine y placement based on alignment within rect
    switch (align) {
        default:
        case NEKO_GUI_ALIGN_START: {
            pos.y = rect.y + padding_top;
        } break;

        case NEKO_GUI_ALIGN_CENTER: {
            pos.y = rect.y + (rect.h - th) * 0.5f;
        } break;

        case NEKO_GUI_ALIGN_END: {
            pos.y = rect.y + (rect.h - th) - padding_bottom;
        } break;
    }

    bool is_content = (opt & NEKO_GUI_OPT_ISCONTENT);
    s32 bg_color = is_content ? NEKO_GUI_COLOR_CONTENT_BACKGROUND : NEKO_GUI_COLOR_BACKGROUND;
    s32 sh_color = is_content ? NEKO_GUI_COLOR_CONTENT_SHADOW : NEKO_GUI_COLOR_SHADOW;
    s32 bd_color = is_content ? NEKO_GUI_COLOR_CONTENT_BORDER : NEKO_GUI_COLOR_BORDER;

    s32 sx = style->shadow_x;
    s32 sy = style->shadow_y;
    const neko_color_t* sc = &style->colors[sh_color];

    // Border
    const neko_color_t* bc = &style->colors[bd_color];
    if (bc->a && ~opt & NEKO_GUI_OPT_NOSTYLEBORDER) {
        neko_gui_pop_clip_rect(ctx);
        neko_gui_rect_t border_rect = neko_gui_expand_rect(rect, (s16*)style->border_width);
        neko_gui_push_clip_rect(ctx, border_rect);
        neko_gui_draw_box(ctx, border_rect, (s16*)style->border_width, *bc);
    }

    // Background
    if (~opt & NEKO_GUI_OPT_NOSTYLEBACKGROUND) {
        neko_gui_draw_rect(ctx, rect, style->colors[bg_color]);
    }

    // Text
    neko_gui_draw_text(ctx, font, str, -1, pos, style->colors[NEKO_GUI_COLOR_CONTENT], sx, sy, *sc);

    neko_gui_pop_clip_rect(ctx);
}

NEKO_API_DECL s32 neko_gui_mouse_over(neko_gui_context_t* ctx, neko_gui_rect_t rect) {
    return neko_gui_rect_overlaps_vec2(rect, ctx->mouse_pos) && !ctx->hover_split && !ctx->lock_hover_id && neko_gui_rect_overlaps_vec2(neko_gui_get_clip_rect(ctx), ctx->mouse_pos) &&
           neko_gui_in_hover_root(ctx);
}

NEKO_API_DECL void neko_gui_update_control(neko_gui_context_t* ctx, neko_gui_id id, neko_gui_rect_t rect, u64 opt) {
    s32 mouseover = 0;
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

    neko_gui_id prev_hov = ctx->prev_hover;
    neko_gui_id prev_foc = ctx->prev_focus;

    // I should do updates in here

    if (opt & NEKO_GUI_OPT_FORCEFOCUS) {
        mouseover = neko_gui_rect_overlaps_vec2(neko_gui_get_clip_rect(ctx), ctx->mouse_pos);
    } else {
        mouseover = neko_gui_mouse_over(ctx, rect);
    }

    // Check for 'mouse-over' with id selection here

    if (ctx->focus == id) {
        ctx->updated_focus = 1;
    }
    if (opt & NEKO_GUI_OPT_NOINTERACT) {
        return;
    }

    // Check for hold focus here
    if (mouseover && !ctx->mouse_down) {
        neko_gui_set_hover(ctx, id);
    }

    if (ctx->focus == id) {
        neko_gui_set_focus(ctx, id);
        if (ctx->mouse_pressed && !mouseover) {
            neko_gui_set_focus(ctx, 0);
        }
        if (!ctx->mouse_down && ~opt & NEKO_GUI_OPT_HOLDFOCUS) {
            neko_gui_set_focus(ctx, 0);
        }
    }

    if (ctx->prev_hover == id && !mouseover) {
        ctx->prev_hover = ctx->hover;
    }

    if (ctx->hover == id) {
        if (ctx->mouse_pressed) {
            if ((opt & NEKO_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT) || (~opt & NEKO_GUI_OPT_LEFTCLICKONLY)) {
                neko_gui_set_focus(ctx, id);
            }
        } else if (!mouseover) {
            neko_gui_set_hover(ctx, 0);
        }
    }

    // Do state check
    if (~opt & NEKO_GUI_OPT_NOSWITCHSTATE) {
        if (ctx->focus == id) {
            if (ctx->prev_focus != id)
                ctx->last_focus_state = NEKO_GUI_ELEMENT_STATE_ON_FOCUS;
            else
                ctx->last_focus_state = NEKO_GUI_ELEMENT_STATE_FOCUS;
        } else {
            if (ctx->prev_focus == id)
                ctx->last_focus_state = NEKO_GUI_ELEMENT_STATE_OFF_FOCUS;
            else
                ctx->last_focus_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->hover == id) {
            if (ctx->prev_hover != id)
                ctx->last_hover_state = NEKO_GUI_ELEMENT_STATE_ON_HOVER;
            else
                ctx->last_hover_state = NEKO_GUI_ELEMENT_STATE_HOVER;
        } else {
            if (ctx->prev_hover == id)
                ctx->last_hover_state = NEKO_GUI_ELEMENT_STATE_OFF_HOVER;
            else
                ctx->last_hover_state = NEKO_GUI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->prev_focus == id && !ctx->mouse_down && ~opt & NEKO_GUI_OPT_HOLDFOCUS) {
            ctx->prev_focus = ctx->focus;
        }

        if (ctx->last_hover_state == NEKO_GUI_ELEMENT_STATE_ON_HOVER || ctx->last_hover_state == NEKO_GUI_ELEMENT_STATE_OFF_HOVER || ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_ON_FOCUS ||
            ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_OFF_FOCUS) {
            // Don't have a hover state switch if focused
            ctx->switch_state = ctx->last_focus_state ? ctx->last_focus_state : ctx->focus != id ? ctx->last_hover_state : NEKO_GUI_ELEMENT_STATE_DEFAULT;
            switch (ctx->switch_state) {
                case NEKO_GUI_ELEMENT_STATE_OFF_HOVER:
                case NEKO_GUI_ELEMENT_STATE_ON_HOVER: {
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

NEKO_API_DECL s32 neko_gui_text_ex(neko_gui_context_t* ctx, const char* text, s32 wrap, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = 0, elementid = NEKO_GUI_ELEMENT_TEXT;
    neko_gui_id id = neko_gui_get_id(ctx, text, strlen(text));
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    const char *start, *end, *p = text;
    s32 width = -1;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_asset_ascii_font_t* font = ctx->style->font;
    neko_color_t* color = &ctx->style->colors[NEKO_GUI_COLOR_CONTENT];
    s32 sx = ctx->style->shadow_x;
    s32 sy = ctx->style->shadow_y;
    if (opt & NEKO_GUI_OPT_NOSTYLESHADOW) {
        sx = 0;
        sy = 0;
    }
    neko_color_t* sc = &ctx->style->colors[NEKO_GUI_COLOR_SHADOW];
    s32 th = neko_gui_font_height(font);
    neko_gui_layout_column_begin(ctx);
    neko_gui_layout_row(ctx, 1, &width, th);

    neko_gui_rect_t tr = neko_gui_layout_next(ctx);
    neko_gui_layout_set_next(ctx, tr, 0);
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_rect_t bg = r;
    do {
        s32 w = 0;
        start = end = p;
        do {
            const char* word = p;
            while (*p && *p != ' ' && *p != '\n') {
                p++;
            }

            if (wrap) w += neko_gui_text_width(font, word, p - word);
            if (w > r.w && end != start) {
                break;
            }

            if (wrap) w += neko_gui_text_width(font, p, 1);
            end = p++;

        } while (*end && *end != '\n');

        if (r.w > tr.w) tr.w = r.w;
        tr.h = (r.y + r.h) - tr.y;

        neko_gui_rect_t txtrct = r;
        bg = r;
        if (*end) {
            r = neko_gui_layout_next(ctx);
            bg.h = r.y - bg.y;
        } else {
            s32 th = neko_gui_text_height(font, start, end - start);
            bg.h = r.h + (float)th / 2.f;
        }

        // Draw frame here for background if applicable (need to do this to account for space between wrap)
        if (ctx->style->colors[NEKO_GUI_COLOR_BACKGROUND].a && ~opt & NEKO_GUI_OPT_NOSTYLEBACKGROUND) {
            neko_gui_draw_rect(ctx, bg, style.colors[NEKO_GUI_COLOR_BACKGROUND]);
        }

        // Draw text
        neko_gui_draw_text(ctx, font, start, end - start, neko_v2(txtrct.x, txtrct.y), *color, sx, sy, *sc);
        p = end + 1;

    } while (*end);

    // draw border
    if (style.colors[NEKO_GUI_COLOR_BORDER].a && ~opt & NEKO_GUI_OPT_NOSTYLEBORDER) {
        neko_gui_draw_box(ctx, neko_gui_expand_rect(tr, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]);
    }

    neko_gui_update_control(ctx, id, tr, 0x00);

    // handle click
    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_GUI_RES_SUBMIT;
    }

    neko_gui_layout_column_end(ctx);
    neko_gui_pop_style(ctx, save);

    return res;
}

// NEKO_API_DECL s32 neko_gui_text_fc_ex(neko_gui_context_t* ctx, const char* text, neko_font_index fontindex) {
//     s32 width = -1;
//     s32 th = 20;
//     neko_gui_layout_column_begin(ctx);
//     neko_gui_layout_row(ctx, 1, &width, th);
//     neko_gui_layout_t* layout = neko_gui_get_layout(ctx);
//     if (fontindex == -1) fontindex = ctx->gui_idraw.data->font_fc_default;
//     neko_graphics_fc_text(text, fontindex, layout->body.x, layout->body.y + layout->body.h / 2);
//     neko_gui_layout_column_end(ctx);
//     return 0;
// }

NEKO_API_DECL s32 neko_gui_label_ex(neko_gui_context_t* ctx, const char* label, const neko_gui_selector_desc_t* desc, u64 opt) {
    // Want to push animations here for styles
    s32 res = 0;
    s32 elementid = NEKO_GUI_ELEMENT_LABEL;
    neko_gui_id id = neko_gui_get_id(ctx, label, neko_strlen(label));

    char id_tag[256] = neko_default_val();
    char label_tag[256] = neko_default_val();
    neko_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    neko_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    if (id_tag) neko_gui_push_id(ctx, id_tag, strlen(id_tag));

    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_update_control(ctx, id, r, 0x00);
    neko_gui_draw_control_text(ctx, label_tag, r, &style, 0x00);
    neko_gui_pop_style(ctx, save);
    if (id_tag) neko_gui_pop_id(ctx);

    /* handle click */
    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_GUI_RES_SUBMIT;
    }

    return res;
}

NEKO_API_DECL s32 neko_gui_image_ex(neko_gui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_vec2 uv0, neko_vec2 uv1, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = 0;
    neko_gui_id id = neko_gui_get_id(ctx, &hndl, sizeof(hndl));
    const s32 elementid = NEKO_GUI_ELEMENT_IMAGE;

    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_update_control(ctx, id, r, opt);

    /* handle click */
    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_GUI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[NEKO_GUI_COLOR_BORDER].a) {
        neko_gui_draw_box(ctx, neko_gui_expand_rect(r, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]);
    }

    neko_gui_draw_image(ctx, hndl, r, uv0, uv1, style.colors[NEKO_GUI_COLOR_CONTENT]);

    neko_gui_pop_style(ctx, save);

    return res;
}

NEKO_API_DECL s32 neko_gui_combo_begin_ex(neko_gui_context_t* ctx, const char* id, const char* current_item, s32 max_items, neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = 0;
    opt = NEKO_GUI_OPT_NOMOVE | NEKO_GUI_OPT_NORESIZE | NEKO_GUI_OPT_NOTITLE | NEKO_GUI_OPT_FORCESETRECT;

    if (neko_gui_button(ctx, current_item)) {
        neko_gui_popup_open(ctx, id);
    }

    s32 ct = max_items > 0 ? max_items : 0;
    neko_gui_rect_t rect = ctx->last_rect;
    rect.y += rect.h;
    rect.h = ct ? (ct + 1) * ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x00].size[1] : rect.h;
    return neko_gui_popup_begin_ex(ctx, id, rect, NULL, opt);
}

NEKO_API_DECL s32 neko_gui_combo_item_ex(neko_gui_context_t* ctx, const char* label, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = neko_gui_button_ex(ctx, label, desc, opt);
    if (res) {
        neko_gui_current_container_close(ctx);
    }
    return res;
}

NEKO_API_DECL void neko_gui_combo_end(neko_gui_context_t* ctx) { neko_gui_popup_end(ctx); }

NEKO_API_DECL void neko_gui_parse_label_tag(neko_gui_context_t* ctx, const char* str, char* buffer, size_t sz) {
    neko_lexer_t lex = neko_lexer_c_ctor(str);
    while (neko_lexer_can_lex(&lex)) {
        neko_token_t token = neko_lexer_next_token(&lex);
        switch (token.type) {
            case NEKO_TOKEN_HASH: {
                if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
                    neko_token_t end = neko_lexer_current_token(&lex);

                    // Determine len
                    size_t len = neko_min(end.text - str, sz);

                    memcpy(buffer, str, len);
                    return;
                }
            } break;
        }
    }

    // Reached end, so just memcpy
    memcpy(buffer, str, neko_min(sz, strlen(str) + 1));
}

NEKO_API_DECL void neko_gui_parse_id_tag(neko_gui_context_t* ctx, const char* str, char* buffer, size_t sz) {
    neko_lexer_t lex = neko_lexer_c_ctor(str);
    while (neko_lexer_can_lex(&lex)) {
        neko_token_t token = neko_lexer_next_token(&lex);
        switch (token.type) {
            case NEKO_TOKEN_HASH: {
                if (neko_lexer_peek(&lex).type == NEKO_TOKEN_HASH) {
                    neko_token_t end = neko_lexer_next_token(&lex);
                    end = neko_lexer_next_token(&lex);

                    // Determine len
                    size_t len = neko_min((str + strlen(str)) - end.text, sz);

                    memcpy(buffer, end.text, len);
                    return;
                }
            } break;
        }
    }
}

NEKO_API_DECL s32 neko_gui_button_ex(neko_gui_context_t* ctx, const char* label, const neko_gui_selector_desc_t* desc, u64 opt) {
    // Note: clip out early here for performance

    s32 res = 0;
    neko_gui_id id = neko_gui_get_id(ctx, label, strlen(label));
    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

    char id_tag[256] = neko_default_val();
    char label_tag[256] = neko_default_val();
    neko_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    neko_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, NEKO_GUI_ELEMENT_BUTTON);

    // Push id if tag available
    if (id_tag) {
        neko_gui_push_id(ctx, id_tag, strlen(id_tag));
    }

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, NEKO_GUI_ELEMENT_BUTTON);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, NEKO_GUI_ELEMENT_BUTTON, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, NEKO_GUI_ELEMENT_BUTTON, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, NEKO_GUI_ELEMENT_BUTTON, 0x00);
    }

    // Temporary copy of style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_update_control(ctx, id, r, opt);

    /* handle click or button press for submission */
    if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_OFF_FOCUS) {
        res |= NEKO_GUI_RES_SUBMIT;
    }

    // draw border
    if (style.colors[NEKO_GUI_COLOR_BORDER].a) {
        neko_gui_draw_box(ctx, neko_gui_expand_rect(r, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]);
    }

    opt |= NEKO_GUI_OPT_ISCONTENT;
    neko_gui_draw_rect(ctx, r, style.colors[NEKO_GUI_COLOR_BACKGROUND]);
    if (label) {
        neko_gui_draw_control_text(ctx, label_tag, r, &style, opt);
    }

    neko_gui_pop_style(ctx, save);

    if (id_tag) neko_gui_pop_id(ctx);

    return res;
}

NEKO_API_DECL s32 neko_gui_checkbox_ex(neko_gui_context_t* ctx, const char* label, s32* state, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = 0;
    neko_gui_id id = neko_gui_get_id(ctx, &state, sizeof(state));
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_rect_t box = neko_gui_rect(r.x, r.y, r.h, r.h);
    s32 ox = (s32)(box.w * 0.2f), oy = (s32)(box.h * 0.2f);
    neko_gui_rect_t inner_box = neko_gui_rect(box.x + ox, box.y + oy, box.w - 2 * ox, box.h - 2 * oy);
    neko_gui_update_control(ctx, id, r, 0);

    s32 elementid = NEKO_GUI_ELEMENT_BUTTON;
    neko_gui_style_t style = neko_default_val();
    style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
            : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                               : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);

    /* handle click */
    if ((ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT || (ctx->mouse_pressed && ~opt & NEKO_GUI_OPT_LEFTCLICKONLY)) && ctx->focus == id) {
        res |= NEKO_GUI_RES_CHANGE;
        *state = !*state;
    }

    /* draw */
    neko_gui_draw_control_frame(ctx, id, box, NEKO_GUI_ELEMENT_INPUT, 0);
    if (*state) {
        // Draw in a filled rect
        neko_gui_draw_rect(ctx, inner_box, style.colors[NEKO_GUI_COLOR_BACKGROUND]);
    }

    r = neko_gui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
    neko_gui_draw_control_text(ctx, label, r, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][0], 0);
    return res;
}

NEKO_API_DECL s32 neko_gui_textbox_raw(neko_gui_context_t* ctx, char* buf, s32 bufsz, neko_gui_id id, neko_gui_rect_t rect, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = 0;

    s32 elementid = NEKO_GUI_ELEMENT_INPUT;
    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);

    neko_gui_update_control(ctx, id, rect, opt | NEKO_GUI_OPT_HOLDFOCUS);

    if (ctx->focus == id) {
        /* handle text input */
        s32 len = strlen(buf);
        s32 n = neko_min(bufsz - len - 1, (s32)strlen(ctx->input_text));
        if (n > 0) {
            memcpy(buf + len, ctx->input_text, n);
            len += n;
            buf[len] = '\0';
            res |= NEKO_GUI_RES_CHANGE;
        }

        /* handle backspace */
        if (ctx->key_pressed & NEKO_GUI_KEY_BACKSPACE && len > 0) {
            /* skip utf-8 continuation bytes */
            while ((buf[--len] & 0xc0) == 0x80 && len > 0)
                ;
            buf[len] = '\0';
            res |= NEKO_GUI_RES_CHANGE;
        }

        /* handle return */
        if (ctx->key_pressed & NEKO_GUI_KEY_RETURN) {
            neko_gui_set_focus(ctx, 0);
            res |= NEKO_GUI_RES_SUBMIT;
        }
    }

    /* draw */

    // Textbox border
    neko_gui_draw_box(ctx, neko_gui_expand_rect(rect, (s16*)style.border_width), (s16*)style.border_width, style.colors[NEKO_GUI_COLOR_BORDER]);

    // Textbox bg
    neko_gui_draw_control_frame(ctx, id, rect, NEKO_GUI_ELEMENT_INPUT, opt);

    // Text and carret
    if (ctx->focus == id) {
        neko_gui_style_t* sp = &style;
        neko_color_t* color = &sp->colors[NEKO_GUI_COLOR_CONTENT];
        s32 sx = sp->shadow_x;
        s32 sy = sp->shadow_y;
        neko_color_t* sc = &sp->colors[NEKO_GUI_COLOR_SHADOW];
        neko_asset_ascii_font_t* font = sp->font;
        s32 textw = neko_gui_text_width(font, buf, -1);
        s32 texth = neko_gui_font_height(font);
        s32 ofx = (s32)(rect.w - sp->padding[NEKO_GUI_PADDING_RIGHT] - textw - 1);
        s32 textx = (s32)(rect.x + neko_min(ofx, sp->padding[NEKO_GUI_PADDING_LEFT]));
        s32 texty = (s32)(rect.y + (rect.h - texth) / 2);
        s32 cary = (s32)(rect.y + 1);
        neko_gui_push_clip_rect(ctx, rect);

        // Draw text
        neko_gui_draw_control_text(ctx, buf, rect, &style, opt);

        // Draw caret (control alpha based on frame)
        static bool on = true;
        static float ct = 0.f;
        if (~opt & NEKO_GUI_OPT_NOCARET) {
            neko_vec2 pos = neko_v2(rect.x, rect.y);

            // Grab stylings
            const s32 padding_left = sp->padding[NEKO_GUI_PADDING_LEFT];
            const s32 padding_top = sp->padding[NEKO_GUI_PADDING_TOP];
            const s32 padding_right = sp->padding[NEKO_GUI_PADDING_RIGHT];
            const s32 padding_bottom = sp->padding[NEKO_GUI_PADDING_BOTTOM];
            const s32 align = sp->align_content;
            const s32 justify = sp->justify_content;

            // Determine x placement based on justification
            switch (justify) {
                default:
                case NEKO_GUI_JUSTIFY_START: {
                    pos.x = rect.x + padding_left;
                } break;

                case NEKO_GUI_JUSTIFY_CENTER: {
                    pos.x = rect.x + (rect.w - textw) * 0.5f;
                } break;

                case NEKO_GUI_JUSTIFY_END: {
                    pos.x = rect.x + (rect.w - textw) - padding_right;
                } break;
            }

            // Determine caret position based on style justification
            neko_gui_rect_t cr = neko_gui_rect(pos.x + textw + padding_right, (f32)rect.y + 5.f, 1.f, (f32)rect.h - 10.f);

            if (ctx->last_focus_state == NEKO_GUI_ELEMENT_STATE_ON_FOCUS) {
                on = true;
                ct = 0.f;
            }
            ct += 0.1f;
            if (ct >= 3.f) {
                on = !on;
                ct = 0.f;
            }
            neko_color_t col = *color;
            col.a = on ? col.a : 0;
            neko_gui_draw_rect(ctx, cr, col);
        }

        neko_gui_pop_clip_rect(ctx);
    } else {
        neko_gui_style_t* sp = &style;
        neko_color_t* color = &sp->colors[NEKO_GUI_COLOR_CONTENT];
        neko_asset_ascii_font_t* font = sp->font;
        s32 sx = sp->shadow_x;
        s32 sy = sp->shadow_y;
        neko_color_t* sc = &sp->colors[NEKO_GUI_COLOR_SHADOW];
        s32 textw = neko_gui_text_width(font, buf, -1);
        s32 texth = neko_gui_text_height(font, buf, -1);
        s32 textx = (s32)(rect.x + sp->padding[NEKO_GUI_PADDING_LEFT]);
        s32 texty = (s32)(rect.y + (rect.h - texth) / 2);
        neko_gui_push_clip_rect(ctx, rect);
        neko_gui_draw_control_text(ctx, buf, rect, &style, opt);
        neko_gui_pop_clip_rect(ctx);
    }

    neko_gui_pop_style(ctx, save);

    return res;
}

static s32 neko_gui_number_textbox(neko_gui_context_t* ctx, neko_gui_real* value, neko_gui_rect_t r, neko_gui_id id, const neko_gui_selector_desc_t* desc) {
    if (ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT && ctx->key_down & NEKO_GUI_KEY_SHIFT && ctx->hover == id) {
        ctx->number_edit = id;
        neko_snprintf(ctx->number_edit_buf, NEKO_GUI_MAX_FMT, NEKO_GUI_REAL_FMT, *value);
    }
    if (ctx->number_edit == id) {
        // This is broken for some reason...
        s32 res = neko_gui_textbox_raw(ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, desc, 0);

        if (res & NEKO_GUI_RES_SUBMIT || ctx->focus != id) {
            *value = strtod(ctx->number_edit_buf, NULL);
            ctx->number_edit = 0;
        } else {
            return 1;
        }
    }
    return 0;
}

NEKO_API_DECL s32 neko_gui_textbox_ex(neko_gui_context_t* ctx, char* buf, s32 bufsz, const neko_gui_selector_desc_t* desc, u64 opt) {
    // Handle animation here...
    s32 res = 0;
    neko_gui_id id = neko_gui_get_id(ctx, &buf, sizeof(buf));
    s32 elementid = NEKO_GUI_ELEMENT_INPUT;
    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        // Need to check that I haven't updated more than once this frame
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Push temp style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t r = neko_gui_layout_next(ctx);
    neko_gui_update_control(ctx, id, r, opt | NEKO_GUI_OPT_HOLDFOCUS);
    res |= neko_gui_textbox_raw(ctx, buf, bufsz, id, r, desc, opt);
    neko_gui_pop_style(ctx, save);

    return res;
}

NEKO_API_DECL s32 neko_gui_slider_ex(neko_gui_context_t* ctx, neko_gui_real* value, neko_gui_real low, neko_gui_real high, neko_gui_real step, const char* fmt, const neko_gui_selector_desc_t* desc,
                                     u64 opt) {
    char buf[NEKO_GUI_MAX_FMT + 1];
    neko_gui_rect_t thumb;
    s32 x, w, res = 0;
    neko_gui_real last = *value, v = last;
    neko_gui_id id = neko_gui_get_id(ctx, &value, sizeof(value));
    s32 elementid = NEKO_GUI_ELEMENT_INPUT;
    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);
    s32 state = ctx->focus == id ? NEKO_GUI_ELEMENT_STATE_FOCUS : ctx->hover == id ? NEKO_GUI_ELEMENT_STATE_HOVER : NEKO_GUI_ELEMENT_STATE_DEFAULT;

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = neko_gui_get_current_element_style(ctx, desc, elementid, state);
    }

    // Temporary copy of style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t base = neko_gui_layout_next(ctx);

    /* handle text input mode */
    if (neko_gui_number_textbox(ctx, &v, base, id, desc)) {
        return res;
    }

    /* handle normal mode */
    neko_gui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && (ctx->mouse_down | ctx->mouse_pressed) == NEKO_GUI_MOUSE_LEFT) {
        v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
        if (step) {
            v = (((v + step / 2) / step)) * step;
        }
    }

    /* clamp and store value, update res */
    *value = v = neko_clamp(v, low, high);
    if (last != v) {
        res |= NEKO_GUI_RES_CHANGE;
    }

    /* draw base */
    neko_gui_draw_control_frame(ctx, id, base, NEKO_GUI_ELEMENT_INPUT, opt);

    /* draw control */
    w = style.thumb_size;  // Don't like this...
    x = (s32)((v - low) * (base.w - w) / (high - low));
    thumb = neko_gui_rect((f32)base.x + (f32)x, base.y, (f32)w, base.h);
    neko_gui_draw_control_frame(ctx, id, thumb, NEKO_GUI_ELEMENT_BUTTON, opt);

    /* draw text    */
    style.colors[NEKO_GUI_COLOR_BACKGROUND] = ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][state].colors[NEKO_GUI_COLOR_BACKGROUND];
    neko_snprintf(buf, NEKO_GUI_MAX_FMT, fmt, v);
    neko_gui_draw_control_text(ctx, buf, base, &style, opt);  // oh...bg

    // Pop style
    neko_gui_pop_style(ctx, save);

    return res;
}

NEKO_API_DECL s32 neko_gui_number_ex(neko_gui_context_t* ctx, neko_gui_real* value, neko_gui_real step, const char* fmt, const neko_gui_selector_desc_t* desc, u64 opt) {
    char buf[NEKO_GUI_MAX_FMT + 1];
    s32 res = 0;
    neko_gui_id id = neko_gui_get_id(ctx, &value, sizeof(value));
    s32 elementid = NEKO_GUI_ELEMENT_INPUT;
    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    // Temporary copy of style
    neko_gui_style_t* save = neko_gui_push_style(ctx, &style);
    neko_gui_rect_t base = neko_gui_layout_next(ctx);
    neko_gui_real last = *value;

    /* handle text input mode */
    if (neko_gui_number_textbox(ctx, value, base, id, desc)) {
        neko_gui_pop_style(ctx, save);
        return res;
    }

    /* handle normal mode */
    neko_gui_update_control(ctx, id, base, opt);

    /* handle input */
    if (ctx->focus == id && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
        *value += ctx->mouse_delta.x * step;
    }

    /* set flag if value changed */
    if (*value != last) {
        res |= NEKO_GUI_RES_CHANGE;
    }

    /* draw base */
    neko_gui_draw_control_frame(ctx, id, base, NEKO_GUI_ELEMENT_INPUT, opt);

    /* draw text    */
    neko_snprintf(buf, NEKO_GUI_MAX_FMT, fmt, *value);
    neko_gui_draw_control_text(ctx, buf, base, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][0], opt);

    neko_gui_pop_style(ctx, save);

    return res;
}

static s32 _neko_gui_header(neko_gui_context_t* ctx, const char* label, s32 istreenode, const neko_gui_selector_desc_t* desc, u64 opt) {
    neko_gui_rect_t r;
    s32 active, expanded;
    neko_gui_id id = neko_gui_get_id(ctx, label, strlen(label));
    s32 idx = neko_gui_pool_get(ctx, ctx->treenode_pool, NEKO_GUI_TREENODEPOOL_SIZE, id);
    s32 width = -1;
    neko_gui_layout_row(ctx, 1, &width, 0);

    char id_tag[256] = neko_default_val();
    char label_tag[256] = neko_default_val();
    neko_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    neko_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    if (id_tag) neko_gui_push_id(ctx, id_tag, strlen(id_tag));

    active = (idx >= 0);
    expanded = (opt & NEKO_GUI_OPT_EXPANDED) ? !active : active;
    r = neko_gui_layout_next(ctx);
    neko_gui_update_control(ctx, id, r, 0);

    /* handle click */
    active ^= (ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT && ctx->focus == id);

    /* update pool ref */
    if (idx >= 0) {
        if (active) {
            neko_gui_pool_update(ctx, ctx->treenode_pool, idx);
        } else {
            memset(&ctx->treenode_pool[idx], 0, sizeof(neko_gui_pool_item_t));
        }

    } else if (active) {
        neko_gui_pool_init(ctx, ctx->treenode_pool, NEKO_GUI_TREENODEPOOL_SIZE, id);
    }

    /* draw */
    if (istreenode) {
        if (ctx->hover == id) {
            neko_gui_draw_frame(ctx, r, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][NEKO_GUI_ELEMENT_STATE_HOVER]);
        }
    } else {
        neko_gui_draw_control_frame(ctx, id, r, NEKO_GUI_ELEMENT_BUTTON, 0);
    }

    const float sz = 6.f;
    if (expanded) {
        neko_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        neko_vec2 b = neko_vec2_add(a, neko_v2(sz, 0.f));
        neko_vec2 c = neko_vec2_add(a, neko_v2(sz / 2.f, sz));
        neko_gui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][0x00].colors[NEKO_GUI_COLOR_CONTENT]);
    } else {
        neko_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        neko_vec2 b = neko_vec2_add(a, neko_v2(sz, sz / 2.f));
        neko_vec2 c = neko_vec2_add(a, neko_v2(0.f, sz));
        neko_gui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][0x00].colors[NEKO_GUI_COLOR_CONTENT]);
    }

    // Draw text for treenode
    r.x += r.h - ctx->style->padding[NEKO_GUI_PADDING_TOP];
    r.w -= r.h - ctx->style->padding[NEKO_GUI_PADDING_BOTTOM];
    neko_gui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_TEXT][0x00], 0);

    if (id_tag) neko_gui_pop_id(ctx);

    return expanded ? NEKO_GUI_RES_ACTIVE : 0;
}

NEKO_API_DECL s32 neko_gui_header_ex(neko_gui_context_t* ctx, const char* label, const neko_gui_selector_desc_t* desc, u64 opt) { return _neko_gui_header(ctx, label, 0, desc, opt); }

NEKO_API_DECL s32 neko_gui_treenode_begin_ex(neko_gui_context_t* ctx, const char* label, const neko_gui_selector_desc_t* desc, u64 opt) {
    s32 res = _neko_gui_header(ctx, label, 1, desc, opt);
    if (res & NEKO_GUI_RES_ACTIVE) {
        neko_gui_get_layout(ctx)->indent += ctx->style->indent;
        neko_gui_stack_push(ctx->id_stack, ctx->last_id);
    }

    return res;
}

NEKO_API_DECL void neko_gui_treenode_end(neko_gui_context_t* ctx) {
    neko_gui_get_layout(ctx)->indent -= ctx->style->indent;
    neko_gui_pop_id(ctx);
}

// -1 for left, + 1 for right
NEKO_API_DECL void neko_gui_tab_item_swap(neko_gui_context_t* ctx, neko_gui_container_t* cnt, s32 direction) {
    neko_gui_tab_bar_t* tab_bar = neko_gui_get_tab_bar(ctx, cnt);
    if (!tab_bar) return;

    s32 item = (s32)cnt->tab_item;
    s32 idx = neko_clamp(item + direction, 0, (s32)tab_bar->size - 1);

    neko_gui_container_t* scnt = (neko_gui_container_t*)tab_bar->items[idx].data;

    neko_gui_tab_item_t* cti = &tab_bar->items[cnt->tab_item];
    neko_gui_tab_item_t* sti = &tab_bar->items[idx];
    neko_gui_tab_item_t tmp = *cti;

    // Swap cti
    sti->data = cnt;
    cnt->tab_item = sti->idx;

    // Swap sti
    cti->data = scnt;
    scnt->tab_item = cti->idx;

    tab_bar->focus = sti->idx;
}

NEKO_API_DECL s32 neko_gui_window_begin_ex(neko_gui_context_t* ctx, const char* title, neko_gui_rect_t rect, bool* open, const neko_gui_selector_desc_t* desc, u64 opt) {
    neko_gui_rect_t body;
    neko_gui_id id = neko_gui_get_id(ctx, title, strlen(title));
    neko_gui_container_t* cnt = neko_gui_get_container_ex(ctx, id, opt);

    char id_tag[256] = neko_default_val();
    char label_tag[256] = neko_default_val();
    neko_gui_parse_id_tag(ctx, title, id_tag, sizeof(id_tag));
    neko_gui_parse_label_tag(ctx, title, label_tag, sizeof(label_tag));

    if (cnt && open) {
        cnt->open = *open;
    }

    if (!cnt || !cnt->open) {
        return 0;
    }

    memcpy(cnt->name, label_tag, 256);

    const s32 title_max_size = 100;

    bool new_frame = cnt->frame != ctx->frame;

    s32 state = ctx->active_root == cnt ? NEKO_GUI_ELEMENT_STATE_FOCUS : ctx->hover_root == cnt ? NEKO_GUI_ELEMENT_STATE_HOVER : NEKO_GUI_ELEMENT_STATE_DEFAULT;

    const float split_size = NEKO_GUI_SPLIT_SIZE;

    neko_gui_stack_push(ctx->id_stack, id);

    // Get splits
    neko_gui_split_t* split = neko_gui_get_split(ctx, cnt);
    neko_gui_split_t* root_split = neko_gui_get_root_split(ctx, cnt);

    // Get root container
    neko_gui_container_t* root_cnt = neko_gui_get_root_container(ctx, cnt);

    // Cache rect
    if ((cnt->rect.w == 0.f || opt & NEKO_GUI_OPT_FORCESETRECT || opt & NEKO_GUI_OPT_FULLSCREEN || cnt->flags & NEKO_GUI_WINDOW_FLAGS_FIRST_INIT) && new_frame) {
        if (opt & NEKO_GUI_OPT_FULLSCREEN) {
            neko_vec2 fb = ctx->framebuffer_size;
            cnt->rect = neko_gui_rect(0, 0, fb.x, fb.y);

            // Set root split rect size
            if (root_split) {
                root_split->rect = cnt->rect;
                neko_gui_update_split(ctx, root_split);
            }
        } else {
            // Set root split rect size
            if (root_split && root_cnt == cnt) {
                root_split->rect = rect;
                neko_gui_update_split(ctx, root_split);
            } else {
                cnt->rect = rect;
            }
        }
        cnt->flags = cnt->flags & ~NEKO_GUI_WINDOW_FLAGS_FIRST_INIT;
    }
    neko_gui_begin_root_container(ctx, cnt, opt);
    rect = body = cnt->rect;
    cnt->opt = opt;

    if (opt & NEKO_GUI_OPT_DOCKSPACE) {
        cnt->zindex = 0;
    }

    // If parent cannot move/resize, set to this opt as well
    if (root_cnt->opt & NEKO_GUI_OPT_NOMOVE) {
        cnt->opt |= NEKO_GUI_OPT_NOMOVE;
    }

    if (root_cnt->opt & NEKO_GUI_OPT_NORESIZE) {
        cnt->opt |= NEKO_GUI_OPT_NORESIZE;
    }

    // If in a tab view, then title has to be handled differently...
    neko_gui_tab_bar_t* tab_bar = cnt->tab_bar ? neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL;
    neko_gui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;

    if (tab_item && tab_item) {
        if (tab_bar->focus == tab_item->idx) {
            cnt->flags |= NEKO_GUI_WINDOW_FLAGS_VISIBLE;
            cnt->opt &= !NEKO_GUI_OPT_NOINTERACT;
            cnt->opt &= !NEKO_GUI_OPT_NOHOVER;
        } else {
            cnt->flags &= ~NEKO_GUI_WINDOW_FLAGS_VISIBLE;
            cnt->opt |= NEKO_GUI_OPT_NOINTERACT;
            cnt->opt |= NEKO_GUI_OPT_NOHOVER;
        }
    }

    bool in_root = false;

    // If hovered root is in the tab group and moused over, then is hovered
    if (tab_bar) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (ctx->hover_root == (neko_gui_container_t*)tab_bar->items[i].data) {
                in_root = true;
                break;
            }
        }
    }

    neko_gui_container_t* s_cnt = cnt;
    if (tab_bar && split) {
        for (u32 i = 0; i < tab_bar->size; ++i) {
            if (((neko_gui_container_t*)tab_bar->items[i].data)->split) {
                s_cnt = (neko_gui_container_t*)tab_bar->items[i].data;
            }
        }
    }

    // Do split size/position
    if (split) {
        const neko_gui_style_t* cstyle = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][state];
        const neko_gui_rect_t* sr = &split->rect;
        const float ratio = split->ratio;
        float shsz = split_size;
        const float omr = (1.f - ratio);

        switch (split->type) {
            case NEKO_GUI_SPLIT_LEFT: {
                if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = neko_gui_rect(sr->x + sr->w * ratio + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                }

            } break;

            case NEKO_GUI_SPLIT_RIGHT: {
                if (split->children[NEKO_GUI_SPLIT_NODE_PARENT].container == s_cnt) {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz);
                } else {
                    cnt->rect = neko_gui_rect(sr->x + sr->w * (1.f - ratio) + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz);
                }
            } break;

            case NEKO_GUI_SPLIT_TOP: {
                if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * ratio - 2.f * shsz);
                } else {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + sr->h * ratio + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;

            case NEKO_GUI_SPLIT_BOTTOM: {
                if (split->children[NEKO_GUI_SPLIT_NODE_CHILD].container == s_cnt) {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + sr->h * (1.f - ratio) + shsz, sr->w - 2.f * shsz, sr->h * (ratio)-2.f * shsz);
                } else {
                    cnt->rect = neko_gui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                }
            } break;
        }
    }

    // Calculate movement
    if (~cnt->opt & NEKO_GUI_OPT_NOTITLE && new_frame) {
        neko_gui_rect_t* rp = root_split ? &root_split->rect : &cnt->rect;

        // Cache rect
        neko_gui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        tr.x += split_size;
        neko_gui_id id = neko_gui_get_id(ctx, "!title", 6);
        neko_gui_update_control(ctx, id, tr, opt);

        // Need to move the entire thing
        if ((id == ctx->focus || id == ctx->hover) && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
            // This lock id is what I need...

            ctx->active_root = cnt;

            if (tab_bar) {
                ctx->next_focus_root = (neko_gui_container_t*)(tab_bar->items[tab_bar->focus].data);
                neko_gui_bring_to_front(ctx, (neko_gui_container_t*)tab_bar->items[tab_bar->focus].data);
                if (id == ctx->focus && tab_bar->focus != tab_item->idx) ctx->lock_focus = id;
            } else {
                ctx->next_focus_root = cnt;
            }

            if (root_split) {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }

        // Tab view
        s32 tw = title_max_size;
        id = neko_gui_get_id(ctx, "!split_tab", 10);
        const float hp = 0.8f;
        tr.x += split_size;
        float h = tr.h * hp;
        float y = tr.y + tr.h * (1.f - hp);

        // Will update tab bar rect size with parent window rect
        if (tab_item) {
            // Get tab bar
            neko_gui_rect_t* r = &tab_bar->rect;

            // Determine width
            s32 tab_width = (s32)neko_min(r->w / (float)tab_bar->size, title_max_size);
            tw = tab_item->zindex ? (s32)tab_width : (s32)(tab_width + 1.f);

            // Determine position (based on zindex and total width)
            float xoff = 0.f;  // tab_item->zindex ? 2.f : 0.f;
            tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
        }

        neko_gui_rect_t r = neko_gui_rect(tr.x + split_size, y, (f32)tw, h);

        neko_gui_update_control(ctx, id, r, opt);

        // Need to move the entire thing
        if ((id == ctx->hover || id == ctx->focus) && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
            neko_gui_set_focus(ctx, id);
            ctx->next_focus_root = cnt;
            ctx->active_root = cnt;

            // Don't move from tab bar
            if (tab_item) {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h) {
                    ctx->undock_root = cnt;
                }

                if (tab_bar->focus != tab_item->idx) {
                    neko_gui_request_t req = neko_default_val();
                    req.type = NEKO_GUI_CNT_FOCUS;
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
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            }
        }
    }

    // Control frame for body movement
    if (~root_cnt->opt & NEKO_GUI_OPT_NOMOVE && ~cnt->opt & NEKO_GUI_OPT_NOMOVE && ~cnt->opt & NEKO_GUI_OPT_NOINTERACT && ~cnt->opt & NEKO_GUI_OPT_NOHOVER && new_frame &&
        cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        // Cache rect
        neko_gui_rect_t br = cnt->rect;

        if (~cnt->opt & NEKO_GUI_OPT_NOTITLE) {
            br.y += ctx->style->title_height;
            br.h -= ctx->style->title_height;
        }
        neko_gui_id id = neko_gui_get_id(ctx, "!body", 5);
        // neko_gui_update_control(ctx, id, br, (opt | NEKO_GUI_OPT_NOSWITCHSTATE));

        // Need to move the entire thing
        if (ctx->hover_root == cnt && !ctx->focus_split && !ctx->focus && !ctx->lock_focus && !ctx->hover && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            ctx->next_focus_root = cnt;
            if (root_split) {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_SPLIT_MOVE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else if (tab_bar) {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_CNT_FOCUS;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);

                req.type = NEKO_GUI_CNT_MOVE;
                req.cnt = cnt;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_CNT_MOVE;
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

    if (~opt & NEKO_GUI_OPT_NOTITLE) {
        neko_gui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }
        body.y += tr.h;
        body.h -= tr.h;
    }

    s32 zindex = INT32_MAX - 1;
    if (root_split) {
        neko_gui_get_split_lowest_zindex(ctx, root_split, &zindex);
        if (zindex == cnt->zindex) {
            neko_gui_style_t* style = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][state];
            neko_gui_draw_rect(ctx, root_split->rect, style->colors[NEKO_GUI_COLOR_BACKGROUND]);
            neko_gui_draw_splits(ctx, root_split);
        }
    }

    // draw body frame
    if (~opt & NEKO_GUI_OPT_NOFRAME && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        neko_gui_style_t* style = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][state];

        if (ctx->active_root == root_cnt) {
            s32 f = 0;
        }

        neko_gui_draw_rect(ctx, body, style->colors[NEKO_GUI_COLOR_BACKGROUND]);

        // draw border (get root cnt and check state of that)
        if (split) {
            s32 root_state = ctx->active_root == root_cnt ? NEKO_GUI_ELEMENT_STATE_FOCUS : ctx->hover_root == root_cnt ? NEKO_GUI_ELEMENT_STATE_HOVER : NEKO_GUI_ELEMENT_STATE_DEFAULT;

            bool share_split = ctx->active_root && neko_gui_get_root_container(ctx, ctx->active_root) == root_cnt ? true : false;

            // Have to look and see if hovered root shares split...
            neko_gui_style_t* root_style = style;
            if (share_split) {
                root_style = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][NEKO_GUI_ELEMENT_STATE_FOCUS];
            } else {
                root_style = state == NEKO_GUI_ELEMENT_STATE_FOCUS        ? style
                             : root_state == NEKO_GUI_ELEMENT_STATE_FOCUS ? &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][root_state]
                             : root_state == NEKO_GUI_ELEMENT_STATE_HOVER ? &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][root_state]
                                                                          : style;
            }
            if (~opt & NEKO_GUI_OPT_NOBORDER && root_style->colors[NEKO_GUI_COLOR_BORDER].a) {
                neko_gui_draw_box(ctx, neko_gui_expand_rect(split->rect, (s16*)root_style->border_width), (s16*)root_style->border_width, root_style->colors[NEKO_GUI_COLOR_BORDER]);
            }
        } else {
            if (~opt & NEKO_GUI_OPT_NOBORDER && style->colors[NEKO_GUI_COLOR_BORDER].a) {
                neko_gui_draw_box(ctx, neko_gui_expand_rect(cnt->rect, (s16*)style->border_width), (s16*)style->border_width, style->colors[NEKO_GUI_COLOR_BORDER]);
            }
        }
    }

    if (split && ~opt & NEKO_GUI_OPT_NOCLIP && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        s16 exp[] = {1, 1, 1, 1};
        neko_gui_push_clip_rect(ctx, neko_gui_expand_rect(cnt->rect, exp));
    }

    if (split) {
        const float sh = split_size * 0.5f;
        body.x += sh;
        body.w -= split_size;
    }

    // do title bar
    if (~opt & NEKO_GUI_OPT_NOTITLE) {
        neko_gui_style_t* cstyle = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][state];
        neko_gui_rect_t tr = cnt->rect;
        tr.h = ctx->style->title_height;
        if (split) {
            const float sh = split_size * 0.5f;
        }

        // Don't draw this unless you're the bottom window or first frame in a tab group (if in dockspace)
        if (tab_bar) {
            bool lowest = true;
            {
                for (u32 i = 0; i < tab_bar->size; ++i) {
                    if (cnt->zindex > ((neko_gui_container_t*)(tab_bar->items[i].data))->zindex) {
                        lowest = false;
                        break;
                    }
                }
                if (lowest) {
                    neko_gui_draw_frame(ctx, tr, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_PANEL][0x00]);
                    // neko_gui_draw_box(ctx, neko_gui_expand_rect(tr, (s16*)cstyle->border_width), (s16*)cstyle->border_width, cstyle->colors[NEKO_GUI_COLOR_BORDER]);
                }
            }
        }

        else {
            neko_gui_draw_frame(ctx, tr, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_PANEL][0x00]);
            // neko_gui_draw_box(ctx, neko_gui_expand_rect(tr, (s16*)cstyle->border_width), cstyle->border_width, cstyle->colors[NEKO_GUI_COLOR_BORDER]);
        }

        // Draw tab control
        {

            // Tab view
            s32 tw = title_max_size;
            id = neko_gui_get_id(ctx, "!split_tab", 10);
            const float hp = 0.8f;
            tr.x += split_size;
            float h = tr.h * hp;
            float y = tr.y + tr.h * (1.f - hp);

            // Will update tab bar rect size with parent window rect
            if (tab_item) {
                // Get tab bar
                neko_gui_rect_t* r = &tab_bar->rect;

                // Determine width
                s32 tab_width = (s32)neko_min(r->w / (float)tab_bar->size, title_max_size);
                tw = (s32)(tab_width - 2.f);

                // Determine position (based on zindex and total width)
                float xoff = !tab_item->zindex ? split_size : 2.f;  // tab_item->zindex ? 2.f : 0.f;
                tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
            }

            neko_gui_rect_t r = neko_gui_rect(tr.x + split_size, y, (f32)tw, h);

            bool hovered = false;

            if (in_root && neko_gui_rect_overlaps_vec2(r, ctx->mouse_pos)) {
                neko_immediate_draw_t* dl = &ctx->overlay_draw_list;
                // neko_idraw_rectvd(dl, neko_v2(r.x, r.y), neko_v2(r.w, r.h), neko_v2s(0.f), neko_v2s(1.f), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);
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

            if (!other_root_active && hovered && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT && !ctx->lock_focus) {
                // This is an issue...
                neko_gui_set_focus(ctx, id);
                ctx->lock_focus = id;

                if (tab_item && tab_bar->focus != tab_item->idx) {
                    neko_gui_request_t req = neko_default_val();
                    req.type = NEKO_GUI_CNT_FOCUS;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            if (!other_root_active && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT && ctx->focus == id) {
                if (ctx->mouse_pos.x < r.x) {
                    neko_gui_request_t req = neko_default_val();
                    req.type = NEKO_GUI_TAB_SWAP_LEFT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
                if (ctx->mouse_pos.x > r.x + r.w) {
                    neko_gui_request_t req = neko_default_val();
                    req.type = NEKO_GUI_TAB_SWAP_RIGHT;
                    req.cnt = cnt;
                    neko_dyn_array_push(ctx->requests, req);
                }
            }

            bool tab_focus = (!tab_bar || (tab_bar && tab_item && tab_bar->focus == tab_item->idx));

            neko_color_t def = ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x00].colors[NEKO_GUI_COLOR_BACKGROUND];
            neko_color_t hov = ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x01].colors[NEKO_GUI_COLOR_BACKGROUND];
            neko_color_t act = ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x02].colors[NEKO_GUI_COLOR_BACKGROUND];
            neko_color_t inactive = neko_color(10, 10, 10, 50);

            s16 exp[] = {1, 1, 1, 1};
            neko_gui_push_clip_rect(ctx, neko_gui_expand_rect(cnt->rect, exp));

            neko_gui_push_clip_rect(ctx, r);

            neko_gui_draw_rect(ctx, r, id == ctx->focus ? act : hovered ? hov : tab_focus ? def : inactive);
            neko_gui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][state], opt);

            neko_gui_pop_clip_rect(ctx);
            neko_gui_pop_clip_rect(ctx);
        }

        // do `close` button
        /*
        if (~opt & NEKO_GUI_OPT_NOCLOSE && false)
        {
            neko_gui_id id = neko_gui_get_id(ctx, "!close", 6);
            neko_gui_rect_t r = neko_gui_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
            tr.w -= r.w;
            neko_gui_draw_icon(ctx, NEKO_GUI_ICON_CLOSE, r, ctx->style->colors[NEKO_GUI_COLOR_TITLETEXT]);
            neko_gui_update_control(ctx, id, r, opt);
            if (ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT && id == ctx->focus)
            {
                cnt->open = 0;
            }
        }
        */
    }

    // resize to content size
    if (opt & NEKO_GUI_OPT_AUTOSIZE && !split) {
        /*
        neko_gui_rect_t r = neko_gui_get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
        cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
        */
    }

    if (split && ~opt & NEKO_GUI_OPT_NOCLIP && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        neko_gui_pop_clip_rect(ctx);
    }

    // Draw border
    if (~opt & NEKO_GUI_OPT_NOFRAME && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        const int* w = (int*)ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][0x00].border_width;
        const neko_color_t* bc = &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_CONTAINER][0x00].colors[NEKO_GUI_COLOR_BORDER];
        // neko_gui_draw_box(ctx, neko_gui_expand_rect(cnt->rect, w), w, *bc);
    }

    neko_gui_push_container_body(ctx, cnt, body, desc, opt);

    /* close if this is a popup window and elsewhere was clicked */
    if (opt & NEKO_GUI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) {
        cnt->open = 0;
    }

    if (~opt & NEKO_GUI_OPT_NOCLIP) {
        if (cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
            neko_gui_push_clip_rect(ctx, cnt->body);
        } else {
            neko_gui_push_clip_rect(ctx, neko_gui_rect(0, 0, 0, 0));
        }
    }

    return NEKO_GUI_RES_ACTIVE;
}

NEKO_API_DECL void neko_gui_window_end(neko_gui_context_t* ctx) {
    neko_gui_container_t* cnt = neko_gui_get_current_container(ctx);

    // Get root container
    neko_gui_container_t* root_cnt = neko_gui_get_root_container(ctx, cnt);

    // Get splits
    neko_gui_split_t* split = neko_gui_get_split(ctx, cnt);
    neko_gui_split_t* root_split = neko_gui_get_root_split(ctx, cnt);

    const bool new_frame = cnt->frame != ctx->frame;

    // Cache opt
    const u64 opt = cnt->opt;

    // Pop clip for rect
    if (~cnt->opt & NEKO_GUI_OPT_NOCLIP) {
        neko_gui_pop_clip_rect(ctx);
    }

    if (~cnt->opt & NEKO_GUI_OPT_NOCLIP) {
        neko_gui_push_clip_rect(ctx, cnt->rect);
    }

    // do `resize` handle
    if (~cnt->opt & NEKO_GUI_OPT_NORESIZE && ~root_cnt->opt & NEKO_GUI_OPT_NORESIZE && new_frame && ~cnt->opt & NEKO_GUI_OPT_DOCKSPACE) {
        s32 sz = ctx->style->title_height;
        neko_gui_id id = neko_gui_get_id(ctx, "!resize", 7);
        neko_gui_rect_t r = neko_gui_rect(cnt->rect.x + cnt->rect.w - (f32)sz, cnt->rect.y + cnt->rect.h - (f32)sz, (f32)sz, (f32)sz);
        neko_gui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == NEKO_GUI_MOUSE_LEFT) {
            ctx->active_root = cnt;
            if (root_split) {
                neko_gui_request_t req = neko_default_val();
                req.type = NEKO_GUI_SPLIT_RESIZE_SE;
                req.split = root_split;
                neko_dyn_array_push(ctx->requests, req);
            } else {
                cnt->rect.w = neko_max(96, cnt->rect.w + ctx->mouse_delta.x);
                cnt->rect.h = neko_max(64, cnt->rect.h + ctx->mouse_delta.y);
            }
        }

        // Draw resize icon (this will also be a callback)
        const u32 grid = 5;
        const float w = r.w / (float)grid;
        const float h = r.h / (float)grid;
        const float m = 2.f;
        const float o = 5.f;

        neko_color_t col = ctx->focus == id   ? ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x02].colors[NEKO_GUI_COLOR_BACKGROUND]
                           : ctx->hover == id ? ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x01].colors[NEKO_GUI_COLOR_BACKGROUND]
                                              : ctx->style_sheet->styles[NEKO_GUI_ELEMENT_BUTTON][0x00].colors[NEKO_GUI_COLOR_BACKGROUND];

        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * grid - o, r.y + h * (grid - 2) - o, w - m, h - m), col);
        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * grid - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * (grid - 1) - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * grid - o, r.y + h * grid - o, w - m, h - m), col);
        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * (grid - 1) - o, r.y + h * grid - o, w - m, h - m), col);
        neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w * (grid - 2) - o, r.y + h * grid - o, w - m, h - m), col);
    }

    if (~cnt->opt & NEKO_GUI_OPT_NOCLIP) {
        neko_gui_pop_clip_rect(ctx);
    }

    // draw shadow
    if (~opt & NEKO_GUI_OPT_NOFRAME && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        neko_gui_rect_t* r = &cnt->rect;
        u32 ssz = (u32)(split ? NEKO_GUI_SPLIT_SIZE : 5);

        neko_gui_draw_rect(ctx, neko_gui_rect(r->x, r->y + r->h, r->w + 1, 1), ctx->style->colors[NEKO_GUI_COLOR_SHADOW]);
        neko_gui_draw_rect(ctx, neko_gui_rect(r->x, r->y + r->h, r->w + (f32)ssz, (f32)ssz), ctx->style->colors[NEKO_GUI_COLOR_SHADOW]);
        neko_gui_draw_rect(ctx, neko_gui_rect(r->x + r->w, r->y, 1, r->h), ctx->style->colors[NEKO_GUI_COLOR_SHADOW]);
        neko_gui_draw_rect(ctx, neko_gui_rect(r->x + r->w, r->y, (f32)ssz, r->h), ctx->style->colors[NEKO_GUI_COLOR_SHADOW]);
    }

#define _gui_window_resize_ctrl(ID, RECT, MOUSE, SPLIT_TYPE, MOD_KEY, ...) \
    do {                                                                   \
        if (ctx->key_down == (MOD_KEY)) {                                  \
            neko_gui_id _ID = (ID);                                        \
            neko_gui_rect_t _R = (RECT);                                   \
            neko_gui_update_control(ctx, (ID), _R, opt);                   \
                                                                           \
            if (_ID == ctx->hover || _ID == ctx->focus) {                  \
                neko_gui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);             \
            }                                                              \
                                                                           \
            if (_ID == ctx->focus && ctx->mouse_down == (MOUSE)) {         \
                neko_gui_draw_rect(ctx, _R, NEKO_COLOR_WHITE);             \
                if (root_split) {                                          \
                    neko_gui_request_t req = neko_default_val();           \
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
    if (~opt & NEKO_GUI_OPT_NORESIZE && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        // Cache main rect
        neko_gui_rect_t* r = root_split ? &root_split->rect : &cnt->rect;
        neko_gui_rect_t* cr = &cnt->rect;

        const float border_ratio = 0.333f;

        if (split) {
            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_w", 8), neko_gui_rect(cr->x, cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)),
                                    NEKO_GUI_MOUSE_RIGHT, NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_e", 8),
                                    neko_gui_rect(cr->x + cr->w * (1.f - border_ratio), cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), NEKO_GUI_MOUSE_LEFT,
                                    NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_n", 8), neko_gui_rect(cr->x + cr->w * border_ratio, cr->y, cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio),
                                    NEKO_GUI_MOUSE_LEFT, NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_s", 8),
                                    neko_gui_rect(cr->x + cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                    NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_se", 9),
                                    neko_gui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * border_ratio, cr->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                    NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_ne", 9), neko_gui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y, cr->w * border_ratio, cr->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                    NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_nw", 9), neko_gui_rect(cr->x, cr->y, cr->w * border_ratio, cr->h * border_ratio), NEKO_GUI_MOUSE_LEFT, NEKO_GUI_SPLIT_RESIZE_INVALID,
                                    NEKO_GUI_KEY_CTRL, {});

            _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!split_sw", 9), neko_gui_rect(cr->x, cr->y + cr->h - cr->h * border_ratio, cr->w * border_ratio, cr->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                    NEKO_GUI_SPLIT_RESIZE_INVALID, NEKO_GUI_KEY_CTRL, {});
        }

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_w", 6), neko_gui_rect(r->x, r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_W, NEKO_GUI_KEY_ALT, {
                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_e", 6),
                                neko_gui_rect(r->x + r->w * (1.f - border_ratio), r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_E, NEKO_GUI_KEY_ALT, { r->w = neko_max(r->w + ctx->mouse_delta.x, 40); });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_n", 6), neko_gui_rect(r->x + r->w * border_ratio, r->y, r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_N, NEKO_GUI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_s", 6),
                                neko_gui_rect(r->x + r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_S, NEKO_GUI_KEY_ALT, { r->h = neko_max(r->h + ctx->mouse_delta.y, 40); });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_se", 7), neko_gui_rect(r->x + r->w - r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * border_ratio, r->h * border_ratio),
                                NEKO_GUI_MOUSE_LEFT, NEKO_GUI_SPLIT_RESIZE_SE, NEKO_GUI_KEY_ALT, {
                                    r->w = neko_max(r->w + ctx->mouse_delta.x, 40);
                                    r->h = neko_max(r->h + ctx->mouse_delta.y, 40);
                                });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_ne", 7), neko_gui_rect(r->x + r->w - r->w * border_ratio, r->y, r->w * border_ratio, r->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_NE, NEKO_GUI_KEY_ALT, {
                                    r->w = neko_max(r->w + ctx->mouse_delta.x, 40);
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                                    }
                                });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_nw", 7), neko_gui_rect(r->x, r->y, r->w * border_ratio, r->h * border_ratio), NEKO_GUI_MOUSE_LEFT, NEKO_GUI_SPLIT_RESIZE_NW,
                                NEKO_GUI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                                    if (fabsf(r->h - h) > 0.f) {
                                        r->y = neko_min(r->y + ctx->mouse_delta.y, max_y);
                                    }

                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        _gui_window_resize_ctrl(neko_gui_get_id(ctx, "!res_sw", 7), neko_gui_rect(r->x, r->y + r->h - r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), NEKO_GUI_MOUSE_LEFT,
                                NEKO_GUI_SPLIT_RESIZE_SW, NEKO_GUI_KEY_ALT, {
                                    float h = r->h;
                                    float max_y = h + r->y;
                                    r->h = neko_max(r->h + ctx->mouse_delta.y, 40);

                                    float w = r->w;
                                    float max_x = r->x + r->w;
                                    r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                                    if (fabsf(r->w - w) > 0.f) {
                                        r->x = neko_min(r->x + ctx->mouse_delta.x, max_x);
                                    }
                                });

        static bool capture = false;
        static neko_vec2 mp = {0};
        static neko_gui_rect_t _rect = {0};

        /*
        _gui_window_resize_ctrl(
            neko_gui_get_id(ctx, "!res_c", 5),
            neko_gui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio),
            NEKO_GUI_SPLIT_RESIZE_CENTER,
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
                r->h = neko_max(r->h - ctx->mouse_delta.y, 40);
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = neko_min(r->y - ctx->mouse_delta.y, max_y);
                }

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = neko_max(r->w - ctx->mouse_delta.x, 40);
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = neko_min(r->x - ctx->mouse_delta.x, max_x);
                }
            });
        */

        if (ctx->mouse_down != NEKO_GUI_MOUSE_LEFT) {
            capture = false;
            mp = neko_v2s(0.f);
        }
    }

    // Determine if focus root in same tab group as current window for docking
    bool can_dock = true;
    if (cnt->tab_bar) {
        neko_gui_tab_bar_t* tab_bar = neko_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
        for (u32 t = 0; t < tab_bar->size; ++t) {
            if (tab_bar->items[t].data == ctx->focus_root) {
                can_dock = false;
            }
        }
    }

    // Do docking overlay (if enabled)
    if (can_dock && ~cnt->opt & NEKO_GUI_OPT_NODOCK && ctx->focus_root && ctx->focus_root != cnt &&
        neko_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) &&  // This is the incorrect part - need to check if this container isn't being overlapped by another
        ctx->mouse_down == NEKO_GUI_MOUSE_LEFT && ~cnt->opt & NEKO_GUI_OPT_NOHOVER && cnt->flags & NEKO_GUI_WINDOW_FLAGS_VISIBLE) {
        neko_gui_split_t* focus_split = neko_gui_get_root_split(ctx, ctx->focus_root);
        neko_gui_split_t* cnt_split = neko_gui_get_root_split(ctx, cnt);

        // NOTE: this is incorrect...
        if ((!focus_split && !cnt_split) || ((focus_split || cnt_split) && (focus_split != cnt_split))) {
            // Set dockable root container
            ctx->dockable_root = ctx->dockable_root && cnt->zindex > ctx->dockable_root->zindex ? cnt : ctx->dockable_root ? ctx->dockable_root : cnt;
        }
    }

    // Set current frame
    cnt->frame = ctx->frame;

    // Pop root container
    neko_gui_root_container_end(ctx);
}

NEKO_API_DECL void neko_gui_popup_open(neko_gui_context_t* ctx, const char* name) {
    neko_gui_container_t* cnt = neko_gui_get_container(ctx, name);

    // Set as hover root so popup isn't closed in window_begin_ex()
    ctx->hover_root = ctx->next_hover_root = cnt;

    // position at mouse cursor, open and bring-to-front
    cnt->rect = neko_gui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100);
    cnt->open = 1;
    neko_gui_bring_to_front(ctx, cnt);
}

NEKO_API_DECL s32 neko_gui_popup_begin_ex(neko_gui_context_t* ctx, const char* name, neko_gui_rect_t r, const neko_gui_selector_desc_t* desc, u64 opt) {
    opt |= (NEKO_GUI_OPT_POPUP | NEKO_GUI_OPT_NODOCK | NEKO_GUI_OPT_CLOSED);
    return neko_gui_window_begin_ex(ctx, name, r, NULL, NULL, opt);
}

NEKO_API_DECL void neko_gui_popup_end(neko_gui_context_t* ctx) { neko_gui_window_end(ctx); }

NEKO_API_DECL void neko_gui_panel_begin_ex(neko_gui_context_t* ctx, const char* name, const neko_gui_selector_desc_t* desc, u64 opt) {
    neko_gui_container_t* cnt;
    const s32 elementid = NEKO_GUI_ELEMENT_PANEL;
    char id_tag[256] = neko_default_val();
    neko_gui_parse_id_tag(ctx, name, id_tag, sizeof(id_tag));

    // if (id_tag) neko_gui_push_id(ctx, id_tag, strlen(id_tag));
    // else neko_gui_push_id(ctx, name, strlen(name));
    neko_gui_push_id(ctx, name, strlen(name));
    cnt = neko_gui_get_container_ex(ctx, ctx->last_id, opt);
    cnt->rect = neko_gui_layout_next(ctx);

    const neko_gui_id id = neko_gui_get_id(ctx, name, strlen(name));

    neko_gui_style_t style = neko_default_val();
    neko_gui_animation_t* anim = neko_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this)
    if (anim) {
        neko_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = neko_gui_animation_get_blend_style(ctx, anim, desc, elementid);
    } else {
        style = ctx->focus == id   ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x02)
                : ctx->hover == id ? neko_gui_get_current_element_style(ctx, desc, elementid, 0x01)
                                   : neko_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    }

    if (~opt & NEKO_GUI_OPT_NOFRAME) {
        neko_gui_draw_frame(ctx, cnt->rect, &style);
    }

    // Need a way to push/pop syles temp styles
    neko_gui_stack_push(ctx->container_stack, cnt);
    neko_gui_push_container_body(ctx, cnt, cnt->rect, desc, opt);
    neko_gui_push_clip_rect(ctx, cnt->body);
}

NEKO_API_DECL void neko_gui_panel_end(neko_gui_context_t* ctx) {
    neko_gui_pop_clip_rect(ctx);
    neko_gui_pop_container(ctx);
}

static u8 uint8_slider(neko_gui_context_t* ctx, unsigned char* value, int low, int high, const neko_gui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    neko_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = neko_gui_slider_ex(ctx, &tmp, (neko_gui_real)low, (neko_gui_real)high, 0, "%.0f", desc, opt);
    *value = (u8)tmp;
    neko_gui_pop_id(ctx);
    return res;
}

static s32 int32_slider(neko_gui_context_t* ctx, s32* value, s32 low, s32 high, const neko_gui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    neko_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = neko_gui_slider_ex(ctx, &tmp, (neko_gui_real)low, (neko_gui_real)high, 0, "%.0f", desc, opt);
    *value = (s32)tmp;
    neko_gui_pop_id(ctx);
    return res;
}

static s16 int16_slider(neko_gui_context_t* ctx, s16* value, s32 low, s32 high, const neko_gui_selector_desc_t* desc, u64 opt) {
    static float tmp;
    neko_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = neko_gui_slider_ex(ctx, &tmp, (neko_gui_real)low, (neko_gui_real)high, 0, "%.0f", desc, opt);
    *value = (s16)tmp;
    neko_gui_pop_id(ctx);
    return res;
}

//=== Gizmo ===//

typedef struct {
    b32 hit;
    neko_vec3 point;
} neko_gui_gizmo_line_intersection_result_t;

enum { NEKO_GUI_AXIS_RIGHT = 0x01, NEKO_GUI_AXIS_UP, NEKO_GUI_AXIS_FORWARD };

typedef struct {
    struct {
        neko_vqs model;
        union {
            neko_cylinder_t cylinder;
            neko_plane_t plane;
        } shape;
    } axis;
    struct {
        neko_vqs model;
        union {
            neko_cone_t cone;
            neko_aabb_t aabb;
        } shape;
    } cap;
} neko_gui_axis_t;

typedef struct {
    neko_gui_axis_t right;
    neko_gui_axis_t up;
    neko_gui_axis_t forward;
} neko_gizmo_translate_t;

typedef struct {
    neko_gui_axis_t right;
    neko_gui_axis_t up;
    neko_gui_axis_t forward;
} neko_gizmo_scale_t;

typedef struct {
    neko_gui_axis_t right;
    neko_gui_axis_t up;
    neko_gui_axis_t forward;
} neko_gizmo_rotate_t;

static neko_gizmo_scale_t neko_gizmo_scale(const neko_vqs* parent) {
    neko_gizmo_scale_t gizmo = neko_default_val();
    const neko_vec3 ax_scl = neko_v3(0.03f, 1.f, 0.03f);
    const neko_vec3 cap_scl = neko_vec3_scale(neko_v3s(0.05f), 2.f * neko_vec3_len(parent->scale));
    neko_vqs local = neko_vqs_default();
    neko_vqs abs = neko_vqs_default();

#define NEKO_GUI_GIZMO_AXIS_DEFINE_SCALE(MEMBER, OFFSET, DEG, AXIS)                                 \
    do {                                                                                            \
        /* Axis */                                                                                  \
        {                                                                                           \
            local = neko_vqs_ctor(OFFSET, neko_quat_angle_axis(neko_deg2rad(DEG), AXIS), ax_scl);   \
            gizmo.MEMBER.axis.model = neko_vqs_absolute_transform(&local, parent);                  \
            neko_cylinder_t axis = neko_default_val();                                              \
            axis.r = 1.f;                                                                           \
            axis.base = neko_v3(0.f, 0.0f, 0.f);                                                    \
            axis.height = 1.f;                                                                      \
            gizmo.MEMBER.axis.shape.cylinder = axis;                                                \
        }                                                                                           \
                                                                                                    \
        /* Cap */                                                                                   \
        {                                                                                           \
            local = neko_vqs_ctor(neko_v3(0.f, 0.5f, 0.f), neko_quat_default(), neko_v3s(1.f));     \
                                                                                                    \
            gizmo.MEMBER.cap.model = neko_vqs_absolute_transform(&local, &gizmo.MEMBER.axis.model); \
            gizmo.MEMBER.cap.model.scale = cap_scl;                                                 \
            neko_aabb_t aabb = neko_default_val();                                                  \
            aabb.min = neko_v3s(-0.5f);                                                             \
            aabb.max = neko_v3s(0.5f);                                                              \
            gizmo.MEMBER.cap.shape.aabb = aabb;                                                     \
        }                                                                                           \
    } while (0)

    const float off = 0.6f;
    NEKO_GUI_GIZMO_AXIS_DEFINE_SCALE(right, neko_v3(-off, 0.f, 0.f), 90.f, NEKO_ZAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_SCALE(up, neko_v3(0.f, off, 0.f), 0.f, NEKO_YAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_SCALE(forward, neko_v3(0.f, 0.f, off), 90.f, NEKO_XAXIS);

    return gizmo;
}

static neko_gizmo_translate_t neko_gizmo_translate(const neko_vqs* parent) {
    neko_gizmo_translate_t trans = neko_default_val();
    const neko_vec3 ax_scl = neko_v3(0.03f, 1.f, 0.03f);
    const neko_vec3 cap_scl = neko_vec3_scale(neko_v3(0.02f, 0.05f, 0.02f), 2.f * neko_vec3_len(parent->scale));
    neko_vqs local = neko_vqs_default();
    neko_vqs abs = neko_vqs_default();

#define NEKO_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(MEMBER, OFFSET, DEG, AXIS)                             \
    do {                                                                                            \
        /* Axis */                                                                                  \
        {                                                                                           \
            local = neko_vqs_ctor(OFFSET, neko_quat_angle_axis(neko_deg2rad(DEG), AXIS), ax_scl);   \
            trans.MEMBER.axis.model = neko_vqs_absolute_transform(&local, parent);                  \
            neko_cylinder_t axis = neko_default_val();                                              \
            axis.r = 1.f;                                                                           \
            axis.base = neko_v3(0.f, 0.0f, 0.f);                                                    \
            axis.height = 1.f;                                                                      \
            trans.MEMBER.axis.shape.cylinder = axis;                                                \
        }                                                                                           \
                                                                                                    \
        /* Cap */                                                                                   \
        {                                                                                           \
            local = neko_vqs_ctor(neko_v3(0.f, 0.5f, 0.f), neko_quat_default(), neko_v3s(1.f));     \
                                                                                                    \
            trans.MEMBER.cap.model = neko_vqs_absolute_transform(&local, &trans.MEMBER.axis.model); \
            trans.MEMBER.cap.model.scale = cap_scl;                                                 \
            neko_cone_t cap = neko_default_val();                                                   \
            cap.r = 1.f;                                                                            \
            cap.base = neko_v3(0.f, 0.0f, 0.f);                                                     \
            cap.height = 1.0f;                                                                      \
            trans.MEMBER.cap.shape.cone = cap;                                                      \
        }                                                                                           \
    } while (0)

    const float off = 0.6f;
    NEKO_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(right, neko_v3(-off, 0.f, 0.f), 90.f, NEKO_ZAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(up, neko_v3(0.f, off, 0.f), 0.f, NEKO_YAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(forward, neko_v3(0.f, 0.f, off), 90.f, NEKO_XAXIS);

    return trans;
}

static neko_gizmo_rotate_t neko_gizmo_rotate(const neko_vqs* parent) {
    neko_gizmo_rotate_t gizmo = neko_default_val();
    const neko_vec3 ax_scl = neko_v3(1.f, 1.f, 1.f);
    neko_vqs local = neko_vqs_default();
    neko_vqs abs = neko_vqs_default();

#define NEKO_GUI_GIZMO_AXIS_DEFINE_ROTATE(MEMBER, OFFSET, DEG, AXIS)                              \
    do {                                                                                          \
        /* Axis */                                                                                \
        {                                                                                         \
            local = neko_vqs_ctor(OFFSET, neko_quat_angle_axis(neko_deg2rad(DEG), AXIS), ax_scl); \
            gizmo.MEMBER.axis.model = neko_vqs_absolute_transform(&local, parent);                \
            neko_plane_t axis = neko_plane_from_pt_normal(neko_v3s(0.f), NEKO_ZAXIS);             \
            gizmo.MEMBER.axis.shape.plane = axis;                                                 \
        }                                                                                         \
    } while (0)

    NEKO_GUI_GIZMO_AXIS_DEFINE_ROTATE(right, neko_v3(0.f, 0.f, 0.f), 90.f, NEKO_YAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_ROTATE(up, neko_v3(0.f, 0.f, 0.f), -90.f, NEKO_XAXIS);
    NEKO_GUI_GIZMO_AXIS_DEFINE_ROTATE(forward, neko_v3(0.f, 0.f, 0.f), 0.f, NEKO_ZAXIS);

    return gizmo;
}

typedef struct {
    s32 op;
    s32 mode;
    neko_vqs xform;
    neko_contact_info_t info;
    union {
        neko_gizmo_translate_t translate;
        neko_gizmo_scale_t scale;
        neko_gizmo_rotate_t rotate;
    } gizmo;
    s16 hover;
    neko_camera_t camera;
    neko_gui_rect_t viewport;
    neko_gui_gizmo_line_intersection_result_t li;
} neko_gizmo_desc_t;

static void neko_gui_gizmo_render(neko_gui_context_t* ctx, neko_gui_customcommand_t* cmd) {
    const neko_vec2 fbs = ctx->framebuffer_size;
    const float t = neko_platform_elapsed_time();
    neko_immediate_draw_t* gui_idraw = &ctx->gui_idraw;
    neko_gizmo_desc_t* desc = (neko_gizmo_desc_t*)cmd->data;
    neko_gui_rect_t clip = cmd->clip;
    neko_gui_rect_t viewport = desc->viewport;
    neko_camera_t cam = desc->camera;
    const u16 segments = 4;
    const neko_gui_gizmo_line_intersection_result_t* li = &desc->li;
    const neko_contact_info_t* info = &desc->info;
    const u8 alpha = 150;

    neko_idraw_defaults(gui_idraw);
    neko_idraw_depth_enabled(gui_idraw, false);
    neko_idraw_camera(gui_idraw, &cam, (u32)viewport.w, (u32)viewport.h);
    neko_graphics_set_viewport(&gui_idraw->commands, (u32)viewport.x, (u32)(fbs.y - viewport.h - viewport.y), (u32)viewport.w, (u32)viewport.h);
    neko_graphics_primitive_type primitive = NEKO_GRAPHICS_PRIMITIVE_TRIANGLES;

#define NEKO_GUI_GIZMO_AXIS_TRANSLATE(ID, AXIS, COLOR)                                                                                                                           \
    do {                                                                                                                                                                         \
        neko_gui_id id = neko_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);                                                                                                   \
        bool hover = cmd->hover == id;                                                                                                                                           \
        bool focus = cmd->focus == id;                                                                                                                                           \
        neko_color_t color = hover || focus ? NEKO_COLOR_YELLOW : COLOR;                                                                                                         \
        /* Axis */                                                                                                                                                               \
        {                                                                                                                                                                        \
            neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);                                                                                                      \
            neko_idraw_mul_matrix(gui_idraw, neko_vqs_to_mat4(&desc->gizmo.translate.AXIS.axis.model));                                                                          \
            {                                                                                                                                                                    \
                neko_cylinder_t* axis = &desc->gizmo.translate.AXIS.axis.shape.cylinder;                                                                                         \
                neko_idraw_cylinder(gui_idraw, axis->base.x, axis->base.y, axis->base.z, axis->r, axis->r, axis->height, segments, color.r, color.g, color.b, alpha, primitive); \
            }                                                                                                                                                                    \
            neko_idraw_pop_matrix(gui_idraw);                                                                                                                                    \
        }                                                                                                                                                                        \
                                                                                                                                                                                 \
        /* Cap */                                                                                                                                                                \
        {                                                                                                                                                                        \
            neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);                                                                                                      \
            neko_idraw_mul_matrix(gui_idraw, neko_vqs_to_mat4(&desc->gizmo.translate.AXIS.cap.model));                                                                           \
            {                                                                                                                                                                    \
                neko_cone_t* cap = &desc->gizmo.translate.AXIS.cap.shape.cone;                                                                                                   \
                neko_idraw_cone(gui_idraw, cap->base.x, cap->base.y, cap->base.z, cap->r, cap->height, segments, color.r, color.g, color.b, alpha, primitive);                   \
            }                                                                                                                                                                    \
            neko_idraw_pop_matrix(gui_idraw);                                                                                                                                    \
        }                                                                                                                                                                        \
    } while (0)

#define NEKO_GUI_GIZMO_AXIS_SCALE(ID, AXIS, COLOR)                                                                                                                               \
    do {                                                                                                                                                                         \
        neko_gui_id id = neko_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);                                                                                                   \
        bool hover = cmd->hover == id;                                                                                                                                           \
        bool focus = cmd->focus == id;                                                                                                                                           \
        neko_color_t color = hover || focus ? NEKO_COLOR_YELLOW : COLOR;                                                                                                         \
        /* Axis */                                                                                                                                                               \
        {                                                                                                                                                                        \
            neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);                                                                                                      \
            neko_idraw_mul_matrix(gui_idraw, neko_vqs_to_mat4(&desc->gizmo.scale.AXIS.axis.model));                                                                              \
            {                                                                                                                                                                    \
                neko_cylinder_t* axis = &desc->gizmo.scale.AXIS.axis.shape.cylinder;                                                                                             \
                neko_idraw_cylinder(gui_idraw, axis->base.x, axis->base.y, axis->base.z, axis->r, axis->r, axis->height, segments, color.r, color.g, color.b, alpha, primitive); \
            }                                                                                                                                                                    \
            neko_idraw_pop_matrix(gui_idraw);                                                                                                                                    \
        }                                                                                                                                                                        \
                                                                                                                                                                                 \
        /* Cap */                                                                                                                                                                \
        {                                                                                                                                                                        \
            neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);                                                                                                      \
            neko_idraw_mul_matrix(gui_idraw, neko_vqs_to_mat4(&desc->gizmo.scale.AXIS.cap.model));                                                                               \
            {                                                                                                                                                                    \
                neko_aabb_t* cap = &desc->gizmo.scale.AXIS.cap.shape.aabb;                                                                                                       \
                neko_vec3 hd = neko_vec3_scale(neko_vec3_sub(cap->max, cap->min), 0.5f);                                                                                         \
                neko_vec3 c = neko_vec3_add(cap->min, hd);                                                                                                                       \
                neko_idraw_box(gui_idraw, c.x, c.y, c.z, hd.x, hd.y, hd.z, color.r, color.g, color.b, alpha, primitive);                                                         \
            }                                                                                                                                                                    \
            neko_idraw_pop_matrix(gui_idraw);                                                                                                                                    \
        }                                                                                                                                                                        \
    } while (0)

#define NEKO_GUI_GIZMO_AXIS_ROTATE(ID, AXIS, COLOR)                                                                                                     \
    do {                                                                                                                                                \
        neko_color_t def_color = (COLOR);                                                                                                               \
        neko_gui_id id = neko_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);                                                                          \
        bool hover = cmd->hover == id;                                                                                                                  \
        bool focus = cmd->focus == id;                                                                                                                  \
        neko_color_t color = hover || focus ? NEKO_COLOR_YELLOW : def_color;                                                                            \
        /* Axis */                                                                                                                                      \
        {                                                                                                                                               \
            neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);                                                                             \
            neko_idraw_mul_matrix(gui_idraw, neko_vqs_to_mat4(&desc->gizmo.rotate.AXIS.axis.model));                                                    \
            {                                                                                                                                           \
                neko_plane_t* axis = &desc->gizmo.rotate.AXIS.axis.shape.plane;                                                                         \
                neko_idraw_arc(gui_idraw, 0.f, 0.f, 0.92f, 1.f, 0.f, 360.f, 48, color.r, color.g, color.b, color.a, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES); \
            }                                                                                                                                           \
            neko_idraw_pop_matrix(gui_idraw);                                                                                                           \
            if (focus) {                                                                                                                                \
                neko_vec3 ls = desc->xform.translation;                                                                                                 \
                neko_vec3 le = neko_vec3_add(ls, neko_vec3_scale(info->normal, 0.5f));                                                                  \
                neko_idraw_line3Dv(gui_idraw, ls, le, NEKO_COLOR_BLUE);                                                                                 \
            }                                                                                                                                           \
        }                                                                                                                                               \
    } while (0)

    switch (desc->op) {
        case NEKO_GUI_GIZMO_TRANSLATE: {
            NEKO_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_right", right, NEKO_COLOR_RED);
            NEKO_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_up", up, NEKO_COLOR_GREEN);
            NEKO_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_forward", forward, NEKO_COLOR_BLUE);
        } break;

        case NEKO_GUI_GIZMO_SCALE: {
            NEKO_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_right", right, NEKO_COLOR_RED);
            NEKO_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_up", up, NEKO_COLOR_GREEN);
            NEKO_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_forward", forward, NEKO_COLOR_BLUE);
        } break;

        case NEKO_GUI_GIZMO_ROTATE: {
            neko_gui_id id_r = neko_gui_get_id_hash(ctx, "#gizmo_rotate_right", strlen("#gizmo_rotate_right"), cmd->hash);
            neko_gui_id id_u = neko_gui_get_id_hash(ctx, "#gizmo_rotate_up", strlen("#gizmo_rotate_up"), cmd->hash);
            neko_gui_id id_f = neko_gui_get_id_hash(ctx, "#gizmo_rotate_forward", strlen("#gizmo_rotate_forward"), cmd->hash);

            NEKO_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_right", right, (cmd->focus == id_u || cmd->focus == id_f) ? neko_color_alpha(NEKO_COLOR_RED, 25) : neko_color_alpha(NEKO_COLOR_RED, alpha));
            NEKO_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_up", up, (cmd->focus == id_r || cmd->focus == id_f) ? neko_color_alpha(NEKO_COLOR_GREEN, 25) : neko_color_alpha(NEKO_COLOR_GREEN, alpha));
            NEKO_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_forward", forward, (cmd->focus == id_r || cmd->focus == id_u) ? neko_color_alpha(NEKO_COLOR_BLUE, 25) : neko_color_alpha(NEKO_COLOR_BLUE, alpha));
        } break;
    }

    if (li->hit) {
        neko_idraw_sphere(gui_idraw, li->point.x, li->point.y, li->point.z, 0.005f, 255, 0, 0, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
        neko_idraw_line3Dv(gui_idraw, li->point, desc->xform.translation, neko_color(255, 0, 0, 255));
    }
}

static neko_gui_gizmo_line_intersection_result_t neko_gui_gizmo_get_line_intersection(const neko_vqs* model, neko_vec3 axis_a, neko_vec3 axis_b, neko_vec3 axis_c, const neko_camera_t* camera,
                                                                                      const neko_ray_t* ray, neko_vec3 plane_normal_axis, bool compare_supporting_axes, bool override_axis) {
    neko_gui_gizmo_line_intersection_result_t res = neko_default_val();

    // Find absolute dot between cam forward and right axis
    neko_vec3 cf = neko_vec3_norm(neko_camera_forward(camera));
    neko_vec3 ta = neko_vec3_norm(neko_quat_rotate(model->rotation, axis_a));
    float cfdta = fabsf(neko_vec3_dot(cf, ta));

    // This doesn't really make sense. I want to project along the x/y or x/z planes.

    neko_plane_t intersection_plane = neko_default_val();
    neko_vec3 op = model->translation;

    if (compare_supporting_axes) {
        // Now determine appropriate axis to move along
        neko_vec3 tb = neko_vec3_norm(neko_quat_rotate(model->rotation, axis_b));
        neko_vec3 tc = neko_vec3_norm(neko_quat_rotate(model->rotation, axis_c));

        float cfdtb = fabsf(neko_vec3_dot(cf, tb));
        float cfdtc = fabsf(neko_vec3_dot(cf, tc));

        intersection_plane = cfdtb < cfdtc ? neko_plane_from_pt_normal(op, tc) : neko_plane_from_pt_normal(op, tb);
    } else {
        if (override_axis) {
            intersection_plane = neko_plane_from_pt_normal(op, plane_normal_axis);
        } else {
            intersection_plane = neko_plane_from_pt_normal(op, ta);
        }
    }

    // Get line intersection from ray and chosen intersection plane
    neko_plane_t* ip = &intersection_plane;
    float denom = neko_vec3_dot(neko_v3(ip->a, ip->b, ip->c), ray->d);
    if (fabsf(denom) >= neko_epsilon) {
        float t = -(ip->a * ray->p.x + ip->b * ray->p.y + ip->c * ray->p.z + ip->d) / denom;
        res.hit = t >= 0.f ? true : false;
        res.point = neko_vec3_add(ray->p, neko_vec3_scale(ray->d, t));
    }

    return res;
}

static neko_vec3 s_intersection_start = neko_default_val();
static neko_vqs s_delta = neko_default_val();
static bool just_set_focus = false;

NEKO_API_DECL s32 neko_gui_gizmo(neko_gui_context_t* ctx, neko_camera_t* camera, neko_vqs* model, neko_gui_rect_t viewport, bool invert_view_y, float snap, s32 op, s32 mode, u64 opt) {
    s32 res = 0;
    if (model->rotation.w == 0.f) model->rotation = neko_quat_default();
    if (neko_vec3_len(model->scale) == 0.f) model->scale = neko_v3s(1.f);

    const neko_vec2 fbs = ctx->framebuffer_size;
    const float t = neko_platform_elapsed_time();
    const bool in_hover_root = neko_gui_in_hover_root(ctx);

    neko_immediate_draw_t* dl = &ctx->overlay_draw_list;

    // This doesn't actually work for the clip...
    neko_gui_rect_t clip = viewport;
    if (invert_view_y) {
        clip.y = fbs.y - clip.h - clip.y;
    }

    // Capture event information (might have to do something like this for smaller framebuffers scaled to higher viewports)
    /*
    neko_vec2 mc = neko_platform_mouse_positionv();

    // Check for scale and bias
    s32 flags = 0x00;
    if (flags & NEKO_GUI_HINT_FLAG_NO_SCALE_BIAS_MOUSE) {
        float px = (mc.x - clip.x) / clip.w;
        float py = (mc.y - clip.y) / clip.h;
        float xv = clip.w * px;
        float yv = clip.h * py;
        mc = neko_v2(xv, yv);
    }
    else {
        neko_vec2 fb_vp_ratio = neko_v2(fbs.x / clip.w, fbs.y / clip.h);
        float px = mc.x - (clip.x * fb_vp_ratio.x);
        float py = mc.y + (clip.y * fb_vp_ratio.y);
        float xv = px / fb_vp_ratio.x;
        float yv = py / fb_vp_ratio.y;
        mc = neko_v2(xv, yv);
    }
    */

    neko_vec2 mc = neko_platform_mouse_positionv();
    mc = neko_vec2_sub(mc, neko_v2(clip.x, clip.y));

    // Project ray to world
    const float ray_len = 1000.f;
    neko_vec3 ms = neko_v3(mc.x, mc.y, 0.f);
    neko_vec3 me = neko_v3(mc.x, mc.y, -ray_len);
    neko_vec3 ro = neko_camera_screen_to_world(camera, ms, 0, 0, (s32)clip.w, (s32)clip.h);
    neko_vec3 rd = neko_camera_screen_to_world(camera, me, 0, 0, (s32)clip.w, (s32)clip.h);
    rd = neko_vec3_norm(neko_vec3_sub(ro, rd));

    neko_ray_t ray = neko_default_val();
    ray.p = ro;
    ray.d = rd;
    ray.len = ray_len;

    // Check for nan
    if (neko_vec3_nan(ray.p)) ray.p = neko_v3s(0.f);
    if (neko_vec3_nan(ray.d)) ray.d = neko_v3s(0.f);

    neko_gizmo_desc_t desc = neko_default_val();
    desc.op = op;
    desc.mode = mode;
    desc.camera = *camera;
    desc.info.depth = FLT_MAX;
    desc.viewport = clip;

    desc.xform = neko_vqs_default();
    desc.xform.translation = model->translation;
    desc.xform.rotation = (mode == NEKO_GUI_TRANSFORM_LOCAL || op == NEKO_GUI_GIZMO_SCALE) ? model->rotation : neko_quat_default();  // This depends on the mode (local/world)

    switch (camera->proj_type) {
        case NEKO_PROJECTION_TYPE_ORTHOGRAPHIC: {
            desc.xform.scale = neko_v3s(camera->ortho_scale * 0.2f);
        } break;

        case NEKO_PROJECTION_TYPE_PERSPECTIVE: {
            float dist_from_cam = neko_vec3_dist(desc.xform.translation, camera->transform.translation);
            desc.xform.scale = neko_v3s(dist_from_cam * 0.3f);
        } break;
    }

#define UPDATE_GIZMO_CONTROL(ID, RAY, SHAPE, MODEL, FUNC, CAP_SHAPE, CAP_MODEL, CAP_FUNC, INFO)                                                   \
    do {                                                                                                                                          \
        s32 mouseover = 0;                                                                                                                        \
        neko_gui_id id = (ID);                                                                                                                    \
        neko_contact_info_t info0 = neko_default_val();                                                                                           \
        neko_contact_info_t info1 = neko_default_val();                                                                                           \
        if (in_hover_root) {                                                                                                                      \
            FUNC(&(SHAPE), &(MODEL), &(RAY), NULL, &info0);                                                                                       \
            info0.depth = neko_vec3_dist(info0.point, (RAY).p);                                                                                   \
            CAP_FUNC(&(CAP_SHAPE), &(CAP_MODEL), &(RAY), NULL, &info1);                                                                           \
            info1.depth = neko_vec3_dist(info1.point, (RAY).p);                                                                                   \
        }                                                                                                                                         \
        neko_contact_info_t* info = info0.depth < info1.depth ? &info0 : &info1;                                                                  \
        mouseover = info->hit && info->depth <= INFO.depth && in_hover_root && !ctx->hover_split && !ctx->lock_hover_id;                          \
        if (ctx->focus == id) {                                                                                                                   \
            ctx->updated_focus = 1;                                                                                                               \
        }                                                                                                                                         \
        if (~opt & NEKO_GUI_OPT_NOINTERACT) {                                                                                                     \
            /* Check for hold focus here */                                                                                                       \
            if (mouseover && !ctx->mouse_down) {                                                                                                  \
                neko_gui_set_hover(ctx, id);                                                                                                      \
                INFO = *info;                                                                                                                     \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->focus == id) {                                                                                                               \
                res |= NEKO_GUI_RES_ACTIVE;                                                                                                       \
                just_set_focus = false;                                                                                                           \
                neko_gui_set_focus(ctx, id);                                                                                                      \
                if (ctx->mouse_pressed && !mouseover) {                                                                                           \
                    neko_gui_set_focus(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
                if (!ctx->mouse_down && ~opt & NEKO_GUI_OPT_HOLDFOCUS) {                                                                          \
                    neko_gui_set_focus(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->prev_hover == id && !mouseover) {                                                                                            \
                ctx->prev_hover = ctx->hover;                                                                                                     \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->hover == id) {                                                                                                               \
                if (ctx->mouse_pressed) {                                                                                                         \
                    if ((opt & NEKO_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT) || (~opt & NEKO_GUI_OPT_LEFTCLICKONLY)) { \
                        neko_gui_set_focus(ctx, id);                                                                                              \
                        just_set_focus = true;                                                                                                    \
                    }                                                                                                                             \
                } else if (!mouseover) {                                                                                                          \
                    neko_gui_set_hover(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
            }                                                                                                                                     \
        }                                                                                                                                         \
    } while (0)

#ifndef NEKO_PHYSICS_NO_CCD

    switch (op) {
        case NEKO_GUI_GIZMO_TRANSLATE: {
            // Construct translate gizmo for this frame based on given parent transform
            desc.gizmo.translate = neko_gizmo_translate(&desc.xform);

            neko_gui_id id_r = neko_gui_get_id(ctx, "#gizmo_trans_right", strlen("#gizmo_trans_right"));
            neko_gui_id id_u = neko_gui_get_id(ctx, "#gizmo_trans_up", strlen("#gizmo_trans_up"));
            neko_gui_id id_f = neko_gui_get_id(ctx, "#gizmo_trans_forward", strlen("#gizmo_trans_forward"));

            // Right
            UPDATE_GIZMO_CONTROL(id_r, ray, desc.gizmo.translate.right.axis.shape.cylinder, desc.gizmo.translate.right.axis.model, neko_cylinder_vs_ray, desc.gizmo.translate.right.cap.shape.cone,
                                 desc.gizmo.translate.right.cap.model, neko_cone_vs_ray, desc.info);

            // Up
            UPDATE_GIZMO_CONTROL(id_u, ray, desc.gizmo.translate.up.axis.shape.cylinder, desc.gizmo.translate.up.axis.model, neko_cylinder_vs_ray, desc.gizmo.translate.up.cap.shape.cone,
                                 desc.gizmo.translate.up.cap.model, neko_cone_vs_ray, desc.info);

            // Forward
            UPDATE_GIZMO_CONTROL(id_f, ray, desc.gizmo.translate.forward.axis.shape.cylinder, desc.gizmo.translate.forward.axis.model, neko_cylinder_vs_ray,
                                 desc.gizmo.translate.forward.cap.shape.cone, desc.gizmo.translate.forward.cap.model, neko_cone_vs_ray, desc.info);

            // Control
            if (ctx->focus == id_r) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = neko_quat(model->translation.x, model->translation.y, model->translation.z, 0.f);
                }

                if (desc.li.hit) {
                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_XAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    if (neko_vec3_eq(axis, NEKO_XAXIS)) {
                        s_delta.translation.y = 0.f;
                        s_delta.translation.z = 0.f;
                    }
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 op = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = neko_vec3_add(op, delta);
                    } else {
                        // Set final translation
                        model->translation = neko_vec3_add(desc.xform.translation, s_delta.translation);
                    }
                }
            } else if (ctx->focus == id_u) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_YAXIS, NEKO_XAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = neko_quat(model->translation.x, model->translation.y, model->translation.z, 0.f);
                }

                if (desc.li.hit) {
                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_YAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    if (neko_vec3_eq(axis, NEKO_YAXIS)) {
                        s_delta.translation.x = 0.f;
                        s_delta.translation.z = 0.f;
                    }
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 op = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = neko_vec3_add(op, delta);
                    } else {
                        // Set final translation
                        model->translation = neko_vec3_add(desc.xform.translation, s_delta.translation);
                    }
                }
            } else if (ctx->focus == id_f) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_ZAXIS, NEKO_XAXIS, NEKO_YAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = neko_quat(model->translation.x, model->translation.y, model->translation.z, 0.f);
                }

                if (desc.li.hit) {
                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_ZAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    if (neko_vec3_eq(axis, NEKO_ZAXIS)) {
                        s_delta.translation.x = 0.f;
                        s_delta.translation.y = 0.f;
                    }
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 op = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = neko_vec3_add(op, delta);
                    } else {
                        // Set final translation
                        model->translation = neko_vec3_add(desc.xform.translation, s_delta.translation);
                    }
                }
            }

        } break;

        case NEKO_GUI_GIZMO_SCALE: {
            // Construct translate gizmo for this frame based on given parent transform
            desc.gizmo.scale = neko_gizmo_scale(&desc.xform);

            neko_gui_id id_r = neko_gui_get_id(ctx, "#gizmo_scale_right", strlen("#gizmo_scale_right"));
            neko_gui_id id_u = neko_gui_get_id(ctx, "#gizmo_scale_up", strlen("#gizmo_scale_up"));
            neko_gui_id id_f = neko_gui_get_id(ctx, "#gizmo_scale_forward", strlen("#gizmo_scale_forward"));

            // Right
            UPDATE_GIZMO_CONTROL(id_r, ray, desc.gizmo.scale.right.axis.shape.cylinder, desc.gizmo.scale.right.axis.model, neko_cylinder_vs_ray, desc.gizmo.scale.right.cap.shape.aabb,
                                 desc.gizmo.scale.right.cap.model, neko_aabb_vs_ray, desc.info);

            // Up
            UPDATE_GIZMO_CONTROL(id_u, ray, desc.gizmo.scale.up.axis.shape.cylinder, desc.gizmo.scale.up.axis.model, neko_cylinder_vs_ray, desc.gizmo.scale.up.cap.shape.aabb,
                                 desc.gizmo.scale.up.cap.model, neko_aabb_vs_ray, desc.info);

            // Forward
            UPDATE_GIZMO_CONTROL(id_f, ray, desc.gizmo.scale.forward.axis.shape.cylinder, desc.gizmo.scale.forward.axis.model, neko_cylinder_vs_ray, desc.gizmo.scale.forward.cap.shape.aabb,
                                 desc.gizmo.scale.forward.cap.model, neko_aabb_vs_ray, desc.info);

            // Control
            if (ctx->focus == id_r) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = neko_quat(model->scale.x, model->scale.y, model->scale.z, 0.f);
                    }

                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_XAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    float neg = neko_vec3_dot(axis, NEKO_XAXIS) < 0.f ? 1.f : -1.f;
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = neko_vec3_scale(s_delta.translation, neg);
                    s_delta.translation.z = 0.f;
                    s_delta.translation.y = 0.f;
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 os = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = neko_vec3_add(os, delta);
                    } else {
                        model->scale = neko_vec3_add(model->scale, s_delta.translation);
                    }
                }
            } else if (ctx->focus == id_u) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_YAXIS, NEKO_XAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = neko_quat(model->scale.x, model->scale.y, model->scale.z, 0.f);
                    }

                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_YAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    float neg = neko_vec3_dot(axis, NEKO_YAXIS) < 0.f ? -1.f : 1.f;
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = neko_vec3_scale(s_delta.translation, neg);
                    s_delta.translation.z = 0.f;
                    s_delta.translation.x = 0.f;
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 os = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = neko_vec3_add(os, delta);
                    } else {
                        model->scale = neko_vec3_add(model->scale, s_delta.translation);
                    }
                }
            } else if (ctx->focus == id_f) {
                desc.li = neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_ZAXIS, NEKO_XAXIS, NEKO_YAXIS, camera, &ray, neko_v3s(0.f), true, false);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = neko_quat(model->scale.x, model->scale.y, model->scale.z, 0.f);
                    }

                    neko_vec3 axis = neko_vec3_norm(neko_quat_rotate(desc.xform.rotation, NEKO_ZAXIS));
                    neko_vec3 u = neko_vec3_sub(desc.li.point, s_intersection_start);
                    float udotn = neko_vec3_dot(u, axis);
                    float neg = neko_vec3_dot(axis, NEKO_ZAXIS) < 0.f ? -1.f : 1.f;
                    s_delta.translation = neko_vec3_scale(axis, udotn);
                    s_intersection_start = neko_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = neko_vec3_scale(s_delta.translation, neg);
                    s_delta.translation.y = 0.f;
                    s_delta.translation.x = 0.f;
                    if (snap > 0.f) {
                        s_delta.scale = neko_vec3_add(s_delta.scale, s_delta.translation);  // Store total delta since interaction began
                        float snap_len = round(neko_vec3_len(s_delta.scale) / snap) * snap;
                        neko_vec3 norm = neko_vec3_norm(s_delta.scale);
                        neko_vec3 delta = neko_vec3_scale(neko_vec3_norm(s_delta.scale), snap_len);
                        neko_vec3 os = neko_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = neko_vec3_add(os, delta);
                    } else {
                        model->scale = neko_vec3_add(model->scale, s_delta.translation);
                    }
                }
            }

        } break;

#define UPDATE_GIZMO_CONTROL_ROTATE(ID, RAY, SHAPE, MODEL, AXIS, INFO)                                                                            \
    do {                                                                                                                                          \
        s32 mouseover = 0;                                                                                                                        \
        neko_gui_id id = (ID);                                                                                                                    \
        neko_contact_info_t info = neko_default_val();                                                                                            \
        neko_vec3 axis = neko_quat_rotate(desc.xform.rotation, AXIS);                                                                             \
        info.normal = axis;                                                                                                                       \
        if (in_hover_root) {                                                                                                                      \
            neko_plane_t ip = neko_plane_from_pt_normal(desc.xform.translation, axis);                                                            \
            float denom = neko_vec3_dot(neko_v3(ip.a, ip.b, ip.c), ray.d);                                                                        \
            denom = fabsf(denom) >= neko_epsilon ? denom : 0.00001f;                                                                              \
            info.depth = -(ip.a * ray.p.x + ip.b * ray.p.y + ip.c * ray.p.z + ip.d) / denom;                                                      \
            neko_gui_gizmo_line_intersection_result_t res = neko_default_val();                                                                   \
            res.point = neko_vec3_add(ray.p, neko_vec3_scale(ray.d, info.depth));                                                                 \
            float dist = neko_vec3_dist(res.point, model->translation);                                                                           \
            float scl = neko_vec3_len(desc.xform.scale);                                                                                          \
            if (dist <= 0.6f * scl && dist >= 0.45f * scl) {                                                                                      \
                info.hit = true;                                                                                                                  \
            }                                                                                                                                     \
        }                                                                                                                                         \
        mouseover = info.hit && info.depth <= INFO.depth && in_hover_root && !ctx->hover_split && !ctx->lock_hover_id;                            \
        if (ctx->focus == id) {                                                                                                                   \
            ctx->updated_focus = 1;                                                                                                               \
            INFO = info;                                                                                                                          \
        }                                                                                                                                         \
        if (~opt & NEKO_GUI_OPT_NOINTERACT) {                                                                                                     \
            /* Check for hold focus here */                                                                                                       \
            if (mouseover && !ctx->mouse_down) {                                                                                                  \
                neko_gui_set_hover(ctx, id);                                                                                                      \
                INFO = info;                                                                                                                      \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->focus == id) {                                                                                                               \
                res |= NEKO_GUI_RES_ACTIVE;                                                                                                       \
                just_set_focus = false;                                                                                                           \
                neko_gui_set_focus(ctx, id);                                                                                                      \
                if (ctx->mouse_pressed && !mouseover) {                                                                                           \
                    neko_gui_set_focus(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
                if (!ctx->mouse_down && ~opt & NEKO_GUI_OPT_HOLDFOCUS) {                                                                          \
                    neko_gui_set_focus(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->prev_hover == id && !mouseover) {                                                                                            \
                ctx->prev_hover = ctx->hover;                                                                                                     \
            }                                                                                                                                     \
                                                                                                                                                  \
            if (ctx->hover == id) {                                                                                                               \
                if (ctx->mouse_pressed) {                                                                                                         \
                    if ((opt & NEKO_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == NEKO_GUI_MOUSE_LEFT) || (~opt & NEKO_GUI_OPT_LEFTCLICKONLY)) { \
                        neko_gui_set_focus(ctx, id);                                                                                              \
                        just_set_focus = true;                                                                                                    \
                    }                                                                                                                             \
                } else if (!mouseover) {                                                                                                          \
                    neko_gui_set_hover(ctx, 0);                                                                                                   \
                }                                                                                                                                 \
            }                                                                                                                                     \
        }                                                                                                                                         \
    } while (0)

        case NEKO_GUI_GIZMO_ROTATE: {
            // Construct translate gizmo for this frame based on given parent transform
            desc.gizmo.rotate = neko_gizmo_rotate(&desc.xform);

            neko_gui_id id_r = neko_gui_get_id(ctx, "#gizmo_rotate_right", strlen("#gizmo_rotate_right"));
            neko_gui_id id_u = neko_gui_get_id(ctx, "#gizmo_rotate_up", strlen("#gizmo_rotate_up"));
            neko_gui_id id_f = neko_gui_get_id(ctx, "#gizmo_rotate_forward", strlen("#gizmo_rotate_forward"));

            // Right
            UPDATE_GIZMO_CONTROL_ROTATE(id_r, ray, desc.gizmo.rotate.right.axis.shape.plane, desc.gizmo.rotate.right.axis.model, NEKO_XAXIS, desc.info);

            // Up
            UPDATE_GIZMO_CONTROL_ROTATE(id_u, ray, desc.gizmo.rotate.up.axis.shape.plane, desc.gizmo.rotate.up.axis.model, NEKO_YAXIS, desc.info);

            // Forward
            UPDATE_GIZMO_CONTROL_ROTATE(id_f, ray, desc.gizmo.rotate.forward.axis.shape.plane, desc.gizmo.rotate.forward.axis.model, NEKO_ZAXIS, desc.info);

            if (ctx->focus == id_r) {
                desc.li = op == NEKO_GUI_TRANSFORM_LOCAL ? neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), false, false)
                                                         : neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), false, true);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.translation = neko_v3(model->rotation.x, model->rotation.y, model->rotation.z);
                        s_delta.scale.y = model->rotation.w;
                    }

                    float dist_from_cam = neko_vec3_dist(desc.xform.translation, camera->transform.translation);
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const neko_vec3 end_vector = neko_vec3_sub(desc.li.point, desc.xform.translation);
                    const neko_vec3 start_norm = neko_vec3_norm(neko_vec3_sub(s_intersection_start, desc.xform.translation));
                    const neko_vec3 end_norm = neko_vec3_norm(end_vector);
                    const neko_vec3 rot_local = neko_quat_rotate(desc.xform.rotation, NEKO_XAXIS);

                    float len = neko_vec3_len(end_vector) / denom;
                    float angle = neko_vec3_angle_between_signed(start_norm, end_norm);

                    if (len > 1.f) {
                        angle *= len;
                    }

                    neko_vec3 cross = neko_vec3_cross(start_norm, end_norm);
                    if (neko_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = neko_rad2deg(angle);
                    s_delta.scale.x += delta;
                    s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(delta), NEKO_XAXIS);

                    if (snap > 0.f) {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(snap_delta), NEKO_XAXIS);
                        neko_quat orot = neko_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, orot);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(orot, s_delta.rotation);
                                break;
                        }
                    } else {
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, model->rotation);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(model->rotation, s_delta.rotation);
                                break;
                        }
                    }
                }
            } else if (ctx->focus == id_u) {
                desc.li = op == NEKO_GUI_TRANSFORM_LOCAL ? neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_YAXIS, NEKO_XAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), false, false)
                                                         : neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_YAXIS, NEKO_XAXIS, NEKO_ZAXIS, camera, &ray, neko_v3s(0.f), false, true);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.translation = neko_v3(model->rotation.x, model->rotation.y, model->rotation.z);
                        s_delta.scale.y = model->rotation.w;
                    }

                    float dist_from_cam = neko_vec3_dist(desc.xform.translation, camera->transform.translation);
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const neko_vec3 end_vector = neko_vec3_sub(desc.li.point, desc.xform.translation);
                    const neko_vec3 start_norm = neko_vec3_norm(neko_vec3_sub(s_intersection_start, desc.xform.translation));
                    const neko_vec3 end_norm = neko_vec3_norm(end_vector);
                    const neko_vec3 rot_local = neko_quat_rotate(desc.xform.rotation, NEKO_YAXIS);

                    float len = neko_vec3_len(end_vector) / denom;
                    float angle = neko_vec3_angle_between_signed(start_norm, end_norm);

                    if (len > 1.f) {
                        angle *= len;
                    }

                    neko_vec3 cross = neko_vec3_cross(start_norm, end_norm);
                    if (neko_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = neko_rad2deg(angle);
                    s_delta.scale.x += delta;
                    s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(delta), NEKO_YAXIS);

                    if (snap > 0.f) {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(snap_delta), NEKO_YAXIS);
                        neko_quat orot = neko_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, orot);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(orot, s_delta.rotation);
                                break;
                        }
                    } else {
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, model->rotation);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(model->rotation, s_delta.rotation);
                                break;
                        }
                    }
                }
            } else if (ctx->focus == id_f) {
                desc.li = op == NEKO_GUI_TRANSFORM_LOCAL ? neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_ZAXIS, NEKO_XAXIS, NEKO_YAXIS, camera, &ray, neko_v3s(0.f), false, false)
                                                         : neko_gui_gizmo_get_line_intersection(&desc.xform, NEKO_ZAXIS, NEKO_XAXIS, NEKO_YAXIS, camera, &ray, neko_v3s(0.f), false, true);

                if (desc.li.hit) {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.translation = neko_v3(model->rotation.x, model->rotation.y, model->rotation.z);
                        s_delta.scale.y = model->rotation.w;
                    }

                    float dist_from_cam = neko_vec3_dist(desc.xform.translation, camera->transform.translation);
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const neko_vec3 end_vector = neko_vec3_sub(desc.li.point, desc.xform.translation);
                    const neko_vec3 start_norm = neko_vec3_norm(neko_vec3_sub(s_intersection_start, desc.xform.translation));
                    const neko_vec3 end_norm = neko_vec3_norm(end_vector);
                    const neko_vec3 rot_local = neko_quat_rotate(desc.xform.rotation, NEKO_ZAXIS);

                    float len = neko_vec3_len(end_vector) / denom;
                    float angle = neko_vec3_angle_between_signed(start_norm, end_norm);

                    if (len > 1.f) {
                        angle *= len;
                    }

                    neko_vec3 cross = neko_vec3_cross(start_norm, end_norm);
                    if (neko_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = neko_rad2deg(angle);
                    s_delta.scale.x += delta;
                    s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(delta), NEKO_ZAXIS);

                    if (snap > 0.f) {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = neko_quat_angle_axis(neko_deg2rad(snap_delta), NEKO_ZAXIS);
                        neko_quat orot = neko_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, orot);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(orot, s_delta.rotation);
                                break;
                        }
                    } else {
                        switch (mode) {
                            case NEKO_GUI_TRANSFORM_WORLD:
                                model->rotation = neko_quat_mul(s_delta.rotation, model->rotation);
                                break;
                            case NEKO_GUI_TRANSFORM_LOCAL:
                                model->rotation = neko_quat_mul(model->rotation, s_delta.rotation);
                                break;
                        }
                    }
                }
            }

        } break;
    }

#endif

    // Have to render the gizmo using view/projection (so a custom render command)
    neko_gui_draw_custom(ctx, clip, neko_gui_gizmo_render, &desc, sizeof(desc));

    return res;
}

//=== Demos ===//

NEKO_API_DECL s32 neko_gui_style_editor(neko_gui_context_t* ctx, neko_gui_style_sheet_t* style_sheet, neko_gui_rect_t rect, bool* open) {
    if (!style_sheet) {
        style_sheet = &neko_gui_default_style_sheet;
    }

    static struct {
        const char* label;
        s32 idx;
    } elements[] = {{"container", NEKO_GUI_ELEMENT_CONTAINER}, {"button", NEKO_GUI_ELEMENT_BUTTON}, {"panel", NEKO_GUI_ELEMENT_PANEL},
                    {"input", NEKO_GUI_ELEMENT_INPUT},         {"label", NEKO_GUI_ELEMENT_LABEL},   {"text", NEKO_GUI_ELEMENT_TEXT},
                    {"scroll", NEKO_GUI_ELEMENT_SCROLL},       {"image", NEKO_GUI_ELEMENT_IMAGE},   {NULL}};

    static char* states[] = {"default", "hover", "focus"};

    static struct {
        const char* label;
        s32 idx;
    } colors[] = {{"background", NEKO_GUI_COLOR_BACKGROUND},
                  {"border", NEKO_GUI_COLOR_BORDER},
                  {"shadow", NEKO_GUI_COLOR_SHADOW},
                  {"content", NEKO_GUI_COLOR_CONTENT},
                  {"content_shadow", NEKO_GUI_COLOR_CONTENT_SHADOW},
                  {"content_background", NEKO_GUI_COLOR_CONTENT_BACKGROUND},
                  {"content_border", NEKO_GUI_COLOR_CONTENT_BORDER},
                  {NULL}};

    if (neko_gui_window_begin_ex(ctx, "Style_Editor", rect, open, NULL, 0x00)) {
        for (u32 i = 0; elements[i].label; ++i) {
            s32 idx = elements[i].idx;

            if (neko_gui_treenode_begin_ex(ctx, elements[i].label, NULL, 0x00)) {
                for (u32 j = 0; j < NEKO_GUI_ELEMENT_STATE_COUNT; ++j) {
                    neko_gui_push_id(ctx, &j, sizeof(j));
                    neko_gui_style_t* s = &style_sheet->styles[idx][j];
                    if (neko_gui_treenode_begin_ex(ctx, states[j], NULL, 0x00)) {
                        neko_gui_style_t* save = neko_gui_push_style(ctx, &ctx->style_sheet->styles[NEKO_GUI_ELEMENT_PANEL][0x00]);
                        s32 row[] = {-1};
                        neko_gui_layout_row(ctx, 1, row, 300);
                        neko_gui_panel_begin(ctx, states[j]);
                        {
                            neko_gui_layout_t* l = neko_gui_get_layout(ctx);
                            neko_gui_rect_t* r = &l->body;

                            const s32 ls = 80;

                            // size
                            s32 w = (s32)((l->body.w - ls) * 0.35f);
                            {
                                s32 row[] = {ls, w, w};
                                neko_gui_layout_row(ctx, 3, row, 0);
                            }

                            neko_gui_label(ctx, "size:");
                            neko_gui_slider(ctx, &s->size[0], 0.f, 500.f);
                            neko_gui_slider(ctx, &s->size[1], 0.f, 500.f);

                            w = (s32)((l->body.w - ls) * 0.2f);

                            {
                                s32 row[] = {ls, w, w, w, w};
                                neko_gui_layout_row(ctx, 5, row, 0);
                            }

                            neko_gui_label(ctx, "border_width:");
                            int16_slider(ctx, &s->border_width[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_width[3], 0, 100, NULL, 0x00);

                            neko_gui_label(ctx, "border_radius:");
                            int16_slider(ctx, &s->border_radius[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->border_radius[3], 0, 100, NULL, 0x00);

                            // padding/margin
                            neko_gui_label(ctx, "padding:");
                            int32_slider(ctx, &s->padding[0], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[1], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[2], 0, 100, NULL, 0x00);
                            int32_slider(ctx, &s->padding[3], 0, 100, NULL, 0x00);

                            neko_gui_label(ctx, "margin:");
                            int16_slider(ctx, &s->margin[0], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[1], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[2], 0, 100, NULL, 0x00);
                            int16_slider(ctx, &s->margin[3], 0, 100, NULL, 0x00);

                            // Colors
                            int sw = (s32)(l->body.w * 0.14);
                            {
                                s32 row[] = {80, sw, sw, sw, sw, -1};
                                neko_gui_layout_row(ctx, 6, row, 0);
                            }

                            for (u32 c = 0; colors[c].label; ++c) {
                                neko_gui_label(ctx, colors[c].label);
                                uint8_slider(ctx, &s->colors[c].r, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].g, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].b, 0, 255, NULL, 0x00);
                                uint8_slider(ctx, &s->colors[c].a, 0, 255, NULL, 0x00);
                                neko_gui_draw_rect(ctx, neko_gui_layout_next(ctx), s->colors[c]);
                            }
                        }
                        neko_gui_panel_end(ctx);
                        neko_gui_pop_style(ctx, save);

                        neko_gui_treenode_end(ctx);
                    }
                    neko_gui_pop_id(ctx);
                }
                neko_gui_treenode_end(ctx);
            }
        }
        neko_gui_window_end(ctx);
    }

    return 0x01;
}

NEKO_API_DECL s32 neko_gui_demo_window(neko_gui_context_t* ctx, neko_gui_rect_t rect, bool* open) {

    if (neko_gui_window_begin_ex(ctx, "Demo_Window", rect, open, NULL, 0x00)) {
        neko_gui_container_t* win = neko_gui_get_current_container(ctx);

        if (neko_gui_treenode_begin(ctx, "Help")) {
            {
                s32 row[] = {-10};
                neko_gui_layout_row(ctx, 1, row, 170);
            }

            neko_gui_panel_begin(ctx, "#!window_info");
            {
                {
                    s32 row[] = {-1};
                    neko_gui_layout_row(ctx, 1, row, 0);
                }
                neko_gui_label(ctx, "ABOUT THIS DEMO:");
                neko_gui_text(ctx, "  - Sections below are demonstrating many aspects of the util.");
                neko_gui_text(ctx, " 测试中文，你好世界");
            }
            neko_gui_panel_end(ctx);
            neko_gui_treenode_end(ctx);
        }

        if (neko_gui_treenode_begin(ctx, "Window Info")) {
            {
                s32 row[] = {-10};
                neko_gui_layout_row(ctx, 1, row, 170);
            }
            neko_gui_panel_begin(ctx, "#!window_info");
            {
                char buf[64];
                {
                    s32 row[] = {65, -1};
                    neko_gui_layout_row(ctx, 2, row, 0);
                }

                neko_gui_label(ctx, "Position:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.x, win->rect.y);
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Size:");
                neko_snprintf(buf, 64, "%.2f, %.2f", win->rect.w, win->rect.h);
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Title:");
                neko_gui_label(ctx, win->name);

                neko_gui_label(ctx, "ID:");
                neko_snprintf(buf, 64, "%zu", win->id);
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Open:");
                neko_snprintf(buf, 64, "%s", win->open ? "true" : "close");
                neko_gui_label(ctx, buf);
            }
            neko_gui_panel_end(ctx);

            neko_gui_treenode_end(ctx);
        }

        if (neko_gui_treenode_begin(ctx, "Context State")) {
            {
                s32 row[] = {-10};
                neko_gui_layout_row(ctx, 1, row, 170);
            }
            neko_gui_panel_begin(ctx, "#!context_state");
            {
                char buf[64];
                {
                    s32 row[] = {80, -1};
                    neko_gui_layout_row(ctx, 2, row, 0);
                }

                neko_gui_label(ctx, "Hovered:");
                neko_snprintf(buf, 64, "%s", ctx->hover_root ? ctx->hover_root->name : "NULL");
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Focused:");
                neko_snprintf(buf, 64, "%s", ctx->focus_root ? ctx->focus_root->name : "NULL");
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Active:");
                neko_snprintf(buf, 64, "%s", ctx->active_root ? ctx->active_root->name : "NULL");
                neko_gui_label(ctx, buf);

                neko_gui_label(ctx, "Lock Focus:");
                neko_snprintf(buf, 64, "%zu", ctx->lock_focus);
                neko_gui_label(ctx, buf);
            }
            neko_gui_panel_end(ctx);

            neko_gui_treenode_end(ctx);
        }

        if (neko_gui_treenode_begin(ctx, "Widgets")) {
            {
                s32 row[] = {-10};
                neko_gui_layout_row(ctx, 1, row, 170);
            }
            neko_gui_panel_begin(ctx, "#!widgets");
            {
                {
                    s32 row[] = {150, 50};
                    neko_gui_layout_row(ctx, 2, row, 0);
                }
                neko_gui_layout_column_begin(ctx);
                {
                    {
                        s32 row[] = {0};
                        neko_gui_layout_row(ctx, 1, row, 0);
                    }
                    neko_gui_button(ctx, "Button");

                    // Label
                    neko_gui_label(ctx, "Label");

                    // Text
                    {
                        s32 row[] = {150};
                        neko_gui_layout_row(ctx, 1, row, 0);
                    }
                    neko_gui_text(ctx, "This is some text");

                    static char buf[64] = {0};
                    neko_gui_textbox(ctx, buf, 64);
                }
                neko_gui_layout_column_end(ctx);

                neko_gui_layout_column_begin(ctx);
                {
                    neko_gui_label(ctx, "(?)");
                    if (ctx->hover == ctx->last_id) neko_println("HOVERED");
                }
                neko_gui_layout_column_end(ctx);
            }
            neko_gui_panel_end(ctx);
            neko_gui_treenode_end(ctx);
        }

        neko_gui_window_end(ctx);
    }
    return 0x01;
}

//==== Resource Loading ===//

typedef enum { NEKO_GUI_SS_DEF_NUMBER = 0x00, NEKO_GUI_SS_DEF_ENUM, NEKO_GUI_SS_DEF_COLOR, NEKO_GUI_SS_DEF_STR } neko_gui_ss_var_def_type;

typedef struct {
    neko_gui_ss_var_def_type type;
    union {
        s32 number;
        neko_color_t color;
        char str[64];
    } val;
} neko_gui_ss_var_def_t;

typedef struct {
    neko_hash_table(u64, neko_gui_ss_var_def_t) variables;
} neko_gui_ss_variables_t;

#define _NEKO_GUI_SS_GET_TO_VALUES(T0, T1)                 \
    do {                                                   \
        bool ret = neko_lexer_require_token_type(lex, T0); \
        ret &= neko_lexer_require_token_type(lex, T1);     \
        if (!ret) {                                        \
            neko_log_warning("Unidentified token.");       \
            return false;                                  \
        }                                                  \
        token = neko_lexer_current_token(lex);             \
    } while (0)

bool _neko_gui_style_sheet_parse_attribute_transition(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, const u64 id_tag, s32 elementid, s32 state,
                                                      neko_gui_ss_variables_t* variables) {
    // Name of enum attribute
    neko_token_t token = neko_lexer_current_token(lex);
    // neko_token_debug_print(&token);

    if (id_tag) {
        if (!neko_hash_table_exists(ss->cid_animations, id_tag)) {
            neko_gui_animation_property_list_t sl = neko_default_val();
            neko_hash_table_insert(ss->cid_animations, id_tag, sl);
        }
    }

#define PARSE_TRANSITION(T)                                                                                                      \
    do {                                                                                                                         \
        u32 time_v = 0;                                                                                                          \
        u32 delay_v = 0;                                                                                                         \
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);                                                         \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR) || neko_lexer_require_token_type(lex, NEKO_TOKEN_NUMBER)); \
        if (!ret) {                                                                                                              \
            neko_log_warning("Transition: Unidentified token.");                                                                 \
            return false;                                                                                                        \
        }                                                                                                                        \
        neko_token_t time = neko_lexer_current_token(lex);                                                                       \
        switch (time.type) {                                                                                                     \
            case NEKO_TOKEN_NUMBER: {                                                                                            \
                neko_snprintfc(TIME, 32, "%.*s", time.len, time.text);                                                           \
                time_v = (u32)atoi(TIME);                                                                                        \
            } break;                                                                                                             \
                                                                                                                                 \
            case NEKO_TOKEN_DOLLAR: {                                                                                            \
                ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER));                                              \
                if (!ret) {                                                                                                      \
                    neko_log_warning("Transition: Variable missing identifier token.");                                          \
                    return false;                                                                                                \
                }                                                                                                                \
                token = neko_lexer_current_token(lex);                                                                           \
                neko_snprintfc(VAR, 64, "%.*s", token.len, token.text);                                                          \
                u64 hash = neko_hash_str64(VAR);                                                                                 \
                if (neko_hash_table_exists(variables->variables, hash)) {                                                        \
                    time_v = (u32)(neko_hash_table_getp(variables->variables, hash))->val.number;                                \
                } else {                                                                                                         \
                    neko_log_warning("Transition: Variable not found: %s.", VAR);                                                \
                    return false;                                                                                                \
                }                                                                                                                \
            } break;                                                                                                             \
        }                                                                                                                        \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR) || neko_lexer_require_token_type(lex, NEKO_TOKEN_NUMBER)); \
        if (!ret) {                                                                                                              \
            neko_log_warning("Transition: Unidentified token.");                                                                 \
            return false;                                                                                                        \
        }                                                                                                                        \
        neko_token_t delay = neko_lexer_current_token(lex);                                                                      \
        switch (delay.type) {                                                                                                    \
            case NEKO_TOKEN_NUMBER: {                                                                                            \
                neko_snprintfc(DELAY, 32, "%.*s", delay.len, delay.text);                                                        \
                u32 delay_v = (u32)atoi(DELAY);                                                                                  \
            } break;                                                                                                             \
                                                                                                                                 \
            case NEKO_TOKEN_DOLLAR: {                                                                                            \
                ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER));                                              \
                if (!ret) {                                                                                                      \
                    neko_log_warning("Transition: Variable missing identifier token.");                                          \
                    return false;                                                                                                \
                }                                                                                                                \
                token = neko_lexer_current_token(lex);                                                                           \
                neko_snprintfc(VAR, 64, "%.*s", token.len, token.text);                                                          \
                u64 hash = neko_hash_str64(VAR);                                                                                 \
                if (neko_hash_table_exists(variables->variables, hash)) {                                                        \
                    delay_v = (u32)(neko_hash_table_getp(variables->variables, hash))->val.number;                               \
                } else {                                                                                                         \
                    neko_log_warning("Transition: Variable not found: %s.", VAR);                                                \
                    return false;                                                                                                \
                }                                                                                                                \
            } break;                                                                                                             \
        }                                                                                                                        \
        neko_gui_animation_property_t prop = neko_default_val();                                                                 \
        prop.type = T;                                                                                                           \
        prop.time = (s16)time_v;                                                                                                 \
        prop.delay = (s16)delay_v;                                                                                               \
        switch (state) {                                                                                                         \
            case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                                                                               \
                for (u32 s = 0; s < 3; ++s) {                                                                                    \
                    neko_dyn_array_push(list->properties[s], prop);                                                              \
                }                                                                                                                \
            } break;                                                                                                             \
            default: {                                                                                                           \
                neko_dyn_array_push(list->properties[state], prop);                                                              \
            } break;                                                                                                             \
        }                                                                                                                        \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON));                                                       \
        if (!ret) {                                                                                                              \
            neko_log_warning("Transition: Missing semicolon.");                                                                  \
            return false;                                                                                                        \
        }                                                                                                                        \
    } while (0)

    _NEKO_GUI_SS_GET_TO_VALUES(NEKO_TOKEN_COLON, NEKO_TOKEN_LBRACE);

    if (!neko_hash_table_exists(ss->animations, (neko_gui_element_type)elementid)) {
        neko_gui_animation_property_list_t list = neko_default_val();
        neko_hash_table_insert(ss->animations, (neko_gui_element_type)elementid, list);
    }
    neko_gui_animation_property_list_t* list = id_tag ? neko_hash_table_getp(ss->cid_animations, id_tag) : neko_hash_table_getp(ss->animations, (neko_gui_element_type)elementid);

    s32 bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE:
                bc++;
                break;
            case NEKO_TOKEN_RBRACE:
                bc--;
                break;
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "color_background"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_BACKGROUND);
                else if (neko_token_compare_text(&token, "color_border"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_BORDER);
                else if (neko_token_compare_text(&token, "color_shadow"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_SHADOW);
                else if (neko_token_compare_text(&token, "color_content"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_CONTENT);
                else if (neko_token_compare_text(&token, "width"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_WIDTH);
                else if (neko_token_compare_text(&token, "height"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_HEIGHT);
                else if (neko_token_compare_text(&token, "padding"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING);
                else if (neko_token_compare_text(&token, "padding_left"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING_LEFT);
                else if (neko_token_compare_text(&token, "padding_right"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING_RIGHT);
                else if (neko_token_compare_text(&token, "padding_top"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING_TOP);
                else if (neko_token_compare_text(&token, "padding_bottom"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING_BOTTOM);
                else if (neko_token_compare_text(&token, "margin"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_PADDING);
                else if (neko_token_compare_text(&token, "margin_left"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_MARGIN_LEFT);
                else if (neko_token_compare_text(&token, "margin_right"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_MARGIN_RIGHT);
                else if (neko_token_compare_text(&token, "margin_top"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_MARGIN_TOP);
                else if (neko_token_compare_text(&token, "margin_bottom"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_MARGIN_BOTTOM);
                else if (neko_token_compare_text(&token, "shadow_x"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_SHADOW_X);
                else if (neko_token_compare_text(&token, "shadow_y"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_SHADOW_Y);
                else if (neko_token_compare_text(&token, "color_content_background"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND);
                else if (neko_token_compare_text(&token, "color_content_border"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_CONTENT_BORDER);
                else if (neko_token_compare_text(&token, "color_content_shadow"))
                    PARSE_TRANSITION(NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW);
                else {
                    neko_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
                    return false;
                }
            } break;
        }
    }

    return true;
}

bool _neko_gui_style_sheet_parse_attribute_font(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, u64 id_tag, s32 elementid, s32 state, neko_gui_ss_variables_t* variables) {
    // Name of enum attribute
    neko_token_t token = neko_lexer_current_token(lex);
    // neko_token_debug_print(&token);
    //
    neko_gui_style_list_t* idsl = NULL;
    if (id_tag) {
        const u64 idhash = id_tag;
        if (!neko_hash_table_exists(ss->cid_styles, idhash)) {
            neko_gui_style_list_t sl = neko_default_val();
            neko_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = neko_hash_table_getp(ss->cid_styles, idhash);
    }

#define SET_FONT(FONT)                                                  \
    do {                                                                \
        switch (state) {                                                \
            case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                      \
                se.font = FONT;                                         \
                for (u32 s = 0; s < 3; ++s) {                           \
                    if (idsl)                                           \
                        neko_dyn_array_push(idsl->styles[s], se);       \
                    else                                                \
                        ss->styles[elementid][s].font = FONT;           \
                }                                                       \
            } break;                                                    \
            default: {                                                  \
                se.font = FONT;                                         \
                if (idsl) neko_dyn_array_push(idsl->styles[state], se); \
                ss->styles[elementid][state].font = FONT;               \
            } break;                                                    \
        }                                                               \
    } while (0)

    if (neko_token_compare_text(&token, "font")) {
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_STRING) || neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR));
        if (!ret) {
            neko_log_warning("Missing either string value or variable.");
            return false;
        }
        neko_gui_style_element_t se = neko_default_val();
        se.type = NEKO_GUI_STYLE_FONT;
        token = neko_lexer_current_token(lex);
        char FONT[64] = neko_default_val();
        switch (token.type) {
            case NEKO_TOKEN_STRING: {
                neko_snprintf(FONT, sizeof(FONT), "%.*s", token.len - 2, token.text + 1);
            } break;

            case NEKO_TOKEN_DOLLAR: {
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);
                if (!ret) {
                    neko_log_warning("Unidentified token.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                u64 hash = neko_hash_str64(TMP);
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    neko_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                memcpy(FONT, var->val.str, sizeof(FONT));
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);
                if (!ret) {
                    neko_log_warning("Missing semicolon.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
            } break;
        }

        u64 hash = neko_hash_str64(FONT);
        bool found = false;
        for (neko_hash_table_iter it = neko_hash_table_iter_new(ctx->font_stash); neko_hash_table_iter_valid(ctx->font_stash, it); neko_hash_table_iter_advance(ctx->font_stash, it)) {
            u64 key = neko_hash_table_getk(ctx->font_stash, it);
            if (hash == key) {
                neko_asset_ascii_font_t* font = neko_hash_table_iter_get(ctx->font_stash, it);
                SET_FONT(font);
                found = true;
                break;
            }
        }
        if (!found) {
            neko_log_warning("Font not found in gui font stash: %s", FONT);
        }
    } else {
        neko_log_warning("Unidentified token.");
        return false;
    }

    return true;
}

bool _neko_gui_style_sheet_parse_attribute_enum(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, u64 id_tag, s32 elementid, s32 state, neko_gui_ss_variables_t* variables) {
    // Name of enum attribute
    neko_token_t token = neko_lexer_current_token(lex);
    // neko_token_debug_print(&token);

    neko_gui_style_list_t* idsl = NULL;
    if (id_tag) {
        const u64 idhash = id_tag;
        if (!neko_hash_table_exists(ss->cid_styles, idhash)) {
            neko_gui_style_list_t sl = neko_default_val();
            neko_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = neko_hash_table_getp(ss->cid_styles, idhash);
    }

#define SET_ENUM(COMP, VAL)                                       \
    do {                                                          \
        se.value = VAL;                                           \
        switch (state) {                                          \
            case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                \
                if (idsl) {                                       \
                    for (u32 s = 0; s < 3; ++s) {                 \
                        neko_dyn_array_push(idsl->styles[s], se); \
                    }                                             \
                } else {                                          \
                    for (u32 s = 0; s < 3; ++s) {                 \
                        ss->styles[elementid][s].COMP = VAL;      \
                    }                                             \
                }                                                 \
            } break;                                              \
            default: {                                            \
                if (idsl)                                         \
                    neko_dyn_array_push(idsl->styles[state], se); \
                else                                              \
                    ss->styles[elementid][state].COMP = VAL;      \
            } break;                                              \
        }                                                         \
    } while (0)

    if (neko_token_compare_text(&token, "justify_content")) {
        neko_gui_style_element_t se = neko_default_val();
        se.type = NEKO_GUI_STYLE_JUSTIFY_CONTENT;
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER) || neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR));
        if (!ret) {
            neko_log_warning("Missing either identifier value or variable.");
            return false;
        }
        token = neko_lexer_current_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "start"))
                    SET_ENUM(justify_content, NEKO_GUI_JUSTIFY_START);
                else if (neko_token_compare_text(&token, "end"))
                    SET_ENUM(justify_content, NEKO_GUI_JUSTIFY_END);
                else if (neko_token_compare_text(&token, "center"))
                    SET_ENUM(justify_content, NEKO_GUI_JUSTIFY_CENTER);
            } break;

            case NEKO_TOKEN_DOLLAR: {
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);
                if (!ret) {
                    neko_log_warning("Unidentified token.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                u64 hash = neko_hash_str64(TMP);
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    neko_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                SET_ENUM(justify_content, var->val.number);
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);
                if (!ret) {
                    neko_log_warning("Missing semicolon.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
            } break;
        }
    } else if (neko_token_compare_text(&token, "align_content")) {
        neko_gui_style_element_t se = neko_default_val();
        se.type = NEKO_GUI_STYLE_ALIGN_CONTENT;
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER) || neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR));
        if (!ret) {
            neko_log_warning("Missing either identifier value or variable.");
            return false;
        }
        token = neko_lexer_current_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "start"))
                    SET_ENUM(align_content, NEKO_GUI_ALIGN_START);
                else if (neko_token_compare_text(&token, "end"))
                    SET_ENUM(align_content, NEKO_GUI_ALIGN_END);
                else if (neko_token_compare_text(&token, "center"))
                    SET_ENUM(align_content, NEKO_GUI_ALIGN_CENTER);
            } break;

            case NEKO_TOKEN_DOLLAR: {
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);
                if (!ret) {
                    neko_log_warning("Unidentified token.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                u64 hash = neko_hash_str64(TMP);
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    neko_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                SET_ENUM(align_content, var->val.number);
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);
                if (!ret) {
                    neko_log_warning("Missing semicolon.");
                    return false;
                }
                token = neko_lexer_current_token(lex);
            } break;
        }
    }

    return true;
}

bool _neko_gui_style_sheet_parse_attribute_val(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, u64 id_tag, s32 elementid, s32 state, neko_gui_ss_variables_t* variables) {
    // Name of value attribute
    neko_token_t token = neko_lexer_current_token(lex);
    // neko_token_debug_print(&token);

    neko_gui_style_list_t* idsl = NULL;
    if (id_tag) {
        const u64 idhash = id_tag;
        if (!neko_hash_table_exists(ss->cid_styles, idhash)) {
            neko_gui_style_list_t sl = neko_default_val();
            neko_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = neko_hash_table_getp(ss->cid_styles, idhash);
    }

#define SET_VAL4(COMP, T, SE)                                                                                                                              \
    do {                                                                                                                                                   \
        neko_gui_style_element_t se = neko_default_val();                                                                                                  \
        se.type = SE;                                                                                                                                      \
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);                                                                                   \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR) || neko_lexer_require_token_type(lex, NEKO_TOKEN_NUMBER));                           \
        token = neko_lexer_current_token(lex);                                                                                                             \
        neko_token_debug_print(&token);                                                                                                                    \
        if (!ret) {                                                                                                                                        \
            neko_log_warning("Unidentified token.");                                                                                                       \
            return false;                                                                                                                                  \
        }                                                                                                                                                  \
        switch (token.type) {                                                                                                                              \
            case NEKO_TOKEN_NUMBER: {                                                                                                                      \
                neko_snprintfc(TMP, 10, "%.*s", token.len, token.text);                                                                                    \
                u32 val = (u32)atoi(TMP);                                                                                                                  \
                se.value = (T)val;                                                                                                                         \
                switch (state) {                                                                                                                           \
                    case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                                                                                                 \
                        if (idsl) {                                                                                                                        \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_DEFAULT], se);                                                         \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_HOVER], se);                                                           \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_FOCUS], se);                                                           \
                        } else {                                                                                                                           \
                            for (u32 p = 0; p < 4; ++p) {                                                                                                  \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_DEFAULT].COMP[p] = (T)val;                                                    \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_HOVER].COMP[p] = (T)val;                                                      \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_FOCUS].COMP[p] = (T)val;                                                      \
                            }                                                                                                                              \
                        }                                                                                                                                  \
                    } break;                                                                                                                               \
                    default: {                                                                                                                             \
                        if (idsl)                                                                                                                          \
                            neko_dyn_array_push(idsl->styles[state], se);                                                                                  \
                        else {                                                                                                                             \
                            for (u32 p = 0; p < 4; ++p) {                                                                                                  \
                                ss->styles[elementid][state].COMP[p] = (T)val;                                                                             \
                            }                                                                                                                              \
                        }                                                                                                                                  \
                    } break;                                                                                                                               \
                }                                                                                                                                          \
            } break;                                                                                                                                       \
            case NEKO_TOKEN_DOLLAR: {                                                                                                                      \
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);                                                                      \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Unidentified token.");                                                                                               \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);                                                                                   \
                u64 hash = neko_hash_str64(TMP);                                                                                                           \
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL; \
                if (!var) {                                                                                                                                \
                    neko_log_warning("Variable not found: %s", TMP);                                                                                       \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                T val = var->val.number;                                                                                                                   \
                se.value = val;                                                                                                                            \
                switch (state) {                                                                                                                           \
                    case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                                                                                                 \
                        if (idsl) {                                                                                                                        \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_DEFAULT], se);                                                         \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_HOVER], se);                                                           \
                            neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_FOCUS], se);                                                           \
                        } else {                                                                                                                           \
                            for (u32 p = 0; p < 4; ++p) {                                                                                                  \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_DEFAULT].COMP[p] = val;                                                       \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_HOVER].COMP[p] = val;                                                         \
                                ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_FOCUS].COMP[p] = val;                                                         \
                            }                                                                                                                              \
                        }                                                                                                                                  \
                    } break;                                                                                                                               \
                    default: {                                                                                                                             \
                        if (idsl)                                                                                                                          \
                            neko_dyn_array_push(idsl->styles[state], se);                                                                                  \
                        else {                                                                                                                             \
                            for (u32 p = 0; p < 4; ++p) {                                                                                                  \
                                ss->styles[elementid][state].COMP[p] = val;                                                                                \
                            }                                                                                                                              \
                        }                                                                                                                                  \
                    } break;                                                                                                                               \
                }                                                                                                                                          \
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);                                                                            \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Missing semicolon.");                                                                                                \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
            } break;                                                                                                                                       \
        }                                                                                                                                                  \
    } while (0)

#define SET_VAL2(COMP0, COMP1, T, SE0, SE1)                                                                                                                \
    do {                                                                                                                                                   \
        neko_gui_style_element_t se0 = neko_default_val();                                                                                                 \
        neko_gui_style_element_t se1 = neko_default_val();                                                                                                 \
        se0.type = SE0;                                                                                                                                    \
        se1.type = SE1;                                                                                                                                    \
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);                                                                                   \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR) || neko_lexer_require_token_type(lex, NEKO_TOKEN_NUMBER));                           \
        if (!ret) {                                                                                                                                        \
            neko_log_warning("Unidentified token.");                                                                                                       \
            return false;                                                                                                                                  \
        }                                                                                                                                                  \
        token = neko_lexer_current_token(lex);                                                                                                             \
        T val = 0;                                                                                                                                         \
        switch (token.type) {                                                                                                                              \
            case NEKO_TOKEN_NUMBER: {                                                                                                                      \
                neko_snprintfc(TMP, 10, "%.*s", token.len, token.text);                                                                                    \
                val = (T)atoi(TMP);                                                                                                                        \
            } break;                                                                                                                                       \
            case NEKO_TOKEN_DOLLAR: {                                                                                                                      \
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);                                                                      \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Unidentified token.");                                                                                               \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);                                                                                   \
                u64 hash = neko_hash_str64(TMP);                                                                                                           \
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL; \
                if (!var) {                                                                                                                                \
                    neko_log_warning("Variable not found: %s", TMP);                                                                                       \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                val = (T)var->val.number;                                                                                                                  \
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);                                                                            \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Missing semicolon.");                                                                                                \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
            } break;                                                                                                                                       \
        }                                                                                                                                                  \
        switch (state) {                                                                                                                                   \
            case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                                                                                                         \
                if (idsl) {                                                                                                                                \
                    se0.value = val;                                                                                                                       \
                    se1.value = val;                                                                                                                       \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_DEFAULT], se0);                                                                \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_HOVER], se0);                                                                  \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_FOCUS], se0);                                                                  \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_DEFAULT], se1);                                                                \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_HOVER], se1);                                                                  \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_FOCUS], se1);                                                                  \
                } else {                                                                                                                                   \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_DEFAULT].COMP0 = val;                                                                     \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_HOVER].COMP0 = val;                                                                       \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_FOCUS].COMP0 = val;                                                                       \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_DEFAULT].COMP1 = val;                                                                     \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_HOVER].COMP1 = val;                                                                       \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_FOCUS].COMP1 = val;                                                                       \
                }                                                                                                                                          \
            } break;                                                                                                                                       \
            default: {                                                                                                                                     \
                if (idsl) {                                                                                                                                \
                    se0.value = val;                                                                                                                       \
                    se1.value = val;                                                                                                                       \
                    neko_dyn_array_push(idsl->styles[state], se0);                                                                                         \
                    neko_dyn_array_push(idsl->styles[state], se1);                                                                                         \
                } else {                                                                                                                                   \
                    ss->styles[elementid][state].COMP0 = val;                                                                                              \
                    ss->styles[elementid][state].COMP1 = val;                                                                                              \
                }                                                                                                                                          \
            } break;                                                                                                                                       \
        }                                                                                                                                                  \
    } while (0)

#define SET_VAL(COMP, T, SE)                                                                                                                               \
    do {                                                                                                                                                   \
        bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_COLON);                                                                                   \
        token = neko_lexer_current_token(lex);                                                                                                             \
        ret &= (neko_lexer_require_token_type(lex, NEKO_TOKEN_DOLLAR) || neko_lexer_require_token_type(lex, NEKO_TOKEN_NUMBER));                           \
        token = neko_lexer_current_token(lex);                                                                                                             \
        if (!ret) {                                                                                                                                        \
            neko_log_warning("Unidentified token: %.*s", token.len, token.text);                                                                           \
            return false;                                                                                                                                  \
        }                                                                                                                                                  \
        neko_gui_style_element_t se = neko_default_val();                                                                                                  \
        se.type = SE;                                                                                                                                      \
        T val = 0;                                                                                                                                         \
        switch (token.type) {                                                                                                                              \
            case NEKO_TOKEN_NUMBER: {                                                                                                                      \
                neko_snprintfc(TMP, 10, "%.*s", token.len, token.text);                                                                                    \
                val = (T)atoi(TMP);                                                                                                                        \
            } break;                                                                                                                                       \
                                                                                                                                                           \
            case NEKO_TOKEN_DOLLAR: {                                                                                                                      \
                bool ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_IDENTIFIER);                                                                      \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Unidentified token.");                                                                                               \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
                neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);                                                                                   \
                u64 hash = neko_hash_str64(TMP);                                                                                                           \
                neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL; \
                if (!var) {                                                                                                                                \
                    neko_log_warning("Variable not found: %s", TMP);                                                                                       \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                val = (T)var->val.number;                                                                                                                  \
                ret = neko_lexer_require_token_type(lex, NEKO_TOKEN_SEMICOLON);                                                                            \
                if (!ret) {                                                                                                                                \
                    neko_log_warning("Missing semicolon.");                                                                                                \
                    return false;                                                                                                                          \
                }                                                                                                                                          \
                token = neko_lexer_current_token(lex);                                                                                                     \
            } break;                                                                                                                                       \
        }                                                                                                                                                  \
        switch (state) {                                                                                                                                   \
            case NEKO_GUI_ELEMENT_STATE_DEFAULT: {                                                                                                         \
                if (idsl) {                                                                                                                                \
                    se.value = (s32)val;                                                                                                                   \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_DEFAULT], se);                                                                 \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_HOVER], se);                                                                   \
                    neko_dyn_array_push(idsl->styles[NEKO_GUI_ELEMENT_STATE_FOCUS], se);                                                                   \
                } else {                                                                                                                                   \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_DEFAULT].COMP = val;                                                                      \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_HOVER].COMP = val;                                                                        \
                    ss->styles[elementid][NEKO_GUI_ELEMENT_STATE_FOCUS].COMP = val;                                                                        \
                }                                                                                                                                          \
            } break;                                                                                                                                       \
            default: {                                                                                                                                     \
                if (idsl) {                                                                                                                                \
                    se.value = (s32)val;                                                                                                                   \
                    neko_dyn_array_push(idsl->styles[state], se);                                                                                          \
                } else {                                                                                                                                   \
                    ss->styles[elementid][state].COMP = val;                                                                                               \
                }                                                                                                                                          \
            } break;                                                                                                                                       \
        }                                                                                                                                                  \
    } while (0)

    if (neko_token_compare_text(&token, "width"))
        SET_VAL(size[0], float, NEKO_GUI_STYLE_WIDTH);
    else if (neko_token_compare_text(&token, "height"))
        SET_VAL(size[1], float, NEKO_GUI_STYLE_HEIGHT);
    else if (neko_token_compare_text(&token, "padding"))
        SET_VAL4(padding, s16, NEKO_GUI_STYLE_PADDING);
    else if (neko_token_compare_text(&token, "padding_left"))
        SET_VAL(padding[NEKO_GUI_PADDING_LEFT], s16, NEKO_GUI_STYLE_PADDING_LEFT);
    else if (neko_token_compare_text(&token, "padding_right"))
        SET_VAL(padding[NEKO_GUI_PADDING_RIGHT], s16, NEKO_GUI_STYLE_PADDING_RIGHT);
    else if (neko_token_compare_text(&token, "padding_top"))
        SET_VAL(padding[NEKO_GUI_PADDING_TOP], s16, NEKO_GUI_STYLE_PADDING_TOP);
    else if (neko_token_compare_text(&token, "padding_bottom"))
        SET_VAL(padding[NEKO_GUI_PADDING_BOTTOM], s16, NEKO_GUI_STYLE_PADDING_BOTTOM);
    else if (neko_token_compare_text(&token, "margin"))
        SET_VAL4(margin, s16, NEKO_GUI_STYLE_MARGIN);
    else if (neko_token_compare_text(&token, "margin_left"))
        SET_VAL(margin[NEKO_GUI_MARGIN_LEFT], s16, NEKO_GUI_STYLE_MARGIN_LEFT);
    else if (neko_token_compare_text(&token, "margin_right"))
        SET_VAL(margin[NEKO_GUI_MARGIN_RIGHT], s16, NEKO_GUI_STYLE_MARGIN_RIGHT);
    else if (neko_token_compare_text(&token, "margin_top"))
        SET_VAL(margin[NEKO_GUI_MARGIN_TOP], s16, NEKO_GUI_STYLE_MARGIN_TOP);
    else if (neko_token_compare_text(&token, "margin_bottom"))
        SET_VAL(margin[NEKO_GUI_MARGIN_BOTTOM], s16, NEKO_GUI_STYLE_MARGIN_BOTTOM);
    else if (neko_token_compare_text(&token, "border"))
        SET_VAL4(border_width, s16, NEKO_GUI_STYLE_BORDER_WIDTH);
    else if (neko_token_compare_text(&token, "border_left"))
        SET_VAL(border_width[0], s16, NEKO_GUI_STYLE_BORDER_WIDTH_LEFT);
    else if (neko_token_compare_text(&token, "border_right"))
        SET_VAL(border_width[1], s16, NEKO_GUI_STYLE_BORDER_WIDTH_RIGHT);
    else if (neko_token_compare_text(&token, "border_top"))
        SET_VAL(border_width[2], s16, NEKO_GUI_STYLE_BORDER_WIDTH_TOP);
    else if (neko_token_compare_text(&token, "border_bottom"))
        SET_VAL(border_width[3], s16, NEKO_GUI_STYLE_BORDER_WIDTH_BOTTOM);
    else if (neko_token_compare_text(&token, "shadow"))
        SET_VAL2(shadow_x, shadow_y, s16, NEKO_GUI_STYLE_SHADOW_X, NEKO_GUI_STYLE_SHADOW_Y);

    return true;
}

bool _neko_gui_style_sheet_parse_attribute_color(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, u64 id_tag, s32 elementid, s32 state, neko_gui_ss_variables_t* variables) {
    // Name of color attribute
    neko_token_t token = neko_lexer_current_token(lex);
    // neko_token_debug_print(&token);

    neko_gui_style_list_t* idsl = NULL;
    if (id_tag) {
        const u64 idhash = id_tag;
        if (!neko_hash_table_exists(ss->cid_styles, idhash)) {
            neko_gui_style_list_t sl = neko_default_val();
            neko_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = neko_hash_table_getp(ss->cid_styles, idhash);
    }

    neko_gui_style_element_type type = (neko_gui_style_element_type)0x00;
    s32 color = NEKO_GUI_COLOR_BACKGROUND;
    if (neko_token_compare_text(&token, "color_background")) {
        color = NEKO_GUI_COLOR_BACKGROUND;
        type = NEKO_GUI_STYLE_COLOR_BACKGROUND;
    } else if (neko_token_compare_text(&token, "color_border")) {
        color = NEKO_GUI_COLOR_BORDER;
        type = NEKO_GUI_STYLE_COLOR_BORDER;
    } else if (neko_token_compare_text(&token, "color_shadow")) {
        color = NEKO_GUI_COLOR_SHADOW;
        type = NEKO_GUI_STYLE_COLOR_SHADOW;
    } else if (neko_token_compare_text(&token, "color_content")) {
        color = NEKO_GUI_COLOR_CONTENT;
        type = NEKO_GUI_STYLE_COLOR_CONTENT;
    } else if (neko_token_compare_text(&token, "color_content_background")) {
        color = NEKO_GUI_COLOR_CONTENT_BACKGROUND;
        type = NEKO_GUI_STYLE_COLOR_CONTENT_BACKGROUND;
    } else if (neko_token_compare_text(&token, "color_content_border")) {
        color = NEKO_GUI_COLOR_CONTENT_BORDER;
        type = NEKO_GUI_STYLE_COLOR_CONTENT_BORDER;
    } else if (neko_token_compare_text(&token, "color_content_shadow")) {
        color = NEKO_GUI_COLOR_CONTENT_SHADOW;
        type = NEKO_GUI_STYLE_COLOR_CONTENT_SHADOW;
    } else {
        neko_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
        return false;
    }

    token = neko_lexer_next_token(lex);
    if (token.type != NEKO_TOKEN_COLON) {
        neko_log_warning("Unidentified token. (Expected colon after attribute type).");
        neko_token_debug_print(&token);
        return false;
    }

    neko_gui_style_element_t se = neko_default_val();
    se.type = type;

    token = neko_lexer_next_token(lex);
    // neko_println("Parsing color: %.*s", token.len, token.text);

    if (neko_token_compare_text(&token, "rgba") || neko_token_compare_text(&token, "rgb")) {
        s32 i = 0;
        while (neko_lexer_can_lex(lex) && token.type != NEKO_TOKEN_RPAREN) {
            token = neko_lexer_next_token(lex);
            switch (token.type) {
                case NEKO_TOKEN_NUMBER: {
                    if (i < 4) {
                        neko_snprintfc(TMP, 10, "%.*s", token.len, token.text);
                        u8 val = (u8)atoi(TMP);

                        if (idsl) {
                            se.color.rgba[i] = val;
                        } else {
#define SET_COLOR(STATE)                                            \
    do {                                                            \
        switch (i) {                                                \
            case 0:                                                 \
                ss->styles[elementid][STATE].colors[color].r = val; \
                break;                                              \
            case 1:                                                 \
                ss->styles[elementid][STATE].colors[color].g = val; \
                break;                                              \
            case 2:                                                 \
                ss->styles[elementid][STATE].colors[color].b = val; \
                break;                                              \
            case 3:                                                 \
                ss->styles[elementid][STATE].colors[color].a = val; \
                break;                                              \
        }                                                           \
    } while (0)

                            switch (state) {
                                case NEKO_GUI_ELEMENT_STATE_HOVER:
                                case NEKO_GUI_ELEMENT_STATE_FOCUS: {
                                    SET_COLOR(state);
                                } break;

                                case NEKO_GUI_ELEMENT_STATE_DEFAULT: {
                                    for (u32 s = 0; s < 3; ++s) {
                                        SET_COLOR(s);
                                    }
                                } break;
                            }
                        }
                        i++;
                        // neko_token_debug_print(&token);
                    }
                } break;
            }
        }

        // Set alpha, if not provided
        if (i < 4) {
            ss->styles[elementid][state].colors[color].a = 255;
            se.color.a = 255;
        }

        // Push style element
        if (idsl) {
            switch (state) {
                case NEKO_GUI_ELEMENT_STATE_DEFAULT: {
                    for (u32 s = 0; s < 3; ++s) {
                        neko_dyn_array_push(idsl->styles[s], se);
                    }
                } break;
                default: {
                    neko_dyn_array_push(idsl->styles[state], se);
                } break;
            }
        }
    } else if (neko_token_compare_text(&token, "$")) {
        token = neko_lexer_next_token(lex);
        if (token.type != NEKO_TOKEN_IDENTIFIER) {
            neko_log_warning("Unidentified symbol found: %.*s. Expecting identifier for variable name.", token.len, token.text);
            return false;
        }

        neko_snprintfc(TMP, 256, "%.*s", token.len, token.text);
        u64 hash = neko_hash_str64(TMP);
        neko_gui_ss_var_def_t* var = neko_hash_table_exists(variables->variables, hash) ? neko_hash_table_getp(variables->variables, hash) : NULL;
        if (var) {

            se.color = var->val.color;

            switch (state) {
                default: {
                    if (idsl)
                        neko_dyn_array_push(idsl->styles[state], se);
                    else
                        ss->styles[elementid][state].colors[color] = var->val.color;
                } break;

                case NEKO_GUI_ELEMENT_STATE_DEFAULT: {
                    for (u32 s = 0; s < 3; ++s) {
                        if (idsl)
                            neko_dyn_array_push(idsl->styles[s], se);
                        else
                            ss->styles[elementid][s].colors[color] = var->val.color;
                    }
                } break;
            }
        } else {
            neko_log_warning("Variable not found: %.*s.", token.len, token.text);
        }
        token = neko_lexer_next_token(lex);
        if (token.type != NEKO_TOKEN_SEMICOLON) {
            neko_log_warning("Syntax error. Expecting semicolon, found: %.*s.", token.len, token.text);
            return false;
        }
    } else {
        neko_log_warning("Unidentified color type found: %.*s. (Expect either 'rgba' or 'rgb').", token.len, token.text);
        return false;
    }

    return true;
}

bool _neko_gui_style_sheet_parse_attribute(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, u64 id_tag, s32 elementid, s32 state, neko_gui_ss_variables_t* variables) {
    // Name of attribute
    neko_token_t token = neko_lexer_current_token(lex);

    if (neko_token_compare_text(&token, "color_background") || neko_token_compare_text(&token, "color_border") || neko_token_compare_text(&token, "color_shadow") ||
        neko_token_compare_text(&token, "color_content") || neko_token_compare_text(&token, "color_content_background") || neko_token_compare_text(&token, "color_content_border") ||
        neko_token_compare_text(&token, "color_content_shadow")) {
        if (!_neko_gui_style_sheet_parse_attribute_color(ctx, lex, ss, id_tag, elementid, state, variables)) {
            neko_log_warning("Failed to parse color attribute.");
            return false;
        }
    } else if (neko_token_compare_text(&token, "width") || neko_token_compare_text(&token, "height") || neko_token_compare_text(&token, "padding") || neko_token_compare_text(&token, "padding_left") ||
               neko_token_compare_text(&token, "padding_right") || neko_token_compare_text(&token, "padding_top") || neko_token_compare_text(&token, "padding_bottom") ||
               neko_token_compare_text(&token, "margin") || neko_token_compare_text(&token, "margin_left") || neko_token_compare_text(&token, "margin_right") ||
               neko_token_compare_text(&token, "margin_top") || neko_token_compare_text(&token, "margin_bottom") || neko_token_compare_text(&token, "border") ||
               neko_token_compare_text(&token, "border_left") || neko_token_compare_text(&token, "border_right") || neko_token_compare_text(&token, "border_top") ||
               neko_token_compare_text(&token, "border_bottom") || neko_token_compare_text(&token, "shadow")) {
        if (!_neko_gui_style_sheet_parse_attribute_val(ctx, lex, ss, id_tag, elementid, state, variables)) {
            neko_log_warning("Failed to parse value attribute.");
            return false;
        }
    } else if (neko_token_compare_text(&token, "justify_content") || neko_token_compare_text(&token, "align_content")) {
        if (!_neko_gui_style_sheet_parse_attribute_enum(ctx, lex, ss, id_tag, elementid, state, variables)) {
            neko_log_warning("Failed to parse enum attribute.");
            return false;
        }
    }

    else if (neko_token_compare_text(&token, "font")) {
        if (!_neko_gui_style_sheet_parse_attribute_font(ctx, lex, ss, id_tag, elementid, state, variables)) {
            neko_log_warning("Failed to parse font attribute.");
            return false;
        }
    }

    else if (neko_token_compare_text(&token, "transition")) {
        if (!_neko_gui_style_sheet_parse_attribute_transition(ctx, lex, ss, id_tag, elementid, state, variables)) {
            neko_log_warning("Failed to parse transition attribute.");
            return false;
        }
    } else {
        neko_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
        return false;
    }
    return true;
}

bool _neko_gui_style_sheet_parse_element(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, s32 elementid, neko_gui_ss_variables_t* variables) {
    s32 state = 0x00;
    s32 bc = 0;
    neko_token_t token = neko_lexer_next_token(lex);
    if (token.type == NEKO_TOKEN_COLON) {
        token = neko_lexer_next_token(lex);
        if (token.type != NEKO_TOKEN_IDENTIFIER) {
            neko_log_warning("Unidentified Token. (Expected identifier after colon).");
            neko_token_debug_print(&token);
            return false;
        }

        if (neko_token_compare_text(&token, "focus"))
            state = NEKO_GUI_ELEMENT_STATE_FOCUS;
        else if (neko_token_compare_text(&token, "hover"))
            state = NEKO_GUI_ELEMENT_STATE_HOVER;
        else {
            neko_log_warning("Unidentified element state provided: %.*s", token.len, token.text);
            return false;
        }

        // Get rbrace
        token = neko_lexer_next_token(lex);
    }

    if (token.type != NEKO_TOKEN_LBRACE) {
        neko_log_warning("Unidentified token. (Expected brace after element declaration).");
        neko_token_debug_print(&token);
        return false;
    }

    bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE:
                bc++;
                break;
            case NEKO_TOKEN_RBRACE:
                bc--;
                break;
            case NEKO_TOKEN_IDENTIFIER: {
                // neko_println("Parsing attribute: %.*s", token.len, token.text);
                if (!_neko_gui_style_sheet_parse_attribute(ctx, lex, ss, 0, elementid, state, variables)) {
                    neko_log_warning("Unable to parse attribute");
                    return false;
                }
            } break;
        }
    }

    return true;
}

bool _neko_gui_style_sheet_parse_cid_tag(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, const u64 cid_tag, neko_gui_ss_variables_t* variables) {
    s32 state = 0x00;
    s32 bc = 0;
    neko_token_t token = neko_lexer_next_token(lex);
    if (token.type == NEKO_TOKEN_COLON) {
        token = neko_lexer_next_token(lex);
        if (token.type != NEKO_TOKEN_IDENTIFIER) {
            neko_log_warning("Unidentified Token. (Expected identifier after colon).");
            neko_token_debug_print(&token);
            return false;
        }

        if (neko_token_compare_text(&token, "focus"))
            state = NEKO_GUI_ELEMENT_STATE_FOCUS;
        else if (neko_token_compare_text(&token, "hover"))
            state = NEKO_GUI_ELEMENT_STATE_HOVER;
        else {
            neko_log_warning("Unidentified element state provided: %.*s", token.len, token.text);
            return false;
        }

        // Get rbrace
        token = neko_lexer_next_token(lex);
    }

    if (token.type != NEKO_TOKEN_LBRACE) {
        neko_log_warning("Unidentified token. (Expected brace after element declaration).");
        neko_token_debug_print(&token);
        return false;
    }

    bc = 1;
    while (neko_lexer_can_lex(lex) && bc) {
        token = neko_lexer_next_token(lex);
        switch (token.type) {
            case NEKO_TOKEN_LBRACE:
                bc++;
                break;
            case NEKO_TOKEN_RBRACE:
                bc--;
                break;
            case NEKO_TOKEN_IDENTIFIER: {
                // neko_println("Parsing attribute: %.*s", token.len, token.text);
                if (!_neko_gui_style_sheet_parse_attribute(ctx, lex, ss, cid_tag, NEKO_GUI_ELEMENT_COUNT, state, variables)) {
                    neko_log_warning("Unable to parse attribute");
                    return false;
                }
            } break;
        }
    }

    return true;
}

NEKO_API_DECL neko_gui_style_sheet_t neko_gui_style_sheet_load_from_file(neko_gui_context_t* ctx, const char* file_path) {
    // Generate new style sheet based on default element styles
    neko_gui_style_sheet_t ss = neko_default_val();
    bool success = true;

    size_t sz = 0;
    char* fd = neko_platform_read_file_contents(file_path, "rb", &sz);

    if (!fd) {
        neko_log_warning("Cannot load file: %s", file_path);
        return ss;
    }

    ss = neko_gui_style_sheet_load_from_memory(ctx, fd, sz, &success);

    if (success) {
        neko_log_trace("Successfully loaded style sheet %s.", file_path);
    } else {
        neko_log_warning("Failed to loaded style sheet %s.", file_path);
    }

    neko_safe_free(fd);
    return ss;
}

static bool _neko_gui_style_sheet_parse_variable(neko_gui_context_t* ctx, neko_lexer_t* lex, neko_gui_style_sheet_t* ss, char* name_buf, size_t sz, neko_gui_ss_var_def_t* out) {
    // Get next token, needs to be identifier
    neko_token_t token = neko_lexer_next_token(lex);
    if (token.type != NEKO_TOKEN_IDENTIFIER) {
        neko_log_warning("Unidentified token. (Expected variable name after percent sign).");
        neko_token_debug_print(&token);
        return false;
    }

    // Copy name of variable
    memcpy(name_buf, token.text, neko_min(token.len, sz));

    // Expect colon
    token = neko_lexer_next_token(lex);
    if (token.type != NEKO_TOKEN_COLON) {
        neko_log_warning("Syntax error. (Expected colon name after variable name).");
        neko_token_debug_print(&token);
        return false;
    }

    // Now to get variable
    token = neko_lexer_next_token(lex);
    while (neko_lexer_can_lex(lex) && token.type != NEKO_TOKEN_SEMICOLON) {
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "rgb")) {
                    token = neko_lexer_next_token(lex);
                    if (token.type != NEKO_TOKEN_LPAREN) {
                        neko_log_warning("rgb: missing paren (", token.len, token.text);
                        neko_token_debug_print(&token);
                        return false;
                    }

                    out->type = NEKO_GUI_SS_DEF_COLOR;

                    for (u32 i = 0; i < 3; ++i) {
                        token = neko_lexer_next_token(lex);
                        if (token.type != NEKO_TOKEN_NUMBER) {
                            neko_log_warning("rgb expects numbers", token.len, token.text);
                            neko_token_debug_print(&token);
                            return false;
                        }
                        neko_snprintfc(VAL, 32, "%.*s", token.len, token.text);
                        u8 v = (u8)atoi(VAL);
                        out->val.color.rgba[i] = v;
                    }
                    out->val.color.rgba[3] = 255;
                } else if (neko_token_compare_text(&token, "rgba")) {
                    token = neko_lexer_next_token(lex);
                    if (token.type != NEKO_TOKEN_LPAREN) {
                        neko_log_warning("rgb: missing paren (", token.len, token.text);
                        neko_token_debug_print(&token);
                        return false;
                    }
                    out->type = NEKO_GUI_SS_DEF_COLOR;
                    for (u32 i = 0; i < 4; ++i) {
                        token = neko_lexer_next_token(lex);
                        if (token.type != NEKO_TOKEN_NUMBER) {
                            neko_log_warning("rgb expects numbers", token.len, token.text);
                            neko_token_debug_print(&token);
                            return false;
                        }
                        neko_snprintfc(VAL, 32, "%.*s", token.len, token.text);
                        u8 v = (u8)atoi(VAL);
                        out->val.color.rgba[i] = v;
                    }
                } else if (neko_token_compare_text(&token, "center")) {
                    out->type = NEKO_GUI_SS_DEF_ENUM;
                    out->val.number = (s32)NEKO_GUI_JUSTIFY_CENTER;
                } else if (neko_token_compare_text(&token, "start")) {
                    out->type = NEKO_GUI_SS_DEF_ENUM;
                    out->val.number = (s32)NEKO_GUI_JUSTIFY_START;
                } else if (neko_token_compare_text(&token, "end")) {
                    out->type = NEKO_GUI_SS_DEF_ENUM;
                    out->val.number = (s32)NEKO_GUI_JUSTIFY_END;
                } else {
                    neko_log_warning("Variable value unknown: %.*s", token.len, token.text);
                    neko_token_debug_print(&token);
                    return false;
                }
            } break;

            case NEKO_TOKEN_NUMBER: {
                neko_snprintfc(VAL, 32, "%.*s", token.len, token.text);
                s32 v = (s32)atoi(VAL);
                out->type = NEKO_GUI_SS_DEF_NUMBER;
                out->val.number = v;
            } break;

            case NEKO_TOKEN_STRING: {
                neko_snprintfc(VAL, 64, "%.*s", token.len - 2, token.text + 1);
                out->type = NEKO_GUI_SS_DEF_STR;
                memcpy(out->val.str, VAL, 64);
            }
        }

        token = neko_lexer_next_token(lex);
    }

    return true;
}

// Going to require a lot of parsing
NEKO_API_DECL neko_gui_style_sheet_t neko_gui_style_sheet_load_from_memory(neko_gui_context_t* ctx, const char* fd, size_t sz, bool* sp) {
    // Generate new style sheet based on default element styles
    neko_gui_style_sheet_t ss = neko_default_val();
    bool success = true;

    neko_gui_ss_variables_t variables = neko_default_val();

    // Copy all default styles
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_CONTAINER);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_LABEL);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_TEXT);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_PANEL);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_INPUT);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_BUTTON);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_SCROLL);
    NEKO_GUI_COPY_STYLES(ss.styles, neko_gui_default_style_sheet.styles, NEKO_GUI_ELEMENT_IMAGE);

#define PARSE_ELEMENT(TYPE, TYPESTR)                                                  \
    do {                                                                              \
        if (!_neko_gui_style_sheet_parse_element(ctx, &lex, &ss, TYPE, &variables)) { \
            neko_log_warning("Failed to parse element: %s", TYPESTR);                 \
            success = false;                                                          \
            break;                                                                    \
        }                                                                             \
    } while (0)

    // Parse style sheet for styles
    neko_lexer_t lex = neko_lexer_c_ctor(fd);
    while (success && neko_lexer_c_can_lex(&lex)) {
        neko_token_t token = lex.next_token(&lex);
        switch (token.type) {
            case NEKO_TOKEN_IDENTIFIER: {
                if (neko_token_compare_text(&token, "button"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_BUTTON, "button");
                else if (neko_token_compare_text(&token, "text"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_TEXT, "text");
                else if (neko_token_compare_text(&token, "label"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_LABEL, "label");
                else if (neko_token_compare_text(&token, "image"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_IMAGE, "image");
                else if (neko_token_compare_text(&token, "scroll"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_SCROLL, "scroll");
                else if (neko_token_compare_text(&token, "panel"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_PANEL, "panel");
                else if (neko_token_compare_text(&token, "container"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_CONTAINER, "container");
                else if (neko_token_compare_text(&token, "input"))
                    PARSE_ELEMENT(NEKO_GUI_ELEMENT_INPUT, "input");
                else {
                    neko_log_warning("Unidentified token. (Invalid element type found).");
                    neko_token_debug_print(&token);
                    break;
                }

            } break;

            case NEKO_TOKEN_PERIOD: {
                // Do single class for now
                neko_token_t cls_tag = neko_lexer_next_token(&lex);
                char CLS_TAG[256] = neko_default_val();
                CLS_TAG[0] = '.';
                memcpy(CLS_TAG + 1, cls_tag.text, cls_tag.len);
                u64 cls_hash = neko_hash_str64(CLS_TAG);
                if (!_neko_gui_style_sheet_parse_cid_tag(ctx, &lex, &ss, cls_hash, &variables)) {
                    neko_log_warning("Failed to parse id tag: %s", CLS_TAG);
                    success = false;
                    break;
                }

            } break;

            case NEKO_TOKEN_HASH: {
                neko_token_t id_tag = neko_lexer_next_token(&lex);
                char ID_TAG[256] = neko_default_val();
                ID_TAG[0] = '#';
                memcpy(ID_TAG + 1, id_tag.text, id_tag.len);
                u64 id_hash = neko_hash_str64(ID_TAG);
                if (!_neko_gui_style_sheet_parse_cid_tag(ctx, &lex, &ss, id_hash, &variables)) {
                    neko_log_warning("Failed to parse id tag: %s", ID_TAG);
                    success = false;
                    break;
                }
            } break;

            case NEKO_TOKEN_ASTERISK: {
                // Save token
                neko_token_t token = neko_lexer_next_token(&lex);
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_CONTAINER, "* (container)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_TEXT, "* (text)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_LABEL, "* (label)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_IMAGE, "* (image)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_BUTTON, "* (button)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_PANEL, "* (panel)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_INPUT, "* (input)");
                neko_lexer_set_token(&lex, token);
                PARSE_ELEMENT(NEKO_GUI_ELEMENT_SCROLL, "* (scroll)");
            } break;

            case NEKO_TOKEN_DOLLAR: {
                neko_gui_ss_var_def_t variable = neko_default_val();
                char variable_name[256] = neko_default_val();
                if (!_neko_gui_style_sheet_parse_variable(ctx, &lex, &ss, variable_name, sizeof(variable_name), &variable)) {
                    neko_log_warning("Failed to parse variable: %s", variable_name);
                    success = false;
                    break;
                } else {
                    neko_hash_table_insert(variables.variables, neko_hash_str64(variable_name), variable);
                }

                /*
                typedef enum
                {
                    NEKO_GUI_SS_DEF_VALUE = 0x00,
                    NEKO_GUI_SS_DEF_COLOR,
                    NEKO_GUI_SS_DEF_STR
                } neko_gui_ss_var_def_type;

                typedef struct
                {
                    neko_gui_ss_var_def_type type;
                    union {
                        s32 value;
                        neko_color_t color;
                        char str[64];
                    } val;
                } neko_gui_ss_var_def_t;
                */

            } break;
        }
    }

    if (!sp) {
        if (success) {
            neko_log_trace("Successfully loaded style sheet from memory.");
        } else {
            neko_log_warning("Failed to loaded style sheet from memory.");
        }
    } else {
        *sp = success;
    }

    if (variables.variables) neko_hash_table_free(variables.variables);

    return ss;
}