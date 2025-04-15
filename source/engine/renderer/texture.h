
#pragma once

#include "base/common/string.hpp"
#include "base/common/math.hpp"

using namespace Neko;

enum TextureFlags : int {
    TEXTURE_ALIASED = 0 << 0,
    TEXTURE_LINEAR = 1 << 0,
    TEXTURE_ANTIALIASED = 1 << 1,
    TEXTURE_SUBTEX = 1 << 2,
    TEXTURE_NO_FLIP_VERTICAL = 1 << 3,
};

typedef struct AssetTexture {
    u32 id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;
    bool flip_image_vertical;
    int unit;

    TextureFlags flags;
} AssetTexture;

struct ase_t;

void neko_tex_flip_vertically(int width, int height, u8* data);
void ase_blend_bind(ase_t* ase);

bool texture_create(AssetTexture* tex, u8* data, u32 width, u32 height, u32 num_comps, TextureFlags flags);
bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind_byname(String filename, u32 slot = 0);
void texture_bind(u32 id, u32 slot = 0);
void texture_bind(AssetTexture* texture, u32 slot = 0);
vec2 texture_get_size(String filename);  // (width, height)
AssetTexture texture_get_ptr(String filename);
bool texture_update(AssetTexture* tex, String filename);
void texture_release(AssetTexture* texture);
AssetTexture texure_aseprite_simple(String filename);
AssetTexture* neko_new_texture_from_memory(void* data, u32 size, TextureFlags flags);
void neko_init_texture_from_memory(AssetTexture* texture, void* data, u32 size, TextureFlags flags);
