
#pragma once

#include "base/common/string.hpp"
#include "base/common/math.hpp"

using namespace Neko;

enum TextureFlags : int {
    TEXTURE_ALIASED = 1 << 0,
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

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
vec2 texture_get_size(const char* filename);  // (width, height)
AssetTexture texture_get_ptr(const char* filename);
bool texture_update(AssetTexture* tex, String filename);
bool texture_update_data(AssetTexture* tex, u8* data);
AssetTexture neko_aseprite_simple(String filename);

// AssetTexture

bool load_texture_data_from_memory(const void* memory, int sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

NEKO_API() AssetTexture* neko_new_texture_from_memory(void* data, u32 size, TextureFlags flags);
NEKO_API() AssetTexture* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, TextureFlags flags);
NEKO_API() void neko_init_texture_from_memory(AssetTexture* texture, void* data, u32 size, TextureFlags flags);
NEKO_API() void neko_init_texture_from_memory_uncompressed(AssetTexture* texture, unsigned char* pixels, i32 width, i32 height, i32 component_count, TextureFlags flags);
NEKO_API() void neko_release_texture(AssetTexture* texture);

NEKO_API() void neko_free_texture(AssetTexture* texture);
NEKO_API() void neko_bind_texture(AssetTexture* texture, u32 slot);