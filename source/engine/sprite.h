#ifndef NEKO_SPRITE_H
#define NEKO_SPRITE_H

#include "base/common/arena.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/string.hpp"
#include "engine/base.hpp"
#include "engine/renderer/texture.h"

// deps
#include "deps/cute_aseprite.h"

using namespace Neko;

#if 0

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

struct AseSpriteFrame {
    i32 duration;
    float u0, v0, u1, v1;
};

struct AseSpriteTag {
    Slice<i32> indices;
    ase_tag_t data;
    String name;
};

struct AseSpriteData {
    Arena arena;
    Slice<AseSpriteFrame> frames;
    HashMap<AseSpriteTag> by_tag;
    Array<String> tags;
    AssetTexture tex;
    i32 width;
    i32 height;
    ase_t* ase;

    bool load(String filepath);
    bool reload_palette();
    void trash();
};

constexpr int SpriteEffectCounts = 5;

struct AseSprite {
    u64 sprite;  // index into assets
    u64 loop;    // index into AseSpriteData::by_tag
    float elapsed;
    i32 current_frame;

    std::bitset<SpriteEffectCounts> effects;

    float pixelate_value;

    void make();
    bool play(String tag);
    void OnPreUpdate(float dt);
    void set_frame(i32 frame);
};

struct AseSpriteView {
    AseSprite* sprite;
    AseSpriteData data;
    AseSpriteTag loop;

    bool make(AseSprite* spr);
    i32 frame();
    u64 len();
};

#endif