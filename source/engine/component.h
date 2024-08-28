#ifndef NEKO_COMPONENT_H
#define NEKO_COMPONENT_H

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/entity.h"
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
void transform_update_all();
void transform_save_all(Store* s);
void transform_load_all(Store* s);

struct Tile {
    float x, y, u, v;
    float u0, v0, u1, v1;
    i32 flip_bits;
};

struct TilemapEntity {
    String identifier;
    float x, y;
};

using TilemapInt = unsigned char;

struct TilemapLayer {
    String identifier;
    AssetTexture image;
    Slice<Tile> tiles;
    Slice<TilemapEntity> entities;
    i32 c_width;
    i32 c_height;
    Slice<TilemapInt> int_grid;
    float grid_size;
};

struct TilemapLevel {
    String identifier;
    String iid;
    float world_x, world_y;
    float px_width, px_height;
    Slice<TilemapLayer> layers;
};

enum TileNodeFlags {
    TileNodeFlags_Open = 1 << 0,
    TileNodeFlags_Closed = 1 << 1,
};

struct TileNode {
    TileNode* prev;
    float g;  // cost so far
    u32 flags;

    i32 x, y;
    float cost;
    Slice<TileNode*> neighbors;
};

struct TileCost {
    TilemapInt cell;
    float value;
};

struct TilePoint {
    float x, y;
};

inline u64 tile_key(i32 x, i32 y) { return ((u64)x << 32) | (u64)y; }

class b2Body;
class b2World;

struct MapLdtk {
    Arena arena;
    Slice<TilemapLevel> levels;
    HashMap<AssetTexture> images;  // key: filepath
    HashMap<b2Body*> bodies;       // key: layer name
    HashMap<TileNode> graph;       // key: x, y
    PriorityQueue<TileNode*> frontier;
    float graph_grid_size;

    bool load(String filepath);
    void trash();
    void destroy_bodies(b2World* world);
    void make_collision(b2World* world, float meter, String layer_name, Slice<TilemapInt> walls);
    void make_graph(i32 bloom, String layer_name, Slice<TileCost> costs);
    TileNode* astar(TilePoint start, TilePoint goal);
};

#define SPRITE_SCALE 3

/*==========================
// Tiled draw
==========================*/

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    neko_handle(gfx_texture_t) texture;
    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_t {
    tile_t* tiles;
    u32 width;
    u32 height;

    Color256 tint;
} layer_t;

typedef struct object_t {
    u32 id;
    i32 x, y, width, height;
    // C2_TYPE phy_type;
    // c2AABB aabb;
    // union {
    //     c2AABB box;
    //     c2Poly poly;
    // } phy;
} object_t;

typedef struct object_group_t {
    neko_dyn_array(object_t) objects;

    Color256 color;

    const_str name;
} object_group_t;

typedef struct map_t {
    xml_document_t* doc;  // xml doc
    neko_dyn_array(tileset_t) tilesets;
    neko_dyn_array(object_group_t) object_groups;
    neko_dyn_array(layer_t) layers;
} map_t;

void tiled_load(map_t* map, const_str tmx_path, const_str res_path);
void tiled_unload(map_t* map);

typedef struct tiled_quad_t {
    u32 tileset_id;
    gfx_texture_t texture;
    vec2 texture_size;
    vec2 position;
    vec2 dimentions;
    vec4 rectangle;
    Color256 color;
    bool use_texture;
} tiled_quad_t;

#define BATCH_SIZE 4096

#define IND_PER_QUAD 6

#define VERTS_PER_QUAD 4   // 一次发送多少个verts数据
#define FLOATS_PER_VERT 9  // 每个verts数据的大小

typedef struct tiled_quad_list_t {
    neko_dyn_array(tiled_quad_t) quad_list;  // quad 绘制队列
} tiled_quad_list_t;

typedef struct tiled_renderer {
    // neko_handle(gfx_vertex_buffer_t) vb;
    // neko_handle(gfx_index_buffer_t) ib;
    // neko_handle(gfx_pipeline_t) pip;
    // neko_handle(gfx_shader_t) shader;

    GLuint vao;
    GLuint vbo;
    GLuint ib;

    // GLuint shader;

    // neko_handle(gfx_uniform_t) u_camera;
    // neko_handle(gfx_uniform_t) u_batch_tex;
    gfx_texture_t batch_texture;                         // 当前绘制所用贴图
    neko_hash_table(u32, tiled_quad_list_t) quad_table;  // 分层绘制哈希表

    u32 quad_count;

    map_t map;  // tiled data

    mat3 camera_mat;
} tiled_renderer;

void tiled_render_init(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_deinit(tiled_renderer* renderer);
void tiled_render_begin(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_flush(command_buffer_t* cb, tiled_renderer* renderer);
void tiled_render_push(command_buffer_t* cb, tiled_renderer* renderer, tiled_quad_t quad);
void tiled_render_draw(command_buffer_t* cb, tiled_renderer* renderer);

struct Tiled;

int tiled_render(command_buffer_t* cb, Tiled* tiled);

NEKO_SCRIPT(tiled,

            NEKO_EXPORT void tiled_add(NativeEntity ent);

            NEKO_EXPORT void tiled_remove(NativeEntity ent);

            NEKO_EXPORT bool tiled_has(NativeEntity ent);

            NEKO_EXPORT void tiled_set_map(NativeEntity ent, const char* str);

            NEKO_EXPORT const char* tiled_get_map(NativeEntity ent);

)

void tiled_init();
void tiled_fini();
void tiled_update_all();
void tiled_draw_all();

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
void camera_update_all();
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
void sprite_update_all();
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

void edit_clear();

void edit_init();
void edit_fini();
void edit_update_all();
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

        typedef enum GuiAlign GuiAlign; enum GuiAlign{
                GA_MIN = 0,    // h: left, v: bottom
                GA_MID = 1,    // h: center, v: center
                GA_MAX = 2,    // h: right, v: top
                GA_TABLE = 3,  // h: left-right table, v: top-down table
                GA_NONE = 4,   // manual position
        };

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

#endif
