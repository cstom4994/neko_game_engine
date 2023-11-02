

#ifndef NEKO_IMGUI_H
#define NEKO_IMGUI_H

#include "engine/neko.h"
#include "engine/util/neko_idraw.h"

// Modified from microui(https://github.com/rxi/microui) and ImGui(https://github.com/ocornut/imgui)

#define NEKO_IMGUI_SPLIT_SIZE 2.f
#define NEKO_IMGUI_MAX_CNT 48
#define NEKO_IMGUI_COMMANDLIST_SIZE (256 * 1024)
#define NEKO_IMGUI_ROOTLIST_SIZE 32
#define NEKO_IMGUI_CONTAINERSTACK_SIZE 32
#define NEKO_IMGUI_CLIPSTACK_SIZE 32
#define NEKO_IMGUI_IDSTACK_SIZE 32
#define NEKO_IMGUI_LAYOUTSTACK_SIZE 16
#define NEKO_IMGUI_CONTAINERPOOL_SIZE 48
#define NEKO_IMGUI_TREENODEPOOL_SIZE 48
#define NEKO_IMGUI_NEKO_IMGUI_SPLIT_SIZE 32
#define NEKO_IMGUI_NEKO_IMGUI_TAB_SIZE 32
#define NEKO_IMGUI_MAX_WIDTHS 16
#define NEKO_IMGUI_REAL f32
#define NEKO_IMGUI_REAL_FMT "%.3g"
#define NEKO_IMGUI_SLIDER_FMT "%.2f"
#define NEKO_IMGUI_MAX_FMT 127
#define NEKO_IMGUI_TAB_ITEM_MAX 24
#define NEKO_IMGUI_CLS_SELECTOR_MAX 4

#define neko_imgui_widths(...)                      \
    [&]() -> const s32* {                         \
        static s32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()

#define neko_imgui_stack(T, n) \
    struct {                 \
        s32 idx;             \
        T items[n];          \
    }

enum { NEKO_IMGUI_CLIP_PART = 1, NEKO_IMGUI_CLIP_ALL };

enum {
    NEKO_IMGUI_COMMAND_JUMP = 1,
    NEKO_IMGUI_COMMAND_CLIP,
    NEKO_IMGUI_COMMAND_SHAPE,
    NEKO_IMGUI_COMMAND_TEXT,
    NEKO_IMGUI_COMMAND_ICON,
    NEKO_IMGUI_COMMAND_IMAGE,
    NEKO_IMGUI_COMMAND_CUSTOM,
    NEKO_IMGUI_COMMAND_PIPELINE,
    NEKO_IMGUI_COMMAND_UNIFORMS,
    NEKO_IMGUI_COMMAND_MAX
};

enum { NEKO_IMGUI_SHAPE_RECT = 1, NEKO_IMGUI_SHAPE_CIRCLE, NEKO_IMGUI_SHAPE_TRIANGLE, NEKO_IMGUI_SHAPE_LINE };

enum { NEKO_IMGUI_TRANSFORM_WORLD = 0x00, NEKO_IMGUI_TRANSFORM_LOCAL };

enum { NEKO_IMGUI_GIZMO_TRANSLATE = 0x00, NEKO_IMGUI_GIZMO_ROTATE, NEKO_IMGUI_GIZMO_SCALE };

enum {
    NEKO_IMGUI_COLOR_BACKGROUND = 0x00,
    NEKO_IMGUI_COLOR_CONTENT,
    NEKO_IMGUI_COLOR_BORDER,
    NEKO_IMGUI_COLOR_SHADOW,
    NEKO_IMGUI_COLOR_CONTENT_BACKGROUND,
    NEKO_IMGUI_COLOR_CONTENT_SHADOW,
    NEKO_IMGUI_COLOR_CONTENT_BORDER,

    NEKO_IMGUI_COLOR_MAX
};

enum { NEKO_IMGUI_ICON_CLOSE = 1, NEKO_IMGUI_ICON_CHECK, NEKO_IMGUI_ICON_COLLAPSED, NEKO_IMGUI_ICON_EXPANDED, NEKO_IMGUI_ICON_MAX };

enum { NEKO_IMGUI_RES_ACTIVE = (1 << 0), NEKO_IMGUI_RES_SUBMIT = (1 << 1), NEKO_IMGUI_RES_CHANGE = (1 << 2) };

typedef enum neko_imgui_alt_drag_mode_type {
    NEKO_IMGUI_ALT_DRAG_QUAD = 0x00,  // Quadrants
    NEKO_IMGUI_ALT_DRAG_NINE,         // Nine splice the window
    NEKO_IMGUI_ALT_DRAG_SINGLE        // Single window drag (controls the width/height, leaving x/y position of window in tact)
} neko_imgui_alt_drag_mode_type;

enum {
    NEKO_IMGUI_OPT_NOSTYLESHADOW = (1 << 0),
    NEKO_IMGUI_OPT_NOSTYLEBORDER = (1 << 1),
    NEKO_IMGUI_OPT_NOINTERACT = (1 << 2),
    NEKO_IMGUI_OPT_NOFRAME = (1 << 3),
    NEKO_IMGUI_OPT_NORESIZE = (1 << 4),
    NEKO_IMGUI_OPT_NOSCROLL = (1 << 5),
    NEKO_IMGUI_OPT_NOCLOSE = (1 << 6),
    NEKO_IMGUI_OPT_NOTITLE = (1 << 7),
    NEKO_IMGUI_OPT_HOLDFOCUS = (1 << 8),
    NEKO_IMGUI_OPT_AUTOSIZE = (1 << 9),
    NEKO_IMGUI_OPT_POPUP = (1 << 10),
    NEKO_IMGUI_OPT_CLOSED = (1 << 11),
    NEKO_IMGUI_OPT_EXPANDED = (1 << 12),
    NEKO_IMGUI_OPT_NOHOVER = (1 << 13),
    NEKO_IMGUI_OPT_FORCESETRECT = (1 << 14),
    NEKO_IMGUI_OPT_NOFOCUS = (1 << 15),
    NEKO_IMGUI_OPT_FORCEFOCUS = (1 << 16),
    NEKO_IMGUI_OPT_NOMOVE = (1 << 17),
    NEKO_IMGUI_OPT_NOCLIP = (1 << 18),
    NEKO_IMGUI_OPT_NODOCK = (1 << 19),
    NEKO_IMGUI_OPT_FULLSCREEN = (1 << 20),
    NEKO_IMGUI_OPT_DOCKSPACE = (1 << 21),
    NEKO_IMGUI_OPT_NOBRINGTOFRONT = (1 << 22),
    NEKO_IMGUI_OPT_LEFTCLICKONLY = (1 << 23),
    NEKO_IMGUI_OPT_NOSWITCHSTATE = (1 << 24),
    NEKO_IMGUI_OPT_NOBORDER = (1 << 25),
    NEKO_IMGUI_OPT_ISCONTENT = (1 << 26),
    NEKO_IMGUI_OPT_NOCARET = (1 << 27),
    NEKO_IMGUI_OPT_NOSCROLLHORIZONTAL = (1 << 28),
    NEKO_IMGUI_OPT_NOSCROLLVERTICAL = (1 << 29),
    NEKO_IMGUI_OPT_NOSTYLEBACKGROUND = (1 << 30)
};

enum { NEKO_IMGUI_MOUSE_LEFT = (1 << 0), NEKO_IMGUI_MOUSE_RIGHT = (1 << 1), NEKO_IMGUI_MOUSE_MIDDLE = (1 << 2) };

enum { NEKO_IMGUI_KEY_SHIFT = (1 << 0), NEKO_IMGUI_KEY_CTRL = (1 << 1), NEKO_IMGUI_KEY_ALT = (1 << 2), NEKO_IMGUI_KEY_BACKSPACE = (1 << 3), NEKO_IMGUI_KEY_RETURN = (1 << 4) };

#define NEKO_IMGUI_OPT_NOSTYLING (NEKO_IMGUI_OPT_NOSTYLEBORDER | NEKO_IMGUI_OPT_NOSTYLEBACKGROUND | NEKO_IMGUI_OPT_NOSTYLESHADOW)

typedef struct neko_imgui_context_t neko_imgui_context_t;
typedef u32 neko_imgui_id;
typedef NEKO_IMGUI_REAL neko_imgui_real;

// Shapes
typedef struct {
    float x, y, w, h;
} neko_imgui_rect_t;
typedef struct {
    float radius;
    neko_vec2 center;
} neko_imgui_circle_t;
typedef struct {
    neko_vec2 points[3];
} neko_imgui_triangle_t;
typedef struct {
    neko_vec2 start;
    neko_vec2 end;
} neko_imgui_line_t;

typedef struct {
    neko_imgui_id id;
    s32 last_update;
} neko_imgui_pool_item_t;

typedef struct {
    s32 type, size;
} neko_imgui_basecommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    void* dst;
} neko_imgui_jumpcommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    neko_imgui_rect_t rect;
} neko_imgui_clipcommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    neko_asset_ascii_font_t* font;
    neko_vec2 pos;
    neko_color_t color;
    char str[1];
} neko_imgui_textcommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    neko_handle(neko_graphics_pipeline_t) pipeline;
    neko_idraw_layout_type layout_type;
    void* layout;
    size_t layout_sz;
} neko_imgui_pipelinecommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    void* data;
    size_t sz;
} neko_imgui_binduniformscommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    neko_imgui_rect_t rect;
    neko_handle(neko_graphics_texture_t) hndl;
    neko_vec4 uvs;
    neko_color_t color;
} neko_imgui_imagecommand_t;

struct neko_imgui_customcommand_t;

// Draw Callback
typedef void (*neko_imgui_draw_callback_t)(neko_imgui_context_t* ctx, struct neko_imgui_customcommand_t* cmd);

typedef struct neko_imgui_customcommand_t {
    neko_imgui_basecommand_t base;
    neko_imgui_rect_t clip;
    neko_imgui_rect_t viewport;
    neko_imgui_id hash;
    neko_imgui_id hover;
    neko_imgui_id focus;
    neko_imgui_draw_callback_t cb;
    void* data;
    size_t sz;
} neko_imgui_customcommand_t;

typedef struct {
    neko_imgui_basecommand_t base;
    u32 type;
    union {
        neko_imgui_rect_t rect;
        neko_imgui_circle_t circle;
        neko_imgui_triangle_t triangle;
        neko_imgui_line_t line;
    };
    neko_color_t color;
} neko_imgui_shapecommand_t;

// NOTE: This is wasteful, given how I'm pushing into the byte buffer anyway for heterogenous types
typedef union {
    s32 type;
    neko_imgui_basecommand_t base;
    neko_imgui_jumpcommand_t jump;
    neko_imgui_clipcommand_t clip;
    neko_imgui_shapecommand_t shape;
    neko_imgui_textcommand_t text;
    neko_imgui_imagecommand_t image;
    neko_imgui_customcommand_t custom;
    neko_imgui_pipelinecommand_t pipeline;
    neko_imgui_binduniformscommand_t uniforms;
} neko_imgui_command_t;

struct neko_imgui_context_t;

typedef void (*neko_imgui_on_draw_button_callback)(struct neko_imgui_context_t* ctx, neko_imgui_rect_t rect, neko_imgui_id id, bool hovered, bool focused, s32 opt, const char* label, s32 icon);

typedef enum {
    NEKO_IMGUI_LAYOUT_ANCHOR_TOPLEFT = 0x00,
    NEKO_IMGUI_LAYOUT_ANCHOR_TOPCENTER,
    NEKO_IMGUI_LAYOUT_ANCHOR_TOPRIGHT,
    NEKO_IMGUI_LAYOUT_ANCHOR_LEFT,
    NEKO_IMGUI_LAYOUT_ANCHOR_CENTER,
    NEKO_IMGUI_LAYOUT_ANCHOR_RIGHT,
    NEKO_IMGUI_LAYOUT_ANCHOR_BOTTOMLEFT,
    NEKO_IMGUI_LAYOUT_ANCHOR_BOTTOMCENTER,
    NEKO_IMGUI_LAYOUT_ANCHOR_BOTTOMRIGHT
} neko_imgui_layout_anchor_type;

typedef enum { NEKO_IMGUI_ALIGN_START = 0x00, NEKO_IMGUI_ALIGN_CENTER, NEKO_IMGUI_ALIGN_END } neko_imgui_align_type;

typedef enum { NEKO_IMGUI_JUSTIFY_START = 0x00, NEKO_IMGUI_JUSTIFY_CENTER, NEKO_IMGUI_JUSTIFY_END } neko_imgui_justification_type;

typedef enum { NEKO_IMGUI_DIRECTION_COLUMN = 0x00, NEKO_IMGUI_DIRECTION_ROW, NEKO_IMGUI_DIRECTION_COLUMN_REVERSE, NEKO_IMGUI_DIRECTION_ROW_REVERSE } neko_imgui_direction;

typedef struct neko_imgui_layout_t {
    neko_imgui_rect_t body;
    neko_imgui_rect_t next;
    neko_vec2 position;
    neko_vec2 size;
    neko_vec2 max;
    s32 padding[4];
    s32 widths[NEKO_IMGUI_MAX_WIDTHS];
    s32 items;
    s32 item_index;
    s32 next_row;
    s32 next_type;
    s32 indent;

    // flex direction / justification / alignment
    s32 direction;
    s32 justify_content;
    s32 align_content;

} neko_imgui_layout_t;

// Forward decl.
struct neko_imgui_container_t;

typedef enum neko_imgui_split_node_type { NEKO_IMGUI_SPLIT_NODE_CONTAINER = 0x00, NEKO_IMGUI_SPLIT_NODE_SPLIT } neko_imgui_split_node_type;

enum { NEKO_IMGUI_SPLIT_NODE_CHILD = 0x00, NEKO_IMGUI_SPLIT_NODE_PARENT };

typedef struct neko_imgui_split_node_t {
    neko_imgui_split_node_type type;
    union {
        u32 split;
        struct neko_imgui_container_t* container;
    };
} neko_imgui_split_node_t;

typedef enum neko_imgui_split_type { NEKO_IMGUI_SPLIT_LEFT = 0x00, NEKO_IMGUI_SPLIT_RIGHT, NEKO_IMGUI_SPLIT_TOP, NEKO_IMGUI_SPLIT_BOTTOM, NEKO_IMGUI_SPLIT_TAB } neko_imgui_split_type;

typedef struct neko_imgui_split_t {
    neko_imgui_split_type type;  // NEKO_IMGUI_SPLIT_LEFT, NEKO_IMGUI_SPLIT_RIGHT, NEKO_IMGUI_SPLIT_TAB, NEKO_IMGUI_SPLIT_BOTTOM, NEKO_IMGUI_SPLIT_TOP
    float ratio;               // Split ratio between children [0.f, 1.f], (left node = ratio), right node = (1.f - ratio)
    neko_imgui_rect_t rect;
    neko_imgui_rect_t prev_rect;
    neko_imgui_split_node_t children[2];
    u32 parent;
    u32 id;
    u32 zindex;
    s32 frame;
} neko_imgui_split_t;

typedef enum neko_imgui_window_flags {
    NEKO_IMGUI_WINDOW_FLAGS_VISIBLE = (1 << 0),     //
    NEKO_IMGUI_WINDOW_FLAGS_FIRST_INIT = (1 << 1),  //
    NEKO_IMGUI_WINDOW_FLAGS_PUSH_ID = (1 << 2)      //
} neko_imgui_window_flags;

// Equidistantly sized tabs, based on rect of window
typedef struct neko_imgui_tab_item_t {
    neko_imgui_id tab_bar;
    u32 zindex;  // Sorting index in tab bar
    void* data;  // User set data pointer for this item
    u32 idx;     // Internal index
} neko_imgui_tab_item_t;

typedef struct neko_imgui_tab_bar_t {
    neko_imgui_tab_item_t items[NEKO_IMGUI_TAB_ITEM_MAX];
    u32 size;              // Current number of items in tab bar
    neko_imgui_rect_t rect;  // Cached sized for tab bar
    u32 focus;             // Focused item in tab bar
} neko_imgui_tab_bar_t;

typedef struct neko_imgui_container_t {
    neko_imgui_command_t *head, *tail;
    neko_imgui_rect_t rect;
    neko_imgui_rect_t body;
    neko_vec2 content_size;
    neko_vec2 scroll;
    s32 zindex;
    s32 open;
    neko_imgui_id id;
    neko_imgui_id split;  // If container is docked, then will have owning split to get sizing (0x00 for NULL)
    u32 tab_bar;
    u32 tab_item;
    struct neko_imgui_container_t* parent;  // Owning parent (for tabbing)
    u64 opt;
    u32 frame;
    u32 visible;
    s32 flags;
    char name[256];
} neko_imgui_container_t;

typedef enum {
    NEKO_IMGUI_ELEMENT_STATE_NEG = -1,
    NEKO_IMGUI_ELEMENT_STATE_DEFAULT = 0x00,
    NEKO_IMGUI_ELEMENT_STATE_HOVER,
    NEKO_IMGUI_ELEMENT_STATE_FOCUS,
    NEKO_IMGUI_ELEMENT_STATE_COUNT,
    NEKO_IMGUI_ELEMENT_STATE_ON_HOVER,
    NEKO_IMGUI_ELEMENT_STATE_ON_FOCUS,
    NEKO_IMGUI_ELEMENT_STATE_OFF_HOVER,
    NEKO_IMGUI_ELEMENT_STATE_OFF_FOCUS
} neko_imgui_element_state;

typedef enum neko_imgui_element_type {
    NEKO_IMGUI_ELEMENT_CONTAINER = 0x00,
    NEKO_IMGUI_ELEMENT_LABEL,
    NEKO_IMGUI_ELEMENT_TEXT,
    NEKO_IMGUI_ELEMENT_PANEL,
    NEKO_IMGUI_ELEMENT_INPUT,
    NEKO_IMGUI_ELEMENT_BUTTON,
    NEKO_IMGUI_ELEMENT_SCROLL,
    NEKO_IMGUI_ELEMENT_IMAGE,
    NEKO_IMGUI_ELEMENT_COUNT
} neko_imgui_element_type;

typedef enum { NEKO_IMGUI_PADDING_LEFT = 0x00, NEKO_IMGUI_PADDING_RIGHT, NEKO_IMGUI_PADDING_TOP, NEKO_IMGUI_PADDING_BOTTOM } neko_imgui_padding_type;

typedef enum { NEKO_IMGUI_MARGIN_LEFT = 0x00, NEKO_IMGUI_MARGIN_RIGHT, NEKO_IMGUI_MARGIN_TOP, NEKO_IMGUI_MARGIN_BOTTOM } neko_imgui_margin_type;

typedef enum {

    // Width/Height
    NEKO_IMGUI_STYLE_WIDTH = 0x00,
    NEKO_IMGUI_STYLE_HEIGHT,

    // Padding
    NEKO_IMGUI_STYLE_PADDING,
    NEKO_IMGUI_STYLE_PADDING_LEFT,
    NEKO_IMGUI_STYLE_PADDING_RIGHT,
    NEKO_IMGUI_STYLE_PADDING_TOP,
    NEKO_IMGUI_STYLE_PADDING_BOTTOM,

    NEKO_IMGUI_STYLE_MARGIN,  // Can set margin for all at once, if -1.f then will assume 'auto' to simulate standard css
    NEKO_IMGUI_STYLE_MARGIN_LEFT,
    NEKO_IMGUI_STYLE_MARGIN_RIGHT,
    NEKO_IMGUI_STYLE_MARGIN_TOP,
    NEKO_IMGUI_STYLE_MARGIN_BOTTOM,

    // Border Radius
    NEKO_IMGUI_STYLE_BORDER_RADIUS,
    NEKO_IMGUI_STYLE_BORDER_RADIUS_LEFT,
    NEKO_IMGUI_STYLE_BORDER_RADIUS_RIGHT,
    NEKO_IMGUI_STYLE_BORDER_RADIUS_TOP,
    NEKO_IMGUI_STYLE_BORDER_RADIUS_BOTTOM,

    // Border Width
    NEKO_IMGUI_STYLE_BORDER_WIDTH,
    NEKO_IMGUI_STYLE_BORDER_WIDTH_LEFT,
    NEKO_IMGUI_STYLE_BORDER_WIDTH_RIGHT,
    NEKO_IMGUI_STYLE_BORDER_WIDTH_TOP,
    NEKO_IMGUI_STYLE_BORDER_WIDTH_BOTTOM,

    // Text
    NEKO_IMGUI_STYLE_TEXT_ALIGN,

    // Flex
    NEKO_IMGUI_STYLE_DIRECTION,
    NEKO_IMGUI_STYLE_ALIGN_CONTENT,
    NEKO_IMGUI_STYLE_JUSTIFY_CONTENT,  // Justify runs parallel to direction (ex. for row, left to right)

    // Shadow
    NEKO_IMGUI_STYLE_SHADOW_X,
    NEKO_IMGUI_STYLE_SHADOW_Y,

    // Colors
    NEKO_IMGUI_STYLE_COLOR_BACKGROUND,
    NEKO_IMGUI_STYLE_COLOR_BORDER,
    NEKO_IMGUI_STYLE_COLOR_SHADOW,
    NEKO_IMGUI_STYLE_COLOR_CONTENT,
    NEKO_IMGUI_STYLE_COLOR_CONTENT_BACKGROUND,
    NEKO_IMGUI_STYLE_COLOR_CONTENT_BORDER,
    NEKO_IMGUI_STYLE_COLOR_CONTENT_SHADOW,

    // Font
    NEKO_IMGUI_STYLE_FONT,

    NEKO_IMGUI_STYLE_COUNT

} neko_imgui_style_element_type;

enum { NEKO_IMGUI_ANIMATION_DIRECTION_FORWARD = 0x00, NEKO_IMGUI_ANIMATION_DIRECTION_BACKWARD };

typedef struct {
    neko_imgui_style_element_type type;
    union {
        s32 value;
        neko_color_t color;
        neko_asset_ascii_font_t* font;
    };
} neko_imgui_style_element_t;

typedef struct neko_imgui_animation_t {
    s16 max;          // max time
    s16 time;         // current time
    s16 delay;        // delay
    s16 curve;        // curve type
    s16 direction;    // current direction
    s16 playing;      // whether or not active
    s16 iterations;   // number of iterations to play the animation
    s16 focus_state;  // cached focus_state from frame (want to delete this somehow)
    s16 hover_state;  // cached hover_state from frame (want to delete this somehow)
    s16 start_state;  // starting state for animation blend
    s16 end_state;    // ending state for animation blend
    s32 frame;        // current frame (to match)
} neko_imgui_animation_t;

typedef struct neko_imgui_style_t {
    // font
    neko_asset_ascii_font_t* font;

    // dimensions
    float size[2];
    s16 spacing;         // get rid of    (use padding)
    s16 indent;          // get rid of    (use margin)
    s16 title_height;    // get rid of    (use title_bar style)
    s16 scrollbar_size;  // get rid of    (use scroll style)
    s16 thumb_size;      // get rid of    (use various styles)

    // colors
    neko_color_t colors[NEKO_IMGUI_COLOR_MAX];

    // padding/margin
    s32 padding[4];
    s16 margin[4];

    // border
    s16 border_width[4];
    s16 border_radius[4];

    // flex direction / justification / alignment
    s16 direction;
    s16 justify_content;
    s16 align_content;

    // shadow amount each direction
    s16 shadow_x;
    s16 shadow_y;

} neko_imgui_style_t;

// Keep animation properties lists within style sheet to look up

typedef struct neko_imgui_animation_property_t {
    neko_imgui_style_element_type type;
    s16 time;
    s16 delay;
} neko_imgui_animation_property_t;

typedef struct neko_imgui_animation_property_list_t {
    neko_dyn_array(neko_imgui_animation_property_t) properties[3];
} neko_imgui_animation_property_list_t;

/*
   element type
   classes
   id

   neko_imgui_button(gui, "Text##.cls#id");
   neko_imgui_label(gui, "Title###title");

    button .class #id : hover {         // All of these styles get concat into one?
    }
*/

typedef struct {
    neko_dyn_array(neko_imgui_style_element_t) styles[3];
} neko_imgui_style_list_t;

typedef struct neko_imgui_style_sheet_t {
    neko_imgui_style_t styles[NEKO_IMGUI_ELEMENT_COUNT][3];  // default | hovered | focused
    neko_hash_table(neko_imgui_element_type, neko_imgui_animation_property_list_t) animations;

    neko_hash_table(u64, neko_imgui_style_list_t) cid_styles;
    neko_hash_table(u64, neko_imgui_animation_property_list_t) cid_animations;
} neko_imgui_style_sheet_t;

typedef struct neko_imgui_style_sheet_element_desc_t {

    struct {

        struct {
            neko_imgui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_imgui_animation_property_t* data;
            size_t size;
        } animation;

    } all;

    struct {

        struct {
            neko_imgui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_imgui_animation_property_t* data;
            size_t size;
        } animation;

    } def;

    struct {
        struct {
            neko_imgui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_imgui_animation_property_t* data;
            size_t size;
        } animation;
    } hover;

    struct {
        struct {
            neko_imgui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_imgui_animation_property_t* data;
            size_t size;
        } animation;
    } focus;

} neko_imgui_style_sheet_element_desc_t;

typedef neko_imgui_style_sheet_element_desc_t neko_imgui_inline_style_desc_t;

typedef struct neko_imgui_style_sheet_desc_t {
    neko_imgui_style_sheet_element_desc_t container;
    neko_imgui_style_sheet_element_desc_t button;
    neko_imgui_style_sheet_element_desc_t panel;
    neko_imgui_style_sheet_element_desc_t input;
    neko_imgui_style_sheet_element_desc_t text;
    neko_imgui_style_sheet_element_desc_t label;
    neko_imgui_style_sheet_element_desc_t scroll;
    neko_imgui_style_sheet_element_desc_t tab;
    neko_imgui_style_sheet_element_desc_t menu;
    neko_imgui_style_sheet_element_desc_t title;
    neko_imgui_style_sheet_element_desc_t image;
} neko_imgui_style_sheet_desc_t;

typedef enum neko_imgui_request_type {
    NEKO_IMGUI_SPLIT_NEW = 0x01,
    NEKO_IMGUI_CNT_MOVE,
    NEKO_IMGUI_CNT_FOCUS,
    NEKO_IMGUI_SPLIT_MOVE,
    NEKO_IMGUI_SPLIT_RESIZE_SW,
    NEKO_IMGUI_SPLIT_RESIZE_SE,
    NEKO_IMGUI_SPLIT_RESIZE_NW,
    NEKO_IMGUI_SPLIT_RESIZE_NE,
    NEKO_IMGUI_SPLIT_RESIZE_W,
    NEKO_IMGUI_SPLIT_RESIZE_E,
    NEKO_IMGUI_SPLIT_RESIZE_N,
    NEKO_IMGUI_SPLIT_RESIZE_S,
    NEKO_IMGUI_SPLIT_RESIZE_CENTER,
    NEKO_IMGUI_SPLIT_RATIO,
    NEKO_IMGUI_SPLIT_RESIZE_INVALID,
    NEKO_IMGUI_TAB_SWAP_LEFT,
    NEKO_IMGUI_TAB_SWAP_RIGHT
} neko_imgui_request_type;

typedef struct neko_imgui_request_t {
    neko_imgui_request_type type;
    union {
        neko_imgui_split_type split_type;
        neko_imgui_split_t* split;
        neko_imgui_container_t* cnt;
    };
    u32 frame;
} neko_imgui_request_t;

typedef struct neko_imgui_inline_style_stack_t {
    neko_dyn_array(neko_imgui_style_element_t) styles[3];
    neko_dyn_array(neko_imgui_animation_property_t) animations[3];
    neko_dyn_array(u32) style_counts;      // amount of styles to pop off at "top of stack" for each state
    neko_dyn_array(u32) animation_counts;  // amount of animations to pop off at "top of stack" for each state
} neko_imgui_inline_style_stack_t;

typedef struct neko_imgui_context_t {
    // Core state
    neko_imgui_style_t* style;              // Active style
    neko_imgui_style_sheet_t* style_sheet;  // Active style sheet
    neko_imgui_id hover;
    neko_imgui_id focus;
    neko_imgui_id last_id;
    neko_imgui_id state_switch_id;  // Id that had a state switch
    s32 switch_state;
    neko_imgui_id lock_focus;
    s32 last_hover_state;
    s32 last_focus_state;
    neko_imgui_id prev_hover;
    neko_imgui_id prev_focus;
    neko_imgui_rect_t last_rect;
    s32 last_zindex;
    s32 updated_focus;
    s32 frame;
    neko_vec2 framebuffer_size;
    neko_imgui_rect_t viewport;
    neko_imgui_container_t* active_root;
    neko_imgui_container_t* hover_root;
    neko_imgui_container_t* next_hover_root;
    neko_imgui_container_t* scroll_target;
    neko_imgui_container_t* focus_root;
    neko_imgui_container_t* next_focus_root;
    neko_imgui_container_t* dockable_root;
    neko_imgui_container_t* prev_dockable_root;
    neko_imgui_container_t* docked_root;
    neko_imgui_container_t* undock_root;
    neko_imgui_split_t* focus_split;
    neko_imgui_split_t* next_hover_split;
    neko_imgui_split_t* hover_split;
    neko_imgui_id next_lock_hover_id;
    neko_imgui_id lock_hover_id;
    char number_edit_buf[NEKO_IMGUI_MAX_FMT];
    neko_imgui_id number_edit;
    neko_imgui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(neko_imgui_request_t) requests;

    // Stacks
    neko_imgui_stack(u8, NEKO_IMGUI_COMMANDLIST_SIZE) command_list;
    neko_imgui_stack(neko_imgui_container_t*, NEKO_IMGUI_ROOTLIST_SIZE) root_list;
    neko_imgui_stack(neko_imgui_container_t*, NEKO_IMGUI_CONTAINERSTACK_SIZE) container_stack;
    neko_imgui_stack(neko_imgui_rect_t, NEKO_IMGUI_CLIPSTACK_SIZE) clip_stack;
    neko_imgui_stack(neko_imgui_id, NEKO_IMGUI_IDSTACK_SIZE) id_stack;
    neko_imgui_stack(neko_imgui_layout_t, NEKO_IMGUI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(neko_imgui_element_type, neko_imgui_inline_style_stack_t) inline_styles;

    // Retained state pools
    neko_imgui_pool_item_t container_pool[NEKO_IMGUI_CONTAINERPOOL_SIZE];
    neko_imgui_container_t containers[NEKO_IMGUI_CONTAINERPOOL_SIZE];
    neko_imgui_pool_item_t treenode_pool[NEKO_IMGUI_TREENODEPOOL_SIZE];

    neko_slot_array(neko_imgui_split_t) splits;
    neko_slot_array(neko_imgui_tab_bar_t) tab_bars;

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
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(neko_imgui_id, neko_imgui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, neko_asset_ascii_font_t*) font_stash;

    // Callbacks
    struct {
        neko_imgui_on_draw_button_callback button;
    } callbacks;

} neko_imgui_context_t;

typedef struct {
    // Core state
    neko_imgui_style_t* style;              // Active style
    neko_imgui_style_sheet_t* style_sheet;  // Active style sheet
    neko_imgui_id hover;
    neko_imgui_id focus;
    neko_imgui_id last_id;
    neko_imgui_id lock_focus;
    neko_imgui_rect_t last_rect;
    s32 last_zindex;
    s32 updated_focus;
    s32 frame;
    neko_imgui_container_t* hover_root;
    neko_imgui_container_t* next_hover_root;
    neko_imgui_container_t* scroll_target;
    neko_imgui_container_t* focus_root;
    neko_imgui_container_t* next_focus_root;
    neko_imgui_container_t* dockable_root;
    neko_imgui_container_t* prev_dockable_root;
    neko_imgui_container_t* docked_root;
    neko_imgui_container_t* undock_root;
    neko_imgui_split_t* focus_split;
    neko_imgui_split_t* next_hover_split;
    neko_imgui_split_t* hover_split;
    neko_imgui_id next_lock_hover_id;
    neko_imgui_id lock_hover_id;
    char number_edit_buf[NEKO_IMGUI_MAX_FMT];
    neko_imgui_id number_edit;
    neko_imgui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(neko_imgui_request_t) requests;

    // Stacks
    neko_imgui_stack(neko_imgui_container_t*, NEKO_IMGUI_CONTAINERSTACK_SIZE) container_stack;

    neko_dyn_array(u8) command_list;
    neko_dyn_array(neko_imgui_container_t*) root_list;
    neko_dyn_array(neko_imgui_rect_t) clip_stack;
    neko_dyn_array(neko_imgui_id) id_stack;
    neko_dyn_array(neko_imgui_layout_t) layout_stack;

    // Retained state pools
    neko_imgui_pool_item_t container_pool[NEKO_IMGUI_CONTAINERPOOL_SIZE];
    neko_imgui_pool_item_t treenode_pool[NEKO_IMGUI_TREENODEPOOL_SIZE];

    neko_slot_array(neko_imgui_split_t) splits;
    neko_slot_array(neko_imgui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    s16 mouse_down;
    s16 mouse_pressed;
    s16 key_down;
    s16 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(neko_imgui_id, neko_imgui_animation_t) animations;

    // Callbacks
    struct {
        neko_imgui_on_draw_button_callback button;
    } callbacks;

} neko_imgui_context_pruned_t;

typedef struct {
    const char* key;
    neko_asset_ascii_font_t* font;
} neko_imgui_font_desc_t;

typedef struct {
    neko_imgui_font_desc_t* fonts;
    size_t size;
} neko_imgui_font_stash_desc_t;

typedef struct {
    const char* id;                                  // Id selector
    const char* classes[NEKO_IMGUI_CLS_SELECTOR_MAX];  // Class selectors
} neko_imgui_selector_desc_t;

enum { NEKO_IMGUI_HINT_FLAG_NO_SCALE_BIAS_MOUSE = (1 << 0), NEKO_IMGUI_HINT_FLAG_NO_INVERT_Y = (1 << 1) };

typedef struct neko_imgui_hints_s {
    neko_vec2 framebuffer_size;  // Overall framebuffer size
    neko_imgui_rect_t viewport;    // Viewport within framebuffer for gui context
    s32 flags;                   // Flags for hints
} neko_imgui_hints_t;

NEKO_API_DECL neko_imgui_rect_t neko_imgui_rect(float x, float y, float w, float h);

//=== Context ===//

NEKO_API_DECL neko_imgui_context_t neko_imgui_new(u32 window_hndl);
NEKO_API_DECL void neko_imgui_init(neko_imgui_context_t* ctx, u32 window_hndl);
NEKO_API_DECL void neko_imgui_init_font_stash(neko_imgui_context_t* ctx, neko_imgui_font_stash_desc_t* desc);
NEKO_API_DECL neko_imgui_context_t neko_imgui_context_new(u32 window_hndl);
NEKO_API_DECL void neko_imgui_free(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_begin(neko_imgui_context_t* ctx, const neko_imgui_hints_t* hints);
NEKO_API_DECL void neko_imgui_end(neko_imgui_context_t* ctx, b32 update);
NEKO_API_DECL void neko_imgui_render(neko_imgui_context_t* ctx, neko_command_buffer_t* cb);

//=== Util ===//
NEKO_API_DECL void neko_imgui_renderpass_submit(neko_imgui_context_t* ctx, neko_command_buffer_t* cb, neko_color_t clear);
NEKO_API_DECL void neko_imgui_renderpass_submit_ex(neko_imgui_context_t* ctx, neko_command_buffer_t* cb, neko_graphics_clear_action_t* action);
NEKO_API_DECL void neko_imgui_parse_id_tag(neko_imgui_context_t* ctx, const char* str, char* buffer, size_t sz);
NEKO_API_DECL void neko_imgui_parse_label_tag(neko_imgui_context_t* ctx, const char* str, char* buffer, size_t sz);

//=== Main API ===//

NEKO_API_DECL void neko_imgui_set_focus(neko_imgui_context_t* ctx, neko_imgui_id id);
NEKO_API_DECL void neko_imgui_set_hover(neko_imgui_context_t* ctx, neko_imgui_id id);
NEKO_API_DECL neko_imgui_id neko_imgui_get_id(neko_imgui_context_t* ctx, const void* data, s32 size);
NEKO_API_DECL void neko_imgui_push_id(neko_imgui_context_t* ctx, const void* data, s32 size);
NEKO_API_DECL void neko_imgui_pop_id(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_push_clip_rect(neko_imgui_context_t* ctx, neko_imgui_rect_t rect);
NEKO_API_DECL void neko_imgui_pop_clip_rect(neko_imgui_context_t* ctx);
NEKO_API_DECL neko_imgui_rect_t neko_imgui_get_clip_rect(neko_imgui_context_t* ctx);
NEKO_API_DECL s32 neko_imgui_check_clip(neko_imgui_context_t* ctx, neko_imgui_rect_t r);
NEKO_API_DECL s32 neko_imgui_mouse_over(neko_imgui_context_t* ctx, neko_imgui_rect_t rect);
NEKO_API_DECL void neko_imgui_update_control(neko_imgui_context_t* ctx, neko_imgui_id id, neko_imgui_rect_t rect, u64 opt);

//=== Conatiner ===//

NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_current_container(neko_imgui_context_t* ctx);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_container(neko_imgui_context_t* ctx, const char* name);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_top_most_container(neko_imgui_context_t* ctx, neko_imgui_split_t* split);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_container_ex(neko_imgui_context_t* ctx, neko_imgui_id id, u64 opt);
NEKO_API_DECL void neko_imgui_bring_to_front(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);
NEKO_API_DECL void neko_imgui_bring_split_to_front(neko_imgui_context_t* ctx, neko_imgui_split_t* split);
NEKO_API_DECL neko_imgui_split_t* neko_imgui_get_split(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);
NEKO_API_DECL neko_imgui_tab_bar_t* neko_imgui_get_tab_bar(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);
NEKO_API_DECL void neko_imgui_tab_item_swap(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt, s32 direction);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_root_container(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_root_container_from_split(neko_imgui_context_t* ctx, neko_imgui_split_t* split);
NEKO_API_DECL neko_imgui_container_t* neko_imgui_get_parent(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);
NEKO_API_DECL void neko_imgui_current_container_close(neko_imgui_context_t* ctx);

//=== Animation ===//

NEKO_API_DECL neko_imgui_animation_t* neko_imgui_get_animation(neko_imgui_context_t* ctx, neko_imgui_id id, const neko_imgui_selector_desc_t* desc, s32 elementid);

NEKO_API_DECL neko_imgui_style_t neko_imgui_animation_get_blend_style(neko_imgui_context_t* ctx, neko_imgui_animation_t* anim, const neko_imgui_selector_desc_t* desc, s32 elementid);

//=== Style Sheet ===//

NEKO_API_DECL neko_imgui_style_sheet_t neko_imgui_style_sheet_create(neko_imgui_context_t* ctx, neko_imgui_style_sheet_desc_t* desc);
NEKO_API_DECL void neko_imgui_style_sheet_destroy(neko_imgui_style_sheet_t* ss);
NEKO_API_DECL void neko_imgui_set_element_style(neko_imgui_context_t* ctx, neko_imgui_element_type element, neko_imgui_element_state state, neko_imgui_style_element_t* style, size_t size);
NEKO_API_DECL void neko_imgui_style_sheet_set_element_styles(neko_imgui_style_sheet_t* style_sheet, neko_imgui_element_type element, neko_imgui_element_state state, neko_imgui_style_element_t* styles,
                                                           size_t size);
NEKO_API_DECL void neko_imgui_set_style_sheet(neko_imgui_context_t* ctx, neko_imgui_style_sheet_t* style_sheet);

//=== Resource Loading ===//

NEKO_API_DECL neko_imgui_style_sheet_t neko_imgui_style_sheet_load_from_file(neko_imgui_context_t* ctx, const char* file_path);
NEKO_API_DECL neko_imgui_style_sheet_t neko_imgui_style_sheet_load_from_memory(neko_imgui_context_t* ctx, const char* memory, size_t sz, bool* success);

//=== Pools ===//

NEKO_API_DECL s32 neko_imgui_pool_init(neko_imgui_context_t* ctx, neko_imgui_pool_item_t* items, s32 len, neko_imgui_id id);
NEKO_API_DECL s32 neko_imgui_pool_get(neko_imgui_context_t* ctx, neko_imgui_pool_item_t* items, s32 len, neko_imgui_id id);
NEKO_API_DECL void neko_imgui_pool_update(neko_imgui_context_t* ctx, neko_imgui_pool_item_t* items, s32 idx);

//=== Input ===//

NEKO_API_DECL void neko_imgui_input_mousemove(neko_imgui_context_t* ctx, s32 x, s32 y);
NEKO_API_DECL void neko_imgui_input_mousedown(neko_imgui_context_t* ctx, s32 x, s32 y, s32 btn);
NEKO_API_DECL void neko_imgui_input_mouseup(neko_imgui_context_t* ctx, s32 x, s32 y, s32 btn);
NEKO_API_DECL void neko_imgui_input_scroll(neko_imgui_context_t* ctx, s32 x, s32 y);
NEKO_API_DECL void neko_imgui_input_keydown(neko_imgui_context_t* ctx, s32 key);
NEKO_API_DECL void neko_imgui_input_keyup(neko_imgui_context_t* ctx, s32 key);
NEKO_API_DECL void neko_imgui_input_text(neko_imgui_context_t* ctx, const char* text);

//=== Commands ===//

NEKO_API_DECL neko_imgui_command_t* neko_imgui_push_command(neko_imgui_context_t* ctx, s32 type, s32 size);
NEKO_API_DECL s32 neko_imgui_next_command(neko_imgui_context_t* ctx, neko_imgui_command_t** cmd);
NEKO_API_DECL void neko_imgui_set_clip(neko_imgui_context_t* ctx, neko_imgui_rect_t rect);
NEKO_API_DECL void neko_imgui_set_pipeline(neko_imgui_context_t* ctx, neko_handle(neko_graphics_pipeline_t) pip, void* layout, size_t layout_sz, neko_idraw_layout_type layout_type);
NEKO_API_DECL void neko_imgui_bind_uniforms(neko_imgui_context_t* ctx, neko_graphics_bind_uniform_desc_t* uniforms, size_t uniforms_sz);

//=== Drawing ===//

NEKO_API_DECL void neko_imgui_draw_rect(neko_imgui_context_t* ctx, neko_imgui_rect_t rect, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_circle(neko_imgui_context_t* ctx, neko_vec2 position, float radius, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_triangle(neko_imgui_context_t* ctx, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_box(neko_imgui_context_t* ctx, neko_imgui_rect_t rect, s16* width, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_line(neko_imgui_context_t* ctx, neko_vec2 start, neko_vec2 end, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_text(neko_imgui_context_t* ctx, neko_asset_ascii_font_t* font, const char* str, s32 len, neko_vec2 pos, neko_color_t color, s32 shadow_x, s32 shadow_y,
                                      neko_color_t shadow_color);
NEKO_API_DECL void neko_imgui_draw_image(neko_imgui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_imgui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_nine_rect(neko_imgui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_imgui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, u32 left, u32 right, u32 top,
                                           u32 bottom, neko_color_t color);
NEKO_API_DECL void neko_imgui_draw_control_frame(neko_imgui_context_t* ctx, neko_imgui_id id, neko_imgui_rect_t rect, s32 elementid, u64 opt);
NEKO_API_DECL void neko_imgui_draw_control_text(neko_imgui_context_t* ctx, const char* str, neko_imgui_rect_t rect, const neko_imgui_style_t* style, u64 opt);
NEKO_API_DECL void neko_imgui_draw_custom(neko_imgui_context_t* ctx, neko_imgui_rect_t rect, neko_imgui_draw_callback_t cb, void* data, size_t sz);

//=== Layout ===//

NEKO_API_DECL neko_imgui_layout_t* neko_imgui_get_layout(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_layout_row(neko_imgui_context_t* ctx, s32 items, const s32* widths, s32 height);
NEKO_API_DECL void neko_imgui_layout_row_ex(neko_imgui_context_t* ctx, s32 items, const s32* widths, s32 height, s32 justification);
NEKO_API_DECL void neko_imgui_layout_width(neko_imgui_context_t* ctx, s32 width);
NEKO_API_DECL void neko_imgui_layout_height(neko_imgui_context_t* ctx, s32 height);
NEKO_API_DECL void neko_imgui_layout_column_begin(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_layout_column_end(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_layout_set_next(neko_imgui_context_t* ctx, neko_imgui_rect_t r, s32 relative);
NEKO_API_DECL neko_imgui_rect_t neko_imgui_layout_peek_next(neko_imgui_context_t* ctx);
NEKO_API_DECL neko_imgui_rect_t neko_imgui_layout_next(neko_imgui_context_t* ctx);
NEKO_API_DECL neko_imgui_rect_t neko_imgui_layout_anchor(const neko_imgui_rect_t* parent, s32 width, s32 height, s32 xoff, s32 yoff, neko_imgui_layout_anchor_type type);

//=== Elements ===//

#define neko_imgui_button(_CTX, _LABEL) neko_imgui_button_ex((_CTX), (_LABEL), NULL, NEKO_IMGUI_OPT_LEFTCLICKONLY)
#define neko_imgui_text(_CTX, _TXT) neko_imgui_text_ex((_CTX), (_TXT), 1, NULL, 0x00)
// #define neko_imgui_text_fc(_CTX, _TXT) neko_imgui_text_fc_ex((_CTX), (_TXT), (-1))
#define neko_imgui_textbox(_CTX, _BUF, _BUFSZ) neko_imgui_textbox_ex((_CTX), (_BUF), (_BUFSZ), NULL, 0x00)
#define neko_imgui_slider(_CTX, _VALUE, _LO, _HI) neko_imgui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, NEKO_IMGUI_SLIDER_FMT, NULL, 0x00)
#define neko_imgui_number(_CTX, _VALUE, _STEP) neko_imgui_number_ex((_CTX), (_VALUE), (_STEP), NEKO_IMGUI_SLIDER_FMT, NULL, 0x00)
#define neko_imgui_header(_CTX, _LABEL) neko_imgui_header_ex((_CTX), (_LABEL), NULL, 0x00)
#define neko_imgui_checkbox(_CTX, _LABEL, _STATE) neko_imgui_checkbox_ex((_CTX), (_LABEL), (_STATE), NULL, NEKO_IMGUI_OPT_LEFTCLICKONLY)
#define neko_imgui_treenode_begin(_CTX, _LABEL) neko_imgui_treenode_begin_ex((_CTX), (_LABEL), NULL, 0x00)
#define neko_imgui_window_begin(_CTX, _TITLE, _RECT) neko_imgui_window_begin_ex((_CTX), (_TITLE), (_RECT), 0, NULL, 0x00)
#define neko_imgui_popup_begin(_CTX, _TITLE, _RECT) neko_imgui_popup_begin_ex((_CTX), (_TITLE), (_RECT), NULL, 0x00)
#define neko_imgui_panel_begin(_CTX, _NAME) neko_imgui_panel_begin_ex((_CTX), (_NAME), NULL, 0x00)
#define neko_imgui_image(_CTX, _HNDL) neko_imgui_image_ex((_CTX), (_HNDL), neko_v2s(0.f), neko_v2s(1.f), NULL, 0x00)
#define neko_imgui_combo_begin(_CTX, _ID, _ITEM, _MAX) neko_imgui_combo_begin_ex((_CTX), (_ID), (_ITEM), (_MAX), NULL, 0x00)
#define neko_imgui_combo_item(_CTX, _NAME) neko_imgui_combo_item_ex((_CTX), (_NAME), NULL, 0x00)
#define neko_imgui_dock(_CTX, _DST, _SRC, _TYPE) neko_imgui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define neko_imgui_undock(_CTX, _NAME) neko_imgui_undock_ex((_CTX), (_NAME))
#define neko_imgui_label(_CTX, _FMT, ...) (neko_snprintf((_CTX)->number_edit_buf, sizeof((_CTX)->number_edit_buf), _FMT, ##__VA_ARGS__), neko_imgui_label_ex((_CTX), (_CTX)->number_edit_buf, NULL, 0x00))
#define neko_imgui_labelf(STR, ...)                        \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        neko_imgui_label(&g_gui, BUFFER);                  \
    } while (0)

//=== Elements (Extended) ===//

NEKO_API_DECL s32 neko_imgui_image_ex(neko_imgui_context_t* ctx, neko_handle(neko_graphics_texture_t) hndl, neko_vec2 uv0, neko_vec2 uv1, const neko_imgui_selector_desc_t* desc, u64 opt);
// NEKO_API_DECL s32 neko_imgui_text_fc_ex(neko_imgui_context_t* ctx, const char* text, neko_font_index fontindex);
NEKO_API_DECL s32 neko_imgui_text_ex(neko_imgui_context_t* ctx, const char* text, s32 text_wrap, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_label_ex(neko_imgui_context_t* ctx, const char* text, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_button_ex(neko_imgui_context_t* ctx, const char* label, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_checkbox_ex(neko_imgui_context_t* ctx, const char* label, s32* state, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_textbox_raw(neko_imgui_context_t* ctx, char* buf, s32 bufsz, neko_imgui_id id, neko_imgui_rect_t r, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_textbox_ex(neko_imgui_context_t* ctx, char* buf, s32 bufsz, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_slider_ex(neko_imgui_context_t* ctx, neko_imgui_real* value, neko_imgui_real low, neko_imgui_real high, neko_imgui_real step, const char* fmt, const neko_imgui_selector_desc_t* desc,
                                     u64 opt);
NEKO_API_DECL s32 neko_imgui_number_ex(neko_imgui_context_t* ctx, neko_imgui_real* value, neko_imgui_real step, const char* fmt, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_header_ex(neko_imgui_context_t* ctx, const char* label, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_imgui_treenode_begin_ex(neko_imgui_context_t* ctx, const char* label, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_imgui_treenode_end(neko_imgui_context_t* ctx);
NEKO_API_DECL s32 neko_imgui_window_begin_ex(neko_imgui_context_t* ctx, const char* title, neko_imgui_rect_t rect, bool* open, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_imgui_window_end(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_popup_open(neko_imgui_context_t* ctx, const char* name);
NEKO_API_DECL s32 neko_imgui_popup_begin_ex(neko_imgui_context_t* ctx, const char* name, neko_imgui_rect_t r, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_imgui_popup_end(neko_imgui_context_t* ctx);
NEKO_API_DECL void neko_imgui_panel_begin_ex(neko_imgui_context_t* ctx, const char* name, const neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_imgui_panel_end(neko_imgui_context_t* ctx);
NEKO_API_DECL s32 neko_imgui_combo_begin_ex(neko_imgui_context_t* ctx, const char* id, const char* current_item, s32 max_items, neko_imgui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_imgui_combo_end(neko_imgui_context_t* ctx);
NEKO_API_DECL s32 neko_imgui_combo_item_ex(neko_imgui_context_t* ctx, const char* label, const neko_imgui_selector_desc_t* desc, u64 opt);

//=== Demos ===//

NEKO_API_DECL s32 neko_imgui_style_editor(neko_imgui_context_t* ctx, neko_imgui_style_sheet_t* style_sheet, neko_imgui_rect_t rect, bool* open);
NEKO_API_DECL s32 neko_imgui_demo_window(neko_imgui_context_t* ctx, neko_imgui_rect_t rect, bool* open);

//=== Docking ===//

NEKO_API_DECL void neko_imgui_dock_ex(neko_imgui_context_t* ctx, const char* dst, const char* src, s32 split_type, float ratio);
NEKO_API_DECL void neko_imgui_undock_ex(neko_imgui_context_t* ctx, const char* name);
NEKO_API_DECL void neko_imgui_dock_ex_cnt(neko_imgui_context_t* ctx, neko_imgui_container_t* dst, neko_imgui_container_t* src, s32 split_type, float ratio);
NEKO_API_DECL void neko_imgui_undock_ex_cnt(neko_imgui_context_t* ctx, neko_imgui_container_t* cnt);

//=== Gizmo ===//

NEKO_API_DECL s32 neko_imgui_gizmo(neko_imgui_context_t* ctx, neko_camera_t* camera, neko_vqs* model, neko_imgui_rect_t viewport, bool invert_view_y, float snap, s32 op, s32 mode, u64 opt);

#endif  // NEKO_IMGUI_H
