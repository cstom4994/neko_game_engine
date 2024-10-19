#include "engine/asset.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <cstdio>
#include <filesystem>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/os.hpp"
#include "engine/base/profiler.hpp"
#include "engine/base/vfs.hpp"
#include "engine/bootstrap.h"
#include "engine/draw.h"
#include "engine/edit.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "vendor/glad/glad.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// deps
#include "vendor/stb_image.h"

// miniz
#include "vendor/miniz.h"

#define CUTE_ASEPRITE_ASSERT neko_assert
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "vendor/cute_aseprite.h"

using namespace neko::luabind;

// -------------------------------------------------------------------------

static void _flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height) {
    unsigned int size, stride, i, j;
    unsigned char *new_data;

    size = width * height * 4;
    stride = sizeof(char) * width * 4;

    new_data = (unsigned char *)mem_alloc(sizeof(char) * size);
    for (i = 0; i < height; i++) {
        j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }

    memcpy(data, new_data, size);
    mem_free(new_data);
}

NEKO_FORCE_INLINE void neko_tex_flip_vertically(int width, int height, u8 *data) {
    u8 rgb[4];
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int top = 4 * (x + y * width);
            int bottom = 4 * (x + (height - y - 1) * width);
            memcpy(rgb, data + top, sizeof(rgb));
            memcpy(data + top, data + bottom, sizeof(rgb));
            memcpy(data + bottom, rgb, sizeof(rgb));
        }
    }
}

gfx_texture_t neko_aseprite_simple(String filename) {
    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);

    neko_defer(mem_free(contents.data));

    ase_t *ase;

    {
        PROFILE_BLOCK("ase_image load");
        ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

        neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
        // neko_aseprite_default_blend_bind(ase);
    }

    u8 *data = reinterpret_cast<u8 *>(ase->frames->pixels);

    gfx_texture_desc_t t_desc = {};

    t_desc.format = GL_RGBA;
    t_desc.mag_filter = GL_NEAREST;
    t_desc.min_filter = GL_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    // t_desc.num_comps = 4;
    t_desc.data = data;

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)(t_desc.data));
    gfx_texture_t tex = gfx_texture_create(t_desc);
    cute_aseprite_free(ase);
    return tex;
}

static void ase_default_blend_bind(ase_t *ase) {

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t *)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t *dst = frame->pixels;

        console_log("neko_aseprite_default_blend_bind: frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t *frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                neko_assert(found);
            }
            void *src = cel->pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -NEKO_MIN(cx, 0);
            int ct = -NEKO_MIN(cy, 0);
            int dl = NEKO_MAX(cx, 0);
            int dt = NEKO_MAX(cy, 0);
            int dr = NEKO_MIN(ase->w, cw + cx);
            int db = NEKO_MIN(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    ase_color_t dst_color = dst[dst_index];
                    ase_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

u64 generate_texture_handle(void *pixels, int w, int h, void *udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (u64)location;
}

void destroy_texture_handle(u64 texture_id, void *udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

bool texture_update_data(AssetTexture *tex, u8 *data) {

    LockGuard<Mutex> lock{g_app->gpu_mtx};

    // 如果存在 则释放旧的 GL 纹理
    if (tex->id != 0) glDeleteTextures(1, &tex->id);

    // 生成 GL 纹理
    glGenTextures(1, &tex->id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (tex->flip_image_vertical) {
        _flip_image_vertical(data, tex->width, tex->height);
    }

    // 将纹理数据复制到 GL

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

static bool _texture_load_vfs(AssetTexture *tex, String filename) {
    neko_assert(tex);

    u8 *data = nullptr;

    console_printf("texture: loading texture '%s' ...", filename.cstr());

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase;

    if (filename.ends_with(".ase")) {

        {
            PROFILE_BLOCK("ase_image load");
            ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

            neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex->width = ase->w;
            tex->height = ase->h;
        }

        data = reinterpret_cast<u8 *>(ase->frames->pixels);

    } else {

        {
            PROFILE_BLOCK("stb_image load");
            stbi_set_flip_vertically_on_load(false);
            data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &tex->width, &tex->height, &tex->components, 0);
        }
    }

    // 读入纹理数据
    if (!data) {
        // tex->last_modified = modtime;
        console_printf(" unsuccessful\n");
        return false;  // 保持旧的GL纹理
    }

    {
        LockGuard<Mutex> lock(g_app->gpu_mtx);

        // 如果存在 则释放旧的 GL 纹理
        if (tex->id != 0) glDeleteTextures(1, &tex->id);

        // 生成 GL 纹理
        glGenTextures(1, &tex->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (tex->flip_image_vertical) {
            _flip_image_vertical(data, tex->width, tex->height);
        }

        // 将纹理数据复制到 GL

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (filename.ends_with(".ase")) {
        cute_aseprite_free(ase);
    } else {
        stbi_image_free(data);
    }

    // tex->last_modified = modtime;
    console_printf(" successful\n");
    return true;
}

bool texture_load(AssetTexture *tex, String filename, bool flip_image_vertical) {
    tex->id = 0;
    tex->flip_image_vertical = flip_image_vertical;
    return _texture_load_vfs(tex, filename);
}

void texture_bind(const char *filename) {
    LockGuard<Mutex> lock{g_app->gpu_mtx};

    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);

    if (ok && a.texture.id != 0) glBindTexture(GL_TEXTURE_2D, a.texture.id);
}

vec2 texture_get_size(const char *filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    error_assert(ok);
    return luavec2(a.texture.width, a.texture.height);
}

AssetTexture texture_get_ptr(const char *filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    return a.texture;
}

bool texture_update(AssetTexture *tex, String filename) { return _texture_load_vfs(tex, filename); }

bool load_texture_data_from_memory(const void *memory, size_t sz, i32 *width, i32 *height, u32 *num_comps, void **data, bool flip_vertically_on_load) {
    // Load texture data

    int channels;
    u8 *image = (u8 *)stbi_load_from_memory((unsigned char *)memory, sz, width, height, &channels, 0);

    // if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = image;

    if (!*data) {
        // neko_image_free(&img);
        console_log("could not load image %p", memory);
        return false;
    }
    return true;
}

bool load_texture_data_from_file(const char *file_path, i32 *width, i32 *height, u32 *num_comps, void **data, bool flip_vertically_on_load) {
    size_t len = 0;
    const_str file_data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, file_path, &len);
    neko_assert(file_data);
    bool ret = load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        console_log("could not load texture: %s", file_path);
    }
    mem_free(file_data);
    return ret;
}

bool neko_asset_texture_load_from_file(const_str path, void *out, gfx_texture_desc_t *desc, bool flip_on_load, bool keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(gfx_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = GL_RGBA;
        t->desc.min_filter = GL_LINEAR;
        t->desc.mag_filter = GL_LINEAR;
        t->desc.wrap_s = GL_REPEAT;
        t->desc.wrap_t = GL_REPEAT;
    }

    void *tex_data = NULL;
    i32 w, h;
    u32 cc;
    load_texture_data_from_file(path, &w, &h, &cc, &tex_data, false);
    neko_defer(mem_free(tex_data));

    t->desc.data = tex_data;
    t->desc.width = w;
    t->desc.height = h;

    if (!t->desc.data) {

        console_log("failed to load texture data %s", path);
        return false;
    }

    t->hndl = gfx_texture_create(t->desc);

    if (!keep_data) {
        // neko_image_free(&img);
        t->desc.data = NULL;
    }

    return true;
}

bool neko_asset_texture_load_from_memory(const void *memory, size_t sz, void *out, gfx_texture_desc_t *desc, bool flip_on_load, bool keep_data) {
    neko_asset_texture_t *t = (neko_asset_texture_t *)out;

    memset(&t->desc, 0, sizeof(gfx_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = GL_RGBA;
        t->desc.min_filter = GL_LINEAR;
        t->desc.mag_filter = GL_LINEAR;
        t->desc.wrap_s = GL_REPEAT;
        t->desc.wrap_t = GL_REPEAT;
    }

    // Load texture data
    i32 num_comps = 0;
    bool loaded = load_texture_data_from_memory(memory, sz, (i32 *)&t->desc.width, (i32 *)&t->desc.height, (u32 *)&num_comps, (void **)&t->desc.data, t->desc.flip_y);

    if (!loaded) {
        return false;
    }

    t->hndl = gfx_texture_create(t->desc);

    if (!keep_data) {
        mem_free(t->desc.data);
        t->desc.data = NULL;
    }

    return true;
}

#if 0

bool Atlas::load(String filepath, bool generate_mips) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    Image img = {};
    HashMap<AtlasImage> by_name = {};

    for (String line : SplitLines(contents)) {
        switch (line.data[0]) {
            case 'a': {
                StringScanner scan = line;
                scan.next_string();  // discard 'a'
                String filename = scan.next_string();

                StringBuilder sb = {};
                neko_defer(sb.trash());
                sb.swap_filename(filepath, filename);
                bool ok = img.load(String(sb), generate_mips);
                if (!ok) {
                    return false;
                }
                break;
            }
            case 's': {
                if (img.id == 0) {
                    return false;
                }

                StringScanner scan = line;
                scan.next_string();  // discard 's'
                String name = scan.next_string();
                scan.next_string();  // discard origin x
                scan.next_string();  // discard origin y
                i32 x = scan.next_int();
                i32 y = scan.next_int();
                i32 width = scan.next_int();
                i32 height = scan.next_int();
                i32 padding = scan.next_int();
                i32 trimmed = scan.next_int();
                scan.next_int();  // discard trim x
                scan.next_int();  // discard trim y
                i32 trim_width = scan.next_int();
                i32 trim_height = scan.next_int();

                AtlasImage atlas_img = {};
                atlas_img.img = img;
                atlas_img.u0 = (x + padding) / (float)img.width;
                atlas_img.v0 = (y + padding) / (float)img.height;

                if (trimmed != 0) {
                    atlas_img.width = (float)trim_width;
                    atlas_img.height = (float)trim_height;
                    atlas_img.u1 = (x + padding + trim_width) / (float)img.width;
                    atlas_img.v1 = (y + padding + trim_height) / (float)img.height;
                } else {
                    atlas_img.width = (float)width;
                    atlas_img.height = (float)height;
                    atlas_img.u1 = (x + padding + width) / (float)img.width;
                    atlas_img.v1 = (y + padding + height) / (float)img.height;
                }

                by_name[fnv1a(name)] = atlas_img;

                break;
            }
            default:
                break;
        }
    }

    console_log("created atlas with image id: %d and %llu entries", img.id, (unsigned long long)by_name.load);

    Atlas a;
    a.by_name = by_name;
    a.img = img;
    *this = a;

    return true;
}

void Atlas::trash() {
    by_name.trash();
    img.trash();
}

AtlasImage *Atlas::get(String name) {
    u64 key = fnv1a(name);
    return by_name.get(key);
}

#endif

/*==========================
// NEKO_PACK
==========================*/

static int _compare_item_paths(const void *_a, const void *_b) {
    // 要保证a与b不为NULL
    char *a = *(char **)_a;
    char *b = *(char **)_b;
    u8 al = (u8)strlen(a);
    u8 bl = (u8)strlen(b);
    int difference = al - bl;
    if (difference != 0) return difference;
    return memcmp(a, b, al * sizeof(u8));
}

static void _destroy_pack_items(u64 item_count, PakItem *items) {
    neko_assert(item_count == 0 || (item_count > 0 && items));

    for (u64 i = 0; i < item_count; i++) mem_free(items[i].path);
    mem_free(items);
}

static bool _create_pack_items(vfs_file *pak, u64 item_count, PakItem **_items) {
    neko_assert(pak);
    neko_assert(item_count > 0);
    neko_assert(_items);

    PakItem *items = (PakItem *)mem_alloc(item_count * sizeof(PakItem));

    if (!items) return false;  // FAILED_TO_ALLOCATE_PACK_RESULT

    for (u64 i = 0; i < item_count; i++) {
        PakItemInfo info;

        size_t result = neko_capi_vfs_fread(&info, sizeof(PakItemInfo), 1, pak);

        if (result != 1) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        if (info.data_size == 0 || info.path_size == 0) {
            _destroy_pack_items(i, items);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        char *path = (char *)mem_alloc((info.path_size + 1) * sizeof(char));

        if (!path) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
        }

        result = neko_capi_vfs_fread(path, sizeof(char), info.path_size, pak);

        path[info.path_size] = 0;

        if (result != info.path_size) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        i64 fileOffset = info.zip_size > 0 ? info.zip_size : info.data_size;

        int seekResult = neko_capi_vfs_fseek(pak, fileOffset, SEEK_CUR);

        if (seekResult != 0) {
            _destroy_pack_items(i, items);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        PakItem *item = &items[i];
        item->info = info;
        item->path = path;
    }

    *_items = items;
    return true;
}

static int _compare_pack_items(const void *_a, const void *_b) {
    const PakItem *a = (PakItem *)_a;
    const PakItem *b = (PakItem *)_b;

    int difference = (int)a->info.path_size - (int)b->info.path_size;

    if (difference != 0) return difference;

    return memcmp(a->path, b->path, a->info.path_size * sizeof(char));
}

bool pak_load(Pak *pak, const_str file_path, u32 data_buffer_capacity, bool is_resources_directory) {
    neko_assert(file_path);

    // memset(pak, 0, sizeof(Pak));

    pak->zip_buffer = NULL;
    pak->zip_size = 0;

    pak->vf = neko_capi_vfs_fopen(file_path);

    if (!pak->vf.data) return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT

    char header[NEKO_PAK_HEAD_SIZE];
    i32 buildnum;

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &pak->vf);
    result += neko_capi_vfs_fread(&buildnum, sizeof(i32), 1, &pak->vf);

    // 检查文件头
    if (result != NEKO_PAK_HEAD_SIZE + 1 ||  //
        header[0] != 'N' ||                  //
        header[1] != 'E' ||                  //
        header[2] != 'K' ||                  //
        header[3] != 'O' ||                  //
        header[4] != 'P' ||                  //
        header[5] != 'A' ||                  //
        header[6] != 'C' ||                  //
        header[7] != 'K') {
        pak_fini(pak);
        return false;
    }

    u64 item_count;

    result = neko_capi_vfs_fread(&item_count, sizeof(u64), 1, &pak->vf);

    if (result != 1 ||  //
        item_count == 0) {
        pak_fini(pak);
        return false;
    }

    PakItem *items;

    bool ok = _create_pack_items(&pak->vf, item_count, &items);

    if (!ok) {
        pak_fini(pak);
        return false;
    }

    pak->item_count = item_count;
    pak->items = items;

    u8 *_data_buffer = NULL;

    if (data_buffer_capacity > 0) {
        _data_buffer = (u8 *)mem_alloc(data_buffer_capacity * sizeof(u8));
    } else {
        _data_buffer = NULL;
    }

    pak->data_buffer = _data_buffer;
    pak->data_size = data_buffer_capacity;

    console_log("load pack %s buildnum: %d (engine %d)", neko_util_get_filename(file_path), buildnum, neko_buildnum());

    return true;
}
void pak_fini(Pak *pak) {
    if (pak->file_ref_count != 0) {
        console_log("assets loader leaks detected %d refs", pak->file_ref_count);
    }

    pak_free_buffer(pak);

    _destroy_pack_items(pak->item_count, pak->items);
    if (pak->vf.data) neko_capi_vfs_fclose(&pak->vf);
}

u64 pak_get_item_index(Pak *pak, const_str path) {
    neko_assert(path);
    neko_assert(strlen(path) <= u8_max);

    PakItem *search_item = &pak->search_item;

    search_item->info.path_size = (u8)strlen(path);
    search_item->path = (char *)path;

    PakItem *items = (PakItem *)bsearch(search_item, pak->items, pak->item_count, sizeof(PakItem), _compare_pack_items);

    if (!items) return u64_max;

    u64 index = items - pak->items;
    return index;
}

bool pak_get_data(Pak *pak, u64 index, String *out, u32 *size) {
    neko_assert((index < pak->item_count) && out && size);

    PakItemInfo info = pak->items[index].info;

    u8 *_data_buffer = pak->data_buffer;
    if (_data_buffer) {
        if (info.data_size > pak->data_size) {
            _data_buffer = (u8 *)mem_realloc(_data_buffer, info.data_size * sizeof(u8));
            pak->data_buffer = _data_buffer;
            pak->data_size = info.data_size;
        }
    } else {
        _data_buffer = (u8 *)mem_alloc(info.data_size * sizeof(u8));
        pak->data_buffer = _data_buffer;
        pak->data_size = info.data_size;
    }

    u8 *_zip_buffer = pak->zip_buffer;
    if (pak->zip_buffer) {
        if (info.zip_size > pak->zip_size) {
            _zip_buffer = (u8 *)mem_realloc(pak->zip_buffer, info.zip_size * sizeof(u8));
            pak->zip_buffer = _zip_buffer;
            pak->zip_size = info.zip_size;
        }
    } else {
        if (info.zip_size > 0) {
            _zip_buffer = (u8 *)mem_alloc(info.zip_size * sizeof(u8));
            pak->zip_buffer = _zip_buffer;
            pak->zip_size = info.zip_size;
        }
    }

    vfs_file *vf = &pak->vf;

    i64 file_offset = (i64)(info.file_offset + sizeof(PakItemInfo) + info.path_size);

    int seek_result = neko_capi_vfs_fseek(vf, file_offset, SEEK_SET);

    if (seek_result != 0) return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT

    if (info.zip_size > 0) {
        size_t result = neko_capi_vfs_fread(_zip_buffer, sizeof(u8), info.zip_size, vf);

        if (result != info.zip_size) return false;  // FAILED_TO_READ_FILE_PACK_RESULT

        // result = neko_lz_decode(_zip_buffer, info.zip_size, _data_buffer, info.data_size);

        unsigned long decompress_buf_len = info.data_size;
        int decompress_status = uncompress(_data_buffer, &decompress_buf_len, _zip_buffer, info.zip_size);

        result = decompress_buf_len;

        console_log("[assets] neko_lz_decode %u %u", info.zip_size, info.data_size);

        if (result < 0 || result != info.data_size || decompress_status != MZ_OK) {
            return false;  // FAILED_TO_DECOMPRESS_PACK_RESULT
        }
    } else {
        size_t result = neko_capi_vfs_fread(_data_buffer, sizeof(u8), info.data_size, vf);

        if (result != info.data_size) return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    }

    (*size) = info.data_size;

    char *data = (char *)mem_alloc(info.data_size + 1);
    memcpy((void *)(data), _data_buffer, info.data_size);
    data[info.data_size] = 0;
    *out = {data, info.data_size};

    pak->file_ref_count++;
    return true;
}

bool pak_get_data(Pak *pak, const_str path, String *out, u32 *size) {
    neko_assert(path && out && size);
    neko_assert(strlen(path) <= u8_max);
    u64 index = pak_get_item_index(pak, path);
    if (index == u64_max) return false;  // FAILED_TO_GET_ITEM_PACK_RESULT
    return pak_get_data(pak, index, out, size);
}

void pak_free_item(Pak *pak, String data) {
    mem_free(data.data);
    pak->file_ref_count--;
}

void pak_free_buffer(Pak *pak) {

    mem_free(pak->data_buffer);
    mem_free(pak->zip_buffer);
    pak->data_buffer = NULL;
    pak->zip_buffer = NULL;
}

static void pak_remove_item(u64 item_count, PakItem *pack_items) {
    neko_assert(item_count == 0 || (item_count > 0 && pack_items));
    for (u64 i = 0; i < item_count; i++) remove(pack_items[i].path);
}

bool neko_pak_unzip(const_str file_path, bool print_progress) {
    neko_assert(file_path);

    Pak pak;

    int pack_result = pak_load(&pak, file_path, 128, false);

    if (pack_result != 0) return pack_result;

    u64 total_raw_size = 0, total_zip_size = 0;

    u64 item_count = pak.item_count;
    PakItem *items = pak.items;

    for (u64 i = 0; i < item_count; i++) {
        PakItem *item = &items[i];

        if (print_progress) {
            console_log("Unpacking %s", item->path);
        }

        String data;
        u32 data_size;

        pack_result = pak_get_data(&pak, i, &data, &data_size);

        if (pack_result != 0) {
            pak_remove_item(i, items);
            pak_fini(&pak);
            return pack_result;
        }

        u8 path_size = item->info.path_size;

        char item_path[u8_max + 1];

        memcpy(item_path, item->path, path_size * sizeof(char));
        item_path[path_size] = 0;

        // 解压的时候路径节替换为 '-'
        for (u8 j = 0; j < path_size; j++)
            if (item_path[j] == '/' || item_path[j] == '\\' || (item_path[j] == '.' && j == 0)) item_path[j] = '-';

        FILE *item_file = neko_fopen(item_path, "wb");

        if (!item_file) {
            pak_remove_item(i, items);
            pak_fini(&pak);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        size_t result = fwrite(data.data, sizeof(u8), data_size, item_file);

        neko_fclose(item_file);

        if (result != data_size) {
            pak_remove_item(i, items);
            pak_fini(&pak);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        if (print_progress) {
            u32 raw_file_size = item->info.data_size;
            u32 zip_file_size = item->info.zip_size > 0 ? item->info.zip_size : item->info.data_size;

            total_raw_size += raw_file_size;
            total_zip_size += zip_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", raw_file_size, zip_file_size, progress);
        }
    }

    pak_fini(&pak);

    if (print_progress) {
        neko_printf("Unpacked %llu files. (%llu/%llu bytes)\n", (long long unsigned int)item_count, (long long unsigned int)total_raw_size, (long long unsigned int)total_zip_size);
    }

    return true;
}

bool neko_write_pack_items(FILE *pack_file, u64 item_count, char **item_paths, bool print_progress) {
    neko_assert(pack_file);
    neko_assert(item_count > 0);
    neko_assert(item_paths);

    u32 buffer_size = 128;  // 提高初始缓冲大小

    u8 *item_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    u8 *zip_data = (u8 *)mem_alloc(sizeof(u8) * buffer_size);
    if (!zip_data) {
        mem_free(item_data);
        return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
    }

    u64 total_zip_size = 0, total_raw_size = 0;

    for (u64 i = 0; i < item_count; i++) {
        char *item_path = item_paths[i];

        if (print_progress) {
            neko_printf("Packing \"%s\" file. ", item_path);
            fflush(stdout);
        }

        size_t path_size = strlen(item_path);

        if (path_size > u8_max) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        FILE *item_file = neko_fopen(item_path, "rb");

        if (!item_file) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT
        }

        int seek_result = neko_fseek(item_file, 0, SEEK_END);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        u64 item_size = (u64)neko_ftell(item_file);

        if (item_size == 0 || item_size > UINT32_MAX) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // BAD_DATA_SIZE_PACK_RESULT
        }

        seek_result = neko_fseek(item_file, 0, SEEK_SET);

        if (seek_result != 0) {
            neko_fclose(item_file);
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_SEEK_FILE_PACK_RESULT
        }

        if (item_size > buffer_size) {
            u8 *new_buffer = (u8 *)mem_realloc(item_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
            }

            item_data = new_buffer;

            new_buffer = (u8 *)mem_realloc(zip_data, item_size * sizeof(u8));

            if (!new_buffer) {
                neko_fclose(item_file);
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_ALLOCATE_PACK_RESULT
            }

            zip_data = new_buffer;
        }

        size_t result = neko_fread(item_data, sizeof(u8), item_size, item_file);

        neko_fclose(item_file);

        if (result != item_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_READ_FILE_PACK_RESULT
        }

        size_t zip_size;

        if (item_size > 1) {

            unsigned long compress_buf_len = compressBound(item_size);
            int compress_status = compress(zip_data, &compress_buf_len, (const unsigned char *)item_data, item_size);

            zip_size = compress_buf_len;

            // const int max_dst_size = neko_lz_bounds(item_size, 0);
            // zip_size = neko_lz_encode(item_data, item_size, zip_data, max_dst_size, 9);

            if (zip_size <= 0 || zip_size >= item_size || compress_status != MZ_OK) {
                zip_size = 0;
            }
        } else {
            zip_size = 0;
        }

        i64 file_offset = neko_ftell(pack_file);

        PakItemInfo info = {
                (u32)zip_size,
                (u32)item_size,
                (u64)file_offset,
                (u8)path_size,
        };

        result = fwrite(&info, sizeof(PakItemInfo), 1, pack_file);

        if (result != 1) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        result = fwrite(item_path, sizeof(char), info.path_size, pack_file);

        if (result != info.path_size) {
            mem_free(zip_data);
            mem_free(item_data);
            return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
        }

        if (zip_size > 0) {
            result = fwrite(zip_data, sizeof(u8), zip_size, pack_file);

            if (result != zip_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        } else {
            result = fwrite(item_data, sizeof(u8), item_size, pack_file);

            if (result != item_size) {
                mem_free(zip_data);
                mem_free(item_data);
                return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
            }
        }

        if (print_progress) {
            u32 zip_file_size = zip_size > 0 ? (u32)zip_size : (u32)item_size;
            u32 raw_file_size = (u32)item_size;

            total_zip_size += zip_file_size;
            total_raw_size += raw_file_size;

            int progress = (int)(((f32)(i + 1) / (f32)item_count) * 100.0f);

            neko_printf("(%u/%u bytes) [%d%%]\n", zip_file_size, raw_file_size, progress);
            fflush(stdout);
        }
    }

    mem_free(zip_data);
    mem_free(item_data);

    if (print_progress) {
        int compression = (int)((1.0 - (f64)(total_zip_size) / (f64)total_raw_size) * 100.0);
        neko_printf("Packed %llu files. (%llu/%llu bytes, %d%% saved)\n", (long long unsigned int)item_count, (long long unsigned int)total_zip_size, (long long unsigned int)total_raw_size,
                    compression);
    }

    return true;
}

bool neko_pak_build(const_str file_path, u64 file_count, const_str *file_paths, bool print_progress) {
    neko_assert(file_path);
    neko_assert(file_count > 0);
    neko_assert(file_paths);

    char **item_paths = (char **)mem_alloc(file_count * sizeof(char *));

    if (!item_paths) return false;  // FAILED_TO_ALLOCATE_PACK_RESULT

    u64 item_count = 0;

    for (u64 i = 0; i < file_count; i++) {
        bool already_added = false;

        for (u64 j = 0; j < item_count; j++) {
            if (i != j && strcmp(file_paths[i], item_paths[j]) == 0) already_added = true;
        }

        if (!already_added) item_paths[item_count++] = (char *)file_paths[i];
    }

    qsort(item_paths, item_count, sizeof(char *), _compare_item_paths);

    FILE *pack_file = neko_fopen(file_path, "wb");

    if (!pack_file) {
        mem_free(item_paths);
        return false;  // FAILED_TO_CREATE_FILE_PACK_RESULT
    }

    char header[NEKO_PAK_HEAD_SIZE] = {
            'N', 'E', 'K', 'O', 'P', 'A', 'C', 'K',
    };

    i32 buildnum = neko_buildnum();

    size_t write_result = fwrite(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, pack_file);
    write_result += fwrite(&buildnum, sizeof(i32), 1, pack_file);

    if (write_result != NEKO_PAK_HEAD_SIZE + 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
    }

    write_result = fwrite(&item_count, sizeof(u64), 1, pack_file);

    if (write_result != 1) {
        mem_free(item_paths);
        neko_fclose(pack_file);
        remove(file_path);
        return false;  // FAILED_TO_WRITE_FILE_PACK_RESULT
    }

    bool ok = neko_write_pack_items(pack_file, item_count, item_paths, print_progress);

    mem_free(item_paths);
    neko_fclose(pack_file);

    if (!ok) {
        remove(file_path);
        return ok;
    }

    neko_printf("Packed %s ok\n", file_path);

    return true;
}

bool neko_pak_info(const_str file_path, i32 *buildnum, u64 *item_count) {
    neko_assert(file_path);
    neko_assert(buildnum);
    neko_assert(item_count);

    vfs_file vf = neko_capi_vfs_fopen(file_path);

    if (!vf.data) return false;  // FAILED_TO_OPEN_FILE_PACK_RESULT

    char header[NEKO_PAK_HEAD_SIZE];

    size_t result = neko_capi_vfs_fread(header, sizeof(u8), NEKO_PAK_HEAD_SIZE, &vf);
    result += neko_capi_vfs_fread(buildnum, sizeof(i32), 1, &vf);

    if (result != NEKO_PAK_HEAD_SIZE + 1) {
        neko_capi_vfs_fclose(&vf);
        return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    }

    if (header[0] != 'N' ||  //
        header[1] != 'E' ||  //
        header[2] != 'K' ||  //
        header[3] != 'O' ||  //
        header[4] != 'P' ||  //
        header[5] != 'A' ||  //
        header[6] != 'C' ||  //
        header[7] != 'K') {
        neko_capi_vfs_fclose(&vf);
        return false;  // BAD_FILE_TYPE_PACK_RESULT
    }

    result = neko_capi_vfs_fread(item_count, sizeof(u64), 1, &vf);

    neko_capi_vfs_fclose(&vf);

    if (result != 1) return false;  // FAILED_TO_READ_FILE_PACK_RESULT
    return true;
}

// mt_pak

struct pak_assets_t {
    const_str name;
    size_t size;
    String data;
};

static Pak *check_pak_udata(lua_State *L, i32 arg) {
    Pak *udata = (Pak *)luaL_checkudata(L, arg, "mt_pak");
    if (!udata || !udata->item_count) {
        luaL_error(L, "cannot read pak");
    }
    return udata;
}

static int mt_pak_gc(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);
    pak_fini(pak);
    console_log("pak __gc %p", pak);
    return 0;
}

static int mt_pak_items(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    u64 item_count = pak_get_item_count(pak);

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, pak_get_item_path(pak, i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int mt_pak_assets_load(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    const_str path = lua_tostring(L, 2);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_newuserdata(L, sizeof(pak_assets_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    bool ok = pak_get_data(pak, path, &assets_user_handle->data, (u32 *)&assets_user_handle->size);

    if (!ok) {
        const_str error_message = "mt_pak_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // asset_write(asset);

    return 1;
}

static int mt_pak_assets_unload(lua_State *L) {
    Pak *pak = check_pak_udata(L, 1);

    pak_assets_t *assets_user_handle = (pak_assets_t *)lua_touserdata(L, 2);

    if (assets_user_handle && assets_user_handle->data.len)
        pak_free_item(pak, assets_user_handle->data);
    else
        console_log("unknown assets unload %p", assets_user_handle);

    // asset_write(asset);

    return 0;
}

int open_mt_pak(lua_State *L) {
    // clang-format off
    luaL_Reg reg[] = {
            {"__gc", mt_pak_gc},
            {"items", mt_pak_items},
            {"assets_load", mt_pak_assets_load},
            {"assets_unload", mt_pak_assets_unload},
            {nullptr, nullptr},
    };
    // clang-format on

    luax_new_class(L, "mt_pak", reg);
    return 0;
}

int neko_pak_load(lua_State *L) {
    String name = luax_check_string(L, 1);
    String path = luax_check_string(L, 2);

    Pak pak;

    bool ok = pak_load(&pak, path.cstr(), 0, false);

    if (!ok) {
        return 0;
    }

    luax_new_userdata(L, pak, "mt_pak");
    return 1;
}

Assets g_assets = {};

static void hot_reload_thread(void *) {
    u32 reload_interval = g_app->reload_interval.load() * 1000;

    while (true) {
        PROFILE_BLOCK("hot reload");

        {
            LockGuard<Mutex> lock{g_assets.shutdown_mtx};
            if (g_assets.shutdown) {
                return;
            }

            bool signaled = g_assets.shutdown_notify.wait_for(lock, std::chrono::microseconds(reload_interval)) == std::cv_status::no_timeout;
            if (signaled) {
                return;
            }
        }

        {
            PROFILE_BLOCK("check for updates");

            g_assets.rw_lock.shared_lock();
            neko_defer(g_assets.rw_lock.shared_unlock());

            g_assets.tmp_changes.len = 0;

            for (auto [k, v] : g_assets.table) {
                PROFILE_BLOCK("read modtime");

                u64 modtime = vfs_file_modtime(v->name);
                if (modtime > v->modtime) {
                    FileChange change = {};
                    change.key = v->hash;
                    change.modtime = modtime;

                    g_assets.tmp_changes.push(change);
                }
            }
        }

        if (g_assets.tmp_changes.len > 0) {
            LockGuard<Mutex> lock{g_assets.changes_mtx};
            for (FileChange change : g_assets.tmp_changes) {
                g_assets.changes.push(change);
            }
        }
    }
}

int assets_perform_hot_reload_changes(App *app, event_t evt) {
    LockGuard<Mutex> lock{g_assets.changes_mtx};

    if (g_assets.changes.len == 0) {
        return 0;
    }

    PROFILE_BLOCK("perform hot reload");

    auto L = ENGINE_LUA();

    for (FileChange change : g_assets.changes) {
        Asset a = {};
        bool exists = asset_read(change.key, &a);
        neko_assert(exists);

        a.modtime = change.modtime;

        bool ok = false;
        switch (a.kind) {
            case AssetKind_LuaRef: {
                luaL_unref(L, LUA_REGISTRYINDEX, a.lua_ref);
                a.lua_ref = luax_require_script(L, a.name);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_update(&a.texture, a.name);
                break;
            }
            case AssetKind_AseSprite: {
                a.sprite.trash();
                ok = a.sprite.load(a.name);
                break;
            }
            case AssetKind_Tiledmap: {
                tiled_unload(&a.tiledmap);
                ok = tiled_load(&a.tiledmap, a.name.cstr(), NULL);
                break;
            }
            case AssetKind_Shader: {
                GLuint save_sid = a.shader.id;
                neko_unload_shader(&a.shader);
                ok = neko_load_shader(&a.shader, a.name);
                neko_assert(save_sid == a.shader.id);
                break;
            }
            default:
                continue;
                break;
        }

        if (!ok) {
            fatal_error(tmp_fmt("failed to hot reload: %s", a.name.data));
            return 0;
        }

        asset_write(a);
        console_log("reloaded: %s", a.name.data);
    }

    g_assets.changes.len = 0;

    return 0;
}

void assets_shutdown() {
    if (g_app->hot_reload_enabled.load()) {
        {
            LockGuard<Mutex> lock{g_assets.shutdown_mtx};
            g_assets.shutdown = true;
        }

        g_assets.shutdown_notify.notify_one();
        g_assets.reload_thread.join();
        g_assets.changes.trash();
        g_assets.tmp_changes.trash();
    }

    for (auto [k, v] : g_assets.table) {
        mem_free(v->name.data);

        switch (v->kind) {
            case AssetKind_Image:
                // v->image.trash();
                break;
            case AssetKind_AseSprite:
                v->sprite.trash();
                break;
            case AssetKind_Shader:
                neko_unload_shader(&v->shader);
                break;
            case AssetKind_Tiledmap:
                tiled_unload(&v->tiledmap);
                break;
            default:
                break;
        }
    }
    g_assets.table.trash();

    g_assets.rw_lock.trash();
}

void assets_start_hot_reload() {

    g_assets.rw_lock.make();

    if (g_app->hot_reload_enabled.load()) {
        g_assets.reload_thread.make(hot_reload_thread, nullptr);
    }
}

bool asset_load_kind(AssetKind kind, String filepath, Asset *out) {
    AssetLoadData data = {};
    data.kind = kind;

    return asset_load(data, filepath, out);
}

bool asset_load(AssetLoadData desc, String filepath, Asset *out) {
    PROFILE_FUNC();

    u64 key = fnv1a(filepath);

    {
        Asset asset = {};
        if (asset_read(key, &asset)) {
            if (out != nullptr) {
                *out = asset;
            }
            return true;
        }
    }

    {
        PROFILE_BLOCK("load new asset");

        Asset asset = {};
        asset.name = to_cstr(filepath);
        asset.hash = key;
        {
            PROFILE_BLOCK("asset modtime")
            asset.modtime = vfs_file_modtime(asset.name);
        }
        asset.kind = desc.kind;

        bool ok = false;
        switch (desc.kind) {
            case AssetKind_LuaRef: {
                asset.lua_ref = LUA_REFNIL;
                asset_write(asset);
                asset.lua_ref = luax_require_script(ENGINE_LUA(), filepath);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_load(&asset.texture, filepath.cstr(), desc.flip_image_vertical);
                break;
            }
            case AssetKind_AseSprite: {
                ok = asset.sprite.load(filepath);
                break;
            }
            case AssetKind_Shader: {
                ok = neko_load_shader(&asset.shader, filepath);
                break;
            }
            case AssetKind_Tiledmap: {
                ok = tiled_load(&asset.tiledmap, filepath.cstr(), NULL);
                break;
            }
            // case AssetKind_Pak:
            //     ok = asset.pak.load(filepath.data, 0, false);
            //     break;
            default:
                console_log("asset_load %d undefined", desc.kind);
                break;
        }

        if (!ok) {
            mem_free(asset.name.data);
            return false;
        }

        asset_write(asset);

        if (out != nullptr) {
            *out = asset;
        }
        return true;
    }
}

bool asset_read(u64 key, Asset *out) {
    g_assets.rw_lock.shared_lock();
    neko_defer(g_assets.rw_lock.shared_unlock());

    const Asset *asset = g_assets.table.get(key);
    if (asset == nullptr) {
        return false;
    }

    *out = *asset;
    return true;
}

void asset_write(Asset asset) {
    g_assets.rw_lock.unique_lock();
    neko_defer(g_assets.rw_lock.unique_unlock());

    g_assets.table[asset.hash] = asset;
}

Asset check_asset(lua_State *L, u64 key) {
    Asset asset = {};
    if (!asset_read(key, &asset)) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

Asset check_asset_mt(lua_State *L, i32 arg, const char *mt) {
    u64 *udata = (u64 *)luaL_checkudata(L, arg, mt);

    Asset asset = {};
    bool ok = asset_read(*udata, &asset);
    if (!ok) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

char *file_pathabs(const char *pathfile) {
    String out = tmp_fmt("%*.s", DIR_MAX + 1, "");
#ifdef NEKO_IS_WIN32
    _fullpath(out.data, pathfile, DIR_MAX);
#else
    realpath(pathfile, out.data);
#endif
    return out.data;
}

bool AseSpriteData::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase = nullptr;
    {
        PROFILE_BLOCK("aseprite load");
        ase = cute_aseprite_load_from_memory(contents.data, (i32)contents.len, nullptr);
    }
    neko_defer(cute_aseprite_free(ase));

    Arena arena = {};

    i32 rect = ase->w * ase->h * 4;

    Slice<AseSpriteFrame> frames = {};
    frames.resize(&arena, ase->frame_count);

    Array<char> pixels = {};
    pixels.reserve(ase->frame_count * rect);
    neko_defer(pixels.trash());

    for (i32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        AseSpriteFrame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (float)i / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (float)(i + 1) / ase->frame_count;

        frames[i] = sf;
        memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    // sg_image_desc desc = {};
    int ase_width = ase->w;
    int ase_height = ase->h * ase->frame_count;
    // desc.data.subimage[0][0].ptr = pixels.data;
    // desc.data.subimage[0][0].size = ase->frame_count * rect;

    u8 *data = reinterpret_cast<u8 *>(pixels.data);

    // stbi_write_png("h.png", ase_width, ase_height, 4, data, 0);

    gfx_texture_t new_tex = NEKO_DEFAULT_VAL();
    {
        PROFILE_BLOCK("make image");

        gfx_texture_desc_t t_desc = {};

        t_desc.format = GL_RGBA;
        t_desc.mag_filter = GL_NEAREST;
        t_desc.min_filter = GL_NEAREST;
        // t_desc.num_mips = 0;
        t_desc.width = ase_width;
        t_desc.height = ase_height;
        // t_desc.num_comps = 4;
        t_desc.data = data;

        // neko_tex_flip_vertically(width, height, (u8 *)(t_desc.data[0]));
        new_tex = gfx_texture_create(t_desc);
    }

    HashMap<AseSpriteLoop> by_tag = {};
    by_tag.reserve(ase->tag_count);

    for (i32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

        AseSpriteLoop loop = {};

        loop.indices.resize(&arena, len);
        for (i32 j = 0; j < len; j++) {
            loop.indices[j] = j + tag.from_frame;
        }

        by_tag[fnv1a(tag.name)] = loop;
    }

    console_log("created sprite with image id: %d and %llu frames", new_tex.id, (unsigned long long)frames.len);

    AseSpriteData s = {};
    s.arena = arena;
    s.tex = new_tex;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *this = s;
    return true;
}

void AseSpriteData::trash() {
    by_tag.trash();
    arena.trash();
}

void AseSprite::make() {}

bool AseSprite::play(String tag) {
    u64 key = fnv1a(tag);
    bool same = loop == key;
    loop = key;
    return same;
}

void AseSprite::update(float dt) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    i32 index = view.frame();
    AseSpriteFrame frame = view.data.frames[index];

    elapsed += dt * 1000;
    if (elapsed > frame.duration) {
        if (current_frame == view.len() - 1) {
            current_frame = 0;
        } else {
            current_frame++;
        }

        elapsed -= frame.duration;
    }
}

void AseSprite::set_frame(i32 frame) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    if (0 <= frame && frame < view.len()) {
        current_frame = frame;
        elapsed = 0;
    }
}

bool AseSpriteView::make(AseSprite *spr) {
    Asset a = {};
    bool ok = asset_read(spr->sprite, &a);
    if (!ok) {
        return false;
    }

    AseSpriteData data = a.sprite;
    const AseSpriteLoop *res = data.by_tag.get(spr->loop);

    AseSpriteView view = {};
    view.sprite = spr;
    view.data = data;

    if (res != nullptr) {
        view.loop = *res;
    }

    *this = view;
    return true;
}

i32 AseSpriteView::frame() {
    if (loop.indices.data != nullptr) {
        return loop.indices[sprite->current_frame];
    } else {
        return sprite->current_frame;
    }
}

u64 AseSpriteView::len() {
    if (loop.indices.data != nullptr) {
        return loop.indices.len;
    } else {
        return data.frames.len;
    }
}

#define xml_expect_not_end(c_)             \
    if (!*(c_)) {                          \
        xml_emit_error("Unexpected end."); \
        return {};                         \
    }

typedef struct xml_entity_t {
    char character;
    const_str name;
} xml_entity_t;

static xml_entity_t g_xml_entities[] = {{'&', "&amp;"}, {'\'', "&apos;"}, {'"', "&quot;"}, {'<', "&lt;"}, {'>', "&gt;"}};
static const_str g_xml_error = NULL;

static void xml_emit_error(const_str error) { g_xml_error = error; }

static char *xml_copy_string(const_str str, u32 len) {
    char *r = (char *)mem_alloc(len + 1);
    if (!r) {
        xml_emit_error("Out of memory!");
        return NULL;
    }
    r[len] = '\0';

    for (u32 i = 0; i < len; i++) {
        r[i] = str[i];
    }

    return r;
}

static bool xml_string_is_decimal(const_str str, u32 len) {
    u32 i = 0;
    if (str[0] == '-') i++;

    bool used_dot = false;

    for (; i < len; i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            if (c == '.' && !used_dot) {
                used_dot = true;
                continue;
            }
            return false;
        }
    }

    return true;
}

static bool xml_string_equal(const_str str_a, u32 len, const_str str_b) {
    for (u32 i = 0; i < len; i++) {
        if (str_a[i] != str_b[i]) return false;
    }

    return true;
}

static u64 xml_hash_string(const_str str, u32 len) {
    u64 hash = 0, x = 0;

    for (u32 i = 0; i < len; i++) {
        hash = (hash << 4) + str[i];
        if ((x = hash & 0xF000000000LL) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash & 0x7FFFFFFFFF);
}

static void xml_node_free(xml_node_t *node) {
    for (auto kv : node->attributes) {
        xml_attribute_t *attrib = kv.value;
        if (attrib->type == NEKO_XML_ATTRIBUTE_STRING) {
            mem_free(attrib->value.string);
        }
        mem_free(attrib->name);
    }

    for (u32 i = 0; i < node->children.len; i++) {
        xml_node_free(&node->children[i]);
    }

    mem_free(node->name);
    mem_free(node->text);
    node->attributes.trash();
    node->children.trash();
}

static char *xml_process_text(const_str start, u32 length) {
    char *r = (char *)mem_alloc(length + 1);

    u32 len_sub = 0;

    for (u32 i = 0, ri = 0; i < length; i++, ri++) {
        bool changed = false;
        if (start[i] == '&') {
            for (u32 ii = 0; ii < sizeof(g_xml_entities) / sizeof(*g_xml_entities); ii++) {
                u32 ent_len = neko_string_length(g_xml_entities[ii].name);
                if (xml_string_equal(start + i, ent_len, g_xml_entities[ii].name)) {
                    r[ri] = g_xml_entities[ii].character;
                    i += ent_len - 1;
                    len_sub += ent_len - 1;
                    changed = true;
                    break;
                }
            }
        }

        if (!changed) r[ri] = start[i];
    }

    r[length - len_sub] = '\0';

    return r;
}

// 解析XML块 返回块中的节点数组
static Array<xml_node_t> xml_parse_block(const_str start, u32 length) {
    Array<xml_node_t> root = {};

    bool is_inside = false;

    for (const_str c = start; *c && c < start + length; c++) {
        if (*c == '<') {
            c++;
            xml_expect_not_end(c);

            if (*c == '?')  // 跳过XML头
            {
                c++;
                xml_expect_not_end(c);
                while (*c != '>') {
                    c++;
                    xml_expect_not_end(c);
                }
                continue;
            } else if (xml_string_equal(c, 3, "!--"))  // 跳过注释
            {
                c++;
                xml_expect_not_end(c);
                c++;
                xml_expect_not_end(c);
                c++;
                xml_expect_not_end(c);
                while (!xml_string_equal(c, 3, "-->")) {
                    c++;
                    xml_expect_not_end(c);
                }

                continue;
            }

            if (is_inside && *c == '/')
                is_inside = false;
            else
                is_inside = true;

            const_str node_name_start = c;
            u32 node_name_len = 0;

            xml_node_t current_node = {0};

            // current_node.attributes = neko_hash_table_new(u64, xml_attribute_t);

            // current_node.children = neko_dyn_array_new(xml_node_t);

            if (is_inside) {
                for (; *c != '>' && *c != ' ' && *c != '/'; c++) node_name_len++;

                if (*c != '>') {
                    while (*c != '>' && *c != '/') {
                        while (neko_token_char_is_white_space(*c)) c++;

                        const_str attrib_name_start = c;
                        u32 attrib_name_len = 0;

                        while (neko_token_char_is_alpha(*c) || neko_token_char_is_numeric(*c) || *c == '_') {
                            c++;
                            attrib_name_len++;
                            xml_expect_not_end(c);
                        }

                        while (*c != '"') {
                            c++;
                            xml_expect_not_end(c);
                        }

                        c++;
                        xml_expect_not_end(c);

                        const_str attrib_text_start = c;
                        u32 attrib_text_len = 0;

                        while (*c != '"') {
                            c++;
                            attrib_text_len++;
                            xml_expect_not_end(c);
                        }

                        c++;
                        xml_expect_not_end(c);

                        xml_attribute_t attrib = {0};
                        attrib.name = xml_copy_string(attrib_name_start, attrib_name_len);

                        if (xml_string_is_decimal(attrib_text_start, attrib_text_len)) {
                            attrib.type = NEKO_XML_ATTRIBUTE_NUMBER;
                            attrib.value.number = strtod(attrib_text_start, NULL);
                        } else if (xml_string_equal(attrib_text_start, attrib_text_len, "true")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = true;
                        } else if (xml_string_equal(attrib_text_start, attrib_text_len, "false")) {
                            attrib.type = NEKO_XML_ATTRIBUTE_BOOLEAN;
                            attrib.value.boolean = false;
                        } else {
                            attrib.type = NEKO_XML_ATTRIBUTE_STRING;
                            attrib.value.string = xml_process_text(attrib_text_start, attrib_text_len);
                        }

                        current_node.attributes[xml_hash_string(attrib_name_start, attrib_name_len)] = attrib;
                    }
                }

                if (*c == '/')  // 对于没有任何文本的节点
                {
                    c++;
                    xml_expect_not_end(c);
                    current_node.name = xml_copy_string(node_name_start, node_name_len);
                    root.push(current_node);
                    is_inside = false;
                }
            } else {
                while (*c != '>') {
                    c++;
                    xml_expect_not_end(c);
                }
            }

            c++;
            xml_expect_not_end(c);

            if (is_inside) {
                const_str text_start = c;
                u32 text_len = 0;

                const_str end_start = c;
                u32 end_len = 0;

                current_node.name = xml_copy_string(node_name_start, node_name_len);

                for (u32 i = 0; i < length; i++) {
                    if (*c == '<' && *(c + 1) == '/') {
                        c++;
                        xml_expect_not_end(c);
                        c++;
                        xml_expect_not_end(c);
                        end_start = c;
                        end_len = 0;
                        while (*c != '>') {
                            end_len++;
                            c++;
                            xml_expect_not_end(c);
                        }

                        if (xml_string_equal(end_start, end_len, current_node.name)) {
                            break;
                        } else {
                            text_len += end_len + 2;
                            continue;
                        }
                    }

                    c++;
                    text_len++;

                    xml_expect_not_end(c);
                }

                current_node.children = xml_parse_block(text_start, text_len);
                if (current_node.children.len == 0)
                    current_node.text = xml_process_text(text_start, text_len);
                else
                    current_node.text = xml_copy_string(text_start, text_len);

                root.push(current_node);

                c--;
            }
        }
    }

    return root;
}

xml_document_t *xml_parse_vfs(const_str path) {
    size_t size;
    const_str source = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, path, &size);
    if (!source) {
        xml_emit_error("Failed to load xml file!");
        return NULL;
    }
    xml_document_t *doc = xml_parse(source);
    mem_free(source);
    return doc;
}

xml_document_t *xml_parse(const_str source) {
    if (!source) return NULL;

    g_xml_error = NULL;
    xml_document_t *doc = (xml_document_t *)mem_calloc(1, sizeof(xml_document_t));
    if (!doc) {
        xml_emit_error("Out of memory!");
        return NULL;
    }

    doc->nodes = xml_parse_block(source, neko_string_length(source));

    if (g_xml_error) {
        xml_free(doc);
        return NULL;
    }

    return doc;
}

void xml_free(xml_document_t *document) {
    for (u32 i = 0; i < document->nodes.len; i++) {
        xml_node_free(&document->nodes[i]);
    }

    document->nodes.trash();
    mem_free(document);
}

xml_attribute_t *xml_find_attribute(xml_node_t *node, const_str name) {
    u64 key = xml_hash_string(name, neko_string_length(name));
    xml_attribute_t *attr = node->attributes.get(key);
    return attr;
}

xml_node_t *xml_find_node(xml_document_t *doc, const_str name) {
    for (u32 i = 0; i < doc->nodes.len; i++) {
        if (neko_string_compare_equal(name, doc->nodes[i].name)) {
            return &doc->nodes[i];
        }
    }

    return NULL;
}

xml_node_t *xml_find_node_child(xml_node_t *node, const_str name) {
    for (u32 i = 0; i < node->children.len; i++) {
        if (neko_string_compare_equal(name, node->children[i].name)) {
            return &node->children[i];
        }
    }

    return NULL;
}

const_str xml_get_error() { return g_xml_error; }

xml_node_iter_t xml_new_node_iter(xml_document_t *doc, const_str name) {
    xml_node_iter_t it = {.doc = doc, .name = name, .idx = 0};
    return it;
}

xml_node_iter_t xml_new_node_child_iter(xml_node_t *parent, const_str name) {
    xml_node_iter_t it = {.node = parent, .name = name, .idx = 0};
    return it;
}

bool xml_node_iter_next(xml_node_iter_t *iter) {
    if (iter->node) {
        for (u32 i = iter->idx; i < iter->node->children.len; i++) {
            if (neko_string_compare_equal(iter->name, iter->node->children[i].name)) {
                iter->current = &iter->node->children[i];
                iter->idx = i + 1;
                return true;
            }
        }

        return false;
    } else {
        for (u32 i = iter->idx; i < iter->doc->nodes.len; i++) {
            if (neko_string_compare_equal(iter->name, iter->doc->nodes[i].name)) {
                iter->current = &iter->doc->nodes[i];
                iter->idx = i + 1;
                return true;
            }
        }
        return false;
    }
}

enum JSONTok : i32 {
    JSONTok_Invalid,
    JSONTok_LBrace,    // {
    JSONTok_RBrace,    // }
    JSONTok_LBracket,  // [
    JSONTok_RBracket,  // ]
    JSONTok_Colon,     // :
    JSONTok_Comma,     // ,
    JSONTok_True,      // true
    JSONTok_False,     // false
    JSONTok_Null,      // null
    JSONTok_String,    // "[^"]*"
    JSONTok_Number,    // [0-9]+\.?[0-9]*
    JSONTok_Error,
    JSONTok_EOF,
};

const char *json_tok_string(JSONTok tok) {
    switch (tok) {
        case JSONTok_Invalid:
            return "Invalid";
        case JSONTok_LBrace:
            return "LBrace";
        case JSONTok_RBrace:
            return "RBrace";
        case JSONTok_LBracket:
            return "LBracket";
        case JSONTok_RBracket:
            return "RBracket";
        case JSONTok_Colon:
            return "Colon";
        case JSONTok_Comma:
            return "Comma";
        case JSONTok_True:
            return "True";
        case JSONTok_False:
            return "False";
        case JSONTok_Null:
            return "Null";
        case JSONTok_String:
            return "String";
        case JSONTok_Number:
            return "Number";
        case JSONTok_Error:
            return "Error";
        case JSONTok_EOF:
            return "EOF";
        default:
            return "?";
    }
}

const char *json_kind_string(JSONKind kind) {
    switch (kind) {
        case JSONKind_Null:
            return "Null";
        case JSONKind_Object:
            return "Object";
        case JSONKind_Array:
            return "Array";
        case JSONKind_String:
            return "String";
        case JSONKind_Number:
            return "Number";
        case JSONKind_Boolean:
            return "Boolean";
        default:
            return "?";
    }
};

struct JSONToken {
    JSONTok kind;
    String str;
    u32 line;
    u32 column;
};

struct JSONScanner {
    String contents;
    JSONToken token;
    u64 begin;
    u64 end;
    u32 line;
    u32 column;
};

static char json_peek(JSONScanner *scan, u64 offset) { return scan->contents.data[scan->end + offset]; }

static bool json_at_end(JSONScanner *scan) { return scan->end == scan->contents.len; }

static void json_next_char(JSONScanner *scan) {
    if (!json_at_end(scan)) {
        scan->end++;
        scan->column++;
    }
}

static void json_skip_whitespace(JSONScanner *scan) {
    while (true) {
        switch (json_peek(scan, 0)) {
            case '\n':
                scan->column = 0;
                scan->line++;
            case ' ':
            case '\t':
            case '\r':
                json_next_char(scan);
                break;
            default:
                return;
        }
    }
}

static String json_lexeme(JSONScanner *scan) { return scan->contents.substr(scan->begin, scan->end); }

static JSONToken json_make_tok(JSONScanner *scan, JSONTok kind) {
    JSONToken t = {};
    t.kind = kind;
    t.str = json_lexeme(scan);
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_err_tok(JSONScanner *scan, String msg) {
    JSONToken t = {};
    t.kind = JSONTok_Error;
    t.str = msg;
    t.line = scan->line;
    t.column = scan->column;

    scan->token = t;
    return t;
}

static JSONToken json_scan_ident(Arena *a, JSONScanner *scan) {
    while (is_alpha(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    JSONToken t = {};
    t.str = json_lexeme(scan);

    if (t.str == "true") {
        t.kind = JSONTok_True;
    } else if (t.str == "false") {
        t.kind = JSONTok_False;
    } else if (t.str == "null") {
        t.kind = JSONTok_Null;
    } else {
        StringBuilder sb = {};
        neko_defer(sb.trash());

        String s = String(sb << "unknown identifier: '" << t.str << "'");
        return json_err_tok(scan, a->bump_string(s));
    }

    scan->token = t;
    return t;
}

static JSONToken json_scan_number(JSONScanner *scan) {
    if (json_peek(scan, 0) == '-' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '-'
    }

    while (is_digit(json_peek(scan, 0))) {
        json_next_char(scan);
    }

    if (json_peek(scan, 0) == '.' && is_digit(json_peek(scan, 1))) {
        json_next_char(scan);  // eat '.'

        while (is_digit(json_peek(scan, 0))) {
            json_next_char(scan);
        }
    }

    return json_make_tok(scan, JSONTok_Number);
}

static JSONToken json_scan_string(JSONScanner *scan) {
    while (json_peek(scan, 0) != '"' && !json_at_end(scan)) {
        json_next_char(scan);
    }

    if (json_at_end(scan)) {
        return json_err_tok(scan, "unterminated string");
    }

    json_next_char(scan);
    return json_make_tok(scan, JSONTok_String);
}

static JSONToken json_scan_next(Arena *a, JSONScanner *scan) {
    json_skip_whitespace(scan);

    scan->begin = scan->end;

    if (json_at_end(scan)) {
        return json_make_tok(scan, JSONTok_EOF);
    }

    char c = json_peek(scan, 0);
    json_next_char(scan);

    if (is_alpha(c)) {
        return json_scan_ident(a, scan);
    }

    if (is_digit(c) || (c == '-' && is_digit(json_peek(scan, 0)))) {
        return json_scan_number(scan);
    }

    if (c == '"') {
        return json_scan_string(scan);
    }

    switch (c) {
        case '{':
            return json_make_tok(scan, JSONTok_LBrace);
        case '}':
            return json_make_tok(scan, JSONTok_RBrace);
        case '[':
            return json_make_tok(scan, JSONTok_LBracket);
        case ']':
            return json_make_tok(scan, JSONTok_RBracket);
        case ':':
            return json_make_tok(scan, JSONTok_Colon);
        case ',':
            return json_make_tok(scan, JSONTok_Comma);
    }

    String msg = tmp_fmt("unexpected character: '%c' (%d)", c, (int)c);
    String s = a->bump_string(msg);
    return json_err_tok(scan, s);
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out);

static String json_parse_object(Arena *a, JSONScanner *scan, JSONObject **out) {
    PROFILE_FUNC();

    JSONObject *obj = nullptr;

    json_scan_next(a, scan);  // eat brace

    while (true) {
        if (scan->token.kind == JSONTok_RBrace) {
            *out = obj;
            json_scan_next(a, scan);
            return {};
        }

        String err = {};

        JSON key = {};
        err = json_parse_next(a, scan, &key);
        if (err.data != nullptr) {
            return err;
        }

        if (key.kind != JSONKind_String) {
            String msg = tmp_fmt("expected string as object key on line: %d. got: %s", (i32)scan->token.line, json_kind_string(key.kind));
            return a->bump_string(msg);
        }

        if (scan->token.kind != JSONTok_Colon) {
            String msg = tmp_fmt("expected colon on line: %d. got %s", (i32)scan->token.line, json_tok_string(scan->token.kind));
            return a->bump_string(msg);
        }

        json_scan_next(a, scan);

        JSON value = {};
        err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONObject *entry = (JSONObject *)a->bump(sizeof(JSONObject));
        entry->next = obj;
        entry->hash = fnv1a(key.string);
        entry->key = key.string;
        entry->value = value;

        obj = entry;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static String json_parse_array(Arena *a, JSONScanner *scan, JSONArray **out) {
    PROFILE_FUNC();

    JSONArray *arr = nullptr;

    json_scan_next(a, scan);  // eat bracket

    while (true) {
        if (scan->token.kind == JSONTok_RBracket) {
            *out = arr;
            json_scan_next(a, scan);
            return {};
        }

        JSON value = {};
        String err = json_parse_next(a, scan, &value);
        if (err.data != nullptr) {
            return err;
        }

        JSONArray *el = (JSONArray *)a->bump(sizeof(JSONArray));
        el->next = arr;
        el->value = value;
        el->index = 0;

        if (arr != nullptr) {
            el->index = arr->index + 1;
        }

        arr = el;

        if (scan->token.kind == JSONTok_Comma) {
            json_scan_next(a, scan);
        }
    }
}

static String json_parse_next(Arena *a, JSONScanner *scan, JSON *out) {
    switch (scan->token.kind) {
        case JSONTok_LBrace: {
            out->kind = JSONKind_Object;
            return json_parse_object(a, scan, &out->object);
        }
        case JSONTok_LBracket: {
            out->kind = JSONKind_Array;
            return json_parse_array(a, scan, &out->array);
        }
        case JSONTok_String: {
            out->kind = JSONKind_String;
            out->string = scan->token.str.substr(1, scan->token.str.len - 1);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Number: {
            out->kind = JSONKind_Number;
            out->number = string_to_double(scan->token.str);
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_True: {
            out->kind = JSONKind_Boolean;
            out->boolean = true;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_False: {
            out->kind = JSONKind_Boolean;
            out->boolean = false;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Null: {
            out->kind = JSONKind_Null;
            json_scan_next(a, scan);
            return {};
        }
        case JSONTok_Error: {
            StringBuilder sb = {};
            neko_defer(sb.trash());

            sb << scan->token.str << tmp_fmt(" on line %d:%d", (i32)scan->token.line, (i32)scan->token.column);

            return a->bump_string(String(sb));
        }
        default: {
            String msg = tmp_fmt("unknown json token: %s on line %d:%d", json_tok_string(scan->token.kind), (i32)scan->token.line, (i32)scan->token.column);
            return a->bump_string(msg);
        }
    }
}

void JSONDocument::parse(String contents) {
    PROFILE_FUNC();

    arena = {};

    JSONScanner scan = {};
    scan.contents = contents;
    scan.line = 1;

    json_scan_next(&arena, &scan);

    String err = json_parse_next(&arena, &scan, &root);
    if (err.data != nullptr) {
        error = err;
        return;
    }

    if (scan.token.kind != JSONTok_EOF) {
        error = "expected EOF";
        return;
    }
}

void JSONDocument::trash() {
    PROFILE_FUNC();
    arena.trash();
}

JSON JSON::lookup(String key, bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        for (JSONObject *o = object; o != nullptr; o = o->next) {
            if (o->hash == fnv1a(key)) {
                return o->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSON JSON::index(i32 i, bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        for (JSONArray *a = array; a != nullptr; a = a->next) {
            if (a->index == i) {
                return a->value;
            }
        }
    }

    *ok = false;
    return {};
}

JSONObject *JSON::as_object(bool *ok) {
    if (*ok && kind == JSONKind_Object) {
        return object;
    }

    *ok = false;
    return {};
}

JSONArray *JSON::as_array(bool *ok) {
    if (*ok && kind == JSONKind_Array) {
        return array;
    }

    *ok = false;
    return {};
}

String JSON::as_string(bool *ok) {
    if (*ok && kind == JSONKind_String) {
        return string;
    }

    *ok = false;
    return {};
}

double JSON::as_number(bool *ok) {
    if (*ok && kind == JSONKind_Number) {
        return number;
    }

    *ok = false;
    return {};
}

JSONObject *JSON::lookup_object(String key, bool *ok) { return lookup(key, ok).as_object(ok); }

JSONArray *JSON::lookup_array(String key, bool *ok) { return lookup(key, ok).as_array(ok); }

String JSON::lookup_string(String key, bool *ok) { return lookup(key, ok).as_string(ok); }

double JSON::lookup_number(String key, bool *ok) { return lookup(key, ok).as_number(ok); }

double JSON::index_number(i32 i, bool *ok) { return index(i, ok).as_number(ok); }

static void json_write_string(StringBuilder &sb, JSON *json, i32 level) {
    switch (json->kind) {
        case JSONKind_Object: {
            sb << "{\n";
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                sb.concat("  ", level);
                sb << o->key;
                json_write_string(sb, &o->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "}";
            break;
        }
        case JSONKind_Array: {
            sb << "[\n";
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                sb.concat("  ", level);
                json_write_string(sb, &a->value, level + 1);
                sb << ",\n";
            }
            sb.concat("  ", level - 1);
            sb << "]";
            break;
        }
        case JSONKind_String:
            sb << "\"" << json->string << "\"";
            break;
        case JSONKind_Number:
            sb << tmp_fmt("%g", json->number);
            break;
        case JSONKind_Boolean:
            sb << (json->boolean ? "true" : "false");
            break;
        case JSONKind_Null:
            sb << "null";
            break;
        default:
            break;
    }
}

void json_write_string(StringBuilder *sb, JSON *json) { json_write_string(*sb, json, 1); }

void json_print(JSON *json) {
    StringBuilder sb = {};
    neko_defer(sb.trash());
    json_write_string(&sb, json);
    neko_println("%s", sb.data);
}

void json_to_lua(lua_State *L, JSON *json) {
    switch (json->kind) {
        case JSONKind_Object: {
            lua_newtable(L);
            for (JSONObject *o = json->object; o != nullptr; o = o->next) {
                lua_pushlstring(L, o->key.data, o->key.len);
                json_to_lua(L, &o->value);
                lua_rawset(L, -3);
            }
            break;
        }
        case JSONKind_Array: {
            lua_newtable(L);
            for (JSONArray *a = json->array; a != nullptr; a = a->next) {
                json_to_lua(L, &a->value);
                lua_rawseti(L, -2, a->index + 1);
            }
            break;
        }
        case JSONKind_String: {
            lua_pushlstring(L, json->string.data, json->string.len);
            break;
        }
        case JSONKind_Number: {
            lua_pushnumber(L, json->number);
            break;
        }
        case JSONKind_Boolean: {
            lua_pushboolean(L, json->boolean);
            break;
        }
        case JSONKind_Null: {
            lua_pushnil(L);
            break;
        }
        default:
            break;
    }
}

static void lua_to_json_string(StringBuilder &sb, lua_State *L, HashMap<bool> *visited, String *err, i32 width, i32 level) {
    auto indent = [&](i32 offset) {
        if (width > 0) {
            sb << "\n";
            sb.concat(" ", width * (level + offset));
        }
    };

    if (err->len != 0) {
        return;
    }

    i32 top = lua_gettop(L);
    switch (lua_type(L, top)) {
        case LUA_TTABLE: {
            uintptr_t ptr = (uintptr_t)lua_topointer(L, top);

            bool *visit = nullptr;
            bool exist = visited->find_or_insert(ptr, &visit);
            if (exist && *visit) {
                *err = "table has cycles";
                return;
            }

            *visit = true;

            lua_pushnil(L);
            if (lua_next(L, -2) == 0) {
                sb << "[]";
                return;
            }

            i32 key_type = lua_type(L, -2);

            if (key_type == LUA_TNUMBER) {
                sb << "[";

                indent(0);
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                i32 len = luax_len(L, top);
                assert(len > 0);
                i32 i = 1;
                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be numbers";
                        return;
                    }

                    sb << ",";
                    indent(0);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    i++;
                }
                indent(-1);
                sb << "]";

                if (i != len) {
                    *err = "array is not continuous";
                    return;
                }
            } else if (key_type == LUA_TSTRING) {
                sb << "{";
                indent(0);

                lua_pushvalue(L, -2);
                lua_to_json_string(sb, L, visited, err, width, level + 1);
                lua_pop(L, 1);
                sb << ":";
                if (width > 0) {
                    sb << " ";
                }
                lua_to_json_string(sb, L, visited, err, width, level + 1);

                for (lua_pop(L, 1); lua_next(L, -2); lua_pop(L, 1)) {
                    if (lua_type(L, -2) != LUA_TSTRING) {
                        lua_pop(L, -2);
                        *err = "expected all keys to be strings";
                        return;
                    }

                    sb << ",";
                    indent(0);

                    lua_pushvalue(L, -2);
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                    lua_pop(L, 1);
                    sb << ":";
                    if (width > 0) {
                        sb << " ";
                    }
                    lua_to_json_string(sb, L, visited, err, width, level + 1);
                }
                indent(-1);
                sb << "}";
            } else {
                lua_pop(L, 2);  // key, value
                *err = "expected table keys to be strings or numbers";
                return;
            }

            visited->unset(ptr);
            break;
        }
        case LUA_TNIL:
            sb << "null";
            break;
        case LUA_TNUMBER:
            sb << tmp_fmt("%g", lua_tonumber(L, top));
            break;
        case LUA_TSTRING:
            sb << "\"" << luax_check_string(L, top) << "\"";
            break;
        case LUA_TBOOLEAN:
            sb << (lua_toboolean(L, top) ? "true" : "false");
            break;
        case LUA_TFUNCTION:
            sb << tmp_fmt("\"func %p\"", lua_topointer(L, top));
            break;
        default:
            *err = "type is not serializable";
    }
}

String lua_to_json_string(lua_State *L, i32 arg, String *contents, i32 width) {
    StringBuilder sb = {};

    HashMap<bool> visited = {};
    neko_defer(visited.trash());

    String err = {};
    lua_pushvalue(L, arg);
    lua_to_json_string(sb, L, &visited, &err, width, 1);
    lua_pop(L, 1);

    if (err.len != 0) {
        sb.trash();
    }

    *contents = String(sb);
    return err;
}