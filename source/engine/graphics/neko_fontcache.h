
#pragma once

#include <cstdio>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/common/neko_util.h"
#include "engine/math/neko_math.h"
#include "libs/stb/stb_truetype.h"

#ifndef NEKO_FONTCACHE_CURNEKO_QUALITY
#define NEKO_FONTCACHE_CURNEKO_QUALITY 6
#endif  // NEKO_FONTCACHE_CURNEKO_QUALITY

#include "libs/external/utf8.h"

#define NEKO_FONTCACHE_ATLAS_WIDTH 4096
#define NEKO_FONTCACHE_ATLAS_HEIGHT 2048
#define NEKO_FONTCACHE_ATLAS_GLYPH_PADDING 1

#define NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH 32
#define NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT 32
#define NEKO_FONTCACHE_ATLAS_REGION_A_XSIZE (NEKO_FONTCACHE_ATLAS_WIDTH / 4)
#define NEKO_FONTCACHE_ATLAS_REGION_A_YSIZE (NEKO_FONTCACHE_ATLAS_HEIGHT / 2)
#define NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_A_XSIZE / NEKO_FONTCACHE_ATLAS_REGION_A_WIDTH)
#define NEKO_FONTCACHE_ATLAS_REGION_A_YCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_A_YSIZE / NEKO_FONTCACHE_ATLAS_REGION_A_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_A_CAPACITY (NEKO_FONTCACHE_ATLAS_REGION_A_XCAPACITY * NEKO_FONTCACHE_ATLAS_REGION_A_YCAPACITY)
#define NEKO_FONTCACHE_ATLAS_REGION_A_XOFFSET 0
#define NEKO_FONTCACHE_ATLAS_REGION_A_YOFFSET 0

#define NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH 32
#define NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT 64
#define NEKO_FONTCACHE_ATLAS_REGION_B_XSIZE (NEKO_FONTCACHE_ATLAS_WIDTH / 4)
#define NEKO_FONTCACHE_ATLAS_REGION_B_YSIZE (NEKO_FONTCACHE_ATLAS_HEIGHT / 2)
#define NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_B_XSIZE / NEKO_FONTCACHE_ATLAS_REGION_B_WIDTH)
#define NEKO_FONTCACHE_ATLAS_REGION_B_YCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_B_YSIZE / NEKO_FONTCACHE_ATLAS_REGION_B_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_B_CAPACITY (NEKO_FONTCACHE_ATLAS_REGION_B_XCAPACITY * NEKO_FONTCACHE_ATLAS_REGION_B_YCAPACITY)
#define NEKO_FONTCACHE_ATLAS_REGION_B_XOFFSET 0
#define NEKO_FONTCACHE_ATLAS_REGION_B_YOFFSET NEKO_FONTCACHE_ATLAS_REGION_A_YSIZE

#define NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH 64
#define NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT 64
#define NEKO_FONTCACHE_ATLAS_REGION_C_XSIZE (NEKO_FONTCACHE_ATLAS_WIDTH / 4)
#define NEKO_FONTCACHE_ATLAS_REGION_C_YSIZE (NEKO_FONTCACHE_ATLAS_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_C_XSIZE / NEKO_FONTCACHE_ATLAS_REGION_C_WIDTH)
#define NEKO_FONTCACHE_ATLAS_REGION_C_YCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_C_YSIZE / NEKO_FONTCACHE_ATLAS_REGION_C_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_C_CAPACITY (NEKO_FONTCACHE_ATLAS_REGION_C_XCAPACITY * NEKO_FONTCACHE_ATLAS_REGION_C_YCAPACITY)
#define NEKO_FONTCACHE_ATLAS_REGION_C_XOFFSET NEKO_FONTCACHE_ATLAS_REGION_A_XSIZE
#define NEKO_FONTCACHE_ATLAS_REGION_C_YOFFSET 0

#define NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH 128
#define NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT 128
#define NEKO_FONTCACHE_ATLAS_REGION_D_XSIZE (NEKO_FONTCACHE_ATLAS_WIDTH / 2)
#define NEKO_FONTCACHE_ATLAS_REGION_D_YSIZE (NEKO_FONTCACHE_ATLAS_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_D_XSIZE / NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH)
#define NEKO_FONTCACHE_ATLAS_REGION_D_YCAPACITY (NEKO_FONTCACHE_ATLAS_REGION_D_YSIZE / NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT)
#define NEKO_FONTCACHE_ATLAS_REGION_D_CAPACITY (NEKO_FONTCACHE_ATLAS_REGION_D_XCAPACITY * NEKO_FONTCACHE_ATLAS_REGION_D_YCAPACITY)
#define NEKO_FONTCACHE_ATLAS_REGION_D_XOFFSET (NEKO_FONTCACHE_ATLAS_WIDTH / 2)
#define NEKO_FONTCACHE_ATLAS_REGION_D_YOFFSET 0

static_assert(NEKO_FONTCACHE_ATLAS_REGION_A_CAPACITY == 1024, "VE FontCache Atlas Sanity check fail. Please update this assert if you changed atlas packing strategy.");
static_assert(NEKO_FONTCACHE_ATLAS_REGION_B_CAPACITY == 512, "VE FontCache Atlas Sanity check fail. Please update this assert if you changed atlas packing strategy.");
static_assert(NEKO_FONTCACHE_ATLAS_REGION_C_CAPACITY == 512, "VE FontCache Atlas Sanity check fail. Please update this assert if you changed atlas packing strategy.");
static_assert(NEKO_FONTCACHE_ATLAS_REGION_D_CAPACITY == 256, "VE FontCache Atlas Sanity check fail. Please update this assert if you changed atlas packing strategy.");

#define NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X 4
#define NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y 4
#define NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH 4
#define NEKO_FONTCACHE_GLYPHDRAW_BUFFER_WIDTH (NEKO_FONTCACHE_ATLAS_REGION_D_WIDTH * NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_X * NEKO_FONTCACHE_GLYPHDRAW_BUFFER_BATCH)
#define NEKO_FONTCACHE_GLYPHDRAW_BUFFER_HEIGHT (NEKO_FONTCACHE_ATLAS_REGION_D_HEIGHT * NEKO_FONTCACHE_GLYPHDRAW_OVERSAMPLE_Y)

// Set to same value as NEKO_FONTCACHE_ATLAS_GLYPH_PADDING for best results!
#define NEKO_FONTCACHE_GLYPHDRAW_PADDING NEKO_FONTCACHE_ATLAS_GLYPH_PADDING

#define NEKO_FONTCACHE_FRAMEBUFFER_PASS_GLYPH 1
#define NEKO_FONTCACHE_FRAMEBUFFER_PASS_ATLAS 2
#define NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET 3
#define NEKO_FONTCACHE_FRAMEBUFFER_PASS_TARGET_UNCACHED 4

// How many to store in text shaping cache. Shaping cache is also stored in LRU format.
#define NEKO_FONTCACHE_SHAPECACHE_SIZE 256

// How much to reserve for each shape cache. This adds up to a ~0.768mb cache.
#define NEKO_FONTCACHE_SHAPECACHE_RESERNEKO_LENGTH 64

// Max. text size for caching. This means the cache has ~3.072mb upper bound.
#define NEKO_FONTCACHE_SHAPECACHE_MAX_LENGTH 256

// Sizes below this snap their advance to next pixel boundary.
#define NEKO_FONTCACHE_ADVANCE_SNAP_SMALLFONT_SIZE 12

// --------------------------------------------------------------- Data Types ---------------------------------------------------

typedef int64_t ve_font_id;
typedef int32_t ve_codepoint;
typedef int32_t ve_glyph;
typedef char ve_atlas_region;

struct neko_fontcache_entry {
    ve_font_id font_id = 0;
    stbtt_fontinfo info;
    bool used = false;
    float size = 24.0f;
    float size_scale = 1.0f;
};

struct neko_fontcache_vertex {
    float x;
    float y;
    float u;
    float v;
};

inline neko_vec2 neko_fontcache_make_vec2(float x_, float y_) {
    neko_vec2 v;
    v.x = x_;
    v.y = y_;
    return v;
}

struct neko_fontcache_draw {
    uint32_t pass = 0;  // One of NEKO_FONTCACHE_FRAMEBUFFER_PASS_* values.
    uint32_t start_index = 0;
    uint32_t end_index = 0;
    bool clear_before_draw = false;
    uint32_t region = 0;
    float colour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct neko_fontcache_drawlist {
    std::vector<neko_fontcache_vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<neko_fontcache_draw> dcalls;
};

typedef uint32_t neko_fontcache_poollist_itr;
typedef uint64_t neko_fontcache_poollist_value;

struct neko_fontcache_poollist_item {
    neko_fontcache_poollist_itr prev = -1;
    neko_fontcache_poollist_itr next = -1;
    neko_fontcache_poollist_value value = 0;
};

struct neko_fontcache_poollist {
    std::vector<neko_fontcache_poollist_item> pool;
    std::vector<neko_fontcache_poollist_itr> freelist;
    neko_fontcache_poollist_itr front = -1;
    neko_fontcache_poollist_itr back = -1;
    size_t size = 0;
    size_t capacity = 0;
};

struct neko_fontcache_LRU_link {
    int value = 0;
    neko_fontcache_poollist_itr ptr;
};

struct neko_fontcache_LRU {
    int capacity = 0;
    std::unordered_map<uint64_t, neko_fontcache_LRU_link> cache;
    neko_fontcache_poollist key_queue;
};

struct neko_fontcache_atlas {
    uint32_t next_atlas_idx_A = 0;
    uint32_t next_atlas_idx_B = 0;
    uint32_t next_atlas_idx_C = 0;
    uint32_t next_atlas_idx_D = 0;

    neko_fontcache_LRU stateA;
    neko_fontcache_LRU stateB;
    neko_fontcache_LRU stateC;
    neko_fontcache_LRU stateD;

    uint32_t glyph_update_batch_x = 0;
    neko_fontcache_drawlist glyph_update_batch_clear_drawlist;
    neko_fontcache_drawlist glyph_update_batch_drawlist;
};

struct neko_fontcache_shaped_text {
    std::vector<ve_glyph> glyphs;
    std::vector<neko_vec2> pos;
    neko_vec2 end_cursor_pos;
};

struct neko_fontcache_shaped_text_cache {
    std::vector<neko_fontcache_shaped_text> storage;
    neko_fontcache_LRU state;
    uint32_t next_cache_idx = 0;
};

struct neko_fontcache {
    std::vector<neko_fontcache_entry> entry;

    std::vector<neko_vec2> temp_path;
    std::unordered_map<uint64_t, bool> temp_codepoint_seen;

    uint32_t snap_width = 0;
    uint32_t snap_height = 0;
    float colour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    neko_vec2 cursor_pos;

    neko_fontcache_drawlist drawlist;
    neko_fontcache_atlas atlas;
    neko_fontcache_shaped_text_cache shape_cache;

    bool text_shape_advanced = true;
};

// 调用此函数来初始化字体缓存
void __neko_fontcache_init(neko_fontcache* cache);

// 调用此命令可以关闭字体缓存所有内容
void __neko_fontcache_shutdown(neko_fontcache* cache);

// 从内存缓冲区加载 hb_font 支持 otf,ttf,STB_truetype 支持的所有内容
// 调用者仍然拥有数据并且必须保留它 当 hb_font 仍在使用时 无法释放该缓冲区
// SBTTT 将跟踪该内存中的弱指针
// 如果您以不同的 size_px 值加载相同的 hb_font 则可以在它们之间共享相同的数据缓冲区
ve_font_id __neko_fontcache_load(neko_fontcache* cache, const void* data, size_t data_size, float size_px = 24.0f);

// 从文件加载 hb_font 支持 otf,ttf,STB_truetype 支持的所有内容
// 调用者仍然拥有给定的缓冲区并且必须保留它 当 hb_font 仍在使用时 无法释放该缓冲区
// SBTTT 将跟踪该内存中的弱指针
// 如果您以不同的 size_px 值加载相同的 hb_font 则可以在它们之间共享相同的缓冲区
ve_font_id __neko_fontcache_loadfile(neko_fontcache* cache, const char* filename, std::vector<uint8_t>& buffer, float size_px = 24.0f);

// 卸载字体并释放内存 调用 __neko fontcache shutdown 已经对所有加载的字体执行了此操作
void __neko_fontcache_unload(neko_fontcache* cache, ve_font_id id);

// 当 hb 字体渲染到 2D 屏幕时 配置将字形捕捉到像素边框 可能会影响字距调整 这可能随时更改 // 将两者设置为零以禁用像素捕捉
void __neko_fontcache_configure_snap(neko_fontcache* cache, uint32_t snap_width = 0, uint32_t snap_height = 0);

// 在执行绘制列表后每帧调用此函数 这将清除下一帧的绘制列表
void __neko_fontcache_flush_drawlist(neko_fontcache* cache);

// 主要绘制文本函数 此批处理会缓存形状和字形 并在不可用时使用后备路径
// 请注意 此函数立即将渲染下一个所需的所有内容附加到缓存->绘制列表中
// 如果要绘制到多个不相关的目标 只需 draw_text 然后循环执行绘制列表 再次 draw_text 再次循环执行绘制列表
// 建议scalex = 1 / screen_width 和scaley = 1 / 屏幕高度 scalex 和 scaley 需要考虑纵横比
bool __neko_fontcache_draw_text(neko_fontcache* cache, ve_font_id font, const std::string& text_utf8, float posx = 0.0f, float posy = 0.0f, float scalex = 1.0f, float scaley = 1.0f);

// 获取最后一次 __neko_fontcache_draw_text 调用停止的位置
neko_vec2 __neko_fontcache_get_cursor_pos(neko_fontcache* cache);

// 合并绘制调用 显着改善绘制调用开销 强烈推荐在循环并执行绘制列表之前调用此函数
void __neko_fontcache_optimise_drawlist(neko_fontcache* cache);

// 从缓存中检索当前绘制列表
neko_fontcache_drawlist* __neko_fontcache_get_drawlist(neko_fontcache* cache);

// 设置后续文本绘制的文本颜色
void __neko_fontcache_set_colour(neko_fontcache* cache, float c[4]);

inline void __neko_fontcache_enable_advanced_text_shaping(neko_fontcache* cache, bool enabled = true) { cache->text_shape_advanced = enabled; }

// 用于无分配 LRU 实现的通用池列表
void __neko_fontcache_poollist_init(neko_fontcache_poollist& plist, int capacity);
void __neko_fontcache_poollist_push_front(neko_fontcache_poollist& plist, neko_fontcache_poollist_value v);
void __neko_fontcache_poollist_erase(neko_fontcache_poollist& plist, neko_fontcache_poollist_itr it);
neko_fontcache_poollist_value __neko_fontcache_poollist_peek_back(neko_fontcache_poollist& plist);
neko_fontcache_poollist_value __neko_fontcache_poollist_pop_back(neko_fontcache_poollist& plist);

// 通用 LRU（Least-Recently-Used）缓存实现 可重用于图集和形状缓存
void __neko_fontcache_LRU_init(neko_fontcache_LRU& LRU, int capacity);
int __neko_fontcache_LRU_get(neko_fontcache_LRU& LRU, uint64_t key);
int __neko_fontcache_LRU_peek(neko_fontcache_LRU& LRU, uint64_t key);
uint64_t __neko_fontcache_LRU_put(neko_fontcache_LRU& LRU, uint64_t key, int val);
void __neko_fontcache_LRU_refresh(neko_fontcache_LRU& LRU, uint64_t key);
uint64_t __neko_fontcache_LRU_get_next_evicted(neko_fontcache_LRU& LRU);

bool __neko_fontcache_cache_glyph(neko_fontcache* cache, ve_font_id font, ve_glyph glyph_index, float scaleX = 1.0f, float scaleY = 1.0f, float translateX = 0.0f, float translateY = 0.0f);
