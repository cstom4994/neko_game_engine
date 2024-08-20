#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/base.h"
#include "engine/gfx.h"
#include "engine/glew_glfw.h"

typedef struct AssetTexture {
    GLuint id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;
    bool flip_image_vertical;
} AssetTexture;

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
LuaVec2 texture_get_size(const char* filename);  // (width, height)
AssetTexture texture_get_ptr(const char* filename);
bool texture_update(AssetTexture* tex, String filename);
bool texture_update_data(AssetTexture* tex, u8* data);

u64 generate_texture_handle(void* pixels, int w, int h, void* udata);
void destroy_texture_handle(u64 texture_id, void* udata);

neko_texture_t neko_aseprite_simple(String filename);

typedef struct neko_asset_texture_t {
    neko_handle(gfx_texture_t) hndl;
    gfx_texture_desc_t desc;
} neko_asset_texture_t;

// AssetTexture

bool load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

// #if 1
// bool neko_asset_texture_load_from_file(const_str path, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// #endif

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

#endif
