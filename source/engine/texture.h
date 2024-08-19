#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine/base.h"
#include "engine/gfx.h"
#include "engine/glew_glfw.h"

typedef struct Texture {
    // char *filename;
    GLuint id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;

    // u64 last_modified;

    bool flip_image_vertical;
} Texture;

bool texture_load(Texture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
LuaVec2 texture_get_size(const char* filename);  // (width, height)
Texture texture_get_ptr(const char* filename);
bool texture_update(Texture* tex, String filename);
bool texture_update_data(Texture* tex, u8* data);

u64 generate_texture_handle(void* pixels, int w, int h, void* udata);
void destroy_texture_handle(u64 texture_id, void* udata);

typedef struct neko_asset_texture_t {
    neko_handle(gfx_texture_t) hndl;
    gfx_texture_desc_t desc;
} neko_asset_texture_t;

// Texture

bool load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

// #if 1
// bool neko_asset_texture_load_from_file(const_str path, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
// #endif

#endif
