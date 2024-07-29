
#ifndef NEKO_IMAGE_H
#define NEKO_IMAGE_H

#include "neko_base.h"
#include "neko_prelude.h"

// deps
#include <stb_truetype.h>

struct Image {
    u32 id;
    i32 width;
    i32 height;
    bool has_mips;

    bool load(String filepath, bool generate_mips);
    void trash();
};

struct SpriteFrame {
    i32 duration;
    float u0, v0, u1, v1;
};

struct SpriteLoop {
    Slice<i32> indices;
};

struct SpriteData {
    Arena arena;
    Slice<SpriteFrame> frames;
    HashMap<SpriteLoop> by_tag;
    Image img;
    i32 width;
    i32 height;

    bool load(String filepath);
    void trash();
};

struct Sprite {
    u64 sprite;  // index into assets
    u64 loop;    // index into SpriteData::by_tag
    float elapsed;
    i32 current_frame;

    bool play(String tag);
    void update(float dt);
    void set_frame(i32 frame);
};

struct SpriteView {
    Sprite *sprite;
    SpriteData data;
    SpriteLoop loop;

    bool make(Sprite *spr);
    i32 frame();
    u64 len();
};

struct FontRange {
    stbtt_bakedchar chars[256];
    Image image;
};

struct FontQuad {
    stbtt_aligned_quad quad;
};

struct FontFamily {
    String ttf;
    HashMap<FontRange> ranges;
    StringBuilder sb;

    bool load(String filepath);
    void load_default();
    void trash();

    stbtt_aligned_quad quad(u32 *img, float *x, float *y, float size, i32 ch);
    float width(float size, String text);
};

struct AtlasImage {
    float u0, v0, u1, v1;
    float width;
    float height;
    Image img;
};

struct Atlas {
    HashMap<AtlasImage> by_name;
    Image img;

    bool load(String filepath, bool generate_mips);
    void trash();
    AtlasImage *get(String name);
};

#endif